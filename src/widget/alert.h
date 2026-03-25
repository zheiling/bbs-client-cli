#ifndef ALERT_H
#define ALERT_H
#include "app.h"
#include <stdarg.h>
void init_alert(app_t *app);
void alert(const char *message);
void notification(const char *title, enum dc_color_scheme color,
                   const char *f_message, ...);
#endif