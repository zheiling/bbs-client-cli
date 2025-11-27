#include "ui/app.h"
#include <stdint.h>

int32_t m_login(char evt_c, app_t *app);

int32_t modal_event_loop(char evt_c, app_t *app) {
  switch (app->modal.active) {

  case none_active:
    return 0;
  case login:
    m_login(evt_c, app);
    break;
  }
  return 1;
}

int32_t m_login(char evt_c, app_t *app) {
    
}