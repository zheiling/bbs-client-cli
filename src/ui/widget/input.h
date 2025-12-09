#ifndef INPUT_H
#define INPUT_H

#include "../../main.h"
#include "../widget.h"
#include <stdint.h>

typedef struct {
  widget_t w;
  char value[INPUT_TEXT];
  uint32_t value_len;
  uint32_t max_len;
  uint32_t is_disabled : 1;
} input_t;

input_t *init_input(WINDOW **win, widget_t *w_parent, char *label, uint32_t size);
int32_t draw_input(input_t *input, uint32_t active_id);
void destroy_input(input_t *input);
#endif