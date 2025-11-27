#include "app.h"
// #include "action.h"
#include <stdlib.h>

void ac_file(WINDOW *win, int is_action_w);

app_t *init_app() {
  // get values from terminal size
  int y_max, x_max;
  getmaxyx(stdscr, y_max, x_max);

  // allocate app struct memory
  app_t *_app = calloc(1, sizeof(app_t));

  // associate values for screen size
  _app->cur_y = y_max;
  _app->cur_x = x_max;

  _app->win = newwin(_app->cur_y, _app->cur_x, 0, 0);

  // define the width for each sub window
  int menu_w_x = x_max / 6;
  int action_w_x = x_max - menu_w_x - 2;

  // create the menu window
  _app->menu_win = newwin(y_max - 4, menu_w_x, 2, 1);

  // create the action window
  _app->action_win = newwin(y_max - 4, action_w_x, 2, menu_w_x + 1);

  // dialogue
  _app->modal.dialogue.win = NULL;

  // print decorative bars
  print_bars(_app);

  // here goes box borders
  draw_borders(_app);

  // init menus
  //   _app->menus = malloc(3 * sizeof(menu_t));
  //   _app->menus[0] = init_menu("(F)ile", 'f');
  //   _app->menus[1] = init_menu("(E)dit", 'e');
  //   _app->menus[2] = init_menu("(A)bout", 'a');

  // set active window by default
  //   _app->active = N_MENU;

  // call the action window content by default
  //   ac_file(_app->action_win, _app->active);

  // refresh the windows
  wnoutrefresh(_app->win);
  wnoutrefresh(_app->menu_win);
  wnoutrefresh(_app->action_win);
  doupdate();

  return _app;
}

void init_nc() {
  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);

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
  wnoutrefresh(app->menu_win);
  wnoutrefresh(app->action_win);
  doupdate();
}

void print_bars(app_t *app) {
  wattrset(app->win, A_REVERSE);

  // print top and bottom bars
  for (int i = 1; i < app->cur_x - 1; i++) {
    mvwprintw(app->win, 1, i, " ");
    mvwprintw(app->win, app->cur_y - 2, i, " ");
  }

  // add content to the top bar
  mvwprintw(app->win, 1, 2, "example cprint_barsurses app | ");
  mvwprintw(app->win, 1, sizeof "example cprint_barsurses app | " + 1,
            "size %d, %d", app->cur_x, app->cur_y);

  // add content to the bottom bar
  mvwprintw(app->win, app->cur_y - 2, 2, "F1 - Help | F9 - Quit");

  wattroff(app->win, A_REVERSE);
}

void destroy_app(app_t *app) {
  delwin(app->win);
  free(app);
}
