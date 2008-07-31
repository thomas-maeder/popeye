#include "pygoal.h"
#include "pydata.h"

boolean testdblmate= False;

boolean goal_checker_target(Side camp, square target)
{
  return (move_generation_stack[nbcou].arrival==target
          && crenkam[nbply]==initsquare
          && !echecc(camp));
}

boolean goal_checker_circuit(Side camp)
{
  square const cazz = move_generation_stack[nbcou].arrival;
  square const renkam = crenkam[nbply];

  return (((renkam==initsquare && DiaRen(spec[cazz])==cazz)
           || (renkam!=initsquare && DiaRen(spec[renkam])==renkam))
          && !echecc(camp));
}

boolean goal_checker_circuitB(Side camp)
{
  square const sqren = sqrenais[nbply];

  return (sqren!=initsquare
          && DiaRen(spec[sqren])==sqren
          && !echecc(camp));
}

boolean goal_checker_exchange(Side camp)
{
  square const sq_rebirth_kamikaze = crenkam[nbply];
  if (sq_rebirth_kamikaze==initsquare)
  {
    square const sq_arrival = move_generation_stack[nbcou].arrival;
    square const sq_rebirth = DiaRen(spec[sq_arrival]);
    if (DiaRen(spec[sq_rebirth])==sq_arrival
        && (camp==blanc ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
        && sq_rebirth!=sq_arrival)
      return !echecc(camp);
  }
  else
  {
    square const sq_rebirth = DiaRen(spec[sq_rebirth_kamikaze]);
    if (DiaRen(spec[sq_rebirth])==sq_rebirth_kamikaze
        && (camp==blanc ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
        && sq_rebirth!=sq_rebirth_kamikaze)
      return !echecc(camp);
  }

  return false;
}

boolean goal_checker_exchangeB(Side camp)
{
  square const sqren = sqrenais[nbply];

  if (sqren!=initsquare)
  {
    square const sq = DiaRen(spec[sqren]);
    if (DiaRen(spec[sq])==sqren
        && (camp==noir ? e[sq]>=roib : e[sq]<=roin)
        && sq!=sqren
        && !echecc(camp))
      return true;
  }

  return false;
}

boolean goal_checker_capture(Side camp)
{
  return pprise[nbply]!=vide && !echecc(camp);
}

boolean goal_checker_mate(Side camp)
{
  boolean flag;
  Side ad= advers(camp);

  if (CondFlag[amu] && !att_1[nbply])
    return false;

  if (TSTFLAG(PieSpExFlags,Paralyse))
  {
    if (!echecc(ad) || echecc(camp) || !immobile(ad))
      return false;
    genmove(ad);
    flag= encore();
    finply();
    return flag;
  }
  else
    return echecc(ad) && !echecc(camp) && immobile(ad);
}

/* ultraschachzwang is supspended in mates */
boolean goal_checker_mate_ultraschachzwang(Side camp)
{
  int cond = camp==blanc ? blackultraschachzwang : whiteultraschachzwang;
  boolean saveflag = CondFlag[cond];
  boolean result;
  
  CondFlag[cond] = false;
  result = goal_checker_mate(camp);
  CondFlag[cond] = saveflag;

  return result;
}

boolean para_immobile(Side camp)
{
  if (echecc(camp))
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

boolean goal_checker_stale(Side camp)
{
  /* modifiziert fuer paralysierende Steine */
  Side ad= advers(camp);

  if (echecc(camp))
    return false;

  if (TSTFLAG(PieSpExFlags, Paralyse))
    return para_immobile(ad);
  else
    return !echecc(ad) && immobile(ad);
}

boolean goal_checker_dblstale(Side camp)
{
  /* ich glaube, fuer paral. Steine sind hier keine
     Modifizierungen erforderlich  TLi */

  Side ad= advers(camp);
  if (TSTFLAG(PieSpExFlags, Paralyse))
    return (para_immobile(ad) && para_immobile(camp));
  else
    return (!echecc(ad) && !echecc(camp)
            && immobile(ad) && immobile(camp));
}

boolean goal_checker_autostale(Side camp)
{
  if (echecc(advers(camp)))
    return false;

  if (TSTFLAG(PieSpExFlags, Paralyse))
    return para_immobile(camp);
  else
    return !echecc(camp) && immobile(camp);
}

boolean goal_checker_check(Side camp)
{
  return echecc(advers(camp)) && !echecc(camp);
}

boolean goal_checker_steingewinn(Side camp)
{
  return (pprise[nbply]!=vide
          && (!anycirce || sqrenais[nbply]==initsquare)
          && !echecc(camp));
}

boolean goal_checker_ep(Side camp)
{
  return ((move_generation_stack[nbcou].arrival
           !=move_generation_stack[nbcou].capture)
          && is_pawn(pjoue[nbply])
          && !echecc(camp));
}

boolean goal_checker_doublemate(Side camp)
{
  boolean flag;
  Side ad= advers(camp);

  if (!echecc(ad) || !echecc(camp))
    return false;

  if (TSTFLAG(PieSpExFlags, Paralyse))
  {
    genmove(ad);
    flag = encore();
    finply();
    if (!flag)
      return false;
    genmove(camp);
    flag = encore();
    finply();
    if (!flag)
      return false;
  }
  testdblmate= flag_nk;
  /* modified to allow isardam + ##  */
  /* may still have problem with isardam + nK + ##  !*/
  flag=immobile(ad) && immobile(camp);
  testdblmate=false;
  return flag;
}

boolean goal_checker_castling(Side camp)
{
  unsigned char const diff = castling_flag[nbply-1]-castling_flag[nbply];

  return ((diff == whk_castling
           || diff == whq_castling
           || diff == blk_castling
           || diff == blq_castling)
          && !echecc(camp));
}

boolean goal_checker_any(Side camp)
{
  return true;
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
