#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../app.h"
#include "../widget.h"
#include "button.h"

button_t *init_button(WINDOW **win, widget_t *w_parent, char *label) {
  button_t *btn = malloc(sizeof(button_t));
  init_widget(&(btn->w), w_parent,  win, label);
  btn->is_disabled = 0;
  btn->is_focused = 0;
  btn->is_hovered = 0;
  btn->callback = NULL;
  btn->w.x = strlen(btn->w.title) + 4; // + "[  ]"
  btn->w.y = 1; // TODO: detect new line
  if (btn->is_focused) btn->w.x += 2; // + "<>"
  return btn;
}

int32_t draw_button(button_t *btn) {
  char title[DIALOGUE_TITLE + 6];
  WINDOW *win = *(btn->w.parent_win);
  uint32_t margin_y = btn->w.m_y + btn->w.w_parent->m_y;
  uint32_t margin_x = btn->w.m_x + btn->w.w_parent->m_x;

  if (btn->is_focused) {
    sprintf(title, "[< %s >]", btn->w.title);
  } else {
    sprintf(title, "[ %s ]", btn->w.title);
  }

  if (btn->is_hovered) {
    wattrset(win, COLOR_PAIR(MODAL_COLOR_PAIR) | A_BOLD | A_REVERSE);
  } else {
    wattrset(win, COLOR_PAIR(0) | A_REVERSE);
  }

  mvwprintw(win, margin_y, margin_x, "%s", title);

  wattroff(win, A_BOLD | A_REVERSE);
  return strlen(title);
}

void destroy_button(button_t *btn) { free(btn); }