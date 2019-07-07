#include "convert.h"

void long2char(char **o, uint32_t l){
  *((*o)++)=l>>24;
  *((*o)++)=l>>16;
  *((*o)++)=l>>8;
  *((*o)++)=l;
}

void off64t2char(char **o, off64_t l){
  *((*o)++)=l>>56;
  *((*o)++)=l>>48;
  *((*o)++)=l>>40;
  *((*o)++)=l>>32;
  *((*o)++)=l>>24;
  *((*o)++)=l>>16;
  *((*o)++)=l>>8;
  *((*o)++)=l;
}

void long2char_s(unsigned char *o, uint32_t l){
  *(o++)=l>>24;
  *(o++)=l>>16;
  *(o++)=l>>8;
  *(o++)=l;
}

void off64t2char_s(unsigned char *o, off64_t l){
  *(o++)=l>>56;
  *(o++)=l>>48;
  *(o++)=l>>40;
  *(o++)=l>>32;
  *(o++)=l>>24;
  *(o++)=l>>16;
  *(o++)=l>>8;
  *(o++)=l;
}

void char2long(char **i, uint32_t &l){
  l^=l;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
}

void char2off64t(char **i, off64_t &l){
  l^=l;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
  l=l<<8;
  l|=(unsigned char)(*((*i)++));
}

void char2long_s(unsigned char *i, uint32_t &l){
  l^=l;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
}

void char2off64t_s(unsigned char *i, off64_t &l){
  l^=l;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
  l=l<<8;
  l|=*(i++);
}
