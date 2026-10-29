#ifndef SWS_H
#define SWS_H
#include <stddef.h>
#define HEADER_MAX 16384
#define BODY_MAX (1024 * 1024)
#define BUF 8192
extern int debug_mode, server_port;
extern char root[4096];
int sendall(int fd, const void *data, size_t len);
int headers(int fd, int status, const char *type, long long size, const char *extra);
void error_response(int fd, int status, int head);
int safe_path(const char *uri);
void static_file(int fd, const char *target, int head);
void cgi_response(int fd, const char *script, const char *method, const char *query,
                  const char *body, size_t bodylen, const char *ctype, int head,
                  const char *clientip);
void handle_client(int fd, const char *ip);
#endif
