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
#define DIALOGUE_TEXT 4096
#define INPUT_TEXT DIALOGUE_TITLE
#ifndef __USE_GNU
#define __USE_GNU 1
#endif

typedef struct params {
  unsigned short port;
  unsigned addr;
  char *uname;
  char *pass;
  char privileges;
  int32_t sd;
  uint32_t is_connected : 1;
} params_t;

enum state {
  S_N_D,
  S_ASK_SEVER_IP,
  S_ASK_LOGIN_TYPE,
  S_ASK_LOGIN_USER,
  S_WAIT_SERVER,
  S_PRINT_SERVER_MESSAGE,
  S_FILE_LIST,
  S_FILE_SELECT,
  S_FILE_DOWNLOAD,
  S_UPLOAD_PARAMS,
  S_UPLOAD_FILE,
  S_UPLOAD_REQUESTED,
  S_ASK_USER_BEFORE_LOGIN,
  S_ERR,
  S_NEXT_ACTION,
  WAIT_SERVER_INIT,
  WAIT_SERVER,
  WAIT_REGISTER,
  WAIT_REGISTER_CONFIRMATION,
  WAIT_CLIENT,
};

typedef struct p_file {
  char *name;
  char *path;
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
  int32_t sd;
  int32_t buf_used;
  p_file_t *file;
  enum state state;
  char *buf;
  uint32_t from_server : 1;
  params_t *params;
  char *next_server_command;
  struct {
    char *text;
    uint64_t capacity;
    uint64_t size;
  } server_message;
  void *file_list_ui; /* TODO: find solution to insert actual type without void pointers */
  void *progress_bar;
  void *active_dialogue;
  char *notification;
} query_args_t;

#endif