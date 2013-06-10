#include "conditions/phantom.h"
#include "pydata.h"
#include "conditions/marscirce/marscirce.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "solving/en_passant.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean phantom_chess_rex_inclusive;

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
 * @param side side for which to generate moves for
 * @param p indicates the walk according to which to generate moves
 * @param sq_departure departure square of moves to be generated
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_chess_generate_moves(Side side, piece p, square sq_departure)
{
  numecoup const start_regular_moves = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TracePiece(p);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  gen_piece_aux(side,sq_departure,abs(p));

  if (!TSTFLAG(spec[sq_departure],Royal) || phantom_chess_rex_inclusive)
  {
    square const sq_rebirth = (*marsrenai)(abs(p),
                                           spec[sq_departure],
                                           sq_departure,initsquare,initsquare,
                                           advers(side));

    if (sq_rebirth!=sq_departure && is_square_empty(sq_rebirth))
    {
      numecoup const start_moves_from_rebirth_square = current_move[nbply];

      occupy_square(sq_rebirth,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
      empty_square(sq_departure);

      gen_piece_aux(side,sq_rebirth,abs(p));

      occupy_square(sq_departure,get_walk_of_piece_on_square(sq_rebirth),spec[sq_rebirth]);
      empty_square(sq_rebirth);

      {
        numecoup top_filtered = start_moves_from_rebirth_square;
        numecoup curr_from_sq_rebirth;
        for (curr_from_sq_rebirth = start_moves_from_rebirth_square+1;
             curr_from_sq_rebirth<=current_move[nbply];
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
            move_generation_stack[top_filtered].departure = sq_departure ;
          }
        }

        current_move[nbply] = top_filtered;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square adjust(Side trait_ply)
{
  square result = initsquare;
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  if (is_pawn(get_walk_of_piece_on_square(sq_arrival))
      && move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    PieNam const pi_moving = abs(move_effect_journal[movement].u.piece_movement.moving);
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_multistep_departure = rennormal(pi_moving,
                                                    spec[sq_arrival],
                                                    sq_departure,
                                                    initsquare,
                                                    initsquare,
                                                    advers(trait_ply));
    result = en_passant_find_potential(sq_multistep_departure);

    /* In positions like pieces black pe6 white pd5, we can't tell whether
     * the black pawn's move to e5 is a double step.
     * As the ep capturer normally has the burden of proof, let's disallow
     * ep capture in that case for now.
     */
    if (result==sq_departure)
      result = initsquare;
  }

  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type phantom_en_passant_adjuster_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square multistep_over;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  multistep_over = adjust(slices[si].starter);
  if (multistep_over==initsquare)
    result = solve(slices[si].next1,n);
  else
  {
    en_passant_remember_multistep_over(0,multistep_over);
    result = solve(slices[si].next1,n);
    en_passant_forget_multistep();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_phantom_en_passant_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STPhantomChessEnPassantAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a specific side is in check in Phantom Chess
 * @param side the side
 * @param evaluate filter for king capturing moves
 * @return true iff side is in check
 */
boolean phantom_echecc(Side side, evalfunction_t *evaluate)
{
  int i,j;
  square square_h = square_h8;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  for (i= nr_rows_on_board; i>0 && !result; i--, square_h += dir_down)
  {
    square pos_checking = square_h;
    for (j= nr_files_on_board; j>0 && !result; j--, pos_checking += dir_left)
      /* in marscirce the kings are included */
      /* in phantomchess the kings are not included, but with rex
         inclusif they are */
      if ((!TSTFLAG(spec[pos_checking],Royal) || phantom_chess_rex_inclusive)
          && piece_belongs_to_opponent(pos_checking,side)
          && pos_checking!=king_square[side]   /* exclude nK */)
      {
        PieNam const pi_checking = get_walk_of_piece_on_square(pos_checking);
        Flags const spec_checking = spec[pos_checking];
        square const sq_rebirth = (*marsrenai)(pi_checking,spec_checking,pos_checking,initsquare,initsquare,side);
        result = mars_does_piece_deliver_check(side,pos_checking,sq_rebirth);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
