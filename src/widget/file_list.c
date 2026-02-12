#include "file_list.h"
#include "../file_processor.h"
#include "../fs.h"
#include "app.h"
#include <bstrlib.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void reset_file_list(ui_file_list_t *fl_ui);

void file_list_cb(callback_args_t *args) {
  app_t *app = args->app;
  ui_file_list_t *fui = app->query_args->file_list_ui;
  int32_t key = *((int32_t *)args->data);
  char query[256];
  char q_prefix[128];
  int32_t q_len = 0;
  if (fui->search_key->slen > 0) {
    sprintf(q_prefix, "file search name %s", fui->search_key->data);
  } else {
    sprintf(q_prefix, "file list");
  }
  if (fui->active_search) {
    if (key == KEY_BACKSPACE || key == KEY_DL) {
      if (blength(fui->search_key)) {
        btrunc(fui->search_key, blength(fui->search_key) - 1);
        draw_file_list(fui);
      }
    } else if (key == '\n') {
      fui->active_search = false;
      fl_clear(fui->start, fui->current);
      app->query_args->state = S_FILE_LIST;
      if (fui->search_key->slen > 0) {
        sprintf(query, "%s %u %u\n%n", q_prefix, fui->max_lines, 1, &q_len);
      } else {
        sprintf(query, "%s %u %u\n%n", q_prefix, fui->max_lines, 1, &q_len);
      }
      write(app->query_args->sd, query, q_len);
      reset_file_list(fui);
    } else {
      bconchar(fui->search_key, key);
      draw_file_list(fui);
    }
    return;
  }
  switch (key) {
  case KEY_DOWN:
    if (fui->current_idx < fui->current_count - 1) {
      fui->current_idx++;
      draw_file_list(fui);
    } else if (fui->current_page < fui->pages) {
      fl_clear(fui->start, fui->current);
      app->query_args->state = S_FILE_LIST;
      sprintf(query, "%s %u %u\n%n", q_prefix, fui->max_lines,
              fui->current_page + 1, &q_len);
      write(app->query_args->sd, query, q_len);
      reset_file_list(fui);
    }
    break;
  case KEY_UP:
    if (fui->current_idx > 0) {
      fui->current_idx--;
      draw_file_list(fui);
    } else if (fui->current_page > 1) {
      fl_clear(fui->start, fui->current);
      app->query_args->state = S_FILE_LIST;
      sprintf(query, "%s %u %u\n%n", q_prefix, fui->max_lines,
              fui->current_page - 1, &q_len);
      write(app->query_args->sd, query, q_len);
      reset_file_list(fui);
      fui->activate_last = true;
    }
    break;
  case '\n':
    ui_file_select(app->file_args, app->query_args, fui->current_idx + 1);
    break;
  }
}

ui_file_list_t *init_file_list(WINDOW **win, WINDOW *const *info_win) {
  ui_file_list_t *fui = malloc(sizeof(ui_file_list_t));
  init_widget(&(fui->w), NULL, win, "");
  WINDOW *parent_win = *(fui->w.parent_win);
  fui->current_idx = 0;
  fui->current_page = 1;
  fui->pages = 1;
  fui->current_count = 0;
  fui->full_count = 0;
  fui->activate_last = false;
  fui->info_win = info_win;
  fui->max_lines = getmaxy(parent_win) - 3; /* 2+1 (bottom info line) */
  fui->active_search = false;
  fui->search_key = bfromcstrrangealloc(12, 64, "");
  return fui;
}

void reset_file_list(ui_file_list_t *fl_ui) {
  fl_ui->current_idx = 0;
  fl_ui->current_page = 0;
  fl_ui->pages = 0;
  fl_ui->current_count = 0;
  fl_ui->full_count = 0;
  fl_ui->activate_last = false;
  fl_ui->active_search = false;
}

void draw_file_list(ui_file_list_t *fui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *parent_win = *(fui->w.parent_win);
  getmaxyx(parent_win, sz_y, sz_x);
  int32_t sz_y_f = sz_y - 2; /* TODO: можно заменить на поле max_lines */
  p_y = 1;
  p_x = 1;
  fl_item_t *el = *(fui->start);
  fl_item_t *active_el = *(fui->start);
  int32_t cur_el_idx = 0;
  curs_set(false);

  if (fui->activate_last) {
    fui->current_idx = fui->current_count - 1;
    fui->activate_last = false;
  }

  if (fui->current_idx + 1 >= sz_y_f) {
    p_y -= fui->current_idx - sz_y_f + 2;
  }

  do {
    if (el == NULL)
      break;
    if (p_y < 1) {
      p_y++;
      cur_el_idx++;
      continue;
    }
    if (cur_el_idx == fui->current_idx) {
      wattrset(parent_win, A_BOLD | A_REVERSE);
      active_el = el;
    }
    p_x = 1;
    mvwprintw(parent_win, p_y, p_x, "%s%n", el->name, &p_x);
    mvwprintw(parent_win, p_y, p_x + 1, "%*s", sz_x - p_x, "");
    if (cur_el_idx == fui->current_idx) {
      wattroff(parent_win, A_BOLD | A_REVERSE);
    }
    p_y++;
    cur_el_idx++;
  } while ((el = el->next) != NULL && p_y < sz_y_f);

  p_x = 1;

  for (; p_y < sz_y_f; p_y++) {
    mvwprintw(parent_win, p_y, p_x, "%*s", sz_x - 1, "");
  }
  char p_info[64];
  int32_t p_len;
  p_x = 1;

  if (fui->active_search) {
    sprintf(p_info, "[search]: %s_%n", fui->search_key->data, &p_len);
  } else {
    sprintf(p_info, "page: %u/%u files: %u left: %u%n", fui->current_page,
            fui->pages, fui->current_count, fui->full_count, &p_len);
  }

  uint32_t l_pad = (sz_x - p_len) / 2;

  wattrset(parent_win, A_BOLD);
  if (fui->active_search) {
    mvwprintw(parent_win, p_y, p_x, "%s%*s", p_info, sz_x - p_len, "");
  } else {
    /* TODO: Correct spacing when lack of space (with search keyword) */
    if (fui->search_key->slen > 0) {
      wattrset(parent_win, A_REVERSE);
      mvwprintw(parent_win, p_y, p_x, "[%s]", fui->search_key->data);
      wattroff(parent_win, A_REVERSE);
      mvwprintw(parent_win, p_y, p_x + fui->search_key->slen + 2, " %s",
                p_info);
    } else {
      mvwprintw(parent_win, p_y, p_x, "%*s%s%*s", l_pad, "", p_info, l_pad, "");
    }
  }
  wattroff(parent_win, A_BOLD);

  /* draw file info */
  p_y = 1;
  p_x = 1;
  WINDOW *i_win = *fui->info_win;
  wclear(i_win);

  /* Write to the right side the information about the file */
  if (active_el == NULL)
    return;

  char size_text[64];
  size_to_text(active_el->size, size_text);

  mvwprintw(i_win, p_y++, p_x, "Size: %s", size_text);
  mvwprintw(i_win, p_y++, p_x, "Owner: %s", active_el->owner);
  mvwprintw(i_win, p_y++, p_x, "Description: ");
  print_multiline_text(i_win, active_el->description, sz_x, p_y, p_x, 0);
}