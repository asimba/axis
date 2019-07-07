#ifndef _FTREE_H
#define _FTREE_H 1

#include "bnode.h"

  #ifndef MAX_PATH
    #define MAX_PATH FILENAME_MAX
  #endif

  /*
    Вспомогательный класс для получения информации о содержимом директории ("Acheron")
  */


  class tnodes{
    public:
      bool memory_fail;
      bool access_fail;
      char *base;
      bnode *node;
      tnodes *next;
      tnodes();
      tnodes(const char *base_name);
      ~tnodes();
  };

  /*
    Базовый класс для получения информации о содержимом директории ("Acheron")
  */

  class ftree{
    private:
      tnodes *tree;
      tnodes *current;
      tnodes *forward;
      off64_t data_size;
      uint32_t header_size;
      void zero_tree();
      void free_tree();
    public:
      bool memory_fail;
      bool access_fail;
      void addnode(const char *base_name);
      void rebuild();
      void relatives();
      bool get_next_relative(char **path, infonode &i);
      char *get_next_content(long &l);
      void load_next_content(long l, char *content);
      off64_t get_data_size();
      uint32_t get_header_size();
      uint32_t get_dirs();
      uint32_t get_files();
      void release();
      ftree();
      ~ftree();
  };

#endif

