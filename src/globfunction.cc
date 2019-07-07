#include "globfunction.h"
#include <stdlib.h>
#include <stdio.h>

glob64_t *glob_getlist(const char *mask, int &errcode){
  if(errcode!=0) return NULL;
  glob64_t *globbuf;
  globbuf=(glob64_t *)calloc(1,sizeof(glob64_t));
  if(globbuf){
#ifndef _WIN32
    globbuf->gl_offs=0;
    errno=0;
    if(glob64(mask,GLOB_ERR,NULL,globbuf)!=0){
      if(errno==GLOB_NOSPACE) errcode=0x00000001;
      if((errno==GLOB_NOMATCH)||(globbuf->gl_pathc==0)) errcode=0x00000040;
      globfree64(globbuf);
      free(globbuf);
      globbuf=NULL;
    };
#else
    struct _wfinddatai64_t fileinfo;
    int handle;
    int rc;
    wchar_t *wmask=codepagetowchar(mask,CP_ACP);
    if(wmask==NULL){
      errcode=0x00000001;
      free(globbuf);
      globbuf=NULL;
    };
    errno=0;
    handle=_wfindfirsti64(wmask,&fileinfo);
    if(handle==-1){
      if((errno==ENOENT)||(errno==EINVAL)){
        errcode=0x00000040;
        free(globbuf);
        globbuf=NULL;
        wmemset(wmask,(wchar_t)0,wcslen(wmask));
        free(wmask);
        wmask=NULL;
      };
    };
    rc=handle;
    while(rc!=-1){
      globbuf->gl_pathv=(char **)realloc(globbuf->gl_pathv,
        (globbuf->gl_pathc+1)*sizeof(char*));
      if(globbuf->gl_pathv==NULL){
        errcode=0x00000001;
        globbuf->gl_pathc=0;
        free(globbuf);
        globbuf=NULL;
        break;
      };
      globbuf->gl_pathv[globbuf->gl_pathc]=wchartocodepage(fileinfo.name,CP_OEMCP);
      if(globbuf->gl_pathv[globbuf->gl_pathc]==NULL){
        errcode=0x00000001;
        for(uint32_t i=0;i<globbuf->gl_pathc;i++){
          free(globbuf->gl_pathv[i]);
          globbuf->gl_pathv[i]=NULL;
        };
        globbuf->gl_pathc=0;
        free(globbuf);
        globbuf=NULL;
        break;
      };
      globbuf->gl_pathc++;
      rc=_wfindnexti64(handle,&fileinfo);
    };
    if(handle!=-1) _findclose(handle);
    if(wmask){
      wmemset(wmask,(wchar_t)0,wcslen(wmask));
      free(wmask);
      wmask=NULL;
    };
#endif
  }
  else errcode=0x00000001;
  return globbuf;
}

void glob_freelist(glob64_t *globbuf){
  if(globbuf){
#ifndef _WIN32
    globfree64(globbuf);
#else
    for(uint32_t i=0;i<globbuf->gl_pathc;i++){
      memset(globbuf->gl_pathv[i],0,strlen(globbuf->gl_pathv[i]));
      free(globbuf->gl_pathv[i]);
      globbuf->gl_pathv[i]=NULL;
    };
    globbuf->gl_pathc=0;
#endif
    free(globbuf);
    globbuf=NULL;
  };
}
