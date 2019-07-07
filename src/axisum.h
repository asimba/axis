#ifndef _AXISUM_H
#define _AXISUM_H 1

#ifdef _WIN32
  #include <wchar.h>
#endif

#include "hash.h"
#include "etc.h"
#include <stdio.h>
#include <unistd.h>


  struct checksum_record{
    uint32_t *hash;
    char *filename;
  };

  typedef void (*checksum_operator)(const struct checksum_record *v);

  void checksum_print(const struct checksum_record *v);
  uint32_t axisum(alfl *alfl_lag, const char *filename, checksum_operator f);
  uint32_t axisum(scxh *hash, const char *filename, checksum_operator f);

#endif
