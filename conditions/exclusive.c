#include "exclusive.h"
#include "pystip.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "pymsg.h"
#include "pydata.h"
#include "stipulation/goal_reached_tester.h"
#include "pyoutput.h"
#include "trace.h"

static slice_index exclusive_goal_leaf;

static boolean is_reaching_goal_allowed[maxply+1];

/* Perform the necessary verification steps for solving an Exclusive
 * Chess problem
 * @return true iff verification passed
 */
boolean exclusive_verifie_position(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* we have to know which goal has to be reached in a dual-free
   * way */
  exclusive_goal_leaf = find_unique_goal(root_slice);
  if (exclusive_goal_leaf==no_slice)
  {
    VerifieMsg(StipNotSupported);
    result = false;
  }
  else
  {
    flag_testlegality = true;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the current position is illegal because of
 * Exclusive Chess rules
 * @return true iff the position is legal
 */
boolean exclusive_pos_legal(void)
{
  boolean result;
  Goal const goal = slices[exclusive_goal_leaf].u.goal_reached_tester.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply>maxply-1)
    FtlMsg(ChecklessUndecidable);

  result = (is_reaching_goal_allowed[nbply]
            || is_goal_reached(trait[nbply],goal)!=goal_reached);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Do preparations before generating moves for a side in an Exclusive
 * Chess problem
 * @param side side for which to generate moves
 */
void exclusive_init_genmove(Side side)
{
  unsigned int nr_moves_reaching_goal = 0;
  Goal const goal = slices[exclusive_goal_leaf].u.goal_reached_tester.goal;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  CondFlag[exclusive] = false;
  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  remove_ortho_mating_moves_generation_obstacle();
  empile_for_goal = slices[exclusive_goal_leaf].u.goal_reached_tester.goal;
  generate_move_reaching_goal(side);
  empile_for_goal.type = no_goal;
  add_ortho_mating_moves_generation_obstacle();
  CondFlag[exclusive] = true;

  is_reaching_goal_allowed[nbply] = true;

  while (encore() && nr_moves_reaching_goal<2)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && is_goal_reached(side,goal)==goal_reached)
      ++nr_moves_reaching_goal;
    repcoup();
  }

  finply();

  is_reaching_goal_allowed[nbply] = nr_moves_reaching_goal<2;
  TraceValue("%u",nr_moves_reaching_goal);
  TraceValue("%u\n",is_reaching_goal_allowed[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
