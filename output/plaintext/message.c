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
#include "output/latex/latex.h"
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
    sprintf(GlobalStr, GetMsgString(id), AdditionalArg);
  else
    sprintf(GlobalStr, GetMsgString(InternalError), id);
  AdditionalArg= NULL;
  StdString(GlobalStr);
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
  if (TextualSolutionBuffer) {
    fputc(c,TextualSolutionBuffer);
    fflush(TextualSolutionBuffer);
  }
#endif
}

void pyfputs(char const *s, FILE *f)
{
#if !defined(QUIET)
  fputs(s,f);
  fflush(f);
  if (TraceFile) {
    fputs(s,TraceFile);
    fflush(TraceFile);
  }
  if (TextualSolutionBuffer) {
    fputs(s,TextualSolutionBuffer);
    fflush(TextualSolutionBuffer);
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

char *MakeTimeString(void)
{
  unsigned long msec;
  unsigned long Seconds;
  unsigned long Minutes;
  unsigned long Hours;

  static char TmString[32];

  StopTimer(&Seconds,&msec);
  Hours= Seconds/3600;
  Minutes= (Seconds%3600)/60;
  Seconds= (Seconds%60);
  if (Hours>0)
  {
    sprintf(TmString,"%lu:%02lu:%02lu h:m:s",Hours,Minutes,Seconds);
  }
  else if (Minutes>0)
  {
    if (msec==MSEC_NOT_SUPPORTED)
      sprintf(TmString,"%lu:%02lu m:s", Minutes, Seconds);
    else
      sprintf(TmString,"%lu:%02lu.%03lu m:s", Minutes, Seconds, msec);
  }
  else
  {
    if (msec==MSEC_NOT_SUPPORTED)
      sprintf(TmString,"%lu s", Seconds);
    else
      sprintf(TmString,"%lu.%03lu s", Seconds, msec);
  }

  return TmString;
}

void PrintTime()
{
  if (!flag_regression) {
    StdString(GetMsgString(TimeString));
    StdString(MakeTimeString());
  }
}

void StdChar(char c)
{
  pyfputc(c, stdout);
}

void StdString(char const *s)
{
  pyfputs(s, stdout);
}

void ErrString(char const *s)
{
  pyfputs(s, stderr);
}
