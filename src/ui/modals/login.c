#include "../widget/dialogue.h"
#include "../widget/group.h"
#include <string.h>

typedef struct {
  button_t *element;
  enum w_type type;
} group_bt_t;

/* TODO: improve init process (make it more readable) */

dialogue_t *init_login_modal() {
  group_el_t children[] = {{.type = w_button},
                           {.type = w_button},
                           {.type = w_button},
                           {.type = w_end}};

  dialogue_t *d = init_dialogue("Login",
                                "Select your login "
                                "option",
                                50, 20);
  d->ch_group = init_group(children);
  group_bt_t *ch_btns = (group_bt_t *)d->ch_group->elements;
  strcpy(ch_btns[0].element->w.title, "Anonymous");
  strcpy(ch_btns[1].element->w.title, "User");
  strcpy(ch_btns[2].element->w.title, "Register");
  return d;
}