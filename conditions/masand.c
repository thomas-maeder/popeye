#include "conditions/masand.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean observed(square on_this, square by_that)
{
  boolean result;
  Side const observed_side = e[by_that]>vide ? Black : White;
  square const save_king_pos = king_square[observed_side];

  TraceFunctionEntry(__func__);
  TraceSquare(on_this);
  TraceSquare(by_that);
  TraceFunctionParamListEnd();

  fromspecificsquare = by_that;
  king_square[observed_side]= on_this;
  result = rechec[observed_side](eval_fromspecificsquare);
  king_square[observed_side]= save_king_pos;

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
    if (e[*bnp]!=vide
        && *bnp!=king_square[Black]
        && *bnp!=king_square[White]
        && *bnp!=observer_pos
        && observed(*bnp,observer_pos))
      move_effect_journal_do_side_change(move_effect_reason_masand,
                                         *bnp,
                                         e[*bnp]<vide ? White : Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type masand_recolorer_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  Side const opponent = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(opponent) && observed(king_square[opponent],sq_arrival))
    change_observed(sq_arrival);

  result = attack(next,n);

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
stip_length_type masand_recolorer_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  Side const opponent = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(opponent) && observed(king_square[opponent],sq_arrival))
    change_observed(sq_arrival);

  result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
