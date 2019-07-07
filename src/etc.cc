#include "etc.h"

#ifdef _WIN32

wchar_t *codepagetowchar(const char *src, UINT cp){
  wchar_t *wc_buf=NULL;
  if(src){
    int ln=MultiByteToWideChar(cp,MB_ERR_INVALID_CHARS,src,-1,NULL,0);
    if(ln){
      wc_buf=(wchar_t *)calloc(ln,sizeof(wchar_t));
      if(wc_buf){
        if(!MultiByteToWideChar(cp,0,src,-1,wc_buf,ln)){
          wmemset(wc_buf,0,wcslen(wc_buf));
          free(wc_buf);
          wc_buf=NULL;
        }
      }
    }
  }
  return wc_buf;
}

char *wchartocodepage(const wchar_t *src, UINT cp){
  char *c_buf=NULL;
  if(src){
    int ln=WideCharToMultiByte(cp,0,src,-1,NULL,0,NULL,NULL);
    if(ln){
      c_buf=(char *)calloc(ln,sizeof(char));
      if(c_buf){
        if(!WideCharToMultiByte(cp,0,src,-1,c_buf,ln,NULL,NULL)){
          memset(c_buf,0,strlen(c_buf));
          free(c_buf);
          c_buf=NULL;
        }
      }
    }
  }
  return c_buf;
}

char *codepagetocodepage(const char *src, UINT src_cp, UINT dst_cp){
  char *c_buf=NULL;
  if(src){
    wchar_t *wc_buf=NULL;
    int ln=MultiByteToWideChar(src_cp,MB_ERR_INVALID_CHARS,src,-1,NULL,0);
    if(ln){
      wc_buf=(wchar_t *)calloc(ln,sizeof(wchar_t));
      if(wc_buf){
        if(MultiByteToWideChar(src_cp,0,src,-1,wc_buf,ln))
          c_buf=wchartocodepage(wc_buf,dst_cp);
        wmemset(wc_buf,0,wcslen(wc_buf));
        free(wc_buf);
        wc_buf=NULL;
      }
    }
  }
  return c_buf;
}

#endif

void clrline(uint32_t w){
  if(w){
    char l[w];
    memset(l,' ',w-1);
    l[w-1]=0;
    fprintf(stdout,"\r%s\r",l);
    fflush(stdout);
  }
}

void version(FILE *stream, const char *command_name, const char *version, const char *authors){
  fprintf(stream, "\n%s %s\nWritten by %s.\n\n\
This is free software. There is NO warranty; not even for MERCHANTABILITY or \
FITNESS FOR A PARTICULAR PURPOSE.\n\
WARNING: This program can be used ONLY for protection of personal information; \
it CANNOT be used for protection of confidential information (personally \
identifiable information etc.) or any other data.\n\n",
command_name,version,authors);
}

void error(const char *err_message){
  fprintf(stderr,"\nError: %s!\n",err_message);
  fflush(stderr);
}
