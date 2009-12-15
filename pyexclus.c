#include "pyexclus.h"
#include "pystip.h"
#include "pyleaf.h"
#include "pymsg.h"
#include "pydata.h"
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

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply>maxply-1)
    FtlMsg(ChecklessUndecidable);

  result = (is_reaching_goal_allowed[nbply]
            || (leaf_is_goal_reached(trait[nbply],exclusive_goal_leaf)
                !=goal_reached));

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

  CondFlag[exclusive] = false;
  active_slice[nbply+1] = active_slice[nbply];
  remove_ortho_mating_moves_generation_obstacle();
  generate_move_reaching_goal(exclusive_goal_leaf,side);
  add_ortho_mating_moves_generation_obstacle();
  CondFlag[exclusive] = true;

  is_reaching_goal_allowed[nbply] = true;

  while (encore() && nr_moves_reaching_goal<2)
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(side,exclusive_goal_leaf)==goal_reached)
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
