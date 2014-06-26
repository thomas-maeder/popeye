#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "platform/pytime.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(DEBUG)
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

void output_plaintext_message(message_id_t id, ...)
{
  DBG((stderr, "Mesage(%d) = %s\n", id, output_message_get(id)));

  if (id<MsgCount)
  {
    va_list args;
    va_start(args,id);
    protocol_vfprintf(stdout,output_message_get(id),args);
    va_end(args);
  }
  else
    output_plaintext_message(InternalError,id);
}

void output_plaintext_error_message(message_id_t id, ...)
{
  DBG((stderr, "ErrorMsg(%d) = %s\n", id, output_message_get(id)));
#if !defined(QUIET)
  if (id<MsgCount)
  {
    va_list args;
    va_start(args,id);
    protocol_vfprintf(stderr,output_message_get(id),args);
    va_end(args);
  }
  else
    output_plaintext_error_message(InternalError,id);

  protocol_fflush(stderr);
#endif
}

void output_plaintext_fatal_message(message_id_t id)
{
  output_plaintext_error_message(ErrFatal);
  output_plaintext_error_message(NewLine);
  output_plaintext_error_message(id);
  output_plaintext_error_message(NewLine);
  exit(id);
}

void output_plaintext_verifie_message(message_id_t id)
{
  output_plaintext_error_message(id);
  output_plaintext_error_message(NewLine);
  output_plaintext_error_message(ProblemIgnored);
  output_plaintext_error_message(NewLine);
}

static void ErrChar(char c)
{
#if !defined(QUIET)
  protocol_fputc(c,stderr);
  protocol_fflush(stderr);
#endif
}

void output_plaintext_io_error_message(message_id_t n, int val)
{
  protocol_fflush(stdout);
  output_plaintext_error_message(InputError,val);
  output_plaintext_error_message(n);
  ErrChar('\n');
  output_plaintext_error_message(OffendingItem,InputLine);
  ErrChar('\n');
}

static void format_time(void)
{
  unsigned long msec;
  unsigned long secs;
  StopTimer(&secs,&msec);

  {
    unsigned long const Hours = secs/3600;
    unsigned long const Minutes = (secs%3600)/60;
    unsigned long const Seconds = (secs%60);

    if (Hours>0)
      protocol_fprintf(stdout,"%lu:%02lu:%02lu h:m:s",Hours,Minutes,Seconds);
    else if (Minutes>0)
    {
      if (msec==MSEC_NOT_SUPPORTED)
        protocol_fprintf(stdout,"%lu:%02lu m:s", Minutes, Seconds);
      else
        protocol_fprintf(stdout,"%lu:%02lu.%03lu m:s", Minutes, Seconds, msec);
    }
    else
    {
      if (msec==MSEC_NOT_SUPPORTED)
        protocol_fprintf(stdout,"%lu s", Seconds);
      else
        protocol_fprintf(stdout,"%lu.%03lu s", Seconds, msec);
    }
  }
}

void output_plaintext_print_time(char const *header, char const *trail)
{
  if (!flag_regression)
  {
    protocol_fprintf(stdout,"%s",header);
    protocol_fprintf(stdout,output_message_get(TimeString));
    format_time();
    protocol_fprintf(stdout,"%s",trail);
  }
}

void output_plaintext_report_aborted(int signal)
{
  protocol_fputc('\n',stdout);
  protocol_fprintf(stdout,output_message_get(Abort),signal);
  format_time();
  protocol_fputc('\n',stdout);
}
