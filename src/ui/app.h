#ifndef N_APP_H
#define N_APP_H

#include <ncurses.h>

typedef enum {
  none = 0,
  login = 1,
} dialogue_e;

typedef struct {
  WINDOW *win;
  WINDOW *menu_win;
  WINDOW *action_win;
  struct {
    dialogue_e active;
    WINDOW *win;
  } modal;
  
  //   menu_bar_t *menu_bar;
  //   menu_t *menus;
  //   active_win_e active;
  int cur_y, cur_x;
} app_t;

app_t *init_app();
void init_nc();
void destroy_app(app_t *app);
void draw_borders(app_t *app);
void print_bars(app_t *app);

#endif // N_APP_H