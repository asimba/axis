#ifndef _RC32_H
#define _RC32_H 1

#include "filters.h"
#include "buffers.h"

#define _RC32_BUFFER_SIZE 32768

  /*
    Базовый класс для упаковки/распаковки потока по упрощённому алгоритму Range Coder 32-bit
  */

  class rc32{
    private:
      uint32_t low;
      uint32_t hlp;
      uint32_t range;
      uint8_t waiting;
      buffers ibuffer;
      buffers obuffer;
      bool eof;
      bool op_code;
      bool init;
      filters *xs;
      uint8_t *pqbuffer;
      void initialize();
      inline void rescale();
      inline void encode();
      inline void decode();
      void finalize_encode();
      void initialize_decode();
    public:
      uint32_t *frequency;
      bool finalize;
      bool decoding_error;
      void set(filters *flt, bool pack);
      int32_t rc32_read(int filedsc, char *buf, int32_t lenght);
      int32_t rc32_write(int filedsc, char *buf, int32_t lenght);
      bool is_eof();
      rc32();
      ~rc32();
  };

#endif
