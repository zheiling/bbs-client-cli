#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <widget.h>

void main_window_set(w_app_t *app, enum main_window_type type);
void main_window_draw(w_app_t *app);
void main_window_destroy(w_app_t *app);

#endif