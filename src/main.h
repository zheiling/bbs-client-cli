/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef MAIN_H
#define MAIN_H

#include "common.h"
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
  S_WAIT_SERVER_READY,
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
  enum package_signal signal;
  int fd;
} p_file_t;

typedef struct fl_item {
  size_t size;
  char *name;
  char *description;
  char *owner;
  struct fl_item *next;
} fl_item_t;

typedef struct file_args {
  int file_d;
  fl_item_t f_selected;
  fl_item_t *l_start;
  fl_item_t *l_current;
} file_args_t;

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
  void (*reset)(void *app);                     /* function to reset the state of the window */
  draw_f_t *draw;
  struct action_key *b_keys;
  int32_t b_keys_len;
} main_window_t;

typedef struct query_args {
  int32_t sd;
  int32_t buf_used;
  p_file_t *file;
  enum state state;
  char *buf;
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

#endif