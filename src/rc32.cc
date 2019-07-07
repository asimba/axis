#include "rc32.h"

/***********************************************************************************************************/
//Базовый класс для упаковки/распаковки потока по упрощённому алгоритму Range Coder 32-bit
/***********************************************************************************************************/

void rc32::initialize(){
  if(frequency==NULL) return;
  for(low=0; low<257; low++) frequency[low]=low;
  memset(pqbuffer,0,_RC32_BUFFER_SIZE);
  waiting=0;
  low=0;
  range=0xffffffff;
  ibuffer.content=NULL;
  obuffer.content=NULL;
  hlp=0;
  eof=false;
  finalize=false;
  decoding_error=false;
  op_code=true;
  init=true;
  xs=NULL;
}

void rc32::set(filters *flt, bool pack){
  ibuffer.state=0;
  obuffer.state=0;
  ibuffer.locked=true;
  op_code=pack;
  xs=flt;
  if(op_code==false){
    ibuffer.content=pqbuffer;
    ibuffer.size=_RC32_BUFFER_SIZE;
    obuffer.locked=true;
  }
  else{
    obuffer.content=pqbuffer;
    obuffer.size=_RC32_BUFFER_SIZE;
    obuffer.locked=false;
    ibuffer.size=0;
  };
}

int32_t rc32::rc32_read(int filedsc, char *buf, int32_t lenght){
  int32_t rw;
  obuffer.content=(uint8_t *)buf;
  obuffer.size=(uint32_t)lenght;
  obuffer.state=0;
  if(obuffer.size) obuffer.locked=false;
  else{
    obuffer.locked=true;
    return 0;
  };
  while(1){
    if(ibuffer.locked&&(finalize==false)){
      rw=xs->read_flt(filedsc,(char *)ibuffer.content,_RC32_BUFFER_SIZE);
      if(rw==0) finalize=true;
      if(rw<0){
        lenght=rw;
        break;
      }
      if(rw>0){
        ibuffer.state=0;
        ibuffer.size=(uint32_t)rw;
        ibuffer.locked=false;
      };
    };
    if(init){
      initialize_decode();
      if(waiting){
        if(ibuffer.locked&&finalize){
          decoding_error=true;
          lenght=-1;
          break;
        }
        else continue;
      }
      else init=false;
    }
    else decode();
    if(decoding_error){
      lenght=-1;
      break;
    };
    if(eof){
      lenght=obuffer.state;
      break;
    };
    if(obuffer.locked) break;
  };
  return lenght;
}

int32_t rc32::rc32_write(int filedsc, char *buf, int32_t lenght){
  ibuffer.content=(uint8_t *)buf;
  ibuffer.size=(uint32_t)lenght;
  ibuffer.state=0;
  if(ibuffer.size) ibuffer.locked=false;
  else{
    ibuffer.locked=true;
    if(finalize==false) return 1;
  };
  while(1){
    if(obuffer.locked||(obuffer.state&&eof)){
      xs->write_flt(filedsc,(char *)obuffer.content,obuffer.state);
      if(xs->errcode) return -1;
      obuffer.state=0;
      obuffer.size=_RC32_BUFFER_SIZE;
      obuffer.locked=false;
    };
    if(finalize){
      if(init){
        if(waiting) encode();
        if(waiting==0) init=false;
      }
      else finalize_encode();
    }
    else encode();
    if(obuffer.state&&eof) continue;
    if((waiting==0)&&(ibuffer.locked)) break;
  };
  return 1;
}

bool rc32::is_eof(){
  return ((eof&&(obuffer.state==0))?true:false);
}

rc32::rc32(){
  frequency=(uint32_t *)calloc(257,sizeof(uint32_t));
  if(frequency){
    pqbuffer=(uint8_t *)calloc(_RC32_BUFFER_SIZE,sizeof(uint8_t));
    if(pqbuffer==NULL){
      free(frequency);
      frequency=NULL;
    };
  };
  initialize();
}

rc32::~rc32(){
  initialize();
  if(frequency){
    memset(frequency,0,257);
    free(frequency);
    frequency=NULL;
    memset(pqbuffer,0,_RC32_BUFFER_SIZE);
    free(pqbuffer);
    pqbuffer=NULL;
  };
}

void rc32::finalize_encode(){
  if(eof) return;
  if(waiting==0) waiting=4;
  while(obuffer.locked==false){
    obuffer<<(uint8_t)(low>>24);
    low<<=8;
    waiting--;
    if(waiting==0){
      eof=true;
      break;
    };
  };
}

void rc32::initialize_decode(){
  if(waiting==0) waiting=4;
  uint8_t c=0;
  while(ibuffer.locked==false){
    ibuffer>>c;
    hlp|=c;
    waiting--;
    if(waiting==0) break;
    hlp<<=8;
  };
}

inline void rc32::rescale(){
  for(uint16_t i=1; i<257; i++){
    frequency[i]>>=1;
    if(frequency[i]<=frequency[i-1]) frequency[i]=frequency[i-1]+1;
  };
}

inline void rc32::encode(){
  uint8_t symbol=0;
  if(waiting) waiting=0;
  else{
    ibuffer>>symbol;
    range/=frequency[256];
    low+=frequency[symbol]*range;
    range*=frequency[symbol+1]-frequency[symbol];
    for(uint16_t i=symbol+1; i<257; i++) frequency[i]++;
    if(frequency[256]>=0x10000) rescale();
  };
  while(range<0x10000){
    if(((low&0xff0000)==0xff0000)&&(range+(low&0xffff)>=0x10000))
      range=0x10000-(low&0xffff);
    obuffer<<(uint8_t)(low>>24);
    waiting=(uint8_t)obuffer.locked;
    low<<=8;
    range<<=8;
    if(waiting) return;
  };
}

inline void rc32::decode(){
  if(eof) return;
  if(finalize&&(hlp==0)){
    eof=true;
    return;
  };
  if(hlp<low){
    decoding_error=true;
    return;
  };
  uint8_t symbol=0;
  uint16_t i;
  if(waiting) waiting=0;
  else{
    range/=frequency[256];
    if(range==0){
      decoding_error=true;
      return;
    };
    uint32_t count=(hlp-low)/range;
    if(count>=frequency[256]){
      decoding_error=true;
      return;
    }
    for(i=255; frequency[i]>count; i--) if(!i) break;
    symbol=(uint8_t)i;
    obuffer<<symbol;
    low+=frequency[i]*range;
    range*=frequency[i+1]-frequency[i];
    for(i=(uint16_t)symbol+1; i<257; i++) frequency[i]++;
    if(frequency[256]>=0x10000) rescale();
  };
  while((range<0x10000)||(hlp<low)){
    if(((low&0xff0000)==0xff0000)&&(range+(low&0xffff)>=0x10000))
      range=0x10000-(low&0xffff);
    hlp<<=8;
    if(finalize==false){
      ibuffer>>symbol;
      waiting=(uint8_t)ibuffer.locked;
    }
    else{
      if(range==0){
        decoding_error=true;
        break;
      };
    };
    hlp|=symbol;
    low<<=8;
    range<<=8;
    if(waiting) break;
  };
}

/***********************************************************************************************************/
