#include "conditions/norsk.h"
#include "pieces/walks/walks.h"
#include "position/walk_change.h"
#include "solving/move_generator.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "debugging/assert.h"

static piece_walk_type norsk_walk(piece_walk_type p)
{
  if (p==standard_walks[Queen])
    return standard_walks[Knight];
  else if (p==standard_walks[Knight])
    return standard_walks[Queen];
  else if (p==standard_walks[Rook])
    return standard_walks[Bishop];
  else if (p==standard_walks[Bishop])
    return standard_walks[Rook];
  else
    return p;
}

static boolean find_promotion(square sq_arrival)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  boolean result = false;

  for (curr = base+move_effect_journal_index_offset_other_effects; curr<top; ++curr)
    if (move_effect_journal[curr].type==move_effect_walk_change
        && move_effect_journal[curr].u.piece_change.on==sq_arrival)
    {
      result = true;
      break;
    }

  return result;
}

static move_effect_journal_index_type find_castling_partner_movement(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (result = top-1; result>base; --result)
    if (move_effect_journal[result].type==move_effect_piece_movement
        && move_effect_journal[result].reason==move_effect_reason_castling_partner_movement)
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void norsk_arriving_adjuster_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;


  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!find_promotion(sq_arrival))
  {
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    piece_walk_type const norsked = get_walk_of_piece_on_square(pos);
    piece_walk_type const norsked_to_walk = norsk_walk(norsked);

    if (norsked!=norsked_to_walk)
      move_effect_journal_do_walk_change(move_effect_reason_norsk_chess,
                                          pos,
                                          norsked_to_walk);

    {
      move_effect_journal_index_type const partner_movement = find_castling_partner_movement();
      if (partner_movement!=base)
      {
        square const sq_arrival = move_effect_journal[partner_movement].u.piece_movement.to;
        PieceIdType const moving_id = GetPieceId(move_effect_journal[partner_movement].u.piece_movement.movingspec);
        square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                                  moving_id,
                                                                                  sq_arrival);
        piece_walk_type const norsked = get_walk_of_piece_on_square(pos);
        piece_walk_type const norsked_to_walk = norsk_walk(norsked);

        if (norsked!=norsked_to_walk)
          move_effect_journal_do_walk_change(move_effect_reason_norsk_chess,
                                             pos,
                                             norsked_to_walk);
      }
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_not_illegal_capture(numecoup n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = !(TSTFLAG(being_solved.spec[move_generation_stack[n].departure],Royal)
             || (get_walk_of_piece_on_square(move_generation_stack[n].capture)
                 !=get_walk_of_piece_on_square(move_generation_stack[n].departure)));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void norsk_remove_illegal_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_captures(MOVEBASE_OF_PLY(nbply),&is_not_illegal_capture);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STNorskRemoveIllegalCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_remove_illegal_captures(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_norsk_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STNorskArrivingAdjuster);
  insert_remove_illegal_captures(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
