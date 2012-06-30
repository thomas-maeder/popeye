#include "optimisations/killer_move/final_defense_move.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/dead_end.h"
#include "stipulation/battle_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Allocate a STKillerMoveFinalDefenseMove defender slice.
 * @return index of allocated slice
 */
static slice_index alloc_defense_move_against_goal_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKillerMoveFinalDefenseMove);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Optimise a STMoveGenerator slice for defending against a goal
 * @param si identifies slice to be optimised
 */
void killer_move_optimise_final_defense_move(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const last_defense = alloc_defense_move_against_goal_slice();
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
    slice_index const proxy3 = alloc_proxy_slice();
    pipe_append(si,proxy3);
    pipe_link(slices[si].prev,fork);
    pipe_link(proxy1,si);
    pipe_link(proxy2,last_defense);
    pipe_set_successor(last_defense,proxy3);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try the defenses generated in the current ply
 * @param si slice index
 * @return true iff a refutation was found
 */
static stip_length_type try_last_defenses(slice_index si)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = defend(next,slack_length+1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try defenses by pieces other than the killer piece
 * @param si slice index
 * @param defender defending side
 * @param killer_pos square occupied by killer piece
 * @return true iff a refutation was found
 */
static stip_length_type iterate_non_killer(slice_index si,
                                           Side defender,
                                           square killer_pos)
{
  square const *selfbnp;
  stip_length_type result = slack_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,defender,"");
  TraceSquare(killer_pos);
  TraceFunctionParamListEnd();

  for (selfbnp = boardnum; result<=slack_length+1 && *selfbnp!=initsquare; ++selfbnp)
    if (*selfbnp!=killer_pos)
    {
      piece p = e[*selfbnp];
      if (p!=vide)
      {
        move_generation_mode = move_generation_not_optimized;
        TraceValue("->%u\n",move_generation_mode);

        if (defender==White)
        {
          if (p>obs)
            gen_wh_piece(*selfbnp,p);
          else if (TSTFLAG(spec[*selfbnp],Neutral))
            gen_wh_piece(*selfbnp,abs(p));
        }
        else
        {
          if (p<vide)
            gen_bl_piece(*selfbnp,p);
          else if (TSTFLAG(spec[*selfbnp],Neutral))
            gen_bl_piece(*selfbnp,-abs(p));
        }

        {
          stip_length_type const result2 = try_last_defenses(si);
          if (result2>result)
            result = result2;
        }
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try defenses first by the killer piece, then by the other pieces
 * @param si slice index
 * @param killer_pos square occupied by killer piece
 * @return true iff a refutation was found
 */
static stip_length_type iterate_killer_first(slice_index si,
                                             square killer_pos,
                                             piece killer)
{
  Side const defender = slices[si].starter;
  Side const attacker = advers(defender);
  stip_length_type result = slack_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TracePiece(killer);
  TraceFunctionParamListEnd();

  nextply(nbply);
  trait[nbply] = defender;

  if (killer!=vide && killer!=obs)
  {
    if (TSTFLAG(spec[killer_pos],Neutral))
      killer = -e[killer_pos];

    move_generation_mode = move_generation_optimized_by_killer_move;
    TraceValue("->%u\n",move_generation_mode);

    init_move_generation_optimizer();

    if (defender==White)
    {
      if (killer>obs)
        gen_wh_piece(killer_pos,killer);
    }
    else
    {
      if (killer<-obs)
        gen_bl_piece(killer_pos,killer);
    }

    finish_move_generation_optimizer();

    result = try_last_defenses(si);
  }

  if (result<=slack_length+1)
  {
    stip_length_type const result2 = iterate_non_killer(si,defender,killer_pos);
    if (result2>result)
      result = result2;
  }

  finply();

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
stip_length_type killer_move_final_defense_move_defend(slice_index si,
                                                       stip_length_type n)
{
  square const killer_pos = kpilcd[nbply+1];
  piece const killer = e[killer_pos];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length+1);

  TraceSquare(killer_pos);
  TracePiece(killer);
  TraceText("\n");

  result = iterate_killer_first(si,killer_pos,killer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
