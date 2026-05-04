#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"

#ifndef _GCC_

int DEBUG(char *format, ...)
{
  int ret;
#ifdef _DEBUG_
  va_list ap;

  va_start(ap, format);
  ret = vfprintf(stderr, format, ap);
  va_end(ap);
#else
  ret = 0;
#endif

  return ret;
}

int DBG(char *format, ...)
{
  int ret;
#ifdef _DEBUG_
  va_list ap;

  va_start(ap, format);
  ret = vfprintf(stderr, format, ap);
  va_end(ap);
#else
  ret = 0;
#endif

  return ret;
}

#endif /* _GCC_ */

