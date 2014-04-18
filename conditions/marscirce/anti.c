#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void antimars_generate_moves_for_piece(slice_index si)
{
  square const sq_departure = curr_generation->departure;
  numecoup curr_id = current_move_id[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = (*marscirce_determine_rebirth_square)(move_generation_current_walk,
                                           spec[sq_departure],
                                           sq_departure,initsquare,initsquare,
                                           advers(trait[nbply]));

    marscirce_generate_captures(si,sq_departure);

    for (; curr_id<current_move_id[nbply]; ++curr_id)
      marscirce_rebirth_square[curr_id] = initsquare;

    if (sq_rebirth==sq_departure)
    {
      marscirce_generate_non_captures(si,sq_rebirth);

      for (; curr_id<current_move_id[nbply]; ++curr_id)
        marscirce_rebirth_square[curr_id] = sq_rebirth;
    }
    else if (is_square_empty(sq_rebirth))
    {
      occupy_square(sq_rebirth,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
      empty_square(sq_departure);

      marscirce_generate_non_captures(si,sq_rebirth);

      occupy_square(sq_departure,get_walk_of_piece_on_square(sq_rebirth),spec[sq_rebirth]);
      empty_square(sq_rebirth);

      for (; curr_id<current_move_id[nbply]; ++curr_id)
        marscirce_rebirth_square[curr_id] = sq_rebirth;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Anti-Mars Circe
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_antimars(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STAntiMarsCirceMovesForPieceGenerator);

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);

  move_effect_journal_register_pre_capture_effect();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
