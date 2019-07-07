#include "permutation.h"

void permutation::set(){
  if(z){
    for(int32_t i=1; i<=n; i++){
      z[i]=p[i]=i;
      d[i]=-1;
    };
    d[1]=0;
    z[0]=z[n+1]=m=n+1;
  };
}

permutation::permutation(unsigned char *src, int32_t length){
  n=length;
  if(n){
    in=src;
    z=(int32_t *)calloc(n+2,sizeof(int32_t));
    if(z){
      p=(int32_t *)calloc(n+2,sizeof(int32_t));
      if(p){
        d=(int32_t *)calloc(n+2,sizeof(int32_t));
        if(d==NULL){
          free(p);
	        free(z);
	        z=NULL;
	      };
      }
      else{
        free(z);
      	z=NULL;
      };
    }
    else z=NULL;
  }
  else z=NULL;
  if(z==NULL){
    in=NULL;
    n=0;
    p=NULL;
    d=NULL;
  }
  else set();
}

permutation::~permutation(){
  if(z){
    n+=2;
    memset(z,0,n);
    free(z);
    z=NULL;
    memset(p,0,n);
    free(p);
    p=NULL;
    memset(d,0,n);
    free(d);
    d=NULL;
    in=NULL;
    n=0;
    m=0;
  };
}

void permutation::next(){
  if(z==NULL) return;
  if(m==1){
    set();
    next();
  }
  else{
    int32_t pm,dm,w;
    unsigned char c;
    m=n;
    while(z[p[m]+d[m]]>m){
      d[m]=-d[m];
      m--;
    };
    pm=p[m];
    dm=pm+d[m];
    w=z[pm];
    z[pm]=z[dm];
    z[dm]=w;
    c=in[z[pm]-1];
    in[z[pm]-1]=in[z[dm]-1];
    in[z[dm]-1]=c;
    dm=z[pm];
    w=p[dm];
    p[dm]=pm;
    p[m]=w;
  };
  return;
}
