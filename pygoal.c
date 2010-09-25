#include "pygoal.h"
#include "pydata.h"
#include "pyproof.h"
#include "trace.h"


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
  if (move_generation_stack[nbcou].arrival==target
      && crenkam[nbply]==initsquare)
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_circuit(Side just_moved)
{
  square const cazz = move_generation_stack[nbcou].arrival;
  square const renkam = crenkam[nbply];

  if ((renkam==initsquare && DiaRen(spec[cazz])==cazz)
      || (renkam!=initsquare && DiaRen(spec[renkam])==renkam))
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_circuitB(Side just_moved)
{
  square const sqren = sqrenais[nbply];

  if (sqren!=initsquare && DiaRen(spec[sqren])==sqren)
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_exchange(Side just_moved)
{
  square const sq_rebirth_kamikaze = crenkam[nbply];
  if (sq_rebirth_kamikaze==initsquare)
  {
    square const sq_arrival = move_generation_stack[nbcou].arrival;
    square const sq_rebirth = DiaRen(spec[sq_arrival]);
    if (DiaRen(spec[sq_rebirth])==sq_arrival
        && (just_moved==White ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
        && sq_rebirth!=sq_arrival)
      return (echecc(nbply,just_moved)
              ? goal_not_reached_selfcheck
              : goal_reached);
    else
      return goal_not_reached;
  }
  else
  {
    square const sq_rebirth = DiaRen(spec[sq_rebirth_kamikaze]);
    if (DiaRen(spec[sq_rebirth])==sq_rebirth_kamikaze
        && (just_moved==White ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
        && sq_rebirth!=sq_rebirth_kamikaze)
      return (echecc(nbply,just_moved)
              ? goal_not_reached_selfcheck
              : goal_reached);
    else
      return goal_not_reached;
  }
}

goal_checker_result_type goal_checker_exchangeB(Side just_moved)
{
  square const sqren = sqrenais[nbply];

  if (sqren==initsquare)
    return goal_not_reached;
  else
  {
    square const sq = DiaRen(spec[sqren]);
    if (DiaRen(spec[sq])==sqren
        && (just_moved==Black ? e[sq]>=roib : e[sq]<=roin)
        && sq!=sqren)
      return (echecc(nbply,just_moved)
              ? goal_not_reached_selfcheck
              : goal_reached);
    else
      return goal_not_reached;
  }
}

goal_checker_result_type goal_checker_capture(Side just_moved)
{
  if (pprise[nbply]==vide)
    return goal_not_reached;
  else
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
}

static boolean is_totally_paralysed(Side side)
{
  boolean result;
  move_generation_mode = move_generation_not_optimized;
  genmove(side);
  result = !encore();
  finply();
  return result;
}

goal_checker_result_type goal_checker_mate(Side just_moved)
{
  Side const ad = advers(just_moved);
  if (echecc(nbply,ad))
  {
    if (echecc(nbply,just_moved))
      return goal_not_reached_selfcheck;
    else if (immobile(ad))
    {
      if (TSTFLAG(PieSpExFlags,Paralyse))
        return is_totally_paralysed(ad) ? goal_not_reached : goal_reached;
      else
        return goal_reached;
    }
    else
      return goal_not_reached;
  }
  else
    return goal_not_reached;
}

/* ultraschachzwang is supspended in mates */
goal_checker_result_type goal_checker_mate_ultraschachzwang(Side just_moved)
{
  int const cond = (just_moved==White
                    ? blackultraschachzwang
                    : whiteultraschachzwang);
  boolean const saveflag = CondFlag[cond];
  boolean result;

  CondFlag[cond] = false;
  result = goal_checker_mate(just_moved);
  CondFlag[cond] = saveflag;

  return result;
}

static boolean para_stalemate(Side camp)
{
  if (echecc(nbply,camp))
    return is_totally_paralysed(camp);
  else
    return immobile(camp);
}

goal_checker_result_type goal_checker_stale(Side just_moved)
{
  Side const ad = advers(just_moved);
  goal_checker_result_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",just_moved);
  TraceFunctionParamListEnd();

  if (echecc(nbply,just_moved))
    result = goal_not_reached_selfcheck;
  else if (TSTFLAG(PieSpExFlags, Paralyse))
    result = para_stalemate(ad) ? goal_reached : goal_not_reached;
  else
    result = !echecc(nbply,ad) && immobile(ad) ? goal_reached : goal_not_reached;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

goal_checker_result_type goal_checker_dblstale(Side just_moved)
{
  Side const ad = advers(just_moved);
  if (TSTFLAG(PieSpExFlags, Paralyse))
    return (para_stalemate(ad) && para_stalemate(just_moved)
            ? goal_reached
            : goal_not_reached);
  else if (echecc(nbply,ad))
    return goal_not_reached;
  else if (echecc(nbply,just_moved))
    return goal_not_reached_selfcheck;
  else
    return (immobile(ad) && immobile(just_moved)
            ? goal_reached
            : goal_not_reached);
}

goal_checker_result_type goal_checker_autostale(Side just_moved)
{
  if (echecc(nbply,advers(just_moved)))
    return goal_not_reached;
  else if (TSTFLAG(PieSpExFlags, Paralyse))
    return para_stalemate(just_moved) ? goal_reached : goal_not_reached;
  else if (echecc(nbply,just_moved))
    return goal_not_reached_selfcheck;
  else
    return immobile(just_moved) ? goal_reached : goal_not_reached;
}

goal_checker_result_type goal_checker_check(Side just_moved)
{
  if (echecc(nbply,advers(just_moved)))
  {
    if (echecc(nbply,just_moved))
      return goal_not_reached_selfcheck;
    else
      return goal_reached;
  }
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_steingewinn(Side just_moved)
{
  if (pprise[nbply]==vide || (anycirce && sqrenais[nbply]!=initsquare))
    return goal_not_reached;
  else
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
}

goal_checker_result_type goal_checker_ep(Side just_moved)
{
  if (move_generation_stack[nbcou].arrival!=move_generation_stack[nbcou].capture
      && is_pawn(pjoue[nbply]))
    return echecc(nbply,just_moved) ? goal_not_reached_selfcheck : goal_reached;
  else
    return goal_not_reached;
}

goal_checker_result_type goal_checker_doublemate(Side just_moved)
{
  Side const ad = advers(just_moved);

  if (echecc(nbply,ad) && echecc(nbply,just_moved))
  {
    if (TSTFLAG(PieSpExFlags,Paralyse))
    {
      if (is_totally_paralysed(ad) || is_totally_paralysed(just_moved))
        return goal_not_reached;
    }

    {
      boolean flag;
      testdblmate = flag_nk;
      /* modified to allow isardam + ##  */
      /* may still have problem with isardam + nK + ##  !*/
      flag = immobile(ad) && immobile(just_moved);
      testdblmate = false;
      return flag ? goal_reached : goal_not_reached;
    }
  }
  else
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
