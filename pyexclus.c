#include "pyexclus.h"
#include "pystip.h"
#include "pyleaf.h"
#include "pymsg.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"

static slice_index exclusive_goal_leaf;

static boolean mateallowed[maxply+1];

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
    optim_orthomatingmoves = false;
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

  result = (mateallowed[nbply]
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
  unsigned int nbrmates = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  mateallowed[nbply] = true;

  /* TODO should we start a new ply here?
   */
  active_slice[nbply+1] = active_slice[nbply];
  if (side==White)
    gen_wh_ply();
  else
    gen_bl_ply();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && leaf_is_goal_reached(side,exclusive_goal_leaf)==goal_reached)
      ++nbrmates;
    repcoup();
  }

  mateallowed[nbply] = nbrmates<2;
  TraceValue("%u",nbrmates);
  TraceValue("%u\n",mateallowed[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
