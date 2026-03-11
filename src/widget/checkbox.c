#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "checkbox.h"
#include <widget_core.h>

bool checkbox_default_key_action(callback_args_t *args) {
  int32_t key = *((int32_t *)args->data);
  group_el_t *g_el = args->active_el;
  checkbox_t *cbx = g_el->element;
  switch (key) {
  case '\40':
    cbx->value = !cbx->value;
    break;
  default:
    break;
  }
  return true;
}

checkbox_t *init_checkbox(WINDOW **win, widget_t *w_parent, char *label) {
  checkbox_t *cbx = malloc(sizeof(checkbox_t));
  init_widget(&(cbx->w), w_parent, win, label);
  cbx->is_disabled = false;
  cbx->value = false;
  cbx->w.sz.x = strlen(cbx->w.title) + 4; // + "[ ] "
  cbx->w.sz.y = 1;
  cbx->w.cur.y = 0;
  cbx->w.cur.x = 1;
  return cbx;
}

int32_t draw_checkbox(checkbox_t *cbx, uint32_t active_id) {
  char title[DIALOGUE_TITLE + 6];
  WINDOW *win = *(cbx->w.parent_win);
  uint32_t margin_y = cbx->w.ps.y + cbx->w.m.y;
  uint32_t margin_x = cbx->w.ps.x + cbx->w.m.x;

  widget_t *w_par = cbx->w.w_parent;

  while (w_par->parent_win == cbx->w.parent_win) {
    margin_y += w_par->ps.y;
    margin_x += w_par->ps.x;
    w_par = w_par->w_parent;
  }

  char val = cbx->value ? 'x' : ' ';

  if (cbx->w.id == active_id) {
    sprintf(title, "[%c] %s", val, cbx->w.title);
    wattrset(win, COLOR_PAIR(modal_color_pair) | A_BOLD | A_REVERSE);
  } else {
    sprintf(title, "[%c] %s", val, cbx->w.title);
    wattrset(win, COLOR_PAIR(0) | A_REVERSE);
  }

  mvwprintw(win, margin_y, margin_x, "%s", title);
  cbx->w.cur.y = margin_y;
  cbx->w.cur.x = margin_x + 1;

  wattroff(win, A_BOLD | A_REVERSE);
  return strlen(title);
}

void destroy_checkbox(checkbox_t *btn) { free(btn); }