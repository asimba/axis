#ifndef _FILTERS_H
#define _FILTERS_H 1

#include <stdio.h>
#include "crypt.h"
#include "ecc.h"

#ifndef _BUFFER_SIZE
  #define _BUFFER_SIZE 0x10000
#endif
#define _LE_BUFFER_SIZE 65536

  /*
    Базовый класс для обработки потока "Axis"(XS)
  */

  class filters{
    private:
      cryptxs *transcoder;
      unsigned char op_code;
      void buffer_transcode(unsigned char *buffer, uint32_t &length);
      int32_t iolength;
      unsigned char *iobuffer;
      int32_t eof;
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
      int32_t write_flt(FILE *filedsc, char *buf, int32_t length);
      int32_t read_flt(FILE *filedsc, char *buf, int32_t length);
      void write_sync_flt(FILE *filedsc);
      void read_sync_flt(FILE *filedsc);
      int32_t is_eof(FILE *filedsc);
      void reset();
      void transcode(const char *ifile, const char *ofile);
      filters();
      ~filters();
  };

#endif
