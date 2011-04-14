#include "optimisations/killer_move/final_defense_move.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/dead_end.h"
#include "optimisations/optimisation_fork.h"
#include "trace.h"

#include <assert.h>

/* for which Side(s) is the optimisation currently enabled? */
static boolean enabled[nr_sides] =  { false };

/* Reset the enabled state of the optimisation of final defense moves
 */
void reset_killer_move_final_defense_move_optimisation(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  enabled[White] = true;
  enabled[Black] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Disable the optimisation of final defense moves for defense by a side
 * @param side side for which to disable the optimisation
 */
void disable_killer_move_final_defense_move_optimisation(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  enabled[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STKillerMoveFinalDefenseMove defender slice.
 * @return index of allocated slice
 */
static slice_index alloc_defense_move_against_goal_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STKillerMoveFinalDefenseMove,
                        slack_length_battle+1,slack_length_battle);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Optimise a STDefenseMoveGenerator slice for defending against a goal
 * @param si identifies slice to be optimised
 * @param goal goal that slice si defends against
 * @param full_length full length of branch
 */
void killer_move_optimise_final_defense_move(slice_index si,
                                             Goal goal,
                                             stip_length_type full_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",enabled[slices[si].starter]);
  if (enabled[slices[si].starter])
  {
    slice_index const last_defense = alloc_defense_move_against_goal_slice();

    if (full_length<slack_length_battle+2)
      pipe_replace(si,last_defense);
    else
    {
      slice_index const proxy1 = alloc_proxy_slice();
      slice_index const fork = alloc_optimisation_fork_slice(proxy1,1);
      slice_index const proxy2 = alloc_proxy_slice();

      pipe_append(si,proxy2);
      pipe_append(slices[si].prev,fork);

      pipe_link(proxy1,last_defense);
      pipe_set_successor(last_defense,proxy2);
    }
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
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(next,
                                   slack_length_battle+1,
                                   slack_length_battle);

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
  stip_length_type result = slack_length_battle-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,defender,"");
  TraceSquare(killer_pos);
  TraceFunctionParamListEnd();

  for (selfbnp = boardnum; result<=slack_length_battle+1 && *selfbnp!=initsquare; ++selfbnp)
    if (*selfbnp!=killer_pos)
    {
      piece p = e[*selfbnp];
      if (p!=vide)
      {
        move_generation_mode = move_generation_not_optimized;
        TraceValue("->%u\n",move_generation_mode);

        if (TSTFLAG(spec[*selfbnp],Neutral))
          p = -p;
        if (defender==White)
        {
          if (p>obs)
            gen_wh_piece(*selfbnp,p);
        }
        else
        {
          if (p<vide)
            gen_bl_piece(*selfbnp,p);
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
  stip_length_type result = slack_length_battle-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TracePiece(killer);
  TraceFunctionParamListEnd();

  nextply(nbply);
  trait[nbply] = defender;
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(attacker);

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

  if (result<=slack_length_battle+1)
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

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
killer_move_final_defense_move_can_defend_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_max_unsolvable)
{
  square const killer_pos = kpilcd[nbply+1];
  piece const killer = e[killer_pos];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n==slack_length_battle+1);

  TraceSquare(killer_pos);
  TracePiece(killer);
  TraceText("\n");

  result = iterate_killer_first(si,killer_pos,killer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
