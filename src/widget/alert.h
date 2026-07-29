#ifndef W_ALERT_H
#define W_ALERT_H

#include <common.h>
#include <stdarg.h>
#include <stdbool.h>

void w_alert_init(app_t *app);
void w_alert(const char *message);
void w_notification(const char *title, enum w_dialogue_color_scheme color,
                    const char *f_message, ...);
int  w_bool_ask(const char *title,  const char *yes_button,
               const char *no_button, bool yes_default, const char *f_message, ...);

#endif