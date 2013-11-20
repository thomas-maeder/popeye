#include "conditions/masand.h"
#include "pieces/attributes/neutral/neutral.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_masand(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMasandRecolorer);
  stip_instrument_observation_validation(si,nr_sides,STMasandEnforceObserver);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square current_observer_pos[maxply+1];

/* Validate an observation or observer by making sure it's the checking piece
 * @param si identifies the validator slice
 */
boolean masand_enforce_observer(slice_index si)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observer = current_observer_pos[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = ((sq_observer== initsquare || sq_observer==sq_departure)
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean observed(square on_this, square by_that)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(on_this);
  TraceSquare(by_that);
  TraceFunctionParamListEnd();

  siblingply(trait[nbply]);
  push_observation_target(on_this);
  current_observer_pos[nbply] = by_that;
  result = is_square_observed(&validate_observation);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void change_observed(square observer_pos)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceSquare(observer_pos);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
    if (!is_square_empty(*bnp)
        && *bnp!=king_square[Black]
        && *bnp!=king_square[White]
        && *bnp!=observer_pos
        && !is_piece_neutral(spec[*bnp])
        && observed(*bnp,observer_pos))
      move_effect_journal_do_side_change(move_effect_reason_masand,*bnp);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type masand_recolorer_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    Side const opponent = advers(slices[si].starter);
    if (is_in_check(opponent) && observed(king_square[opponent],pos))
      change_observed(pos);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
