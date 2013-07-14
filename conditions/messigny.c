#include "conditions/messigny.h"
#include "pydata.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include <assert.h>

boolean messigny_rex_exclusive;

static boolean is_not_illegal_repetition(square sq_departure,
                                         square sq_arrival,
                                         square sq_capture)
{
  boolean result = true;
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  if (parent_movement<parent_top
      && sq_capture==messigny_exchange
      && move_effect_journal[parent_movement].type==move_effect_piece_exchange
      && move_effect_journal[parent_movement].reason==move_effect_reason_messigny_exchange)
  {
    square const sq_parent_departure = move_effect_journal[parent_movement].u.piece_exchange.from;
    square const sq_parent_arrival = move_effect_journal[parent_movement].u.piece_exchange.to;

    if (sq_departure==sq_parent_arrival
        || sq_departure==sq_parent_departure
        || sq_arrival==sq_parent_arrival
        || sq_arrival==sq_parent_departure)
      result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
stip_length_type messigny_remove_illegal_swap_repetitions_solve(slice_index si,
                                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_not_illegal_repetition);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STMessignyRemoveIllegalSwapRepetitions);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_messigny(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STMessignyMovesForPieceGenerator);
  insert_alternative_move_players(si,STMessignyMovePlayer);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void messigny_generate_moves_for_piece(slice_index si,
                                       square sq_departure,
                                       PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,sq_departure,p);

  if (!(king_square[trait[nbply]]==sq_departure && messigny_rex_exclusive))
  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (piece_belongs_to_opponent(*bnp) && get_walk_of_piece_on_square(*bnp)==p)
        add_to_move_generation_stack(sq_departure,*bnp,messigny_exchange);
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
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
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
