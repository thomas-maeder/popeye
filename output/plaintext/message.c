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

static char *AdditionalArg;
/* This is used to record an argument which is used
 * in a message-string. There are only message strings
 * that contain at most one format specifier.  Therefore
 * one pointer is sufficient.
 * Three small routines are provided to assign a value:
 */

void logStrArg(char *arg)
{
  AdditionalArg= arg;
}

void logIntArg(int arg)
{
  static char IntBuffer[10];
  sprintf(IntBuffer, "%d", arg);
  AdditionalArg= IntBuffer;
}

void logLngArg(long arg)
{
  static char LngBuffer[16];
  sprintf(LngBuffer, "%ld", arg);
  AdditionalArg= LngBuffer;
}

void logChrArg(char arg)
{
  static char CharBuffer[2];
  CharBuffer[0]= arg;
  CharBuffer[1]= '\0';
  AdditionalArg= CharBuffer;
}

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

void Message(message_id_t id)
{
  DBG((stderr, "Mesage(%d) = %s\n", id, GetMsgString(id)));
  if (id<StringCnt)
    fprintf(stdout,GetMsgString(id), AdditionalArg);
  else
    fprintf(stdout,GetMsgString(InternalError), id);
  if (TraceFile)
  {
    if (id<StringCnt)
      fprintf(TraceFile,GetMsgString(id), AdditionalArg);
    else
      fprintf(TraceFile,GetMsgString(InternalError), id);
  }
  AdditionalArg= NULL;
}

void Message2(FILE *file, message_id_t id)
{
  DBG((stderr, "Mesage(%d) = %s\n", id, GetMsgString(id)));
  if (id<StringCnt)
    fprintf(file,GetMsgString(id),AdditionalArg);
  else
    fprintf(file,GetMsgString(InternalError),id);
  AdditionalArg = NULL;
}

void ErrorMsg(message_id_t id)
{
  DBG((stderr, "ErrorMsg(%d) = %s\n", id, GetMsgString(id)));
  if (id<StringCnt)
    sprintf(GlobalStr, GetMsgString(id), AdditionalArg);
  else
    sprintf(GlobalStr, GetMsgString(InternalError), id);
  AdditionalArg= NULL;
  ErrString(GlobalStr);
}

void FtlMsg(message_id_t id)
{
  char *SaveArg= 0;
  if (AdditionalArg)
    SaveArg= AdditionalArg;
  ErrorMsg(ErrFatal);
  ErrorMsg(NewLine);
  if (SaveArg)
    AdditionalArg= SaveArg;
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

static void pyfputs(char const *s, FILE *f)
{
#if !defined(QUIET)
  fputs(s,f);
  fflush(f);
  if (TraceFile) {
    fputs(s,TraceFile);
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
  ErrorMsg(InputError);
  logIntArg(val);
  ErrorMsg(n);
  ErrChar('\n');
  logStrArg(InputLine);
  ErrorMsg(OffendingItem);
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

void ErrString(char const *s)
{
  pyfputs(s, stderr);
}
