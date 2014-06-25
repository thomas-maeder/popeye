#include "output/plaintext/protocol.h"
#include "output/plaintext/stdio.h"
#include "output/plaintext/plaintext.h"

#include <stdio.h>

/* like putchar().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to standard output
 */
int protocol_putchar(int c)
{
  int const result = putchar(c);

  if (TraceFile!=0)
    fputc(c,TraceFile);

  return result;
}

/* like vprintf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to standard output
 */
int protocol_vprintf(char const *format, va_list args)
{
  int result;

  if (TraceFile==0)
    result = vfprintf(stdout,format,args);
  else
  {
    va_list args2;
    va_copy(args2,args);
    result = vfprintf(stdout,format,args);
    vfprintf(TraceFile,format,args2);
    va_end(args2);
  }

  return result;
}

int protocol_printf(char const *format, ...)
{
  int result;

  va_list args;
  va_start(args,format);
  result = protocol_vprintf(format,args);
  va_end(args);

  return result;
}

int protocol_printf_r(int width, char const *format, ...)
{
  int result;
  va_list args;

  va_start(args,format);

  if (TraceFile==0)
    result = vfprintf_r(stdout,width,format,args);
  else
  {
    va_list args2;

    va_copy(args2,args);
    result = vfprintf_r(stdout,width,format,args);
    vfprintf_r(TraceFile,width,format,args2);
    va_end(args2);
  }

  va_end(args);

  return result;
}

int protocol_printf_c(int width, char const *format, ...)
{
  int result;
  va_list args;

  va_start(args,format);

  if (TraceFile==0)
    result = vfprintf_c(stdout,width,format,args);
  else
  {
    va_list args2;

    va_copy(args2,args);
    result = vfprintf_c(stdout,width,format,args);
    vfprintf_c(TraceFile,width,format,args2);
    va_end(args2);
  }

  va_end(args);

  return result;
}

void protocol_puts_c_multi(int width, char const *lines)
{
  fputs_c_multi(stdout,width,lines);

  if (TraceFile!=0)
    fputs_c_multi(TraceFile,width,lines);
}

int protocol_flush(void)
{
  if (TraceFile!=0)
    fflush(TraceFile);

  return fflush(stdout);
}
