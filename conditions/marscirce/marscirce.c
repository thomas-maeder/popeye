#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

/* Generate non-capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_non_captures(slice_index si,
                                     PieNam p,
                                     square sq_generate_from,
                                     square sq_real_departure)
{
  numecoup const base = current_move[nbply];
  numecoup top_filtered = base;
  numecoup curr;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(sq_generate_from);
  TraceSquare(sq_real_departure);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,sq_generate_from,p);

  for (curr = base+1; curr<=current_move[nbply]; ++curr)
    if (is_square_empty(move_generation_stack[curr].capture))
    {
      ++top_filtered;
      move_generation_stack[top_filtered] = move_generation_stack[curr];
      move_generation_stack[top_filtered].departure = sq_real_departure;
    }

  current_move[nbply] = top_filtered;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate capturing moves
 * @param p walk according to which to generate moves
 * @param sq_generate_from generate the moves from here
 * @param sq_real_departure real departure square of the generated moves
 */
void marscirce_generate_captures(slice_index si,
                                 PieNam p,
                                 square sq_generate_from,
                                 square sq_real_departure)
{
  numecoup const base = current_move[nbply];
  numecoup top_filtered = base;
  numecoup curr;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(sq_generate_from);
  TraceSquare(sq_real_departure);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,sq_generate_from,p);

  for (curr = base+1; curr<=current_move[nbply]; ++curr)
    if (!is_square_empty(move_generation_stack[curr].capture))
    {
      ++top_filtered;
      move_generation_stack[top_filtered] = move_generation_stack[curr];
      move_generation_stack[top_filtered].departure = sq_real_departure;
    }

  current_move[nbply] = top_filtered;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not necessarily have walk p
 */
void marscirce_generate_moves_for_piece(slice_index si,
                                        square sq_departure,
                                        PieNam p)
{
  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  {
    square const sq_rebirth = (*marsrenai)(p,
                                           spec[sq_departure],
                                           sq_departure,initsquare,initsquare,
                                           advers(trait[nbply]));

    if (sq_rebirth==sq_departure)
      gen_piece_aux(sq_departure,p);
    else
    {
      marscirce_generate_non_captures(si,p,sq_departure,sq_departure);

      if (is_square_empty(sq_rebirth))
      {
        occupy_square(sq_rebirth,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
        empty_square(sq_departure);

        marscirce_generate_captures(si,p,sq_rebirth,sq_departure);

        occupy_square(sq_departure,get_walk_of_piece_on_square(sq_rebirth),spec[sq_rebirth]);
        empty_square(sq_rebirth);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a specific piece delivers check to a specific side from a
 * specific rebirth square
 * @param pos_checking potentially delivering check ...
 * @param sq_rebrirth ... from this square
 * @note the piece on pos_checking must belong to advers(side)
 */
boolean mars_is_square_observed_by(square pos_observer,
                                   square sq_rebirth,
                                   square sq_target,
                                   evalfunction_t *evaluate)
{
  boolean result = false;

  if (is_square_empty(sq_rebirth) || sq_rebirth==pos_observer)
  {
    PieNam const pi_checking = get_walk_of_piece_on_square(pos_observer);
    Flags const spec_checking = spec[pos_observer];
    square const save_fromspecificsquare = fromspecificsquare;

    empty_square(pos_observer);
    occupy_square(sq_rebirth,pi_checking,spec_checking);

    fromspecificsquare = sq_rebirth;
    result = (*checkfunctions[pi_checking])(sq_target,pi_checking,&eval_fromspecificsquare);
    fromspecificsquare = save_fromspecificsquare;

    empty_square(sq_rebirth);
    occupy_square(pos_observer,pi_checking,spec_checking);
  }

  return result;
}

/* Determine whether a side observes a specific square
 * @param side_observing the side
 * @param sq_target square potentially observed
 * @return true iff side is in check
 */
boolean marscirce_is_square_observed(square sq_target, evalfunction_t *evaluate)
{
  int i,j;
  square square_h = square_h8;
  boolean result = false;
  Side const side_observing = trait[nbply];
  Side const side_observed = advers(side_observing);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_observing,"\n");

  for (i= nr_rows_on_board; i>0 && !result; i--, square_h += dir_down)
  {
    square pos_checking = square_h;
    for (j= nr_files_on_board; j>0 && !result; j--, pos_checking += dir_left)
      if (TSTFLAG(spec[pos_checking],side_observing)
          && pos_checking!=king_square[side_observed]   /* exclude nK */)
      {
        PieNam const pi_checking = get_walk_of_piece_on_square(pos_checking);
        Flags const spec_checking = spec[pos_checking];
        square const sq_rebirth = (*marsrenai)(pi_checking,spec_checking,pos_checking,initsquare,initsquare,side_observed);
        result = mars_is_square_observed_by(pos_checking,sq_rebirth,sq_target,evaluate);
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
