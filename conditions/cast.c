#include "conditions/cast.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/observation.h"
#include "solving/king_capture_avoider.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>
#include <string.h>

cast_mode_type cast_mode;

static boolean deactivated = false;

/* Validate an observation according to CAST
 * @return true iff the observation is valid
 */
boolean cast_validate_observation(slice_index si)
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

  assert(!deactivated);
  deactivated = true;

  siblingply(side_observing);
  generate_moves_for_piece(pos_observer);

  assert(deactivated);
  deactivated = false;

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
void cast_generate_moves_for_piece(slice_index si)
{
  numecoup const save_top = MOVEBASE_OF_PLY(nbply+1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (!deactivated)
  {
    numecoup const base = MOVEBASE_OF_PLY(nbply);
    numecoup top = MOVEBASE_OF_PLY(nbply+1);
    numecoup curr;
    unsigned int nr_captures = 0;

    TraceText("hallo\n");

    TraceValue("%u",save_top);
    TraceValue("%u",top);
    TraceEOL();

    for (curr = save_top+1; curr<=top; ++curr)
    {
      TraceSquare(move_generation_stack[curr].departure);
      TraceSquare(move_generation_stack[curr].arrival);
      TraceSquare(move_generation_stack[curr].capture);
      TraceEOL();
      assert(curr_generation->departure==move_generation_stack[curr].departure);
      if (is_on_board(move_generation_stack[curr].capture))
        ++nr_captures;
    }

    TraceValue("%u",nr_captures);
    TraceEOL();

    if (nr_captures>1)
    {
      for (curr = save_top+1; curr<=top; ++curr)
        if (is_on_board(move_generation_stack[curr].capture))
        {
          move_generation_stack[curr] = move_generation_stack[top];
          --top;
          --curr;
        }

      MOVEBASE_OF_PLY(nbply+1) = top;
    }

    for (curr = save_top+1; curr<=top; ++curr)
    {
      TraceSquare(move_generation_stack[curr].departure);
      TraceSquare(move_generation_stack[curr].arrival);
      TraceSquare(move_generation_stack[curr].capture);
      TraceEOL();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for CAST
 * @param si identifies root slice of stipulation
 */
void cast_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,nr_sides,STCASTMovesForPieceGenerator);
  stip_instrument_check_validation(si,nr_sides,STCASTValidateObservation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation according to CASTInverse
 * @return true iff the observation is valid
 */
boolean cast_inverse_validate_observation(slice_index si)
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

  assert(!deactivated);
  deactivated = true;

  siblingply(side_observing);
  generate_moves_for_piece(pos_observer);

  assert(deactivated);
  deactivated = false;

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
    result = pipe_validate_observation_recursive_delegate(si);
  else
    result = false;

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
void cast_inverse_generate_moves_for_piece(slice_index si)
{
  numecoup const save_top = MOVEBASE_OF_PLY(nbply+1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (!deactivated)
  {
    numecoup const base = MOVEBASE_OF_PLY(nbply);
    numecoup top = MOVEBASE_OF_PLY(nbply+1);
    numecoup curr;
    unsigned int nr_captures = 0;

    TraceValue("%u",save_top);
    TraceValue("%u",top);
    TraceEOL();

    for (curr = save_top+1; curr<=top; ++curr)
    {
      TraceSquare(move_generation_stack[curr].departure);
      TraceSquare(move_generation_stack[curr].arrival);
      TraceSquare(move_generation_stack[curr].capture);
      TraceEOL();
      assert(curr_generation->departure==move_generation_stack[curr].departure);
      if (is_on_board(move_generation_stack[curr].capture))
        ++nr_captures;
    }

    TraceValue("%u",nr_captures);
    TraceEOL();

    if (nr_captures==1)
    {
      for (curr = save_top+1; curr<=top; ++curr)
        if (is_on_board(move_generation_stack[curr].capture))
        {
          move_generation_stack[curr] = move_generation_stack[top];
          --top;
          --curr;
        }

      MOVEBASE_OF_PLY(nbply+1) = top;
    }

    for (curr = save_top+1; curr<=top; ++curr)
    {
      TraceSquare(move_generation_stack[curr].departure);
      TraceSquare(move_generation_stack[curr].arrival);
      TraceSquare(move_generation_stack[curr].capture);
      TraceEOL();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for CASTInverse
 * @param si identifies root slice of stipulation
 */
void cast_inverse_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_moves_for_piece_generation(si,nr_sides,STCASTInverseMovesForPieceGenerator);
  stip_instrument_check_validation(si,nr_sides,STCASTInverseValidateObservation);

  solving_insert_king_capture_avoiders(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
