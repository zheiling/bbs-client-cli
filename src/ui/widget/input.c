#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../app.h"
#include "../widget.h"
#include "input.h"

input_t *init_input(WINDOW **win, widget_t *w_parent, char *label,
                    uint32_t length) {
  input_t *input = malloc(sizeof(input_t));
  init_widget(&(input->w), w_parent, win, label);
  input->is_disabled = 0;
  input->w.y = 3;                              // with borders
  input->w.x = length + 2;                     // with borders
  uint32_t t_len = strlen(input->w.title) + 4; // with borders and space
  if (input->w.x < t_len)
    input->w.x = t_len;
  input->w.m_x = 1;
  input->value[0] = '\0';
  input->value_len = 0;
  input->max_len = length;
  return input;
}

int32_t draw_input(input_t *input, uint32_t active_id) {
  WINDOW *win = *(input->w.parent_win);
  uint32_t margin_y = input->w.m_y + input->w.w_parent->m_y;
  uint32_t margin_x = input->w.m_x + input->w.w_parent->m_x;

  wattrset(win, COLOR_PAIR(MODAL_COLOR_PAIR));

  mvwhline(win, margin_y, margin_x, 0, input->w.x);
  mvwhline(win, margin_y + 2, margin_x, 0, input->w.x);

  mvwprintw(win, margin_y, margin_x + 1, " %s ", input->w.title);

  // left
  mvwvline(win, margin_y, margin_x, ACS_ULCORNER, 1);
  mvwvline(win, margin_y + 1, margin_x, 0, 1);
  mvwvline(win, margin_y + 2, margin_x, ACS_LLCORNER, 1);
  // right
  mvwvline(win, margin_y, margin_x + input->w.x - 1, ACS_URCORNER, 1);
  mvwvline(win, margin_y + 1, margin_x + input->w.x - 1, 0, 1);
  mvwvline(win, margin_y + 2, margin_x + input->w.x - 1, ACS_LRCORNER, 1);

  // TODO: correct sizes (x)

  margin_x++;

  if (input->w.id == active_id) {
    wattrset(win, COLOR_PAIR(MODAL_COLOR_PAIR) | A_BOLD | A_REVERSE);
  } else {
    wattrset(win, COLOR_PAIR(0) | A_BOLD);
  }

  uint32_t x_c = 0;
  mvwprintw(win, margin_y + 1, margin_x, "%s", input->value);
  input->w.cur.y = margin_y + 1;
  input->w.cur.x = margin_x;
  for (int i = 0; i < input->w.x - input->value_len - 2; i++) {
    mvwprintw(win, margin_y + 1, margin_x + i + input->value_len, " ");
  }

  wattroff(win, A_BOLD | A_REVERSE);
  return 0;
}

void destroy_input(input_t *btn) { free(btn); }