#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

static void generate_additional_captures_from(slice_index si,
                                              PieNam p,
                                              square from,
                                              square sq_departure)
{
  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(from);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (from!=sq_departure && is_square_empty(from))
  {
    occupy_square(from,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
    empty_square(sq_departure);

    marscirce_generate_captures(si,p,from,sq_departure);

    occupy_square(sq_departure,get_walk_of_piece_on_square(from),spec[from]);
    empty_square(from);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not necessarily have walk p
 */
void plus_generate_moves_for_piece(slice_index si, square sq_departure, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,sq_departure,p);

  if (sq_departure==square_d4
      || sq_departure==square_e4
      || sq_departure==square_d5
      || sq_departure==square_e5)
  {
    generate_additional_captures_from(si,p,square_d4,sq_departure);
    generate_additional_captures_from(si,p,square_e4,sq_departure);
    generate_additional_captures_from(si,p,square_d5,sq_departure);
    generate_additional_captures_from(si,p,square_e5,sq_departure);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a side observes a specific square
 * @param identifies tester slice
 * @return true iff side is in check
 */
boolean plus_is_square_observed(slice_index si, evalfunction_t *evaluate)
{
  int i,j;
  Side const side_observing = trait[nbply];
  square square_h = square_h8;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  for (i= nr_rows_on_board; i>0 && !result; i--, square_h += dir_down)
  {
    square pos_observing = square_h;
    for (j= nr_files_on_board; j>0 && !result; j--, pos_observing += dir_left)
      if (TSTFLAG(spec[pos_observing],side_observing)
          && pos_observing!=move_generation_stack[current_move[nbply]-1].capture) /* exclude nK */
      {
        if (pos_observing==square_d4 || pos_observing==square_d5 || pos_observing==square_e4 || pos_observing==square_e5)
          result = (mars_is_square_observed_by(pos_observing,square_d4,evaluate)
                    || mars_is_square_observed_by(pos_observing,square_d5,evaluate)
                    || mars_is_square_observed_by(pos_observing,square_e4,evaluate)
                    || mars_is_square_observed_by(pos_observing,square_e5,evaluate));
        else
          result = mars_is_square_observed_by(pos_observing,pos_observing,evaluate);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise thet solving machinery with Plus Chess
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_plus(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STPlusMovesForPieceGenerator);
  stip_instrument_is_square_observed_testing(si,nr_sides,STPlusIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
