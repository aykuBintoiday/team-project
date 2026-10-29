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

int debug_mode=0, server_port=8080;
char root[4096];
static void logmsg(const char *fmt,...){if(!debug_mode)return;va_list a;va_start(a,fmt);vfprintf(stderr,fmt,a);fputc('\n',stderr);va_end(a);}

static void reap(int sig){(void)sig;while(waitpid(-1,NULL,WNOHANG)>0){}}
static void daemonize(void){pid_t pid=fork();if(pid<0)exit(1);if(pid>0)exit(0);if(setsid()<0)exit(1);pid=fork();if(pid<0)exit(1);if(pid>0)exit(0);umask(027);int dev=open("/dev/null",O_RDWR);if(dev>=0){dup2(dev,0);dup2(dev,1);dup2(dev,2);if(dev>2)close(dev);}}
int main(int argc,char**argv){const char *directory="www";int opt;while((opt=getopt(argc,argv,"dp:r:h"))!=-1){switch(opt){case 'd':debug_mode=1;break;case 'p':server_port=atoi(optarg);break;case 'r':directory=optarg;break;default:fprintf(stderr,"Usage: %s [-d] [-p port] [-r docroot]\n",argv[0]);return opt=='h'?0:1;}}if(server_port<1||server_port>65535){fprintf(stderr,"Invalid port\n");return 1;}if(!realpath(directory,root)){perror("docroot");return 1;}signal(SIGPIPE,SIG_IGN);struct sigaction sa;memset(&sa,0,sizeof sa);sa.sa_handler=reap;sigemptyset(&sa.sa_mask);sa.sa_flags=SA_RESTART;sigaction(SIGCHLD,&sa,NULL);int s=socket(AF_INET,SOCK_STREAM,0);if(s<0){perror("socket");return 1;}int yes=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes);struct sockaddr_in addr;memset(&addr,0,sizeof addr);addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_ANY);addr.sin_port=htons((unsigned short)server_port);if(bind(s,(struct sockaddr*)&addr,sizeof addr)||listen(s,128)){perror("bind/listen");close(s);return 1;}if(!debug_mode)daemonize();logmsg("sws serving %s on port %d",root,server_port);for(;;){struct sockaddr_in peer;socklen_t len=sizeof peer;int fd=accept(s,(struct sockaddr*)&peer,&len);if(fd<0){if(errno==EINTR)continue;continue;}pid_t pid=fork();if(pid==0){close(s);char ip[INET_ADDRSTRLEN];inet_ntop(AF_INET,&peer.sin_addr,ip,sizeof ip);handle_client(fd,ip);close(fd);_exit(0);}close(fd);if(pid<0)logmsg("fork failed");}return 0;}
