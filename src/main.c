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

#include "connection.h"
#include "main.h"
#include "ui/app.h"
#include "ui/modals/login.h"
#include "ui/widget.h"
#include "ui/widget/dialogue.h"

uint32_t m_id = 0;

void event_loop(app_t *app);
void app_refresh(app_t *app);

int main(int argc, char **argv) {
  app_t *app;
  params_t params;
  app = calloc(1, sizeof(app_t));

  init_nc();

  app = init_app();
  init_params(&params);
  app->params = &params;
  analyze_args(argc, argv, &params);

  wrefresh(app->left_win);
  wrefresh(app->right_win);

  // temporal
  app->modal.type = login;

  event_loop(app);

  destroy_app(app);
  endwin();

  return 0;
}

void app_draw_modal(app_t *app);

void event_loop(app_t *app) {
  int c;

  callback_args_t d_args = {.app = app,
                            .win = NULL,
                            .widget = NULL,
                            .data = (void *)&c,
                            .resp_data = NULL};
  app_draw_modal(app);
  app_refresh(app);
  for (;;) {
    c = wgetch(app->win);
    switch (c) {
      // 113 is q key. This exits the program
      case 113:
      return;
      case KEY_F(9):
      return;
      default:
      if (app->modal.type != none) {
        d_args.win = app->modal.dialogue.win;
        d_args.widget = &(app->modal.dialogue);
        app->modal.dialogue.w.callback(&d_args);
      }
      break;
    }
    app_draw_modal(app);
    app_refresh(app);
  };
}

void app_refresh(app_t *app) {
  draw_borders(app);

  wnoutrefresh(app->win);
  wnoutrefresh(app->left_win);
  wnoutrefresh(app->right_win);
  if (app->modal.type != none) {
    if (app->modal.dialogue.is_initiated == 0) {
      app->modal.type = none;
    } else {
      wnoutrefresh(app->modal.dialogue.win);
    }
  }
  doupdate();
}

void app_draw_modal(app_t *app) {
  if (app->modal.type != none && !app->modal.dialogue.is_initiated) {
    switch (app->modal.type) {
    case login:
      init_login_modal(app);
      break;
    case none:
    default:
      break;
    }
    draw_dialogue(&(app->modal.dialogue));
  }
}

// int main(int argc, char *argv[]) {
//   params_t params;
//   init_params(&params);
//   analyze_args(argc, argv, &params);
//   get_missing_params(&params);
//   int sd = init_client();
//   connect_to_server(sd, &params);
//   query_loop(sd, &params);
//   clear_params(&params);
//   return 0;
// }