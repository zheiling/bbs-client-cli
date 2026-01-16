#ifndef FS_FILE_LIST_H
#define FS_FILE_LIST_H
#include "../widget.h"
#include <ncurses.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct fs_fl_item {
  char *name;
  struct fs_fl_item *next;
  struct fs_fl_item *prev;
  u_char d_type;
} fs_fl_item_t;

typedef struct {
    widget_t w;
    fs_fl_item_t *current;
    fs_fl_item_t *start;
    int32_t current_idx;
    WINDOW *const* info_win;
    char d_path[512];
} ui_fs_file_list_t;

void draw_fs_file_list(ui_fs_file_list_t *fl_ui);
void redraw_fs_file_list(ui_fs_file_list_t *fl_ui);
void destroy_fs_file_list(ui_fs_file_list_t *fl_ui);
ui_fs_file_list_t *init_fs_file_list(WINDOW **win, widget_t *w_parent);
void fs_file_list_cb(callback_args_t *args);
void reset_fs_file_list(ui_fs_file_list_t *fl_ui);
#endif