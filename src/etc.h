#ifndef _ETC_H
#define _ETC_H 1

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32

  #include <windows.h>
  #include <wchar.h>

  wchar_t *codepagetowchar(const char *src, UINT cp);
  char *wchartocodepage(const wchar_t *src, UINT cp);
  char *codepagetocodepage(const char *src, UINT src_cp, UINT dst_cp);

#else

  #include <sys/ioctl.h>

#endif

  void clrline(uint32_t w=50);
  void version(FILE *stream, const char *command_name, const char *version,
               const char *authors);
  void error(const char *err_message);

#endif
