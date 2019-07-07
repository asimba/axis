#include "random.h"

/***********************************************************************************************************/
//"Additive Lagged-Fibonacci Generator"
/***********************************************************************************************************/

alfl::alfl(){ //конструктор класса (выделение памяти и инициализация массива смещений)
  lag=(unsigned short *)calloc(L_LAG,sizeof(unsigned short));
  if(lag) for(unsigned short l=0; l<L_LAG; l++) lag[l]=(l+K_LAG)%L_LAG;
}

alfl::~alfl(){ //деструктор класса (обнуление массива смещений и высвобождение памяти)
  if(lag){
    memset(lag,0,L_LAG);
    free(lag);
    lag=NULL;
  };
}

void alfg::alfg_reset(){ //сброс параметров генератора и выделение основной памяти
  cycles=NULL;
  index=0;
  if(lags){
    if(buf) memset(buf,0,L_LAG);
    else buf=(uint32_t *)calloc(L_LAG,sizeof(uint32_t));
    if(buf) return;
    lags=NULL;
  };
}

void alfg::alfg_init(uint32_t i_seed){ //инициализация генератора некоторым начальным числом
  if(lags){
    unsigned long c;
    if(i_seed!=0xaaaaaaaa) i_seed^=0xaaaaaaaa;
    for(index=0; index<L_LAG; index++){
      i_seed=((i_seed*22695477+1)>>16)&0x00007fff;
      buf[index]=i_seed;
    };
    if(cycles) (*cycles)+=L_LAG;
    for(c=0; c<18; c++){
      for(index=0; index<L_LAG; index++){
        i_seed=((i_seed*22695477+1)>>16)&0x00007fff;
        buf[index]^=(uint32_t)(i_seed<<c);
      };
    };
    if(cycles) (*cycles)+=(L_LAG*18);
    index=0;
    for(c=0; c<(L_LAG*L_LAG+1); c++){
      buf[index]^=buf[lags[index]];
      if(index) index--;
      else index=L_LAG-1;
    };
    if(cycles) (*cycles)+=(L_LAG*L_LAG+1);
    index=0;
    (*this)<<(uint32_t)(L_LAG*L_LAG*3);
    if(cycles) (*cycles)+=(L_LAG*L_LAG*3+1);
  };
}

#ifndef _WIN32
void *th_alfg_init(void *in){
#else
DWORD *WINAPI th_alfg_init(LPVOID *in){
#endif
  *(((th_alfg_helper *)in)->state)=((th_alfg_helper *)in)->v;
#ifndef _WIN32
  pthread_exit(NULL);
#else
  return 0;
#endif
}

uint32_t *alfg::get_buf(){
  return buf;
}

void alfg::set_index(uint32_t v){
  index=v;
}

uint32_t alfg::get_index(){
  return index;
}

alfg::alfg(alfl *alfl_lag){ //конструктор с возможностью указания внешней дополнительной памяти
  buf=NULL;
  lags=NULL;
  index=0;
  if((alfl_lag)&&(alfl_lag->lag)) lags=alfl_lag->lag;
  alfg_reset();
}

alfg::~alfg(){ //деструктор класса (обнуление и высвобождение памяти)
  if(buf){
    memset(buf,0,L_LAG);
    free(buf);
    buf=NULL;
  };
  lags=NULL;
  index=0;
  cycles=NULL;
}

/***********************************************************************************************************/
