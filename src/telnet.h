/*
 * telnet.h - Telnet protocol handler interface
 *
 * Copyright (C) 2026 Kapil Singaria
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef CURL_REPL_TELNET_H
#define CURL_REPL_TELNET_H

#include <curl/curl.h>
#include "command.h"

int           telnet_connect(const char *url);
int           telnet_send(const char *message);
void          telnet_disconnect(void);
curl_socket_t telnet_get_socket(void);
int           telnet_recv_and_print(void);

extern const struct command telnet_commands[];

#endif /* CURL_REPL_TELNET_H */
