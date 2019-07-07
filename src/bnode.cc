#include "bnode.h"

/***********************************************************************************************************/
//Вспомогательный класс для получения информации о содержимом директории ("Acheron")
/***********************************************************************************************************/

void bnode::reset(){ //функция сброса основных параметров объекта класса
  content=NULL;
  lenght=0;
  data_size=0;
  position=NULL;
  memory_fail=false;
  access_fail=false;
}

bnode::bnode(){ //базовый конструктор класса
  reset();
}

bnode::bnode(const char *base_name){ //конструктор класса с возможностью указания доп. параметров
  reset();
  fcontent(base_name);
}

bnode::~bnode(){ //деструктор класса
  if(content) free(content);
  reset();
}

off64_t bnode::get_data_size(){ //функция получения общей длины файлов узла
  return data_size;
}

void bnode::addnode(const char *base_name){ //функция простого получения описания единичного файла
  fnode *ls;
  ls=new fnode;
  if(!ls){
    memory_fail=true;
    return;
  }
  else ls->scannode(base_name);
  memory_fail=ls->memory_fail;
  access_fail=ls->access_fail;
  if(memory_fail||access_fail){
    delete ls;
    return;
  };
  uint32_t new_lenght=0;
  (*ls)>>new_lenght;
  char *new_content=NULL;
  if(content){
    new_content=(char *)calloc(lenght+new_lenght,sizeof(char));
    if(new_content){
      memcpy(new_content,content,lenght);
      memset(content,0,lenght);
      free(content);
      content=new_content;
      long2char(&content,lenght+new_lenght);
      content=new_content;
      position=content+lenght;
      lenght+=new_lenght;
    }
    else{
      memory_fail=true;
      memset(content,0,lenght);
      free(content);
      delete ls;
      return;
    };
  }
  else{
    lenght=new_lenght+sizeof(uint32_t);
    content=(char *)calloc(lenght,sizeof(char));
    position=content;
    long2char(&position,lenght);
  };
  if(content){
    char *name;
    struct nstat *info;
    name=ls->get_node_name();
    info=ls->get_node_info();
    strcpy(position,name);
    position+=(strlen(name)+1);
    if(!S_ISDIR(info->attributes)) data_size+=info->size;
    long2char(&position,info->attributes);
    long2char(&position,info->atime);
    long2char(&position,info->mtime);
    off64t2char(&position,info->size);
    long2char(&position,info->uids);
    long2char(&position,info->gids);
    position=content;
  }
  else memory_fail=true;
  delete ls;
}

void bnode::fcontent(const char *base_name){ //функция построения очереди содержимого директории
  if(content){
    free(content);
    reset();
  };
  fnode *ls;
  ls=new fnode(base_name);
  if(!ls){
    memory_fail=true;
    return;
  };
  memory_fail=ls->memory_fail;
  access_fail=ls->access_fail;
  if(memory_fail||access_fail){
    delete ls;
    return;
  };
  (*ls)>>lenght;
  lenght+=sizeof(uint32_t);
  content=(char *)calloc(lenght,sizeof(char));
  position=content;
  if(content){
    if(lenght==sizeof(uint32_t)){
      long2char(&position,sizeof(uint32_t));
      position=content;
      delete ls;
      return;
    };
    char *name;
    struct nstat *info;
    long2char(&position,lenght);
    while(1){
      name=ls->get_node_name();
      info=ls->get_node_info();
      if((!name)||(!info)) break;
      strcpy(position,name);
      position+=(strlen(name)+1);
      if(!S_ISDIR(info->attributes)) data_size+=info->size;
      long2char(&position,info->attributes);
      long2char(&position,info->atime);
      long2char(&position,info->mtime);
      off64t2char(&position,info->size);
      long2char(&position,info->uids);
      long2char(&position,info->gids);
      if(!ls->set_next()) break;
    };
    position=content;
  }
  else memory_fail=true;
  delete ls;
}

char *bnode::get_content(){ //функция получения указателя на область памяти хранящей очередь
  return content;
}

uint32_t bnode::get_lenght(){ //функция получения физической длины области памяти хранящей очередь
  return lenght;
}

bool bnode::get_next(struct infonode &in){ //функция получения информации об элементе очереди
  if(lenght==sizeof(uint32_t)) return false;
  if((uint32_t)(position-content)>=lenght){
    position=content;
    return false;
  }
  if(position==content) position+=sizeof(uint32_t);
  in.name=position;
  position+=(strlen(position)+1);
  char2long(&position,in.info.attributes);
  char2long(&position,in.info.atime);
  char2long(&position,in.info.mtime);
  char2off64t(&position,in.info.size);
  char2long(&position,in.info.uids);
  char2long(&position,in.info.gids);
  return true;
}

void bnode::load_content(uint32_t l, char *b){ //функция загрузки очереди из внешнего источника
  if(content) free(content);
  content=(char *)calloc(l,sizeof(char));
  position=content;
  if(content){
    long2char(&position,l);
    lenght=l;
    memcpy(position,b,l-sizeof(uint32_t));
    position=content;
  }
  else memory_fail=true;
}

/***********************************************************************************************************/

