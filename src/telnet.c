/*
 * telnet.c - Telnet protocol handler
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "protocol.h"
#include "telnet.h"

struct telnet_state {
  CURL          *handle;
  curl_socket_t  sock;
};

static struct telnet_state g_telnet = {NULL, CURL_SOCKET_BAD};

static int
cmd_telnet_send(const char *args)
{
  return telnet_send(args);
}

static int
cmd_telnet_disconnect(const char *args)
{
  (void)args;
  telnet_disconnect();
  return 0;
}

const struct command telnet_commands[] = {
  {"send",       cmd_telnet_send,       "send <msg>   - send text to server"},
  {"disconnect", cmd_telnet_disconnect, "disconnect   - close the connection"},
  {NULL, NULL, NULL}
};

int
telnet_connect(const char *url)
{
  CURLcode res;

  if(g_telnet.handle) {
    fprintf(stderr, "already connected; disconnect first\n");
    return -1;
  }

  g_telnet.handle = curl_easy_init();
  if(!g_telnet.handle) {
    fprintf(stderr, "curl_easy_init failed\n");
    return -1;
  }

  curl_easy_setopt(g_telnet.handle, CURLOPT_URL, url);
  /* 1L = TCP connect only; no protocol upgrade */
  curl_easy_setopt(g_telnet.handle, CURLOPT_CONNECT_ONLY, 1L);

  res = curl_easy_perform(g_telnet.handle);
  if(res != CURLE_OK) {
    fprintf(stderr, "connect failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(g_telnet.handle);
    g_telnet.handle = NULL;
    g_telnet.sock = CURL_SOCKET_BAD;
    return -1;
  }

  res = curl_easy_getinfo(g_telnet.handle, CURLINFO_ACTIVESOCKET,
                          &g_telnet.sock);
  if(res != CURLE_OK || g_telnet.sock == CURL_SOCKET_BAD) {
    fprintf(stderr, "could not get active socket\n");
    curl_easy_cleanup(g_telnet.handle);
    g_telnet.handle = NULL;
    g_telnet.sock = CURL_SOCKET_BAD;
    return -1;
  }

  printf("connected to %s\n", url);
  return 0;
}

int
telnet_send(const char *message)
{
  CURLcode res;
  size_t sent;
  size_t len;
  char *buf;

  if(!g_telnet.handle) {
    fprintf(stderr, "not connected\n");
    return -1;
  }

  len = strlen(message);
  /* append telnet line terminator \r\n */
  buf = malloc(len + 3);
  if(!buf) {
    fprintf(stderr, "malloc failed\n");
    return -1;
  }
  memcpy(buf, message, len);
  buf[len]     = '\r';
  buf[len + 1] = '\n';

  res = curl_easy_send(g_telnet.handle, buf, len + 2, &sent);
  free(buf);

  if(res != CURLE_OK) {
    fprintf(stderr, "send failed: %s\n", curl_easy_strerror(res));
    return -1;
  }
  return 0;
}

void
telnet_disconnect(void)
{
  if(!g_telnet.handle) {
    printf("not connected\n");
    return;
  }

  curl_easy_cleanup(g_telnet.handle);
  g_telnet.handle = NULL;
  g_telnet.sock = CURL_SOCKET_BAD;
  protocol_set_active(NULL);
  printf("disconnected\n");
}

curl_socket_t
telnet_get_socket(void)
{
  return g_telnet.sock;
}

int
telnet_recv_and_print(void)
{
  char buf[4096];
  size_t nread;
  CURLcode res;

  if(!g_telnet.handle)
    return -1;

  res = curl_easy_recv(g_telnet.handle, buf, sizeof(buf) - 1, &nread);
  if(res == CURLE_AGAIN)
    return 0;

  if(res != CURLE_OK) {
    fprintf(stderr, "\ntelnet recv error: %s\n", curl_easy_strerror(res));
    telnet_disconnect();
    return -1;
  }

  if(nread == 0) {
    printf("\n[server closed connection]\n");
    telnet_disconnect();
    return -1;
  }

  buf[nread] = '\0';
  printf("\n%s", buf);
  return 0;
}
