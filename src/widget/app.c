#include "app.h"
#include <locale.h>
#include <ncursesw/ncurses.h>
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
  set_escdelay(50);
  setlocale(LC_ALL, "");

  if (!has_colors()) {
    printf("Your terminal does not support color\n");
  } else {
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(modal_color_pair, COLOR_BLUE, COLOR_WHITE);
    init_pair(4, COLOR_CYAN, COLOR_WHITE);
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

struct action_key {
  char *const key;
  char *const title;
};

int64_t print_bottom_menu_option(WINDOW *win, char *key, char *title, int64_t y,
                                 int64_t x, int32_t size) {
  int32_t key_len = 0;
  int32_t title_len = 0;

  wattrset(win, A_BOLD);
  mvwprintw(win, y, x, " %s%n", key, &key_len);
  size -= key_len + strlen(title) + 1;
  wattrset(win, COLOR_PAIR(4) | A_REVERSE);
  mvwprintw(win, y, x + key_len, "%s %*s%n", title, size, "", &title_len);
  wattrset(win, COLOR_PAIR(0) | A_REVERSE);
  return key_len + title_len;
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

  struct action_key action_keys[] = {
      {.key = "F1", .title = "Help"},
      {.key = "U", .title = "Upload"},
      {.key = "S", .title = "Search"},
      {.key = "F9", .title = "Quit"},
  };

  int t_margin = 0; /* tab margin */
  int c_size = app->coordinates.max_x / 4; /* column size */
  int incr_last = app->coordinates.max_x % 4; /* increment size fo the last element (fill empty space) */

  for (int i = 0; i < 3; i++) {
    struct action_key *k = &action_keys[i];
    t_margin +=
        print_bottom_menu_option(app->win, k->key, k->title,
                                 app->coordinates.cur_y - 2, t_margin, c_size);
  }

  t_margin += print_bottom_menu_option(
      app->win, action_keys[3].key, action_keys[3].title,
      app->coordinates.cur_y - 2, t_margin, c_size + incr_last);

  wattroff(app->win, A_REVERSE);
}

void app_refresh(app_t *app) {
  draw_borders(app);

  wnoutrefresh(app->win);
  wnoutrefresh(app->right_win);
  wnoutrefresh(app->left_win);
  if (app->modal.win != NULL) {
    if (app->modal.is_initiated != 0) {
      wnoutrefresh(app->modal.win);
    }
  }
  doupdate();
}

void destroy_app(app_t *app, int32_t exit_code) {
  delwin(app->win);
  endwin();
  app->win = NULL;
  exit(exit_code);
}
