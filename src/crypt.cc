#include "crypt.h"

/***********************************************************************************************************/
//Реализация алгоритма Simple Stream Cipher "Styx"(SX)
/***********************************************************************************************************/
#include <stdio.h>
cryptxs::cryptxs(){ //основной конструктор класса
  slag=new alfl;
  cycles=NULL;
  if(slag){
#ifndef _WIN32
    if(uid){
      if(mlock(cpassword,20)<0){
        delete slag;
        slag=NULL;
        return;
      };
    };
#endif
    pm=new permutation(cpassword,20);
    if(pm){
      for(unsigned char c=0; c<6; c++){
        state[c]=new alfg(slag);
        if(state[c])
          if(state[c]->lags) continue;
        delete pm;
        pm=NULL;
        delete slag;
        slag=NULL;
        break;
      };
    }
    else{
      delete slag;
      slag=NULL;
    };
  };
}

void cryptxs::set(char *p, bool decode){ //функция инициализации параметров операции
  char *cp=(char *)cpassword;
  scxh *hash;
#ifndef _WIN32
  pthread_t th_id[5];
  pthread_attr_t th_attr;
#else
  HANDLE th_id[5];
#endif
  th_alfg_helper th_in[5];
  if(cycles){
    for(uint8_t i=0; i<6; i++) state[i]->cycles=cycles;
  };
  hash=new scxh(state[0]);
  if(hash){
    if(!(hash->state)){
      delete hash;
      hash=NULL;
      delete slag;
      slag=NULL;
      return;
    };
  }
  else{
    delete slag;
    slag=NULL;
    return;
  };
  operation=decode;
#ifndef _WIN32
  if(uid){
    if(mlock(hash->hash,sizeof(uint32_t)*5)<0){
      delete hash;
      hash=NULL;
      delete slag;
      slag=NULL;
      return;
    };
  };
#endif
  (*hash)<<strlen(p);
  (*hash)<<(unsigned char*)p;
#ifndef _WIN32
  if(pthread_attr_init(&th_attr)){
    if(uid) munlock(hash->hash,sizeof(uint32_t)*5);
    delete hash;
    hash=NULL;
    delete slag;
    slag=NULL;
    return;
  }
  else{
    pthread_attr_setdetachstate(&th_attr,PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&th_attr,1024);
  };
#endif
  for(uint8_t i=0; i<5; i++){
    long2char(&cp,hash->hash[i]);
    th_in[i].state=state[i];
    th_in[i].v=hash->hash[i];
#ifndef _WIN32
    if(pthread_create(&th_id[i],&th_attr,
                      th_alfg_init,(void *)&(th_in[i]))){
      pthread_attr_destroy(&th_attr);
      if(uid) munlock(hash->hash,sizeof(uint32_t)*5);
      delete hash;
      hash=NULL;
      delete slag;
      slag=NULL;
      return;
    };
#else
    th_alfg_helper *th_in_tmp=&(th_in[i]);
    th_id[i]=CreateThread(NULL,0,
                          (LPTHREAD_START_ROUTINE)th_alfg_init,
                          (LPVOID *)th_in_tmp,0,NULL);
    if(th_id[i]==NULL){
      delete hash;
      hash=NULL;
      delete slag;
      slag=NULL;
      return;
    };
#endif
  };
  for(uint8_t i=0; i<5; i++){
#ifndef _WIN32
    pthread_join(th_id[i],NULL);
#else
    WaitForSingleObjectEx(th_id[i],INFINITE,false);
    CloseHandle(th_id[i]);
#endif
  };
  (*state[5])=state[0];
#ifndef _WIN32
  if(uid) munlock(hash->hash,sizeof(uint32_t)*5);
#endif
  delete hash;
  lenght=0;
  for(uint8_t c=0; c<6; c++){
    state[c]->cycles=NULL;
  };
  cycles=NULL;
}

inline void cryptxs::noise(unsigned char *i){ //функция наложения элементарного "авто-шума"
  uint8_t c=i[0];
  uint32_t s;
  (*state[5])>>s;
  i[0]^=s;
  s=((*state[5])<<c);
  i[1]^=(s>>24);
  i[2]^=(s>>16);
  i[3]^=(s>>8);
}

inline void cryptxs::denoise(unsigned char *i){ //функция устранения элементарного "авто-шума"
  uint32_t s;
  (*state[5])>>s;
  i[0]^=s;
  s=((*state[5])<<i[0]);
  i[1]^=(s>>24);
  i[2]^=(s>>16);
  i[3]^=(s>>8);
}

inline void cryptxs::transcoder(unsigned char *i){ //основная функция обработки 32х-разрядного блока
  uint32_t l=0;
  for(unsigned char c=0; c<20; c++){
    l^=((*state[c>>2])<<cpassword[c]);
    if (c<16) (*state[(c>>2)+1])<<(uint8_t)(l^(l>>8)^(l>>16)^(l>>24));
  };
  pm->next();
  l^=0;
  i[0]^=(l>>24);
  i[1]^=(l>>16);
  i[2]^=(l>>8);
  i[3]^=l;
}

void cryptxs::buffer_transcoder(unsigned char *i){ //базовый цикл обработки содержимого буфера
  uint32_t bl=lenght/4;
  if(lenght%4) bl++;
  if(operation){
    while(bl){
      transcoder(i);
      denoise(i);
      i+=4;
      bl--;
    };
  }
  else{
    while(bl){
      noise(i);
      transcoder(i);
      i+=4;
      bl--;
    };
  };
}

cryptxs::~cryptxs(){ //деструктор класса
  operation=false;
  lenght=6;
  if(pm){
    delete pm;
    pm=NULL;
  };
  memset(cpassword,0,20);
  while(lenght){
    lenght--;
    if(state[lenght]){
      delete state[lenght];
      state[lenght]=NULL;
    };
  };
  if(slag){
#ifndef _WIN32
    if(uid) munlock(cpassword,20);
#endif
    delete slag;
    slag=NULL;
  };
}

/***********************************************************************************************************/
