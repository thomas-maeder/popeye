#include "stipulation/battle_play/defense_move_against_goal.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/defense_fork.h"
#include "trace.h"

#include <assert.h>

/* for which Side(s) is the optimisation currently enabled? */
static boolean enabled[nr_sides] =  { false };

/* Reset the enabled state of the optimisation of final defense moves
 */
void reset_defense_move_against_goal_enabled_state(void)
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
void disable_defense_move_against_goal(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  enabled[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STDefenseMoveAgainstGoal defender slice.
 * @return index of allocated slice
 */
static slice_index alloc_defense_move_against_goal_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_branch(STDefenseMoveAgainstGoal,
                        slack_length_battle+1,slack_length_battle);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Optimise a STDefenseMove slice for defending against a goal
 * @param si identifies slice to be optimised
 * @param goal goal that slice si defends against
 * @return index of allocated slice
 */
void optimise_final_defense_move(slice_index si, Goal goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",enabled[slices[si].starter]);
  if (enabled[slices[si].starter])
  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const fork = alloc_defense_fork_slice(length,min_length,
                                                      proxy1);
    slice_index const last_defense = alloc_defense_move_against_goal_slice();
    slice_index const proxy2 = alloc_proxy_slice();

    pipe_append(slices[si].prev,fork);

    pipe_link(proxy1,last_defense);
    pipe_link(last_defense,proxy2);
    pipe_set_successor(proxy2,slices[si].u.pipe.next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean last_defense_stalemate;

/* Try the defenses generated in the current ply
 * @param si identifies slice
 * @param max_nr_refutations maximum number of refutations to look for
 * @return number of refutations found (0 .. max_nr_refutations+1)
 */
static unsigned int try_last_defenses(slice_index si,
                                      unsigned int max_nr_refutations)
{
  slice_index const next = slices[si].u.pipe.next;
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  while (result<=max_nr_refutations && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
    {
      stip_length_type const
          length_sol = attack_has_solution_in_n(next,
                                                slack_length_battle,
                                                slack_length_battle,
                                                slack_length_battle-1);
      if (length_sol>slack_length_battle)
      {
        last_defense_stalemate = false;
        ++result;
        coupfort();
      }
      else if (length_sol==slack_length_battle)
        last_defense_stalemate = false;
    }

    repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try defenses by pieces other than the killer piece
 * @param si identifies slice
 * @param defender defending side
 * @param killer_pos square occupied by killer piece
 * @param max_nr_refutations maximum number of refutations to look for
 * @return number of refutations found (0 .. max_nr_refutations+1)
 */
static unsigned int iterate_non_killer(slice_index si,
                                       Side defender,
                                       square killer_pos,
                                       unsigned int max_nr_refutations)
{
  square const *selfbnp;
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,defender,"");
  TraceSquare(killer_pos);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  for (selfbnp = boardnum;
       *selfbnp!=initsquare && result<=max_nr_refutations;
       ++selfbnp)
    if (*selfbnp!=killer_pos)
    {
      piece p = e[*selfbnp];
      if (p!=vide)
      {
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
      }

      result += try_last_defenses(si,max_nr_refutations-result);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try defenses first by the killer piece, then by the other pieces
 * @param si identifies slice
 * @param killer_pos square occupied by killer piece
 * @param max_nr_refutations maximum number of refutations to look for
 * @return number of refutations found (0 .. max_nr_refutations+1)
 */
static unsigned int iterate_killer_first(slice_index si,
                                         square killer_pos,
                                         piece killer,
                                         unsigned int max_nr_refutations)
{
  Side const defender = slices[si].starter;
  Side const attacker = advers(defender);
  unsigned int result;
    
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TracePiece(killer);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);

  nextply(nbply);
  trait[nbply] = defender;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(attacker);
  if (TSTFLAG(spec[killer_pos],Neutral))
    killer = -e[killer_pos];

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

  result = try_last_defenses(si,max_nr_refutations);

  if (result<=max_nr_refutations)
  {
    unsigned int const remaining = max_nr_refutations-result;
    result += iterate_non_killer(si,defender,killer_pos,remaining);
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
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
defense_move_against_goal_can_defend_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable,
                                          unsigned int max_nr_refutations)
{
  square const killer_pos = kpilcd[nbply+1];
  piece const killer = e[killer_pos];
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  assert(n==slack_length_battle+1);

  TraceSquare(killer_pos);
  TracePiece(killer);
  TraceText("\n");

  if (killer==obs || killer==vide)
    result = defense_move_can_defend_in_n(si,
                                          slack_length_battle+1,
                                          n_max_unsolvable,
                                          max_nr_refutations);
  else
  {
    last_defense_stalemate = true;

    {
      unsigned int const
          nr_refutations = iterate_killer_first(si,
                                                killer_pos,
                                                killer,
                                                max_nr_refutations);

      if (last_defense_stalemate /* stalemate */
          || nr_refutations>0)   /* refuted */
        result = slack_length_battle+5;
      else
        result = slack_length_battle+1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
