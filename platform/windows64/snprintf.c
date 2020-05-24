/* It's 2013 and Microsoft still doesn't support snprintf ...
 * Thanks to the guys who posted in
 * http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
 */

/* these #includes are placed here rather than below the #ifdef because empty translation units are not allowed */
#include <stdarg.h>
#include <stdio.h>

#ifdef _MSC_VER

static int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
  int result = -1;

  if (size!=0)
    result = _vsnprintf_s(str,size,_TRUNCATE,format,ap);
  if (result==-1)
    result = _vscprintf(format,ap);

  return result;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
  int result;
  va_list ap;

  va_start(ap,format);
  result = c99_vsnprintf(str,size,format,ap);
  va_end(ap);

  return result;
}

#endif
