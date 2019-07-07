#include "filters.h"
#include "signature.h"

/***********************************************************************************************************/
//"Axis" main class (XS)
/***********************************************************************************************************/

void filters::set_correct(){ //установка флага коррекции обнаруженных ошибок
  op_code=0x10;
}

void filters::set_check(){ //установка флага проверки потока на наличие ошибок
  op_code=0x20;
}

void filters::set_decrypt(){ //установка флага декодирования
  op_code=0x40;
  if(ignore_rc==false) corrector.set_decode();
}

void filters::reset(){ //функция сброса параметров объекта
  corrector.reset();
  ignore_sn=false;
  ignore_rc=false;
  op_code=0;
  errcode=0;
  slag=NULL;
  if(transcoder){
    delete transcoder;
    transcoder=NULL;
  };
  if(iobuffer){
    iolenght=0;
    memset(iobuffer,0,_LE_BUFFER_SIZE);
  }
  else{
    iobuffer=(unsigned char *)calloc(_LE_BUFFER_SIZE,sizeof(unsigned char));
    if(!iobuffer) errcode=0x00000001;
    iolenght=0;
  };
}

filters::filters(){ //базовый конструктор класса
  transcoder=NULL;
  iobuffer=NULL;
  cycles=NULL;
  reset(); //сброс параметров
}

void filters::set(char *p){ //функция установки основных параметров преобразования
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  if(op_code==0x40){
    transcoder=new cryptxs();
    if(transcoder){
      if(cycles) transcoder->cycles=cycles;
      transcoder->set(p,true);
      transcoder->cycles=NULL;
      cycles=NULL;
      slag=transcoder->slag;
    };
  }
  else{
    if(op_code==0){
      transcoder=new cryptxs();
      if(transcoder){
        if(cycles) transcoder->cycles=cycles;
        transcoder->set(p,false);
        transcoder->cycles=NULL;
        cycles=NULL;
        slag=transcoder->slag;
      };
    };
  };
  if((op_code!=0x20)&&(op_code!=0x10)){
    if(transcoder){
      if(!transcoder->slag) errcode=0x00000001;
    }
    else errcode=0x00000001;
  };
}

filters::~filters(){ //деструктор класса
  reset();
  if(iobuffer){
    memset(iobuffer,0,_LE_BUFFER_SIZE);
    free(iobuffer);
    iobuffer=NULL;
  };
}

void filters::buffer_transcode(unsigned char *buffer, uint32_t &lenght){ //функция преобразования буфера
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  if(op_code==0x10) corrector.buffer_transcoder(buffer,lenght);
  else{
    if(op_code==0){
      (*transcoder)<<lenght;
      (*transcoder)<<buffer;
      if(ignore_rc==false)
        lenght=corrector.buffer_transcoder(buffer,lenght);
    }
    else{
      if(ignore_rc==false)
        lenght=corrector.buffer_transcoder(buffer,lenght);
      if(op_code==0x40){
        (*transcoder)<<lenght;
        (*transcoder)<<buffer;
      };
    };
  };
}

#include <unistd.h>

void filters::write_sync_flt(int filedsc){ //синхронизация буфера фильтра (режим записи)
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  if(iolenght){
    buffer_transcode(iobuffer,(uint32_t &)iolenght);
    if(op_code!=0x20){
      iolenght=write(filedsc,(char *)iobuffer,iolenght);
      if(iolenght<=0) errcode=0x00000200;
    };
    iolenght=0;
  };
}

void filters::read_sync_flt(int filedsc){ //синхронизация буфера фильтра (режим чтения)
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  if(ignore_rc)
    iolenght=read(filedsc,(char *)iobuffer,_BUFFER_SIZE);
  else
    iolenght=read(filedsc,(char *)iobuffer,_LE_BUFFER_SIZE);
  if(iolenght<0) errcode=0x00000004;
  else{
    if(iolenght) buffer_transcode(iobuffer,(uint32_t &)iolenght);
  };
}

int32_t filters::read_flt(int filedsc, char *buf, int32_t lenght){ //чтение из буфера фильтра
  if(errcode) return -1;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return -1;
  };
  int32_t ln=0;
  while(lenght){
    if(lenght>iolenght){
      memcpy(buf,iobuffer,iolenght);
      lenght-=iolenght;
      ln+=iolenght;
      buf+=iolenght;
      read_sync_flt(filedsc);
      if(errcode||(iolenght==0)) break;
    }
    else{
      memcpy(buf,iobuffer,lenght);
      memmove(iobuffer,&(iobuffer[lenght]),_BUFFER_SIZE-lenght);
      iolenght-=lenght;
      ln+=lenght;
      lenght=0;
    };
  }
  if(errcode) return -1;
  else return ln;
}

void filters::write_flt(int filedsc, char *buf, int32_t lenght){ //запись в буфер фильтра
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  int32_t bc;
  if(ignore_rc) bc=_BUFFER_SIZE;
  else  bc=(op_code?_LE_BUFFER_SIZE:_BUFFER_SIZE);
  int32_t c=bc-iolenght;
  while(lenght){
    if(lenght>c){
      memcpy(&(iobuffer[iolenght]),buf,c);
      buf+=c;
      iolenght+=c;
      write_sync_flt(filedsc);
      if(errcode) break;
      lenght-=c;
      c=bc;
      iolenght=0;
    }
    else{
      memcpy(&(iobuffer[iolenght]),buf,lenght);
      iolenght+=lenght;
      if(lenght==bc) write_sync_flt(filedsc);
      lenght=0;
    };
  };
}

void filters::transcode(const char *ifile, const char *ofile){ //основная функция преобразования
  if(errcode) return;
  if((((op_code==0x10)||(op_code==0x20))&&(ignore_rc))||(op_code==0)||(op_code==0x40)){
    errcode=0x00000008;
    return;
  };
  int32_t lenght=0;
  FILE *in_stream=NULL;
  FILE *out_stream=NULL;
  int in_file=0;
  int out_file=0;
  unsigned char *buffer=(unsigned char *)calloc(_LE_BUFFER_SIZE,sizeof(unsigned char));
  if(buffer==NULL){
    errcode=0x00000001;
    return;
  };
  char *ibuffer=(char *)calloc(_LE_BUFFER_SIZE,sizeof(char));
  if(ibuffer==NULL){
    errcode=0x00000001;
    free(buffer);
    return;
  };
  char *obuffer=NULL;
  if(op_code==0x10){
    obuffer=(char *)calloc(_LE_BUFFER_SIZE,sizeof(char));
    if(obuffer==NULL){
      errcode=0x00000001;
      free(buffer);
      free(ibuffer);
      return;
    };
    if(ofile){
      out_stream=fopen64(ofile,"wb");
      if(!out_stream){
        errcode=0x000000f0;
        free(buffer);
        free(ibuffer);
        free(obuffer);
        return;
      };
    }
    else out_stream=fdopen(fileno(stdout),"ab");
    out_file=fileno(out_stream);
    setvbuf(out_stream,obuffer,_IOFBF,_LE_BUFFER_SIZE);
  };
  if(ifile){
    in_stream=fopen64(ifile,"rb");
    if(in_stream==NULL){
      errcode=0x00000040;
      if(op_code==0x10){
        free(obuffer);
        fclose(out_stream);
      };
      free(buffer);
      free(ibuffer);
      return;
    };
  }
  else in_stream=fdopen(fileno(stdin),"rb");
  in_file=fileno(in_stream);
  setvbuf(in_stream,ibuffer,_IOFBF,_LE_BUFFER_SIZE);
  corrector.set_decode();
  if(ignore_sn==false){    
    if(axis_sign(in_file,true)==-1){
      errcode=0x00000100;
    }
    else{
      if(op_code==0x10){
        if(axis_sign(out_file,false)==-1){
          errcode=0x00000200;
        };
      };
    };
  };
  if(errcode==0){
    while(1){
      lenght=read(in_file,(char *)buffer,_LE_BUFFER_SIZE);
#ifdef EINTR
      if(lenght<0 && errno==EINTR) continue;
#endif
      if(lenght<=0) break;
      write_flt(out_file,(char *)buffer,(uint32_t)lenght);
      if(errcode) break;
    };
    write_sync_flt(out_file);
  };
  fclose(in_stream);
  if((errcode==0)&&(lenght==-1)) errcode=0x00000004;
  if(op_code!=0x20){
    fclose(out_stream);
    memset(obuffer,0,_LE_BUFFER_SIZE);
    free(obuffer);
    obuffer=NULL;
  };
  memset(buffer,0,_LE_BUFFER_SIZE);
  free(buffer);
  buffer=NULL;
  memset(ibuffer,0,_LE_BUFFER_SIZE);
  free(ibuffer);
  ibuffer=NULL;
}

/***********************************************************************************************************/
