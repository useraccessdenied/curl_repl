/*
 * protocol.c - protocol registry and active protocol tracking
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <string.h>
#include "protocol.h"
#include "mqtt.h"
#include "telnet.h"
#include "ws.h"

static const struct protocol PROTOCOLS[] = {
  {"ws",  "websocket", ws_commands,
   ws_connect, ws_disconnect, ws_get_socket, ws_recv_and_print},
  {"wss", "websocket", ws_commands,
   ws_connect, ws_disconnect, ws_get_socket, ws_recv_and_print},
  {"telnet", "telnet", telnet_commands,
   telnet_connect, telnet_disconnect, telnet_get_socket,
   telnet_recv_and_print},
  {"mqtt",  "mqtt",  mqtt_commands,
   mqtt_connect, mqtt_disconnect, mqtt_get_socket, mqtt_recv_and_print},
  {"mqtts", "mqtts", mqtt_commands,
   mqtt_connect, mqtt_disconnect, mqtt_get_socket, mqtt_recv_and_print},
  {NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

static const struct protocol *g_active = NULL;

const struct protocol *
protocol_find(const char *scheme)
{
  int i;
  for(i = 0; PROTOCOLS[i].scheme; i++) {
    if(strcmp(PROTOCOLS[i].scheme, scheme) == 0)
      return &PROTOCOLS[i];
  }
  return NULL;
}

const struct protocol *
protocol_active(void)
{
  return g_active;
}

void
protocol_set_active(const struct protocol *p)
{
  g_active = p;
}
