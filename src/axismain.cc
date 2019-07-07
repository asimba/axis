#include "archive.h"
#include "etc.h"
#include <getopt.h>
#include "globfunction.h"
#include "getpasswd.h"
#ifndef VERSION
  #include "version.h"
#endif

#ifndef _WIN32
  #include <pthread.h>
  #include <signal.h>
  #include <sys/mman.h>
  #include "vars.h"
#else
  #include <windows.h>
#endif

#define PROGRAM_NAME "Axis"
#define AUTHORS "Alexey V. Simbarsky"

char *program_name;

static struct option const long_options[]=
{
  {"encrypt", no_argument, NULL, 'e'},
  {"decrypt", optional_argument, NULL, 'd'},
  {"list", optional_argument, NULL, 'l'},
  {"check", no_argument, NULL, 'c'},
  {"repair", no_argument, NULL, 'r'},
  {"checksum", optional_argument, NULL, 'u'},
  {"ignore", no_argument, NULL, 'n'},
  {"incognito", no_argument, NULL, 'g'},
  {"irreplaceable", no_argument, NULL, 'm'},
  {"jam", no_argument, NULL, 'j'},
  {"password", required_argument, NULL, 'p'},
  {"input", required_argument, NULL, 'i'},
  {"output", required_argument, NULL, 'o'},
  {"statistics", no_argument, NULL, 's'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

void usage(int status){
  if(status) fprintf(stderr, "Try `%s --help' for more information.\n", program_name);
  else{
    fprintf(stdout,"\nAxis (XS). This program provides simple protection of user data.\n\
\nUsage  : %s <OPTIONS>\nExamples:\
\n     axis --encrypt -i ./ --password=abcdef > encrypted.xs\
\n     axis --encrypt -p abcdef --input=./source.in --output=./encrypted.xs\
\n     axis --encrypt -p abcdef -i ./src1 -i ./src2 -o ./encrypted.xs\
\n     axis --decrypt -p abcdef -i ./source.xs\
\n     axis --decrypt -p abcdef -i ./source.xs -o ./\
\n     axis -e -p abcdef -i ./source.in -o ./encrypted.xs\
\n     axis -e -j -s -p abcdef -i \"./source.*\" -o ./encrypted.xs\
\n     axis --statistics -e -p abcdef -i ./source.in -o ./encrypted.xs\
\n     axis -d -i ./source.xs\
\n     axis -s -d -i ./source.xs\
\n     axis --repair -i ./source.xs --output=./repaired.xs\
\n     axis -r -i ./source.xs -o ./repaired.xs\
\n     cat source.xs | axis --repair > repaired.xs\
\n     axis --check -i ./source.xs\
\n     axis -c -i ./source.xs\
\n     cat source.xs | axis --check\
\n     axis --list -i ./source.xs -p abcdef\
\n     axis -l -i ./source.xs\
\n     axis -u -i \"./*.in\"\
\n     axis -u\"./*.in\"\
\n     cat source.in | axis -u\n\
Options:\n\
  -e, --encrypt                  encode input data (\"encrypt\" MODE)\n\
  -d, --decrypt                  decode input data (\"decrypt\" MODE)\n\
  -d0,--decrypt=0                decode input data (\"decrypt\" MODE)\n\
                                 but without writing - a kind of check\n\
  -l, --list                     list contents of the XS-archive\n\
  -l0,--list=0                   list contents of the XS-archive,\n\
                                 but without printing - a kind of check\n\
  -c, --check                    check integrity of the encoded data\n\
  -r, --repair                   repair corrupted XS-data\n\
  -u[NAME], --checksum[=NAME]    calculate Axis (160-bit) checksums - ONLY ONE\n\
                                 FILENAME or PATH\n\
  -p STRING, --password=STRING   max size of the STRING: 255 characters\n\
                                 (or 127 characters in a language other than\n\
                                 English)\n\
                                 min size of the STRING:  6  bytes \n\
                                 (required if MODE is NOT \"check\" or \"repair\";\n\
                                 if it is not set - it will be requested)\n\
  -n, --ignore                   ignore impossibility of change file\n\
                                 ownership/permissions or access/modification\n\
                                 times (ONLY for \"decrypt\" MODE)\n\
  -g, --incognito                don't write/check identification signature\n\
                                 in XS-archive\n\
  -m, --irreplaceable            don't write/check recovery data in XS-archive\n\
                                 (ONLY for \"encrypt\" or \"decrypt\" MODE)\n\
                                 WARNING: HIGH probability of damage to the\n\
                                 output data!\n\
  -j, --jam                      synonymous for \"-g -m\"\n\
  -i NAME  , --input=NAME        input file or directory\n\
                                 (wildcards accepted ONLY for \"encrypt\" MODE)\n\
                                 NOTE: when using wildcards - put them in\n\
                                 double quotes\n\
  -o NAME  , --output=NAME       output file or directory - ONLY ONE FILENAME\n\
                                 or PATH\n\
  -s, --statistics               display work progress\n\
  -h, --help                     display this help and exit\n\
  -v, --version                  output version information and exit\n",
  program_name);
    version(stdout,PROGRAM_NAME,VERSION,AUTHORS);
  };
}

struct th_archiver_helper{
  archiver *xs;
  char *input_file;
  char *output_file;
  bool opcode;
  uint8_t unarc_null;
#ifndef _WIN32
  pthread_mutex_t lock;
#else
  HANDLE lock;
#endif
};

#ifndef _WIN32
void msg_cycle(th_archiver_helper &th_in, pthread_t &th_id,
              int32_t &errcode, uint32_t &softerr,
              uint32_t &harderr, char *msg){
#else
void msg_cycle(th_archiver_helper &th_in, HANDLE &th_id,
              int32_t &errcode, uint32_t &softerr,
              uint32_t &harderr, char *msg){
#endif
  fprintf(stdout,"\n");
  uint8_t c=0;
  while(1){
#ifndef _WIN32
    if(pthread_mutex_trylock(&th_in.lock)==EBUSY){
#else
    if(WaitForSingleObjectEx(th_in.lock,0,false)!=WAIT_OBJECT_0){
#endif
      if(c>0){
        fprintf(stdout,".");
        c--;
      }
      else{
        clrline();
        fprintf(stdout,"%s",msg);
        c=3;
      };
      fflush(stdout);
#ifndef _WIN32
      usleep(300000);
#else
      Sleep(300);
#endif
    }
    else{
      errcode=th_in.xs->flt.errcode;
      clrline();
      fprintf(stdout,"%s...",msg);
      fflush(stdout);
      if(errcode){
        fprintf(stderr,"\nError!\n");
        fflush(stderr);
      }
      else{
        softerr=th_in.xs->flt.corrector.softerr;
        harderr=th_in.xs->flt.corrector.harderr;
        if(softerr) errcode|=0x00008000;
        if(harderr) errcode|=0x00100000;
        if(errcode){
          fprintf(stderr,"\nError\n");
          fflush(stderr);
        }
        else{
          fprintf(stdout,"Ok\n");
          fflush(stdout);
        };
      };
#ifndef _WIN32
      pthread_join(th_id,NULL);
#else
      WaitForSingleObjectEx(th_id,INFINITE,false);
      ReleaseMutex(th_in.lock);
      CloseHandle(th_id);
#endif
      break;
    };
  };
}

#ifndef _WIN32
void *th_xs_check(void *in){
#else
DWORD WINAPI *th_xs_check(LPVOID *in){
#endif
#ifndef _WIN32
  pthread_mutex_lock(&(((th_archiver_helper *)in)->lock));
#else
  WaitForSingleObjectEx(((th_archiver_helper *)in)->lock,INFINITE,false);
#endif
  ((th_archiver_helper *)in)->opcode=true;
  ((th_archiver_helper *)in)->xs->flt.transcode(
    ((th_archiver_helper *)in)->input_file,
    ((th_archiver_helper *)in)->output_file);
#ifndef _WIN32
  pthread_mutex_unlock(&(((th_archiver_helper *)in)->lock));
  pthread_exit(NULL);
#else
  ReleaseMutex(((th_archiver_helper *)in)->lock);
  return 0;
#endif
}

#ifndef _WIN32
void *th_xs_repair(void *in){
#else
DWORD WINAPI *th_xs_repair(LPVOID *in){
#endif
#ifndef _WIN32
  pthread_mutex_lock(&(((th_archiver_helper *)in)->lock));
#else
  WaitForSingleObjectEx(((th_archiver_helper *)in)->lock,INFINITE,false);
#endif
  ((th_archiver_helper *)in)->opcode=true;
  ((th_archiver_helper *)in)->xs->flt.transcode(
    ((th_archiver_helper *)in)->input_file,
    ((th_archiver_helper *)in)->output_file);
  if(((th_archiver_helper *)in)->xs->flt.errcode==0){
    ((th_archiver_helper *)in)->xs->flt.reset();
    ((th_archiver_helper *)in)->xs->flt.set_check();
    ((th_archiver_helper *)in)->xs->flt.transcode(
      ((th_archiver_helper *)in)->output_file,NULL);
  };
#ifndef _WIN32
  pthread_mutex_unlock(&(((th_archiver_helper *)in)->lock));
  pthread_exit(NULL);
#else
  ReleaseMutex(((th_archiver_helper *)in)->lock);
  return 0;
#endif
}

#ifndef _WIN32
void *th_xs_set(void *in){
#else
DWORD WINAPI *th_xs_set(LPVOID *in){
#endif
#ifndef _WIN32
  pthread_mutex_lock(&(((th_archiver_helper *)in)->lock));
#else
  WaitForSingleObjectEx(((th_archiver_helper *)in)->lock,INFINITE,false);
#endif
  ((th_archiver_helper *)in)->xs->flt.set(
    ((th_archiver_helper *)in)->input_file);
#ifndef _WIN32
  pthread_mutex_unlock(&(((th_archiver_helper *)in)->lock));
  pthread_exit(NULL);
#else
  ReleaseMutex(((th_archiver_helper *)in)->lock);
  return 0;
#endif
}

#ifndef _WIN32
void *th_xs_archive(void *in){
#else
DWORD WINAPI *th_xs_archive(LPVOID *in){
#endif
#ifndef _WIN32
  pthread_mutex_lock(&(((th_archiver_helper *)in)->lock));
#else
  WaitForSingleObjectEx(((th_archiver_helper *)in)->lock,INFINITE,false);
#endif
  ((th_archiver_helper *)in)->xs->archive(
    ((th_archiver_helper *)in)->input_file,
    ((th_archiver_helper *)in)->opcode,
    ((th_archiver_helper *)in)->unarc_null);
#ifndef _WIN32
  pthread_mutex_unlock(&(((th_archiver_helper *)in)->lock));
  pthread_exit(NULL);
#else
  ReleaseMutex(((th_archiver_helper *)in)->lock);
  return 0;
#endif
}

#ifndef _WIN32
void xs_archive(th_archiver_helper &th_in, pthread_t &th_id,
               pthread_attr_t &th_attr,
               bool &stats, int &errcode){
#else
void xs_archive(th_archiver_helper &th_in, HANDLE &th_id,
               bool &stats, int &errcode){
#endif
  if(stats){
    struct timeval tv_start,tv_stop;
    uint32_t elapsed=0;
    uint32_t estimated=0;
    char strbuf_el[12];
    char strbuf_es[12];
    bool stopped=false;
    gettimeofday(&tv_start,NULL);
    char msg[]="Processing  :";
#ifndef _WIN32
    pthread_mutex_unlock(&th_in.lock);
#endif
#ifndef _WIN32
    if(pthread_create(&th_id,&th_attr,th_xs_archive,(void *)&th_in)){
      errcode=0x04000000;
    }
#else
    th_id=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)th_xs_archive,
      (LPVOID *)&th_in,0,NULL);
    if(th_id==NULL){
      errcode=0x04000000;
    }
#endif
    else{
      uint8_t c=0;
      while((!th_in.xs->header_size)&&(!th_in.xs->errcode)){
#ifndef _WIN32
        if(pthread_kill(th_id,0)!=0){
          stopped=true;
          break;
        };
        usleep(1000);
#else
        if(WaitForSingleObjectEx(th_id,0,false)!=WAIT_TIMEOUT){
          stopped=true;
          break;
        };
        Sleep(10);
#endif
      };
      while(1){
        clrline();
        fprintf(stdout,"%s",msg);
        fflush(stdout);
#ifndef _WIN32
        if((!stopped)&&(pthread_mutex_trylock(&th_in.lock)==EBUSY)){
#else
        if((!stopped)&&(WaitForSingleObjectEx(th_in.lock,0,false)!=WAIT_OBJECT_0)){
#endif
          if((th_in.xs->header_size)||(th_in.xs->data_size!=0L)){
            gettimeofday(&tv_stop,NULL);
            elapsed=tv_stop.tv_sec-tv_start.tv_sec+(tv_stop.tv_usec-tv_start.tv_usec)/1000000;
            double tmp=(double)((double)th_in.xs->rwl/
                   (double)((double)th_in.xs->header_size+
                   (double)th_in.xs->data_size)*100);
            if(tmp!=0) estimated=(uint32_t)((elapsed/tmp)*100.0);
            uint32_t el_h=elapsed/3600;
            uint32_t es_h=estimated/3600;
            strbuf_el[0]=0;
            strbuf_es[0]=0;
            if(el_h) sprintf(strbuf_el,"%u:",el_h);
            if(es_h) sprintf(strbuf_es,"%u:",es_h);
            if(tmp<=100) fprintf(stdout,"%7.3f%% [%s%02u:%02u/%s%02u:%02u]",
                                 tmp,strbuf_el,(elapsed%3600)/60,elapsed%60,
                                 strbuf_es,(estimated%3600)/60,estimated%60);
            else fprintf(stdout,"%7.3f%%        ",0.0);
            fflush(stdout);
          }
          else{
            if(c>0){
              fprintf(stdout,".");
              fflush(stdout);
              c--;
            }
            else{
              clrline();
              fprintf(stdout,"%s",msg);
              fflush(stdout);
              c=3;
            };
          };
#ifndef _WIN32
          usleep(300000);
#else
          Sleep(300);
#endif
        }
        else{
          errcode=th_in.xs->errcode;
          if((!stopped)&&((th_in.xs->header_size)||(th_in.xs->data_size!=0L))){
            double tmp=(double)((double)th_in.xs->rwl/
                   (double)((double)th_in.xs->header_size+
                   (double)th_in.xs->data_size)*100);
            if(tmp!=0) estimated=(uint32_t)((elapsed/tmp)*100.0);
            uint32_t el_h=elapsed/3600;
            uint32_t es_h=estimated/3600;
            strbuf_el[0]=0;
            strbuf_es[0]=0;
            if(el_h) sprintf(strbuf_el,"%u:",el_h);
            if(es_h) sprintf(strbuf_es,"%u:",es_h);
            if(tmp<=100) fprintf(stdout,"%7.3f%% [%s%02u:%02u/%s%02u:%02u]",
                                 tmp,strbuf_el,(elapsed%3600)/60,elapsed%60,
                                 strbuf_es,(estimated%3600)/60,estimated%60);
            fflush(stdout);
          };
          if(errcode){
            fprintf(stderr,"\nError!\n");
            fflush(stderr);
          }
          else{
            fprintf(stdout,"\n");
            fflush(stdout);
          };
#ifndef _WIN32
          if(!stopped) pthread_mutex_unlock(&th_in.lock);
          pthread_join(th_id,NULL);
#else
          if(!stopped){
            WaitForSingleObjectEx(th_id,INFINITE,false);
            ReleaseMutex(th_in.lock);
          };
          CloseHandle(th_id);
#endif
          break;
        };
      };
    };
    memset(strbuf_el,0,12);
    memset(strbuf_es,0,12);
  }
  else{
    th_in.xs->archive(th_in.input_file,th_in.opcode,th_in.unarc_null);
    errcode=th_in.xs->errcode;
  };
  return;
}

#ifndef _WIN32
void xs_initializing(th_archiver_helper &th_in, pthread_t &th_id,
                     pthread_attr_t &th_attr,
                     bool &stats, int &errcode){
#else
void xs_initializing(th_archiver_helper &th_in, HANDLE &th_id,
                     bool &stats, int &errcode){
#endif
  if(stats){
    uint32_t cycles;
    struct timeval tv_start,tv_stop;
    double tmp=0;
    uint32_t elapsed=0;
    uint32_t estimated=0;
    char strbuf_el[12];
    char strbuf_es[12];
    bool stopped=false;
    gettimeofday(&tv_start,NULL);
#ifndef HAVE_HASH_TBL
    uint32_t base_cycles=0x865b8946;
#else
    uint32_t base_cycles=0x6ff6f265;
#endif
    cycles=0;
    th_in.output_file=NULL;
    th_in.xs->flt.cycles=&cycles;
#ifndef _WIN32
    pthread_mutex_unlock(&th_in.lock);
#endif
#ifndef _WIN32
    if(pthread_create(&th_id,&th_attr,th_xs_set,(void *)&th_in)){
      errcode=0x04000000;
    }
#else
    th_id=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)th_xs_set,
      (LPVOID *)&th_in,0,NULL);
    if(th_id==NULL){
      errcode=0x04000000;
    }
#endif
    else{
      fprintf(stdout,"\n");
      while((!cycles)&&(!th_in.xs->flt.errcode)){
#ifndef _WIN32
        if(pthread_kill(th_id,0)!=0){
          stopped=true;
          break;
        };
        usleep(1000);
#else
        if(WaitForSingleObjectEx(th_id,0,false)!=WAIT_TIMEOUT){
          stopped=true;
          break;
        };
        Sleep(10);
#endif
      };
      while(1){
        clrline();
        fprintf(stdout,"Initializing:");
        fflush(stdout);
#ifndef _WIN32
        if((!stopped)&&(pthread_mutex_trylock(&th_in.lock)==EBUSY)){
#else
        if((!stopped)&&(WaitForSingleObjectEx(th_in.lock,0,false)!=WAIT_OBJECT_0)){
#endif
          gettimeofday(&tv_stop,NULL);
          elapsed=tv_stop.tv_sec-tv_start.tv_sec+(tv_stop.tv_usec-tv_start.tv_usec)/1000000;
          tmp=(double)((double)cycles/(double)base_cycles)*100;
          if(tmp!=0) estimated=(uint32_t)((elapsed/tmp)*100.0);
          uint32_t el_h=elapsed/3600;
          uint32_t es_h=estimated/3600;
          strbuf_el[0]=0;
          strbuf_es[0]=0;
          if(el_h) sprintf(strbuf_el,"%u:",el_h);
          if(es_h) sprintf(strbuf_es,"%u:",es_h);
          fprintf(stdout,"%7.3f%% [%s%02u:%02u/%s%02u:%02u]",
                  tmp,strbuf_el,(elapsed%3600)/60,elapsed%60,
                  strbuf_es,(estimated%3600)/60,estimated%60);
          fflush(stdout);
#ifndef _WIN32
          usleep(300000);
#else
          Sleep(300);
#endif
        }
        else{
          errcode=th_in.xs->flt.errcode;
          if(!stopped){
            elapsed=tv_stop.tv_sec-tv_start.tv_sec+(tv_stop.tv_usec-tv_start.tv_usec)/1000000;
            tmp=(double)((double)cycles/(double)base_cycles)*100;
            uint32_t el_h=elapsed/3600;
            strbuf_el[0]=0;
            if(el_h) sprintf(strbuf_el,"%u:",el_h);
            fprintf(stdout,"%7.3f%% [%s%02u:%02u/%s%02u:%02u]",
                    tmp,strbuf_el,(elapsed%3600)/60,elapsed%60,
                    strbuf_el,(elapsed%3600)/60,elapsed%60);
            fflush(stdout);
          };
          if(errcode){
            fprintf(stderr,"\nError!\n");
            fflush(stderr);
          }
          else{
            fprintf(stdout,"\n");
            fflush(stdout);
          }
#ifndef _WIN32
          if(!stopped) pthread_mutex_unlock(&th_in.lock);
          pthread_join(th_id,NULL);
#else
          if(!stopped){
            WaitForSingleObjectEx(th_id,INFINITE,false);
            ReleaseMutex(th_in.lock);
          };
          CloseHandle(th_id);
#endif
          break;
        };
      };
    };
    memset(strbuf_el,0,12);
    memset(strbuf_es,0,12);
  }
  else{
    th_in.xs->flt.set(th_in.input_file);
    errcode=th_in.xs->flt.errcode;
  };
  return;
}

int main(int argc, char *argv[]){
  int errcode=0;
#ifndef _WIN32
  uid=getuid();
  if(uid==0){
    if(mlockall(MCL_FUTURE)<0) errcode=0x08000000;
  };
#endif
  archiver xs;
  xs.set_checksum_operator(checksum_print);
  xs.set_list_operator(list_print);
  xs.set_list_summary_operator(list_summary_print);
#ifndef _WIN32
  pthread_t th_id;
  pthread_attr_t th_attr;
#else
  HANDLE th_id;
#endif
  th_archiver_helper th_in;
  int optc;
  bool emode=true;
  bool stats=false;
  uint32_t softerr=0;
  uint32_t harderr=0;
  char *pass=NULL;
  char *errhlp=NULL;
  unsigned char mode=0;
  const char *input_file=NULL;
  const char *output_file=NULL;
  glob64_t *globbuf=NULL;
  program_name=argv[0];
#ifndef _WIN32
  if(!errcode){
#endif
  while((optc=getopt_long(argc,argv,"ed::l::cru::ngmjp:i:o:vsh",long_options,NULL))!=-1){
    switch(optc){
      case  0 : break;
      case 'e': if(mode){
                  errcode=0x00000020;
                  break;
                };
                mode=1;
                break;
      case 'd': if(mode){
                  errcode=0x00000020;
                  break;
                };
                mode=2;
                th_in.unarc_null=0;
                if(optarg!=NULL){
                  if(strcmp(optarg,"0")!=0){
                    errcode=0x00000008;
                    break;
                  }
                  else th_in.unarc_null=1;
                };
                break;
      case 'l': if(mode){
                  errcode=0x00000020;
                  break;
                };
                mode=3;
                if(optarg!=NULL){
                  if(strcmp(optarg,"0")!=0){
                    errcode=0x00000008;
                    break;
                  }
                  else{
                    xs.set_checksum_operator(NULL);
                    xs.set_list_operator(NULL);
                    xs.set_list_summary_operator(NULL);
                  };
                };
                break;
      case 'c': if(mode){
                  errcode=0x00000020;
                  break;
                };
                if(xs.flt.ignore_rc){
                  errcode=0x00000008;
                  break;
                };
                mode=4;
                break;
      case 'r': if(mode){
                  errcode=0x00000020;
                  break;
                };
                if(xs.flt.ignore_rc){
                  errcode=0x00000008;
                  break;
                };
                mode=5;
                break;
      case 'u': if(mode){
                  errcode=0x00000020;
                  break;
                };
                mode=6;
                if(optarg!=NULL){
                  globbuf=glob_getlist(optarg,errcode);
                  if(globbuf!=NULL){
                    for(uint32_t i=0;i<globbuf->gl_pathc;i++){
                      xs.add_name(globbuf->gl_pathv[i]);
                      errcode=xs.errcode;
                      if(errcode) break;
                    };
                    glob_freelist(globbuf);
                  }
                  else{
                    if(errcode==0) errcode=0x00000040;
                  };
                  if(errcode) errhlp=optarg;
                  input_file=optarg;
                };
                break;
      case 'n': if(mode==6){
                  errcode=0x00000008;
                  break;
                };
                if(xs.ignore_pt){
                  errcode=0x00000020;
                  break;
                };
                xs.ignore_pt=true;
                break;
      case 'g': if(mode==6){
                  errcode=0x00000008;
                  break;
                };
                if(xs.flt.ignore_sn){
                  errcode=0x00000020;
                  break;
                };
                xs.flt.ignore_sn=true;
                break;
      case 'm': if((mode==4)||(mode==5)||(mode==6)){
                  errcode=0x00000008;
                  break;
                };
                if(xs.flt.ignore_rc){
                  errcode=0x00000020;
                  break;
                };
                xs.flt.ignore_rc=true;
                break;
      case 'j': if((mode==4)||(mode==5)||(mode==6)){
                  errcode=0x00000008;
                  break;
                };
                if(xs.flt.ignore_sn||xs.flt.ignore_rc){
                  errcode=0x00000020;
                  break;
                };
                xs.flt.ignore_sn=true;
                xs.flt.ignore_rc=true;
                break;
      case 'p': if((pass!=NULL)||(mode==6)){
                  errcode=0x00000020;
                  break;
                };
                if(optarg){
                  pass=strdup(optarg);
                  if(pass==NULL){
                    errcode=0x00000001;
                    break;
                  }
                  else{
                    memset(optarg,'x',strlen(optarg));
                    if(strlen(optarg)>6) optarg[6]=0;
                  };
#ifndef _WIN32
                  if(uid){
                    if(mlock(pass,strlen(pass))<0){
                      memset(pass,0,strlen(pass));
                      free(pass);
                      pass=NULL;
                      errcode=0x08000000;
                    };
                  };
#endif
                };
                break;
      case 'i': globbuf=glob_getlist(optarg,errcode);
                if(globbuf!=NULL){
                  for(uint32_t i=0;i<globbuf->gl_pathc;i++){
                    xs.add_name(globbuf->gl_pathv[i]);
                    errcode=xs.errcode;
                    if(errcode) break;
                  };
                  glob_freelist(globbuf);
                }
                else{
                  if(errcode==0) errcode=0x00000040;
                };
                if(errcode) errhlp=optarg;
                input_file=optarg;
                break;
      case 'o': if((output_file!=NULL)||(mode==6)){
                  errcode=0x00000020;
                  break;
                };
                output_file=optarg;
                break;
      case 's': if(stats||(mode==6)){
                  errcode=0x00000020;
                  break;
                };
                stats=true;
                break;
      case 'h': usage(0);
                return errcode;
      case 'v': version(stdout,PROGRAM_NAME,VERSION,AUTHORS);
                fclose(stdout);
                return errcode;
      default : errcode=0x00000010;
    };
    if(errcode) break;
  };
#ifndef _WIN32
  }
#endif
  if(errcode==0){
    if(mode!=6){
      th_in.xs=&xs;
      if(stats){
#ifndef _WIN32
        if(pthread_attr_init(&th_attr)){
          errcode=0x01000000;
        }
        else{
          pthread_attr_setdetachstate(&th_attr,PTHREAD_CREATE_JOINABLE);
          pthread_attr_setstacksize(&th_attr,16384);
          if(pthread_mutex_init(&th_in.lock,NULL)){
            pthread_attr_destroy(&th_attr);
#else
          th_in.lock=CreateMutex(NULL,FALSE,NULL);
          if(th_in.lock==NULL){
#endif
            errcode=0x02000000;
          };
        };
#ifndef _WIN32
      };
#endif
    };
  };
  if(!errcode){
    if(mode){
      if(mode==6){
        xs.checksums();
        errcode=xs.errcode;
        if(errcode==0x00002000) errcode=axisum(xs.flt.slag,NULL,checksum_print);
        emode=false;
      };
      if(mode==4){
        if(output_file||pass||xs.ignore_pt) errcode=0x00000020;
        else{
          xs.flt.set_check();
          if(input_file)
            if(access(input_file,F_OK)<0) errcode=0x00000040;
          if(!errcode){
            if(stats){
              char msg[]="\rChecking integrity of the encoded data";
              th_in.input_file=(char *)input_file;
              th_in.output_file=NULL;
              th_in.opcode=false;
#ifndef _WIN32
              pthread_mutex_unlock(&th_in.lock);
              if(pthread_create(&th_id,&th_attr,th_xs_check,(void *)&th_in)){
                errcode=0x04000000;
              }
#else
              th_id=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)th_xs_check,
                                (LPVOID *)&th_in,0,NULL);
              if(th_id==NULL){
                errcode=0x04000000;
              }
#endif
              else{
                while(!th_in.opcode)
#ifndef _WIN32
                usleep(1000);
#else
                Sleep(10);
#endif
                msg_cycle(th_in,th_id,errcode,softerr,harderr,msg);
              };
            }
            else{
              xs.flt.transcode(input_file,NULL);
              errcode=xs.flt.errcode;
            };
          };
        };
        emode=false;
      };
      if(mode==5){
        if(pass||xs.ignore_pt) errcode=0x00000020;
        else{
          xs.flt.set_correct();
          if(input_file)
            if(access(input_file,F_OK)<0) errcode=0x00000040;
          if(!errcode)
            if(output_file)
              if(access(output_file,F_OK)==0) errcode=0x00000080;
          if(!errcode){
            if(stats){
              char msg[]="\rRepairing of the encoded data";
              th_in.input_file=(char *)input_file;
              th_in.output_file=(char *)output_file;
              th_in.opcode=false;
#ifndef _WIN32
              pthread_mutex_unlock(&th_in.lock);
              if(pthread_create(&th_id,&th_attr,th_xs_repair,(void *)&th_in)){
                errcode=0x04000000;
              }
#else
              th_id=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)th_xs_repair,
                                (LPVOID *)&th_in,0,NULL);
              if(th_id==NULL){
                errcode=0x04000000;
              }
#endif
              else{
                while(!th_in.opcode)
#ifndef _WIN32
                usleep(1000);
#else
                Sleep(10);
#endif
                msg_cycle(th_in,th_id,errcode,softerr,harderr,msg);
              };
            }
            else{
              xs.flt.transcode(input_file,output_file);
              errcode=xs.flt.errcode;
            };
          };
        };
        emode=false;
      };
      if(!errcode){
        if(emode){
          if(pass==NULL){
            char *pass_verify=NULL;
            for(;;){
              fprintf(stdout,"Enter   password (will be not shown):");
              fflush(stdout);
              pass=getpasswd();
              printf("\n");
              if(pass&&(strlen(pass)<6)){
                memset(pass,0,strlen(pass));
#ifndef _WIN32
                if(uid) munlock(pass,256);
#endif
                free(pass);
                pass=NULL;
                fprintf(stdout,"The password is too short! Please, try again.\n");
                fflush(stdout);
                continue;
              };
              if(pass&&(mode==1)){
                fprintf(stdout,"Reenter password (will be not shown):");
                fflush(stdout);
                pass_verify=getpasswd();
                printf("\n");
                if(pass_verify){
                  if(strcmp(pass,pass_verify)==0){
                    memset(pass_verify,0,strlen(pass));
#ifndef _WIN32
                    if(uid) munlock(pass_verify,256);
#endif
                    free(pass_verify);
                    pass_verify=NULL;
                    break;
                  }
                  else{
                    memset(pass_verify,0,strlen(pass));
#ifndef _WIN32
                    if(uid) munlock(pass_verify,256);
#endif
                    free(pass_verify);
                    pass_verify=NULL;
                    memset(pass,0,strlen(pass));
#ifndef _WIN32
                    if(uid) munlock(pass,256);
#endif
                    free(pass);
                    pass=NULL;
                    fprintf(stdout,"Mismatch! Please, try again.\n");
                    fflush(stdout);
                  };
                }
                else{
                  memset(pass,0,strlen(pass));
#ifndef _WIN32
                  if(uid) munlock(pass,256);
#endif
                  free(pass);
                  pass=NULL;
                };
              }
              else break;
            };
#ifndef _WIN32
          };
#else
          }
          else{
            char *tmp=codepagetocodepage(pass,CP_ACP,CP_UTF8);
            free(pass);
            pass=tmp;
          };
#endif
          if(!pass) errcode=0x0000000f;
          else if(strlen(pass)<6) errcode=0x0000000f;
        };
      };
      if(!errcode){
        if(mode==2){
          if(output_file) xs.set_base(output_file);
          errcode=xs.errcode;
          if(!errcode){
            th_in.input_file=(char *)pass;
            xs.flt.set_decrypt();
#ifndef _WIN32
            xs_initializing(th_in,th_id,th_attr,stats,errcode);
#else
            xs_initializing(th_in,th_id,stats,errcode);
#endif
            memset(pass,0,strlen(pass));
#ifndef _WIN32
            if(uid) munlock(pass,strlen(pass));
#endif
            free(pass);
            pass=NULL;
          };
          if(!errcode){
            th_in.input_file=(char *)input_file;
            th_in.opcode=true;
#ifndef _WIN32
            xs_archive(th_in,th_id,th_attr,stats,errcode);
#else
            xs_archive(th_in,th_id,stats,errcode);
#endif
          };
          emode=false;
        };
        if(mode==1){
          if(xs.ignore_pt) errcode=0x00000020;
          else{
            if(output_file)
              if(access(output_file,F_OK)==0) errcode=0x00000080;
            if(!errcode){
              th_in.input_file=(char *)pass;
#ifndef _WIN32
              xs_initializing(th_in,th_id,th_attr,stats,errcode);
#else
              xs_initializing(th_in,th_id,stats,errcode);
#endif
              memset(pass,0,strlen(pass));
#ifndef _WIN32
              if(uid) munlock(pass,strlen(pass));
#endif
              free(pass);
              pass=NULL;
            };
            if(!errcode){
              th_in.input_file=(char *)output_file;
              th_in.opcode=false;
#ifndef _WIN32
              xs_archive(th_in,th_id,th_attr,stats,errcode);
#else
              xs_archive(th_in,th_id,stats,errcode);
#endif
            };
          };
          emode=false;
        };
        if(mode==3){
          if(output_file||xs.ignore_pt) errcode=0x00000020;
          else{
            if(!errcode){
              th_in.input_file=(char *)pass;
              xs.flt.set_decrypt();
#ifndef _WIN32
              xs_initializing(th_in,th_id,th_attr,stats,errcode);
#else
              xs_initializing(th_in,th_id,stats,errcode);
#endif
              memset(pass,0,strlen(pass));
#ifndef _WIN32
              if(uid) munlock(pass,strlen(pass));
#endif
              free(pass);
              pass=NULL;
              errcode=xs.flt.errcode;
            };
            if(!errcode) xs.archive(input_file,true,(uint8_t)2);
          };
          emode=false;
        };
      };
      if(!errcode && emode) errcode=0x00000008;
    }
    else errcode=0x00000008;
  };
  if(mode!=6){
    if(stats){
#ifndef _WIN32
      pthread_mutex_destroy(&th_in.lock);
      pthread_attr_destroy(&th_attr);
#else
      ReleaseMutex(th_in.lock);
      CloseHandle(th_in.lock);
#endif
    };
  };
  if((errcode==0)&&(mode!=6)) errcode=xs.errcode;
  if((errcode==0)&&(mode!=6)){
    softerr=xs.flt.corrector.softerr;
    harderr=xs.flt.corrector.harderr;
    if(softerr) errcode|=0x00008000;
    if(harderr) errcode|=0x00100000;
  };
  switch(errcode){
    case 0x00000000: break;
    case 0x00000001: error("memory allocation failed");
                     break;
    case 0x00000002: error("initialization failed");
                     break;
    case 0x00000004: error("read error");
                     break;
    case 0x00000008: error("wrong mode");
                     usage(1);
                     break;
    case 0x0000000f: error("the password is too short");
                     usage(1);
                     break;
    case 0x00000010: usage(1);
                     break;
    case 0x00000020: error("too many arguments");
                     usage(1);
                     break;
    case 0x00000040: error("input file/directory is not accessible");
                     if(errhlp){
                       fprintf(stderr,"\nUnable to open file/directory: \"%s\"!\n\n",input_file);
                     };
                     break;
    case 0x00000080: error("output file already exists");
                     break;
    case 0x000000f0: error("output file is not accessible");
                     break;
    case 0x00000100: error("input file is not XS-file (try \"--jam\" key)");
                     break;
    case 0x00000200: error("write error");
                     break;
    case 0x00000400: error("access failed or bad file");
                     break;
    case 0x00000800: error("broken file or I/O error");
                     break;
    case 0x00000f00: error("unable to create directory");
                     break;
    case 0x00001000: error("unable to change directory");
                     break;
    case 0x00002000: error("no data for archiving/processing");
                     break;
    case 0x00004000: error("input file was changed");
                     break;
    case 0x00010000: error("unable to change file access permissions");
                     break;
    case 0x00020000: error("unable to set file access and modification times");
                     break;
    case 0x00040000: error("unable to change ownership of a file/directory");
                     break;
    case 0x00080000: error("unable to get current directory name");
                     break;
    case 0x01000000: error("unable to set thread attributes");
                     break;
    case 0x02000000: error("unable to create mutex");
                     break;
    case 0x04000000: error("unable to create thread");
                     break;
#ifndef _WIN32
    case 0x08000000: error("unable to lock memory");
                     break;
#endif
    default        : if(errcode&0x00008000){
                       error("single errors detected");
                       fprintf(stderr,"Err.num.: %d\n",softerr);
                       fflush(stderr);
                     };
                     if(errcode&0x00100000){
                       error("multiple errors detected");
                       fprintf(stderr,"Err.num.: %d\n",harderr);
                       fflush(stderr);
                     };
                     break;
  };
#ifndef _WIN32
  if(uid==0) munlockall();
#endif
  return errcode;
}
