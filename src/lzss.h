#ifndef _LZSS_H
#define _LZSS_H 1

#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

//#include "iobits.h"
#include "rc32.h"
#include "nvoc.h"

  #define LZ_BUF_SIZE 258
  #define LZ_VOC_SIZE 65536
  #define LZ_LENGHT_CAPACITY 8
  #define LZ_OFFSET_CAPACITY 16
  #define LZ_MIN_MATCH 3

  /*
    Базовый класс для упаковки/распаковки потока по упрощённому алгоритму LZSS
  */

  class codebuf{
    public:
      uint8_t flags;
      uint8_t flags_count;
      uint8_t *buffer;
      uint8_t buffer_position;
      uint8_t buffer_size;
      void write(uint8_t flag, uint32_t lenght, uint32_t offset);
      void read(uint8_t &flag, uint32_t &lenght, uint32_t &offset);
      void reset();
      uint8_t flags_buffer_size();
      codebuf();
      ~codebuf();
  };

  class lzss{
    private:
      uint16_t buf_size;
      rc32 pack;
      nvoc *voc;
      codebuf *cbuf;
      bool op_code;
    public:
      uint8_t *lzbuf;
      bool decoding_error;
      bool finalize;
      void set(filters *flt, bool mode);
      void lzss_write(int filedsc, char *buf, int32_t ln);
      int32_t lzss_read(int filedsc, char *buf, int32_t ln);
      int is_eof(int filedsc);
      lzss();
      ~lzss();
  };

#endif
