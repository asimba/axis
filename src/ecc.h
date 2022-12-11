#ifndef _LETHE_ECC
#define _LETHE_ECC 1

#include <stdint.h>
#include <stdio.h>

  /*
    Класс реализация алгоритма коррекции ошибок (ECC)
  */

  class ecc{
    private:
      bool op_code;
      unsigned char syndrome;
      inline void encode(unsigned char *i, unsigned char *o);
      inline void decode(unsigned char *i, unsigned char *o);
    public:
      uint32_t softerr;
      uint32_t harderr;
      uint32_t buffer_transcoder(unsigned char *iobuffer, uint32_t length);
      void set_decode();
      void reset();
      ecc();
      ~ecc();
  };

#endif
