#ifndef _TIMEBITS_H
#define _TIMEBITS_H 1

#include <sys/types.h>
#include <stdint.h>
#ifndef _MSC_VER
  #include <sys/time.h>
#endif
#include <time.h>

#ifndef _WIN32
  typedef struct timespec stopwatch_t;
  #if defined(CLOCK_MONOTONIC_RAW)
  #   define STOPWATCH_CLOCK_TYPE CLOCK_MONOTONIC_RAW
  #else
    #define STOPWATCH_CLOCK_TYPE CLOCK_MONOTONIC
  #endif
  #define TIME_TYPE stopwatch_t
  #define GET_TIME(s) (clock_gettime(STOPWATCH_CLOCK_TYPE,&(s)))
  #define GET_TIME_LAST_2BYTES(s) ((uint32_t)((s).tv_nsec&0xffff))
  #define GET_TIME_LAST_BIT(s) ((uint32_t)((s).tv_nsec&1))
#else
  #include <windows.h>
  #define TIME_TYPE LARGE_INTEGER
  #define GET_TIME(s) (QueryPerformanceCounter(&(s)))
  #define GET_TIME_LAST_2BYTES(s) ((uint32_t)((s).QuadPart&0xffff))
  #define GET_TIME_LAST_BIT(s) ((uint32_t)((s).QuadPart&1))
#endif

  inline double pi(uint32_t t); //фукция вычисления числа пи
  uint32_t timeshift();
  uint32_t timebits();   //функция получения 32-бит "случайных" значений (исходя из времени выполнения)

#endif
