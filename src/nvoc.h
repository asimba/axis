#ifndef __NVOC_H
#define __NVOC_H 1

#include <stdint.h>
#include <string.h>
#include <malloc.h>

  struct vocnode{
    uint8_t value;
    uint32_t index;
    struct vocnode *next;
    struct vocnode *relative;
  };

  struct vocpntr{
    struct vocnode *in;
    struct vocnode *out;
  };

  class nvoc{
    private:
      struct vocnode *vocarea;
      struct vocnode *vocroot;
      struct vocnode *vocstop;
      struct vocnode *voclast;
      struct vocpntr *vocindx;
    public:
      uint32_t vocsize;
      uint32_t offset;
      uint32_t lenght;
      void search(uint8_t *str, uint32_t size);
      void write(uint8_t *str, uint32_t size);
      void write_woupdate(uint8_t *str, uint16_t size);
      int read(uint8_t *str, uint32_t index, uint32_t size);
      nvoc(uint32_t size);
      ~nvoc();
  };

#endif
