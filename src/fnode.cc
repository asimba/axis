#include "fnode.h"

/***********************************************************************************************************/
//Базовый класс для получения информации о содержимом директории ("Acheron")
/***********************************************************************************************************/

fnode::fnode(){ //базовый конструктор класса
  t_nodes=NULL;
  nodes=NULL;
  memory_fail=false;
  access_fail=false;
  free_nodes();
}

void fnode::free_nodes(){ //функция сброса всех параметров и высвобождения очереди
  nodes=t_nodes;
  while(nodes!=NULL){
    t_nodes=nodes->next;
    if(nodes->name){
      memset(nodes->name,0,strlen(nodes->name));
      free(nodes->name);
      nodes->name=NULL;
    };
    if(nodes->info){
      memset(nodes->info,0,NSTAT_SIZE);
      free(nodes->info);
      nodes->info=NULL;
    };
    free(nodes);
    nodes=t_nodes;
  };
  nodes=NULL;
  t_nodes=NULL;
  length=0;
}

fnode::~fnode(){ //деструктор класса
  memory_fail=false;
  access_fail=false;
  free_nodes();
}

void fnode::scannode(const char *base_name){ //функция простого получения описания единичного файла
  if(memory_fail||access_fail) return;
  if(nodes) free_nodes();
  nodes=(struct node *)calloc(1, sizeof(struct node));
  if(nodes) t_nodes=nodes;
  else{
    memory_fail=true;
    return;
  };
  nodes->next=NULL;
#ifndef _WIN32
  struct stat64 t_stat;
#else
  struct _stati64 t_stat;
  wchar_t *wc_name=NULL;
#endif
  nodes->name=strdup(base_name);
  if(!nodes->name){
    memory_fail=true;
    free_nodes();
    return;
  };
  if(((nodes->name)[strlen(base_name)-1]=='/')&&(strlen(base_name)>1))
    (nodes->name)[strlen(base_name)-1]=0;
  nodes->info=(struct nstat *)calloc(1, sizeof(struct nstat));
  if(!nodes->info){
    memory_fail=true;
    free_nodes();
    return;
  };
#ifndef _WIN32
  if((stat64(nodes->name,&t_stat)<0)||(access(nodes->name,R_OK)<0))
#else
  wc_name=codepagetowchar(nodes->name,CP_UTF8);
  if(wc_name==NULL){
    memory_fail=true;
    free_nodes();
    return;
  };
  if((_wstati64(wc_name,&t_stat)<0)||(_waccess(wc_name,R_OK)<0))
#endif
  {
    access_fail=true;
#ifdef _WIN32
    memset(wc_name,0,wcslen(wc_name)*sizeof(wchar_t));
    free(wc_name);
    wc_name=NULL;
#endif
    free_nodes();
    return;
  }
  else{
#ifdef _WIN32
    memset(wc_name,0,wcslen(wc_name)*sizeof(wchar_t));
    free(wc_name);
    wc_name=NULL;
#else
    if(S_ISCHR(t_stat.st_mode)||
       S_ISBLK(t_stat.st_mode)||
       S_ISFIFO(t_stat.st_mode)){
      access_fail=true;
      free_nodes();
      return;
    };
#endif
    nodes->info->attributes=(uint32_t)t_stat.st_mode;
    nodes->info->atime=(uint32_t)t_stat.st_atime;
    nodes->info->mtime=(uint32_t)t_stat.st_mtime;
    nodes->info->size=t_stat.st_size;
    nodes->info->uids=(uint32_t)t_stat.st_uid;
    nodes->info->gids=(uint32_t)t_stat.st_gid;
  };
  length+=(strlen(nodes->name)+NSTAT_SIZE+1);
  nodes=t_nodes;
}

void fnode::scandir(const char *base_name){ //основная функция постороения очереди содержимого директории
  if(memory_fail||access_fail) return;
  if(nodes) free_nodes();
#ifndef _WIN32
  DIR *dp=NULL;
  struct dirent *dir=NULL;
#else
  _WDIR *dp=NULL;
  struct _wdirent *dir=NULL;
  char *utf8_name=NULL;
  wchar_t *wc_name=NULL;
#endif
#ifndef _WIN32
  struct stat64 t_stat;
#else
  struct _stati64 t_stat;
  wchar_t *w_base_name=codepagetowchar(base_name,CP_UTF8);
#endif
  struct node *p_node=NULL;
  char *t_name=NULL;
  int pnum;
  errno=0;
#ifndef _WIN32
  dp=opendir(base_name);
#else
  dp=_wopendir(w_base_name);
  if(w_base_name){
    memset(w_base_name,0,wcslen(w_base_name)*sizeof(wchar_t));
    free(w_base_name);
    w_base_name=NULL;
  };
#endif
  if(errno==ENOTDIR) return;
  if((errno==EACCES)||(errno==ENAMETOOLONG)||(errno==ENOENT)){
    access_fail=true;
    return;
  };
  if(dp){
    nodes=(struct node *)calloc(1, sizeof(struct node));
    if(nodes) t_nodes=nodes;
    else goto errmem;
    p_node=nodes;
#ifndef _WIN32
    while((dir=readdir(dp))){
#else
    while((dir=_wreaddir(dp))){
#endif
#ifndef _WIN32
      if(!(strcmp(dir->d_name,"."))) continue;
      if(!(strcmp(dir->d_name,".."))) continue;
      nodes->name=strdup(dir->d_name);
#else
      if(!(wcscmp(dir->d_name,L"."))) continue;
      if(!(wcscmp(dir->d_name,L".."))) continue;
      nodes->name=wchartocodepage(dir->d_name,CP_UTF8);
#endif
      if(!nodes->name) goto errmem;
      nodes->info=(struct nstat *)calloc(1, sizeof(struct nstat));
      if(!nodes->info) goto errmem;
      pnum=strlen(base_name);
#ifndef _WIN32
      t_name=(char *)calloc(pnum+strlen(dir->d_name)+2,sizeof(char));
#else
      utf8_name=wchartocodepage(dir->d_name,CP_UTF8);
      if(utf8_name==NULL) goto errmem;
      t_name=(char *)calloc(pnum+strlen(utf8_name)+2,sizeof(char));
#endif
      if(t_name==NULL) goto errmem;
      strcpy(t_name,base_name);
      if(t_name[pnum-1]!=0x2f){
        t_name[pnum]=0x2f;
        pnum++;
      };
#ifndef _WIN32
      strcpy(t_name+pnum,dir->d_name);
#else
      strcpy(t_name+pnum,utf8_name);
#endif
#ifndef _WIN32
      if((stat64(t_name,&t_stat)<0)||(access(t_name,R_OK)<0))
#else
      wc_name=codepagetowchar(t_name,CP_UTF8);
      if(wc_name==NULL) goto errmem;
      if((_wstati64(wc_name,&t_stat)<0)||(_waccess(wc_name,R_OK)<0))
#endif
      {
        access_fail=true;
#ifndef _WIN32
        closedir(dp);
#else
        memset(wc_name,0,wcslen(wc_name)*sizeof(wchar_t));
        free(wc_name);
        wc_name=NULL;
        _wclosedir(dp);
#endif
        free_nodes();
        memset(t_name,0,strlen(t_name));
        free(t_name);
        t_name=NULL;
        return;
      }
      else{
#ifdef _WIN32
        memset(wc_name,0,wcslen(wc_name)*sizeof(wchar_t));
        free(wc_name);
        wc_name=NULL;
#else
        if(S_ISCHR(t_stat.st_mode)||
           S_ISBLK(t_stat.st_mode)||
           S_ISFIFO(t_stat.st_mode)){
          access_fail=true;
          closedir(dp);
          free_nodes();
          memset(t_name,0,strlen(t_name));
          free(t_name);
          t_name=NULL;
          return;
        };
#endif
        nodes->info->attributes=(uint32_t)t_stat.st_mode;
        nodes->info->atime=(uint32_t)t_stat.st_atime;
        nodes->info->mtime=(uint32_t)t_stat.st_mtime;
        nodes->info->size=t_stat.st_size;
        nodes->info->uids=(uint32_t)t_stat.st_uid;
        nodes->info->gids=(uint32_t)t_stat.st_gid;
      };
      memset(t_name,0,strlen(t_name));
      free(t_name);
      t_name=NULL;
#ifndef _WIN32
      length+=(strlen(dir->d_name)+NSTAT_SIZE+1);
#else
      length+=(strlen(utf8_name)+NSTAT_SIZE+1);
#endif
      nodes->next=(struct node *)calloc(1, sizeof(struct node));
      if(nodes->next) p_node=nodes;
      else goto errmem;
      nodes=nodes->next;
    };
    if(p_node!=nodes) p_node->next=NULL;
    free(nodes);
#ifndef _WIN32
    closedir(dp);
#else
    _wclosedir(dp);
#endif
    if(length) nodes=t_nodes;
    else{
      nodes=NULL;
      t_nodes=NULL;
    };
    sort_nodes();
  }
  return;
errmem:
  memory_fail=true;
#ifndef _WIN32
  closedir(dp);
#else
  _wclosedir(dp);
#endif
  free_nodes();
}

fnode::fnode(const char *base_name){ //конструктор класса с возможностью указания доп. параметров
  t_nodes=NULL;
  nodes=NULL;
  memory_fail=false;
  access_fail=false;
  free_nodes();
  this->scandir(base_name);
}

bool fnode::set_next(){ //функция установки указателя чтения/записи на следующий элемент очереди
  if(nodes){
    nodes=nodes->next;
    return true;
  }
  else{
   nodes=t_nodes;
   return false;
  };
}

char *fnode::get_node_name(){ //функция получения указателя на имя элемента (узла) очереди
  if(nodes) return nodes->name;
  return NULL;
}

struct nstat *fnode::get_node_info(){ //функция получения указателя на доп. информацию элемента очереди
  if(nodes) return nodes->info;
  return NULL;
}

void fnode::sort_nodes(){ //функция сортировки очереди найденых узлов
  if(!nodes) return;
  char *s_name;
  struct nstat *s_info;
  int rot;
  while(1){
    rot=0;
    while(nodes->next){
      if(strcmp(nodes->name,nodes->next->name)>0){
        s_name=nodes->name;
        s_info=nodes->info;
        nodes->name=nodes->next->name;
        nodes->info=nodes->next->info;
        nodes->next->name=s_name;
        nodes->next->info=s_info;
        rot++;
      };
      nodes=nodes->next;
    };
    nodes=t_nodes;
    if(!rot) break;
  };
}

/***********************************************************************************************************/

