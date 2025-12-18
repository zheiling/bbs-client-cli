#include "client.h"
#include "main.h"
#include "ui.h"
#include "ui/widget/file_list.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

void ask_uname_and_password(params_t *params);
void ask_register(params_t *params, char *email);

#define PRINT_SRV_MESSAGE(q_args)                                              \
  if (q_args->server_message.size > 0) {                                       \
    q_args->state = S_PRINT_SERVER_MESSAGE;                                    \
    q_args->next_server_command = malloc(l_len + 1);                           \
    strcpy(q_args->next_server_command, line);                                 \
    return 0;                                                                  \
  }

int process_server_command(char *line, int l_len, query_args_t *q_args) {
  int ws_pos = l_len;
  params_t *params = q_args->params;
  const char *login_options[] = {"Username", "Anonymous", "Register", NULL};
  uint32_t answer;
  uint64_t new_capacity;

  char *cptr = strchr(line, ' ');
  if (cptr != NULL && cptr > line)
    ws_pos = cptr - line;

  /* LOGIN */
  if (!strncmp(line, "login>", ws_pos)) {
    if (q_args->server_message.size > 0) {
      PRINT_SRV_MESSAGE(q_args);
    }
    if (q_args->params->uname != NULL && q_args->params->pass != NULL) {
      write(params->sd, q_args->params->uname, strlen(q_args->params->uname));
    } else {
      q_args->state = S_ASK_LOGIN_TYPE;
    }
    return 0;
  }

  /* PASSWORD */
  if (!strncmp(line, "password>", ws_pos)) {
    if (q_args->params->pass == NULL) {
      return 1;
    }
    write(q_args->sd, params->pass, strlen(params->pass));
    return 0;
  }

  /* LOGIN AGAIN */
  if (!strncmp(line, "login_again>", ws_pos)) {
    free(params->uname);
    free(params->pass);
    params->uname = NULL;
    params->pass = NULL;
    // get_missing_params(params);
    write(q_args->sd, params->uname, strlen(params->uname));
    return 0;
  }

  /* REGISTER CONFIRMATION */
  if (q_args->state == WAIT_REGISTER_CONFIRMATION) {
    if (!strcmp(line, "ok")) {
      write(q_args->sd, "file list", sizeof("file list"));
      q_args->state = S_FILE_LIST;
      return 0;
    } else {
      write(STDOUT_FILENO, line, strlen(line));
      q_args->state = WAIT_REGISTER;
      return 1;
    }
  }

  /* DOWNLOAD START */
  if (!strncmp(line, "download_start", ws_pos)) {
    return 0;
  }

  /* WELCOME MES */
  if (!strncmp(line, "Welcome, ", ws_pos)) {
    write(q_args->sd, "file list", sizeof("file list"));
    q_args->state = S_FILE_LIST;
    return 0;
  }

  if (q_args->state == S_WAIT_SERVER) {
    if (q_args->server_message.text == NULL) {
      q_args->server_message.text = malloc(l_len + 1);
      memcpy(q_args->server_message.text, line, l_len);
      q_args->server_message.text[l_len] = 0;
      q_args->server_message.size = l_len + 1;
      q_args->server_message.capacity = l_len + 1;
    } else {
      if (q_args->server_message.size + l_len >
          q_args->server_message.capacity) {
        new_capacity = (l_len + q_args->server_message.capacity) * 2;
        q_args->server_message.text =
            realloc(q_args->server_message.text, new_capacity);
        q_args->server_message.capacity = new_capacity;
      }
      q_args->server_message.size += l_len;
      strncat(q_args->server_message.text, line, l_len);
      q_args->server_message.text[q_args->server_message.size + l_len] = 0;
    }
  } else {
    char buf[INBUFSIZE + sizeof("server: ")] = "server: ";
    strcat(buf, line);
    write(STDOUT_FILENO, buf, l_len + sizeof("server: "));
    if (q_args->state == WAIT_CLIENT) {
      print_prompt(q_args->params);
    }
  }

  return 0;
}

static void file_receive(int sd, char *line) {
  char fname[128];
  char command[32];
  size_t fsize;
  sscanf(line, "%s %s %zu", command, fname, &fsize);
  char buf[INBUFSIZE];
  int rlen;

  int file_d = open(fname, O_WRONLY);

  if (file_d == -1) {
    char err_mes[256];
    int mlen;
    mlen = sprintf(err_mes, "Can't write file named \"%s\"\n", fname);
    write(sd, err_mes, mlen);
    close(file_d);
    return;
  }

  while (fsize && (rlen = read(sd, buf, INBUFSIZE))) {
    write(file_d, buf, rlen);
    fsize -= rlen;
  }

  close(file_d);
}

void ask_uname_and_password(params_t *params) {
  size_t lsize;
  char *bufptr = NULL;

  struct termios ts_hide, ts_show;

  if (isatty(0)) {
    tcgetattr(0, &ts_show);
    memcpy(&ts_hide, &ts_show, sizeof(ts_show));
    ts_hide.c_lflag &= ~ECHO;
  }

  if (params->uname == NULL) {
    printf("login> ");
    fflush(stdout);
    getline(&(params->uname), &lsize, stdin);
    lsize = strlen(params->uname);
    params->uname[lsize - 1] = '\0'; /* get rid of '\n' */
  }

  if (params->pass == NULL) {
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_hide);
    }
    printf("password> ");
    getline(&(params->pass), &lsize, stdin);
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_show);
      putchar('\n');
    }
    lsize = strlen(params->pass);
    params->pass[lsize - 1] = '\0'; /* get rid of '\n' */
    free(bufptr);
    bufptr = NULL;
  }
}

void ask_register(params_t *params, char *email) {
  size_t lsize;
  char *bufptr = NULL;

  struct termios ts_hide, ts_show;

  if (isatty(0)) {
    tcgetattr(0, &ts_show);
    memcpy(&ts_hide, &ts_show, sizeof(ts_show));
    ts_hide.c_lflag &= ~ECHO;
  }

  if (params->uname == NULL) {
    printf("username> ");
    fflush(stdout);
    getline(&(params->uname), &lsize, stdin);
    lsize = strlen(params->uname);
    params->uname[lsize - 1] = '\0'; /* get rid of '\n' */
    free(bufptr);
    bufptr = NULL;
  }

  if (params->pass == NULL) {
    // hides input letters
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_hide);
    }
    do {
      if (params->pass != NULL) {
        printf("\nERROR: password do not match! Try again!\n");
        free(params->pass);
        params->pass = NULL;
      }
      if (bufptr != NULL) {
        free(bufptr);
        bufptr = NULL;
      }
      printf("password> ");
      getline(&(params->pass), &lsize, stdin);
      printf("\npassword (repeat)> ");
      getline(&(bufptr), &lsize, stdin);
    } while (strcmp(bufptr, params->pass));

    // shows input letters again
    if (isatty(0)) {
      tcsetattr(0, TCSANOW, &ts_show);
      putchar('\n');
    }

    printf("email> ");
    fflush(stdout);
    scanf("%s", email);

    lsize = strlen(params->pass);
    params->pass[lsize - 1] = '\0'; /* get rid of '\n' */
    free(bufptr);
    bufptr = NULL;
  }
}