#include "conditions/kobul.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "debugging/trace.h"

#include <assert.h>
#include <string.h>

boolean kobulking[nr_sides];
piece kobul[nr_sides][maxply+1];
Flags kobulspec[nr_sides][maxply+1];

void substitute(Side trait_ply)
{
  piece const pi_captured = pprise[nbply];
  if (pi_captured!=vide
      && kobulking[advers(trait_ply)]
      && king_square[advers(trait_ply)]!=initsquare)
  {
    piece pi_kobul;
    Flags spec_kobul = pprispec[nbply];

    if (is_pawn(pi_captured))
      pi_kobul = trait_ply==White ? roin : roib;
    else
      pi_kobul = pi_captured;

    SETFLAG(spec_kobul,Royal);
    if (TSTFLAG(spec[king_square[advers(trait_ply)]],Neutral))
    {
      SETFLAG(spec_kobul,Black);
      SETFLAG(spec_kobul,White);
      SETFLAG(spec_kobul,Neutral);
      setneutre(&pi_kobul);
    }

    --nbpiece[e[king_square[advers(trait_ply)]]];
    e[king_square[advers(trait_ply)]] = pi_kobul;
    spec[king_square[advers(trait_ply)]] = spec_kobul;
    ++nbpiece[e[king_square[advers(trait_ply)]]];
  }
}

void restore(Side trait_ply)
{
  if (kobulking[advers(trait_ply)]
      && king_square[advers(trait_ply)]!=initsquare)
  {
    --nbpiece[e[king_square[advers(trait_ply)]]];
    e[king_square[advers(trait_ply)]] = kobul[advers(trait_ply)][nbply];
    spec[king_square[advers(trait_ply)]] = kobulspec[advers(trait_ply)][nbply];
    ++nbpiece[kobul[advers(trait_ply)][nbply]];
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
  restore(slices[si].starter);

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
  restore(slices[si].starter);

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

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STKobulKingSubstitutor);
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
void stip_insert_kobul_king_substitutors(slice_index si)
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
