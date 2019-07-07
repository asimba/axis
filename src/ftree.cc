#include "ftree.h"

/***********************************************************************************************************/
//Базовый класс для получения информации о содержимом директории ("Acheron")
/***********************************************************************************************************/

tnodes::tnodes(){ //базовый конструктор класса
  memory_fail=false;
  access_fail=false;
  base=NULL;
  base=strdup(". ");
  node=NULL;
  next=NULL;
  if(base){
    base[1]=0;
    node=new bnode;
  };
  if(node) memory_fail=node->memory_fail;
  else memory_fail=true;
}

tnodes::tnodes(const char *base_name){ //конструктор класса с возможностью указания доп. параметров
  memory_fail=false;
  base=NULL;
  base=strdup(base_name);
  node=NULL;
  next=NULL;
  if(base){
    if(base[strlen(base)-1]=='/') base[strlen(base)-1]=0;
    node=new bnode(base_name);
  };
  if(node==NULL){
    memory_fail=true;
    memset(base,0,strlen(base));
    free(base);
    base=NULL;
  }
  else{
    memory_fail=node->memory_fail;
    access_fail=node->access_fail;
    if(memory_fail||access_fail){
      delete node;
      node=NULL;
      memset(base,0,strlen(base));
      free(base);
      base=NULL;
    };
  };
}

tnodes::~tnodes(){
  memory_fail=false;
  access_fail=false;
  if(node) delete node;
  if(base){
    memset(base,0,strlen(base));
    free(base);
  };
  base=NULL;
  node=NULL;
  next=NULL;
}

ftree::ftree(){ //базовый конструктор класса
  data_size=0L;
  header_size=0;
  zero_tree();
}

void ftree::zero_tree(){ //функция сброса основных параметров объекта
  tree=NULL;
  current=NULL;
  forward=NULL;
  memory_fail=false;
  access_fail=false;
}

void ftree::free_tree(){ //функция удаления полной (рекурсивной) очереди описания директории
  if(tree){
    while(tree){
      current=tree->next;
      delete tree;
      tree=current;
    };
    data_size=0L;
    header_size=0;
  };
}

void ftree::addnode(const char *base_name){ //функция добавления в начальный узел нового элемента
  if(memory_fail||access_fail) return;
  if(current!=tree){
    free_tree();
    zero_tree();
    addnode(base_name);
  }
  else{
    if(!tree) tree=new tnodes;
    if(!tree){
      memory_fail=true;
      return;
    };
    tree->node->addnode(base_name);
    memory_fail=tree->memory_fail;
    access_fail=tree->access_fail;
    if(memory_fail||access_fail){
      free_tree();
      return;
    };
    tree->base[0]=0;
    current=tree;
    forward=tree;
  };
}

void ftree::rebuild(){ //функция построения полного (рекурсивного) описания ничальных элементов (файлов)
  if(memory_fail||access_fail) return;
  if(!current) return;
  char *path=NULL;
  infonode info;
  while(current){
    while(current->node->get_next(info)){
      if(S_ISDIR(info.info.attributes)){
        path=(char *)calloc(strlen(current->base)+strlen(info.name)+2,sizeof(char));
        if(!path){
          memory_fail=true;
          free_tree();
          return;
        };
        strcpy(path,current->base);
        if(path[0]) path[strlen(path)]='/';
        strcat(path,info.name);
        forward->next=new tnodes(path);
        free(path);
        forward=forward->next;
        if(!forward){
          memory_fail=true;
          free_tree();
          return;
        };
        memory_fail=forward->memory_fail;
        access_fail=forward->access_fail;
        if(memory_fail||access_fail){
          free_tree();
          return;
        };
      };
    };
    header_size+=current->node->get_lenght();
    current=current->next;
  }
  current=tree;
  forward=tree;
}

void ftree::relatives(){ //функция формирования базовых имён узлов очереди описания
  if(memory_fail||access_fail) return;
  if(!tree) return;
  char *path=NULL;
  infonode info;
  current=tree;
  forward=tree;
  while(current){
    while(current->node->get_next(info)){
      if(S_ISDIR(info.info.attributes)){
        if((strlen(info.name)>MAX_PATH)||(strlen(info.name)==0)){
          free_tree();
          return;
        };
        path=(char *)calloc(strlen(current->base)+strlen(info.name)+2,sizeof(char));
        if(path==NULL){
          memory_fail=true;
          free_tree();
          return;
        };
        strcpy(path,current->base);
        if(path[0]&&(info.name[0]!='/')) path[strlen(path)]='/';
        strcat(path,info.name);
        forward=forward->next;
        memset(forward->base,0,strlen(forward->base));
        free(forward->base);
        forward->base=path;
      }
      else{
        data_size+=info.info.size;
      };
    };
    current=current->next;
  }
  current=tree;
}

off64_t ftree::get_data_size(){ //функция получения общей длины архивируемых файлов
  return data_size;
}

uint32_t ftree::get_header_size(){ //функция получения общей длины заголовка архива
  return header_size;
}

ftree::~ftree(){ //деструктор класса
  release();
}

bool ftree::get_next_relative(char **path, infonode &i){ //функция получения полной информации о файле
  if(memory_fail||access_fail) return false;
  if(current==NULL){
    current=tree;
    return false;
  };
  if(current==forward){
    if(current->node->get_next(i)){
      *path=current->base;
      return true;
    }
    else{
      current=tree;
      return false;
    };
  }
  else{
    while((current->node->get_next(i))==false){
      current=current->next;
      if(!current){
        current=tree;
        return false;
      };
    };
  };
  *path=current->base;
  return true;
}

char *ftree::get_next_content(long &l){ //функция получения информации об области памяти хранящей элемент
  char *ret=NULL;
  l=0;
  if(!current) current=tree;
  else{
    ret=current->node->get_content();
    l=current->node->get_lenght();
    current=current->next;
  };
  return ret;
}

void ftree::load_next_content(long l, char *content){ //функция загрузки элемента очереди извне
  if(forward){
    forward->next=new tnodes;
    forward=forward->next;
  }
  else{
    tree=new tnodes;
    current=tree;
    forward=tree;
  };
  if(forward) memory_fail=forward->memory_fail;
  else memory_fail=true;
  if(memory_fail){
    free_tree();
    return;
  };
  forward->node->load_content((uint32_t)l,content);
  memory_fail=forward->memory_fail;
  if(memory_fail){
    free_tree();
    return;
  };
}

void ftree::release(){ //функция полного сброса параметров объекта
  free_tree();
  zero_tree();
}

/***********************************************************************************************************/

