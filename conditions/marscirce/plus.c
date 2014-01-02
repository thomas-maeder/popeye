#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

static square const center_squares[] = { square_d4, square_d5, square_e4, square_e5 };
enum { nr_center_squares = sizeof center_squares / sizeof center_squares[0] };

static void generate_additional_captures_from(slice_index si,
                                              PieNam p,
                                              square from)
{
  square const sq_departure = curr_generation->departure;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  if (from!=sq_departure && is_square_empty(from))
  {
    occupy_square(from,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
    empty_square(sq_departure);

    marscirce_generate_captures(si,p,from);

    occupy_square(sq_departure,get_walk_of_piece_on_square(from),spec[from]);
    empty_square(from);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @note the piece on the departure square need not necessarily have walk p
 */
void plus_generate_moves_for_piece(slice_index si, PieNam p)
{
  square const sq_departure = curr_generation->departure;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,p);

  for (i = 0; i!=nr_center_squares; ++i)
    if (sq_departure==center_squares[i])
    {
      unsigned int j;
      for (j = 0; j!=nr_center_squares; ++j)
        generate_additional_captures_from(si,p,center_squares[j]);
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_square_observed_from_center(slice_index si,
                                              validator_id evaluate,
                                              square observer_origin)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceSquare(observer_origin);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_center_squares; ++i)
    if (observer_origin!=center_squares[i] /* already tested without rebirth */
        && mars_is_square_observed_from_rebirth_square(si,evaluate,observer_origin,center_squares[i]))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side observes a specific square
 * @param identifies tester slice
 * @return true iff side is in check
 */
boolean plus_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_observed_recursive(slices[si].next1,evaluate))
    result = true;
  else
  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    Side const side_observing = trait[nbply];
    unsigned int i;

    for (i = 0; i!=nr_center_squares; ++i)
    {
      square const observer_origin = center_squares[i];
      if (TSTFLAG(spec[observer_origin],side_observing)
          && get_walk_of_piece_on_square(observer_origin)==observing_walk[nbply]
          && observer_origin!=sq_target /* no auto-observation */
          && is_square_observed_from_center(si,evaluate,observer_origin))
      {
        result = true;
        break;
      }
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
