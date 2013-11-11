#include "conditions/castling_chess.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotion.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/castling.h"
#include "solving/move_generator.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/check.h"
#include "debugging/trace.h"

#include <assert.h>

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
stip_length_type castling_chess_move_player_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_capture>offset_platzwechsel_rochade)
  {
    square const sq_departure = move_gen_top->departure;
    square const sq_arrival = move_gen_top->arrival;
    square const sq_passed = (sq_departure+sq_arrival) / 2;

    assert(sq_arrival!=nullsquare);

    move_effect_journal_do_no_piece_removal();
    move_effect_journal_do_piece_movement(move_effect_reason_castling_king_movement,
                                          sq_departure,sq_arrival);
    move_effect_journal_do_piece_movement(move_effect_reason_castling_partner_movement,
                                          sq_capture-offset_platzwechsel_rochade,
                                          sq_passed);

    result = solve(slices[si].next2,n);
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void castlingchess_generate_moves_for_piece(slice_index si, PieNam p)
{
  square const sq_departure = curr_generation->departure;

  generate_moves_for_piece(slices[si].next1,p);

  if (p==King && !is_in_check(trait[nbply]))
  {
    vec_index_type k;
    for (k = vec_queen_end; k>= vec_queen_start; --k)
    {
      square const sq_passed = sq_departure+vec[k];
      square const sq_partner = find_end_of_line(sq_departure,vec[k]);

      curr_generation->arrival = sq_passed+vec[k];

      if (sq_partner!=sq_passed && sq_partner!=curr_generation->arrival
          && !is_square_blocked(sq_partner)
          && castling_is_intermediate_king_move_legal(trait[nbply],sq_passed))
        push_special_move(offset_platzwechsel_rochade+sq_partner);
    }
  }
}

/* Instrument slices with move tracers
 */
void stip_insert_castling_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STCastlingChessMovesForPieceGenerator);
  insert_alternative_move_players(si,STCastlingChessMovePlayer);
  solving_disable_castling(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
