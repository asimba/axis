#ifndef _BNODE_H
#define _BNODE_H 1

#include <stdlib.h>
#include <stdint.h>
#include "fnode.h"
#include "convert.h"

  /*
    Объявление структуры для хранения полной информации об элементе директории ("Acheron")
  */

  struct infonode{
    char *name;
    struct nstat info;
  };

  /*
    Вспомогательный класс для получения информации о содержимом директории ("Acheron")
  */

  class bnode{
    private:
      char *content;
      uint32_t length;
      off64_t data_size;
      char *position;
      void fcontent(const char *base_name);
      void reset();
    public:
      bool memory_fail;
      bool access_fail;
      void addnode(const char *base_name);
      bool get_next(struct infonode &in);
      char *get_content();
      uint32_t get_length();
      off64_t get_data_size();
      void load_content(uint32_t l, char *b);
      bnode();
      bnode(const char *base_name);
      ~bnode();
  };

#endif

