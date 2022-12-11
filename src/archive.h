#ifndef _ARCHIVE_H
#define _ARCHIVE_H 1

#include <unistd.h>
#ifndef __STDC_FORMAT_MACROS
  #define __STDC_FORMAT_MACROS 1
#endif
#include <inttypes.h>
#include <sys/time.h>
#include <libgen.h>
#include "filters.h"
#include "ftree.h"
#include "canonicalize.h"
#include "mdirs.h"
#include "signature.h"
#include "packer.h"
#include "axisum.h"
#include "etc.h"
#include "timebits.h"

  struct list_record{
    struct infonode *info;
    char *full_name;
    uint32_t *errcode;
  };

  struct list_summary_record{
    uint32_t dirs;
    uint32_t files;
    off64_t size;
  };

  typedef void (*list_operator)(const struct list_record *v);

  typedef void (*list_summary_operator)(const struct list_summary_record *v);

  void list_print(const struct list_record *v);
  void list_summary_print(const struct list_summary_record *v);

  /*
    Основной класс для осуществления архивирования файлов ("archiver")
  */

  class archiver{
    private:
      ftree *tree;
      char *base;
      void read_header(FILE *filedsc);
      checksum_operator c_f;
      list_operator l_f;
      list_summary_operator ls_f;
    public:
      bool ignore_pt;
      filters flt;
      packer pack;
      uint32_t errcode;
      uint32_t header_size;
      uint64_t data_size;
      uint64_t rwl;
      void add_name(const char *name);
      void set_base(const char *name);
      void set_list_operator(list_operator f);
      void set_list_summary_operator(list_summary_operator f);
      void archive(const char *filename, bool unarc, uint8_t unarc_null);
      void set_checksum_operator(checksum_operator f);
      void checksums();
      archiver();
      ~archiver();
  };

#endif
