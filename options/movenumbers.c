#include "options/movenumbers.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "options/movenumbers/restart_guard_intelligent.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/message.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <limits.h>
#include <stdlib.h>

/* number of current move at root level
 */
static unsigned int MoveNbr[maxply+1];

/* number of first move at root level to be considered
 * we use the parent ply as index because the ply number of the parent ply of the
 * root branch is always known, while the ply of the root branch isn't known ahead
 * of playing it in all cases.
 */
static unsigned int RestartNbr[nr_movenumbers_boundaries][maxply+1];

static boolean restart_deep;

static ply recursion_level;

static void write_history_recursive(ply level)
{
  if (level>0)
  {
    write_history_recursive(level-1);
    putchar(':');
  }

  printf("%u",MoveNbr[level]);
}

void move_numbers_write_history(void)
{
  fputs("\nuse option movenumbers start ",stdout);

  if (restart_deep)
  {
    write_history_recursive(recursion_level);
    puts(" to replay");
  }
  else if (nbply>ply_retro_move)
  {
    ply ply = parent_ply[nbply];

    fputs("1",stdout);
    while (ply!=ply_retro_move)
    {
      fputs(":1",stdout);
      ply = parent_ply[ply];
    }

    puts(" to get replay information");
  }
}

/* Reset the restart number setting.
 */
void reset_restart_number(void)
{
  ply level;
  for (level = 0; level<=maxply; ++level)
  {
    RestartNbr[movenumbers_start][level] = 0;
    RestartNbr[movenumbers_end][level] = UINT_MAX;
  }

  assert(recursion_level==0);
  MoveNbr[0] = 0;

  restart_deep = false;
}

/* Retrieve the current restart number
 * @param start or end number?
 */
unsigned int get_restart_number(movenumbers_boundary_type mb)
{
  return RestartNbr[mb][0];
}

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
boolean read_restart_number(movenumbers_boundary_type mb,
                            char const *optionValue)
{
  boolean result = false;

  ply level = 0;
  char *end;

  while (1)
  {
    unsigned long const restartNbrRequested = strtoul(optionValue,&end,10);
    if (optionValue!=end && restartNbrRequested<=UINT_MAX)
    {
      RestartNbr[mb][level] = (unsigned int)restartNbrRequested;
      result = true;

      if (*end==':')
      {
        optionValue = end+1;
        ++level;
        restart_deep = true;
      }
      else
        break;
    }
  }

  return result;
}

static void WriteMoveNbr(slice_index si)
{
  ply level;

  protocol_fprintf(stdout,"\n%3u",MoveNbr[0]);
  for (level = 1; level<=recursion_level; ++level)
    protocol_fprintf(stdout,":%u",MoveNbr[level]);

  protocol_fprintf(stdout,"  (");
  output_plaintext_write_move(&output_plaintext_engine,
                              stdout,
                              &output_plaintext_symbol_table);
  if (!output_plaintext_check_indication_disabled
      && is_in_check(SLICE_STARTER(si)))
    protocol_fprintf(stdout,"%s"," +");
  protocol_fputc(' ',stdout);
  output_plaintext_print_time("   ","");
  protocol_fputc(')',stdout);
  protocol_fflush(stdout);
}

static boolean skip_this_move(void)
{
  boolean result = (MoveNbr[recursion_level]<RestartNbr[movenumbers_start][recursion_level]
                    || MoveNbr[recursion_level]>RestartNbr[movenumbers_end][recursion_level]);

  ply level;
  for (level = 0; result && level<recursion_level; ++level)
    if (MoveNbr[level]>RestartNbr[movenumbers_start][level]
        && MoveNbr[level]<RestartNbr[movenumbers_end][level])
      result = false;

  return result;
}

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
void restart_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++MoveNbr[recursion_level];

  TraceValue("%u",recursion_level);
  TraceValue("%u",MoveNbr[recursion_level]);
  TraceValue("%u",RestartNbr[movenumbers_start][recursion_level]);
  TraceValue("%u",RestartNbr[movenumbers_end][recursion_level]);
  TraceEOL();

  if (skip_this_move())
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  else
  {
    if (recursion_level==0
        || (RestartNbr[movenumbers_start][recursion_level]>0
            || RestartNbr[movenumbers_end][recursion_level]<UINT_MAX))
      WriteMoveNbr(si);

    ++recursion_level;
    assert(MoveNbr[recursion_level]==0);
    pipe_solve_delegate(si);
    MoveNbr[recursion_level] = 0;
    --recursion_level;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef enum
{
  insert_guard_mode_unknown,
  insert_guard_mode_regular,
  insert_guard_mode_intelligent
} insert_guard_mode_help_play;

typedef struct
{
    insert_guard_mode_help_play mode;
    boolean instrumented;
} insertion_status_type;

static void insert_guard_battle(slice_index si, stip_structure_traversal *st)
{
  insertion_status_type * const status = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level!=structure_traversal_level_nested)
  {
    slice_index const prototype = alloc_pipe(STRestartGuard);
    slice_insertion_insert(si,&prototype,1);
    status->instrumented = !restart_deep;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_help_move(slice_index si, stip_structure_traversal *st)
{
  unsigned int * const counter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++*counter;
  stip_traverse_structure_children_pipe(si,st);
  --*counter;

  TraceValue("%u",*counter);TraceEOL();

  if (*counter==0)
  {
    slice_index const prototypes[] = {
        alloc_pipe(STRestartGuard)
    };
    slice_insertion_insert_contextually(si,st->context,prototypes,1);
  }
  else if (restart_deep && st->level==structure_traversal_level_nested)
  {
    slice_index const prototypes[] = {
        alloc_pipe(STRestartGuardNested)
    };
    slice_insertion_insert_contextually(si,st->context,prototypes,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_help(slice_index si, stip_structure_traversal *st)
{
  insertion_status_type * const status = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_top
      || st->level==structure_traversal_level_setplay)
  {
    assert(status->mode==insert_guard_mode_unknown);

    stip_traverse_structure_children_pipe(si,st);

    assert(status->mode!=insert_guard_mode_unknown);

    if (status->mode==insert_guard_mode_regular)
    {
      stip_structure_traversal st_nested;
      unsigned int counter = 0;
      stip_structure_traversal_init(&st_nested,&counter);
      stip_structure_traversal_override_single(&st_nested,
                                               STMove,
                                               &insert_guard_help_move);
      stip_structure_traversal_override_single(&st_nested,
                                               STGoalReachedTester,
                                               &stip_traverse_structure_children_pipe);
      stip_structure_traversal_override_single(&st_nested,
                                               STConstraintSolver,
                                               &stip_traverse_structure_children_pipe);
      stip_traverse_structure(si,&st_nested);
      assert(counter==0);
    }
    else
    {
      slice_index const prototypes[] = {
          alloc_restart_guard_intelligent(),
          alloc_intelligent_target_counter()
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_prototypes);
    }

    status->mode = insert_guard_mode_unknown;
    status->instrumented = true;
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_intelligent(slice_index si,
                                     stip_structure_traversal *st)
{
  insertion_status_type * const status = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(status->mode==insert_guard_mode_unknown
         || status->mode==insert_guard_mode_intelligent);

  status->mode = insert_guard_mode_intelligent;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_regular(slice_index si,
                                     stip_structure_traversal *st)
{
  insertion_status_type * const status = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(status->mode==insert_guard_mode_unknown
         || status->mode==insert_guard_mode_regular);

  status->mode = insert_guard_mode_regular;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor restart_guard_inserters[] =
{
  { STAttackAdapter,     &insert_guard_battle                   },
  { STDefenseAdapter,    &stip_structure_visitor_noop           },
  { STHelpAdapter,       &insert_guard_help                     },
  { STIntelligentFilter, &insert_guard_intelligent              },
  { STIntelligentProof,  &insert_guard_regular                  },
  { STMove,              &insert_guard_regular                  },
  { STEndOfBranchForced, &stip_traverse_structure_children_pipe },
  { STConstraintSolver,  &stip_traverse_structure_children_pipe }
};

enum
{
  nr_restart_guard_inserters = (sizeof restart_guard_inserters
                                / sizeof restart_guard_inserters[0])
};

/* Instrument stipulation with STRestartGuard slices
 * @param si identifies slice where to start
 * @return true iff instrumentation according to requested restart numbers was successful
 **/
boolean solving_insert_restart_guards(slice_index si)
{
  insertion_status_type status = { insert_guard_mode_unknown, false };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&status);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    restart_guard_inserters,
                                    nr_restart_guard_inserters);
  stip_traverse_structure(si,&st);

  if (!status.instrumented)
  {
    RestartNbr[movenumbers_start][0] = 0;
    RestartNbr[movenumbers_end][0] = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",status.instrumented);
  TraceFunctionResultEnd();
  return status.instrumented;
}
