#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/move_player.h"
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
      CLRFLAG(spec[sq_arrival],advers(trait_ply));
      CLRFLAG(spec[sq_arrival],Neutral);
      replace_arriving_piece(trait_ply==Black ? -abs(e[sq_arrival]) : abs(e[sq_arrival]));
      if (king_square[advers(trait_ply)]==sq_arrival)
        king_square[advers(trait_ply)] = initsquare;
    }
    else
    {
      assert(TSTFLAG(spec[sq_arrival],trait_ply));
      SETFLAG(spec[sq_arrival],Neutral);
      SETFLAG(spec[sq_arrival],advers(trait_ply));
      replace_arriving_piece(neutral_side==Black ? -abs(e[sq_arrival]) : abs(e[sq_arrival]));
      if (king_square[trait_ply]==sq_arrival)
        king_square[advers(trait_ply)] = sq_arrival;
    }
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

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STPiecesHalfNeutralRecolorer);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_half_neutral_recolorers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
