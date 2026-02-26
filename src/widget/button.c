#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include <widget_core.h>
#include "button.h"

button_t *init_button(WINDOW **win, widget_t *w_parent, char *label) {
  button_t *btn = malloc(sizeof(button_t));
  init_widget(&(btn->w), w_parent, win, label);
  btn->is_disabled = 0;
  btn->w.x = strlen(btn->w.title) + 6; // + "[  ]"
  btn->w.y = 1;                        // TODO: detect new line
  btn->w.cur.y = 0;
  btn->w.cur.x = 0;
  return btn;
}

int32_t draw_button(button_t *btn, uint32_t active_id) {
  char title[DIALOGUE_TITLE + 6];
  WINDOW *win = *(btn->w.parent_win);
  uint32_t margin_y = btn->w.m_y;
  uint32_t margin_x = btn->w.m_x;

  widget_t *w_par = btn->w.w_parent;

  while (w_par->parent_win == btn->w.parent_win) {
    margin_y += w_par->m_y;
    margin_x += w_par->m_x;
    w_par = w_par->w_parent;
  }

  if (btn->w.id == active_id) {
    sprintf(title, "[< %s >]", btn->w.title);
    wattrset(win, COLOR_PAIR(modal_color_pair) | A_BOLD | A_REVERSE);
  } else {
    sprintf(title, "[  %s  ]", btn->w.title);
    wattrset(win, COLOR_PAIR(0) | A_REVERSE);
  }

  mvwprintw(win, margin_y, margin_x, "%s", title);
  btn->w.cur.y = margin_y;
  btn->w.cur.x = margin_x;

  wattroff(win, A_BOLD | A_REVERSE);
  return strlen(title);
}

void destroy_button(button_t *btn) { free(btn); }