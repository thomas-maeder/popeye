#include "pygoal.h"
#include "pydata.h"

boolean testdblmate= False;

/* TODO we shouldn't need this: */
#include "pystip.h"
/* and slices[1] */
static boolean stipChecker_target(couleur camp)
{
  return (move_generation_stack[nbcou].arrival==slices[1].u.leaf.target
          && crenkam[nbply]==initsquare
          && !echecc(camp));
}

static boolean stipChecker_circuit(couleur camp)
{
  square const cazz = move_generation_stack[nbcou].arrival;
  square const renkam = crenkam[nbply];

  return (((renkam==initsquare && DiaRen(spec[cazz])==cazz)
           || (renkam!=initsquare && DiaRen(spec[renkam])==renkam))
          && !echecc(camp));
}

static boolean stipChecker_circuitB(couleur camp)
{
  square const sqren = sqrenais[nbply];

  return (sqren!=initsquare
          && DiaRen(spec[sqren])==sqren
          && !echecc(camp));
}

static boolean stipChecker_exchange(couleur camp)
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

static boolean stipChecker_exchangeB(couleur camp)
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

static boolean stipChecker_capture(couleur camp)
{
  return pprise[nbply]!=vide && !echecc(camp);
}

static boolean stipChecker_mate(couleur camp)
{
  boolean flag;
  couleur ad= advers(camp);

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

/* ultraschachzwang isn't in vigor in mates */
static boolean stipChecker_mate_ultraschachzwang(couleur camp)
{
  int cond = camp==blanc ? blackultraschachzwang : whiteultraschachzwang;
  boolean saveflag = CondFlag[cond];
  boolean result;
  
  CondFlag[cond] = false;
  result = stipChecker_mate(camp);
  CondFlag[cond] = saveflag;

  return result;
}

static boolean para_immobile(couleur camp)
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

static boolean stipChecker_stale(couleur camp)
{
  /* modifiziert fuer paralysierende Steine */
  couleur ad= advers(camp);

  if (echecc(camp))
    return false;

  if (TSTFLAG(PieSpExFlags, Paralyse))
    return para_immobile(ad);
  else
    return !echecc(ad) && immobile(ad);
}

static boolean stipChecker_mate_or_stale(couleur camp)
{
  /* This may be suboptimal if the standard goal_mate and goal_stale
   * are used. On the other hand, it's correct for all goal_checkers,
   * and how many problems with #= are there anyway? TM */

  /* used for writing the correct symbol in output */
  mate_or_stale_patt = goal_checkers[goal_stale](camp);
  
  return mate_or_stale_patt || goal_checkers[goal_mate](camp);
}


static boolean stipChecker_dblstale(couleur camp)
{
  /* ich glaube, fuer paral. Steine sind hier keine
     Modifizierungen erforderlich  TLi */

  couleur ad= advers(camp);
  if (TSTFLAG(PieSpExFlags, Paralyse))
    return (para_immobile(ad) && para_immobile(camp));
  else
    return (!echecc(ad) && !echecc(camp)
            && immobile(ad) && immobile(camp));
}

static boolean stipChecker_autostale(couleur camp)
{
  if (echecc(advers(camp)))
    return false;

  if (TSTFLAG(PieSpExFlags, Paralyse))
    return para_immobile(camp);
  else
    return !echecc(camp) && immobile(camp);
}

static boolean stipChecker_check(couleur camp)
{
  return echecc(advers(camp)) && !echecc(camp);
}

static boolean stipChecker_steingewinn(couleur camp)
{
  return (pprise[nbply]!=vide
          && (!anycirce || sqrenais[nbply]==initsquare)
          && !echecc(camp));
}

static boolean stipChecker_ep(couleur camp)
{
  return ((move_generation_stack[nbcou].arrival
           !=move_generation_stack[nbcou].capture)
          && is_pawn(pjoue[nbply])
          && !echecc(camp));
}

static boolean stipChecker_doublemate(couleur camp)
{
  boolean flag;
  couleur ad= advers(camp);

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

static boolean stipChecker_castling(couleur camp)
{
  unsigned char const diff = castling_flag[nbply-1]-castling_flag[nbply];

  return ((diff == whk_castling
           || diff == whq_castling
           || diff == blk_castling
           || diff == blq_castling)
          && !echecc(camp));
}

static boolean stipChecker_any(couleur camp)
{
  return true;
}

goal_function_t goal_checkers[nr_goals] =
{
  &stipChecker_mate,
  &stipChecker_stale,
  &stipChecker_dblstale,
  &stipChecker_target,
  &stipChecker_check,
  &stipChecker_capture,
  &stipChecker_steingewinn,
  &stipChecker_ep,
  &stipChecker_doublemate,
  &stipChecker_doublemate, /* for countermate */
  &stipChecker_castling,
  &stipChecker_autostale,
  &stipChecker_circuit,
  &stipChecker_exchange,
  &stipChecker_circuitB,
  &stipChecker_exchangeB,
  &stipChecker_mate_or_stale,
  &stipChecker_any,
  0,
#if !defined(DATABASE)
  0
#endif
};
  
char const *goal_end_marker[nr_goals] =
{
  " #",
  " =",
  " ==",
  " z",
  " +",
  " x",
  " %",
  "",
  " ##",
  " ##!",
  "",
  " !=",
  "",
  "",
  "",
  "",
  " #=",
  "",
  " dia",
#if !defined(DATABASE)
  " a=>b"
#endif
};

void initStipCheckers()
{
  if (CondFlag[blackultraschachzwang] || CondFlag[whiteultraschachzwang])
    goal_checkers[goal_mate] = &stipChecker_mate_ultraschachzwang;
  else
    goal_checkers[goal_mate] = &stipChecker_mate;
}
