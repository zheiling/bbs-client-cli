#ifndef FS_FILE_LIST_H
#define FS_FILE_LIST_H
#include "../widget.h"
#include <ncurses.h>
#include <stdint.h>

typedef struct {
    widget_t w;
    fl_item_t **current;
    fl_item_t **start;
    int32_t current_idx;
    uint32_t current_count;
    uint32_t activate_last : 1;
    WINDOW *const* info_win;
} ui_fs_file_list_t;

void draw_fs_file_list(ui_fs_file_list_t *fl_ui);
void redraw_fs_file_list(ui_fs_file_list_t *fl_ui);
void destroy_fs_file_list(ui_fs_file_list_t *fl_ui);
ui_fs_file_list_t *init_fs_file_list(WINDOW **win, widget_t *w_parent);
void fs_file_list_cb(callback_args_t *args);
void reset_fs_file_list(ui_fs_file_list_t *fl_ui);
#endif