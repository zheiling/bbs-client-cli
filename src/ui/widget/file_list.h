#ifndef FILE_LIST_H
#define FILE_LIST_H
#include "../widget.h"
#include <stdint.h>

typedef struct {
    widget_t w;
    fl_item_t **current;
    fl_item_t **start;
    int32_t current_idx;
    uint32_t current_count;
    uint32_t full_count;
    uint32_t pages;
    uint32_t current_page;
    uint32_t activate_last : 1;
    WINDOW *const* info_win;
} ui_file_list_t;

void draw_file_list(ui_file_list_t *fl_ui);
void redraw_file_list(ui_file_list_t *fl_ui);
ui_file_list_t *init_file_list(WINDOW **win, WINDOW *const* info_win);
void file_list_cb(callback_args_t *args);
#endif