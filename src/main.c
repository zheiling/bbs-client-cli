#include <arpa/inet.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <widget.h>

#include "connection.h"
#include "file_list.h"
#include "main.h"
#include "query.h"
#include "modals/alert.h"

uint32_t m_id = 0;

int32_t process_user_input(app_t *app, callback_args_t *d_args);

int main(int argc, char **argv) {
  app_t *app;
  params_t params;
  query_args_t *q_args = malloc(sizeof(query_args_t));

  app = calloc(1, sizeof(app_t));

  init_nc();

  app = init_app();
  init_params(&params);
  app->params = &params;
  analyze_args(argc, argv, &params);

  /* init alert */
  init_alert(app);

  wrefresh(app->left_win);
  wrefresh(app->right_win);

  init_query_args(q_args, app->params);
  q_args->buf = malloc(INBUFSIZE);
  app->query_args = q_args;
  q_args->file_list_ui = init_file_list(&(app->left_win), &(app->right_win));

  /* init client to connect to the server */
  app->params->sd = init_client();

  if (params.addr == 0) {
    app->query_args->state = S_ASK_SEVER_IP;
  } else {
    connect_to_server(app);
    app->query_args->state = S_WAIT_SERVER;
  }

  app->query_args->sd = app->params->sd;

  query_loop(app);
  clear_params(&params);
  destroy_app(app, 0);

  return OK;
}

int32_t process_user_input(app_t *app, callback_args_t *d_args) {
  int32_t c;
  ui_file_list_t *fui = (ui_file_list_t *)app->query_args->file_list_ui;
  c = wgetch(app->win);
  switch (c) {
  case KEY_F(9):
    destroy_app(app, 0);
    return OK;
  case 'U':
  case 'u':
    if (!app->modal.is_initiated) {
      app->query_args->state = S_UPLOAD_FILE_SELECT;
      break;
    }
  case 's':
  case 'S':
    if (!app->modal.is_initiated && !fui->active_search) {
      fui->active_search = true;
      draw_file_list(fui);
      return OK;
    }
  default:
    d_args->data = (void *)&c;
    app->active_callback(d_args);
    break;
  }
  return OK;
}