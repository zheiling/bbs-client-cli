/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef COMMON_H
#define COMMON_H

#include "dlist.h"
#include <d_array.h>
#include <ncursesw/ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LOCAL_PORT 1999
#define SERVER_PORT 2000
#define LISTEN_QLEN 32
#define INBUFSIZE 1024
#define DOWNLOADS_DIR "./Downloads"
#define DIALOGUE_TITLE 64
#define DIALOGUE_TEXT 4096
#define INPUT_TEXT DIALOGUE_TITLE
#define PACKAGE_SIZE INBUFSIZE * 10

typedef struct {
  int32_t cur_y, cur_x;
  int32_t max_y, max_x;
} coordinates_t;

#define MAIN_UI_RESET(app)                                                     \
  app->main_ui.ui = NULL;                                                      \
  app->main_ui.cb_ui_refresh = NULL;                                           \
  app->main_ui.cb_b_press = NULL;                                              \
  app->main_ui.b_keys = NULL;                                                  \
  app->main_ui.b_keys_len = 0;

struct action_key {
  char *const key;
  char *const title;
  int const code;
  int (*callback)(void *app, void *d_args);
};

typedef struct fl_item {
  size_t size;
  char *name;
  char *description;
  char *owner;
} fl_item_t;

enum main_window_type {
  mw_fl_server, /* file list on the server */
  mw_fl_local,  /* local file list */
  mw_f_desc,    /* file description */
};

typedef void(draw_f_t)(void *fui);

typedef struct main_window {
  enum main_window_type type;
  void *ui; /* structure which describes window UI */
  int32_t (*cb_b_press)(void *app, void *args); /* callback on button press */
  void (*cb_ui_refresh)(void *app);             /* refresh callback */
  void (*reset)(void *app); /* function to reset the state of the window */
  draw_f_t *draw;
  struct action_key *b_keys;
  int32_t b_keys_len;
} main_window_t;

typedef struct file_args {
  int file_d;
  fl_item_t f_selected;
  fl_item_t *l_start;
  fl_item_t *l_current;
  dlist_t *f_list;
} file_args_t;

typedef struct params {
  unsigned short port;
  unsigned addr;
  char *uname;
  char *pass;
  char privileges;
  int32_t sd;
  int32_t is_connected : 1;
} params_t;

enum state {
  /* TUI-based */
  S_N_D,
  S_ASK_SEVER_IP,
  S_ASK_LOGIN_TYPE,
  S_ASK_LOGIN_USER,
  S_ASK_REGISTER,
  S_WAIT_SERVER,
  S_PREP_SERVER_MESSAGE,
  S_PRINT_SERVER_MESSAGE,
  S_FILE_LIST,
  S_WAIT_PASS,
  /* S_FILE_SELECT, */
  S_DOWNLOAD_FILE,
  S_UPLOAD_PARAMS,
  S_UPLOAD_FILE,
  /* S_UPLOAD_FILE_SELECT, */
  S_UPLOAD_REQUESTED,
  S_UPLOAD_SERVER_FINISHES,
  S_ASK_USER_BEFORE_LOGIN,
  S_ERR,
  S_NEXT_ACTION,
  S_WAIT_REGISTER_CONFIRMATION,
  S_WAIT_USER_DESCRIPTION,
  /* Legacy */
  WAIT_SERVER_INIT,
  WAIT_SERVER,
  WAIT_REGISTER,
  WAIT_REGISTER_CONFIRMATION,
  WAIT_CLIENT,
};

enum package_signal { sig_continue, sig_cancel, sig_finish, sig_pause };

typedef struct {
  enum package_signal signal;
  size_t package_size;
} s_file_pd_t; /* file package descriptor */

typedef struct p_file {
  char *name;
  char *path;
  char *description;
  size_t size;
  size_t rest;
  size_t package_rest;
  int32_t it_count;
  size_t it_interval;
  enum package_signal signal;
  int fd;
} p_file_t;

typedef struct query_args {
  int32_t sd;
  int32_t buf_used;
  p_file_t *file;
  enum state state;
  char buf[INBUFSIZE];
  bool from_server;
  params_t *params;
  struct {
    char *text;
    int32_t capacity;
    int32_t size;
  } server_message;
  main_window_t *main_ui;
  void *progress_bar;
  void *active_dialogue;
  void *notification;
} query_args_t;

enum w_dialogue_color_scheme { dc_normal, dc_alert };

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

enum w_pmt_attrs {
  PMT_POS_CENTER = 01,
  PMT_ALIGN_CENTER = 03,
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

typedef int32_t (*w_cb_press_t)(void *app, void *data);

typedef void (*w_cb_t)(w_cb_args_t *args);

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

typedef struct group_el_init_t {
  enum w_type type;
  bool is_default;
  bool is_hidden_value;
  char label[DIALOGUE_TITLE];
  int32_t length;
  enum w_g_dir direction;
  union {
    int32_t num;
  } val;
  struct group_el_init_t *children;
} w_group_el_init_t;

typedef struct group_t {
  w_t w;
  w_g_el_t *elements;
  enum w_g_dir direction;
  int32_t count;
  int32_t first_id, last_id;
  struct group_t *parent_group;
} w_group_t;

typedef struct {
  w_t w;
  WINDOW *win;
  w_group_t *g_content;
  w_group_t *g_action;
  w_g_el_t *active_el;
  char text[DIALOGUE_TEXT];
  bool is_initiated;
  bool needs_update;
  bool needs_destroy;
  coordinates_t *p_coordinates;
  enum w_dialogue_color_scheme color_scheme;
  d_array_ptr_t id_map;
} w_dialogue_t;

typedef struct app_t {
  WINDOW *win;
  w_dialogue_t modal;
  params_t *params;
  coordinates_t coordinates;
  query_args_t *query_args;
  file_args_t *file_args;
  void (*active_callback)(w_cb_args_t *args);
  void (*callback_after_notification)(struct app_t *);
  void *active_widget;
  wchar_t *top_text;
  main_window_t main_ui;
} app_t;

#endif