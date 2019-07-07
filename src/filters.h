#ifndef _FILTERS_H
#define _FILTERS_H 1

#include <stdio.h>
#include "crypt.h"
#include "ecc.h"

#ifndef _BUFFER_SIZE
  #define _BUFFER_SIZE 32768
#endif
#define _LE_BUFFER_SIZE 65536

  /*
    Базовый класс для обработки потока "Axis"(XS)
  */

  class filters{
    private:
      cryptxs *transcoder;
      unsigned char op_code;
      void buffer_transcode(unsigned char *buffer, uint32_t &lenght);
      int32_t iolenght;
      unsigned char *iobuffer;
    public:
      bool ignore_sn;
      bool ignore_rc;
      alfl *slag;
      ecc corrector;
      uint32_t *cycles;
      uint32_t errcode;
      void set_correct();
      void set_check();
      void set_decrypt();
      void set(char *p);
      void write_flt(int filedsc, char *buf, int32_t lenght);
      int32_t read_flt(int filedsc, char *buf, int32_t lenght);
      void write_sync_flt(int filedsc);
      void read_sync_flt(int filedsc);
      void reset();
      void transcode(const char *ifile, const char *ofile);
      filters();
      ~filters();
  };

#endif
