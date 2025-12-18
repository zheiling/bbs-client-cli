#include "file_list.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

file_list_t *init_file_list(WINDOW **win) {
  file_list_t *fl_ui = malloc(sizeof(file_list_t));
  init_widget(&(fl_ui->w), NULL, win,"");
  return fl_ui;
}

void draw_file_list(file_list_t *fl_ui, fl_item_t **fl_start) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  getmaxyx(*(fl_ui->w.parent_win), sz_y, sz_x);
  p_y = 1;
  p_x = 1;
  fl_item_t *el = *fl_start;

  do {
    p_x = 1;
    mvwprintw(*(fl_ui->w.parent_win), p_y, p_x, "%s%n", el->name, &p_x);
    for (uint i = 1; p_x < sz_x; i++) {
        mvwprintw(*(fl_ui->w.parent_win), p_y, ++p_x, " ");
    }
    p_y++;
  } while ((el = el->next) != NULL && p_y < sz_y);

}