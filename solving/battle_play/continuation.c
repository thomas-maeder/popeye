#include "solving/battle_play/continuation.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/battle_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STContinuationSolver defender slice.
 * @return index of allocated slice
 */
slice_index alloc_continuation_solver_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_testing_pipe(STContinuationSolver);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type continuation_solver_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(slices[si].next2,n);
  if (slack_length<=result && result<=n)
  {
    stip_length_type const n_next = n<result ? n : result;
#if !defined(NDEBUG)
    stip_length_type const defend_result =
#endif
    solve(slices[si].next1,n_next);
    assert(defend_result==result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_continuation_solvers_postkey_play(slice_index si,
                                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->level!=structure_traversal_level_top
      && slices[si].u.branch.length>slack_length)
  {
    slice_index const prototype = alloc_continuation_solver_slice();
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_continuation_solvers_remember_attack(slice_index si,
                                                        stip_structure_traversal *st)
{
  boolean * const attack_played = st->param;
  boolean const save_attack_played = *attack_played;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *attack_played = true;
  stip_traverse_structure_children_pipe(si,st);
  *attack_played = save_attack_played;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_continuation_solvers_attack(slice_index si,
                                               stip_structure_traversal *st)
{
  boolean const * const attack_played = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (*attack_played && st->context==stip_traversal_context_defense)
  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STSolvingContinuation),
        alloc_continuation_solver_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    defense_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const continuation_solver_inserters[] =
{
  { STReadyForAttack, &insert_continuation_solvers_remember_attack },
  { STDefenseAdapter, &insert_continuation_solvers_postkey_play    },
  { STNotEndOfBranch, &insert_continuation_solvers_attack          }
};

enum
{
  nr_continuation_solver_inserters = sizeof continuation_solver_inserters / sizeof continuation_solver_inserters[0]
};

/* Instrument the stipulation structure with STContinuationSolver slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_continuation_solvers(slice_index si)
{
  stip_structure_traversal st;
  boolean attack_played = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&attack_played);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    continuation_solver_inserters,
                                    nr_continuation_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

