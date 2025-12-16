#include "app.h"
#include "modals/ask_server_addr.h"
#include "modals/login_credentials.h"
#include "modals/login_option.h"
#include "modals/server_message.h"
// #include "modals/server_message.h"
// #include "action.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void ac_file(WINDOW *win, int is_action_w);

app_t *init_app() {
  // get values from terminal size
  int y_max, x_max;
  getmaxyx(stdscr, y_max, x_max);

  // allocate app struct memory
  app_t *_app = calloc(1, sizeof(app_t));

  // associate values for screen size
  _app->coordinates.max_x = _app->coordinates.cur_x = x_max;
  _app->coordinates.max_y = _app->coordinates.cur_y = y_max;

  _app->win = newwin(_app->coordinates.cur_y, _app->coordinates.cur_x, 0, 0);

  // define the width for each sub window
  int menu_w_x = x_max / 6;
  int action_w_x = x_max - menu_w_x - 2;

  // create the menu window
  _app->left_win = newwin(y_max - 4, menu_w_x, 2, 1);

  // create the action window
  _app->right_win = newwin(y_max - 4, action_w_x, 2, menu_w_x + 1);

  // dialogue
  _app->modal.win = NULL;
  _app->modal.is_initiated = 0;

  // print decorative bars
  print_bars(_app);

  // here goes box borders
  draw_borders(_app);

  keypad(_app->win, TRUE);

  // refresh the windows
  wnoutrefresh(_app->win);
  wnoutrefresh(_app->left_win);
  wnoutrefresh(_app->right_win);
  doupdate();

  return _app;
}

void init_nc() {
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);

  if (!has_colors()) {
    printf("Your terminal does not support color\n");
  } else {
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(MODAL_COLOR_PAIR, COLOR_BLUE, COLOR_WHITE);
  }
}

void draw_borders(app_t *app) {
  clear();
  box(app->win, 0, 0);

  // refresh the windows
  wnoutrefresh(app->win);
  wnoutrefresh(app->left_win);
  wnoutrefresh(app->right_win);
  doupdate();
}

void print_bars(app_t *app) {
  wattrset(app->win, A_REVERSE);

  // print top and bottom bars
  for (int i = 1; i < app->coordinates.cur_x - 1; i++) {
    mvwprintw(app->win, 1, i, " ");
    mvwprintw(app->win, app->coordinates.cur_y - 2, i, " ");
  }

  // add content to the top bar
  mvwprintw(app->win, 1, 2, "example cprint_barsurses app | ");
  mvwprintw(app->win, 1, sizeof "example cprint_barsurses app | " + 1,
            "size %d, %d", app->coordinates.cur_x, app->coordinates.cur_y);

  // add content to the bottom bar
  mvwprintw(app->win, app->coordinates.cur_y - 2, 2, "F1 - Help | F9 - Quit");

  wattroff(app->win, A_REVERSE);
}
void app_refresh(app_t *app) {
  draw_borders(app);

  wnoutrefresh(app->win);
  wnoutrefresh(app->left_win);
  wnoutrefresh(app->right_win);
  if (app->modal.win != NULL) {
    if (app->modal.is_initiated != 0) {
      wnoutrefresh(app->modal.win);
    }
  }
  doupdate();
}

void app_draw_modal(app_t *app) {
  if (!app->modal.is_initiated) {
    switch (app->query_args->state) {
    case S_ASK_SEVER_IP:
      init_asa_modal(app);
      break;
    case S_ASK_LOGIN_TYPE:
      init_login_option_modal(app);
      break;
    case S_ASK_LOGIN_USER:
      init_login_credentials_modal(app);
      break;
    case S_PRINT_SERVER_MESSAGE:
      init_server_message_modal(app);
      break;
    case WAIT_SERVER_INIT:
    case WAIT_SERVER:
    case WAIT_REGISTER:
    case WAIT_REGISTER_CONFIRMATION:
    case WAIT_CLIENT:
    case UPLOAD_FILE:
    case S_FILE_LIST:
    case S_FILE_SELECT:
    case S_FILE_DOWNLOAD:
    case S_UPLOAD_PARAMS:
    case S_UPLOAD_FILE:
    case S_UPLOAD_REQUESTED:
    case S_ASK_USER_BEFORE_LOGIN:
    case S_ERR:
    case S_WAIT_SERVER:
    case S_N_D:
      return;
    }
  }
  draw_dialogue(&(app->modal));
}

void destroy_app(app_t *app) {
  delwin(app->win);
  endwin();
}
