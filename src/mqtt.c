/*
 * mqtt.c - MQTT protocol handler
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
#include "mqtt.h"

struct mqtt_state {
  CURL          *handle;   /* persistent handle for subscribe/recv */
  curl_socket_t  sock;
  char          *url;      /* stored for publish */
};

static struct mqtt_state g_mqtt = {NULL, CURL_SOCKET_BAD, NULL};

static int
cmd_mqtt_publish(const char *args)
{
  return mqtt_publish(args);
}

static int
cmd_mqtt_disconnect(const char *args)
{
  (void)args;
  mqtt_disconnect();
  return 0;
}

const struct command mqtt_commands[] = {
  {"publish",    cmd_mqtt_publish,    "publish <msg> - publish to the topic"},
  {"disconnect", cmd_mqtt_disconnect, "disconnect    - close the connection"},
  {NULL, NULL, NULL}
};

int
mqtt_connect(const char *url)
{
  CURLcode res;

  if(g_mqtt.handle) {
    fprintf(stderr, "already connected; disconnect first\n");
    return -1;
  }

  g_mqtt.handle = curl_easy_init();
  if(!g_mqtt.handle) {
    fprintf(stderr, "curl_easy_init failed\n");
    return -1;
  }

  curl_easy_setopt(g_mqtt.handle, CURLOPT_URL, url);
  /* 2L = connect + MQTT CONNECT + SUBSCRIBE handshake */
  curl_easy_setopt(g_mqtt.handle, CURLOPT_CONNECT_ONLY, 2L);

  res = curl_easy_perform(g_mqtt.handle);
  if(res != CURLE_OK) {
    fprintf(stderr, "connect failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(g_mqtt.handle);
    g_mqtt.handle = NULL;
    g_mqtt.sock = CURL_SOCKET_BAD;
    return -1;
  }

  res = curl_easy_getinfo(g_mqtt.handle, CURLINFO_ACTIVESOCKET,
                          &g_mqtt.sock);
  if(res != CURLE_OK || g_mqtt.sock == CURL_SOCKET_BAD) {
    fprintf(stderr, "could not get active socket\n");
    curl_easy_cleanup(g_mqtt.handle);
    g_mqtt.handle = NULL;
    g_mqtt.sock = CURL_SOCKET_BAD;
    return -1;
  }

  g_mqtt.url = strdup(url);
  if(!g_mqtt.url) {
    fprintf(stderr, "strdup failed\n");
    curl_easy_cleanup(g_mqtt.handle);
    g_mqtt.handle = NULL;
    g_mqtt.sock = CURL_SOCKET_BAD;
    return -1;
  }

  printf("connected to %s\n", url);
  return 0;
}

int
mqtt_publish(const char *message)
{
  CURL *pub;
  CURLcode res;

  if(!g_mqtt.url) {
    fprintf(stderr, "not connected\n");
    return -1;
  }

  pub = curl_easy_init();
  if(!pub) {
    fprintf(stderr, "curl_easy_init failed\n");
    return -1;
  }

  curl_easy_setopt(pub, CURLOPT_URL, g_mqtt.url);
  curl_easy_setopt(pub, CURLOPT_POSTFIELDS, message);

  res = curl_easy_perform(pub);
  curl_easy_cleanup(pub);

  if(res != CURLE_OK) {
    fprintf(stderr, "publish failed: %s\n", curl_easy_strerror(res));
    return -1;
  }
  return 0;
}

void
mqtt_disconnect(void)
{
  if(!g_mqtt.handle) {
    printf("not connected\n");
    return;
  }

  curl_easy_cleanup(g_mqtt.handle);
  g_mqtt.handle = NULL;
  g_mqtt.sock = CURL_SOCKET_BAD;
  free(g_mqtt.url);
  g_mqtt.url = NULL;
  protocol_set_active(NULL);
  printf("disconnected\n");
}

curl_socket_t
mqtt_get_socket(void)
{
  return g_mqtt.sock;
}

int
mqtt_recv_and_print(void)
{
  char buf[4096];
  size_t nread;
  CURLcode res;

  if(!g_mqtt.handle)
    return -1;

  res = curl_easy_recv(g_mqtt.handle, buf, sizeof(buf) - 1, &nread);
  if(res == CURLE_AGAIN)
    return 0;

  if(res != CURLE_OK) {
    fprintf(stderr, "\nmqtt recv error: %s\n", curl_easy_strerror(res));
    mqtt_disconnect();
    return -1;
  }

  if(nread == 0) {
    printf("\n[server closed connection]\n");
    mqtt_disconnect();
    return -1;
  }

  buf[nread] = '\0';
  printf("\n< %s\n", buf);
  return 0;
}
