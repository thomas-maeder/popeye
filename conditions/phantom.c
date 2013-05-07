#include "conditions/phantom.h"
#include "pydata.h"
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

  if (p!=e[king_square[side]] && !phantom_chess_rex_inclusive)
  {
    square const sq_rebirth = (*marsrenai)(p,
                                           spec[sq_departure],
                                           sq_departure,initsquare,initsquare,
                                           advers(side));

    if (sq_rebirth!=sq_departure && e[sq_rebirth] == vide)
    {
      numecoup const start_moves_from_rebirth_square = current_move[nbply];

      spec[sq_rebirth] = spec[sq_departure];
      e[sq_rebirth] = e[sq_departure];

      e[sq_departure] = vide;
      spec[sq_departure] = EmptySpec;

      gen_piece_aux(side,sq_rebirth,abs(p));

      spec[sq_departure] = spec[sq_rebirth];
      e[sq_departure] = e[sq_rebirth];

      e[sq_rebirth] = vide;
      spec[sq_rebirth] = EmptySpec;

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

static void adjust(Side trait_ply)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  piece const pi_moving = move_effect_journal[movement].u.piece_movement.moving;

  ep[nbply] = initsquare;

  if (is_pawn(abs(e[sq_arrival]))
      && move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_multistep_departure = rennormal(pi_moving,
                                                    spec[sq_arrival],
                                                    sq_departure,
                                                    initsquare,
                                                    initsquare,
                                                    advers(trait_ply));
    adjust_ep_squares(sq_multistep_departure);

    /* In positions like pieces black pe6 white pd5, we can't tell whether
     * the black pawn's move to e5 is a double step.
     * As the ep capturer normally has the burden of proof, let's disallow
     * ep capture in that case for now.
     */
    if (ep[nbply]==sq_departure)
      ep[nbply] = initsquare;
  }
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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  adjust(slices[si].starter);
  result = solve(slices[si].next1,n);

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
