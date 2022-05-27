#include "conditions/dister.h"
#include "position/pieceid.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

square dister_reference_square[2];

static PieceIdType dister_reference_id[2];

/* Determine the length of a move for the MaxDister conditions; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
mummer_length_type maxdister_measure_length(void)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  mummer_length_type result;

  square dister_square[2] = { initsquare, initsquare };
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceEOL();

  for (s = square_a1; s<=square_h8; ++s)
    if (is_on_board(s))
    {
      if (GetPieceId(being_solved.spec[s])==dister_reference_id[0])
        dister_square[0] = s;
      if (GetPieceId(being_solved.spec[s])==dister_reference_id[1])
        dister_square[1] = s;
    }

  if (GetPieceId(being_solved.spec[sq_departure])==dister_reference_id[0])
    dister_square[0] = sq_arrival;
  if (GetPieceId(being_solved.spec[sq_departure])==dister_reference_id[1])
    dister_square[1] = sq_arrival;

  if (GetPieceId(being_solved.spec[sq_capture])==dister_reference_id[0])
    dister_square[0] = initsquare;
  if (GetPieceId(being_solved.spec[sq_capture])==dister_reference_id[1])
    dister_square[1] = initsquare;

  TraceSquare(dister_square[0]);
  TraceSquare(dister_square[1]);
  TraceEOL();

  if (dister_square[0]==initsquare || dister_square[1]==initsquare)
    result = 0;
  else
    result = move_diff_code[abs(dister_square[0]-dister_square[1])];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();

  return result;
}

/* Determine the length of a move for the MinDister conditions; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
mummer_length_type mindister_measure_length(void)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  mummer_length_type result;

  square dister_square[2] = { initsquare, initsquare };
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceEOL();

  for (s = square_a1; s<=square_h8; ++s)
    if (is_on_board(s))
    {
      if (GetPieceId(being_solved.spec[s])==dister_reference_id[0])
        dister_square[0] = s;
      if (GetPieceId(being_solved.spec[s])==dister_reference_id[1])
        dister_square[1] = s;
    }

  if (GetPieceId(being_solved.spec[sq_departure])==dister_reference_id[0])
    dister_square[0] = sq_arrival;
  if (GetPieceId(being_solved.spec[sq_departure])==dister_reference_id[1])
    dister_square[1] = sq_arrival;

  if (GetPieceId(being_solved.spec[sq_capture])==dister_reference_id[0])
    dister_square[0] = initsquare;
  if (GetPieceId(being_solved.spec[sq_capture])==dister_reference_id[1])
    dister_square[1] = initsquare;

  TraceSquare(dister_square[0]);
  TraceSquare(dister_square[1]);
  TraceEOL();

  if (dister_square[0]==initsquare || dister_square[1]==initsquare)
    result = 0;
  else
    result = 1000-move_diff_code[abs(dister_square[0]-dister_square[1])];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();

  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void dister_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  dister_reference_id[0] = GetPieceId(being_solved.spec[dister_reference_square[0]]);
  dister_reference_id[1] = GetPieceId(being_solved.spec[dister_reference_square[1]]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
