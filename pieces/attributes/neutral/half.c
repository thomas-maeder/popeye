#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static void recolor(Side trait_ply)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  if (TSTFLAG(spec[sq_arrival],HalfNeutral))
  {
    if (TSTFLAG(spec[sq_arrival],Neutral))
    {
      piece const deneutralised = trait[nbply]==Black ? -abs(e[sq_arrival]) : abs(e[sq_arrival]);

      Flags deneutralised_spec = spec[sq_arrival];
      assert(TSTFLAG(deneutralised_spec,White));
      assert(TSTFLAG(deneutralised_spec,Black));
      assert(TSTFLAG(deneutralised_spec,Neutral));
      CLRFLAG(deneutralised_spec,Neutral);
      CLRFLAG(deneutralised_spec,advers(trait_ply));

      move_effect_journal_do_piece_change(move_effect_reason_half_neutral_deneutralisation,
                                          sq_arrival,
                                          deneutralised);

      move_effect_journal_do_flags_change(move_effect_reason_half_neutral_deneutralisation,
                                          sq_arrival,
                                          deneutralised_spec);

      if (king_square[advers(trait_ply)]==sq_arrival)
        move_effect_journal_do_king_square_movement(move_effect_reason_half_neutral_king_movement,
                                                    advers(trait_ply),
                                                    initsquare);
    }
    else
    {
      piece const neutralised = neutral_side ? -abs(e[sq_arrival]) : abs(e[sq_arrival]);

      Flags neutralised_spec = spec[sq_arrival];
      assert(TSTFLAG(neutralised_spec,trait[nbply]));
      SETFLAG(neutralised_spec,Neutral);
      SETFLAG(neutralised_spec,advers(trait_ply));

      move_effect_journal_do_piece_change(move_effect_reason_half_neutral_neutralisation,
                                          sq_arrival,
                                          neutralised);

      move_effect_journal_do_flags_change(move_effect_reason_half_neutral_neutralisation,
                                          sq_arrival,
                                          neutralised_spec);

      if (king_square[trait_ply]==sq_arrival)
        move_effect_journal_do_king_square_movement(move_effect_reason_half_neutral_king_movement,
                                                    advers(trait_ply),
                                                    sq_arrival);
    }

    jouearr[nbply] = e[sq_arrival];
  }
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type half_neutral_recolorer_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  recolor(slices[si].starter);
  result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type half_neutral_recolorer_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  recolor(slices[si].starter);
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_half_neutral_recolorers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_instrument_moves(si,STPiecesHalfNeutralRecolorer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
