#include "conditions/einstein/reverse.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "conditions/einstein/einstein.h"
#include "stipulation/pipe.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "position/walk_change.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static void adjust(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const capturer_origin = einstein_collect_capturers();

    move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
    move_effect_journal_index_type curr;
    assert(move_effect_journal_base[parent_ply[nbply]+1]<=top);
    for (curr = move_effect_journal_base[parent_ply[nbply]+1]; curr!=top; ++curr)
      if (move_effect_journal[curr].type==move_effect_piece_movement
          && (move_effect_journal[curr].reason==move_effect_reason_moving_piece_movement
              || move_effect_journal[curr].reason==move_effect_reason_castling_king_movement
              || move_effect_journal[curr].reason==move_effect_reason_castling_partner_movement))
      {
        square const from = move_effect_journal[curr].u.piece_movement.from;
        square const to = move_effect_journal[curr].u.piece_movement.to;
        piece_walk_type const einsteined = get_walk_of_piece_on_square(to);
        piece_walk_type const substitute = (capturer_origin==from
                                  ? einstein_decrease_walk(einsteined)
                                  : einstein_increase_walk(einsteined));
        if (einsteined!=substitute)
          move_effect_journal_do_walk_change(move_effect_reason_einstein_chess,
                                              to,substitute);
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
void reverse_einstein_moving_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  adjust();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_reverse_einstein_moving_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STReverseEinsteinArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
