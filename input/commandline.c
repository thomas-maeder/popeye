#include "input/commandline.h"
#include "optimisations/hash.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/protocol.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "input/plaintext/memory.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include <stdlib.h>
#include <string.h>

static int parseCommandlineOptions(int argc, char *argv[])
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

void command_line_options_parser_solve(slice_index si)
{
  int const argc = SLICE_U(si).command_line_options_parser.argc;
  char **argv = SLICE_U(si).command_line_options_parser.argv;
  int const idx_end_of_options = parseCommandlineOptions(argc,argv);
  char const *filename = idx_end_of_options<argc ? argv[idx_end_of_options] : "";
  slice_index const opener = input_plaintext_alloc_opener(filename);

  slice_insertion_insert(si,&opener,1);

  pipe_solve_delegate(si);
}

slice_index alloc_command_line_options_parser(int argc, char **argv)
{
  slice_index const result = alloc_pipe(STCommandLineOptionsParser);
  SLICE_U(result).command_line_options_parser.argc = argc;
  SLICE_U(result).command_line_options_parser.argv = argv;
  return result;
}
