#include "lzss.h"

/***********************************************************************************************************/
//Базовый класс для упаковки/распаковки потока по упрощённому алгоритму LZSS
/***********************************************************************************************************/

codebuf::codebuf(){
  reset();
  buffer_size=LZ_LENGHT_CAPACITY+LZ_OFFSET_CAPACITY+1;
  buffer=(uint8_t *)calloc(buffer_size,sizeof(uint8_t));
}

codebuf::~codebuf(){
  if(buffer){
    memset(buffer,0,buffer_size);
    free(buffer);
    buffer=NULL;
  };
  reset();
  buffer_size=0;
}

void codebuf::write(uint8_t flag, uint32_t lenght, uint32_t offset){
  if(flags_count==8) return;
  flags<<=1;
  if(flag){
    flags|=0x01;
    buffer[buffer_position]=(uint8_t)lenght;
    buffer_position++;
    buffer[buffer_position]=(uint8_t)(offset>>8);
    buffer_position++;
    buffer[buffer_position]=(uint8_t)offset;
  }
  else buffer[buffer_position]=(uint8_t)lenght;
  buffer_position++;
  flags_count++;
}

void codebuf::read(uint8_t &flag, uint32_t &lenght, uint32_t &offset){
  if(flags_count==0) return;
  if(flags&0x80){
    flag=1;
    lenght=buffer[buffer_position];
    buffer_position++;
    offset=buffer[buffer_position];
    buffer_position++;
    offset<<=8;
    offset|=buffer[buffer_position];
    buffer_position++;
  }
  else{
    flag=0;
    lenght=buffer[buffer_position];
    buffer_position++;
  };
  flags<<=1;
  flags_count--;
}

void codebuf::reset(){
  flags=0;
  flags_count=0;
  buffer_position=0;
}

uint8_t codebuf::flags_buffer_size(){
  uint8_t l=0;
  if(flags_count){
    uint8_t tflags=flags;
    for(uint8_t c=0; c<8; c++){
      if(tflags&0x01) l+=3;
      else l++;
      tflags>>=1;
    };
  };
  return l;
}

lzss::lzss(){
  lzbuf=NULL;
  if(pack.frequency==NULL) return;
  cbuf=new codebuf();
  if(cbuf&&(cbuf->buffer)) voc=new nvoc(LZ_VOC_SIZE);
  else return;
  if(voc){
    if(voc->vocsize==0){
      delete voc;
      voc=NULL;
      delete cbuf;
      cbuf=NULL;
    };
  }
  else{
    delete cbuf;
    cbuf=NULL;
  };
  if(voc){
    lzbuf=(uint8_t *)calloc(LZ_BUF_SIZE,sizeof(uint8_t));
    if(lzbuf){
      buf_size=0;
      op_code=true;
      finalize=false;
      decoding_error=false;
    }
    else{
      delete voc;
      voc=NULL;
      delete cbuf;
      cbuf=NULL;
    };
  };
}

lzss::~lzss(){
  if(lzbuf){
    delete voc;
    voc=NULL;
    delete cbuf;
    cbuf=NULL;
    memset(lzbuf,0,LZ_BUF_SIZE);
    free(lzbuf);
    lzbuf=NULL;
  };
  buf_size=0;
  op_code=false;
  finalize=false;
  decoding_error=false;
}

void lzss::set(filters *flt, bool mode){
  op_code=mode;
  pack.set(flt,mode);
}

void lzss::lzss_write(int filedsc, char *buf, int32_t ln){
  int retln=0;
  uint32_t c;
  if(finalize) ln+=buf_size;
  while(ln){
    retln=LZ_BUF_SIZE-buf_size;
    if(finalize==false){
      if(retln!=0){
        if(retln>ln) retln=ln;
        memcpy(lzbuf+buf_size,buf,retln);
        buf+=retln;
        ln-=retln;
        buf_size+=retln;
      }
    }
    else ln-=buf_size;
    voc->search(lzbuf,buf_size);
    if(cbuf->flags_count==8){
      if(pack.rc32_write(filedsc,(char*)(&(cbuf->flags)),1)<0) return;
      if(pack.rc32_write(filedsc,(char*)cbuf->buffer,
                         cbuf->buffer_position)<0) return;
      cbuf->reset();
    };
    if(voc->lenght){
      c=voc->lenght;
      cbuf->write(1,c-LZ_MIN_MATCH,voc->offset);
    }
    else{
      c=1;
      cbuf->write(0,lzbuf[0],0);
    };
    voc->write(lzbuf,c);
    memmove(lzbuf,lzbuf+c,LZ_BUF_SIZE-c);
    buf_size-=c;
  };
  return;
}

int32_t lzss::lzss_read(int filedsc, char *buf, int32_t ln){
  int32_t retln;
  int32_t rl=0;
  uint8_t f=0;
  uint32_t c=0;
  uint32_t o=0;
  while(ln>0){
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
      if(cbuf->flags_count==0){
        if(pack.rc32_read(filedsc,(char*)(&cbuf->flags),1)<=0) return -1;
        if(pack.decoding_error){
          decoding_error=true;
          return -1;
        };
        cbuf->flags_count=8;
        cbuf->buffer_position=0;
        if(pack.rc32_read(filedsc,(char*)cbuf->buffer,
                          cbuf->flags_buffer_size())<=0) return -1;
        if(pack.decoding_error){
          decoding_error=true;
          return -1;
        };
      };
      cbuf->read(f,c,o);
      if(f==0){
        lzbuf[0]=(uint8_t)(c);
        voc->write_woupdate(lzbuf,1);
        buf_size=1;
      }
      else{
        c+=LZ_MIN_MATCH;
        if(voc->read(lzbuf,o,c)<0) return -1;
        voc->write_woupdate(lzbuf,c);
        buf_size=c;
      };
      if(finalize) break;
    };
  };
  return rl;
}

int lzss::is_eof(int filedsc){
  if(finalize&&(buf_size==0)){
    if(op_code){
      if(cbuf->flags_count){
        while(cbuf->flags_count<8) cbuf->write(0,0,0);
        if(pack.rc32_write(filedsc,(char*)(&(cbuf->flags)),1)<0) return -1;
        if(pack.rc32_write(filedsc,(char*)cbuf->buffer,
                           cbuf->buffer_position)<0) return -1;
        cbuf->reset();
      };
      pack.finalize=true;
      while(pack.is_eof()==false){
        if(pack.rc32_write(filedsc,NULL,0)<0) return -1;
      };
      return 1;
    }
    else return 1;
  }
  else return 0;
}

/***********************************************************************************************************/
