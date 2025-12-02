#ifndef MAIN_H
#define MAIN_H

#include <stddef.h>
#include <stdint.h>
#include "types.h"

#define LOCAL_PORT 1999
#define SERVER_PORT 2000
#define LISTEN_QLEN 32
#define INBUFSIZE 1024
#define DOWNLOADS_DIR "./Downloads"
#define DIALOGUE_TITLE 64
#define DIALOGUE_TEXT 128

typedef struct params {
  unsigned short port;
  unsigned addr;
  char *uname;
  char *pass;
  char privileges;
} params_t;

enum state {
  WAIT_SERVER_INIT,
  WAIT_SERVER,
  WAIT_REGISTER,
  WAIT_REGISTER_CONFIRMATION,
  WAIT_CLIENT,
  UPLOAD_FILE,
  STATE_FILE_LIST,
  STATE_FILE_SELECT,
  STATE_FILE_DOWNLOAD,
  STATE_UPLOAD_PARAMS,
  STATE_UPLOAD_FILE,
  STATE_UPLOAD_REQUESTED,
  STATE_ASK_USER_BEFORE_LOGIN,
  STATE_ERR,
};

typedef struct p_file {
  char *name;
  size_t size;
  size_t rest;
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

typedef struct query_args {
  int sd;
  int buf_used;
  p_file_t *file;
  enum state state;
  char *buf;
  char from_server;
  params_t *params;
} query_args_t;

#endif