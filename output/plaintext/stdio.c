#include "output/plaintext/stdio.h"
#include "utilities/boolean.h"

#include <string.h>

/* Like vfprintf(), but writes right-aligned into a space of the indicated
 * width.
 * format and argument strings shouldn't contain line breaks (except maybe at
 * the end) or the output will look "interesting".
 * vfprintf_r() performs two conversion runs, the first for measuring and the
 * second for writing. If the return value is <0, the caller can't determine
 * which conversion run was the cause.
 */
int vfprintf_r(FILE *file, int width, char const *format, va_list args)
{
  int result;
  va_list args2;

  va_copy(args2,args);

  {
    int const used = vsnprintf(0,0,format,args);
    if (used<0)
      result = used;
    else
    {
      if (width>used)
        fprintf(file,"%*c",width-used,' ');
      result = vfprintf(file,format,args2);
    }
  }

  va_end(args2);

  return result;
}

/* Like fprintf(), but writes right-aligned into a space of the indicated
 * width.
 * format and argument strings shouldn't contain line breaks (except maybe at
 * the end) or the output will look "interesting".
 * fprintf_r() performs two conversion runs, the first for measuring and the
 * second for writing. If the return value is <0, the caller can't determine
 * which conversion run was the cause.
 */
int fprintf_r(FILE *file, int width, char const *format, ...)
{
  int result;
  va_list args;

  va_start(args,format);
  result = vfprintf_r(file,width,format,args);
  va_end(args);

  return result;
}

/* like vfprintf(), but writes centered into a space of the indicated width
 * format and argument strings shouldn't contain line breaks (except maybe at
 * the end) or the output will look "interesting".
 * vfprintf_c() performs two conversion runs, the first for measuring and the
 * second for writing. If the return value is <0, the caller can't determine
 * which conversion run was the cause.
 */
int vfprintf_c(FILE *file, int width, char const *format, va_list args)
{
  int result;
  va_list args2;

  va_copy(args2,args);

  {
    int const used = vsnprintf(0,0,format,args);
    if (used<0)
      result = used;
    else
    {
      if (width>used)
        /* adding 1 or not is a matter of taste */
        fprintf(file,"%*c",(width+1-used)/2,' ');
      result = vfprintf(file,format,args2);
    }
  }

  va_end(args2);

  return result;
}

/* like fprintf(), but writes centered into a space of the indicated width
 * format and argument strings shouldn't contain line breaks (except maybe at
 * the end) or the output will look "interesting".
 * fprintf_c() performs two conversion runs, the first for measuring and the
 * second for writing. If the return value is <0, the caller can't determine
 * which conversion run was the cause.
 */
int fprintf_c(FILE *file, int width, char const *format, ...)
{
  int result;
  va_list args;

  va_start(args,format);
  result = vfprintf_c(file,width,format,args);
  va_end(args);

  return result;
}

/* write a series of centered lines using fprintf_c()
 * @param lines string consisting of >=1 lines separated by \n
 * @note ignores return values from <stdio.h> functions invoked
 */
void fputs_c_multi(FILE *file, int width, char const *lines)
{
  char const *start_of_line = lines;

  while (true)
  {
    char const * const end_of_line = strchr(start_of_line,'\n');
    if (end_of_line==0)
      break;
    else
    {
      char const * const next_start_of_line = end_of_line+1;
      fprintf_c(file,width,"%.*s",
                (int)(next_start_of_line-start_of_line),start_of_line);
      start_of_line = next_start_of_line;
    }
  }
}
