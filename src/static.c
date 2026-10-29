#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


#include <sys/time.h>
#include "sws.h"

static const char*mime(const char*name){const char*e=strrchr(name,'.');if(!e)return "application/octet-stream";if(!strcasecmp(e,".html")||!strcasecmp(e,".htm"))return "text/html; charset=utf-8";if(!strcasecmp(e,".css"))return "text/css; charset=utf-8";if(!strcasecmp(e,".js"))return "application/javascript";if(!strcasecmp(e,".txt"))return "text/plain; charset=utf-8";if(!strcasecmp(e,".json"))return "application/json";if(!strcasecmp(e,".png"))return "image/png";if(!strcasecmp(e,".jpg")||!strcasecmp(e,".jpeg"))return "image/jpeg";if(!strcasecmp(e,".gif"))return "image/gif";if(!strcasecmp(e,".svg"))return "image/svg+xml";if(!strcasecmp(e,".pdf"))return "application/pdf";return "application/octet-stream";}
int safe_path(const char *s){if(*s!='/')return 0;for(const char*p=s;*p;p++)if((p==s||p[-1]=='/')&&p[0]=='.'&&p[1]=='.'&&(p[2]=='/'||p[2]=='\0'))return 0;return !strchr(s,'\\')&&!strchr(s,'\r')&&!strchr(s,'\n');}
void static_file(int fd,const char *target,int head){struct stat st;if(stat(target,&st)<0){error_response(fd,errno==EACCES?403:404,head);return;}if(!S_ISREG(st.st_mode)){error_response(fd,403,head);return;}int in=open(target,O_RDONLY|O_NONBLOCK);if(in<0){error_response(fd,403,head);return;}headers(fd,200,mime(target),(long long)st.st_size,NULL);if(!head){char b[BUF];ssize_t n;while((n=read(in,b,sizeof b))>0)if(sendall(fd,b,(size_t)n)<0)break;}close(in);}
