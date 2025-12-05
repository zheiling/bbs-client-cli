#include "../app.h"
#include "../widget/dialogue.h"
#include "../widget/group.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

/* TODO: improve init process (make it more readable) */

void group_callback(callback_args_t *args, void *widget, void *data,
                    void *resp_data) {
  group_resp_t response;
  callback_args_t ch_args = {.win = args->win, .app = NULL};
  group_default_callback(&ch_args, widget, data, (void *)&response);
  if (response.value > -1) {
    // TODO: finish actions
    switch (response.value) {
    case 0:
      exit(10);
    case 1:
      exit(11);
    case 2:
      exit(12);
    }
  }
}

dialogue_t *init_login_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t children[] = {{.type = w_button, .label = "Anonymous"},
                                {.type = w_button, .label = "User"},
                                {.type = w_button, .label = "Register"},
                                {.type = w_end}};

  dialogue_t *d = init_dialogue("Login",
                                "Select your login "
                                "option",
                                app->cur_y, app->cur_x);

  memcpy(&(app->modal.dialogue), d, sizeof(dialogue_t));
  free(d);
  app->modal.dialogue.ch_group =
      init_group(&(app->modal.dialogue.win), &(d->w), children, horizontal);
  app->modal.dialogue.ch_group->w.callback = group_callback;

  return NULL;
}