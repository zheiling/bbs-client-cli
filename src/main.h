#include <stddef.h>

#define LOCAL_PORT 1999
#define SERVER_PORT 2000
#define LISTEN_QLEN 32
#define INBUFSIZE 1024
#define DOWNLOADS_DIR "./Downloads"

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
  WAIT_CLIENT,
  UPLOAD_FILE,
  STATE_FILE_LIST,
  STATE_FILE_SELECT,
  STATE_FILE_DOWNLOAD,
  STATE_UPLOAD_PARAMS,
  STATE_UPLOAD_FILE,
  STATE_UPLOAD_REQUESTED,
  STATE_ASK_USER_BEFORE_LOGIN,
  ERR,
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

void init_params(params_t *restrict);
void analyze_args(int argc, char *argv[], params_t *restrict);
void get_missing_params(params_t *restrict);
void connect_to_server(int sd, params_t *params);
int init_client();
void close_session(int sd);
int process_query(query_args_t *query_args, file_args_t *file_args);
void query_loop(int sd, params_t *);
int query_extract_from_buf(char *buf, int *buf_used, char **output_line);
int process_client_command(char *line, int l_len, query_args_t *q_args);
int process_server_command(char *line, int l_len, query_args_t *q_args);

void clear_params(params_t *params);

/* file processor */
void file_list(file_args_t *f_args, query_args_t *q_args);
void file_download(file_args_t *f_args, query_args_t *q_args);
void file_select(file_args_t *fargs, query_args_t *q_args);
void init_file_args(file_args_t *fargs);
int file_upload_request(char *, query_args_t *q_args);
int file_upload_start(query_args_t *q_args);
int file_upload(query_args_t *q_args);

void print_prompt(params_t *params);