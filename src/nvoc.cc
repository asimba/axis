#include "nvoc.h"

/***********************************************************************************************************/
//Базовый класс циклической очереди поискового словаря алгоритма сжатия.
/***********************************************************************************************************/

nvoc::nvoc(uint32_t size){
  vocroot=NULL;
  vocstop=NULL;
  vocindx=NULL;
  vocsize=0;
  offset=0;
  lenght=0;
  if(size){
    vocarea=(struct vocnode *)calloc(size,sizeof(struct vocnode));
    if(vocarea){
      vocindx=(struct vocpntr *)calloc(65536,sizeof(struct vocpntr));
      if(vocindx==NULL){
        free(vocarea);
        vocarea=NULL;
      }
      else{
        for(vocsize=0;vocsize<size-1;vocsize++){
          vocarea[vocsize].next=&vocarea[vocsize+1];
          vocarea[vocsize].index=vocsize;
        };
        vocarea[vocsize].index=vocsize;
        vocroot=vocarea;
        vocstop=vocarea;
        voclast=NULL;
        vocsize++;
      };
    };
  };
}

nvoc::~nvoc(){
  if(vocindx){
    memset(vocarea,0,sizeof(struct vocnode)*vocsize);
    free(vocarea);
    vocarea=NULL;
    memset(vocindx,0,sizeof(struct vocpntr)*65536);
    free(vocindx);
    vocindx=NULL;
  };
  vocsize=0;
  offset=0;
  lenght=0;
  vocroot=NULL;
  vocstop=NULL;
  voclast=NULL;
}

void nvoc::search(uint8_t *str,uint32_t size){
  offset=0;
  lenght=0;
  if((size<3)||(size>vocsize)) return;
  struct vocnode *cnode=vocindx[((uint16_t)str[0]<<8)|str[1]].in;
  struct vocnode *tnode=NULL;
  uint32_t cl;
  while(cnode){
    cl=2;
    tnode=cnode->next->next;
    while((tnode)&&(cl<size)&&(str[cl]==tnode->value)){
      cl++;
      tnode=tnode->next;
    };
    if((cl>2)&&(cl>lenght)){
      lenght=cl;
      offset=cnode->index;
    };
    cnode=cnode->relative;
  };
  if(lenght>2){
    if(offset<vocroot->index) offset=offset+vocsize-vocroot->index;
    else offset-=vocroot->index;
  };
}

void nvoc::write(uint8_t *str,uint32_t size){
  if((size==0)||(size>vocsize)) return;
  uint32_t i;
  uint8_t c;
  uint16_t indx=0;
  struct vocnode *tmp=vocroot;
  struct vocnode *tmplast;
  while(size){
    i=0;
    while(vocstop){
      if(i==size) break;
      c=str[i];
      vocstop->value=c;
      if(voclast){
        indx=voclast->value;
        indx<<=8;
        indx|=c;
        if(vocindx[indx].in==NULL){
          vocindx[indx].in=voclast;
          vocindx[indx].out=voclast;
        }
        else{
          vocindx[indx].out->relative=voclast;
          vocindx[indx].out=voclast;
        };
      };
      voclast=vocstop;
      vocstop=vocstop->next;
      i++;
    };
    str+=i;
    size-=i;
    if(size){
      indx=vocroot->value;
      for(i=0; i<size; i++){
        indx<<=8;
        indx|=tmp->next->value;
        vocindx[indx].in=tmp->relative;
        tmp->relative=NULL;
        tmplast=tmp;
        tmp=tmp->next;
      };
      tmplast->next=NULL;
      voclast->next=vocroot;
      vocstop=vocroot;
      vocroot=tmp;
    };
  };
}

void nvoc::write_woupdate(uint8_t *str,uint16_t size){
  if((size==0)||(size>vocsize)) return;
  uint32_t i;
  struct vocnode *tmp=vocroot;
  struct vocnode *tmplast;
  while(size){
    i=0;
    while(vocstop){
      if(i==size) break;
      vocstop->value=str[i];
      voclast=vocstop;
      vocstop=vocstop->next;
      i++;
    };
    str+=i;
    size-=i;
    if(size){
      for(i=0; i<size; i++){
        tmplast=tmp;
        tmp=tmp->next;
      };
      tmplast->next=NULL;
      voclast->next=vocroot;
      vocstop=vocroot;
      vocroot=tmp;
    };
  };
}

int nvoc::read(uint8_t *str,uint32_t index,uint32_t size){
  if(size==0) return 0;
  if((size>vocsize)||(index>vocsize)) return -1;
  uint32_t ln;
  struct vocnode *tmp=&vocarea[(index+vocroot->index)%vocsize];
  for(ln=0;ln<size;ln++){
    str[ln]=tmp->value;
    tmp=tmp->next;
    if(tmp==NULL){
      ln++;
      break;
    };
  };
  if(ln<size) return -1;
  return 0;
}

/***********************************************************************************************************/
