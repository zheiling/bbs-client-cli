#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "input.h"
#include <widget_core.h>

input_t *init_input(WINDOW **win, widget_t *w_parent, char *label,
                    uint32_t length, uint32_t is_hidden_value) {
  input_t *input = malloc(sizeof(input_t));
  init_widget(&(input->w), w_parent, win, label);
  input->is_disabled = 0;
  input->w.sz.y = 3;          // with borders
  input->w.sz.x = length + 3; // with borders and extra space for the last element
  uint32_t t_len = strlen(input->w.title) + 4; // with borders and space
  if (input->w.sz.x < t_len)
    input->w.sz.x = t_len;
  input->w.ps.x = 1;
  input->value[0] = '\0';
  input->value_len = 0;
  input->max_len = length;
  input->is_hidden = is_hidden_value;
  input->cur_pos = 0;
  return input;
}

int32_t draw_input(input_t *input, uint32_t active_id) {
  const char stars[] = "*******************";
  WINDOW *win = *(input->w.parent_win);
  uint32_t pos_y = input->w.ps.y + input->w.m.y;
  uint32_t pos_x = input->w.ps.x + input->w.m.x;

  /* counts margins of the ancestors of the same window */
  widget_t *w_par = input->w.w_parent;
  while (w_par->parent_win == input->w.parent_win) {
    pos_y += w_par->ps.y;
    pos_x += w_par->ps.x;
    w_par = w_par->w_parent;
  }

  if (input->w.id == active_id) {
    wattrset(win, COLOR_PAIR(3));
  } else {
    wattrset(win, COLOR_PAIR(0) | A_REVERSE | A_BOLD);
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

  if (input->is_hidden) {
    mvwprintw(win, pos_y + 1, pos_x, "%.*s%*s", (int)input->value_len,
              stars, (int)(input->w.sz.x - input->value_len - 2), "");
  } else {
    mvwprintw(win, pos_y + 1, pos_x, "%s%*s", input->value,
              (int)(input->w.sz.x - input->value_len - 2), "");
  }

  input->w.cur.y = pos_y + 1;
  input->w.cur.x = pos_x;

  wattroff(win, A_BOLD | A_REVERSE);
  return 0;
}

void destroy_input(input_t *btn) { free(btn); }