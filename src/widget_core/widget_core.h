/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef WIDGET_CORE_H
#define WIDGET_CORE_H

#include "../main.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>

enum w_g_type { g_content, g_action };

enum w_g_dir {
  horizontal,
  vertical,
};

enum w_type {
  w_end,
  w_button,
  w_box,
  w_group,
  w_input,
  w_checkbox,
  w_progress,
};

enum w_val_type {
  val_num,
  val_ptr,
  val_nul,
};

struct w_val_t {
  enum w_val_type type;
  union {
    void *ptr;
    int32_t num;
  } val;
};

typedef struct {
  int32_t id;
  int32_t idx; /* element index in the group */
  void *element;
  enum w_type type;
  enum w_g_type g_type;
  bool is_default;
  struct w_val_t val;
} w_g_el_t;

/* widget callback response code */
enum w_cbrp_code {
  cbrc_none, /* default */
  cbrp_val,
  cbrp_g_el, /* element of type group_el_t */
  cbrp_err
};

/* widget callback response data */
typedef struct {
 enum w_cbrp_code code;
 struct w_val_t val;
} w_cbrp_data;

typedef struct {
  void *app;
  void *element;
  void *data;
  w_cbrp_data resp_data;
  void *active_el;
} w_cb_args_t;

typedef void (*w_cb_t)(w_cb_args_t *args);
typedef int32_t (*w_cb_press_t)(void *app, void *data);

typedef struct w_t {
  int32_t id;
  char title[DIALOGUE_TITLE];
  struct {
    int32_t y;
    int32_t x;
  } cur; /* cursor position */
  struct {
    int32_t y;
    int32_t x;
  } sz; /* size */
  struct {
    int32_t y;
    int32_t x;
  } m; /* margin (relative to the parent) */
  struct {
    int32_t y;
    int32_t x;
  } ps; /* position (relative to the window) */
  WINDOW *const *parent_win;
  struct w_t *w_parent;
  w_cb_t callback;
} w_t;

enum w_pmt_attrs {
  PMT_POS_CENTER = 01,
  PMT_ALIGN_CENTER = 03,
};

void     w_init(w_t *w, w_t *w_parent, WINDOW **win, char *title);
int32_t  w_get_max_line_len(const char *text, int32_t *line_count);
int32_t  w_print_multiline_text(WINDOW *win, const char *text,
                              const int32_t win_width, const int32_t y,
                              const int32_t x, const int16_t attrs);

#endif