#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L
#include "sws.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CGI_OUTPUT_LIMIT (BODY_MAX + HEADER_MAX)
#define CGI_TIMEOUT 10
static int full_write(int fd,const void *buf,size_t len){
  const char *p=buf;
  while(len){ssize_t n=write(fd,p,len);if(n<0&&errno==EINTR)continue;
    if(n<=0)return -1;
    p+=n;len-=(size_t)n;}
  return 0;
}
void cgi_response(int fd,const char *script,const char *method,const char *query,
                  const char *body,size_t bodylen,const char *ctype,int head,const char *clientip){
  char tmp[]="/tmp/sws-cgi-XXXXXX";
  int input=mkstemp(tmp);
  if(input<0){error_response(fd,500,head);return;}
  unlink(tmp);
  if(full_write(input,body,bodylen)<0||lseek(input,0,SEEK_SET)<0){close(input);error_response(fd,500,head);return;}
  int output[2];
  if(pipe(output)<0){close(input);error_response(fd,500,head);return;}
  pid_t pid=fork();
  if(pid<0){close(input);close(output[0]);close(output[1]);error_response(fd,500,head);return;}
  if(pid==0){
    close(output[0]);
    if(dup2(input,STDIN_FILENO)<0||dup2(output[1],STDOUT_FILENO)<0)_exit(127);
    close(input);close(output[1]);
    char port[16],length[32];
    snprintf(port,sizeof(port),"%d",server_port);
    snprintf(length,sizeof(length),"%zu",bodylen);
    setenv("GATEWAY_INTERFACE","CGI/1.1",1);
    setenv("SERVER_SOFTWARE","sws/1.0",1);
    setenv("SERVER_PROTOCOL","HTTP/1.0",1);
    setenv("SERVER_PORT",port,1);
    setenv("SERVER_NAME","localhost",1);
    setenv("REQUEST_METHOD",method,1);
    setenv("QUERY_STRING",query,1);
    setenv("SCRIPT_NAME",script+strlen(root),1);
    setenv("REMOTE_ADDR",clientip,1);
    setenv("CONTENT_LENGTH",length,1);
    setenv("CONTENT_TYPE",ctype?ctype:"",1);
    execl(script,script,(char*)NULL);
    _exit(127);
  }
  close(input);close(output[1]);
  size_t cap=BUF,used=0;
  char *data=malloc(cap);
  if(!data){close(output[0]);kill(pid,SIGKILL);waitpid(pid,NULL,0);error_response(fd,500,head);return;}
  int failure=0;
  for(;;){
    fd_set fds;FD_ZERO(&fds);FD_SET(output[0],&fds);
    struct timeval tv={CGI_TIMEOUT,0};
    int ready=select(output[0]+1,&fds,NULL,NULL,&tv);
    if(ready<0&&errno==EINTR)continue;
    if(ready<=0){failure=1;break;}
    char buf[BUF];ssize_t n=read(output[0],buf,sizeof(buf));
    if(n<0&&errno==EINTR)continue;
    if(n<0){failure=1;break;}
    if(n==0)break;
    if(used+(size_t)n>CGI_OUTPUT_LIMIT){failure=1;break;}
    if(used+(size_t)n>cap){size_t newcap=cap*2;
      while(newcap<used+(size_t)n)newcap*=2;
      char *p=realloc(data,newcap);
      if(!p){failure=1;break;}
      data=p;cap=newcap;
    }
    memcpy(data+used,buf,(size_t)n);used+=(size_t)n;
  }
  close(output[0]);
  if(failure)kill(pid,SIGKILL);
  int status=0;while(waitpid(pid,&status,0)<0&&errno==EINTR){}
  if(failure||!WIFEXITED(status)||WEXITSTATUS(status)!=0){free(data);error_response(fd,502,head);return;}
  size_t boundary=0,seplen=0;
  for(size_t i=0;i+1<used;i++){
    if(i+3<used&&!memcmp(data+i,"\r\n\r\n",4)){boundary=i;seplen=4;break;}
    if(!memcmp(data+i,"\n\n",2)){boundary=i;seplen=2;break;}
  }
  if(!seplen){free(data);error_response(fd,502,head);return;}
  char *raw=malloc(boundary+1);
  if(!raw){free(data);error_response(fd,500,head);return;}
  memcpy(raw,data,boundary);raw[boundary]=0;
  int code=200;char type[256]="text/plain; charset=utf-8";
  char extra[1024]="";char *save=NULL;
  for(char *line=strtok_r(raw,"\r\n",&save);line;line=strtok_r(NULL,"\r\n",&save)){
    while(*line==' '||*line=='\t')line++;
    if(!strncasecmp(line,"Status:",7)){
      int c=atoi(line+7);if(c>=100&&c<=599)code=c;
    }else if(!strncasecmp(line,"Content-Type:",13)){
      snprintf(type,sizeof(type),"%s",line+13);
      while(type[0]==' '||type[0]=='\t')memmove(type,type+1,strlen(type));
      /* Prevent response-splitting by CGI-provided content type. */
      if(strpbrk(type,"\r\n"))strcpy(type,"text/plain");
    }else if(!strncasecmp(line,"Location:",9)){
      const char *loc=line+9;while(*loc==' '||*loc=='\t')loc++;
      if(!strpbrk(loc,"\r\n")){snprintf(extra,sizeof(extra),"Location: %.900s\r\n",loc);if(code==200)code=302;}
    }
  }
  size_t len=used-boundary-seplen;
  headers(fd,code,type,(long long)len,extra);
  if(!head)sendall(fd,data+boundary+seplen,len);
  free(raw);free(data);
}
