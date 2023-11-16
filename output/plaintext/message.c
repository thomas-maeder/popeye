#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "platform/timer.h"
#include "platform/maxmem.h"
#include "platform/platform.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static boolean is_variable_output_suppressed = false;

#if defined(DEBUG)
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

/* Suppress output that is supposed to be different in two runs with the same
 * input, e.g. timing information
 */
void output_plaintext_suppress_variable(void)
{
  is_variable_output_suppressed = true;
}

/* Issue a message text
 * @param id identifies the message
 * @param ... additional parameters according the printf() like conversion
 *            specifiers in message id
 */
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

static void output_plaintext_error_message_va_list(message_id_t id, va_list args)
{
  DBG((stderr, "ErrorMsg(%d) = %s\n", id, output_message_get(id)));
#if !defined(QUIET)
  if (id<MsgCount)
  {
    protocol_vfprintf(stderr,output_message_get(id),args);
  }
  else
    output_plaintext_error_message(InternalError,id);

  protocol_fflush(stderr);
#endif
}

/* Report an error
 * @param id identifies the diagnostic message
 * @param ... additional parameters according the printf() like conversion
 *            specifiers in message id
 */
void output_plaintext_error_message(message_id_t id, ...)
{
  va_list args;
  va_start(args,id);
  output_plaintext_error_message_va_list(id,args);
  va_end(args);
}

/* Issue a fatal message
 * @param id identifies the message
 * @note terminates the program with exit status code id
 */
void output_plaintext_fatal_message(message_id_t id)
{
  output_plaintext_error_message(ErrFatal);
  output_plaintext_error_message(NewLine);
  output_plaintext_error_message(NewLine);
  output_plaintext_error_message(id);
  output_plaintext_error_message(NewLine);
  exit(id);
}

/* Report a verification error that causes the current problem to be ignored
 * @param id identiifes the diagnostic message
 */
void output_plaintext_verifie_message(message_id_t id)
{
  output_plaintext_error_message(id);
  output_plaintext_error_message(NewLine);
  output_plaintext_error_message(ProblemIgnored);
  output_plaintext_error_message(NewLine);
}

/* Issue an input error message
 * @param id identifies the diagnostic message
 * @param ... additional parameters according to the printf() conversion
 *            specifier in message id
 */
void output_plaintext_input_error_message(message_id_t n, ...)
{
#if !defined(QUIET)
  va_list args;
  protocol_fflush(stdout);
  output_plaintext_error_message(InputError);
  va_start(args,n);
  output_plaintext_error_message_va_list(n,args);
  va_end(args);
  protocol_fputc('\n',stderr);
  output_plaintext_error_message(OffendingItem,InputLine);
  protocol_fputc('\n',stderr);
  protocol_fflush(stderr);
#endif
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

/* Issue a solving time indication
 * @param header text printed before the time
 * @param trail text printed after the time
 * @note nothing is issued if we are in regression testing mode
 */
void output_plaintext_print_time(char const *header, char const *trail)
{
  if (!is_variable_output_suppressed)
  {
    protocol_fprintf(stdout,"%s",header);
    protocol_fprintf(stdout,output_message_get(TimeString));
    format_time();
    protocol_fprintf(stdout,"%s",trail);
  }
  protocol_fflush(stdout);
}

#if !defined(OSTYPE)
#  if defined(C370)
#    define OSTYPE "MVS"
#  elseif defined(DOS)
#    define OSTYPE "DOS"
#  elseif defined(ATARI)
#    define OSTYPE "ATARI"
#  elseif defined(_WIN98)
#    define OSTYPE "WINDOWS98"
#  elseif defined(_WIN16) || defined(_WIN32)
#    define OSTYPE "WINDOWS"
#  elseif defined(__unix)
#    if defined(__GO32__)
#      define OSTYPE "DOS"
#    else
#      define OSTYPE "UNIX"
#    endif  /* __GO32__ */
#  else
#    define OSTYPE "C"
#  endif
#endif

static void format_allocated_memory(FILE *file, maxmem_kilos_type allocated)
{
  if (allocated>=10*one_giga)
    fprintf(file, " (%lu GB)", allocated/one_giga);
  else if (allocated>=10*one_mega)
    fprintf(file, " (%lu MB)", allocated/one_mega);
  else
    fprintf(file, " (%lu KB)", allocated);
}

/* Print information about the program version, platform, maximum memory ...
 */
void output_plaintext_print_version_info(FILE *file)
{
  if (!is_variable_output_suppressed)
  {
    fprintf(file,"Popeye %s-%uBit v%.2f",
            OSTYPE,platform_guess_bitness(),VERSION);
    format_allocated_memory(file,platform_get_allocated_memory());
    fputc('\n',file);
    fflush(file);
  }
}

/* Issue a message that the program is being aborted
 * @param signal identifies the aborting signal
 * @note can be used inside a signal handler; does *not* terminate the program
 */
void output_plaintext_report_aborted(int signal)
{
  protocol_fputc('\n',stdout);
  protocol_fprintf(stdout,output_message_get(Abort),signal);
  format_time();
  protocol_fputc('\n',stdout);
}
