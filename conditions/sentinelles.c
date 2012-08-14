#include "conditions/sentinelles.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "debugging/trace.h"

#include <assert.h>

static boolean insert_sentinelle(Side trait_ply)
{
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  piece const pi_departing = pjoue[nbply];
  Flags const spec_pi_moving = spec[sq_arrival];
  piece const pi_captured = pprise[nbply];

  if (sq_departure>=square_a2 && sq_departure<=square_h7
      && !is_pawn(pi_departing))
  {
    if (SentPionNeutral) {
      if (TSTFLAG(spec_pi_moving, Neutral)) {
        nbpiece[e[sq_departure]= sentinelb]++;
        SETFLAG(spec[sq_departure], Neutral);
        setneutre(&e[sq_departure]);
      }
      else if ((trait_ply==Black) != SentPionAdverse) {
        nbpiece[e[sq_departure]= sentineln]++;
        SETFLAG(spec[sq_departure], Black);
      }
      else {
        nbpiece[e[sq_departure]= sentinelb]++;
        SETFLAG(spec[sq_departure], White);
      }
      if (   nbpiece[sentinelb] > max_pb
             || nbpiece[sentineln] > max_pn
             || nbpiece[sentinelb]+nbpiece[sentineln] > max_pt)
      {
        /* rules for sentinelles + neutrals not yet
           written but it's very likely this logic will
           need to be refined
        */
        nbpiece[e[sq_departure]]--;
        e[sq_departure]= vide;
        spec[sq_departure]= 0;
      }
      else {
        senti[nbply]= true;
      }
    }
    else if ((trait_ply==Black) != SentPionAdverse) {
      if (   nbpiece[sentineln] < max_pn
             && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
             && (  !flagparasent
                   ||(nbpiece[sentineln]
                      <= nbpiece[sentinelb]
                      +(pi_captured==sentinelb ? 1 : 0))))
      {
        nbpiece[e[sq_departure]= sentineln]++;
        SETFLAG(spec[sq_departure], Black);
        senti[nbply]= true;
      }
    }
    else if ( nbpiece[sentinelb] < max_pb
              && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
              && (!flagparasent
                  ||(nbpiece[sentinelb]
                     <= nbpiece[sentineln]
                     + (pi_captured==sentineln ? 1 : 0))))
    {
      nbpiece[e[sq_departure]= sentinelb]++;
      SETFLAG(spec[sq_departure], White);
      senti[nbply]= true;
    }
  }

  return senti[nbply];
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type sentinelles_inserter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (insert_sentinelle(slices[si].starter))
  {
    result = attack(slices[si].next1,n);
    --nbpiece[e[move_generation_stack[current_move[nbply]].departure]];
    senti[nbply] = false;
  }
  else
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
stip_length_type sentinelles_inserter_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (insert_sentinelle(slices[si].starter))
  {
    result = defend(slices[si].next1,n);
    --nbpiece[e[move_generation_stack[current_move[nbply]].departure]];
    senti[nbply] = false;
  }
  else
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

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STSentinellesInserter);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_sentinelles_inserters(slice_index si)
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
