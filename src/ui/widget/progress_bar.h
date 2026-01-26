#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H
#include "../widget.h"
typedef struct {
  widget_t w;
  uint32_t procent;
  char text[24];
} ui_progress_bar_t;
void destroy_ui_progress_bar(void *_pb);
void draw_ui_progress_bar(ui_progress_bar_t *pb);
ui_progress_bar_t *init_ui_progress_bar(WINDOW **win, widget_t *w_parent);
#endif