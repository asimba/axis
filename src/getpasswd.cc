#ifndef _WIN32
  #include <sys/mman.h>
#endif
#include "getpasswd.h"
#include "etc.h"
#include "vars.h"

char *getpasswd(){
  char *ps=(char *)calloc(256,sizeof(char));
  int c;
  if(ps){
#ifndef _WIN32
    if(uid){
      if(mlock(ps,256)<0){
        free(ps);
        ps=NULL;
        return NULL;
      };
    };
    struct termios orig_state,new_state;
    if(tcgetattr(fileno(stdin),&orig_state)==-1){
      free(ps);
      return NULL;
    };
    new_state=orig_state;
    new_state.c_lflag&=~(ICANON|ECHO);
    if(tcsetattr(fileno(stdin),TCSAFLUSH,&new_state)==-1){
      free(ps);
      return NULL;
    };
#endif
    for(uint16_t i=0; i<256;){
#ifdef _WIN32
      c=getch();
#else
      c=getchar();
#endif
      if((c==10)||(c==13)) break;
      if((c==8)||(c==127)){
        if(i>0) i--;
        ps[i]=0;
        continue;
      };
      if((c==0)||(c==EOF)){
        memset(ps,0,strlen(ps));
        free(ps);
        c=0;
        ps=NULL;
        break;
      };
      ps[i]=(char)c;
      i++;
    };
    c=0;
#ifndef _WIN32
    if(tcsetattr(fileno(stdin),TCSANOW,&orig_state)==-1){
      memset(ps,0,strlen(ps));
      free(ps);
      ps=NULL;
      return NULL;
    };
#else
    char *tmp=codepagetocodepage(ps,CP_OEMCP,CP_UTF8);
    memset(ps,0,strlen(ps));
    free(ps);
    ps=tmp;
#endif
  };
  return ps;
}
