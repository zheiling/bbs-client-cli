/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../app.h"
#include "input.h"
#include <wchar.h>
#include <widget_core.h>

bool w_input_default_key_action(w_cb_args_t *args) {
  int32_t key = *((int32_t *)args->data);
  w_g_el_t *g_el = args->active_el;
  w_input_t *input = g_el->element;
  int32_t start_pos = 0;
  wchar_t w_key[2];
  app_t *app = args->app;
  char long_key[3];
  int short_val_len = 1;

  switch (key) {
  case KEY_BACKSPACE:
  case KEY_DL:
    if (input->value_len) {
      if (input->cur_pos > 0) {
        start_pos = input->value_len-- - input->cur_pos;
        memmove(input->w_value + start_pos - 1, input->w_value + start_pos,
                sizeof(wchar_t) * (input->value_len - start_pos + 2));
      } else {
        input->w_value[--input->value_len] = '\0';
      }
    }
    break;
  case KEY_RIGHT:
    if (input->cur_pos > 0) {
      input->cur_pos--;
    }
    break;
  case KEY_LEFT:
    if (input->value_len > input->cur_pos) {
      input->cur_pos++;
    }
    break;
  default:
    if (input->value_len == input->max_len)
      break;
    long_key[0] = key;
    input->value_len++;
    if ((key & 0xC0) == 0xC0) {
      long_key[1] = wgetch(app->win);
      long_key[2] = '\0';
      short_val_len++;
    } else {
      long_key[1] = '\0';
    }
    mbstowcs(w_key, (const char *)&long_key, 2);
    if (input->cur_pos > 0) {
      start_pos = input->value_len - input->cur_pos - 1;
      memmove(input->w_value + start_pos + 1, input->w_value + start_pos,
              sizeof(wchar_t) * (input->value_len - start_pos));
      input->w_value[start_pos] = key;
    } else {
      input->w_value[input->value_len - 1] = w_key[0];
      input->w_value[input->value_len] = '\0';
    }
    break;
  }
  args->resp_data.code = cbrc_none;
  return true;
}

w_input_t *w_input_init(WINDOW **win, w_t *w_parent, char *label,
                        int32_t length, int32_t is_hidden_value) {
  w_input_t *input = malloc(sizeof(w_input_t));
  w_init(&(input->w), w_parent, win, label);
  input->is_disabled = 0;
  input->w.sz.y = 3; // with borders
  input->w.sz.x =
      length + 3; // with borders and extra space for the last element
  int32_t t_len = strlen(input->w.title) + 4; // with borders and space
  if (input->w.sz.x < t_len)
    input->w.sz.x = t_len;
  input->w.ps.x = 1;
  input->w_value[0] = '\0';
  input->value_len = 0;
  input->max_len = length;
  input->is_hidden = is_hidden_value;
  input->cur_pos = 0;
  return input;
}

int32_t w_input_draw(w_input_t *input, int32_t active_id) {
  const char stars[] = "*******************";
  WINDOW *win = *(input->w.parent_win);
  int32_t pos_y = input->w.ps.y + input->w.m.y;
  int32_t pos_x = input->w.ps.x + input->w.m.x;

  /* counts margins of the ancestors of the same window */
  w_t *w_par = input->w.w_parent;
  while (w_par->parent_win == input->w.parent_win) {
    pos_y += w_par->ps.y;
    pos_x += w_par->ps.x;
    w_par = w_par->w_parent;
  }

  if (input->w.id == active_id) {
    wattrset(win, COLOR_PAIR(3) | A_BOLD);
  } else {
    wattrset(win, COLOR_PAIR(0) | A_REVERSE);
  }

  mvwhline(win, pos_y, pos_x, 0, input->w.sz.x);
  mvwhline(win, pos_y + 2, pos_x, 0, input->w.sz.x);

  // left
  mvwvline(win, pos_y, pos_x, ACS_ULCORNER, 1);
  mvwvline(win, pos_y + 1, pos_x, 0, 1);
  mvwvline(win, pos_y + 2, pos_x, ACS_LLCORNER, 1);
  // right
  mvwvline(win, pos_y, pos_x + input->w.sz.x - 1, ACS_URCORNER, 1);
  mvwvline(win, pos_y + 1, pos_x + input->w.sz.x - 1, 0, 1);
  mvwvline(win, pos_y + 2, pos_x + input->w.sz.x - 1, ACS_LRCORNER, 1);

  wattrset(win, COLOR_PAIR(0) | A_REVERSE);
  mvwprintw(win, pos_y, pos_x + 1, " %s ", input->w.title);

  pos_x++;

  if (input->w.id == active_id) {
    wattrset(win, COLOR_PAIR(modal_color_pair) | A_BOLD | A_REVERSE);
  } else {
    wattrset(win, COLOR_PAIR(0) | A_BOLD);
  }

  pos_y++;

  if (input->is_hidden) {
    mvwprintw(win, pos_y, pos_x, "%.*s%*s", (int)input->value_len, stars,
              (int)(input->w.sz.x - input->value_len - 2), "");
  } else {
    if (input->value_len > 0) {
      mvwaddnwstr(win, pos_y, pos_x, input->w_value, input->value_len);
    }
    mvwprintw(win, pos_y, pos_x + input->value_len, "%*s",
              input->w.sz.x - input->value_len - 2, "");
  }

  input->w.cur.y = pos_y;
  input->w.cur.x = pos_x + input->value_len - input->cur_pos;

  wattroff(win, A_BOLD | A_REVERSE);
  return 0;
}
/* Uses malloc! need to invoke free() after use of output */
char *w_input_get_value(w_input_t *input) {
  char *c_output = malloc(sizeof(wchar_t) * input->value_len);
  const wchar_t *p;
  p = input->w_value;
  wcsrtombs(c_output, &p, input->value_len, NULL);
  return c_output;
}

void w_input_destroy(w_input_t *input) { free(input); }