#include "conditions/cast.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/observation.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>
#include <string.h>

cast_mode_type cast_mode;

/* Validate an observation according to CAST
 * @return true iff the observation is valid
 */
boolean cast_remove_illegal_captures_solve(slice_index si)
{
  boolean result;
  square const pos_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  Side const side_observing = SLICE_STARTER(si);
  unsigned nr_captures = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceSquare(pos_observer);
  TraceEnumerator(Side,side_observing);
  TraceEOL();

  siblingply(side_observing);
  generate_moves_for_piece(pos_observer);

  {
    numecoup const base = MOVEBASE_OF_PLY(nbply);
    numecoup const top = MOVEBASE_OF_PLY(nbply+1);
    numecoup curr;
    for (curr = base+1; curr<=top && nr_captures<=1; ++curr)
      if (is_on_board(move_generation_stack[curr].capture))
      {
        TraceSquare(move_generation_stack[curr].capture);
        ++nr_captures;
      }
  }

  finply();

  TraceValue("%u",nr_captures);
  TraceEOL();

  if (nr_captures>1)
    result = false;
  else
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static int compare_nr_opponent_moves(void const *a, void const *b)
{
  move_generation_elmt const * const elmt_a = a;
  move_generation_elmt const * const elmt_b = b;

  if (!is_on_board(elmt_a->capture) && is_on_board(elmt_b->capture))
    return -1;
  else if (is_on_board(elmt_a->capture) && !is_on_board(elmt_b->capture))
    return +1;
  else if (elmt_a->departure<elmt_b->departure)
    return -1;
  else if (elmt_a->departure>elmt_b->departure)
    return +1;
  else if (elmt_a->arrival<elmt_b->arrival)
    return -1;
  else if (elmt_a->arrival>elmt_b->arrival)
    return +1;
  else
    return 0;
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
void cast_multi_captures_remover_solve(slice_index si)
{
  numecoup const base = MOVEBASE_OF_PLY(nbply);
  numecoup top = MOVEBASE_OF_PLY(nbply+1);
  unsigned int const nr_moves = top-base;
  numecoup curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  qsort(&move_generation_stack[base+1],
        nr_moves,
        sizeof move_generation_stack[0],
        &compare_nr_opponent_moves);

  for (curr = top; curr>base+1; --curr)
  {
    numecoup next = curr-1;
    move_generation_elmt const * elmt_curr = &move_generation_stack[curr];
    move_generation_elmt const * elmt_next = &move_generation_stack[next];

    if (is_on_board(elmt_curr->capture) && is_on_board(elmt_next->capture)
        && elmt_curr->departure==elmt_next->departure)
    {
      while (next>base+1
             && is_on_board(move_generation_stack[next-1].capture)
             && move_generation_stack[next-1].departure==elmt_curr->departure)
        --next;

      memmove(&move_generation_stack[next],
              &move_generation_stack[curr+1],
              sizeof move_generation_stack[0]*(top-curr));

      top -= curr-next+1;
      curr = next;
    }
  }

  MOVEBASE_OF_PLY(nbply+1) = top;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move_generator(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STCASTMultiCapturesRemover);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for CAST
 * @param si identifies root slice of stipulation
 */
void cast_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STGeneratingMoves,&instrument_move_generator);
  stip_traverse_structure(si,&st);

  stip_instrument_check_validation(si,nr_sides,STCASTRemoveIllegalCaptures);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
