#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <widget_core.h>
#include <stdint.h>

typedef struct {
  widget_t w;
  bool is_disabled;
  bool value;
} checkbox_t;

bool checkbox_default_key_action(callback_args_t *args);
checkbox_t *init_checkbox(WINDOW **win, widget_t *w_parent, char *label);
int32_t draw_checkbox(checkbox_t *cbx, uint32_t active_id);
void destroy_checkbox(checkbox_t *cbx);
#endif
