#ifndef _ALFG_RANDOM_H
#define _ALFG_RANDOM_H 1

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef _WIN32
  #include <pthread.h>
#else
  #include <windows.h>
#endif

  /*
    Параметры, определяющие глубину памяти генератора псевдослучайных последовательностей.
  */
  #define K_LAG 4187
  #define L_LAG 9689

  /*
    Класс для хранения предвычисленных значений смещений ячеек памяти генератора.
  */

  class alfl{
    public:
      uint16_t *lag;
      alfl();
      ~alfl();
  };

#ifndef _WIN32
      void *th_alfg_init(void *in);
#else
      DWORD *WINAPI th_alfg_init(LPVOID *in);
#endif
  /*
    Базовый класс генератора псевдослучайных последовательностей.
  */

  class alfg{
    private:
      uint32_t *buf;
      uint32_t index;
      void alfg_reset();
      void alfg_init(uint32_t i_seed);
      inline void alfg_rand(){ //вычисление следующего значения генератора
        buf[index]+=buf[lags[index]];
        if(index) index--;
        else index=L_LAG-1;
      };
    public:
      uint16_t *lags;
      uint32_t *cycles;
      uint32_t operator ++ (){
        alfg_rand();
        return buf[index];
      };
      uint32_t operator ++ (int notused){
        alfg_rand();
        return buf[index];
      };
      uint32_t operator >> (uint32_t &v){
        v=buf[index];
        return v;
      };
      uint32_t operator << (uint32_t v){
        for(;;){
          alfg_rand();
          if(v==0) break;
          v--;
        };
        return buf[index];
      };
      uint32_t operator << (unsigned char v){
        for(;;){
          alfg_rand();
          if(v==0) break;
          v--;
        };
        return buf[index];
      };
      uint32_t operator = (uint32_t v){
        alfg_init(v);
        return buf[index];
      };
      void operator = (alfg *v){
        if(lags){
          memcpy(buf,v->get_buf(),L_LAG*sizeof(uint32_t));
          index=v->get_index();
          if(cycles) (*cycles)=*v->cycles;
        }
        else return;
      };
      uint32_t *get_buf();
      void set_index(uint32_t v);
      uint32_t get_index();
      alfg(alfl *alfl_lag);
      ~alfg();
  };

  struct th_alfg_helper{
    alfg *state;
    uint32_t  v;
  };

#endif
