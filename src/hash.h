#ifndef _SDX_HASH
#define _SDX_HASH 1

#include "random.h"
#include "convert.h"

#ifdef HAVE_HASH_TBL
  #include <string.h>
  #include "hashtbl.h"
#endif

  /*
    Базовый класс вычисления контрольной суммы ключа.
  */
  class scxh{
    private:
      uint32_t lenght;
      bool init;
      void scxh_update(unsigned char *i);
      bool self_check();
      void reset(bool use_hash_tbl);
    public:
      uint32_t hash[5];
      alfg *state;
      void operator << (uint32_t v){
        if(state) lenght=v;
      };
      void operator << (unsigned char *v){
        if(state) scxh_update(v);
      };
      void clear();
      scxh(alfg *alfgh);
      ~scxh();
  };

#endif
