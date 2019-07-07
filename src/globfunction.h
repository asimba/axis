#ifndef __GLOBFUNCTION_H
#define __GLOBFUNCTION_H 1

#include <stdint.h>
#include <malloc.h>
#include <errno.h>

#ifndef _WIN32

  #include <glob.h>

#else

  #include <io.h>
  #include <string.h>
  #include <sys/types.h>
  #include "etc.h"

  typedef struct {
    size_t gl_pathc;
    size_t gl_offs;
    char **gl_pathv;
  } glob64_t;

#endif

  glob64_t *glob_getlist(const char *mask, int &errcode);
  void glob_freelist(glob64_t *globbuf);

#endif
