#include "conditions/marscirce/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/circe/circe.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

boolean phantom_chess_rex_inclusive;

square marscirce_rebirth_square[toppile+1];

static boolean is_regular_arrival(square sq_arrival,
                                  numecoup start_regular_moves,
                                  numecoup start_moves_from_rebirth_square)
{
  boolean result = false;
  numecoup curr_regular_move;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParam("%u",start_regular_moves);
  TraceFunctionParam("%u",start_moves_from_rebirth_square);
  TraceFunctionParamListEnd();

  for (curr_regular_move = start_regular_moves+1;
       curr_regular_move<=start_moves_from_rebirth_square;
       ++curr_regular_move)
    if (move_generation_stack[curr_regular_move].arrival==sq_arrival)
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_generate_moves_for_piece(slice_index si)
{
  square const sq_departure = curr_generation->departure;
  numecoup const start_regular_moves = CURRMOVE_OF_PLY(nbply);
  numecoup curr_id = current_move_id[nbply];

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1);

  for (; curr_id<current_move_id[nbply]; ++curr_id)
    marscirce_rebirth_square[curr_id] = initsquare;

  if (!TSTFLAG(spec[sq_departure],Royal) || phantom_chess_rex_inclusive)
  {
    square const sq_rebirth = rennormal(move_generation_current_walk,
                                        spec[sq_departure],
                                        sq_departure,
                                        advers(trait[nbply]));

    if (sq_rebirth!=sq_departure && is_square_empty(sq_rebirth))
    {
      numecoup const start_moves_from_rebirth_square = CURRMOVE_OF_PLY(nbply);

      occupy_square(sq_rebirth,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
      empty_square(sq_departure);

      curr_generation->departure = sq_rebirth;
      generate_moves_for_piece(slices[si].next1);
      curr_generation->departure = sq_departure;

      for (; curr_id<current_move_id[nbply]; ++curr_id)
        marscirce_rebirth_square[curr_id] = sq_rebirth;

      occupy_square(sq_departure,get_walk_of_piece_on_square(sq_rebirth),spec[sq_rebirth]);
      empty_square(sq_rebirth);

      {
        numecoup top_filtered = start_moves_from_rebirth_square;
        numecoup curr_from_sq_rebirth;
        for (curr_from_sq_rebirth = start_moves_from_rebirth_square+1;
             curr_from_sq_rebirth<=CURRMOVE_OF_PLY(nbply);
             ++curr_from_sq_rebirth)
        {
          square const sq_arrival = move_generation_stack[curr_from_sq_rebirth].arrival;
          if (sq_arrival!=sq_departure
              && !is_regular_arrival(sq_arrival,
                                     start_regular_moves,
                                     start_moves_from_rebirth_square))
          {
            ++top_filtered;
            move_generation_stack[top_filtered] = move_generation_stack[curr_from_sq_rebirth];
            move_generation_stack[top_filtered].departure = sq_departure;
          }
        }

        SET_CURRMOVE(nbply,top_filtered);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);

  solving_instrument_move_generation(si,nr_sides,STPhantomMovesForPieceGenerator);
  stip_instrument_is_square_observed_testing(si,nr_sides,STPhantomIsSquareObserved);

  move_effect_journal_register_pre_capture_effect();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a specific side is in check in Phantom Chess
 * @param si identifies tester slice
 * @return true iff side is in check
 */
boolean phantom_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_observed_recursive(slices[si].next1,evaluate))
    result = true;
  else
  {
    Side const side_observing = trait[nbply];
    square const *observer_origin;
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

    for (observer_origin = boardnum; *observer_origin; ++observer_origin)
      if (*observer_origin!=sq_target /* no auto-observation */
          && (!TSTFLAG(spec[*observer_origin],Royal) || phantom_chess_rex_inclusive)
          && TSTFLAG(spec[*observer_origin],side_observing)
          && get_walk_of_piece_on_square(*observer_origin)==observing_walk[nbply]
          && mars_is_square_observed_by(si,evaluate,*observer_origin))
      {
        result = true;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
