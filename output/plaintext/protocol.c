#include "output/plaintext/protocol.h"
#include "output/plaintext/plaintext.h"

#include <stdio.h>

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
