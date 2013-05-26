#include "conditions/magic_square.h"
#include "pydata.h"
#include "position/pieceid.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

magic_square_type_type magic_square_type;

static void side_change_if_magic(square on, Flags changedspec)
{
  if (TSTFLAG(sq_spec[on],MagicSq)
      && !TSTFLAG(changedspec,Royal)
      && e[on]!=vide
      && GetPieceId(changedspec)==GetPieceId(spec[on]))
    move_effect_journal_do_side_change(move_effect_reason_magic_square,
                                       on,
                                       e[on]<vide ? White : Black);
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
stip_length_type magic_square_side_changer_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
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
        square const on = move_effect_journal[curr].u.piece_addition.on;
        Flags const addedspec = move_effect_journal[curr].u.piece_addition.addedspec;
        side_change_if_magic(on,addedspec);
        break;
      }

      case move_effect_piece_exchange:
      {
        square const from = move_effect_journal[curr].u.piece_exchange.from;
        square const to = move_effect_journal[curr].u.piece_exchange.to;
        side_change_if_magic(from,spec[from]);
        side_change_if_magic(from,spec[to]);
        break;
      }

      default:
        break;
    }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with magic square side changers
 */
void stip_insert_magic_square(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (magic_square_type==magic_square_type1)
    stip_instrument_moves(si,STMagicSquareSideChanger);
  else
    stip_instrument_moves(si,STMagicSquareType2SideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
