#include "debugging/trace.h"
#include "pyproc.h"
#include "pydata.h"
#include "pystip.h"
#include "optimisations/hash.h"
#include "output/plaintext/plaintext.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(DOTRACE)
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"

static trace_level level;

static unsigned long move_counter;

trace_level max_level = ULONG_MAX;

enum
{
  max_nesting_level = 10000,
  entry_length = 1000
};

static char const *entryNames[max_nesting_level];

#if defined(DOTRACECALLSTACK)
static char entries[max_nesting_level][entry_length];
static unsigned int entry_cursor[max_nesting_level];

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
#endif

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

#if defined(DOTRACECALLSTACK)
  entry_cursor[level-1] = snprintf(entries[level-1],
                                   entry_length,
                                   "> #%lu %s ",
                                   level,
                                   name);
#endif

  entryNames[level] = name;
}

void TraceFunctionParamListEnd(void)
{
  if (level<=max_level)
    fprintf(stdout,"\n");

#if defined(DOTRACECALLSTACK)
  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "\n");
#endif
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

#if defined(DOTRACECALLSTACK)
  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "< #%lu %s",
                                    level,
                                    name);
#endif

  --level;
}

void TraceFunctionResultImpl(char const *format, size_t value)
{
  if (level+1<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  entry_cursor[level] += snprintf(entries[level]+entry_cursor[level],
                                  entry_length-entry_cursor[level],
                                  format,
                                  value);
#endif
}

void TraceFunctionResultEnd(void)
{
  if (level<=max_level)
    fprintf(stdout,"\n");

#if defined(DOTRACECALLSTACK)
  entry_cursor[level] += snprintf(entries[level]+entry_cursor[level],
                                  entry_length-entry_cursor[level],
                                  "\n");
#endif
}

void TraceValueImpl(char const *format, size_t value)
{
  if (level<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    format,
                                    value);
#endif
}

void TracePointerValueImpl(char const *format, void const *value)
{
  if (level<=max_level)
  {
    fprintf(stdout,format,value);
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    format,
                                    value);
#endif
}

void TraceText(char const *text)
{
  if (level<=max_level)
    fprintf(stdout,"  #%lu %s",level,text);

#if defined(DOTRACECALLSTACK)
  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    "  #%lu %s",
                                    level,
                                    text);
#endif
}

void TraceEnumeratorImpl(char const *format,
                         char const *enumerator_name,
                         unsigned int value)
{
  if (level<=max_level)
    fprintf(stdout,format,enumerator_name,value);

#if defined(DOTRACECALLSTACK)
  entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                    entry_length-entry_cursor[level-1],
                                    format,
                                    enumerator_name,
                                    value);
#endif
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

#if defined(DOTRACECALLSTACK)
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
#endif
}

#if defined(DOTRACECALLSTACK)
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
#endif

void TracePieceImpl(char const *prefix, piece p)
{
#if defined(DOTRACECALLSTACK)
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
#endif

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

static void TraceCurrentMove(void)
{
  if (level<=max_level)
  {
    fprintf(stdout," #%lu %lu ",level,move_counter++);
    output_plaintext_write_move(nbply);
    fprintf(stdout," nbcou:%d",nbcou);
    fprintf(stdout," current_ply:%d\n",nbply);
    fflush(stdout);
  }
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
    fprintf(stdout,"%s----%s ",prefix,suffix);
  else
    fprintf(stdout,"%s%4u%s ",prefix,si,suffix);
}

static void Trace_slice(slice_index si)
{
  Trace_link("",slices[si].prev,"<");
}

static void Trace_pipe(slice_index si)
{
  Trace_slice(si);
  Trace_link(">",slices[si].next1,"");
}

static void Trace_fork(slice_index si)
{
  Trace_pipe(si);
  Trace_link("fork:",slices[si].next2,"");
}

static void Trace_branch(slice_index si)
{
  Trace_pipe(si);
  fprintf(stdout,"%2u/",slices[si].u.branch.length);
  fprintf(stdout,"%2u ",slices[si].u.branch.min_length);
}

static void Trace_binary(slice_index si)
{
  Trace_slice(si);
  Trace_link("op1:",slices[si].next1,"");
  Trace_link("op2:",slices[si].next2,"");
}

static void TraceStipulationRecursive(slice_index si, boolean done_slices[])
{
  if (si!=no_slice && !done_slices[si])
  {
    done_slices[si] = true;

    fprintf(stdout,"[%4u] ",si);
    fprintf(stdout,"%-34s ",slice_type_names[slices[si].type]);
    fprintf(stdout,"%c ",Side_names[slices[si].starter][0]);
    switch (slices[si].type)
    {
      case STForkOnRemaining:
        Trace_binary(si);
        fprintf(stdout,"threshold:%u\n",slices[si].u.fork_on_remaining.threshold);
        TraceStipulationRecursive(slices[si].next1,done_slices);
        TraceStipulationRecursive(slices[si].next2,done_slices);
        break;

      case STKeepMatingFilter:
        Trace_pipe(si);
        fprintf(stdout,"mating:%s ",
                Side_names[slices[si].u.keepmating_guard.mating]);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].next1,
                                  done_slices);
        break;

      case STMoveGenerator:
        Trace_pipe(si);
        fprintf(stdout,"mode:%u ",slices[si].u.move_generator.mode);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].next1,done_slices);
        break;

      case STOutputModeSelector:
        Trace_pipe(si);
        TraceEnumerator(output_mode,slices[si].u.output_mode_selector.mode,"\n");
        TraceStipulationRecursive(slices[si].next1,done_slices);
        break;

      case STPiecesParalysingMateFilter:
      case STPiecesParalysingMateFilterTester:
      case STPiecesParalysingStalemateSpecial:
      case STGoalCheckReachedTester:
      {
        Trace_pipe(si);
        TraceValue("%u",slices[si].u.goal_filter.applies_to_who);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].next1,done_slices);
        break;
      }

      case STGoalImmobileReachedTester:
      {
        Trace_pipe(si);
        Trace_link("?",slices[si].next2,"");
        TraceValue("%u",slices[si].u.goal_filter.applies_to_who);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].next1,done_slices);
        TraceStipulationRecursive(slices[si].next2,done_slices);
        break;
      }

      case STOutputPlaintextLineLineWriter:
      case STOutputPlaintextTreeGoalWriter:
        Trace_pipe(si);
        fprintf(stdout,"goal:%u\n",slices[si].u.goal_handler.goal.type);
        TraceStipulationRecursive(slices[si].next1,done_slices);
        break;

      case STGoalReachedTester:
        Trace_fork(si);
        fprintf(stdout,"goal:%u ",slices[si].u.goal_handler.goal.type);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].next1,done_slices);
        TraceStipulationRecursive(slices[si].next2,done_slices);
        break;

      case STAttackHashedTester:
      case STHelpHashedTester:
        Trace_pipe(si);
        fprintf(stdout,"\n");
        TraceStipulationRecursive(slices[si].next1,done_slices);
        break;

      default:
        switch (slice_type_get_structural_type(slices[si].type))
        {
          case slice_structure_leaf:
            Trace_link("",slices[si].prev,"<");
            fprintf(stdout,"\n");
            break;

          case slice_structure_pipe:
            Trace_pipe(si);
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].next1,done_slices);
            break;

          case slice_structure_branch:
            Trace_branch(si);
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].next1,done_slices);
            break;

          case slice_structure_fork:
            Trace_fork(si);
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].next1,done_slices);
            TraceStipulationRecursive(slices[si].next2,done_slices);
            break;

          case slice_structure_binary:
            Trace_binary(si);
            fprintf(stdout,"\n");
            TraceStipulationRecursive(slices[si].next1,done_slices);
            TraceStipulationRecursive(slices[si].next2,done_slices);
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

#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_play.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STMoveTracer slice.
 * @return index of allocated slice
 */
static slice_index alloc_move_tracer_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMoveTracer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_tracer_attack(slice_index si, stip_length_type n)
{
  TraceCurrentMove();
  return attack(slices[si].next1,n);
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type move_tracer_defend(slice_index si, stip_length_type n)
{
  TraceCurrentMove();
  return defend(slices[si].next1,n);
}

static void insert_move_tracer(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_move_tracer_slice();
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void stip_insert_move_tracers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&insert_move_tracer);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#endif
