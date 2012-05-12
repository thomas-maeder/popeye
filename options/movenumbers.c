#include "options/movenumbers.h"
#include "pypipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "options/movenumbers/restart_guard_intelligent.h"
#include "output/plaintext/plaintext.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "debugging/trace.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/* number of current move at root level
 */
static unsigned int MoveNbr;

/* number of first move at root level to be considered
 */
static unsigned int RestartNbr;

/* Reset the restart number setting.
 */
void reset_restart_number(void)
{
  RestartNbr = 0;
  MoveNbr = 1;
}

unsigned int get_restart_number(void)
{
  return RestartNbr;
}

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 */
boolean read_restart_number(char const *optionValue)
{
  boolean result = false;

  char *end;
  unsigned long const restartNbrRequested = strtoul(optionValue,&end,10);
  if (optionValue!=end && restartNbrRequested<=UINT_MAX)
  {
    RestartNbr = (unsigned int)restartNbrRequested;
    result = true;
  }

  return result;
}

/* Increase the current move number; write the previous move number
 * provided it is above the number where the user asked us to restart
 * solving.
 * @param si slice index
 */
static void IncrementMoveNbr(slice_index si)
{
  if (MoveNbr>=RestartNbr)
  {
    sprintf(GlobalStr,"\n%3u  (", MoveNbr);
    StdString(GlobalStr);
    output_plaintext_write_move(nbply);
    if (echecc(nbply,slices[si].starter))
      StdString(" +");
    StdChar(blank);

    if (!flag_regression)
    {
      StdString("   ");
      PrintTime();
    }

    StdString(")");
  }

  ++MoveNbr;
}

/* Allocate a STRestartGuard slice
 * @return allocated slice
 */
static slice_index alloc_restart_guard(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRestartGuard);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type restart_guard_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  IncrementMoveNbr(si);

  if (MoveNbr<=RestartNbr)
    result = n+2;
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type restart_guard_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  IncrementMoveNbr(si);

  TraceValue("%u",MoveNbr);
  TraceValue("%u\n",RestartNbr);
  if (MoveNbr<=RestartNbr)
    result = n+2;
  else
    result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_guard_attack(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_restart_guard();
    branch_insert_slices(si,&prototype,1);
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
    slice_index const prototype = alloc_restart_guard();
    branch_insert_slices(si,&prototype,1);
  }
  else
  {
    slice_index const prototypes[] = {
        alloc_restart_guard_intelligent(),
        alloc_intelligent_target_counter()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices(si,prototypes,nr_prototypes);
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

static structure_traversers_visitors restart_guard_inserters[] =
{
  { STAttackAdapter,              &insert_guard_attack          },
  { STDefenseAdapter,             &stip_structure_visitor_noop  },
  { STHelpAdapter,                &insert_guard_help            },
  { STIntelligentMateFilter,      &insert_guard_intelligent     },
  { STIntelligentStalemateFilter, &insert_guard_intelligent     },
  { STIntelligentProof,           &insert_guard_regular         },
  { STMove,                       &insert_guard_regular         }
};

enum
{
  nr_restart_guard_inserters = (sizeof restart_guard_inserters
                                / sizeof restart_guard_inserters[0])
};

/* Instrument stipulation with STRestartGuard slices
 * @param si identifies slice where to start
 */
void stip_insert_restart_guards(slice_index si)
{
  insert_guard_mode mode = insert_guard_mode_unknown;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    restart_guard_inserters,
                                    nr_restart_guard_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
