#include "optimisations/hash.h"
#include "solving/moves_traversal.h"
#include "optimisations/orthodox_check_directions.h"
#include "input/plaintext/plaintext.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/memory.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/protocol.h"
#include "output/latex/latex.h"
#include "platform/platform.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "platform/pytime.h"
#include "platform/priority.h"
#include "debugging/trace.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
      input_plaintext_read_requested_memory(argv[idx+1]);
      idx += 2;
      continue;
    }
    else if (strcmp(argv[idx], "-regression")==0)
    {
      protocol_overwrite();
      output_plaintext_suppress_variable();
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

int main(int argc, char *argv[])
{
  int idx_end_of_options;

  checkGlobalAssumptions();

  set_nice_priority();

  initialise_slice_properties();
  init_structure_children_visitors();
  init_moves_children_visitors();

  idx_end_of_options = parseCommandlineOptions(argc,argv);

  if (OpenInput(idx_end_of_options<argc ? argv[idx_end_of_options] : ""))
  {
    platform_init();

    if (!dimensionHashtable())
      fputs("Couldn't allocate the requested amount of memory\n",stdout);

    /* start timer to be able to display a reasonable time if the user
     * aborts execution before the timer is started for the first
     * problem */
    StartTimer();

    InitCheckDir();

    output_plaintext_print_version_info(stdout);

    input_plaintext_start();

    CloseInput();
  }

  LaTeXShutdown();

  return 0;
}
