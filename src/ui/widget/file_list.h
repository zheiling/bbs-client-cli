#ifndef FILE_LIST_H
#define FILE_LIST_H
#include "../widget.h"
#include <stdint.h>
typedef struct {
    widget_t w;
    fl_item_t *current;
    fl_item_t *start;
    int32_t current_idx;
} file_list_t;

void draw_file_list(file_list_t *fl_ui, fl_item_t **fl_start);
file_list_t *init_file_list(WINDOW **win);
#endif