/**
 * NOTE: CONTAINS PLATFORM DEPENDEND CODE
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#ifdef _WIN32
#else
#include <sys/time.h>
#endif

#include "port.h"

unsigned int port_rand(void)
{
  static int init_rand = 1;
  if (init_rand) {
    init_rand = 0;
    srand((unsigned int) time(0));
  }
  return (unsigned int) rand();
}

#if 0
void port_message(char *buf, unsigned int bufsiz, const char *format, ...)
{
  va_list args;
  va_start(args, format);
}

int port_timetostr(char *str, unsigned int len, long long int timestamp)
{
  struct tm ltime;
#if defined(_WIN64)
  __time64_t timestamp_second = (__time64_t) (timestamp / 1000);
  _localtime64_s(&ltime, &timestamp_second);
#elif  defined(_WIN32)
  __time32_t timestamp_second = (__time32_t) (timestamp / 1000);
  _localtime32_s(&ltime, &timestamp_second);
#else
  time_t timestamp_second = (time_t) (timestamp / 1000);
  localtime_r(&timestamp_second, &ltime);
#endif
  strftime(str, len, "%m/%d/%y %H:%M:%S", &ltime);
  int millisecond = (int) (timestamp % 1000);
  return snprintf(str, len, "%s.%03d", str, millisecond);
}
#endif

unsigned long long int port_current_time(void)
{
#if defined(_WIN32)
  FILETIME ftime;
  unsigned long long int ret;
  GetSystemTimeAsFileTime(&ftime);
  ret = ftime.dwHighDateTime;
  ret <<= 32Ui64;
  ret |= ftime.dwLowDateTime;
  return ret;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (unsigned long long int) (tv.tv_sec * 1000LL + tv.tv_usec / 1000);
#endif
}

char *port_strncpy(char *dst, const char *src, unsigned int dsiz)
{
  return strncpy(dst, src, (size_t) dsiz);
}

void *port_malloc(unsigned int size)
{
  void *ptr = malloc(size);
  memset(ptr, 0, (size_t) size);
  return ptr;
}

void port_free(void *ptr)
{
  free(ptr);
}

int port_strcmp(const char *str1, const char *str2)
{
  return strcmp(str1, str2);
}

char *port_strndup(const char *str, unsigned int maxsize)
{
  return strndup(str, maxsize);
}

int port_strncasecmp(const char *s1, const char *s2, unsigned int n)
{
  return strncasecmp(s1, s2, n);
}

unsigned int port_strnlen(const char *str, unsigned int maxsize)
{
  return (unsigned int) strnlen(str, (size_t) maxsize);
}

int port_inttostr(char *str, unsigned int size, long long int number)
{
  return snprintf(str, size, "%lld", number);
}
