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

#include "client.h"
#include "main.h"
#include "query.h"
#include "ui/widget/dialogue.h"
#include "ui/widget/file_list.h"
#include "ui/widget/progress_bar.h"

static void fl_add(fl_item_t **cur, fl_item_t **start, char *fname);
fl_item_t *fl_select(fl_item_t *start, int num);
void fl_clear(fl_item_t **start, fl_item_t **current);

static void clear_file_in_query(query_args_t *q_args);

void file_list(file_args_t *f_args, query_args_t *q_args) {
  uint32_t qlen;
  char *query = NULL;
  static uint32_t idx = 1;
  static char qbuf[INBUFSIZE * 2];
  static uint32_t qbuf_used = 0;
  ui_file_list_t *fui = (ui_file_list_t *)q_args->file_list_ui;
  fui->start = &(f_args->l_start);
  fui->current = &(f_args->l_current);

  if (qbuf_used == 0)
    qbuf[0] = 0;

  while ((qlen = query_extract_from_buf(q_args->buf, &(q_args->buf_used),
                                        &query))) {
    if (!strcmp("list_end\n", query)) {
      q_args->state = S_FILE_SELECT;
      draw_file_list(fui);
      idx = 1;
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

  uint32_t current_page, pages, current_count, full_count;

  while ((qlen = query_extract_from_buf(q_args->buf, &(q_args->buf_used),
                                        &query))) {
    if (4 == sscanf(query, "=== PAGE %u/%u COUNT: %u/%u ===", &current_page,
                    &pages, &current_count, &full_count)) {
      fui->current_page = current_page;
      fui->current_count = current_count;
      fui->full_count = full_count;
      fui->pages = pages;
      draw_file_list(fui);
      return;
    }
  }
}

/* file_select: old function, can be removed in the future */
void file_select(file_args_t *f_args, query_args_t *q_args) {
  char *buf = q_args->buf;
  fl_item_t *l_start = f_args->l_start;
  // fl_item_t *l_current = f_args->l_current;
  fl_item_t *l_selected; /* from the list */
  fl_item_t *f_selected =
      &(f_args->f_selected); /* new copy of file struct (list be cleared) */
  uint32_t filenum;
  uint32_t qlen;
  char send_buf[256];
  struct stat st = {0};

  if (!(sscanf(buf, "%u", &filenum))) {
    write(q_args->sd, buf, strlen(buf));
    q_args->state = S_FILE_LIST;
    fl_clear(&f_args->l_start, &f_args->l_current);
    return;
  }

  if (filenum == 0) {
    print_prompt(q_args->params);
    q_args->state = WAIT_CLIENT;
    return;
  }
  l_selected = fl_select(l_start, filenum);

  if (l_selected == NULL) {
    printf("Selected number is not correct!\nPlease try again!\n");
    return;
  }

  memcpy(f_selected, l_selected, sizeof(*f_selected));

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
    qlen = sprintf(send_buf, "error: %s\n", f_selected->name);
    write(q_args->sd, send_buf, qlen);
    perror(f_selected->name);
    print_prompt(q_args->params);
    q_args->state = WAIT_CLIENT;
    free(file_path);
    return;
  }
  qlen = sprintf(send_buf, "file download %s\n", f_selected->name);
  write(q_args->sd, send_buf, qlen);
  q_args->state = S_FILE_DOWNLOAD;
  free(file_path);
}

int32_t ui_file_select(file_args_t *f_args, query_args_t *q_args, int32_t idx) {
  fl_item_t *l_selected; /* from the list */
  fl_item_t *f_selected =
      &(f_args->f_selected); /* new copy of file struct (list be cleared) */
  uint32_t qlen;
  char send_buf[256];
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
  /* fl_clear(&f_args->l_start, &f_args->l_current); */
  f_args->file_d = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (f_args->file_d == -1) {
    qlen = sprintf(send_buf, "error: %s\n", f_selected->name);
    write(q_args->sd, send_buf, qlen);
    perror(f_selected->name);
    print_prompt(q_args->params);
    q_args->state = WAIT_CLIENT;
    free(file_path);
    return -2;
  }
  qlen = sprintf(send_buf, "file download %s\n", f_selected->name);
  write(q_args->sd, send_buf, qlen);
  q_args->state = S_FILE_DOWNLOAD;
  free(file_path);
  return OK;
}

void file_download(file_args_t *f_args, query_args_t *q_args) {
  fl_item_t *f_selected = &(f_args->f_selected);
  static uint32_t it_count = 0;
  static size_t it_interval = 0;
  if (it_interval == 0) {
    it_interval = (f_selected->size / INBUFSIZE / 100) * 5; /* every 1% */
    if (it_interval == 0)
      it_interval = 1;
  }

  static size_t size_rest = 0;
  uint32_t progress = (f_selected->size - size_rest) * 100 / f_selected->size;
  ui_progress_bar_t *pb = (ui_progress_bar_t *)q_args->progress_bar;
  dialogue_t *d  = (dialogue_t *)q_args->active_dialogue;

  if (size_rest == 0)
    size_rest = f_selected->size;
  uint32_t qlen = write(f_args->file_d, q_args->buf, q_args->buf_used);
  if (qlen) {
    it_count++;
    if (!(it_count % it_interval)) {
      pb->procent = progress;
      d->needs_update = true;
    }
    if (it_count % it_interval)
      q_args->buf_used = 0;
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
      free(f_selected->name);
      d->needs_destroy = true;
      q_args->state = WAIT_CLIENT;
    }
  }
}

int32_t file_upload_request(char *query, query_args_t *q_args) {
  char qbuf[INBUFSIZE];
  char fpath[512];
  char a_perm;
  sscanf(query, "file upload \"%s %c", fpath, &a_perm);
  fpath[strlen(fpath) - 1] = 0; // remove \"
  int fd = open(fpath, O_RDONLY);
  if (fd == -1) {
    perror(fpath);
    return -1;
  }
  size_t fsize = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *fname = strrchr(fpath, '/') + 1;
  int qlen = snprintf(qbuf, INBUFSIZE, "file upload \"%s\" %zu %c", fname,
                      fsize, a_perm);
  if (qlen > INBUFSIZE) {
    fprintf(stderr, "query is too large to be sent\n");
    return -1;
  }
  q_args->file = malloc(sizeof(p_file_t));
  q_args->file->fd = fd;
  q_args->file->name = malloc(strlen(fname) + 1);
  strcpy(q_args->file->name, fname);
  q_args->file->size = fsize;
  q_args->file->rest = fsize;
  write(q_args->sd, qbuf, qlen);
  return 0;
}

int32_t file_upload_start(query_args_t *q_args) {
  char *query;
  query_extract_from_buf(q_args->buf, &(q_args->buf_used), &query);
  if (strcmp(query, "accept")) {
    clear_file_in_query(q_args);
    write(STDERR_FILENO, query, strlen(query));
    return -1;
  }
  return 0;
}

int32_t file_upload(query_args_t *q_args) {
  /* static size_t it_int = 0;
   static int it_count = 0;

   if (it_int == 0) {
     it_int = (q_args->file->size / INBUFSIZE / 100) * 10; // every 10%
} */

  if (q_args->buf_used > 0) {
    int wlen = write(q_args->sd, q_args->buf, q_args->buf_used);
    if (q_args->buf_used != wlen) {
      /* TODO: ошибка */
      return -1;
    } else {
      q_args->buf[0] = 0;
      q_args->buf_used = 0;
      q_args->file->rest -= wlen;
    }
  } else {
    /* file upload is finished */
    printf("Upload of %s is finished.\n", q_args->file->name);
    /* TODO: ждать ответа от сервера */
    clear_file_in_query(q_args);
    return 1;
  }
  return 0;
}

static void clear_file_in_query(query_args_t *q_args) {
  free(q_args->file->name);
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
  char fname[128];
  char fowner[32];
  int h_len;
  fl_item_t *fitem = malloc(sizeof(fl_item_t));
  sscanf(line, "%s %zu %s%n", fname, &(fitem->size), fowner, &h_len);
  /* file name */
  fitem->name = malloc((sizeof(char)) * (strlen(fname) + 1));
  strcpy(fitem->name, fname);
  /* file owner */
  fitem->owner = malloc((sizeof(char)) * (strlen(fowner) + 1));
  strcpy(fitem->owner, fowner);
  /* file description */
  int d_len = strlen(line) - h_len;
  fitem->description = malloc((sizeof(char)) * (d_len));
  strcpy(fitem->description, line + h_len + 1); // +1 to escape the dividing \32
  int i;
  for (i = 0; i < d_len; i++) {
    if (fitem->description[i] == '\a') {
      fitem->description[i] = '\n';
    }
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