#include "pygoal.h"
#include "pydata.h"
#include "pyproof.h"
#include "trace.h"

#include <assert.h>

#define ENUMERATION_TYPENAME goal_checker_result_type
#define ENUMERATORS                             \
  ENUMERATOR(goal_not_reached),                 \
    ENUMERATOR(goal_not_reached_selfcheck),     \
    ENUMERATOR(goal_reached)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


boolean testdblmate = false;

goal_checker_result_type goal_checker_target(Side just_moved, square target)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_circuit(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_circuitB(Side just_moved)
{
  /* goal is only reachable in some fairy conditions */
  return goal_not_reached;
}

goal_checker_result_type goal_checker_exchange(Side just_moved)
{
  square const sq_arrival = move_generation_stack[nbcou].arrival;
  square const sq_diagram = GetDiaRen(spec[sq_arrival]);

  if (GetDiaRen(spec[sq_diagram])==sq_arrival
      && (just_moved==White ? e[sq_diagram]>=roib : e[sq_diagram]<=roin)
      && sq_diagram!=sq_arrival)
    return (echecc(nbply,just_moved)
            ? goal_not_reached_selfcheck
            : goal_reached);
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_exchangeB(Side just_moved)
{
  /* goal is only reachable in some fairy conditions */
  return goal_not_reached;
}

goal_checker_result_type goal_checker_capture(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_mate(Side just_moved)
{
  Side const ad = advers(just_moved);
  if (echecc(nbply,ad))
  {
    if (echecc(nbply,just_moved))
      return goal_not_reached_selfcheck;
    else if (immobile(ad))
      return goal_reached;
    else
      return goal_not_reached;
  }
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_stale(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_dblstale(Side just_moved)
{
  Side const ad = advers(just_moved);
  goal_checker_result_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",just_moved);
  TraceFunctionParamListEnd();

  if (echecc(nbply,just_moved))
    result = goal_not_reached_selfcheck;
  else if (echecc(nbply,ad))
    result = goal_not_reached;
  else if (immobile(ad) && immobile(just_moved))
    result = goal_reached;
  else
    result = goal_not_reached;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

goal_checker_result_type goal_checker_autostale(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_check(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_steingewinn(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_ep(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_doublemate(Side just_moved)
{
  assert(0);
  return goal_not_reached;
}

goal_checker_result_type goal_checker_castling(Side just_moved)
{
  unsigned char const diff = castling_flag[nbply-1]-castling_flag[nbply];

  if (diff==whk_castling || diff==whq_castling
      || diff==blk_castling || diff==blq_castling)
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_any(Side just_moved)
{
  return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
}

goal_checker_result_type goal_checker_proof(Side just_moved)
{
  if (ProofIdentical())
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
  else
    return goal_not_reached;
}

char const *goal_end_marker[nr_goals] =
{
  " #"
  , " ="
  , " =="
  , " z"
  , " +"
  , " x"
  , " %"
  , ""
  , " ##"
  , " ##!"
  , ""
  , " !="
  , ""
  , ""
  , ""
  , ""
  , ""
  , " dia"
  , " a=>b"
  , " #="
};
