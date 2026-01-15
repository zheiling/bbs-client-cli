#include "app.h"
#include "modals/ask_server_addr.h"
#include "modals/download_pr.h"
#include "modals/login_credentials.h"
#include "modals/login_option.h"
#include "modals/notification.h"
#include "modals/server_message.h"
#include "modals/upload_dialogue.h"
#include "widget/dialogue.h"
#include <ncurses.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void ac_file(WINDOW *win, int is_action_w);

app_t *init_app() {
  /* get values from terminal size */
  int32_t y_max, x_max;
  getmaxyx(stdscr, y_max, x_max);

  /* allocate app struct memory */
  app_t *_app = calloc(1, sizeof(app_t));

  /* associate values for screen size */
  _app->coordinates.max_x = _app->coordinates.cur_x = x_max;
  _app->coordinates.max_y = _app->coordinates.cur_y = y_max;

  _app->win = newwin(_app->coordinates.cur_y, _app->coordinates.cur_x, 0, 0);

  /* define the width for each sub window */
  int32_t left_w_x = x_max / 10 * 4;
  int32_t right_w_x = x_max - left_w_x - 2;

  /* create the menu window */
  _app->left_win = newwin(y_max - 4, left_w_x, 2, 1);

  /* create the action window */
  _app->right_win = newwin(y_max - 4, right_w_x, 2, left_w_x + 1);

  /* dialogue */
  _app->modal.win = NULL;
  _app->modal.is_initiated = 0;

  /* default win */
  _app->active_win_type = aw_left;

  /* print decorative bars */
  print_bars(_app);

  /* here goes box borders */
  draw_borders(_app);

  keypad(_app->win, TRUE);

  /* refresh the windows */
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
  curs_set(false);

  if (!has_colors()) {
    printf("Your terminal does not support color\n");
  } else {
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(modal_color_pair, COLOR_BLUE, COLOR_WHITE);
  }
}

void draw_borders(app_t *app) {
  if (app->modal.win != NULL && !app->modal.needs_update) {
    return;
  }
  clear();
  box(app->win, 0, 0);
  box(app->left_win, 0, 0);
  box(app->right_win, 0, 0);
}

void print_bars(app_t *app) {
  char top_text[64] = "Hello!";

  wattrset(app->win, A_REVERSE);

  /* print top and bottom bars */
  for (int i = 1; i < app->coordinates.cur_x - 1; i++) {
    mvwprintw(app->win, 1, i, " ");
    mvwprintw(app->win, app->coordinates.cur_y - 2, i, " ");
  }

  /* add content to the top bar */
  if (app->params && app->params->is_connected) {
    uint32_t ip_address = app->params->addr;
    u_char ip_addr[4];
    mempcpy(ip_addr, &ip_address, 4);
    sprintf(top_text, "Connected to %u.%u.%u.%u, user: %s", ip_addr[0],
            ip_addr[1], ip_addr[2], ip_addr[3], app->params->uname);
  }
  mvwprintw(app->win, 1, 2, "%s", top_text);

  /* add content to the bottom bar */
  mvwprintw(app->win, app->coordinates.cur_y - 2, 2,
            "F1 - Help | U - upload | F9 - Quit");

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
  if (app->modal.needs_destroy) {
    destroy_dialogue(&(app->modal), app);
    app_refresh(app);
  }
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
    case S_FILE_DOWNLOAD:
      init_dwn_pr_modal(app);
      break;
    case S_UPLOAD_FILE:
      init_upload_dialogue_modal(app);
      break;
    case WAIT_SERVER_INIT:
    case WAIT_SERVER:
    case WAIT_REGISTER:
    case WAIT_REGISTER_CONFIRMATION:
    case WAIT_CLIENT:
    case S_FILE_LIST:
    case S_FILE_SELECT:
    case S_UPLOAD_PARAMS:
    case S_UPLOAD_REQUESTED:
    case S_ASK_USER_BEFORE_LOGIN:
    case S_ERR:
    case S_WAIT_SERVER:
    case S_N_D:
    case S_NEXT_ACTION:
      if (app->query_args->notification != NULL) {
        init_notification_modal(app);
      } else {
        return;
      }
    }
    app->active_callback = app->modal.w.callback;
    app->active_win = app->modal.win;
    app->active_win_type = aw_modal;
    app->active_widget = &(app->modal);
  }
  draw_dialogue(&(app->modal));
}

void destroy_app(app_t *app, int32_t exit_code) {
  delwin(app->win);
  endwin();
  app->win = NULL;
  exit(exit_code);
}
