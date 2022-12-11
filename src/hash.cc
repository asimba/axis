#include "hash.h"

/***********************************************************************************************************/
//Реализация алгоритма Simple Stream Cipher "Styx" Hash (SCXH)
/***********************************************************************************************************/

void scxh::reset(bool use_hash_tbl){
  if(state){
#ifndef HAVE_HASH_TBL
    use_hash_tbl=false;
#endif
    if(use_hash_tbl){
#ifdef HAVE_HASH_TBL
      uint32_t *ht=state->get_buf();
      for(uint32_t i=0; i<L_LAG; i++) ht[i]=sdx_hash_tbl[i];
      state->set_index(sdx_hash_tbl[L_LAG]);
#else
      (*state)=L_LAG*L_LAG;
#endif
    }
    else (*state)=L_LAG*L_LAG;
    length=5;
    while(length--) hash[length]=0;
    init=true;
  };
}

bool scxh::self_check(){
#ifndef HAVE_HASH_TBL
  return false;
#else
  if(state){
    unsigned char chash[4];
    uint32_t i=0;
    for(uint32_t i=0; i<L_LAG+1; i++){
      long2char_s(chash,sdx_hash_tbl[i]);
      (*this)<<(uint32_t)sizeof(uint32_t);
      (*this)<<(unsigned char *)chash;
    };
    for(i=L_LAG+1; i<(L_LAG+6); i++){
      if(hash[i-L_LAG-1]!=sdx_hash_tbl[i]) return false;
    };
  };
  return true;
#endif
}

scxh::scxh(alfg *alfgh){ //конструктор с возможностью указания внешней дополнительной памяти
  state=NULL;
  if(alfgh){
    if(alfgh->lags){
      state=alfgh;
#ifndef HAVE_HASH_TBL
      reset(false);
#else
      reset(true);
      reset(self_check());
#endif
    };
  };
}

void scxh::clear(){
  if(state){
#ifndef HAVE_HASH_TBL
    reset(false);
#else
    reset(true);
#endif
  };
}

scxh::~scxh(){ //деструктор класса (обнуление и высвобождение памяти)
  length=0;
  init=true;
  memset(hash,0,5*sizeof(uint32_t));
  state=NULL;
}

void scxh::scxh_update(unsigned char *i){ //обработка блока информации
  if(length){
    uint32_t bl, l;
    if(state){
      bl=length/4;
      l=length%4;
      unsigned char c[]={0,0,0};
      uint8_t t=0;
      if(l) for(l=bl*4; l<length; l++) c[t++]=i[l];
      if(init){
        (*state)<<i[0];
        init=false;
      };
      while(bl){
        hash[0]^=((*state)>>l);
        hash[0]+=hash[3];
        hash[1]^=((*state)<<i[0]);
        hash[1]+=hash[4];
        hash[2]^=((*state)<<i[1]);
        hash[2]+=hash[0];
        hash[3]^=((*state)<<i[2]);
        hash[3]+=hash[1];
        hash[4]^=((*state)<<i[3]);
        hash[4]+=hash[2];
        i+=4;
        bl--;
      };
      if(t){
        hash[0]^=((*state)>>l);
        hash[0]+=hash[3];
        hash[1]^=((*state)<<c[0]);
        hash[1]+=hash[4];
        hash[2]^=((*state)<<c[1]);
        hash[2]+=hash[0];
        hash[3]^=((*state)<<c[2]);
        hash[3]+=hash[1];
        hash[4]^=((*state)++);
        hash[4]+=hash[2];
      };
      length=0;
    };
  };
}

/***********************************************************************************************************/
