#include "conditions/kobul.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean kobulking[nr_sides];

static void substitute(Side trait_ply)
{
  piece const pi_captured = pprise[nbply];
  square const king_pos = king_square[advers(trait_ply)];

  if (pi_captured!=vide
      && kobulking[advers(trait_ply)]
      && king_pos!=initsquare)
  {
    PieNam const kobul_kind = is_pawn(abs(pi_captured)) ? King : abs(pi_captured);
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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type kobul_king_substitutor_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  substitute(slices[si].starter);
  result = solve(slices[si].next1,n);

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

  stip_instrument_moves(si,STKobulKingSubstitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
