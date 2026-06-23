# curl_repl — Roadmap

## Milestone 1: WebSocket ✅

Interactive WebSocket client using libcurl's `CURLOPT_CONNECT_ONLY=2` mode.
Commands: `connect`, `send`, `ping`, `disconnect`, `help`, `quit`/`exit`.
Incoming messages are displayed via a `select()` loop that watches both
stdin and the active WebSocket socket simultaneously, so the connection
stays alive and messages arrive in real time without polling.

## Milestone 2: Telnet ✅

libcurl supports TELNET URLs (`telnet://host:port`). Add `telnet.c` with
`connect`, `send`, and `disconnect` commands. Use `CURLOPT_TELNETOPTIONS`
for negotiation options and read incoming data via the same `select()`
pattern used by the WebSocket handler.

## Milestone 3: SFTP

libcurl supports SFTP (`sftp://user@host/path`). Commands: `ls`, `get`,
`put`, `rm`, `mkdir`. Unlike WebSocket and Telnet, SFTP is request-response,
so each command performs a full `curl_easy_perform()` call and no persistent
socket select loop is needed.

## Milestone 4: MQTT ✅

libcurl supports MQTT (`mqtt://host/topic`). Commands: `subscribe`,
`publish`, `disconnect`. The `subscribe` command uses the `select()` pattern
from the WebSocket milestone to display incoming messages in real time.

## Milestone 5: Script file execution ✅

Non-interactive mode: `curl_repl -f script.txt` reads a file and executes
each line as a command in order, then exits. Lines starting with `#` are
treated as comments and skipped. Blank lines are ignored. Errors print to
stderr and abort execution by default; a `-k` / `--keep-going` flag can
optionally continue past errors. The same `command_dispatch` path is reused,
so script files use exactly the same syntax as the interactive REPL.
