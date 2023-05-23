/***********************************************************************************************************/
//Базовая реализация упаковки/распаковки файла по упрощённому алгоритму LZSS+RLE+RC32
/***********************************************************************************************************/
#ifndef _PACKER_H
#define _PACKER_H 1

  #include <cstdint>
  #include <algorithm>
  #include <stdio.h>
  #include <fcntl.h>
  #include "filters.h"

  using namespace std;

  #define LZ_BUF_SIZE 259
  #define LZ_CAPACITY 24
  #define LZ_MIN_MATCH 3

  #define _RC32_BUFFER_SIZE 0x10000

  typedef union{
    struct{
      uint16_t in;
      uint16_t out;
    };
    uint32_t val;
  } vocpntr;

  class packer{
    private:
      uint8_t *iobuf;
      uint8_t *cbuffer;
      uint8_t *vocbuf;
      uint8_t *lowp;
      uint8_t *hlpp;
      uint8_t *cpos;
      uint8_t cstate;
      uint8_t rle_flag;
      uint16_t *vocarea;
      uint16_t *hashes;
      uint16_t buf_size;
      uint16_t voclast;
      uint16_t vocroot;
      uint16_t offset;
      uint16_t length;
      uint16_t symbol;
      uint16_t *fcs;
      uint16_t **frequency;
      int32_t icbuf;
      uint32_t wpos;
      uint32_t rpos;
      uint32_t low;
      uint32_t hlp;
      uint32_t range;
      vocpntr *vocindx;
      filters *flt;
      template <class T,class V> void del(T& p,uint32_t s,V v);
      void wbuf(void* file, uint8_t c);
      bool rbuf(void* file, uint8_t *c);
      bool rc32_getc(void* file, uint8_t *c);
      bool rc32_putc(void* file, uint8_t c);
    public:
      uint8_t flags;
      bool finalize;
      void init();
      void set_filters(filters *f);
      bool load_header(void* file);
      bool packer_putc(void* file, uint8_t c);
      bool packer_getc(void* file, uint8_t *c);
      int32_t packer_read(void* file, char *buf, int32_t l);
      int32_t packer_write(void* file, char *buf, int32_t l);
      packer();
      ~packer();
  };

#endif
