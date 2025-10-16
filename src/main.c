#include "main.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

/* TODO: перед загрузкой файла спрашивать права доступа
 TODO: сделать при интерфес выбора по пунктам меню */

int main(int argc, char *argv[]) {
  params_t params;
  init_params(&params);
  analyze_args(argc, argv, &params);
  get_missing_params(&params);
  int sd = init_client();
  connect_to_server(sd, &params);
  query_loop(sd, &params);
  clear_params(&params);
  return 0;
}