#include "trace.h"
#include "pyproc.h"
#include "pydata.h"
#include "pyhash.h"
#include "pystip.h"
#include "output/plaintext/plaintext.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(DOTRACE)

static trace_level level;

static unsigned long move_counter;

trace_level max_level = ULONG_MAX;

static char const *entryNames[1000];

enum
{
  entry_length = 1000
};

static char entries[1000][entry_length];
static unsigned int entry_cursor[1000];

/* Write the call stack
 * @param file where to write the call stack
 */
void TraceCallStack(FILE *file)
{
  unsigned int i;
  for (i = 0; i<level; ++i)
    fprintf(file,"%s",entries[i]);
  fflush(file);
}


void TraceSetMaxLevel(trace_level tl)
{
  max_level = tl;
}

void TraceFunctionEntry(char const *name)
{
  ++level;

  if (level<=max_level)
  {
    fprintf(stdout,"> #%lu %s ",level,name);
    fflush(stdout);
  }

  entry_cursor[level-1] = snprintf(entries[level-1],
                                   entry_length,
                                   "> #%lu %s ",
                                   level,
                                   name);

  entryNames[level] = name;
}

void TraceFunctionParamListEnd(void)
{
  if (level<=max_level)
    fprintf(stdout,"\n");

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "\n");
}

void TraceFunctionExit(char const *name)
{
  if (strcmp(name,entryNames[level])!=0)
    fprintf(stderr,"Level:%lu Expected: %s. Got: %s\n",
            level,entryNames[level],name);

  assert(strcmp(name,entryNames[level])==0);

  if (level<=max_level)
  {
    fprintf(stdout,"< #%lu %s",level,name);
    fflush(stdout);
  }

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "< #%lu %s",
                                    level,
                                    name);

  --level;
}

void TraceFunctionResultImpl(char const *format, size_t value)
{
  if (level+1<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }

  entry_cursor[level] += snprintf(entries[level]+entry_cursor[level],
                                  entry_length-entry_cursor[level],
                                  format,
                                  value);
}

void TraceFunctionResultEnd(void)
{
  if (level<=max_level)
    fprintf(stdout,"\n");

  entry_cursor[level] += snprintf(entries[level]+entry_cursor[level],
                                  entry_length-entry_cursor[level],
                                  "\n");
}

void TraceValueImpl(char const *format, size_t value)
{
  if (level<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    format,
                                    value);
}

void TracePointerValueImpl(char const *format, void const *value)
{
  if (level<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    format,
                                    value);
}

void TraceText(char const *text)
{
  if (level<=max_level)
    fprintf(stdout,"  #%lu %s",level,text);

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "  #%lu %s",
                                    level,
                                    text);
}

void TraceEnumeratorImpl(char const *format,
                         char const *enumerator_name,
                         unsigned int value)
{
  if (level<=max_level)
    fprintf(stdout,format,enumerator_name,value);

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    format,
                                    enumerator_name,
                                    value);
}

void TraceSquareImpl(char const *prefix, square s)
{
  if (level<=max_level)
  {
    fprintf(stdout,"%s",prefix);

    if (s==initsquare)
      printf("initsquare");
    else
      WriteSquare(s);
    fflush(stdout);
  }

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "%s",
                                    prefix);
  if (s==initsquare)
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "initsquare");
  else
  {
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "%c",
                                      'a' - nr_files_on_board + s%onerow);
    if (isBoardReflected)
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "%c",
                                        '8' + nr_rows_on_board - s/onerow);
    else
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "%c",
                                        '1' - nr_rows_on_board + s/onerow);
  }
}

static void remember_regular_piece(PieNam pnam)
{
  char const p1 = PieceTab[pnam][1];

  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "%c",
                                    toupper(PieceTab[pnam][0]));
  if (p1!=' ')
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "%c",
                                      toupper(p1));
}

void TracePieceImpl(char const *prefix, piece p)
{
  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "%s",
                                    prefix);
  if (p==vide)
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "vide");
  else if (p==obs)
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "obs");
  else
  {
    PieNam const pnam = abs(p);

    if (pnam<Hunter0 || pnam >= (Hunter0 + maxnrhuntertypes))
      remember_regular_piece(pnam);
    else
    {
      unsigned int const i = pnam-Hunter0;
      assert(i<maxnrhuntertypes);
      remember_regular_piece(abs(huntertypes[i].away));
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "/");
      remember_regular_piece(abs(huntertypes[i].home));
    }
  }

  if (level<=max_level)
  {
    fprintf(stdout,"%s",prefix);
    if (p==vide)
      printf("vide");
    else if (p==obs)
      printf("obs");
    else
      WritePiece(p);
    fflush(stdout);
  }
}

boolean TraceCurrentMove(ply current_ply)
{
  if (level<=max_level)
  {
    fprintf(stdout," #%lu %lu ",level,move_counter++);
    output_plaintext_write_move(current_ply);
    fprintf(stdout," nbcou:%d",nbcou);
    fprintf(stdout," current_ply:%d\n",current_ply);
    fflush(stdout);
  }

  return true;
}

void TraceCurrentHashBuffer(void)
{
  if (level<=max_level)
  {
    HashBuffer const *hb = &hashBuffers[nbply];
    unsigned int i;

    fprintf(stdout," #%lu nbply:%u Leng:%u ",level,nbply,hb->cmv.Leng);
    for (i = 0; i<hb->cmv.Leng; ++i)
      fprintf(stdout,"%02x ",(unsigned int)hb->cmv.Data[i]);
    fprintf(stdout,"\n");
    fflush(stdout);
  }
}

void TracePosition(echiquier e, Flags flags[maxsquare+4])
{
  if (level<=max_level)
  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (e[*bnp]!=vide && e[*bnp]!=obs)
      {
        WriteSpec(spec[*bnp],true);
        WritePiece(abs(e[*bnp]));
        WriteSquare(*bnp);
        fprintf(stdout," ");
      }

    fprintf(stdout,"\n");
    fflush(stdout);
  }
}

static void Trace_link(char const *prefix, slice_index si, char const *suffix)
{
  if (si==no_slice)
    fprintf(stdout,"%s---%s ",prefix,suffix);
  else
    fprintf(stdout,"%s%3u%s ",prefix,si,suffix);
}

static void Trace_slice(slice_index si)
{
  Trace_link("",slices[si].prev,"<");
}

static void Trace_pipe(slice_index si)
{
  Trace_slice(si);
  Trace_link(">",slices[si].u.pipe.next,"");
}

static void Trace_branch(slice_index si)
{
  Trace_pipe(si);
  fprintf(stdout,"%2u/",slices[si].u.branch.length);
  fprintf(stdout,"%2u ",slices[si].u.branch.min_length);
}

static void TraceStipulationRecursive(slice_index si, boolean done_slices[])
{
  if (si!=no_slice && !done_slices[si])
  {
    done_slices[si] = true;

    fprintf(stdout,"[%3u] ",si);
    fprintf(stdout,"%-34s ",slice_type_names[slices[si].type]);
    fprintf(stdout,"%c ",Side_names[slices[si].starter][0]);
    switch (slices[si].type)
    {
      case STForkOnRemaining:
        Trace_slice(si);
        fprintf(stdout,"threshold:%u ",slices[si].u.fork_on_remaining.threshold);
        Trace_link("op1:",slices[si].u.fork_on_remaining.op1,"");
        Trace_link("op2:",slices[si].u.fork_on_remaining.op2,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.binary.op1,done_slices);
        TraceStipulationRecursive(slices[si].u.binary.op2,done_slices);
        break;

      case STKeepMatingFilter:
        Trace_pipe(si);
        fprintf(stdout,"mating:%s ",
                Side_names[slices[si].u.keepmating_guard.mating]);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.keepmating_guard.next,
                                  done_slices);
        break;

      case STOutputModeSelector:
        Trace_pipe(si);
        TraceEnumerator(output_mode,slices[si].u.output_mode_selector.mode,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STPiecesParalysingMateFilter:
      case STPiecesParalysingStalemateSpecial:
      case STGoalCheckReachedTester:
      {
        Trace_pipe(si);
        TraceValue("%u",slices[si].u.goal_filter.applies_to_who);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.goal_filter.next,done_slices);
        break;
      }

      case STGoalImmobileReachedTester:
      {
        Trace_pipe(si);
        Trace_link("fork:",slices[si].u.immobility_tester.fork,"");
        TraceValue("%u",slices[si].u.immobility_tester.applies_to_who);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.immobility_tester.next,done_slices);
        TraceStipulationRecursive(slices[si].u.immobility_tester.fork,done_slices);
        break;
      }

      case STOutputPlaintextLineLineWriter:
      case STOutputPlaintextTreeGoalWriter:
        Trace_pipe(si);
        fprintf(stdout,"goal:%u\n",slices[si].u.goal_handler.goal.type);
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        break;

      case STGoalReachedTester:
        Trace_pipe(si);
        fprintf(stdout,"goal:%u ",slices[si].u.goal_tester.goal.type);
        Trace_link("fork:",slices[si].u.goal_tester.fork,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
        TraceStipulationRecursive(slices[si].u.goal_tester.fork,done_slices);
        break;

      case STIntelligentMateFilter:
        Trace_pipe(si);
        Trace_link("fork:",slices[si].u.intelligent_mate_filter.fork,"");
        Trace_link("tester:",slices[si].u.intelligent_mate_filter.goal_tester_fork,"");
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].u.intelligent_mate_filter.next,done_slices);
        TraceStipulationRecursive(slices[si].u.intelligent_mate_filter.fork,done_slices);
        break;

      default:
        switch (slice_get_structural_type(si))
        {
          case slice_structure_leaf:
            Trace_link("",slices[si].prev,"<");
            fprintf(stdout,"\n");
            break;

          case slice_structure_pipe:
            Trace_pipe(si);
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].u.pipe.next,done_slices);
            break;

          case slice_structure_branch:
            Trace_branch(si);
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].u.branch.next,done_slices);
            break;

          case slice_structure_fork:
            Trace_pipe(si);
            Trace_link("fork:",slices[si].u.fork.fork,"");
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].u.fork.next,done_slices);
            TraceStipulationRecursive(slices[si].u.fork.fork,done_slices);
            break;

          case slice_structure_binary:
            Trace_slice(si);
            Trace_link("op1:",slices[si].u.binary.op1,"");
            Trace_link("op2:",slices[si].u.binary.op2,"");
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].u.binary.op1,done_slices);
            TraceStipulationRecursive(slices[si].u.binary.op2,done_slices);
            break;

          default:
            fprintf(stdout,"\n");
            break;
        }
        break;
    }
  }
}

void TraceStipulation(slice_index si)
{
  if (level<=max_level)
  {
    boolean done_slices[max_nr_slices] = { false };
    fprintf(stdout,"stipulation structure:\n");
    TraceStipulationRecursive(si,done_slices);
  }
}

#endif
