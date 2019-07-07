#ifndef __GETPASSWD_H
#define __GETPASSWD_H 1

#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#ifndef _WIN32
  #include <termios.h>
#else
  #include <conio.h>
#endif

  char *getpasswd();

#endif

