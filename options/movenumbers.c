#include "options/movenumbers.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/check.h"
#include "solving/ply.h"
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
unsigned int MoveNbr[maxply+1];

/* number of first move at root level to be considered
 */
static unsigned int RestartNbr[maxply+1];

static boolean restart_deep;

static void write_history_recursive(ply ply)
{
  if (ply>ply_retro_move+1)
  {
    write_history_recursive(parent_ply[ply]);
    fputs(":",stdout);
  }

  fprintf(stdout,"%u",MoveNbr[ply]-1);
}

void move_numbers_write_history(void)
{
  if (restart_deep)
  {
    fputs("\nuse option start ",stdout);
    write_history_recursive(nbply-1);
    fputs(" to replay\n",stdout);
  }
  else
    fputs("\nuse option start 1:1 to get replay information\n",stdout);
}

/* Reset the restart number setting.
 */
void reset_restart_number(void)
{
  ply ply;
  for (ply = ply_retro_move+1; ply<=maxply; ++ply)
  {
    RestartNbr[ply] = 0;
    MoveNbr[ply] = 1;
  }

  restart_deep = false;
}

unsigned int get_restart_number(void)
{
  return RestartNbr[ply_retro_move+1];
}

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
boolean read_restart_number(char const *optionValue)
{
  boolean result = false;

  ply ply = ply_retro_move+1;
  char *end;

  while (1)
  {
    unsigned long const restartNbrRequested = strtoul(optionValue,&end,10);
    if (optionValue!=end && restartNbrRequested<=UINT_MAX)
    {
      RestartNbr[ply] = (unsigned int)restartNbrRequested;
      result = true;

      if (*end==':')
      {
        optionValue = end+1;
        ++ply;
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
  if (MoveNbr[nbply]>=RestartNbr[nbply])
  {
    protocol_fprintf(stdout,"\n%3u  (", MoveNbr[nbply]);
    output_plaintext_write_move(&output_plaintext_engine,
                                stdout,
                                &output_plaintext_symbol_table);
    if (is_in_check(SLICE_STARTER(si)))
      protocol_fprintf(stdout,"%s"," +");
    protocol_fputc(' ',stdout);
    output_plaintext_print_time("   ","");
    protocol_fputc(')',stdout);
    protocol_fflush(stdout);
  }
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

  WriteMoveNbr(si);

  ++MoveNbr[nbply];

  TraceValue("%u",nbply);
  TraceValue("%u",MoveNbr[nbply]);
  TraceValue("%u\n",RestartNbr[nbply]);
  pipe_this_move_doesnt_solve_if(si,MoveNbr[nbply]<=RestartNbr[nbply]);

  MoveNbr[nbply+1] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void restart_guard_nested_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (MoveNbr[nbply-1]>0)
    ++MoveNbr[nbply];

  TraceValue("%u",nbply);
  TraceValue("%u",MoveNbr[nbply]);
  TraceValue("%u\n",RestartNbr[nbply]);
  pipe_this_move_doesnt_solve_if(si,MoveNbr[nbply]<=RestartNbr[nbply]);

  MoveNbr[nbply+1] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_attack(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STRestartGuard);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef enum
{
  insert_guard_mode_unknown,
  insert_guard_mode_regular,
  insert_guard_mode_intelligent
} insert_guard_mode;

static void insert_guard_help(slice_index si, stip_structure_traversal *st)
{
  insert_guard_mode const * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  assert(*mode!=insert_guard_mode_unknown);

  if (*mode==insert_guard_mode_regular)
  {
    if (st->level!=structure_traversal_level_nested)
    {
      if (restart_deep)
      {
        slice_index const prototypes[] = {
            alloc_pipe(STRestartGuard),
            alloc_pipe(STRestartGuardNested),
            alloc_pipe(STRestartGuardNested)
        };
        slice_insertion_insert(si,prototypes,3);
      }
      else
      {
        slice_index const prototype = alloc_pipe(STRestartGuard);
        slice_insertion_insert(si,&prototype,1);
      }
    }
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_intelligent(slice_index si,
                                     stip_structure_traversal *st)
{
  insert_guard_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = insert_guard_mode_intelligent;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guard_regular(slice_index si,
                                     stip_structure_traversal *st)
{
  insert_guard_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = insert_guard_mode_regular;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor restart_guard_inserters[] =
{
  { STAttackAdapter,     &insert_guard_attack          },
  { STDefenseAdapter,    &stip_structure_visitor_noop  },
  { STHelpAdapter,       &insert_guard_help            },
  { STIntelligentFilter, &insert_guard_intelligent     },
  { STIntelligentProof,  &insert_guard_regular         },
  { STMove,              &insert_guard_regular         }
};

enum
{
  nr_restart_guard_inserters = (sizeof restart_guard_inserters
                                / sizeof restart_guard_inserters[0])
};

/* Instrument stipulation with STRestartGuard slices
 * @param si identifies slice where to start
 */
void solving_insert_restart_guards(slice_index si)
{
  insert_guard_mode mode = insert_guard_mode_unknown;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    restart_guard_inserters,
                                    nr_restart_guard_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
