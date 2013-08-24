#include "conditions/wormhole.h"
#include "pydata.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotion.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/post_move_iteration.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

square wormhole_positions[wormholes_capacity];
static unsigned int nr_wormholes;

unsigned int wormhole_next_transfer[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id_transfer[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_promotion[maxply+1];

static pieces_pawns_promotion_sequence_type promotion_of_transfered[maxply+1];

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
stip_length_type wormhole_transfered_promoter_solve(slice_index si, stip_length_type n)
{
  square const sq_transfer = wormhole_positions[wormhole_next_transfer[nbply]-1];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_promotion[nbply])
    pieces_pawns_initialise_promotion_sequence(sq_transfer,&promotion_of_transfered[nbply]);

  if (promotion_of_transfered[nbply].promotee==Empty)
    result = solve(slices[si].next1,n);
  else
  {
    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        sq_transfer,
                                        promotion_of_transfered[nbply].promotee);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      pieces_pawns_continue_promotion_sequence(&promotion_of_transfered[nbply]);
      if (promotion_of_transfered[nbply].promotee!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_promotion[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void skip_wormhole(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++wormhole_next_transfer[nbply];
  TraceValue("%u",wormhole_next_transfer[nbply]);
  TraceSquare(wormhole_positions[wormhole_next_transfer[nbply]-1]);
  TraceText("\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void advance_wormhole(square sq_departure, square sq_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  skip_wormhole();

  while (wormhole_next_transfer[nbply]<=nr_wormholes)
  {
    if (wormhole_positions[wormhole_next_transfer[nbply]-1]==sq_arrival)
      /* wormhole is arrival square - we transfer *from* here! */
      skip_wormhole();
    else if (!is_square_empty(wormhole_positions[wormhole_next_transfer[nbply]-1]))
      /* wormhole is occupied */
      skip_wormhole();
    else
    {
      move_effect_journal_index_type const top = move_effect_journal_base[nbply];
      move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
      if (move_effect_journal[capture].type==move_effect_no_piece_removal
          && moving_pawn_promotion_state[nbply].promotee==Empty
          && wormhole_positions[wormhole_next_transfer[nbply]-1]==sq_departure)
        /* illegal null move */
        skip_wormhole();
      else
        /* next wormhole found! */
        break;
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
stip_length_type wormhole_transferer_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceSquare(sq_arrival);
  TraceValue("%u\n",TSTFLAG(sq_spec[sq_arrival],Wormhole));

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id_transfer[nbply])
  {
    if (TSTFLAG(sq_spec[sq_arrival],Wormhole))
    {
      wormhole_next_transfer[nbply] = 0;
      advance_wormhole(sq_departure,sq_arrival);
    }
    else
      wormhole_next_transfer[nbply] = nr_wormholes+2;
  }

  if (wormhole_next_transfer[nbply]==nr_wormholes+1)
    result = previous_move_is_illegal;
  else if (wormhole_next_transfer[nbply]==nr_wormholes+2)
    result = solve(slices[si].next1,n);
  else
  {
    PieNam const added = get_walk_of_piece_on_square(sq_arrival);
    Flags const addedspec = spec[sq_arrival];
    move_effect_journal_do_piece_removal(move_effect_reason_wormhole_transfer,
                                         sq_arrival);
    move_effect_journal_do_piece_readdition(move_effect_reason_wormhole_transfer,
                                            wormhole_positions[wormhole_next_transfer[nbply]-1],
                                            added,addedspec);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      advance_wormhole(sq_departure,sq_arrival);
      if (wormhole_next_transfer[nbply]<=nr_wormholes)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id_transfer[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_move_allowed(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  square const sq_arrival = move_generation_stack[n].arrival;

  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* we don't detect illegal null moves here because
   * - they are irrelevant for validating observations
   * - they are hard to detect without having played them
   */

  if (TSTFLAG(sq_spec[sq_departure],Wormhole))
    result = true;
  else if (TSTFLAG(sq_spec[sq_arrival],Wormhole))
  {
    unsigned int i;
    for (i = 0; i!=nr_wormholes; ++i)
    {
      square const pos = wormhole_positions[i];
      if (pos!=sq_arrival && is_square_empty(pos))
      {
        result = true;
        break;
      }
    }
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Worm holes
 * @return true iff the observation is valid
 */
boolean wormhole_validate_observation(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (is_move_allowed(current_move[nbply]-1)
            && validate_observation_recursive(slices[si].next1));

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
stip_length_type wormhole_remove_illegal_captures_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_move_allowed);

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
    slice_index const prototype = alloc_pipe(STWormholeRemoveIllegalCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Wormholes
 * @param si root slice of stipulation
 */
void wormhole_initialse_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_wormholes = 0;

  {
    square s;
    for (s = square_a1; s<=square_h8; ++s)
      if (TSTFLAG(sq_spec[s],Wormhole))
      {
        wormhole_positions[nr_wormholes] = s;
        ++nr_wormholes;
      }
  }

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STDoneGeneratingMoves,
                                             &insert_remover);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_moves(si,STWormholeTransferer);
  stip_instrument_moves(si,STWormholeTransferedPromoter);

  stip_instrument_observation_validation(si,nr_sides,STWormholeRemoveIllegalCaptures);
  stip_instrument_check_validation(si,nr_sides,STWormholeRemoveIllegalCaptures);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
