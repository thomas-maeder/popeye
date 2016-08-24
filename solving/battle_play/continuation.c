#include "solving/battle_play/continuation.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/battle_play/branch.h"
#include "solving/machinery/slack_length.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
void continuation_solver_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  fork_solve_delegate(si);

  fprintf(stderr,
          "solve_result:%u "
          "MOVE_HAS_SOLVED_LENGTH():%u "
          "move_has_solved():%u\n",
          solve_result,
          MOVE_HAS_SOLVED_LENGTH(),
          move_has_solved());
  if (move_has_solved())
  {
#if !defined(NDEBUG)
    stip_length_type const test_result = solve_result;
#endif
    stip_length_type const save_solve_nr_remaining = solve_nr_remaining;

    if (solve_nr_remaining>solve_result)
      solve_nr_remaining = solve_result;
    pipe_solve_delegate(si);
    solve_nr_remaining = save_solve_nr_remaining;

    fprintf(stderr,
            "solve_nr_remaining:%u "
            "test_result:%u "
            "solve_result:%u\n",
            solve_nr_remaining,
            test_result,
            solve_result);
    assert(solve_result==test_result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_continuation_solvers_postkey_play(slice_index si,
                                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->level!=structure_traversal_level_top
      && SLICE_U(si).branch.length>slack_length)
  {
    slice_index const prototype = alloc_continuation_solver_slice();
    slice_insertion_insert(si,&prototype,1);
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

/* Instrument the solving machinery with STContinuationSolver slices
 * @param root_slice root slice of the solving machinery
 */
void solving_insert_continuation_solvers(slice_index si)
{
  stip_structure_traversal st;
  boolean attack_played = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&attack_played);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    continuation_solver_inserters,
                                    nr_continuation_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

