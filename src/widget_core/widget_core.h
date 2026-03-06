#ifndef WIDGET_CORE_H
#define WIDGET_CORE_H

#include "../main.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>

enum g_type { g_content, g_action };

enum g_direction {
  horizontal,
  vertical,
};

enum w_type {
  w_end,
  w_button,
  w_box,
  w_group,
  w_input,
  w_progress,
  w_fs_file_list,
};

enum val_type {
  val_num,
  val_ptr,
  val_nul,
};

struct val_t {
  enum val_type type;
  union {
    void *ptr;
    int64_t num;
  } val;
};

typedef struct {
  int64_t id;
  int64_t idx; /* element index in the group */
  void *element;
  enum w_type type;
  enum g_type g_type;
  bool is_default;
  struct val_t val;
} group_el_t;

enum cbrp_code {
  cbrc_none, /* default */
  cbrp_val,
  cbrp_g_el, /* element of type group_el_t */
  cbrp_err
};

typedef struct {
  void *app;
  void *element;
  void *data;
  struct {
    enum cbrp_code code;
    struct val_t val;
  } resp_data;
  void *active_el;
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
  int64_t id;
  char title[DIALOGUE_TITLE];
  struct {
    int64_t y;
    int64_t x;
  } cur; /* cursor position */
  struct {
    int64_t y;
    int64_t x;
  } sz; /* size */
  struct {
    int64_t y;
    int64_t x;
  } m; /* margin (relative to the parent) */
  struct {
    int64_t y;
    int64_t x;
  } ps; /* position (relative to the window) */
  WINDOW *const *parent_win;
  struct widget_t *w_parent;
  void (*callback)(callback_args_t *args);
} widget_t;

typedef void (*callback_t)(callback_args_t *args);

enum pmt_attrs {
  PMT_POS_CENTER = 01,
  PMT_ALIGN_CENTER = 03,
};

void init_widget(widget_t *w, widget_t *w_parent, WINDOW **win, char *title);
int32_t get_max_line_len(const char *text, uint32_t *line_count);
uint32_t print_multiline_text(WINDOW *win, const char *text,
                              const uint32_t win_width, const uint32_t y,
                              const uint32_t x, const uint16_t attrs);

#endif