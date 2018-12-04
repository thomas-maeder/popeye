#include "conditions/castling_chess.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotion.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/castling.h"
#include "solving/move_generator.h"
#include "solving/post_move_iteration.h"
#include "position/piece_movement.h"
#include "position/piece_removal.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void castling_chess_move_player_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
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

    fork_solve_delegate(si);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void castlingchess_generate_moves_for_piece(slice_index si)
{
  square const sq_departure = curr_generation->departure;

  pipe_move_generation_delegate(si);

  if (move_generation_current_walk==King && !is_in_check(trait[nbply]))
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

static boolean castling_only_with_rook(numecoup n)
{
  square const special_capture = move_generation_stack[n].capture;

  if (special_capture>offset_platzwechsel_rochade)
  {
    square const sq_partner = special_capture-offset_platzwechsel_rochade;
    return get_walk_of_piece_on_square(sq_partner)==Rook;
  }
  else
    return true;
}

/* Filter out castlings that are allowed in CastlingChess but not in Rokagogo
 * @param identifies generator slice
 */
void rokagogo_filter_moves_for_piece(slice_index si)
{
  numecoup const save_numecoup = CURRMOVE_OF_PLY(nbply);

  pipe_move_generation_delegate(si);

  move_generator_filter_moves(save_numecoup,&castling_only_with_rook);
}

/* Instrument slices with Castling Chess slices
 * @param is_rokagogo true iff we are instrumenting for Rokagogo
 */
void solving_insert_castling_chess(slice_index si, boolean is_rokagogo)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_rokagogo)
    solving_instrument_move_generation(si,nr_sides,STRokagogoMovesForPieceGeneratorFilter);

  solving_instrument_move_generation(si,nr_sides,STCastlingChessMovesForPieceGenerator);

  insert_alternative_move_players(si,STCastlingChessMovePlayer);
  solving_disable_castling(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
