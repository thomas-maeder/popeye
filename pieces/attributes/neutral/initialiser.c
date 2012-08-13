#include "pieces/attributes/neutral/initialiser.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/move_player.h"
#include "pieces/side_change.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides slice type STPiecesNeutralInitialiser
 */

/* Side that the neutral pieces currently belong to
 */
Side neutral_side;

/* Change the side of the piece on a square so that the piece belongs to the
 * neutral side
 * @param p address of piece whose side to change
 */
void setneutre(piece *p)
{
  Side const current_side = *p<=roin ? Black : White;
  if (neutral_side!=current_side)
    piece_change_side(p);
}

/* Initialise the neutral pieces to belong to the side to be captured in the
 * subsequent move
 * @param captured_side side of pieces to be captured
 */
void initialise_neutrals(Side captured_side)
{
  /* I don't know why, but the solution below is not slower */
  /* than the double loop solution of genblanc(). NG */

  if (neutral_side!=captured_side)
  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (TSTFLAG(spec[*bnp],Neutral))
        piece_change_side(&e[*bnp]);

    neutral_side = captured_side;
  }
}

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index alloc_neutral_initialiser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesNeutralInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type neutral_initialiser_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  initialise_neutrals(advers(slices[si].starter));
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
stip_length_type neutral_initialiser_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  initialise_neutrals(advers(slices[si].starter));
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type neutral_retracting_recolorer_attack(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  /* remember these squares here - attack() may increase current_move[nbply]! */
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(slices[si].next1,n);

  /* make sure that the retracting neutrals belong to the right side if our
   * posteriority has changed the neutral side*/
  if (TSTFLAG(spec[sq_departure],Neutral))
    setneutre(&e[sq_departure]);
  if (TSTFLAG(spec[sq_capture],Neutral))
    setneutre(&e[sq_capture]);

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
stip_length_type neutral_retracting_recolorer_defend(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;
  /* remember these squares here - defend() may increase current_move[nbply]! */
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(slices[si].next1,n);

  /* make sure that the retracting neutrals belong to the right side if our
   * posteriority has changed the neutral side*/
  if (TSTFLAG(spec[sq_departure],Neutral))
    setneutre(&e[sq_departure]);
  if (TSTFLAG(spec[sq_capture],Neutral))
    setneutre(&e[sq_capture]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_initialser(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_neutral_initialiser_slice();
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

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_neutral_initialisers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STGeneratingMoves,
                                           &insert_initialser);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &insert_initialser);
  stip_traverse_structure(si,&st);

  stip_instrument_moves(si,STPiecesNeutralRetractingRecolorer);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
