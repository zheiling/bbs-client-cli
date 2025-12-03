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

#include "ui/app.h"
#include "ui/modals/login.h"
#include "ui/widget/dialogue.h"

uint32_t m_id = 0;

void event_loop(app_t *app);

int main(int argc, char **argv) {
  app_t *app;
  app = calloc(1, sizeof(app_t));

  init_nc();

  app = init_app();

  wrefresh(app->menu_win);
  wrefresh(app->action_win);

  // temporal
  app->modal.active = login;

  init_login_modal(app);
  draw_dialogue(app, &(app->modal.dialogue));

  event_loop(app);

  destroy_app(app);
  endwin();

  return 0;
}

void event_loop(app_t *app) {
  int c;

  for (;;) {
    c = wgetch(app->win);
    switch (c) {
    // 113 is q key. This exits the program
    case 113:
      return;
    case KEY_F(9):
      return;
    default:
      if (app->modal.active != none_active) {
        app->modal.dialogue.w.callback(app->modal.dialogue.win,
                                       &(app->modal.dialogue), (void *)&c);
      }
      break;
    }

    draw_borders(app);

    wnoutrefresh(app->win);
    wnoutrefresh(app->menu_win);
    wnoutrefresh(app->action_win);
    if (app->modal.active != none_active) {
      wnoutrefresh(app->modal.dialogue.win);
    }
    doupdate();
  };
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