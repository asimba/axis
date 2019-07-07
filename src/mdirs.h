#ifndef _MDIRS_H
#define _MDIRS_H 1

#include <unistd.h>
#include <sys/time.h>
#include "bnode.h"

  uint32_t mdirs(const char *fname, struct infonode &info, bool ipt);

#endif
