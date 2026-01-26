#ifndef WIDGET_H
#define WIDGET_H

#include "../main.h"
#include <ncurses.h>
#include <stdint.h>

enum w_type {
  w_end,
  w_button,
  w_box,
  w_group,
  w_input,
  w_progress,
  w_fs_file_list,
};

typedef struct {
  void *app;
  void *element;
  void *data;
  void *resp_data;
  uint32_t active_id;
} callback_args_t;

enum rsize {
  s_auto,
  s_1,
  s_1_2,
  s_1_3,
  s_2_3,
  s_1_4,
  s_3_4,
};

typedef struct widget_t {
  uint32_t id;
  char title[DIALOGUE_TITLE];
  uint32_t x;
  uint32_t y;
  uint32_t m_x;
  uint32_t m_y;
  struct {
    uint32_t y;
    uint32_t x;
  } cur;
  WINDOW *const *parent_win;
  struct widget_t *w_parent;
  // enum rsize rsize;
  void (*callback)(callback_args_t *args);
} widget_t;

typedef void (*callback_t)(callback_args_t *args);

enum pmt_attrs {
  PMT_POS_CENTER = 01,
  PMT_ALIGN_CENTER = 03,
};

void init_widget(widget_t *w, widget_t *w_parent, WINDOW **win, char *title);
int32_t get_max_line_len(const char *text, uint32_t *line_count);
uint32_t print_multiline_text(WINDOW *win, const char *text, const uint32_t win_width, const uint32_t y,
                              const uint32_t x, const uint16_t attrs);

#endif