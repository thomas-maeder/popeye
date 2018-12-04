#include "conditions/magic_square.h"
#include "pieces/pieces.h"
#include "pieces/attributes/neutral/neutral.h"
#include "position/pieceid.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "position/effects/side_change.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

ConditionNumberedVariantType magic_square_type;

static void side_change_if_magic(square on, Flags changedspec)
{
  if (TSTFLAG(sq_spec[on],MagicSq)
      && !TSTFLAG(changedspec,Royal)
      && !is_piece_neutral(changedspec)
      && !is_square_empty(on)
      && GetPieceId(changedspec)==GetPieceId(being_solved.spec[on]))
    move_effect_journal_do_side_change(move_effect_reason_magic_square,on);
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
void magic_square_side_changer_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_movement:
      {
        square const to = move_effect_journal[curr].u.piece_movement.to;
        Flags const movingspec = move_effect_journal[curr].u.piece_movement.movingspec;
        side_change_if_magic(to,movingspec);
        break;
      }

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
      {
        square const on = move_effect_journal[curr].u.piece_addition.added.on;
        Flags const addedspec = move_effect_journal[curr].u.piece_addition.added.flags;
        side_change_if_magic(on,addedspec);
        break;
      }

      case move_effect_piece_exchange:
      {
        square const from = move_effect_journal[curr].u.piece_exchange.from;
        square const to = move_effect_journal[curr].u.piece_exchange.to;
        side_change_if_magic(from,being_solved.spec[from]);
        side_change_if_magic(from,being_solved.spec[to]);
        break;
      }

      default:
        break;
    }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with magic square side changers
 */
void solving_insert_magic_square(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (magic_square_type==ConditionType1)
    stip_instrument_moves(si,STMagicSquareSideChanger);
  else
    stip_instrument_moves(si,STMagicSquareType2SideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
