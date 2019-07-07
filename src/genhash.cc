#include "hash.h"
#include "etc.h"
#include "convert.h"
#include <unistd.h>
#include <getopt.h>

#define _BUFFER_SIZE 32768

int main(){
  alfl *lags=new alfl;
  if(lags){
    alfg *state=new alfg(lags);
    alfg *state_crc=new alfg(lags);
    if(state&&state_crc){
      if(state->lags!=NULL){
        scxh *hash=new scxh(state);
        if(hash!=NULL){
          scxh *hash_crc=new scxh(state_crc);
          if(hash_crc!=NULL){
            uint32_t *ht=hash->state->get_buf();
            unsigned char chash[4];
            if(ht){
              fprintf(stdout,"#ifndef _SDX_HASH_TBL\n#define _SDX_HASH_TBL 1\n");
              fprintf(stdout,"static const uint32_t sdx_hash_tbl[]={\n");
              uint8_t c=0;
              for(uint32_t i=0; i<L_LAG; i++){
                if(c==5){
                  c=0;
                  fprintf(stdout,"\n");
                };
                fprintf(stdout,"0x%08x,",ht[i]);
                long2char_s(chash,ht[i]);
                (*hash_crc)<<(uint32_t)sizeof(uint32_t);
                (*hash_crc)<<(unsigned char *)chash;
                c++;
              };
              fprintf(stdout,"0x%08x,\n",hash->state->get_index());
              long2char_s(chash,hash->state->get_index());
              (*hash_crc)<<(uint32_t)sizeof(uint32_t);
              (*hash_crc)<<(unsigned char *)chash;
              for(c=0; c<4; c++){
                fprintf(stdout,"0x%08x,",hash_crc->hash[c]);
              };
              fprintf(stdout,"0x%08x\n};\n#endif\n",hash_crc->hash[c]);
            };
            delete hash_crc;  
          };
          delete hash;
        };
      };
      delete state;
      if(state_crc) delete state_crc;
    };
    delete lags;
  };
  return 0;
}
