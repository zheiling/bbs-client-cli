#ifndef INPUT_H
#define INPUT_H

#include "../main.h"
#include <ui-widget-core.h>
#include <stdint.h>

typedef struct {
  widget_t w;
  char value[INPUT_TEXT];
  uint32_t cur_pos; /* is relative to the end of a string */
  uint32_t value_len;
  uint32_t max_len;
  uint32_t is_disabled : 1;
  uint32_t is_hidden : 1;
} input_t;

input_t *init_input(WINDOW **win, widget_t *w_parent, char *label,
                    uint32_t size, uint32_t is_hidden_value);
int32_t draw_input(input_t *input, uint32_t active_id);
void destroy_input(input_t *input);
#endif