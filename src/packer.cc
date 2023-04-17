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
  vocarea=new uint16_t[0x10000];
  hashes=new uint16_t[0x10000];
  vocindx=new vocpntr[0x10000];
  frequency=new uint16_t*[256];
  for(int i=0;i<256;i++) frequency[i]=new uint16_t[256];
  fcs=new uint16_t[256];
  lowp=&((uint8_t *)&low)[3];
  hlpp=&((uint8_t *)&hlp)[0];
  wpntr=iobuf;
  flt=NULL;
}

packer::~packer(){
  del(iobuf,0x10000,(uint8_t)0);
  del(vocbuf,0x10000,(uint8_t)0);
  del(cbuffer,LZ_CAPACITY+1,(uint8_t)0);
  del(vocarea,0x10000,(uint16_t)0);
  del(hashes,0x10000,(uint16_t)0);
  for(int i=0;i<256;i++) del(frequency[i],256,(uint16_t)0);
  delete[] frequency;
  del(fcs,256,(uint16_t)0);
  del(vocindx,0x10000,(vocpntr){0,0});
  lowp=NULL;
  hlpp=NULL;
  wpntr=NULL;
  flt=NULL;
  buf_size=flags=vocroot=voclast=range=low=hlp=icbuf=wpos=rpos=cstate=symbol=hs=0;
}

void packer::set_filters(filters *f){
  if(f) flt=f;
}

void packer::init(){
  length=buf_size=flags=vocroot=*cbuffer=low=hlp=icbuf=wpos=rpos=cstate=rle_flag=0;
  voclast=0xfffd;
  range=0xffffffff;
  uint32_t i,j;
  for(i=0;i<256;i++){
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
  hs=0x00ff;
  cpos=&cbuffer[1];
  finalize=false;
  eofs=false;
}

void packer::wbuf(void *file, uint8_t c){
  if(flt==NULL){
    wpos=0;
    return;
  };
  if(wpos<0x10000){
    *wpntr++=c;
    wpos++;
  }
  else{
    if(flt->write_flt((FILE *)file,(char*)iobuf,wpos)>=0){
      *iobuf=c;
      wpos=1;
      wpntr=iobuf+1;
      return;
    };
    wpos=0;
  };
}

bool packer::rbuf(void *file, uint8_t *c){
  if(flt==NULL){
    rpos=0;
    return true;
  };
  if(rpos<(uint32_t)icbuf) *c=iobuf[rpos++];
  else{
    rpos=0;
    if((icbuf=flt->read_flt((FILE *)file,(char*)iobuf,0x10000))<0) return true;
    if(icbuf>0) *c=iobuf[rpos++];
  };
  return false;
}

bool packer::rc32_getc(void *file, uint8_t *c){
  uint16_t *f=frequency[cstate],fc=fcs[cstate];
  uint32_t count,s=0;
  while((low^(low+range))<0x1000000||range<0x10000||hlp<low){
    hlp<<=8;
    if(rbuf(file,hlpp)) return true;
    if(rpos==0) return false;
    low<<=8;
    range<<=8;
    if((uint32_t)(range+low)<low) range=~low;
  };
  if((count=(hlp-low)/(range/=fc))>=fc) return true;
  while((s+=*f++)<=count);
  *c=(uint8_t)(--f-frequency[cstate]);
  low+=(s-*f)*range;
  range*=(*f)++;
  if(++fc==0){
    f=frequency[cstate];
    for(s=0;s<256;s++){
      *f=((*f)>>1)|1;
      fc+=*f++;
    };
  };
  fcs[cstate]=fc;
  cstate=*c;
  return false;
}

bool packer::rc32_putc(void *file, uint8_t c){
  uint16_t *f=frequency[cstate],fc=fcs[cstate];
  uint32_t s=0,i;
  while((low^(low+range))<0x1000000||range<0x10000){
    wbuf(file,*lowp);
    if(wpos==0) return true;
    low<<=8;
    range<<=8;
    if((uint32_t)(range+low)<low) range=~low;
  };
  range/=fc;
  for(i=0;i<c;i++) s+=f[i];
  low+=s*range;
  range*=f[i]++;
  if(++fc==0){
    for(i=0;i<256;i++){
      *f=((*f)>>1)|1;
      fc+=*f++;
    };
  };
  fcs[cstate]=fc;
  cstate=c;
  return false;
}

bool packer::packer_putc(void *file, uint8_t c){
  for(;;){
    if((LZ_BUF_SIZE-buf_size)>0&&finalize==false){
      vocbuf[vocroot]=c;
      if(vocarea[vocroot]==vocroot) vocindx[hashes[vocroot]].val=1;
      else vocindx[hashes[vocroot]].in=vocarea[vocroot];
      vocarea[vocroot]=vocroot;
      hs^=vocbuf[vocroot];
      hs=(hs<<4)|(hs>>12);
      hashes[voclast]=hs;
      hs^=vocbuf[voclast];
      vocpntr *indx=&vocindx[hashes[voclast]];
      if(indx->val==1) indx->in=voclast;
      else vocarea[indx->out]=voclast;
      indx->out=voclast;
      voclast++;
      vocroot++;
      buf_size++;
      return false;
    }
    else{
      uint16_t i,rle,cnode,rle_shift=0;;
      uint8_t *w;
      *cbuffer<<=1;
      rle=symbol=vocroot-buf_size;
      if(buf_size){
        cnode=vocbuf[symbol];
        while(rle!=vocroot&&vocbuf[++rle]==cnode);
        rle-=symbol;
        length=LZ_MIN_MATCH;
        if(buf_size>LZ_MIN_MATCH&&rle<buf_size){
          cnode=vocindx[hashes[symbol]].in;
          rle_shift=(uint16_t)(vocroot+LZ_BUF_SIZE-buf_size);
          while(cnode!=symbol){
            if(vocbuf[(uint16_t)(symbol+length)]==vocbuf[(uint16_t)(cnode+length)]){
              uint16_t k=cnode;
              i=symbol;
              while(vocbuf[i++]==vocbuf[k]&&k++!=symbol);
              if((i=(uint16_t)(i-symbol)-1)>=length){
                //while buf_size==LZ_BUF_SIZE: minimal offset > 0x0104;
                if(buf_size<LZ_BUF_SIZE){
                  if((uint16_t)(cnode-rle_shift)>0xfeff){
                    cnode=vocarea[cnode];
                    continue;
                  };
                };
                offset=cnode;
                if(i>=buf_size){
                  length=buf_size;
                  break;
                }
                else length=i;
              };
            };
            cnode=vocarea[cnode];
          };
        };
        if(rle>length){
          *cpos++=rle-LZ_MIN_MATCH-1;
          *(uint16_t*)cpos++=((uint16_t)(vocbuf[symbol]));
          buf_size-=rle;
        }
        else{
          if(length>LZ_MIN_MATCH){
            *cpos++=length-LZ_MIN_MATCH-1;
            *(uint16_t*)cpos++=~(uint16_t)(offset-rle_shift);
            buf_size-=length;
          }
          else{
            *cbuffer|=0x01;
            *cpos=vocbuf[symbol];
            buf_size--;
          };
        };
      }
      else{
        cpos++;
        *(uint16_t*)cpos++=0x0100;
        if(finalize) eofs=true;
      };
      cpos++;
      flags--;
      if(!flags||eofs){
        *cbuffer<<=flags;
        w=cbuffer;
        for(i=cpos-cbuffer;i;i--)
          if(rc32_putc(file,*w++)) return true;
        flags=8;
        cpos=&cbuffer[1];
        if(eofs){
          for(i=4;i;i--){
            wbuf(file,*lowp);
            if(wpos==0) return true;
            low<<=8;
          };
          if(flt->write_flt((FILE *)file,(char*)iobuf,wpos)<0) return true;
          break;
        };
      };
    };
  };
  return false;
}

bool packer::load_header(void *file){
  for(unsigned int i=0;i<sizeof(uint32_t);i++){
    hlp<<=8;
    if(rbuf(file,hlpp)||rpos==0) return true;
  };
  length=0;
  return false;
}

bool packer::packer_getc(void *file, uint8_t *c){
  for(;;){
    if(length){
      if(rle_flag==0) symbol=vocbuf[offset++];
      vocbuf[vocroot++]=symbol;
      *c=(uint8_t)symbol;
      length--;
      return false;
    }
    else{
      uint8_t s;
      if(flags==0){
        cpos=cbuffer;
        if(rc32_getc(file,cpos++)) return true;
        for(s=~*cbuffer;s;flags++) s&=s-1;
        for(s=8+(flags<<1);s;s--)
          if(rc32_getc(file,cpos++)) return true;
        flags=8;
        cpos=cbuffer+1;
      };
      length=rle_flag=1;
      if(*cbuffer&0x80) symbol=*cpos;
      else{
        length=LZ_MIN_MATCH+1+*cpos++;
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
  int32_t ret=0;
  while(l--){
    if(packer_getc(file,(uint8_t *)buf++)) return -1;
    if(finalize) return 0;
    else ret++;
  };
  return ret;
}

int32_t packer::packer_write(void* file, char *buf, int32_t l){
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
