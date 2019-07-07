#include "axisum.h"

#ifndef _BUFFER_SIZE
  #define _BUFFER_SIZE 32768
#endif

void checksum_print(const struct checksum_record *v){
  if(v!=NULL){
    if(v->hash!=NULL){
      for(int i=0; i<5; i++){
        fprintf(stdout,"%02x%02x%02x%02x",
          (uint8_t)(v->hash[i]>>24),
          (uint8_t)(v->hash[i]>>16),
          (uint8_t)(v->hash[i]>>8),
          (uint8_t)(v->hash[i]));
      };
      if(v->filename!=NULL) fprintf(stdout," %s",v->filename);
      fprintf(stdout,"\n");
    };
  };
}

uint32_t axisum(scxh *hash, const char *filename, checksum_operator f){
  if((hash==NULL)&&(hash->state==NULL)) return 0x00000001;
  uint32_t errcode=0;
#ifdef _WIN32
  wchar_t *wc_buf=NULL;
#endif
  FILE *i_stream=NULL;
  char *ibuffer=(char *)calloc(_BUFFER_SIZE,sizeof(char));
  if(ibuffer==NULL) errcode=0x00000001;
  else{
    if(filename){
#ifndef _WIN32
      i_stream=fopen64(filename,"rb");
#else
      wc_buf=codepagetowchar(filename,CP_UTF8);
      if(wc_buf){
        i_stream=_wfopen(wc_buf,L"rb");
        free(wc_buf);
      }
      else errcode=0x00000001;
#endif
    }
    else i_stream=fdopen(fileno(stdin),"rb");
    if(i_stream==NULL) errcode=0x00000040;
    if(errcode==0){
      setvbuf(i_stream,ibuffer,_IOFBF,_BUFFER_SIZE);
      char *content=(char *)calloc(_BUFFER_SIZE,sizeof(char));
      if(content){
        int lenght=0;
        int filedsc=fileno(i_stream);
        char *t_buf=content;
        int t_len=0;
        int t_size=_BUFFER_SIZE;
        while(1){
          lenght=read(filedsc,t_buf,t_size);
#ifdef EINTR
          if(lenght<0 && errno=EINTR) continue;
#endif
          if(lenght<0){
            errcode=0x00000040;
            break;
          };
          t_len+=lenght;
          if((t_len<_BUFFER_SIZE)&&(lenght!=0)){
            t_buf+=lenght;
            t_size-=lenght;
            continue;
          };
          if(t_len){
            (*hash)<<(uint32_t)t_len;
            (*hash)<<(unsigned char *)content;
          };
          t_len=0;
          t_size=_BUFFER_SIZE;
          t_buf=content;
          if(lenght==0) break;
        };
        if(errcode==0){
          if(f!=NULL){
            struct checksum_record v;
            v.hash=hash->hash;
            v.filename=(char *)filename;
            (*f)(&v);
          };
          memset(content,0,_BUFFER_SIZE);
          free(content);
        };
        hash->clear();
      }
      else errcode=0x00000001;
    };
    if(i_stream) fclose(i_stream);
    memset(ibuffer,0,_BUFFER_SIZE);
    free(ibuffer);
  };
  return errcode;
}

uint32_t axisum(alfl *alfl_lag, const char *filename, checksum_operator f){
  uint32_t errcode=0;
  alfl *lags;
  if(alfl_lag!=NULL) lags=alfl_lag;
  else lags=new alfl;
  if(lags){
    alfg *state=new alfg(lags);
    if(state){
      if(state->lags){
        scxh *hash=new scxh(state);
        if(hash){
          errcode=axisum(hash,filename,f);
          delete hash;
          hash=NULL;
        }
        else errcode=0x00000001;
      }
      else errcode=0x00000001;
      delete state;
      state=NULL;
    }
    else errcode=0x00000001;
    if(alfl_lag==NULL) delete lags;
    else lags=NULL;
  }
  else errcode=0x00000001;
  return errcode;
}
