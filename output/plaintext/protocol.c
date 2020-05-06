#include "output/plaintext/protocol.h"
#include "output/plaintext/stdio.h"
#include "output/plaintext/stipulation.h"
#include "output/plaintext/sstipulation.h"

#include <stdarg.h>
#include <stdio.h>

static FILE *TraceFile;
static char const *open_mode = "a";

/* Remember for this run's proptocol (if any) to overwrite (rather than append
 * to) a previous run's protocol (if any)
 */
void protocol_overwrite(void)
{
  open_mode = "w";
}

/* Open a new protocol file
 * @param filename name of protocol file
 * @return the opened file (for writing some intro text)
 *         0 if it couln't be opened
 * @note the previous protocol file (if any) is closed
 */
FILE *protocol_open(char const *filename)
{
  if (TraceFile!=NULL)
    fclose(TraceFile);

  TraceFile = fopen(filename,open_mode);

  return TraceFile;
}

/* Get the current protocol file
 * @return the opened file
 *         0 if no file opened
 */
FILE *protocol_get(void)
{
  return TraceFile;
}

/* Close the current protocol file, if one is opened; NOP otherwise
 * @return the return value of fclose if a file were opened, 0 otherwise
 */
int protocol_close(void)
{
  int ret = 0;
  if (TraceFile)
  {
    ret = fclose(TraceFile);
    TraceFile = NULL;
  }
  return ret;
}

/* like putchar().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fputc(int c, FILE *regular)
{
  int const result = fputc(c,regular);

  if (TraceFile!=0)
    fputc(c,TraceFile);

  return result;
}

/* like vprintf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_vfprintf(FILE *regular, char const *format, va_list args)
{
  int result;

  if (TraceFile==0)
    result = vfprintf(regular,format,args);
  else
  {
    va_list args2;
    va_copy(args2,args);
    result = vfprintf(regular,format,args);
    vfprintf(TraceFile,format,args2);
    va_end(args2);
  }

  return result;
}

/* like printf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fprintf(FILE *regular, char const *format, ...)
{
  int result;

  va_list args;
  va_start(args,format);
  result = protocol_vfprintf(regular,format,args);
  va_end(args);

  return result;
}

/* like fprintf_r().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fprintf_r(FILE *regular, int width, char const *format, ...)
{
  int result;
  va_list args;

  va_start(args,format);

  if (TraceFile==0)
    result = vfprintf_r(regular,width,format,args);
  else
  {
    va_list args2;

    va_copy(args2,args);
    result = vfprintf_r(regular,width,format,args);
    vfprintf_r(TraceFile,width,format,args2);
    va_end(args2);
  }

  va_end(args);

  return result;
}

/* like fprintf_c().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fprintf_c(FILE *regular, int width, char const *format, ...)
{
  int result;
  va_list args;

  va_start(args,format);

  if (TraceFile==0)
    result = vfprintf_c(regular,width,format,args);
  else
  {
    va_list args2;

    va_copy(args2,args);
    result = vfprintf_c(regular,width,format,args);
    vfprintf_c(TraceFile,width,format,args2);
    va_end(args2);
  }

  va_end(args);

  return result;
}

/* like fputs_c_multi().
 * If a trace file is active, output goes to the trace file as well
 */
void protocol_fputs_c_multi(FILE *regular, int width, char const *lines)
{
  fputs_c_multi(regular,width,lines);

  if (TraceFile!=0)
    fputs_c_multi(TraceFile,width,lines);
}

/* write a stipulation
 * If a trace file is active, output goes to the trace file as well
 */
unsigned int protocol_write_stipulation(FILE *regular, slice_index si)
{
  unsigned int const result = WriteStipulation(regular,si);

  if (TraceFile!=0)
    WriteStipulation(TraceFile,si);

  return result;
}

/* write a stipulation
 * If a trace file is active, output goes to the trace file as well
 */
unsigned int protocol_write_sstipulation(FILE *regular, slice_index si)
{
  unsigned int const result = WriteSStipulation(regular,si);

  if (TraceFile!=0)
    WriteSStipulation(TraceFile,si);

  return result;
}

/* like fflush().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fflush(FILE *regular)
{
  if (TraceFile!=0)
    fflush(TraceFile);

  return fflush(regular);
}
