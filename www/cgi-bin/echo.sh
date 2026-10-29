#!/bin/sh
printf "Content-Type: text/plain; charset=utf-8\r\n\r\n"
printf "method=%s\nquery=%s\n" "$REQUEST_METHOD" "$QUERY_STRING"
if [ "$REQUEST_METHOD" = POST ]; then cat; fi
