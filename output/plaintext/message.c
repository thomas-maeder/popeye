#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(DOS)
#if defined(_MSC_VER)
# if defined(SHARING)        /* Import defines for locking regions  StH */
#  include <sys/locking.h>
# endif
#endif    /* _MSC_VER */
#endif    /* DOS */

#define PYMSG

#include "output/plaintext/message.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "platform/pytime.h"

typedef unsigned int UnInt;
typedef unsigned char UnChar;
typedef unsigned long UnLong;


#if !defined(SEEK_SET)
#    define SEEK_SET 0
#endif    /* not SEEK_SET */

static char const * const *ActualMsgTab;

static message_id_t StringCnt;

/* This is used to record an argument which is used
 * in a message-string. There are only message strings
 * that contain at most one format specifier.  Therefore
 * one pointer is sufficient.
 * Three small routines are provided to assign a value:
 */

boolean InitMsgTab(Language l)
{
  StringCnt= MsgCount;
  ActualMsgTab= MessageTabs[l];
  return true;
}

char const *GetMsgString(message_id_t id)
{
  return ActualMsgTab[id];
}

#if defined(DEBUG)
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

static void vMessage(FILE *file, message_id_t id, va_list args)
{
  if (id<StringCnt)
    vfprintf(file,GetMsgString(id),args);
  else
    fprintf(file,GetMsgString(InternalError),id);
}

void Message(message_id_t id, ...)
{
  va_list args;
  DBG((stderr, "Mesage(%d) = %s\n", id, GetMsgString(id)));

  va_start(args,id);

  if (TraceFile)
  {
    va_list args2;
    va_copy(args2,args);
    vMessage(stdout,id,args);
    vMessage(TraceFile,id,args2);
    va_end(args2);
  }
  else
    vMessage(stdout,id,args);

  va_end(args);
}

void Message2(FILE *file, message_id_t id, ...)
{
  va_list args;
  DBG((stderr, "Mesage(%d) = %s\n", id, GetMsgString(id)));
  va_start(args,id);
  vMessage(file,id,args);
  va_end(args);
}

void ErrorMsg(message_id_t id, ...)
{
  DBG((stderr, "ErrorMsg(%d) = %s\n", id, GetMsgString(id)));
#if !defined(QUIET)
  if (id<StringCnt)
  {
    va_list args;
    va_start(args,id);
    if (TraceFile)
    {
      va_list args2;
      va_copy(args2,args);
      vMessage(stderr,id,args);
      vMessage(TraceFile,id,args2);
      va_end(args2);
    }
    else
      vMessage(stderr,id,args);
    va_end(args);
  }
  else
  {
    fprintf(stderr,GetMsgString(InternalError),id);
    if (TraceFile)
      fprintf(TraceFile,GetMsgString(InternalError),id);
  }

  fflush(stderr);

  if (TraceFile)
    fflush(TraceFile);
#endif
}

void FtlMsg(message_id_t id)
{
  ErrorMsg(ErrFatal);
  ErrorMsg(NewLine);
  ErrorMsg(id);
  ErrorMsg(NewLine);
  exit(id);
}

void VerifieMsg(message_id_t id)
{
  ErrorMsg(id);
  ErrorMsg(NewLine);
  ErrorMsg(ProblemIgnored);
  ErrorMsg(NewLine);
}

static void pyfputc(char c, FILE *f)
{
#if !defined(QUIET)
  fputc(c,f);
  fflush(f);
  if (TraceFile) {
    fputc(c,TraceFile);
    fflush(TraceFile);
  }
#endif
}

static void ErrChar(char c)
{
  pyfputc(c, stderr);
}

void IoErrorMsg(message_id_t n, int val)
{
  fflush(stdout);
  ErrorMsg(InputError,val);
  ErrorMsg(n);
  ErrChar('\n');
  ErrorMsg(OffendingItem,InputLine);
  ErrChar('\n');
}

void FormatTime(FILE *file)
{
  unsigned long msec;
  unsigned long secs;
  StopTimer(&secs,&msec);

  {
    unsigned long const Hours = secs/3600;
    unsigned long const Minutes = (secs%3600)/60;
    unsigned long const Seconds = (secs%60);

    if (Hours>0)
      fprintf(file,"%lu:%02lu:%02lu h:m:s",Hours,Minutes,Seconds);
    else if (Minutes>0)
    {
      if (msec==MSEC_NOT_SUPPORTED)
        fprintf(file,"%lu:%02lu m:s", Minutes, Seconds);
      else
        fprintf(file,"%lu:%02lu.%03lu m:s", Minutes, Seconds, msec);
    }
    else
    {
      if (msec==MSEC_NOT_SUPPORTED)
        fprintf(file,"%lu s", Seconds);
      else
        fprintf(file,"%lu.%03lu s", Seconds, msec);
    }
  }
}

void PrintTime(FILE *file)
{
  if (!flag_regression)
  {
    fprintf(file,GetMsgString(TimeString));
    FormatTime(file);
  }
}
