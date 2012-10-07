#include "conditions/dynasty.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

static void update_king_square(Side side)
{
  piece const king_type = side==White ? roib : roin;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbpiece[king_type]);
  if (nbpiece[king_type]==1)
  {
    if (king_square[side]==initsquare)
    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
        if (e[*bnp]==king_type)
        {
          Flags flags = spec[*bnp];
          SETFLAG(flags,Royal);
          move_effect_journal_do_flags_change(move_effect_reason_royal_dynasty,
                                              *bnp,flags);
          enable_castling_rights(*bnp);
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
      disable_castling_rights(king_square[side]);

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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
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
