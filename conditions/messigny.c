#include "conditions/messigny.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "solving/castling.h"
#include "position/piece_exchange.h"
#include "position/piece_removal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

boolean messigny_rex_inclusive;

static void find_forbidden_squares(square *from, square *to)
{
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
  move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_base[parent+1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (parent_movement<parent_top
      && move_effect_journal[parent_movement].type==move_effect_piece_exchange
      && move_effect_journal[parent_movement].reason==move_effect_reason_messigny_exchange)
  {
    *from = move_effect_journal[parent_movement].u.piece_exchange.from;
    *to = move_effect_journal[parent_movement].u.piece_exchange.to;
  }
  else
  {
    *from = initsquare;
    *to = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_messigny(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STMessignyMovesForPieceGenerator);
  insert_alternative_move_players(si,STMessignyMovePlayer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void messigny_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  if (!(being_solved.king_square[trait[nbply]]==curr_generation->departure
      && !messigny_rex_inclusive))
  {
    square forbidden_from;
    square forbidden_to;

    find_forbidden_squares(&forbidden_from,&forbidden_to);

    if (curr_generation->departure!=forbidden_from
        && curr_generation->departure!=forbidden_to)
    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
        if (piece_belongs_to_opponent(*bnp)
            && get_walk_of_piece_on_square(*bnp)==move_generation_current_walk
            && *bnp!=forbidden_from && *bnp!=forbidden_to
            /* prevent neutral piece from swapping with itself */
            && *bnp!=curr_generation->departure)
        {
          curr_generation->arrival = *bnp;
          push_special_move(messigny_exchange);
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
void messigny_move_player_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sq_capture==messigny_exchange)
  {
    square const sq_arrival = move_gen_top->arrival;
    square const sq_departure = move_gen_top->departure;

    move_effect_journal_do_no_piece_removal();
    move_effect_journal_do_piece_exchange(move_effect_reason_messigny_exchange,
                                          sq_arrival,sq_departure);

    fork_solve_delegate(si);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
