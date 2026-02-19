#ifndef ALERT_H
#define ALERT_H
#include "app.h"
void init_alert(app_t *app);
void alert(char *message);
void notification(const char *title, const char *message,
                  enum d_color_scheme color);
#endif