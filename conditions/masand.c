#include "conditions/masand.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STMasandRecolorer);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_masand(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean observed(square on_this, square by_that)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(on_this);
  TraceSquare(by_that);
  TraceFunctionParamListEnd();

  fromspecificsquare = by_that;
  if (e[by_that] > vide)
  {
    square const save_king_pos = king_square[Black];
    king_square[Black]= on_this;
    result = rnechec(eval_fromspecificsquare);
    king_square[Black]= save_king_pos;
  }
  else
  {
    square const save_king_pos = king_square[White];
    king_square[White]= on_this;
    result = rbechec(eval_fromspecificsquare);
    king_square[White]= save_king_pos;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void change_observed(square observer_pos, boolean push)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceSquare(observer_pos);
  TraceFunctionParam("%u",push);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
    if (e[*bnp]!=vide
        && *bnp!=king_square[Black]
        && *bnp!=king_square[White]
        && *bnp!=observer_pos
        && observed(*bnp,observer_pos))
    {
      ChangeColour(*bnp);
      if (push)
        PushChangedColour(colour_change_sp[nbply],
                          colour_change_stack_limit,
                          *bnp,
                          e[*bnp]);
    }

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
  {
    change_observed(sq_arrival,flag_outputmultiplecolourchanges);
    TraceValue("%d\n",e[square_f1]);
    result = attack(next,n);
    change_observed(sq_arrival,false);
    TraceValue("%d\n",e[square_f1]);
  }
  else
  {
    TraceValue("!%d\n",e[square_f1]);
    result = attack(next,n);
  }

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
  {
    change_observed(sq_arrival,flag_outputmultiplecolourchanges);
    result = defend(next,n);
    change_observed(sq_arrival,false);
  }
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
