#include "lzss.h"

/***********************************************************************************************************/
//Базовый класс для упаковки/распаковки потока по упрощённому алгоритму LZSS
/***********************************************************************************************************/

lzss::lzss(){
  lzbuf=NULL;
  if((pack.frequency==NULL)||(voc.vocindx==NULL)) return;
  cbuffer=(uint8_t *)calloc(LZ_CAPACITY+1,sizeof(uint8_t));
  if(cbuffer){
    cbuffer[0]=cflags_count=0;
    cbuffer_position=1;
  }
  else return;
  lzbuf_pntr=lzbuf=(uint8_t *)calloc(LZ_BUF_SIZE*2,sizeof(uint8_t));
  if(lzbuf){
    lzbuf_indx=buf_size=0;
    op_code=true;
    finalize=false;
  }
  else free(cbuffer);
}

lzss::~lzss(){
  if(lzbuf){
    memset(cbuffer,0,LZ_CAPACITY+1);
    free(cbuffer);
    cbuffer=NULL;
    memset(lzbuf,0,LZ_BUF_SIZE*2);
    free(lzbuf);
    lzbuf=NULL;
  };
  cflags_count=0;
  cbuffer_position=0;
  buf_size=0;
  lzbuf_pntr=NULL;
  lzbuf_indx=0;
  op_code=false;
  finalize=false;
}

void lzss::set_filters(filters *f){
  if(f) pack.flt=f;
}

int32_t lzss::lzss_write(FILE* file, char *buf, int32_t ln){
  int32_t retln=0;
  uint32_t c;
  if(finalize) ln+=buf_size;
  while(ln){
    retln=LZ_BUF_SIZE-buf_size;
    if(finalize==false){
      if(retln!=0){
        if(retln>ln) retln=ln;
        memcpy(lzbuf_pntr+buf_size,buf,retln);
        buf+=retln;
        ln-=retln;
        buf_size+=retln;
      }
    }
    else ln-=buf_size;
    if((finalize==false)&&(ln==0)) break;
    voc.search(lzbuf_pntr,buf_size);
    if(cflags_count==8){
      if(pack.rc32_write(file,(char*)cbuffer,cbuffer_position)<0) return -1;
      cbuffer[0]=cflags_count=0;
      cbuffer_position=1;
    };
    if(voc.lenght>=LZ_MIN_MATCH){
      c=voc.lenght;
      cbuffer[0]<<=1;
      cbuffer[0]|=0x01;
      cbuffer[cbuffer_position++]=(uint8_t)(c-LZ_MIN_MATCH);
      *(uint16_t*)&cbuffer[cbuffer_position]=voc.offset;
      cbuffer_position+=sizeof(uint16_t);
      cflags_count++;
    }
    else{
      c=1;
      cbuffer[0]<<=1;
      cbuffer[cbuffer_position++]=lzbuf_pntr[0];
      cflags_count++;
    };
    voc.write(lzbuf_pntr,c);
    if((lzbuf_indx+c)>=LZ_BUF_SIZE){
      memmove(lzbuf,lzbuf_pntr+c,LZ_BUF_SIZE-c);
      lzbuf_pntr=lzbuf;
      lzbuf_indx=0;
    }
    else{
      lzbuf_pntr=&lzbuf_pntr[c];
      lzbuf_indx+=c;
    };
    buf_size-=c;
  };
  return 1;
}

int32_t lzss::lzss_read(FILE* file, char *buf, int32_t ln){
  int32_t retln;
  int32_t rl=0;
  uint8_t f=0;
  uint16_t c=0;
  uint16_t o=0;
  if(pack.eof) return 0;
  while(ln){
    if(buf_size){
      retln=(ln>buf_size)?buf_size:ln;
      memcpy(buf,lzbuf,retln);
      memmove(lzbuf,lzbuf+retln,LZ_BUF_SIZE-retln);
      buf+=retln;
      ln-=retln;
      rl+=retln;
      buf_size-=retln;
    }
    else{
      if(cflags_count==0){
        if(pack.rc32_read(file,(char*)cbuffer,1)<0) return -1;
        if(pack.eof) return 0;
        cbuffer_position=1;
        c=0;
        f=cbuffer[0];
        for(cflags_count=0; cflags_count<8; cflags_count++){
          if(f&0x01) c+=3;
          else c++;
          f>>=1;
        };
        if(pack.rc32_read(file,(char*)(&cbuffer[1]),c)<0) return -1;
      };
      if(cbuffer[0]&0x80){
        c=cbuffer[cbuffer_position++]+LZ_MIN_MATCH;
        o=*(uint16_t*)&cbuffer[cbuffer_position];
        cbuffer_position+=sizeof(uint16_t);
        voc.read(lzbuf,o,c);
        voc.write_woupdate(lzbuf,c);
        buf_size=c;
      }
      else{
        lzbuf[0]=cbuffer[cbuffer_position++];
        voc.write_woupdate(lzbuf,1);
        buf_size=1;
      };
      cbuffer[0]<<=1;
      cflags_count--;
      if(finalize) break;
    };
  };
  return rl;
}


int lzss::is_eof(FILE* file){
  if(finalize&&(buf_size==0)){
    if(op_code){
      if(cflags_count){
        while(cflags_count<8){
          cbuffer[0]<<=1;
          cbuffer[cbuffer_position++]=0;
          cflags_count++;
        };
        if(pack.rc32_write(file,(char*)cbuffer,cbuffer_position)<0) return -1;
      };
      pack.finalize=true;
      while(pack.eof==false){
        if(pack.rc32_write(file,NULL,0)<0) return -1;
      };
      return 1;
    }
    else return 1;
  }
  else return 0;
}

/***********************************************************************************************************/
