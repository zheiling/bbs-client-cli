/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include "alert.h"
#include "dialogue.h"
#include "server.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <widget.h>
#include <windows.h>

#include "file_processor.h"
#include "main.h"
#include "query.h"

static void fl_add(fl_item_t **cur, fl_item_t **start, char *fname);
fl_item_t *fl_select(fl_item_t *start, int num);
void fl_clear(fl_item_t **start, fl_item_t **current);

void file_list(file_args_t *f_args, query_args_t *q_args) {
  int32_t qlen;
  char *query = NULL;
  static char qbuf[INBUFSIZE * 2];
  static int32_t qbuf_used = 0;
  w_ui_file_list_t *fui = (w_ui_file_list_t *)q_args->main_ui->ui;
  w_dialogue_t *d = (w_dialogue_t *)q_args->active_dialogue;
  fui->start = &(f_args->l_start);
  fui->current = &(f_args->l_current);

  if (qbuf_used == 0)
    qbuf[0] = 0;

  while ((qlen = query_extract_from_buf(q_args->buf, &(q_args->buf_used),
                                        &query))) {
    if (!strncmp(":END:", query, sizeof(":END:") - 1)) {
      sscanf(query, ":END: PAGE %d/%d COUNT: %d/%d\n", &fui->current_page,
             &fui->pages, &fui->current_count, &fui->full_count);
      if (d != NULL && d->is_initiated) {
        d->needs_update = true;
      }
      w_fl_draw(fui);
      free(query);
      q_args->state = WAIT_CLIENT;
      break;
    }
    strcat(qbuf, query);
    qbuf_used += strlen(qbuf);
    if (strchr(qbuf, '\n') == NULL)
      continue;
    fl_add(&(f_args->l_current), &(f_args->l_start), qbuf);
    fui->current_count++;
    free(query);
    query = NULL;
    qbuf_used = 0;
    qbuf[0] = 0;
  }
}

int32_t ui_file_select(file_args_t *f_args, query_args_t *q_args, int32_t idx) {
  fl_item_t *l_selected; /* from the list */
  fl_item_t *f_selected =
      &(f_args->f_selected); /* new copy of file struct (list be cleared) */
  struct stat st = {0};

  l_selected = fl_select(f_args->l_start, idx);

  if (l_selected == NULL) {
    return -1;
  }

  memcpy(f_selected, l_selected, sizeof(fl_item_t));

  if (stat(DOWNLOADS_DIR, &st) == -1) {
    mkdir(DOWNLOADS_DIR, 0700);
  }
  char *file_path = malloc(sizeof(DOWNLOADS_DIR) + strlen(l_selected->name) +
                           2); // + '/' + '\0'
  f_selected->name = NULL;
  sprintf(file_path, "%s/%s", DOWNLOADS_DIR, l_selected->name);
  f_selected->name = strdup(l_selected->name);
  fl_clear(&f_args->l_start, &f_args->l_current);
  f_args->file_d = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (f_args->file_d == -1) {
    server_send_string(q_args, "error: %s\n", f_selected->name);
    perror(f_selected->name);
    q_args->state = WAIT_CLIENT;
    free(file_path);
    return -2;
  }
  server_send_string(q_args, "file download [%s]\n", f_selected->name);
  q_args->state = S_DOWNLOAD_FILE;
  free(file_path);
  return OK;
}

void file_download(file_args_t *f_args, query_args_t *q_args) {
  fl_item_t *f_selected = &(f_args->f_selected);
  static int32_t it_count = 0;
  static size_t it_interval = 0;

  if (it_interval == 0) {
    it_interval = (f_selected->size / INBUFSIZE / 100) * 5; /* every 1% */
    if (it_interval == 0)
      it_interval = 1;
  }
  char answer[256];

  static size_t size_rest = 0;
  int32_t progress = (f_selected->size - size_rest) * 100 / f_selected->size;
  w_pgb_ui_t *pb = (w_pgb_ui_t *)q_args->progress_bar;
  w_dialogue_t *d = (w_dialogue_t *)q_args->active_dialogue;
  w_ui_file_list_t *fui = (w_ui_file_list_t *)q_args->main_ui->ui;
  int32_t a_len = 0;

  if (size_rest == 0 && it_count == 0)
    size_rest = f_selected->size;
  int32_t qlen = write(f_args->file_d, q_args->buf, q_args->buf_used);
  if (qlen) {
    it_count++;
    if (!(it_count % it_interval)) {
      pb->percentage = progress;
      d->needs_update = true;
    } else {
      q_args->buf_used = 0;
    }
    if (size_rest < qlen) {
      size_rest = 0;
    } else {
      size_rest -= qlen;
    }
    if (size_rest == 0) {
      it_count = 0;
      it_interval = 0;
      f_selected->size = 0;
      close(f_args->file_d);
      d->needs_destroy = true;
      sprintf(answer, "File %s is downloaded from the server!",
              f_selected->name);
      q_args->notification = malloc(strlen(answer) + 1);
      strcpy(q_args->notification, answer);
      free(f_selected->name);
      server_send_string(q_args, "file list %d %d\n%n", fui->max_lines,
                         fui->current_page, &a_len);
      q_args->state = S_FILE_LIST;
    }
  }
}

int32_t file_upload_open(char *dpath, char *fname, query_args_t *q_args) {
  q_args->file = malloc(sizeof(p_file_t));
  q_args->file->path = malloc(strlen(dpath) + strlen(fname) + 2);
  q_args->file->signal = sig_continue;
  sprintf(q_args->file->path, "%s/%s", dpath, fname);
  int fd = open(q_args->file->path, O_RDONLY);
  if (fd == -1) {
    perror(q_args->file->path);
    return -1;
  }
  size_t fsize = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  q_args->file->fd = fd;
  q_args->file->name = malloc(strlen(fname) + 1);
  strcpy(q_args->file->name, fname);
  q_args->file->size = fsize;
  q_args->file->rest = fsize;
  q_args->file->description = NULL;
  return 0;
}

int32_t file_upload_start(query_args_t *q_args) {
  char *query;
  query_extract_from_buf(q_args->buf, &(q_args->buf_used), &query);
  if (strcmp(query, "accept")) {
    clear_file_in_query(q_args);
    w_notification("Server response", dc_alert, query);
    return -1;
  }
  return 0;
}

#define file_upload_send(send_len, package_rest, buf_indent)                   \
  {                                                                            \
    if (q_args->buf_used > 0) {                                                \
      int wlen = write(q_args->sd, q_args->buf + buf_indent, send_len);        \
      if (q_args->buf_used != wlen) {                                          \
        /* TODO: ошибка */                                                     \
        /* reset function before exit */                                       \
        it_count = 0;                                                          \
        it_interval = 0;                                                       \
        package_rest = 0;                                                      \
        ret = -1;                                                              \
      } else {                                                                 \
        it_count++;                                                            \
        if (!(it_count % it_interval)) {                                       \
          pb->percentage = progress;                                              \
          d->needs_update = true;                                              \
        }                                                                      \
        q_args->file->rest -= wlen;                                            \
        package_rest -= wlen;                                                  \
        q_args->buf_used -= send_len;                                          \
        ret = 0;                                                               \
      }                                                                        \
    } else {                                                                   \
      /* reset function before exit */                                         \
      it_count = 0;                                                            \
      it_interval = 0;                                                         \
      package_rest = 0;                                                        \
      ret = 1;                                                                 \
    }                                                                          \
  }

int32_t file_upload(query_args_t *q_args) {
  static int32_t it_count = 0;
  static size_t it_interval = 0;
  static int package_rest = 0;
  int ret = 0;
  int buf_indent = 0;
  int send_len = q_args->buf_used;

  int32_t progress =
      (q_args->file->size - q_args->file->rest) * 100 / q_args->file->size;
  w_pgb_ui_t *pb = (w_pgb_ui_t *)q_args->progress_bar;
  w_dialogue_t *d = (w_dialogue_t *)q_args->active_dialogue;
  if (it_interval == 0) {
    it_interval = (q_args->file->size / INBUFSIZE / 100) * 5; /* every 1% */
    if (it_interval == 0)
      it_interval = 1;
  }

  if (package_rest < send_len) {
    if (package_rest > 0) {
      send_len -= package_rest;
      buf_indent = package_rest;
      file_upload_send(package_rest, package_rest, 0);
      if (ret) {
        /* TODO: error case */
      }
    }

    package_rest = PACKAGE_SIZE;

    s_file_pd_t fpd = {.signal = q_args->file->signal,
                       .package_size = package_rest};
    write(q_args->sd, &fpd, sizeof(s_file_pd_t));
    if (q_args->file->signal == sig_cancel) {
      /* reset function before exit */
      it_count = 0;
      it_interval = 0;
      return -2;
    }
  }

  file_upload_send(send_len, package_rest, buf_indent);

  q_args->buf[0] = 0;
  return ret;
}

void clear_file_in_query(query_args_t *q_args) {
  free(q_args->file->name);
  free(q_args->file->path);
  if (q_args->file->description != NULL) {
    free(q_args->file->description);
  }
  free(q_args->file);
  q_args->file = NULL;
}

void init_file_args(file_args_t *f_args) {
  f_args->file_d = 0;
  f_args->f_selected.name = NULL;
  f_args->f_selected.next = NULL;
  f_args->f_selected.size = 0;
  f_args->l_current = NULL;
  f_args->l_start = NULL;
}

/* work with file list */
static void fl_add(fl_item_t **cur, fl_item_t **start, char *line) {
  char fowner[32];
  char *descr_begin = NULL;
  int h_len;
  char *name_begin = strchr(line, '[') + 1;
  char *name_end = strrchr(line, ']');
  int name_len = name_end - name_begin;
  fl_item_t *fitem = malloc(sizeof(fl_item_t));
  sscanf(name_end, "] %zu %s%n", &(fitem->size), fowner, &h_len);
  /* file name */
  fitem->name = malloc((sizeof(char)) * (name_len + 1));
  strncpy(fitem->name, name_begin, name_len);
  fitem->name[name_len] = 0;
  /* file owner */
  fitem->owner = malloc((sizeof(char)) * (strlen(fowner) + 1));
  strcpy(fitem->owner, fowner);
  /* file description */
  descr_begin = name_end + h_len + 1;
  if (strcmp(descr_begin, "\n")) {
    int d_len = strlen(line) - h_len;
    fitem->description = malloc((sizeof(char)) * (d_len));
    strcpy(fitem->description, descr_begin); // +1 to escape the dividing \32
    for (int i = 0; i < d_len; i++) {
      if (fitem->description[i] == '\a') {
        fitem->description[i] = '\n';
      }
    }
  } else {
    fitem->description = NULL;
  }
  /* fitem->description[strlen(line + line_pos)] = 0; */
  fitem->next = NULL;
  if (*cur != NULL) {
    (*cur)->next = fitem;
  } else {
    *start = fitem;
  }
  *cur = fitem;
}

fl_item_t *fl_select(fl_item_t *start, int num) {
  fl_item_t *current = start;
  for (; num != 1; num--) {
    if (current->next != NULL) {
      current = current->next;
    } else {
      return NULL;
    }
  }
  return current;
}

void fl_clear(fl_item_t **start, fl_item_t **arg_current) {
  if (*start == NULL)
    return;
  fl_item_t *next, *current;
  current = *start;

  do {
    next = current->next;
    free(current->description);
    free(current->name);
    free(current);
  } while ((current = next) != NULL);
  *start = NULL;
  *arg_current = NULL;
}