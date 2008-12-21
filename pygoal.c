#include "pygoal.h"
#include "pydata.h"
#include "trace.h"

boolean testdblmate= false;

boolean goal_checker_target(Side just_moved, square target)
{
  return (move_generation_stack[nbcou].arrival==target
          && crenkam[nbply]==initsquare
          && !echecc(nbply,just_moved));
}

boolean goal_checker_circuit(Side just_moved)
{
  square const cazz = move_generation_stack[nbcou].arrival;
  square const renkam = crenkam[nbply];

  return (((renkam==initsquare && DiaRen(spec[cazz])==cazz)
           || (renkam!=initsquare && DiaRen(spec[renkam])==renkam))
          && !echecc(nbply,just_moved));
}

boolean goal_checker_circuitB(Side just_moved)
{
  square const sqren = sqrenais[nbply];

  return (sqren!=initsquare
          && DiaRen(spec[sqren])==sqren
          && !echecc(nbply,just_moved));
}

boolean goal_checker_exchange(Side just_moved)
{
  square const sq_rebirth_kamikaze = crenkam[nbply];
  if (sq_rebirth_kamikaze==initsquare)
  {
    square const sq_arrival = move_generation_stack[nbcou].arrival;
    square const sq_rebirth = DiaRen(spec[sq_arrival]);
    if (DiaRen(spec[sq_rebirth])==sq_arrival
        && (just_moved==White ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
        && sq_rebirth!=sq_arrival)
      return !echecc(nbply,just_moved);
  }
  else
  {
    square const sq_rebirth = DiaRen(spec[sq_rebirth_kamikaze]);
    if (DiaRen(spec[sq_rebirth])==sq_rebirth_kamikaze
        && (just_moved==White ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
        && sq_rebirth!=sq_rebirth_kamikaze)
      return !echecc(nbply,just_moved);
  }

  return false;
}

boolean goal_checker_exchangeB(Side just_moved)
{
  square const sqren = sqrenais[nbply];

  if (sqren!=initsquare)
  {
    square const sq = DiaRen(spec[sqren]);
    if (DiaRen(spec[sq])==sqren
        && (just_moved==Black ? e[sq]>=roib : e[sq]<=roin)
        && sq!=sqren
        && !echecc(nbply,just_moved))
      return true;
  }

  return false;
}

boolean goal_checker_capture(Side just_moved)
{
  return pprise[nbply]!=vide && !echecc(nbply,just_moved);
}

boolean goal_checker_mate(Side just_moved)
{
  boolean flag;
  Side ad= advers(just_moved);

  if (CondFlag[amu] && !att_1[nbply])
    return false;

  if (TSTFLAG(PieSpExFlags,Paralyse))
  {
    if (!echecc(nbply,ad) || echecc(nbply,just_moved) || !immobile(ad))
      return false;
    genmove(ad);
    flag= encore();
    finply();
    return flag;
  }
  else
    return echecc(nbply,ad) && !echecc(nbply,just_moved) && immobile(ad);
}

/* ultraschachzwang is supspended in mates */
boolean goal_checker_mate_ultraschachzwang(Side just_moved)
{
  int cond = (just_moved==White
              ? blackultraschachzwang
              : whiteultraschachzwang);
  boolean saveflag = CondFlag[cond];
  boolean result;
  
  CondFlag[cond] = false;
  result = goal_checker_mate(just_moved);
  CondFlag[cond] = saveflag;

  return result;
}

boolean para_immobile(Side camp)
{
  if (echecc(nbply,camp))
  {
    boolean flag;
    genmove(camp);
    flag= !encore();
    finply();
    return flag;
  }
  else
    return immobile(camp);
}

boolean goal_checker_stale(Side just_moved)
{
  /* modifiziert fuer paralysierende Steine */
  Side ad= advers(just_moved);
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",just_moved);

  if (!echecc(nbply,just_moved))
  {
    if (TSTFLAG(PieSpExFlags, Paralyse))
      result = para_immobile(ad);
    else
      result = !echecc(nbply,ad) && immobile(ad);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

boolean goal_checker_dblstale(Side just_moved)
{
  /* ich glaube, fuer paral. Steine sind hier keine
     Modifizierungen erforderlich  TLi */

  Side ad= advers(just_moved);
  if (TSTFLAG(PieSpExFlags, Paralyse))
    return (para_immobile(ad) && para_immobile(just_moved));
  else
    return (!echecc(nbply,ad) && !echecc(nbply,just_moved)
            && immobile(ad) && immobile(just_moved));
}

boolean goal_checker_autostale(Side just_moved)
{
  if (echecc(nbply,advers(just_moved)))
    return false;

  if (TSTFLAG(PieSpExFlags, Paralyse))
    return para_immobile(just_moved);
  else
    return !echecc(nbply,just_moved) && immobile(just_moved);
}

boolean goal_checker_check(Side just_moved)
{
  return echecc(nbply,advers(just_moved)) && !echecc(nbply,just_moved);
}

boolean goal_checker_steingewinn(Side just_moved)
{
  return (pprise[nbply]!=vide
          && (!anycirce || sqrenais[nbply]==initsquare)
          && !echecc(nbply,just_moved));
}

boolean goal_checker_ep(Side just_moved)
{
  return ((move_generation_stack[nbcou].arrival
           !=move_generation_stack[nbcou].capture)
          && is_pawn(pjoue[nbply])
          && !echecc(nbply,just_moved));
}

boolean goal_checker_doublemate(Side just_moved)
{
  boolean flag;
  Side ad= advers(just_moved);

  if (!echecc(nbply,ad) || !echecc(nbply,just_moved))
    return false;

  if (TSTFLAG(PieSpExFlags, Paralyse))
  {
    genmove(ad);
    flag = encore();
    finply();
    if (!flag)
      return false;
    genmove(just_moved);
    flag = encore();
    finply();
    if (!flag)
      return false;
  }
  testdblmate= flag_nk;
  /* modified to allow isardam + ##  */
  /* may still have problem with isardam + nK + ##  !*/
  flag=immobile(ad) && immobile(just_moved);
  testdblmate=false;
  return flag;
}

boolean goal_checker_castling(Side just_moved)
{
  unsigned char const diff = castling_flag[nbply-1]-castling_flag[nbply];

  return ((diff == whk_castling
           || diff == whq_castling
           || diff == blk_castling
           || diff == blq_castling)
          && !echecc(nbply,just_moved));
}

boolean goal_checker_any(Side just_moved)
{
  return !echecc(nbply,just_moved);
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
#if !defined(DATABASE)
  , " a=>b"
#endif
  , " #="
};
