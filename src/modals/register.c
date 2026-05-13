#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <widget.h>
#include <widget_core.h>

#include "alert.h"
#include "group.h"
#include "../server.h"

typedef struct {
  w_button_t *element;
  enum w_type type;
} group_bt_t;

void init_register_modal_cb(w_cb_args_t *args) {
  w_cb_args_t d_args;
  app_t *app = args->app;
  w_dialogue_t *d = (w_dialogue_t *)app->active_widget;
  memcpy(&d_args, args, sizeof(w_cb_args_t));
  /* d_args.app = NULL; */
  d_args.element = app->active_widget;
  w_dialogue_callback_default(&d_args);
  if (d_args.resp_data.code == cbrp_val &&
      d_args.resp_data.val.type == val_num) {

    if (d_args.resp_data.val.val.num == 2) {
      app->query_args->state = S_ASK_LOGIN_TYPE;
      app->modal.needs_destroy = true;
      return;
    }

    w_input_t *in_name = d->g_content->elements[0].element;
    w_group_t *g_passwords = d->g_content->elements[1].element;
    w_input_t *in_email = d->g_content->elements[2].element;
    w_input_t *in_pass = g_passwords->elements[0].element;
    w_input_t *in_pass_r = g_passwords->elements[1].element;
    if (in_name->value_len == 0) {
      w_alert("Name field is empty!");
      return;
    }
    if (in_email->value_len == 0) {
      w_alert("Email field is empty!");
      return;
    }
    if (!in_pass->value_len || !in_pass->value_len) {
      w_alert("One of password's field is empty!");
      return;
    }
    if (strcmp(in_pass->value, in_pass_r->value)) {
      w_alert("Your passwords do not match!");
      in_pass->value[0] = '\0';
      in_pass->value_len = 0;
      in_pass_r->value[0] = '\0';
      in_pass_r->value_len = 0;
      return;
    }
    app->params->uname = malloc(in_name->value_len + 1);
    app->params->pass = malloc(in_pass->value_len + 1);
    strncpy(app->params->uname, in_name->value, in_name->value_len);
    app->params->uname[in_name->value_len] = 0;
    strncpy(app->params->pass, in_pass->value, in_pass->value_len);
    app->params->pass[in_pass->value_len] = 0;
    server_send_string(app->query_args, "register %s %s %*s\n", app->params->uname,
            app->params->pass, (int)in_email->value_len, in_email->value
            );
    app->query_args->state = S_WAIT_REGISTER_CONFIRMATION;
  }
}

w_dialogue_t *m_register_init(app_t *app) {
  if (app == NULL)
    return NULL;

  w_group_el_init_t content_pass[] = {{.type = w_input,
                                     .label = "Password",
                                     .length = 18,
                                     .is_hidden_value = true},
                                    {.type = w_input,
                                     .label = "Password (repeat)",
                                     .length = 18,
                                     .is_hidden_value = true},
                                    {.type = w_end}};

  w_group_el_init_t content[] = {
      {.type = w_input, .label = "Username", .length = 18},
      {.type = w_group, .direction = horizontal, .children = content_pass},
      {.type = w_input, .label = "Email", .length = 40},
      {.type = w_end}};

  w_group_el_init_t actions[] = {
      {.type = w_button, .label = "Register", .is_default = true, .val.num = 1},
      {.type = w_button, .label = "Cancel", .is_default = false, .val.num = 2},
      {.type = w_end}};

  w_dialogue_init(&(app->modal), "Credentials", "There is information needed",
                &(app->coordinates));
  w_dialogue_t *d = &(app->modal);

  d->w.callback = init_register_modal_cb;
  d->g_content = w_group_init(&(d->win), &(d->w), content, &(d->id_map), vertical,
                            g_content);
  d->g_action = w_group_init(&(d->win), &(d->w), actions, &(d->id_map),
                           horizontal, g_action);

  w_dialogue_init_active_id(d);

  return NULL;
}