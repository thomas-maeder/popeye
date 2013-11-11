#include "conditions/messigny.h"
#include "pieces/pieces.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include <assert.h>

boolean messigny_rex_exclusive;

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
void stip_insert_messigny(slice_index si)
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
 * @param p walk to be used for generating
 */
void messigny_generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,p);

  if (!(king_square[trait[nbply]]==curr_generation->departure
      && messigny_rex_exclusive))
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
            && get_walk_of_piece_on_square(*bnp)==p
            && *bnp!=forbidden_from && *bnp!=forbidden_to)
        {
          curr_generation->arrival = *bnp;
          push_special_move(messigny_exchange);
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type messigny_move_player_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_capture==messigny_exchange)
  {
    square const sq_arrival = move_gen_top->arrival;
    square const sq_departure = move_gen_top->departure;

    move_effect_journal_do_no_piece_removal();
    move_effect_journal_do_piece_exchange(move_effect_reason_messigny_exchange,
                                          sq_arrival,sq_departure);

    result = solve(slices[si].next2,n);
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
