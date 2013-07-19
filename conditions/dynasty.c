#include "conditions/dynasty.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

static void update_king_square(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  TraceValue("%u\n",number_of_pieces[side][King]);
  if (number_of_pieces[side][King]==1)
  {
    if (king_square[side]==initsquare)
    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
        if (TSTFLAG(spec[*bnp],side) && get_walk_of_piece_on_square(*bnp)==King)
        {
          Flags flags = spec[*bnp];
          SETFLAG(flags,Royal);
          move_effect_journal_do_flags_change(move_effect_reason_royal_dynasty,
                                              *bnp,flags);
          enable_castling_rights(move_effect_reason_royal_dynasty,*bnp);
          break;
        }
    }
  }
  else
  {
    if (king_square[side]!=initsquare)
    {
      Flags flags = spec[king_square[side]];

      /* before move_effect_journal_do_flags_change() or king_square may have
       * been modified
       */
      disable_castling_rights(move_effect_reason_royal_dynasty,king_square[side]);

      CLRFLAG(flags,Royal);
      move_effect_journal_do_flags_change(move_effect_reason_royal_dynasty,
                                          king_square[side],flags);
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
stip_length_type dynasty_king_square_updater_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  update_king_square(White);
  update_king_square(Black);
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_dynasty(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDynastyKingSquareUpdater);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
