#ifndef PERMUTATION_H
#define PERMUTATION_H 1

#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

  class permutation{
    private:
      unsigned char *in;
      int32_t n;
      int32_t m;
      int32_t *z;
      int32_t *p;
      int32_t *d;
      void set();
    public:
      void next();
      permutation(unsigned char *src, int32_t length);
      ~permutation();
  };

#endif
