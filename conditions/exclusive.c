#include "exclusive.h"
#include "pystip.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "pymsg.h"
#include "pydata.h"
#include "stipulation/goals/goals.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

static Goal exclusive_goal;

static boolean is_reaching_goal_allowed[maxply+1];

/* Perform the necessary verification steps for solving an Exclusive
 * Chess problem
 * @param si identifies root slice of stipulation
 * @return true iff verification passed
 */
boolean exclusive_verifie_position(slice_index si)
{
  boolean result;
  Goal goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we have to know which goal has to be reached in a dual-free
   * way */
  goal = find_unique_goal(si);
  if (goal.type==no_goal)
  {
    VerifieMsg(StipNotSupported);
    result = false;
  }
  else
  {
    exclusive_goal = goal;
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

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(exclusive_goal.type==goal_mate);

  if (nbply>maxply-1)
    FtlMsg(ChecklessUndecidable);

  result = (is_reaching_goal_allowed[nbply]
            || goal_checker_mate(trait[nbply])!=goal_reached);

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

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  assert(exclusive_goal.type==goal_mate);

  CondFlag[exclusive] = false;
  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  remove_ortho_mating_moves_generation_obstacle();
  empile_for_goal = exclusive_goal;
  generate_move_reaching_goal(side);
  empile_for_goal.type = no_goal;
  add_ortho_mating_moves_generation_obstacle();
  CondFlag[exclusive] = true;

  is_reaching_goal_allowed[nbply] = true;

  while (encore() && nr_moves_reaching_goal<2)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && goal_checker_mate(side)==goal_reached)
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
