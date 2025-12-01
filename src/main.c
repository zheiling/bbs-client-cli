#include <stdlib.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>
#include <ncurses.h>

#include "ui/app.h"
#include "ui/widget/dialogue.h"
#include "ui/modals/login.h"


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
  
  dialogue_t *dialogue = init_login_modal();
  draw_dialogue(dialogue, app->cur_x, app->cur_y);

  event_loop(app);

  destroy_app(app);
  endwin();

  return 0;
}

void event_loop(app_t *app) {
  int c;

  for (;;) {
    c = wgetch(app->win);

    // if (app->modal.active != none_active) {
    //   modal_event_loop(c, app);
    //   continue;
    // }

    switch (c) {
    // 113 is q key. This exits the program
    case 113:
      return;
    case KEY_F(9):
      return;
    case 'e':
      // ac_edit(app);
      break;
    // 27 is ESC key. Usage of ESC is not recommended, but for now this clears
    // the menus
    case 27:
      // app->menu_bar->selected_menu = -1;
      break;
    // 9 or \t is horizontal tab
    case 9:
      // if (app->active == N_MENU) {
      //   app->active = N_ACTION;
      // } else {
      //   app->active = N_MENU;
      // }
      break;
    default:
      break;
    }

    draw_borders(app);

    wnoutrefresh(app->win);
    wnoutrefresh(app->menu_win);
    wnoutrefresh(app->action_win);
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