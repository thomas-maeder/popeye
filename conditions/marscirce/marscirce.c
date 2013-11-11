#include "conditions/marscirce/marscirce.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

square (*marscirce_determine_rebirth_square)(PieNam, Flags, square, square, square, Side);

static boolean always_reject(numecoup n)
{
  return false;
}

/* Generate non-capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @note the function is implemented in a way too generic for Mars Circe to
 *       allow it to be reused in Anti-Mars Circe.
 */
void marscirce_generate_non_captures(slice_index si,
                                     PieNam p,
                                     square sq_generate_from)
{
  square const sq_real_departure = curr_generation->departure;
  numecoup const base = CURRMOVE_OF_PLY(nbply);
  numecoup curr;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(sq_generate_from);
  TraceFunctionParamListEnd();

  curr_generation->departure = sq_generate_from;
  generate_moves_for_piece(slices[si].next1,p);
  curr_generation->departure = sq_real_departure;

  move_generator_filter_captures(base,&always_reject);

  for (curr = base+1; curr<=CURRMOVE_OF_PLY(nbply); ++curr)
    move_generation_stack[curr].departure = sq_real_departure;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 */
void marscirce_generate_captures(slice_index si,
                                 PieNam p,
                                 square sq_generate_from)
{
  square const sq_real_departure = curr_generation->departure;
  numecoup const base = CURRMOVE_OF_PLY(nbply);
  numecoup curr;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(sq_generate_from);
  TraceSquare(sq_real_departure);
  TraceFunctionParamListEnd();

  curr_generation->departure = sq_generate_from;
  generate_moves_for_piece(slices[si].next1,p);
  curr_generation->departure = sq_real_departure;

  move_generator_filter_noncaptures(base,&always_reject);

  for (curr = base+1; curr<=CURRMOVE_OF_PLY(nbply); ++curr)
    move_generation_stack[curr].departure = sq_real_departure;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_generate_moves_for_piece(slice_index si, PieNam p)
{
  square const sq_departure = curr_generation->departure;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = (*marscirce_determine_rebirth_square)(p,
                                           spec[sq_departure],
                                           sq_departure,initsquare,initsquare,
                                           advers(trait[nbply]));

    if (sq_rebirth==sq_departure)
      generate_moves_for_piece(slices[si].next1,p);
    else
    {
      marscirce_generate_non_captures(si,p,sq_departure);

      if (is_square_empty(sq_rebirth))
      {
        occupy_square(sq_rebirth,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
        empty_square(sq_departure);

        marscirce_generate_captures(si,p,sq_rebirth);

        occupy_square(sq_departure,get_walk_of_piece_on_square(sq_rebirth),spec[sq_rebirth]);
        empty_square(sq_rebirth);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square current_rebirth_square[maxply+1];

boolean mars_enforce_observer(slice_index si)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observer = current_rebirth_square[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = ((sq_observer== initsquare || sq_observer==sq_departure)
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a specific piece delivers check to a specific side from a
 * specific rebirth square
 * @param pos_checking potentially delivering check ...
 * @param sq_rebrirth ... from this square
 * @note the piece on pos_checking must belong to advers(side)
 */
boolean mars_is_square_observed_by(square pos_observer,
                                   square sq_rebirth,
                                   evalfunction_t *evaluate)
{
  boolean result = false;

  if (is_square_empty(sq_rebirth) || sq_rebirth==pos_observer)
  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    PieNam const pi_checking = get_walk_of_piece_on_square(pos_observer);
    if (pi_checking<Queen || pi_checking>Bishop
        || CheckDir[pi_checking][sq_target-sq_rebirth]!=0)
    {
      Flags const spec_checking = spec[pos_observer];

      empty_square(pos_observer);
      occupy_square(sq_rebirth,pi_checking,spec_checking);

      current_rebirth_square[nbply] = sq_rebirth;

      observing_walk[nbply] = pi_checking;
      result = (*checkfunctions[pi_checking])(evaluate);

      current_rebirth_square[nbply] = initsquare;

      empty_square(sq_rebirth);
      occupy_square(pos_observer,pi_checking,spec_checking);
    }
  }

  return result;
}

/* Determine whether a side observes a specific square
 * @param si identifies the tester slice
 * @return true iff side is in check
 */
boolean marscirce_is_square_observed(slice_index si, evalfunction_t *evaluate)
{
  int i,j;
  square square_h = square_h8;
  boolean result = false;
  Side const side_observing = trait[nbply];
  Side const side_observed = advers(side_observing);

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  for (i= nr_rows_on_board; i>0 && !result; i--, square_h += dir_down)
  {
    square pos_observing = square_h;
    for (j= nr_files_on_board; j>0 && !result; j--, pos_observing += dir_left)
      if (TSTFLAG(spec[pos_observing],side_observing)
          && pos_observing!=king_square[side_observed]   /* exclude nK */)
      {
        PieNam const pi_checking = get_walk_of_piece_on_square(pos_observing);
        Flags const spec_checking = spec[pos_observing];
        square const sq_rebirth = (*marscirce_determine_rebirth_square)(pi_checking,
                                               spec_checking,
                                               pos_observing,
                                               initsquare,
                                               initsquare,
                                               side_observed);
        result = mars_is_square_observed_by(pos_observing,
                                            sq_rebirth,
                                            evaluate);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_marscirce(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STMarsCirceMovesForPieceGenerator);

  stip_instrument_is_square_observed_testing(si,nr_sides,STMarsIsSquareObserved);

  stip_instrument_check_validation(si,nr_sides,STMarsCirceMovesForPieceGenerator);
  stip_instrument_observation_validation(si,nr_sides,STMarsCirceMovesForPieceGenerator);
  stip_instrument_observer_validation(si,nr_sides,STMarsCirceMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
