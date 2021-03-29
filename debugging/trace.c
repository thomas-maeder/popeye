#include "debugging/trace.h"
#include "stipulation/stipulation.h"
#include "optimisations/hash.h"
#include "output/plaintext/plaintext.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if defined(DOTRACE)
#include "output/plaintext/pieces.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/move.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#if defined(DOTRACECALLSTACK)
#include "output/plaintext/language_dependant.h"
#include "pieces/walks/hunters.h"
#endif
static boolean do_trace = false;
static trace_level level;
static unsigned long move_counter;
static trace_level max_level = ULONG_MAX;
static boolean pointers_suppressed = false;

void set_trace(boolean b) {do_trace = b;}
boolean get_trace(void) {return do_trace;}

enum
{
  max_nesting_level = 10000,
  entry_length = 1000
};

static char const *entryNames[max_nesting_level];

#if defined(DOTRACECALLSTACK)
static boolean do_trace_call_stack = false;
static char entries[max_nesting_level][entry_length];
static unsigned int entry_cursor[max_nesting_level];

void set_trace_call_stack(boolean b) {do_trace_call_stack = b;}
boolean get_trace_call_stack(void) {return do_trace_call_stack;}

/* Write the call stack
 * @param file where to write the call stack
 */
void TraceCallStack(FILE *file)
{
  if (do_trace_call_stack)
  {
    unsigned int i;
    for (i = 0; i<level; ++i)
      fputs(entries[i],file);
    fflush(file);
  }
}
#endif

void TraceSetMaxLevel(trace_level tl)
{
  max_level = tl;
}

void TraceSuppressPointerValues(void)
{
  pointers_suppressed = true;
}

void TraceEOL(void)
{
  if (do_trace)
    if (level<=max_level)
    {
      putchar('\n');
      fflush(stdout);
    }
}

void TraceFunctionEntry(char const *name)
{
  ++level;

  if (do_trace && (level<=max_level))
  {
    printf("> #%lu %s ",level,name);
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    entry_cursor[level-1] = snprintf(entries[level-1],
                                     entry_length,
                                     "> #%lu %s ",
                                     level,
                                     name);
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1;
  }
  else
  {
    entries[level-1][0] = '\0';
    entry_cursor[level-1] = 0;
  }
#endif

  entryNames[level] = name;
}

void TraceFunctionParamListEnd(void)
{
  if (do_trace && (level<=max_level))
    putchar('\n');

#if defined(DOTRACECALLSTACK)
    if (do_trace_call_stack)
    {
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "\n");
      if (entry_cursor[level-1] >= entry_length)
        entry_cursor[level-1] = entry_length-1;
    }
#endif
}

void TraceFunctionExit(char const *name)
{
  if (do_trace && (strcmp(name,entryNames[level])!=0))
    fprintf(stderr,"Level:%lu Expected: %s. Got: %s\n",
            level,entryNames[level],name);

  assert(strcmp(name,entryNames[level])==0);

  if (do_trace && (level<=max_level))
  {
    printf("< #%lu %s",level,name);
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "< #%lu %s",
                                      level,
                                      name);
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1;
  }
#endif

  --level;
}

void TraceFunctionResultImpl(char const *prefix, char const *format, ...)
{
  va_list ap;
  if (do_trace && (level<max_level))
  {
    fputs(prefix,stdout);
    if (strcmp(format,"%p")==0 && pointers_suppressed)
      fputs("...",stdout);
    else
    {
      va_start(ap, format);
      vprintf(format,ap);
      va_end(ap);
    }
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    va_start(ap, format);
    entry_cursor[level] += vsnprintf(entries[level]+entry_cursor[level],
                                     entry_length-entry_cursor[level],
                                     format,
                                     ap);
    va_end(ap);
    if (entry_cursor[level] >= entry_length)
      entry_cursor[level] = entry_length-1;
  }
#endif
}

void TraceFunctionResultEnd(void)
{
  if (do_trace && (level<=max_level))
  {
    putchar('\n');
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    entry_cursor[level] += snprintf(entries[level]+entry_cursor[level],
                                    entry_length-entry_cursor[level],
                                    "\n");
    if (entry_cursor[level] >= entry_length)
      entry_cursor[level] = entry_length-1;
  }
#endif
}

void TraceValueImpl(char const *prefix, char const *format, ...)
{
  va_list ap;
  if (do_trace && (level<=max_level))
  {
    fputs(prefix,stdout);
    if (strcmp(format,"%p")==0 && pointers_suppressed)
      fputs("...",stdout);
    else
    {
      va_start(ap,format);
      vprintf(format,ap);
      va_end(ap);
    }
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    va_start(ap,format);
    entry_cursor[level-1] += vsnprintf(entries[level-1]+entry_cursor[level-1],
                                       entry_length-entry_cursor[level-1],
                                       format,
                                       ap);
    va_end(ap);
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1;
  }
#endif
}

void TraceText(char const *text)
{
  if (do_trace && (level<=max_level))
  {
    printf("  #%lu %s",level,text);
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "  #%lu %s",
                                      level,
                                      text);
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1;
  }
#endif
}

void TraceEnumeratorImpl(char const *format,
                         char const *enumerator_name,
                         unsigned int value)
{
  if (do_trace && (level<=max_level))
  {
    printf(format,enumerator_name,value);
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      format,
                                      enumerator_name,
                                      value);
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1;
  }
#endif
}

void TraceSquareImpl(char const *prefix, square s)
{
  if (do_trace && (level<=max_level))
  {
    fputs(prefix,stdout);

    if (s==initsquare)
      fputs("initsquare",stdout);
    else
    {
      if (is_on_board(s))
        WriteSquare(&output_plaintext_engine,stdout,s);
      else
        output_plaintext_engine.fprintf(stdout, "[square %d]", s);
    }
    fflush(stdout);
  }

#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "%s",
                                      prefix);
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1;
    if (s==initsquare)
    {
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "initsquare");
    }
    else
    {
      if (is_on_board(s))
      {
        entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                          entry_length-entry_cursor[level-1],
                                          "%c",
                                          (int)getBoardFileLabel((s%onerow) - nr_files_on_board));
        if (entry_cursor[level-1] >= entry_length)
          entry_cursor[level-1] = entry_length-1;
        if (isBoardReflected)
          entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                            entry_length-entry_cursor[level-1],
                                            "%c",
                                            (int)getBoardRowLabel(((2*nr_rows_on_board)-1) - (s/onerow)));
        else
          entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                            entry_length-entry_cursor[level-1],
                                            "%c",
                                            (int)getBoardRowLabel((s/onerow) - nr_rows_on_board));
      }
      else
        entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                          entry_length-entry_cursor[level-1],
                                          "[square %d]",
                                          s);
    }
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1; 
  }
#endif
}

#if defined(DOTRACECALLSTACK)
static void remember_regular_piece(piece_walk_type pnam)
{
  if (do_trace_call_stack)
  {
    char const p1 = PieceTab[pnam][1];

    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "%c",
                                      toupper((unsigned char)PieceTab[pnam][0]));
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1; 
    if (p1!=' ')
    {
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "%c",
                                        toupper((unsigned char)p1));
      if (entry_cursor[level-1] >= entry_length)
        entry_cursor[level-1] = entry_length-1;
    }
  }
}
#endif

void TraceWalkImpl(char const *prefix, piece_walk_type p)
{
#if defined(DOTRACECALLSTACK)
  if (do_trace_call_stack)
  {
    entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                      entry_length-entry_cursor[level-1],
                                      "%s",
                                      prefix);
    if (entry_cursor[level-1] >= entry_length)
      entry_cursor[level-1] = entry_length-1; 
    if (p==Empty) /* TODO: Is Empty the correct value here? */
    {
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "vide");
      if (entry_cursor[level-1] >= entry_length)
        entry_cursor[level-1] = entry_length-1; 
    }
    else if (p==Invalid) /* TODO: Is Invalid the correct value here? */
    {
      entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                        entry_length-entry_cursor[level-1],
                                        "obs");
      if (entry_cursor[level-1] >= entry_length)
        entry_cursor[level-1] = entry_length-1; 
    }
    else
    {
      if ((p<Hunter0) || (p>=nr_piece_walks))
        remember_regular_piece(p);
      else
      {
        unsigned int const i = (unsigned int)p-(unsigned int)Hunter0;
        assert(i<max_nr_hunter_walks);
        remember_regular_piece(huntertypes[i].away);
        entry_cursor[level-1] += snprintf(entries[level-1]+entry_cursor[level-1],
                                          entry_length-entry_cursor[level-1],
                                          "/");
        if (entry_cursor[level-1] >= entry_length)
          entry_cursor[level-1] = entry_length-1; 
        remember_regular_piece(huntertypes[i].home);
      }
    }
  }
#endif

  if (do_trace && (level<=max_level))
  {
    fputs(prefix,stdout);
    if (p==Empty)
      fputs("Empty",stdout);
    else if (p==Invalid)
      fputs("Invalid",stdout);
    else
      WriteWalk(&output_plaintext_engine,stdout,p);
    fflush(stdout);
  }
}

#include "position/underworld.h"
static void TraceCurrentMove(void)
{
  if (do_trace && (level<=max_level))
  {
    printf(" #%lu %lu ",level,move_counter++);
    output_plaintext_write_move(&output_plaintext_engine,
                                stdout,
                                &output_plaintext_symbol_table);
    printf(" CURRMOVE_OF_PLY(nbply):%u",CURRMOVE_OF_PLY(nbply));
    printf(" nr_ghosts:%u",nr_ghosts);
    printf(" current_ply:%u\n",nbply);
    fflush(stdout);
  }
}

void TraceCurrentHashBuffer(void)
{
  if (do_trace && (level<=max_level))
  {
    HashBuffer const *hb = &hashBuffers[nbply];
    unsigned int i;

    printf(" #%lu nbply:%u Leng:%u ",level,nbply,hb->cmv.Leng);
    for (i = 0; i<hb->cmv.Leng; ++i)
      printf("%02x ",(unsigned int)hb->cmv.Data[i]);
    putchar('\n');
    fflush(stdout);
  }
}

void TracePosition(echiquier e, Flags flags[maxsquare+4])
{
  if (do_trace && (level<=max_level))
  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (!is_square_empty(*bnp) && !is_square_blocked(*bnp))
      {
        WriteSpec(&output_plaintext_engine,
                  stdout,being_solved.spec[*bnp],
                  get_walk_of_piece_on_square(*bnp),true);
        WriteWalk(&output_plaintext_engine,
                  stdout,
                  get_walk_of_piece_on_square(*bnp));
        if (is_on_board(*bnp))
          WriteSquare(&output_plaintext_engine,stdout,*bnp);
        else
          output_plaintext_engine.fprintf(stdout, "[square %d]", *bnp);
        putchar(' ');
      }

    putchar('\n');
    fflush(stdout);
  }
}

static void trace_link(char const *prefix, slice_index si, char const *suffix)
{
  if (do_trace)
  {
    if (si==no_slice)
      printf("%s----%s ",prefix,suffix);
    else
      printf("%s%4u%s ",prefix,si,suffix);
  }
}

static char const context_shortcuts[] = { 'I', 'A', 'D', 'H', 'M', 'T', 'N' };
static char const level_shortcuts[]   = { 'T', 'S', 'N' };

static void trace_common(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    printf("[%4u] ",si);
    printf("%-34s ",slice_type_names[SLICE_TYPE(si)]);
    printf("%c%c%c%c ",
           Side_names[SLICE_STARTER(si)][0],
           level_shortcuts[st->level],
           context_shortcuts[st->context],
           st->activity==stip_traversal_activity_solving ? 'S' : 'T');
    trace_link("",SLICE_PREV(si),"<");
    trace_link(">",SLICE_NEXT1(si),"");
    trace_link("(",SLICE_TESTER(si),")");
  }
}

static void trace_branch(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    printf("%2u/",SLICE_U(si).branch.length);
    printf("%2u ",SLICE_U(si).branch.min_length);
    putchar('\n');

    stip_traverse_structure_children(si,st);
  }
}

static void trace_pipe(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    putchar('\n');

    stip_traverse_structure_children(si,st);
  }
}

static void trace_fork(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    trace_link("fork:",SLICE_NEXT2(si),"");
    putchar('\n');

    stip_traverse_structure_children(si,st);
  }
}

static void trace_leaf(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    putchar('\n');
  }
}

static void trace_hashed_tester(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    trace_link("base:",SLICE_U(si).derived_pipe.base,"");
    putchar('\n');

    stip_traverse_structure_children(si,st);
  }
}

static void trace_goal_reached_tester(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    trace_link("fork:",SLICE_NEXT2(si),"");
    printf("goal:%u ",(unsigned int)SLICE_U(si).goal_handler.goal.type);
    putchar('\n');

    stip_traverse_structure_children(si,st);
  }
}

static void trace_end_of_solution_line_writer(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    printf("goal:%u\n",(unsigned int)SLICE_U(si).goal_handler.goal.type);

    stip_traverse_structure_children(si,st);
  }
}

static void trace_fork_on_remaining(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    trace_link("fork:",SLICE_NEXT2(si),"");
    printf("threshold:%u\n",SLICE_U(si).fork_on_remaining.threshold);

    stip_traverse_structure_children(si,st);
  }
}

static void trace_keep_mating_filter(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    printf("mating:%s\n",Side_names[SLICE_U(si).keepmating_guard.mating]);

    stip_traverse_structure_children(si,st);
  }
}

static void trace_output_mode_selector(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    printf(" mode:%s(%u)\n",
           output_mode_names[SLICE_U(si).output_mode_selector.mode],
           (unsigned int)SLICE_U(si).output_mode_selector.mode);

    stip_traverse_structure_children(si,st);
  }
}

static void trace_goal_filter(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    printf("%u\n",(unsigned int)SLICE_U(si).goal_filter.applies_to_who);

    stip_traverse_structure_children(si,st);
  }
}

static void trace_goal_immobile_reached_tester(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    trace_link("?",SLICE_NEXT2(si),"");
    printf("%u\n",(unsigned int)SLICE_U(si).goal_filter.applies_to_who);

    stip_traverse_structure_children(si,st);
  }
}

static void trace_if_then_else(slice_index si, stip_structure_traversal *st)
{
  if (do_trace)
  {
    trace_common(si,st);
    trace_link("next2:",SLICE_NEXT2(si),"");
    trace_link("condition:",SLICE_U(si).if_then_else.condition,"");
    putchar('\n');

    stip_traverse_structure_children(si,st);
  }
}

static structure_traversers_visitor tracers[] =
{
  { STAttackHashedTester,               &trace_hashed_tester                },
  { STHelpHashedTester,                 &trace_hashed_tester                },
  { STGoalReachedTester,                &trace_goal_reached_tester          },
  { STOutputPlaintextLineLineWriter,    &trace_end_of_solution_line_writer  },
  { STOutputPlaintextGoalWriter,    &trace_end_of_solution_line_writer  },
  { STGoalImmobileReachedTester,        &trace_goal_immobile_reached_tester },
  { STPiecesParalysingMateFilter,       &trace_goal_filter                  },
  { STPiecesParalysingMateFilterTester, &trace_goal_filter                  },
  { STPiecesParalysingStalemateSpecial, &trace_goal_filter                  },
  { STGoalCheckReachedTester,           &trace_goal_filter                  },
  { STOutputModeSelector,               &trace_output_mode_selector         },
  { STKeepMatingFilter,                 &trace_keep_mating_filter           },
  { STForkOnRemaining,                  &trace_fork_on_remaining            },
  { STIfThenElse,                       &trace_if_then_else                 }
};

enum
{
  nr_tracers = sizeof tracers / sizeof tracers[0]
};

void TraceStipulation(slice_index si)
{
  if (do_trace && (level<=max_level))
  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_by_structure(&st,slice_structure_pipe,&trace_pipe);
    stip_structure_traversal_override_by_structure(&st,slice_structure_branch,&trace_branch);
    stip_structure_traversal_override_by_structure(&st,slice_structure_fork,&trace_fork);
    stip_structure_traversal_override_by_structure(&st,slice_structure_leaf,&trace_leaf);
    stip_structure_traversal_override(&st,tracers,nr_tracers);

    {
      trace_level const save_max_level = max_level;
      max_level = 0; /* avoid tracing during traversal */
      puts("stipulation structure:");
      stip_traverse_structure(si,&st);
      max_level = save_max_level;
    }
  }
}

#include "pieces/pieces.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void move_tracer_solve(slice_index si)
{
  TraceCurrentMove();
  pipe_solve_delegate(si);
}

/* Instrument slices with move tracers
 */
void solving_insert_move_tracers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMoveTracer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#endif
