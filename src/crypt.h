#ifndef _CRYPT_RAND_H
#define _CRYPT_RAND_H 1

#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "convert.h"
#include "permutation.h"
#include "vars.h"
#ifndef _WIN32
  #include <sys/mman.h>
#endif

  /*
    Класс реализация алгоритма Simple Stream Cipher "Styx"(SX)
  */

  class cryptxs{
    private:
      alfg *state[6];
      unsigned char cpassword[20];
      permutation *pm;
      uint32_t lenght;
      bool operation;
      inline void noise(unsigned char *i);
      inline void denoise(unsigned char *i);
      inline void transcoder(unsigned char *i);
      void buffer_transcoder(unsigned char *i);
    public:
      alfl *slag;
      uint32_t *cycles;
      void set(char *p, bool decode);
      void operator << (uint32_t buffer_lenght){
        if(slag) lenght=buffer_lenght;
      };
      void operator << (unsigned char *i){
        if(slag) buffer_transcoder(i);
      };
      cryptxs();
      ~cryptxs();
  };

#endif
