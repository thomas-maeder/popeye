#include "solving/for_each_move.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"

#include "solving/find_move.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STForEachAttack slice.
 * @return index of allocated slice
 */
static slice_index alloc_for_each_attack_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STForEachAttack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STForEachDefense slice.
 * @return index of allocated slice
 */
static slice_index alloc_for_each_defense_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STForEachDefense);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_move_iterator_move(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototype = (st->activity==stip_traversal_activity_testing
                                   ? alloc_find_defense_slice()
                                   : alloc_for_each_defense_slice());
    slice_index const prototypes[] = {
        prototype,
        alloc_pipe(STMoveEffectJournalUndoer),
        alloc_pipe(STMovePlayer),
        alloc_pipe(STLandingAfterMovePlay)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    move_insert_slices(si,st->context,prototypes,nr_prototypes);
  }
  else
  {
    slice_index const prototype = (st->activity==stip_traversal_activity_testing
                                   ? alloc_find_attack_slice()
                                   : alloc_for_each_attack_slice());
    slice_index const prototypes[] = {
        prototype,
        alloc_pipe(STMoveEffectJournalUndoer),
        alloc_pipe(STMovePlayer),
        alloc_pipe(STLandingAfterMovePlay)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    move_insert_slices(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const move_iterator_inserters[] =
{
  { STMove, &insert_move_iterator_move },
};

enum
{
  nr_move_iterator_inserters = (sizeof move_iterator_inserters
                                / sizeof move_iterator_inserters[0])
};

/* Instrument the solving machinery with move iterator slices
 * @param root_slice identifies root slice of the solving machinery
 */
void solving_insert_move_iterators(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    move_iterator_inserters,
                                    nr_move_iterator_inserters);
  stip_traverse_structure(root_slice,&st);

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
void for_each_attack_solve(slice_index si)
{
  stip_length_type result_intermediate = MOVE_HAS_NOT_SOLVED_LENGTH();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while (encore())
  {
    pipe_solve_delegate(si);
    if (slack_length<solve_result && solve_result<result_intermediate)
      result_intermediate = solve_result;
  }

  solve_result = result_intermediate;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
void for_each_defense_solve(slice_index si)
{
  stip_length_type result_intermediate = immobility_on_next_move;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while(encore())
  {
    pipe_solve_delegate(si);
    if (result_intermediate<solve_result)
      result_intermediate = solve_result;
  }

  solve_result = result_intermediate;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
