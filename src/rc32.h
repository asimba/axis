#ifndef _RC32_H
#define _RC32_H 1

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <fcntl.h>
#include "filters.h"

#define _RC32_BUFFER_SIZE 32768

  /*
    Базовый класс для упаковки/распаковки потока по упрощённому алгоритму Range Coder 32-bit
  */

  class rc32{
    private:
      uint32_t low;
      uint32_t hlp;
      uint32_t range;
      bool init;
      uint8_t *pqbuffer;
      uint32_t bufsize;
      uint32_t rbufsize;
    public:
      uint32_t *frequency;
      bool eof;
      bool finalize;
      filters *flt;
      int32_t rc32_read(FILE* file, char *buf, int32_t lenght);
      int32_t rc32_write(FILE* file, char *buf, int32_t lenght);
      rc32();
      ~rc32();
  };

#endif
