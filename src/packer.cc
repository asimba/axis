/***********************************************************************************************************/
//Базовая реализация упаковки/распаковки файла по упрощённому алгоритму LZSS+RLE+RC32
/***********************************************************************************************************/

#include "packer.h"

template <class T,class V> void packer::del(T& p,uint32_t s,V v){
  if(p){
    fill(p,p+s,v);
    delete[] p;
  };
  p=NULL;
}

packer::packer(){
  iobuf=new uint8_t[0x10000];
  vocbuf=new uint8_t[0x10000];
  cbuffer=new uint8_t[LZ_CAPACITY+1];
  cntxs=new uint8_t[LZ_CAPACITY+1];
  vocarea=new uint16_t[0x10000];
  hashes=new uint16_t[0x10000];
  vocindx=new vocpntr[0x10000];
  frequency=new uint16_t*[256];
  for(int i=0;i<5;i++) frequency[i]=new uint16_t[256];
  fcs=new uint16_t[5];
  lowp=&((uint8_t *)&low)[3];
  hlpp=&((uint8_t *)&hlp)[0];
  flt=NULL;
}

packer::~packer(){
  del(iobuf,0x10000,(uint8_t)0);
  del(vocbuf,0x10000,(uint8_t)0);
  del(cbuffer,LZ_CAPACITY+1,(uint8_t)0);
  del(cntxs,LZ_CAPACITY+1,(uint8_t)0);
  del(vocarea,0x10000,(uint16_t)0);
  del(hashes,0x10000,(uint16_t)0);
  for(int i=0;i<5;i++) del(frequency[i],256,(uint16_t)0);
  delete[] frequency;
  del(fcs,5,(uint16_t)0);
  del(vocindx,0x10000,(vocpntr){0,0});
  lowp=NULL;
  hlpp=NULL;
  flt=NULL;
  cnt=flags=buf_size=symbol=vocroot=voclast=range=low=hlp=icbuf=wpos=rpos=0;
}

void packer::set_filters(filters *f){
  if(f) flt=f;
}

void packer::init(){
  cntxs[0]=flags=rle_flag=length=buf_size=symbol=vocroot=low=hlp=icbuf=wpos=rpos=0;
  cnt=1;
  voclast=0xfffc;
  range=0xffffffff;
  uint32_t i,j;
  for(i=0;i<5;i++){
    for(j=0;j<256;j++) frequency[i][j]=1;
    fcs[i]=256;
  };
  for(i=0;i<0x10000;i++){
    vocbuf[i]=0xff;
    hashes[i]=0;
    vocindx[i].val=1;
    vocarea[i]=(uint16_t)(i+1);
  };
  vocindx[0].in=0;
  vocindx[0].out=0xfffc;
  vocarea[0xfffc]=0xfffc;
  vocarea[0xfffd]=0xfffd;
  vocarea[0xfffe]=0xfffe;
  vocarea[0xffff]=0xffff;
  cpos=&cbuffer[1];
  finalize=false;
}

void packer::wbuf(void *file, uint8_t c){
  if(wpos==0x10000&&(wpos=0,(flt->write_flt((FILE *)file,(char*)iobuf,0x10000)<0))) return;
  iobuf[wpos++]=c;
}

bool packer::rbuf(void *file, uint8_t *c){
  if((int32_t)rpos==icbuf&&(rpos=0,((icbuf=flt->read_flt((FILE *)file,(char*)iobuf,0x10000))<0))) return true;
  if(icbuf) *c=iobuf[rpos++];
  return false;
}

bool packer::rc32_getc(void *file, uint8_t *c, uint8_t cntx){
  uint16_t *f=frequency[cntx],fc=fcs[cntx];
  uint32_t s=0,i;
  while(hlp<low||(low^(low+range))<0x1000000||range<0x10000){
    hlp<<=8;
    if(rbuf(file,hlpp)) return true;
    if(!rpos) return false;
    low<<=8;
    range<<=8;
    if((uint32_t)(range+low)<low) range=~low;
  };
  if((i=(hlp-low)/(range/=fc))<fc){
    while((s+=*f)<=i) f++;
    low+=(s-*f)*range;
    *c=(uint8_t)(f-frequency[cntx]);
    range*=(*f)++;
    if(!++fc){
      f=frequency[cntx];
      for(s=0;s<256;s++){
        *f=((*f)>>1)|1;
        fc+=*f++;
      };
    };
    fcs[cntx]=fc;
    return false;
  }
  else return true;
}

bool packer::rc32_putc(void *file, uint8_t c, uint8_t cntx){
  uint16_t *f=frequency[cntx],fc=fcs[cntx];
  uint32_t s=0,i=c;
  while((low^(low+range))<0x1000000||range<0x10000){
    if(!(wbuf(file,*lowp),wpos)) return true;
    low<<=8;
    range<<=8;
    if((uint32_t)(range+low)<low) range=~low;
  };
  while(i--) s+=*f++;
  low+=s*(range/=fc);
  range*=(*f)++;
  if(!++fc){
    f=frequency[cntx];
    for(s=0;s<256;s++){
      *f=((*f)>>1)|1;
      fc+=*f++;
    };
  };
  fcs[cntx]=fc;
  return false;
}

bool packer::packer_putc(void *file, uint8_t c){
  for(;;){
    if(!finalize&&buf_size!=LZ_BUF_SIZE){
      vocbuf[vocroot]=c;
      if(vocarea[vocroot]==vocroot) vocindx[hashes[vocroot]].val=1;
      else vocindx[hashes[vocroot]].in=vocarea[vocroot];
      vocarea[vocroot]=vocroot;
      uint16_t hs=hashes[voclast]^vocbuf[voclast]^vocbuf[vocroot++];
      vocpntr *indx=&vocindx[(hashes[++voclast]=(hs<<4)|(hs>>12))];
      if(indx->val==1) indx->in=voclast;
      else vocarea[indx->out]=voclast;
      indx->out=voclast;
      buf_size++;
      return false;
    }
    else{
      *cbuffer<<=1;
      if(buf_size){
        uint16_t symbol=vocroot-buf_size,rle=symbol,rle_shift=symbol+LZ_BUF_SIZE;
        while(rle!=vocroot&&vocbuf[++rle]==vocbuf[symbol]);
        rle-=symbol;
        if(buf_size>(length=LZ_MIN_MATCH)&&rle!=buf_size){
          uint16_t cnode=vocindx[hashes[symbol]].in;
          while(cnode!=symbol){
            if(vocbuf[(uint16_t)(symbol+length)]==vocbuf[(uint16_t)(cnode+length)]){
              uint16_t i=symbol,j=cnode;
              while(i!=vocroot&&vocbuf[i]==vocbuf[j++]) i++;
              if((i-=symbol)>=length){
                if(buf_size!=LZ_BUF_SIZE&&(uint16_t)(cnode-rle_shift)>0xfefe) cnode=vocarea[cnode];
                else{
                  offset=cnode;
                  if((length=i)==buf_size) break;
                };
              };
            };
            cnode=vocarea[cnode];
          };
        };
        if(rle>length){
          length=rle;
          offset=vocbuf[symbol];
        }
        else offset=~(uint16_t)(offset-rle_shift);
        uint16_t i=length-LZ_MIN_MATCH;
        if(i){
          cntxs[cnt++]=1;
          cntxs[cnt++]=2;
          cntxs[cnt++]=3;
          *cpos++=--i;
          *(uint16_t*)cpos++=offset;
          buf_size-=length;
        }
        else{
          cntxs[cnt++]=4;
          *cbuffer|=1;
          *cpos=vocbuf[symbol];
          buf_size--;
        };
      }
      else{
        cntxs[cnt++]=1;
        cntxs[cnt++]=2;
        cntxs[cnt++]=3;
        length=0;
        cpos++;
        *(uint16_t*)cpos++=0x0100;
      };
      cpos++;
      if(--flags&&length) continue;
      *cbuffer<<=flags;
      for(int i=0;i<cpos-cbuffer;i++)
        if(rc32_putc(file,cbuffer[i],cntxs[i])) return true;
      if(!length){
        for(int i=4;i;i--){
          if(!(wbuf(file,*lowp),wpos)) return true;
          low<<=8;
        };
        if(flt->write_flt((FILE *)file,(char*)iobuf,wpos)<0) return true;
        break;
      };
      cpos=&cbuffer[1];
      cnt=1;
      flags=8;
    };
  };
  return false;
}

bool packer::load_header(void *file){
  for(unsigned int i=0;i<sizeof(uint32_t);i++){
    hlp<<=8;
    if(rbuf(file,hlpp)||!rpos) return true;
  };
  length=0;
  return false;
}

bool packer::packer_getc(void *file, uint8_t *c){
  for(;;){
    if(length){
      if(!rle_flag) symbol=vocbuf[offset++];
      vocbuf[vocroot++]=symbol;
      *c=(uint8_t)symbol;
      length--;
      return false;
    }
    else{
      uint8_t s;
      if(!flags){
        cpos=cbuffer;
        if(rc32_getc(file,cpos++,0)) return true;
        for(s=~*cbuffer;s;flags++) s&=s-1;
        uint8_t cflags=*cbuffer;
        cnt=8+(flags<<1);
        for(s=0;s<cnt;){
          if(cflags&0x80) cntxs[s++]=4;
          else{
            cntxs[s++]=1;
            cntxs[s++]=2;
            cntxs[s++]=3;
          };
          cflags<<=1;
        };
        for(s=0;s<cnt;s++)
          if(rc32_getc(file,cpos++,cntxs[s])) return true;
        cpos=&cbuffer[1];
        flags=8;
      };
      length=rle_flag=1;
      if(*cbuffer&0x80) symbol=*cpos;
      else{
        length+=LZ_MIN_MATCH+*cpos++;
        if((offset=*(uint16_t*)cpos++)<0x0100) symbol=(uint8_t)(offset);
        else{
          if(offset==0x0100){
            finalize=true;
            break;
          };
          offset=~offset+(uint16_t)(vocroot+LZ_BUF_SIZE);
          rle_flag=0;
        };
      };
      *cbuffer<<=1;
      cpos++;
      flags--;
    };
  };
  return false;
}

int32_t packer::packer_read(void* file, char *buf, int32_t l){
  if(flt==NULL) return -1;
  int32_t ret=0;
  while(l--){
    if(packer_getc(file,(uint8_t *)buf++)) return -1;
    if(finalize) return 0;
    else ret++;
  };
  return ret;
}

int32_t packer::packer_write(void* file, char *buf, int32_t l){
  if(flt==NULL) return -1;
  if(buf==NULL||l==0){
    finalize=true;
    packer_putc(file,0);
  }
  else{
    while(l--){
      if(packer_putc(file,*buf++)) return -1;
    };
  };
  return 1;
}
