#ifndef _FNODE_H
#define _FNODE_H 1

#ifdef _WIN32
  #include <wchar.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "convert.h"
#include "etc.h"

  /*
    Объявление структуры для хранения дополнительной информации об объекте файловой системы
  */

  struct nstat{
    uint32_t attributes;
    uint32_t atime;
    uint32_t mtime;
    off64_t  size;
    uint32_t uids;
    uint32_t gids;
  };

  #define NSTAT_SIZE 28

  /*
    Объявление структуры для хранения полной информации об объекте файловой системы (элементе очереди)
  */

  struct node{
    char *name;
    struct nstat *info;
    struct node *next;
  };

  /*
    Базовый класс для получения информации о содержимом директории ("Acheron")
  */

  class fnode{
    private:
      struct node *nodes;
      struct node *t_nodes;
      void free_nodes();
      uint32_t length;
      void sort_nodes();
    public:
      bool memory_fail;
      bool access_fail;
      void scannode(const char *base_name);
      void scandir(const char *base_name);
      bool set_next();
      char *get_node_name();
      struct nstat *get_node_info();
      void operator >> (uint32_t &l){
        l=length;
      };
      fnode();
      fnode(const char *base_name);
      ~fnode();
  };

#endif

