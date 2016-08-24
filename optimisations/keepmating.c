#include "optimisations/keepmating.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "stipulation/pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"


/* Allocate a STKeepMatingFilter slice
 * @param mating mating side
 * @return identifier of allocated slice
 */
static slice_index alloc_keepmating_filter_slice(Side mating)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,mating);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKeepMatingFilter);
  SLICE_U(result).keepmating_guard.mating = mating;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the mating side still has a piece that could
 * deliver the mate.
 * @return true iff the mating side has such a piece
 */
static boolean is_a_mating_piece_left(Side mating_side)
{
  piece_walk_type p = King+1;
  while (p<nr_piece_walks && being_solved.number_of_pieces[mating_side][p]==0)
    p++;

  return p<nr_piece_walks;
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
void keepmating_filter_solve(slice_index si)
{
  Side const mating = SLICE_U(si).keepmating_guard.mating;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,mating);
  TraceEOL();

  pipe_this_move_doesnt_solve_if(si,!is_a_mating_piece_left(mating));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Stipulation instrumentation ***************
 */

/* Data structure for remembering the side(s) that needs to keep >= 1
 * piece that could deliver mate
 */
typedef struct
{
  boolean for_side[nr_sides];
} insertion_state_type;

static slice_index alloc_appropriate_filter(insertion_state_type const *state)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (state->for_side[White]+state->for_side[Black]==1)
  {
    Side const side = state->for_side[White] ? White : Black;
    result = alloc_keepmating_filter_slice(side);
  }
  else
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void keepmating_filter_inserter_goal(slice_index si,
                                            stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->for_side[advers(SLICE_STARTER(si))] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void keepmating_filter_inserter_end_of_branch(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we can't rely on the (arbitrary) order stip_traverse_structure_children()
   * would use; instead make sure that we first traverse towards the
   * goal(s).
   */
  stip_traverse_structure_next_branch(si,st);
  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_binary(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we can't rely on the (arbitrary) order stip_traverse_structure_children()
   * would use; instead make sure that we first traverse towards the
   * goal(s).
   */
  stip_traverse_structure_binary_operand2(si,st);
  stip_traverse_structure_binary_operand1(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_battle(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context!=stip_traversal_context_help)
  {
    insertion_state_type const * const state = st->param;
    slice_index const prototype = alloc_appropriate_filter(state);
    if (prototype!=no_slice)
    {
      SLICE_STARTER(prototype) = SLICE_STARTER(si);
      if (st->context==stip_traversal_context_attack)
        attack_branch_insert_slices(si,&prototype,1);
      else
        defense_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void keepmating_filter_inserter_help(slice_index si,
                                            stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_appropriate_filter(state);
    if (prototype!=no_slice)
    {
      SLICE_STARTER(prototype) = advers(SLICE_STARTER(si));
      help_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor keepmating_filter_inserters[] =
{
  { STTemporaryHackFork,  &stip_traverse_structure_children_pipe },
  { STNotEndOfBranchGoal, &keepmating_filter_inserter_battle     },
  { STReadyForHelpMove,   &keepmating_filter_inserter_help       },
  { STGoalReachedTester,  &keepmating_filter_inserter_goal       }
};

enum
{
  nr_keepmating_filter_inserters = (sizeof keepmating_filter_inserters
                                    / sizeof keepmating_filter_inserters[0])
};

/* Instrument stipulation with STKeepMatingFilter slices
 * @param si identifies slice where to start
 */
void solving_insert_keepmating_filters(slice_index si)
{
  insertion_state_type state = { { false, false } };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_end_of_branch,
                                                  &keepmating_filter_inserter_end_of_branch);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &keepmating_filter_inserter_end_of_branch);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_binary,
                                                  &keepmating_filter_inserter_binary);
  stip_structure_traversal_override(&st,
                                    keepmating_filter_inserters,
                                    nr_keepmating_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
