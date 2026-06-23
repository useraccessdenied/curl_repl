/*
 * mqtt.h - MQTT protocol handler interface
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef CURL_REPL_MQTT_H
#define CURL_REPL_MQTT_H

#include <curl/curl.h>
#include "command.h"

int           mqtt_connect(const char *url);
int           mqtt_publish(const char *message);
void          mqtt_disconnect(void);
curl_socket_t mqtt_get_socket(void);
int           mqtt_recv_and_print(void);

extern const struct command mqtt_commands[];

#endif /* CURL_REPL_MQTT_H */
