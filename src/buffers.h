#ifndef __BUFFERS_H
#define __BUFFERS_H 1

#include <stdint.h>
#include <malloc.h>

  /*
    Вспомогательный класс для упаковки/распаковки потока по упрощённому алгоритму Range Coder 32-bit
  */

  class buffers{
    public:
      uint8_t *content;
      uint32_t size;
      uint32_t state;
      bool locked;
      void operator << (uint8_t c){
        if(locked) return;
        else{
          content[state++]=c;
          if(state==size) locked=true;
        };
      };
      void operator >> (uint8_t &c){
        if(locked) return;
        else{
          c=content[state++];
          if(state==size){
            locked=true;
            state=0;
          };
        };
      };
      buffers();
      ~buffers();
  };

#endif

