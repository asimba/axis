#include "ecc.h"

/***********************************************************************************************************/
//Реализация алгоритма коррекции ошибок (ECC)
/***********************************************************************************************************/

ecc::ecc(){ //основной конструктор класса
  reset();
}

ecc::~ecc(){ //деструктор класса
  reset();
}

void ecc::set_decode(){ //функция установки флага декодирования
  op_code=true;
}

void ecc::reset(){ //функция сброса параметров объекта
  op_code=false;
  syndrome=0;
  softerr=0;
  harderr=0;
}

inline void ecc::encode(unsigned char *i, unsigned char *o){ //элементарная функция кодирования
  o[0]=i[0]^i[1]^i[2];
  o[1]=i[1]^i[2]^i[3];
  o[2]=i[0]^i[2]^i[3];
  o[3]=i[0]^i[1]^i[3];
}

inline void ecc::decode(unsigned char *i, unsigned char *o){ //элементарная функция декодирования
  unsigned char t[4];
  syndrome=0;
  encode(i,t);
  if(t[0]^o[0]) syndrome|=0x08;
  if(t[1]^o[1]) syndrome|=0x04;
  if(t[2]^o[2]) syndrome|=0x02;
  if(t[3]^o[3]) syndrome|=0x01;
  switch(syndrome){
    case 0x00: break;
    case 0x08: o[0]=t[0];
               break;
    case 0x04: o[1]=t[1];
               break;
    case 0x02: o[2]=t[2];
               break;
    case 0x01: o[3]=t[3];
               break;
    case 0x0b: i[0]=o[0]^o[2]^o[3];
               break;
    case 0x0d: i[1]=o[0]^o[1]^o[3];
               break;
    case 0x0e: i[2]=o[0]^o[1]^o[2];
               break;
    case 0x07: i[3]=o[1]^o[2]^o[3];
               break;
    default  : syndrome=0xff;
               break;
  };
}

uint32_t ecc::buffer_transcoder(unsigned char *iobuffer, uint32_t length){ //функция обработки буфера
  uint32_t bc;
  uint32_t c;
  uint32_t i=0;
  uint32_t j=0;
  unsigned char ib[4];
  unsigned char ob[4];
  if(op_code){
    if((!(length/4))&&(length%4)){
      harderr++;
      return length;
    };
    bc=(length>>3);
    length=(bc<<2)+(length&0x00000003);
    while(i<bc){
      c=i+length;
      for(j=0; j<4; j++){
        ib[j]=iobuffer[i+j*bc];
        ob[j]=iobuffer[c+j*bc];
      };
      decode(ib,ob);
      switch(syndrome){
        case 0x00: break;
        case 0xff: harderr++;
                   break;
        default  : softerr++;
                   for(j=0; j<4; j++){
                     iobuffer[i+j*bc]=ib[j];
                     iobuffer[c+j*bc]=ob[j];
                   };
                   break;
      };
      i++;
    };
    c=(length&0x00000003);
    if(c){
      for(j=0; j<4; j++) ib[j]=0;
      i=length-c;
      bc=i+length;
      for(j=0; j<4; j++) ob[j]=iobuffer[bc+j];
      for(j=0; j<c; j++) ib[j]=iobuffer[i+j];
      decode(ib,ob);
      switch(syndrome){
        case 0x00: break;
        case 0xff: harderr++;
                   break;
        default  : softerr++;
                   for(j=0; j<c; j++) iobuffer[i+j]=ib[j];
                   for(j=0; j<4; j++) iobuffer[bc+j]=ob[j];
                   break;
      };
    };
    bc=length;
  }
  else{
    bc=(length>>2);
    while(i<bc){
      for(j=0; j<4; j++) ib[j]=iobuffer[i+j*bc];
      encode(ib,ob);
      c=i+length;
      for(j=0; j<4; j++) iobuffer[c+j*bc]=ob[j];
      i++;
    };
    c=(length&0x00000003);
    i=length-c;
    if(c){
      for(j=0; j<4; j++) ib[j]=0;
      for(j=0; j<c; j++) ib[j]=iobuffer[i+j];
      encode(ib,ob);
      i+=length;
      for(j=0; j<4; j++) iobuffer[i+j]=ob[j];
    }
    else i+=length;
    bc=i+(c?4:0);
  };
  return bc;
}

/***********************************************************************************************************/
