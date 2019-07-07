#include "mdirs.h"
#include "etc.h"
#ifdef _WIN32
  #include <wchar.h>
#endif

uint32_t mdirs(const char *fname, struct infonode &info, bool ipt){
  uint32_t errcode=0;
  if((strcmp(fname,"./.")==0)||(strcmp(fname,"./..")==0)||
    (strcmp(fname,".")==0)) return 0;
  char *tmf=strdup(fname);
  if(tmf==NULL) return 0x00000001;
  char *src=strdup(fname);
#ifdef _WIN32
  wchar_t *wc_buf=NULL;
#endif
  if(src==NULL){
    memset(tmf,0,strlen(tmf));
    free(tmf);
    tmf=NULL;
    return 0x00000001;
  };
  char *dst=strdup(fname);
  if(dst==NULL){
    memset(tmf,0,strlen(tmf));
    free(tmf);
    tmf=NULL;
    memset(src,0,strlen(src));
    free(src);
    src=NULL;
    return 0x00000001;
  };
  dst[0]=0;
  char *pnt=NULL;
  while((pnt=strchr(src,'/'))){
    strcpy(tmf,pnt+1);
    if(strchr(tmf,'/')) *(strchr(tmf,'/'))=0;
    if(dst[0]){
      dst[strlen(dst)+1]=0;
      dst[strlen(dst)]='/';
    };
    strcat(dst,tmf);
#ifdef _WIN32
    wc_buf=codepagetowchar(dst,CP_UTF8);
    if(wc_buf==NULL){
      errcode=0x00000001;
      break;
    };
#endif
#ifndef _WIN32
    if(access(dst,F_OK)!=0){
      if(mkdir(dst,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)==-1)
#else
    if(_waccess(wc_buf,F_OK)!=0){
      if(_wmkdir(wc_buf)==-1)
#endif
        {
          errcode=0x00000f00;
          break;
        };
    };
    strcpy(tmf,pnt+1);
    strcpy(src,tmf);
  };
  if(tmf){
    memset(tmf,0,strlen(tmf));
    free(tmf);
    tmf=NULL;
  };
  if(src){
    memset(src,0,strlen(src));
    free(src);
    src=NULL;
  };
  if(dst){
    memset(dst,0,strlen(dst));
    free(dst);
    dst=NULL;
  };
#ifdef _WIN32
  if(wc_buf){
    wmemset(wc_buf,0,wcslen(wc_buf));
    free(wc_buf);
    wc_buf=NULL;
  };
#endif
  return errcode;
}
