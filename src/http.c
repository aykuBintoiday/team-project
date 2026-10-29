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

int sendall(int fd,const void *data,size_t len){const char*p=data;while(len){ssize_t n=send(fd,p,len,0);if(n<0&&errno==EINTR)continue;if(n<=0)return -1;p+=n;len-=(size_t)n;}return 0;}
static const char *reason(int status){switch(status){case 200:return "OK";case 302:return "Found";case 400:return "Bad Request";case 403:return "Forbidden";case 404:return "Not Found";case 405:return "Method Not Allowed";case 408:return "Request Timeout";case 411:return "Length Required";case 413:return "Content Too Large";case 414:return "URI Too Long";case 431:return "Request Header Fields Too Large";case 500:return "Internal Server Error";case 501:return "Not Implemented";case 502:return "Bad Gateway";default:return "Error";}}
int headers(int fd,int status,const char*type,long long size,const char*extra){char b[2048];int n=snprintf(b,sizeof b,"HTTP/1.0 %d %s\r\nServer: sws/1.0\r\nConnection: close\r\nContent-Type: %s\r\n",status,reason(status),type?type:"text/plain; charset=utf-8");if(size>=0)n+=snprintf(b+n,sizeof b-(size_t)n,"Content-Length: %lld\r\n",size);if(extra&&*extra)n+=snprintf(b+n,sizeof b-(size_t)n,"%s",extra);n+=snprintf(b+n,sizeof b-(size_t)n,"\r\n");return sendall(fd,b,(size_t)n);}
void error_response(int fd,int status,int head){char b[256];int n=snprintf(b,sizeof b,"%d %s\n",status,reason(status));headers(fd,status,"text/plain; charset=utf-8",n,status==405?"Allow: GET, HEAD, POST\r\n":"");if(!head)sendall(fd,b,(size_t)n);}
void handle_client(int fd,const char *ip){struct timeval timeout={10,0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof timeout);char request[HEADER_MAX+1];size_t got=0,he=0;while(got<HEADER_MAX){ssize_t n=recv(fd,request+got,HEADER_MAX-got,0);if(n<=0){if(got)error_response(fd,408,0);return;}got+=(size_t)n;request[got]=0;char*p=strstr(request,"\r\n\r\n");if(p){he=(size_t)(p-request)+4;break;}p=strstr(request,"\n\n");if(p){he=(size_t)(p-request)+2;break;}}if(!he){error_response(fd,431,0);return;}char *end=strchr(request,'\n');if(!end){error_response(fd,400,0);return;}*end=0;if(end>request&&end[-1]=='\r')end[-1]=0;char method[16],uri[4096],ver[32],extra[2];if(sscanf(request,"%15s %4095s %31s %1s",method,uri,ver,extra)!=3){error_response(fd,400,0);return;}int head=!strcmp(method,"HEAD");if(strcmp(ver,"HTTP/1.0")&&strcmp(ver,"HTTP/1.1")){error_response(fd,400,head);return;}if(strcmp(method,"GET")&&strcmp(method,"HEAD")&&strcmp(method,"POST")){error_response(fd,405,head);return;}if(strlen(uri)>=4000){error_response(fd,414,head);return;}char *q=strchr(uri,'?');if(q)*q++=0;else q="";if(!*uri||!safe_path(uri)){error_response(fd,403,head);return;}long long clen=0;char ct[256]="";int seen_cl=0;char*line=end+1;while(line<request+he){char*next=strchr(line,'\n');if(!next)break;*next=0;if(next>line&&next[-1]=='\r')next[-1]=0;if(!strncasecmp(line,"Content-Length:",15)){if(seen_cl++){error_response(fd,400,head);return;}char *val=line+15;while(*val==' ')val++;char *e;clen=strtoll(val,&e,10);if(e==val||(*e&&*e!=' ')||clen<0){error_response(fd,400,head);return;}}else if(!strncasecmp(line,"Content-Type:",13)){snprintf(ct,sizeof ct,"%s",line+13);while(ct[0]==' ')memmove(ct,ct+1,strlen(ct));}line=next+1;}
if(!strcmp(method,"POST")&&!seen_cl){error_response(fd,411,head);return;}if(clen>BODY_MAX){error_response(fd,413,head);return;}char target[8192];if(snprintf(target,sizeof target,"%s%s%s",root,uri,(strlen(uri)>0&&uri[strlen(uri)-1]=='/')?"index.html":"")>=(int)sizeof target){error_response(fd,414,head);return;}
/* Resolve symlinks and ensure every served resource remains within docroot. */
char resolved[8192];if(!realpath(target,resolved)){error_response(fd,404,head);return;}size_t rl=strlen(root);if(strncmp(resolved,root,rl)||(resolved[rl]!='/'&&resolved[rl]!='\0')){error_response(fd,403,head);return;}
int iscgi=!strncmp(uri,"/cgi-bin/",9);if(!iscgi&&strcmp(method,"POST")==0){error_response(fd,405,head);return;}if(!iscgi){static_file(fd,resolved,head);return;}struct stat st;if(stat(resolved,&st)||!S_ISREG(st.st_mode)||access(resolved,X_OK)){error_response(fd,403,head);return;}
size_t have=got-he;char *body=malloc((size_t)clen+1);if(!body){error_response(fd,500,head);return;}size_t used=have<(size_t)clen?have:(size_t)clen;memcpy(body,request+he,used);while(used<(size_t)clen){ssize_t n=recv(fd,body+used,(size_t)clen-used,0);if(n<=0){free(body);error_response(fd,400,head);return;}used+=(size_t)n;}cgi_response(fd,resolved,method,q,body,(size_t)clen,ct,head,ip);free(body);}
