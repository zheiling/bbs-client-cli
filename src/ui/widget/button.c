#include "button.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

button_t *init_button() {
  button_t *btn = malloc(sizeof(button_t));
  btn->is_disabled = 0;
  btn->is_focused = 0;
  btn->is_hovered = 0;
  btn->callback = NULL;
  return btn;
}

int32_t draw_button(WINDOW *win, button_t *btn, uint32_t cur_x, uint32_t cur_y) {
  char title[DIALOGUE_TITLE + 6];

  if (btn->is_focused) {
    sprintf(title, "[< %s >]", btn->w.title);
  } else {
    sprintf(title, "[ %s ]", btn->w.title);
  }

  if (btn->is_hovered) {
    wattrset(win, COLOR_PAIR(1) | A_BOLD | A_REVERSE);
  } else {
    wattrset(win, COLOR_PAIR(1) | A_BOLD);
  }

  mvwprintw(win, cur_y, cur_x, "%s", title);

  wattroff(win, A_REVERSE | A_BOLD);
  return strlen(title);
}

void destroy_button(button_t *btn) {
    free(btn);
}