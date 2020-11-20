#include "nvoc.h"

/***********************************************************************************************************/
//Базовый класс циклической очереди поискового словаря алгоритма сжатия.
/***********************************************************************************************************/

nvoc::nvoc(){
  offset=0;
  lenght=0;
  vocroot=0;
  voclast=0xffff;
  vocarea=(struct vocnode *)calloc(0x10000,sizeof(struct vocnode));
  if(vocarea){
    vocindx=(struct vocpntr *)calloc(0x10000,sizeof(struct vocpntr));
    if(vocindx==NULL) free(vocarea);
    else for(uint32_t i=0;i<0x10000;i++) vocarea[i].relative=vocindx[i].in=vocindx[i].out=-1;
  };
}

nvoc::~nvoc(){
  if(vocindx){
    memset(vocarea,0,0x10000*sizeof(struct vocnode));
    free(vocarea);
    vocarea=NULL;
    memset(vocindx,0,0x10000*sizeof(struct vocpntr));
    free(vocindx);
    vocindx=NULL;
    vocroot=0;
    voclast=0;
    offset=0;
    lenght=0;
  };
}

void nvoc::search(uint8_t *str,uint16_t size){
  offset=lenght=0;
  if(size<3) return;
  int32_t cnode=vocindx[*(uint16_t*)str].in;
  while(cnode>=0){
    if(str[lenght]==vocarea[(uint16_t)(cnode+lenght)].value){
      uint16_t tnode=cnode+2;
      uint16_t cl=2;
      while((cl<size)&&(str[cl]==vocarea[tnode++].value)) cl++;
      if(cl>lenght){
        lenght=cl;
        offset=cnode;
      };
      if(cl==size) break;
    }
    cnode=vocarea[cnode].relative;
  };
  if(lenght>2){
    if(offset<vocroot) offset=(uint32_t)offset+0x10000-vocroot;
    else offset-=vocroot;
  };
}

void nvoc::write(uint8_t *str,uint16_t size){
  vocpntr *indx;
  while(size--){
    union {uint8_t c[sizeof(uint16_t)];uint16_t i16;} u;
    u.c[0]=vocarea[vocroot].value;
    u.c[1]=vocarea[(uint16_t)(vocroot+1)].value;
    vocindx[u.i16].in=vocarea[vocroot].relative;
    vocarea[vocroot].relative=-1;
    vocarea[vocroot].value=*str++;
    u.c[0]=vocarea[voclast].value;
    u.c[1]=vocarea[vocroot].value;
    indx=&vocindx[u.i16];
    if(indx->in>=0) vocarea[indx->out].relative=voclast;
    else indx->in=voclast;
    indx->out=voclast;
    voclast++;
    vocroot++;
  };
}

void nvoc::write_woupdate(uint8_t *str,uint16_t size){
  while(size--) vocarea[vocroot++].value=*str++;
}

void nvoc::read(uint8_t *str,uint16_t index,uint16_t size){
  index+=vocroot;
  while(size--) *str++=vocarea[index++].value;
}

/***********************************************************************************************************/
