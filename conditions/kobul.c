#include "conditions/kobul.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean kobulking[nr_sides];
piece kobul[nr_sides][maxply+1];
Flags kobulspec[nr_sides][maxply+1];

static void substitute(Side trait_ply)
{
  piece const pi_captured = pprise[nbply];
  square const king_pos = king_square[advers(trait_ply)];

  if (pi_captured!=vide
      && kobulking[advers(trait_ply)]
      && king_pos!=initsquare)
  {
    PieNam const kobul_kind = is_pawn(pi_captured) ? King : abs(pi_captured);
    piece const pi_kobul = e[king_pos]<=roin ? -kobul_kind : kobul_kind;

    Flags const colour_mask = BIT(White)|BIT(Black)|BIT(Neutral);
    Flags spec_kobul = pprispec[nbply];

    SETFLAG(spec_kobul,Royal);
    SETFLAGMASK(spec_kobul, spec[king_pos]&colour_mask);

    move_effect_journal_do_piece_change(move_effect_reason_kobul_king,
                                        king_pos,
                                        pi_kobul);

    move_effect_journal_do_flags_change(move_effect_reason_kobul_king,
                                        king_pos,
                                        spec_kobul);
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
stip_length_type kobul_king_substitutor_attack(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  substitute(slices[si].starter);
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
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type kobul_king_substitutor_defend(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  substitute(slices[si].starter);
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_kobul_king_substitutors(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves_no_replay(si,STKobulKingSubstitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
