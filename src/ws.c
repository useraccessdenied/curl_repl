/*
 * ws.c - WebSocket protocol handler
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <string.h>
#include "protocol.h"
#include "ws.h"

struct ws_state {
  CURL          *handle;
  curl_socket_t  sock;
};

static struct ws_state g_ws = {NULL, CURL_SOCKET_BAD};

static int
cmd_ws_send(const char *args)
{
  return ws_send(args);
}

static int
cmd_ws_ping(const char *args)
{
  (void)args;
  return ws_ping();
}

static int
cmd_ws_disconnect(const char *args)
{
  (void)args;
  ws_disconnect();
  return 0;
}

const struct command ws_commands[] = {
  {"send",       cmd_ws_send,       "send <msg>   - send a text frame"},
  {"ping",       cmd_ws_ping,       "ping         - send a ping frame"},
  {"disconnect", cmd_ws_disconnect, "disconnect   - close the connection"},
  {NULL, NULL, NULL}
};

int
ws_connect(const char *url)
{
  CURLcode res;

  if(g_ws.handle) {
    fprintf(stderr, "already connected; disconnect first\n");
    return -1;
  }

  g_ws.handle = curl_easy_init();
  if(!g_ws.handle) {
    fprintf(stderr, "curl_easy_init failed\n");
    return -1;
  }

  curl_easy_setopt(g_ws.handle, CURLOPT_URL, url);
  /* 2L means connect and perform the WebSocket upgrade handshake.
   * 1L would stop at the TCP connection without upgrading. */
  curl_easy_setopt(g_ws.handle, CURLOPT_CONNECT_ONLY, 2L);

  res = curl_easy_perform(g_ws.handle);
  if(res != CURLE_OK) {
    fprintf(stderr, "connect failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(g_ws.handle);
    g_ws.handle = NULL;
    g_ws.sock = CURL_SOCKET_BAD;
    return -1;
  }

  res = curl_easy_getinfo(g_ws.handle, CURLINFO_ACTIVESOCKET, &g_ws.sock);
  if(res != CURLE_OK || g_ws.sock == CURL_SOCKET_BAD) {
    fprintf(stderr, "could not get active socket\n");
    curl_easy_cleanup(g_ws.handle);
    g_ws.handle = NULL;
    g_ws.sock = CURL_SOCKET_BAD;
    return -1;
  }

  printf("connected to %s\n", url);
  return 0;
}

int
ws_send(const char *message)
{
  CURLcode res;
  size_t sent;

  if(!g_ws.handle) {
    fprintf(stderr, "not connected\n");
    return -1;
  }

  res = curl_ws_send(g_ws.handle, message, strlen(message), &sent,
                     0, CURLWS_TEXT);
  if(res != CURLE_OK) {
    fprintf(stderr, "send failed: %s\n", curl_easy_strerror(res));
    return -1;
  }
  return 0;
}

int
ws_ping(void)
{
  CURLcode res;
  size_t sent;

  if(!g_ws.handle) {
    fprintf(stderr, "not connected\n");
    return -1;
  }

  res = curl_ws_send(g_ws.handle, "", 0, &sent, 0, CURLWS_PING);
  if(res != CURLE_OK) {
    fprintf(stderr, "ping failed: %s\n", curl_easy_strerror(res));
    return -1;
  }
  printf("ping sent\n");
  return 0;
}

void
ws_disconnect(void)
{
  size_t sent;

  if(!g_ws.handle) {
    printf("not connected\n");
    return;
  }

  /* best-effort close frame; ignore errors */
  curl_ws_send(g_ws.handle, "", 0, &sent, 0, CURLWS_CLOSE);
  curl_easy_cleanup(g_ws.handle);
  g_ws.handle = NULL;
  g_ws.sock = CURL_SOCKET_BAD;
  protocol_set_active(NULL);
  printf("disconnected\n");
}

curl_socket_t
ws_get_socket(void)
{
  return g_ws.sock;
}

int
ws_recv_and_print(void)
{
  char buf[4096];
  size_t recvd;
  const struct curl_ws_frame *meta;
  CURLcode res;

  if(!g_ws.handle)
    return -1;

  res = curl_ws_recv(g_ws.handle, buf, sizeof(buf) - 1, &recvd, &meta);
  if(res == CURLE_AGAIN)
    return 0;

  if(res != CURLE_OK) {
    fprintf(stderr, "\nws recv error: %s\n", curl_easy_strerror(res));
    ws_disconnect();
    return -1;
  }

  buf[recvd] = '\0';

  if(meta->flags & CURLWS_CLOSE) {
    printf("\n[server closed connection]\n");
    ws_disconnect();
    return -1;
  }
  if(meta->flags & CURLWS_PING) {
    printf("\n[ping from server]\n");
    return 0;
  }
  if(meta->flags & CURLWS_PONG) {
    printf("\n[pong received]\n");
    return 0;
  }

  if(meta->flags & CURLWS_BINARY) {
    printf("\n< [binary frame, %zu bytes]\n", recvd);
    return 0;
  }

  printf("\n< %s\n", buf);
  return 0;
}
