#include "canonicalize.h"

bool canonicalize(char *src){
  if(src==NULL) return false;
  char *dst=strdup(src);
  if(dst==NULL) return false;
  char *pnt=NULL;
  while((pnt=strstr(src,"/../"))){
    dst[pnt-src]=0;
    strcat(dst,pnt+3);
    strcpy(src,dst);
  };
  while((pnt=strstr(src,"/./"))){
    dst[pnt-src]=0;
    strcat(dst,pnt+2);
    strcpy(src,dst);
  };
  while((pnt=strstr(src,"//"))){
    dst[pnt-src]=0;
    strcat(dst,pnt+1);
    strcpy(src,dst);
  };
  memset(dst,0,strlen(dst));
  free(dst);
  dst=NULL;
  return true;
}
