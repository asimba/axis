#include "signature.h"
#include <unistd.h>

int axis_sign(int fildes, bool check){
  unsigned char original_sign[]={0x78,0x73,0x03,0x99};
  unsigned char target_sign[4];
  int c=0;
  if(check){
    if(read(fildes,target_sign,4)<4) c=-1;
    else{ 
      for(c=0; c<4; c++){
        if(original_sign[c]!=target_sign[c]){
          c=-1;
          break;
        };
      };
    };
  }
  else{
    if(write(fildes,original_sign,4)<4) c=-1;
  };
  return c;
}
