/*
 * repl.c - REPL loop
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif
#include <curl/curl.h>
#include "repl.h"
#include "command.h"
#include "protocol.h"

#define PROMPT    "curl_repl> "
#define INPUT_MAX 1024

static void
print_prompt(void)
{
  printf(PROMPT);
  fflush(stdout);
}

static int
read_line(char *buf, int bufsize)
{
  if(!fgets(buf, bufsize, stdin))
    return -1;
  buf[strcspn(buf, "\n")] = '\0';
  return 0;
}

void
repl_run(void)
{
  char buf[INPUT_MAX];
  fd_set readfds;
  struct timeval tv;
  curl_socket_t sock;
  const struct protocol *proto;
  int nfds;
  int ret;

  print_prompt();

  for(;;) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    nfds = STDIN_FILENO + 1;

    sock = CURL_SOCKET_BAD;
    proto = protocol_active();
    if(proto) {
      sock = proto->get_socket_fn();
      if(sock != CURL_SOCKET_BAD) {
        FD_SET(sock, &readfds);
        if((int)sock >= nfds)
          nfds = (int)sock + 1;
      }
    }

    tv.tv_sec = 0;
    tv.tv_usec = 100000; /* 100 ms */

    ret = select(nfds, &readfds, NULL, NULL, &tv);
    if(ret < 0) {
      /* EINTR from SIGINT (Ctrl+C) — the signal handler set quit flag */
      if(errno == EINTR)
        continue;
      perror("select");
      if(proto)
        proto->disconnect_fn();
      break;
    }

    if(sock != CURL_SOCKET_BAD && FD_ISSET(sock, &readfds)) {
      proto->recv_fn();
      if(!FD_ISSET(STDIN_FILENO, &readfds))
        print_prompt();
    }
    if(FD_ISSET(STDIN_FILENO, &readfds)) {
      if(read_line(buf, INPUT_MAX) < 0)
        break;
      command_dispatch(buf);
      if(command_should_quit())
        break;
      print_prompt();
    }
  }

  /* Clean up any active protocol handle before exit */
  proto = protocol_active();
  if(proto)
    proto->disconnect_fn();

  printf("\n");
}
