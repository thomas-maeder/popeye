#include "conditions/wormhole.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/move_generator.h"
#include "solving/post_move_iteration.h"
#include "solving/observation.h"
#include "pieces/walks/pawns/promotion.h"
#include "position/position.h"
#include "position/piece_readdition.h"
#include "position/piece_removal.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

square wormhole_positions[wormholes_capacity];
static unsigned int nr_wormholes;

unsigned int wormhole_next_transfer[maxply+1];

static void skip_wormhole(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++wormhole_next_transfer[nbply];
  TraceValue("%u",wormhole_next_transfer[nbply]);
  TraceSquare(wormhole_positions[wormhole_next_transfer[nbply]-1]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean find_promotion_in_wormhole(square sq_arrival)
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
      move_effect_journal_index_type const base = move_effect_journal_base[nbply];
      move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
      if (move_effect_journal[capture].type==move_effect_no_piece_removal
          && !find_promotion_in_wormhole(sq_arrival)
          && wormhole_positions[wormhole_next_transfer[nbply]-1]==sq_departure)
        /* no capture and no promotion and transfer back to departure square
         * -> this is an illegal null move */
        skip_wormhole();
      else
        /* next wormhole found! */
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transfer_and_solve(slice_index si,
                               square sq_departure,
                               square sq_arrival)
{
  piece_walk_type const added = get_walk_of_piece_on_square(sq_arrival);
  Flags const addedspec = being_solved.spec[sq_arrival];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_removal(move_effect_reason_transfer_choice,
                                       sq_arrival);
  move_effect_journal_do_piece_readdition(move_effect_reason_rebirth_choice,
                                          wormhole_positions[wormhole_next_transfer[nbply]-1],
                                          added,
                                          addedspec,
                                          trait[nbply]);

  post_move_iteration_solve_delegate(si);

  if (!post_move_iteration_is_locked())
  {
    advance_wormhole(sq_departure,sq_arrival);
    if (wormhole_next_transfer[nbply]>nr_wormholes)
      post_move_iteration_end();
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
void wormhole_transferer_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal_get_departure_square(nbply);
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceSquare(sq_arrival);
  TraceValue("%u",TSTFLAG(sq_spec[sq_arrival],Wormhole));
  TraceEOL();

  if (!post_move_am_i_iterating())
  {
    if (TSTFLAG(sq_spec[sq_arrival],Wormhole))
    {
      wormhole_next_transfer[nbply] = 0;
      advance_wormhole(sq_departure,sq_arrival);

      if (wormhole_next_transfer[nbply]==nr_wormholes+1)
        solve_result = this_move_is_illegal;
      else
        transfer_and_solve(si,sq_departure,sq_arrival);
    }
    else
      pipe_solve_delegate(si);
  }
  else
    transfer_and_solve(si,sq_departure,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  result = (is_move_allowed(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

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
void wormhole_remove_illegal_captures_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(MOVEBASE_OF_PLY(nbply),&is_move_allowed);

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
    slice_index const prototype = alloc_pipe(STWormholeRemoveIllegalCaptures);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STWormholeTransferer);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
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
    stip_structure_traversal_override_single(&st,STMove,&instrument_move);
    stip_traverse_structure(si,&st);
  }

  promotion_insert_slice_sequence(si,STWormholeTransferer,&move_insert_slices);

  stip_instrument_observation_validation(si,nr_sides,STWormholeRemoveIllegalCaptures);
  stip_instrument_check_validation(si,nr_sides,STWormholeRemoveIllegalCaptures);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
