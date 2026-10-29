#!/bin/sh
printf 'Content-Type: text/plain; charset=utf-8\r\n\r\n'
printf 'CGI works!\nMethod: %s\nQuery: %s\nRemote: %s\n' "$REQUEST_METHOD" "$QUERY_STRING" "$REMOTE_ADDR"
if [ "$REQUEST_METHOD" = POST ]; then
  printf 'POST data: '
  head -c "${CONTENT_LENGTH:-0}"
  printf '\n'
fi
