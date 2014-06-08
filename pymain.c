#include "optimisations/hash.h"
#include "stipulation/structure_traversal.h"
#include "solving/moves_traversal.h"
#include "optimisations/orthodox_check_directions.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/token.h"
#include "input/plaintext/language.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/message.h"
#include "output/latex/latex.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"
#include "debugging/trace.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Guess the "bitness" of the platform
 * @return 32 if we run on a 32bit platform etc.
 */
static unsigned int guessPlatformBitness(void)
{
#if defined(__unix) || __APPLE__ & __MACH__
#  if defined(ULONG_MAX) && ULONG_MAX==18446744073709551615U
  return 64;
#  else
  return 32;
#  endif
#elif defined(_WIN64)
  return 64;
#elif defined(_WIN32)
  return 32;
#endif
}

/* assert()s below this line must remain active even in "productive"
 * executables. */
#undef NDEBUG
#include <assert.h>

/* Check assumptions made throughout the program. Abort if one of them
 * isn't met. */
static void checkGlobalAssumptions(void)
{
  /* Make sure that the characters relevant for entering problems are
   * encoded contiguously and in the natural order. This is assumed
   * in pyio.c.
   *
   * NB: There is no need for the analoguous check for digits, because
   * decimal digits are guaranteed by the language to be encoded
   * contiguously and in the natural order. */
  assert('b'=='a'+1);
  assert('c'=='b'+1);
  assert('d'=='c'+1);
  assert('e'=='d'+1);
  assert('f'=='e'+1);
  assert('g'=='f'+1);
  assert('h'=='g'+1);

  check_hash_assumptions();
  enforce_piecename_uniqueness();
}

int parseCommandlineOptions(int argc, char *argv[])
{
  int idx = 1;

  while (idx<argc)
  {
    if (idx+1<argc && strcmp(argv[idx], "-maxpos")==0)
    {
      char *end;
      idx++;
      hash_max_number_storable_positions = strtoul(argv[idx], &end, 10);
      if (argv[idx]==end)
      {
        /* conversion failure
         * -> set to 0 now and to default value later */
        hash_max_number_storable_positions = 0;
      }
      idx++;
      continue;
    }
    else if (idx+1<argc && strcmp(argv[idx], "-maxtime")==0)
    {
      char *end;
      maxtime_type value;
      idx++;
      value = strtoul(argv[idx], &end, 10);
      if (argv[idx]==end)
        ; /* conversion failure -> assume no max time */
      else
        setCommandlineMaxtime(value);

      idx++;
      continue;
    }
    else if (idx+1<argc && strcmp(argv[idx],"-maxmem")==0)
    {
      readMaxmem(argv[idx+1]);
      idx += 2;
      continue;
    }
    else if (strcmp(argv[idx], "-regression")==0)
    {
      flag_regression = true;
      idx++;
      continue;
    }
    else if (strcmp(argv[idx], "-maxtrace")==0)
    {
#if defined(DOTRACE)
      trace_level max_trace_level;
      char *end;

      idx++;
      if (idx<argc)
      {
        max_trace_level = strtoul(argv[idx], &end, 10);
        if (*end==0)
          TraceSetMaxLevel(max_trace_level);
        else
        {
          /* conversion failure  - ignore option */
        }
      }
#else
      /* ignore the value*/
      idx++;
#endif

      idx++;
      continue;
    }
    else
      break;
  }

  return idx;
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

int main(int argc, char *argv[])
{
  int idx_end_of_options;

  checkGlobalAssumptions();

  set_nice_priority();

  initialise_slice_properties();
  init_slice_allocator();
  init_structure_children_visitors();
  init_moves_children_visitors();

  sprintf(versionString,
          "Popeye %s-%uBit v%.2f",
          OSTYPE,guessPlatformBitness(),VERSION);

  hash_max_number_storable_positions = ULONG_MAX;
  LaTeXout = false;
  flag_regression = false;

  initMaxmem();

  /* Initialize message table with default language.
   * This default setting is hopefully overriden later by ReadBeginSpec().
   */
  InitMsgTab(LanguageDefault);

  idx_end_of_options = parseCommandlineOptions(argc,argv);

  if (OpenInput(idx_end_of_options<argc ? argv[idx_end_of_options] : ""))
  {
    initMaxtime();

    if (!dimensionHashtable())
      pyfputs("Couldn't allocate the requested amount of memory\n",stdout);

    /* start timer to be able to display a reasonable time if the user
     * aborts execution before the timer is started for the first
     * problem */
    StartTimer();

    InitCheckDir();

    /* Don't use StdString() - possible trace file is not yet opened
     */
    pyfputs(versionString,stdout);
    pyfputs(maxmemString(),stdout);

    iterate_problems();

    CloseInput();
  }

  if (LaTeXout)
    LaTeXShutdown();

  return 0;
}
