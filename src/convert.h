#ifndef _CONVERT_H
#define _CONVERT_H 1

#include <sys/types.h>
#include <stdint.h>

  void long2char(char **o, uint32_t l);
  void long2char_s(unsigned char *o, uint32_t l);
  void char2long(char **i, uint32_t &l);
  void char2long_s(unsigned char *i, uint32_t &l);
  void off64t2char(char **o, off64_t l);
  void off64t2char_s(unsigned char *o, off64_t l);
  void char2off64t(char **i, off64_t &l);
  void char2off64t_s(unsigned char *i, off64_t &l);

#endif
