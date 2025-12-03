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

dialogue_t *init_login_modal(app_t *app) {
  if (app == NULL)
    return NULL;
  group_el_init_t children[] = {{.type = w_button, .label = "Anonymous"},
                                {.type = w_button, .label = "User"},
                                {.type = w_button, .label = "Register"},
                                {.type = w_end}};

  dialogue_t *d = init_dialogue("Login",
                                "Select your login "
                                "option");

  memcpy(&(app->modal.dialogue), d, sizeof(dialogue_t));
  free(d);
  app->modal.dialogue.ch_group =
      init_group(&(app->modal.dialogue.win), &(d->w), children, horizontal);
  group_bt_t *ch_btns = (group_bt_t *)app->modal.dialogue.ch_group->elements;
  ch_btns[0].element->is_hovered = 1;

  return NULL;
}