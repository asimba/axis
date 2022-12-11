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
  eof=0;
  slag=NULL;
  if(transcoder){
    delete transcoder;
    transcoder=NULL;
  };
  if(iobuffer){
    iolength=0;
    memset(iobuffer,0,_LE_BUFFER_SIZE);
  }
  else{
    iobuffer=(unsigned char *)calloc(_LE_BUFFER_SIZE,sizeof(unsigned char));
    if(!iobuffer) errcode=0x00000001;
    iolength=0;
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

void filters::buffer_transcode(unsigned char *buffer, uint32_t &length){ //функция преобразования буфера
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  if(op_code==0x10) corrector.buffer_transcoder(buffer,length);
  else{
    if(op_code==0){
      (*transcoder)<<length;
      (*transcoder)<<buffer;
      if(ignore_rc==false)
        length=corrector.buffer_transcoder(buffer,length);
    }
    else{
      if(ignore_rc==false)
        length=corrector.buffer_transcoder(buffer,length);
      if(op_code==0x40){
        (*transcoder)<<length;
        (*transcoder)<<buffer;
      };
    };
  };
}

void filters::write_sync_flt(FILE *filedsc){ //синхронизация буфера фильтра (режим записи)
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  if(iolength){
    buffer_transcode(iobuffer,(uint32_t &)iolength);
    if(op_code!=0x20){
      if((int32_t)fwrite(iobuffer,1,iolength,filedsc)<iolength) errcode=0x00000200;
    };
    iolength=0;
  };
}

void filters::read_sync_flt(FILE *filedsc){ //синхронизация буфера фильтра (режим чтения)
  if(errcode) return;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return;
  };
  if(ignore_rc)
    iolength=fread(iobuffer,1,_BUFFER_SIZE,filedsc);
  else
    iolength=fread(iobuffer,1,_LE_BUFFER_SIZE,filedsc);
  if(ferror(filedsc)) errcode=0x00000004;
  else{
    if(iolength) buffer_transcode(iobuffer,(uint32_t &)iolength);
  };
}

int32_t filters::read_flt(FILE *filedsc, char *buf, int32_t length){ //чтение из буфера фильтра
  if(errcode) return -1;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return -1;
  };
  int32_t ln=0;
  while(length){
    if(length>iolength){
      memcpy(buf,iobuffer,iolength);
      length-=iolength;
      ln+=iolength;
      buf+=iolength;
      read_sync_flt(filedsc);
      if(errcode||(iolength==0)) break;
    }
    else{
      memcpy(buf,iobuffer,length);
      memmove(iobuffer,&(iobuffer[length]),_BUFFER_SIZE-length);
      iolength-=length;
      ln+=length;
      length=0;
    };
  }
  if(errcode) return -1;
  else return ln;
}

int32_t filters::write_flt(FILE *filedsc, char *buf, int32_t length){ //запись в буфер фильтра
  if(errcode) return -1;
  if(((op_code==0x10)||(op_code==0x20))&&(ignore_rc)){
    errcode=0x00000008;
    return -1;
  };
  int32_t bc;
  if(ignore_rc) bc=_BUFFER_SIZE;
  else  bc=(op_code?_LE_BUFFER_SIZE:_BUFFER_SIZE);
  int32_t c=bc-iolength;
  while(length){
    if(length>c){
      memcpy(&(iobuffer[iolength]),buf,c);
      buf+=c;
      iolength+=c;
      write_sync_flt(filedsc);
      if(errcode) break;
      length-=c;
      c=bc;
      iolength=0;
    }
    else{
      memcpy(&(iobuffer[iolength]),buf,length);
      iolength+=length;
      if(length==bc) write_sync_flt(filedsc);
      length=0;
    };
  };
  if(errcode) return -1;
  return 0;
}

int32_t filters::is_eof(FILE *filedsc){
  return eof;
}

void filters::transcode(const char *ifile, const char *ofile){ //основная функция преобразования
  if(errcode) return;
  if((((op_code==0x10)||(op_code==0x20))&&(ignore_rc))||(op_code==0)||(op_code==0x40)){
    errcode=0x00000008;
    return;
  };
  int32_t length=0;
  FILE *in_stream=NULL;
  FILE *out_stream=NULL;
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
  setvbuf(in_stream,ibuffer,_IOFBF,_LE_BUFFER_SIZE);
  corrector.set_decode();
  if(ignore_sn==false){
    if(axis_sign(in_stream,true)==-1){
      errcode=0x00000100;
    }
    else{
      if(op_code==0x10){
        if(axis_sign(out_stream,false)==-1){
          errcode=0x00000200;
        };
      };
    };
  };
  if(errcode==0){
    while(1){
      length=fread(buffer,1,_LE_BUFFER_SIZE,in_stream);
#ifdef EINTR
      if(length<0 && errno==EINTR) continue;
#endif
      if(ferror(in_stream)) length=-1;
      if(length<=0) break;
      write_flt(out_stream,(char *)buffer,(uint32_t)length);
      if(errcode) break;
    };
    write_sync_flt(out_stream);
  };
  fclose(in_stream);
  eof=1;
  if((errcode==0)&&(length==-1)) errcode=0x00000004;
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
