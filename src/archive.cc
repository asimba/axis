#include "archive.h"

#ifdef _WIN32

  #include <windows.h>
  #include <time.h>

bool filetime_from_time(PFILETIME pFT,time_t tm){
  struct tm *ptm=localtime(&tm);
  SYSTEMTIME SystemTime;
  FILETIME LocalTime;
  if(ptm==NULL) return false;
  SystemTime.wYear=ptm->tm_year+1900;
  SystemTime.wMonth=ptm->tm_mon + 1;
  SystemTime.wDay=ptm->tm_mday;
  SystemTime.wHour=ptm->tm_hour;
  SystemTime.wMinute=ptm->tm_min;
  SystemTime.wSecond=ptm->tm_sec;
  SystemTime.wMilliseconds=0;
  return SystemTimeToFileTime(&SystemTime,&LocalTime)&&
         LocalFileTimeToFileTime(&LocalTime, pFT);
}

#endif

void list_print(const struct list_record *v){
  if(v!=NULL){
#ifndef _WIN32
    fprintf(stdout,"%s->%s\n",(S_ISDIR(v->info->info.attributes)?"[dir ]":"[file]"),v->full_name);
#else
    char *ansi_full_name=codepagetocodepage(v->full_name,CP_UTF8,CP_OEMCP);
    if(ansi_full_name){
      fprintf(stdout,"%s->%s\n",(S_ISDIR(v->info->info.attributes)?"[dir ]":"[file]"),ansi_full_name);
      memset(ansi_full_name,0,strlen(ansi_full_name));
      free(ansi_full_name);
      ansi_full_name=NULL;
    }
    else (v->errcode)[0]=0x00000001;
#endif
  };
}

void list_summary_print(const struct list_summary_record *v){
  if(v!=NULL){
    fprintf(stdout,"Summary:\n  dirs : %u\n  files: %u\n  size : %" PRIu64 "\n",v->dirs,v->files,v->size);
  };
}

/***********************************************************************************************************/
//Основной класс для осуществления архивирования файлов ("archiver")
/***********************************************************************************************************/

archiver::archiver(){ //конструктор класса
  errcode=0;
  ignore_pt=false;
  base=NULL;
  tree=new ftree;
  header_size=0;
  data_size=0L;
  rwl=0L;
  c_f=NULL;
  l_f=NULL;
  ls_f=NULL;
  if(tree){
    if(tree->memory_fail) errcode=0x00000001;
  }
  else errcode=0x00000001;
  errcode=flt.errcode;
}

archiver::~archiver(){ //деструктор класса
  errcode=0;
  ignore_pt=false;
  if(tree) delete tree;
  tree=NULL;
  if(base){
    memset(base,0,strlen(base));
    free(base);
    base=NULL;
  };
  header_size=0;
  data_size=0L;
  rwl=0L;
  c_f=NULL;
  l_f=NULL;
  ls_f=NULL;
}

void archiver::set_checksum_operator(checksum_operator f){
  if(f!=NULL) c_f=f;
  else c_f=NULL;
}

void archiver::set_list_operator(list_operator f){
  if(f!=NULL) l_f=f;
  else l_f=NULL;
}

void archiver::set_list_summary_operator(list_summary_operator f){
  if(f!=NULL) ls_f=f;
  else ls_f=NULL;
}

void archiver::add_name(const char *name){ //функция добавления имени в список архивирования
  if(errcode) return;
  if(name==NULL) return;
#ifndef _WIN32
  char *src=strdup(name);
#else
  char *src=codepagetocodepage(name,CP_OEMCP,CP_UTF8);
#endif
  if(src==NULL){
    errcode=0x00000001;
    return;
  };
  while(strchr(src,'\\')) *(strchr(src,'\\'))='/';
  tree->addnode(src);
  memset(src,0,strlen(src));
  free(src);
  src=NULL;
  if(tree->memory_fail) errcode=0x00000001;
  if(tree->access_fail) errcode=0x00000400;
}

void archiver::set_base(const char *name){ //функция задания базового пути для разархивирования
  if(errcode) return;
  if(name){
    base=(char *)calloc(strlen(name)+2,sizeof(char));
    if(base){
      strcpy(base,name);
      if(base[strlen(base)-1]!='/') base[strlen(base)]='/';
    }
    else errcode=0x00000001;
  };
}

void archiver::read_header(FILE *filedsc){
  unsigned char clength[4];
  char *content=NULL;
  uint32_t rw_length=0;
  int32_t length=0;
  tree->release();
  if(pack.load_header((void *)filedsc)){
    errcode=0x00000800;
    return;
  };
  while(1){
    if((length=pack.packer_read((void *)filedsc,(char *)clength,sizeof(uint32_t)))<0) errcode=0x00000800;
#ifdef EINTR
    if(length<0 && errno==EINTR) continue;
#endif
    errcode=flt.errcode;
    if(errcode) return;;
    break;
  };
  while(1){
    if((length=pack.packer_read((void *)filedsc,(char *)clength,sizeof(uint32_t)))<0) errcode=0x00000800;
#ifdef EINTR
    if(length<0 && errno==EINTR) continue;
#endif
    errcode=flt.errcode;
    if(errcode) break;
    else rwl+=length;
    if(length<4){
      errcode=0x00000800;
      break;
    };
    char2long_s(clength,rw_length);
    if(rw_length==0xffffffff) break;
    if((rw_length>0x00ffffff)||(rw_length<sizeof(uint32_t))){
      errcode=0x00000800;
      break;
    };
    content=(char *)calloc(rw_length,sizeof(char));
    rw_length-=sizeof(uint32_t);
    if(content){
      if((length=pack.packer_read((void *)filedsc,content,rw_length))<0) errcode=0x00000800;
      errcode=flt.errcode;
      if(errcode){
        memset(content,0,rw_length);
        free(content);
        content=NULL;
        break;
      }
      else rwl+=length;
      if(length<(int32_t)rw_length){
        errcode=0x00000800;
        memset(content,0,rw_length);
        free(content);
        content=NULL;
        break;
      };
      tree->load_next_content(length+sizeof(uint32_t),content);
      header_size+=(length+sizeof(uint32_t));
      memset(content,0,rw_length);
      free(content);
      content=NULL;
      if(tree->memory_fail){
        errcode=0x00000001;
        break;
      };
    }
    else{
      errcode=0x00000001;
      break;
    };
  };
}

void archiver::archive(const char *filename, bool unarc, uint8_t unarc_null){ //функция основного цикла архивирования
  if(errcode) return;
  FILE *io_stream=NULL;
  FILE *tm_stream=NULL;
  int tm_file=0;
  unsigned char clength[4];
  long length=0;
  off64_t rw_length_file=0;
  char *content=NULL;
  char *full_name=NULL;
  char *path=NULL;
  char *tmp_full_name=NULL;
  uint32_t files=0;
  uint32_t dirs=0;
  off64_t size=0;
  struct infonode info;
  struct list_record v;
  pack.init();
  pack.set_filters(&flt);
#ifdef _WIN32
  wchar_t *wc_buf=NULL;
  HANDLE dhndl;
  FILETIME atime;
  FILETIME mtime;
#endif
  if(unarc){
    if(filename) io_stream=fopen64(filename,"rb");
    else io_stream=fdopen(fileno(stdin),"rb");
    if(io_stream==NULL){
      errcode=0x00000040;
      return;
    };
    if(flt.ignore_sn==false){
      if(axis_sign(io_stream,unarc)==-1){
        errcode=0x00000100;
        fclose(io_stream);
        return;
      };
    };
    char *current_dir=(char *)calloc(MAX_PATH+1,sizeof(char));
    if(current_dir==NULL){
      errcode=0x00000001;
      fclose(io_stream);
      return;
    };
    if(getcwd(current_dir,MAX_PATH+1)==NULL){
      errcode=0x00080000;
      memset(current_dir,0,strlen(current_dir));
      free(current_dir);
      current_dir=NULL;
      fclose(io_stream);
      return;
    };
    if(unarc_null){
      if(base){
        memset(base,0,strlen(base));
        free(base);
        base=NULL;
      };
    };
    if(base){
      if(chdir(base)==-1){
        errcode=0x00001000;
        memset(current_dir,0,strlen(current_dir));
        free(current_dir);
        current_dir=NULL;
        fclose(io_stream);
        return;
      };
    };
    content=(char *)calloc(_BUFFER_SIZE,sizeof(char));
    if(content==NULL){
      errcode=0x00000001;
      fclose(io_stream);
      if(base) chdir(current_dir);
      memset(current_dir,0,strlen(current_dir));
      free(current_dir);
      current_dir=NULL;
      return;
    };
    read_header(io_stream);
    if(errcode){
      fclose(io_stream);
      if(content){
        memset(content,0,_BUFFER_SIZE);
        free(content);
        content=NULL;
      };
      if(base) chdir(current_dir);
      memset(current_dir,0,strlen(current_dir));
      free(current_dir);
      current_dir=NULL;
      return;
    };
    tree->relatives();
    data_size=tree->get_data_size();
#ifndef _WIN32
    struct timeval ut[2];
#endif
    while(tree->get_next_relative(&path,info)){
      full_name=(char *)calloc(strlen(path)+strlen(info.name)+2,sizeof(char));
      if(full_name==NULL){
        errcode=0x00000001;
        break;
      };
      strcat(full_name,path);
      full_name[strlen(full_name)]='/';
      strcat(full_name,info.name);
      if(canonicalize(full_name)==false){
        errcode=0x00000001;
        memset(full_name,0,strlen(full_name));
        free(full_name);
        full_name=NULL;
        break;
      };
      if(S_ISCHR(info.info.attributes)||
         S_ISBLK(info.info.attributes)||
         S_ISFIFO(info.info.attributes)){
        errcode=0x00000800;
        memset(full_name,0,strlen(full_name));
        free(full_name);
        full_name=NULL;
        break;
      };
      if(S_ISDIR(info.info.attributes)){
        if(!unarc_null) errcode=mdirs(full_name,info,ignore_pt);
        else{
          if(unarc_null==2){
            dirs++;
            if(l_f!=NULL){
              v.info=&info;
              v.full_name=full_name;
              v.errcode=&errcode;
              (*l_f)(&v);
            };
          };
        };
        memset(full_name,0,strlen(full_name));
        free(full_name);
        full_name=NULL;
        if(errcode) break;
        continue;
      }
      else{
        if(unarc_null==2){
          files++;
          size+=info.info.size;
          if(l_f!=NULL){
            v.info=&info;
            v.full_name=full_name;
            v.errcode=&errcode;
            (*l_f)(&v);
          };
          memset(full_name,0,strlen(full_name));
          free(full_name);
          full_name=NULL;
          if(errcode) break;
          continue;
        };
        if(!unarc_null){
          tmp_full_name=strdup(full_name);
          if(tmp_full_name==NULL){
            errcode=0x00000001;
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
            break;
          };
          errcode=mdirs(dirname(tmp_full_name),info,ignore_pt);
          memset(tmp_full_name,0,strlen(full_name));
          free(tmp_full_name);
          tmp_full_name=NULL;
          if(errcode){
            errcode=0x00000080;
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
            break;
          };
#ifndef _WIN32
          if(access(full_name,F_OK)==0){
            errcode=0x00000080;
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
            break;
          };
          tm_stream=fopen64(full_name,"wb");
#else
          wc_buf=codepagetowchar(full_name,CP_UTF8);
          memset(full_name,0,strlen(full_name));
          free(full_name);
          full_name=NULL;
          if(wc_buf){
            if(_waccess(wc_buf,F_OK)==0){
              errcode=0x00000080;
              wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
              free(wc_buf);
              wc_buf=NULL;
              break;
            };
            tm_stream=_wfopen(wc_buf,L"wb");
          }
          else{
            errcode=0x00000001;
            break;
          };
#endif
          if(tm_stream==NULL){
#ifndef _WIN32
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
#else
            wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
            free(wc_buf);
            wc_buf=NULL;
#endif
            errcode=0x000000f0;
            break;
          };
          tm_file=fileno(tm_stream);
        };
        rw_length_file=info.info.size;
        while(1){
          if(!rw_length_file) break;
          if(rw_length_file>_BUFFER_SIZE) length=_BUFFER_SIZE;
          else length=(uint32_t)rw_length_file;
          if((length=pack.packer_read((void *)io_stream,content,length))<0) errcode=0x00000800;
#ifdef EINTR
          if(length<0 && errno==EINTR) continue;
#endif
          errcode=flt.errcode;
          if(errcode) break;
          else rwl+=length;
          if((length!=_BUFFER_SIZE)&&((off64_t)length!=rw_length_file)){
            errcode=0x00000004;
            break;
          };
          if(!unarc_null) length=write(tm_file,content,length);
          if((length!=_BUFFER_SIZE)&&((off64_t)length!=rw_length_file)){
            errcode=0x00000200;
            break;
          };
          if(rw_length_file>_BUFFER_SIZE) rw_length_file-=_BUFFER_SIZE;
          else rw_length_file=0;
        };
        if(unarc_null){
          memset(full_name,0,strlen(full_name));
          free(full_name);
          full_name=NULL;
          continue;
        };
        if(fflush(tm_stream)==EOF) errcode=0x00000200;
        fclose(tm_stream);
        if(errcode){
#ifndef _WIN32

          memset(full_name,0,strlen(full_name));
          free(full_name);
          full_name=NULL;
#else
          wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
          free(wc_buf);
          wc_buf=NULL;
#endif
          break;
        };
#ifndef _WIN32
        if((chown(full_name,info.info.uids,info.info.gids)==-1)&&(ignore_pt==false)){
          errcode=0x00040000;
          memset(full_name,0,strlen(full_name));
          free(full_name);
          full_name=NULL;
          break;
        };
        if((chmod(full_name,info.info.attributes)==-1)&&(ignore_pt==false)){
#else
        if((_wchmod(wc_buf,info.info.attributes)==-1)&&(ignore_pt==false)){
#endif
          errcode=0x00010000;
#ifndef _WIN32
          memset(full_name,0,strlen(full_name));
          free(full_name);
          full_name=NULL;
#else
          wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
          free(wc_buf);
          wc_buf=NULL;
#endif
          break;
        };
#ifndef _WIN32
        ut[0].tv_sec=info.info.atime;
        ut[0].tv_usec=0;
        ut[1].tv_sec=info.info.mtime;
        ut[1].tv_usec=0;
        if((utimes(full_name,ut)==-1)&&(ignore_pt==false)){
#else
        dhndl=CreateFileW(wc_buf,GENERIC_READ|GENERIC_WRITE,
                         0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,
                         NULL);
        if(dhndl){
          if((filetime_from_time(&atime,info.info.atime)==false)||
             (filetime_from_time(&mtime,info.info.mtime)==false)||
             (SetFileTime(dhndl,NULL,&atime,&mtime)==-1)){
            errcode=0x00020000;
          };
          CloseHandle(dhndl);
          wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
          free(wc_buf);
          wc_buf=NULL;
          if(errcode) break;
        }
        else{
#endif
          errcode=0x00020000;
#ifndef _WIN32
          memset(full_name,0,strlen(full_name));
          free(full_name);
          full_name=NULL;
#else
          wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
          free(wc_buf);
          wc_buf=NULL;
#endif
          break;
        };
#ifndef _WIN32
          if(full_name!=NULL){
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
          };
#else
          if(wc_buf!=NULL){
            wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
            free(wc_buf);
            wc_buf=NULL;
          };
#endif
      };
    };
    if((unarc_null==0)&&(errcode==0)){
      while(tree->get_next_relative(&path,info)){
        if(S_ISDIR(info.info.attributes)){
          full_name=(char *)calloc(strlen(path)+strlen(info.name)+2,
            sizeof(char));
          if(full_name==NULL){
            errcode=0x00000001;
            break;
          };
          strcat(full_name,path);
          full_name[strlen(full_name)]='/';
          strcat(full_name,info.name);
          if(canonicalize(full_name)==false){
            errcode=0x00000001;
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
            break;
          };
#ifndef _WIN32
          if((chown(full_name,info.info.uids,info.info.gids)==-1)&&(ignore_pt==false)){
            errcode=0x00040000;
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
            break;
          };
          if((chmod(full_name,info.info.attributes)==-1)&&(ignore_pt==false)){
#else
          wc_buf=codepagetowchar(full_name,CP_UTF8);
          if(wc_buf==NULL){
            errcode=0x00000001;
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
            break;
          };
          if((_wchmod(wc_buf,info.info.attributes)==-1)&&(ignore_pt==false)){
#endif
            errcode=0x00010000;
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
#ifdef _WIN32
            wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
            free(wc_buf);
            wc_buf=NULL;
#endif
            break;
          };
#ifndef _WIN32
          ut[0].tv_sec=info.info.atime;
          ut[0].tv_usec=0;
          ut[1].tv_sec=info.info.mtime;
          ut[1].tv_usec=0;
          if((utimes(full_name,ut)==-1)&&(ignore_pt==false)&&
             (strcmp(full_name,".")!=0)&&(strcmp(full_name,"./")!=0)&&
             (strcmp(full_name,"./.")!=0)){
#else
          dhndl=CreateFileW(wc_buf,GENERIC_READ|GENERIC_WRITE,
                           0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,
                           NULL);
          if(dhndl){
            if((filetime_from_time(&atime,info.info.atime)==false)||
               (filetime_from_time(&mtime,info.info.mtime)==false)||
               (SetFileTime(dhndl,NULL,&atime,&mtime)==-1)){
              errcode=0x00020000;
            };
            CloseHandle(dhndl);
            memset(full_name,0,strlen(full_name));
            free(full_name);
            wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
            free(wc_buf);
            wc_buf=NULL;
            full_name=NULL;
            if(errcode) break;
          }
          else{
#endif
            errcode=0x00020000;
          };
          if(full_name){
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
          };
#ifdef _WIN32
          if(wc_buf){
            wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
            free(wc_buf);
            wc_buf=NULL;
          };
#endif
          if(errcode) break;
        };
      };
    };
    chdir(current_dir);
    memset(current_dir,0,strlen(current_dir));
    free(current_dir);
    current_dir=NULL;
    memset(content,0,_BUFFER_SIZE);
    free(content);
    content=NULL;
    fclose(io_stream);
    rwl-=sizeof(uint32_t);
    if(unarc_null==2){
      if(ls_f!=NULL){
        struct list_summary_record s_v;
        s_v.dirs=dirs;
        s_v.files=files;
        s_v.size=size;
        (*ls_f)(&s_v);
      };
    };
  }
  else{
    tree->rebuild();
    tree->relatives();
    if(tree->memory_fail){
      errcode=0x00000001;
      return;
    };
    if(tree->access_fail){
      errcode=0x00000400;
      return;
    };
    data_size=tree->get_data_size();
    header_size=tree->get_header_size();
    if((header_size==0)&&(data_size==0L)){
      errcode=0x00002000;
      return;
    };
    if(filename){
      if(access(filename,F_OK)==0){
        errcode=0x00000080;
        return;
      }
      else{
        io_stream=fopen64(filename,"wb");
        if(io_stream==NULL){
          errcode=0x000000f0;
          return;
        };
      };
    }
    else io_stream=fdopen(fileno(stdout),"ab");
    if(flt.ignore_sn==false){
      if(axis_sign(io_stream,unarc)==-1){
        errcode=0x00000200;
        fclose(io_stream);
        return;
      };
    };
    long2char_s(clength,timebits());
    pack.flags=8;
    if(pack.packer_write((void *)io_stream,(char *)clength,sizeof(uint32_t))<0) errcode=0x00000200;
    errcode=flt.errcode;
    if(errcode==0){
      while(1){
        content=tree->get_next_content(length);
        if(content){
          if(pack.packer_write((void *)io_stream,content,length)<0){
            errcode=0x00000200;
            break;
          };
          if(errcode) break;
          errcode=flt.errcode;
          if(errcode) break;
          else rwl+=length;
          continue;
        };
        break;
      };
    };
    if(errcode){
      fclose(io_stream);
      if(filename) remove(filename);
      return;
    };
    long2char_s(clength,0xffffffff);
    if(pack.packer_write((void *)io_stream,(char *)clength,sizeof(uint32_t))<0){
      errcode=0x00000200;
    };
    errcode=flt.errcode;
    if(errcode==0){
      content=(char *)calloc(_BUFFER_SIZE,sizeof(char));
      if(content==NULL) errcode=0x00000001;
    }
    else rwl+=length;
    if(errcode){
      fclose(io_stream);
      if(filename) remove(filename);
      return;
    };
    while(tree->get_next_relative(&path,info)){
      if(S_ISDIR(info.info.attributes)) continue;
      full_name=(char *)calloc(strlen(path)+strlen(info.name)+2,sizeof(char));
      if(full_name==NULL){
        errcode=0x00000001;
        break;
      };
      strcpy(full_name,path);
      if(path[0]) full_name[strlen(path)]='/';
      strcat(full_name,info.name);
#ifndef _WIN32
      tm_stream=fopen64(full_name,"rb");
#else
      wc_buf=codepagetowchar(full_name,CP_UTF8);
      if(wc_buf){
        tm_stream=_wfopen(wc_buf,L"rb");
        wmemset(wc_buf,(wchar_t)0,wcslen(wc_buf));
        free(wc_buf);
        wc_buf=NULL;
      }
      else{
        errcode=0x00000001;
        memset(full_name,0,strlen(full_name));
        free(full_name);
        full_name=NULL;
        break;
      };
#endif
      memset(full_name,0,strlen(full_name));
      free(full_name);
      full_name=NULL;
      if(tm_stream==NULL){
        errcode=0x00000004;
        break;
      };
      tm_file=fileno(tm_stream);
      rw_length_file=0;
      while(1){
        length=read(tm_file,content,_BUFFER_SIZE);
#ifdef EINTR
        if(length<0 && errno==EINTR) continue;
#endif
        if(length<=0) break;
        rw_length_file+=length;
        if(rw_length_file>info.info.size){
          errcode=0x00004000;
          break;
        };
        if(pack.packer_write((void *)io_stream,content,length)<0) errcode=0x00000200;
        if(errcode) break;
        errcode=flt.errcode;
        if(errcode) break;
        else rwl+=length;
      };
      fclose(tm_stream);
      if(rw_length_file<info.info.size){
        errcode=0x00000004;
      };
      if(errcode) break;
    };
    if(pack.packer_write((void *)io_stream,NULL,0)<0) errcode=0x00000200;
    flt.write_sync_flt(io_stream);
    errcode=flt.errcode;
    memset(content,0,_BUFFER_SIZE);
    free(content);
    content=NULL;
    if(fflush(io_stream)==EOF) errcode=0x00000200;
    fclose(io_stream);
    if(errcode){
      if(filename) remove(filename);
      return;
    };
  };
}

void archiver::checksums(){ //функция основного цикла получения контрольных сумм
  if(errcode) return;
  char *full_name=NULL;
  char *path=NULL;
  struct infonode info;
  tree->rebuild();
  tree->relatives();
  if(tree->memory_fail){
    errcode=0x00000001;
    return;
  };
  if(tree->access_fail){
    errcode=0x00000400;
    return;
  };
  data_size=tree->get_data_size();
  header_size=tree->get_header_size();
  if((header_size==0)&&(data_size==0L)){
    errcode=0x00002000;
    return;
  };
  alfl *lags;
  if(flt.slag!=NULL) lags=flt.slag;
  else lags=new alfl;
  if(lags){
    alfg *state=new alfg(lags);
    if(state){
      if(state->lags){
        scxh *hash=new scxh(state);
        if(hash){
          while(tree->get_next_relative(&path,info)){
            if(S_ISDIR(info.info.attributes)) continue;
            full_name=(char *)calloc(strlen(path)+strlen(info.name)+2,sizeof(char));
            if(full_name==NULL){
              errcode=0x00000001;
              break;
            };
            strcpy(full_name,path);
            if(path[0]) full_name[strlen(path)]='/';
            strcat(full_name,info.name);
            errcode=axisum(hash,full_name,c_f);
            memset(full_name,0,strlen(full_name));
            free(full_name);
            full_name=NULL;
            if(errcode) break;
          };
          delete hash;
          hash=NULL;
        }
        else errcode=0x00000001;
      }
      else errcode=0x00000001;
      delete state;
      state=NULL;
    }
    else errcode=0x00000001;
    if(flt.slag==NULL) delete lags;
    else lags=NULL;
  }
  else errcode=0x00000001;
  if(full_name!=NULL){
    memset(full_name,0,strlen(full_name));
    free(full_name);
    full_name=NULL;
  };
}

/***********************************************************************************************************/
