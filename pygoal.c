#include "pygoal.h"
#include "pydata.h"

#include <stdlib.h>

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
        && (camp==White ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
        && sq_rebirth!=sq_arrival)
      return !echecc(camp);
  }
  else
  {
    square const sq_rebirth = DiaRen(spec[sq_rebirth_kamikaze]);
    if (DiaRen(spec[sq_rebirth])==sq_rebirth_kamikaze
        && (camp==White ? e[sq_rebirth]>=roib : e[sq_rebirth]<=roin)
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
        && (camp==Black ? e[sq]>=roib : e[sq]<=roin)
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
  int cond = camp==White ? blackultraschachzwang : whiteultraschachzwang;
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

typedef Flags ColourSpec;

static boolean IsABattery(square KingSquare,
                          square FrontSquare,
                          numvec Direction,
                          ColourSpec ColourMovingPiece,
                          piece BackPiece1,
                          piece BackPiece2)
{
  square sq;
  piece p;

  /* is the line between king and front piece empty? */
  EndOfLine(FrontSquare, Direction, sq);
  if (sq == KingSquare) {
    /* the line is empty, but is there really an appropriate back
    ** battery piece? */
    EndOfLine(FrontSquare, -Direction, sq);
    p= e[sq];
    if (p < vide)
      p= -p;
    if (   (p == BackPiece1 || p == BackPiece2)
           && TSTFLAG(spec[sq], ColourMovingPiece))
    {
      /* So, it is a battery. */
      return true;
    }
  }
  return false;
} /* IsABattery */

static void GenMatingPawn(square sq_departure,
                          square sq_king,
                          ColourSpec ColourMovingPiece)
{
  boolean Battery = false;
  numvec k;
  square sq_arrival;

  k= CheckDirBishop[sq_king-sq_departure];
  if (k!=0)
    Battery=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else {
    k= CheckDirRook[sq_king-sq_departure];
    if (k!=0)
      Battery=
        IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen);
  }

  /* if the pawn is not the front piece of a battery reset k,
     otherwise normalise it to be positiv. This is necessary to
     avoid moves along the battery line subsequently.
  */
  if (Battery) {
    if (k<0)
      k= -k;
  }
  else
    k= 0;

  if (ColourMovingPiece==White) {
    if (sq_departure<=square_h1)
      return;
    else {
      /* not first rank */
      /* ep captures */
      if (ep[nbply-1]!=initsquare
          && trait[nbply-1]!=trait[nbply]
          && (sq_departure+dir_up+dir_right==ep[nbply-1]
              || sq_departure+dir_up+dir_left==ep[nbply-1]))
      {
        if (nbply==2)    /* ep.-key  standard pawn */
          move_generation_stack[repere[2]].arrival= ep[nbply-1]+dir_down;
        empile(sq_departure,
               ep[nbply-1],
               move_generation_stack[repere[nbply]].arrival);
      }

      /* single step */
      if (k!=24) {
        /* suppress moves along a battery line */
        sq_arrival= sq_departure+dir_up;
        if (e[sq_arrival]==vide) {
          if (Battery
              || sq_arrival+dir_up+dir_left == sq_king
              || sq_arrival+dir_up+dir_right == sq_king
              || (PromSq(White,sq_arrival)
                  && (CheckDirQueen[sq_king-sq_arrival]
                      || CheckDirKnight[sq_king-sq_arrival])))
            empile(sq_departure,sq_arrival,sq_arrival);

          /* double step */
          if (sq_departure<=square_h2) {
            sq_arrival+= onerow;
            if (e[sq_arrival]==vide
                && (Battery
                    || sq_arrival+dir_up+dir_left==sq_king
                    || sq_arrival+dir_up+dir_right==sq_king))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }

      /* capture+dir_up+dir_left */
      sq_arrival= sq_departure+dir_up+dir_left;
      if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],Black))
        if (Battery
            || sq_arrival+dir_up+dir_left == sq_king
            || sq_arrival+dir_up+dir_right == sq_king
            || (PromSq(White,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);
      
      /* capture+dir_up+dir_right */
      sq_arrival= sq_departure+dir_up+dir_right;
      if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],Black))
        if (Battery
            || sq_arrival+dir_up+dir_left==sq_king
            || sq_arrival+dir_up+dir_right==sq_king
            || (PromSq(White,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    if (sq_departure>=square_a8)
      return;

    /* not last rank */
    /* ep captures */
    if (ep[nbply-1]!=initsquare
        && trait[nbply-1] != trait[nbply]
        && (sq_departure+dir_down+dir_left==ep[nbply-1]
            || sq_departure+dir_down+dir_right==ep[nbply-1]))
    {
      if (nbply==2)    /* ep.-key  standard pawn */
        move_generation_stack[repere[2]].arrival= ep[nbply-1]+dir_up;
      empile(sq_departure,
             ep[nbply-1],
             move_generation_stack[repere[nbply]].arrival);
    }

    /* single step */
    if (k!=24) {    /* suppress moves along a battery line */
      sq_arrival= sq_departure+dir_down;
      if (e[sq_arrival]==vide) {
        if (Battery
            || sq_arrival+dir_down+dir_right==sq_king
            || sq_arrival+dir_down+dir_left==sq_king
            || (PromSq(Black,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);

        /* double step */
        if (sq_departure>=square_a7) {
          sq_arrival-= onerow;
          if (e[sq_arrival] == vide
              && (Battery
                  || sq_arrival+dir_down+dir_right==sq_king
                  || sq_arrival+dir_down+dir_left==sq_king))
            empile(sq_departure,sq_arrival,sq_arrival);
        }
      }
    }

    /* capture+dir_up+dir_left */
    sq_arrival= sq_departure+dir_down+dir_right;
    if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],White)) {
      if (Battery
          || sq_arrival+dir_down+dir_right==sq_king
          || sq_arrival+dir_down+dir_left==sq_king
          || (PromSq(Black,sq_arrival)
              && (CheckDirQueen[sq_king-sq_arrival]
                  || CheckDirKnight[sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }

    /* capture+dir_up+dir_right */
    sq_arrival= sq_departure+dir_down+dir_left;
    if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],White)) {
      if (Battery
          || sq_arrival+dir_down+dir_right==sq_king
          || sq_arrival+dir_down+dir_left==sq_king
          || (PromSq(Black,sq_arrival)
              && (CheckDirQueen[sq_king-sq_arrival]
                  || CheckDirKnight[sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
} /* GenMatingPawn */

static void GenMatingKing(Goal goal,
                          square sq_departure,
                          square sq_king,
                          ColourSpec ColourMovingPiece)
{
  numvec    k, k2;
  boolean   Generate = false;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  if (rb==rn) {
    /* neutral kings */
    for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
      sq_arrival= sq_departure+vec[k2];
      /* they must capture to mate the opponent */
      if (e[sq_arrival]!=vide
          && TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    /* check if the king is the front piece of a battery
       that can fire */
    k= CheckDirBishop[sq_king-sq_departure];
    if (k!=0)
      Generate=
        IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
    else {
      k= CheckDirRook[sq_king-sq_departure];
      if (k!=0)
        Generate= IsABattery(sq_king,sq_departure,k,ColourMovingPiece,
                             Rook,Queen);
    }

    if (Generate)
      for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
        /* prevent the king from moving along the battery line*/
        if (k2==k || k2==-k)
          continue;
        sq_arrival= sq_departure+vec[k2];
        if ((e[sq_arrival]==vide
             || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
            && move_diff_code[abs(sq_king-sq_arrival)]>1+1) /* no contact */
          empile(sq_departure,sq_arrival,sq_arrival);
      }

    if (CondFlag[ColourCapturedPiece==White ? whiteedge : blackedge]
        || goal==goal_doublemate)
      for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
        sq_arrival= sq_departure + vec[k2];
        if ((e[sq_arrival]==vide
             || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
            && move_diff_code[abs(sq_king-sq_arrival)]<=1+1)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
  }

  /* castling */
  if (castling_supported) {
    if (ColourMovingPiece==White)
      /* white to play */
      genrb_cast();
    else
      /* black to play */
      genrn_cast();
  }
}

static void GenMatingKnight(square sq_departure,
                            square sq_king,
                            ColourSpec ColourMovingPiece)
{
  numvec    k;
  boolean   Generate = false;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  /* check if the knight is the front piece of a battery that can
     fire
  */
  if ((k = CheckDirBishop[sq_king-sq_departure])!=0)
    Generate=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else if ((k = CheckDirRook[sq_king-sq_departure])!=0)
    Generate= IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen);

  k= abs(sq_king-sq_departure);
  if (Generate
      || (SquareCol(sq_departure) == SquareCol(sq_king)
          && move_diff_code[k]<=move_diff_code[square_a3-square_e1]
          && move_diff_code[k]!=move_diff_code[square_a3-square_c1]))
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_arrival= sq_departure+vec[k];
      if (e[sq_arrival]==vide
          || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        if (Generate || CheckDirKnight[sq_arrival-sq_king]!=0)
          empile(sq_departure,sq_arrival,sq_arrival);
    }
}

static void GenMatingRook(square sq_departure,
                          square sq_king,
                          ColourSpec ColourMovingPiece)
{
  square    sq2;
  numvec    k, k2;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  /* check if the rook is the front piece of a battery that can fire
   */
  k= CheckDirBishop[sq_king-sq_departure];
  if (k != 0
      && IsABattery(sq_king, sq_departure, k, ColourMovingPiece, Bishop, Queen))
  {
    for (k= vec_rook_start; k<=vec_rook_end; k++) {
      sq_arrival= sq_departure+vec[k];
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= vec[k];
      }
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    int OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];

    k2= CheckDirRook[sq_king-sq_departure];
    if (k2!=0) {
      /* the rook is already on a line with the king */
      EndOfLine(sq_departure,k2,sq_arrival);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
    else {
      for (k= vec_rook_start; k<=vec_rook_end; k++) {
        sq_arrival= sq_departure+vec[k];
        if (e[sq_arrival]==obs)
          continue;
        if (move_diff_code[abs(sq_arrival-sq_king)]<OriginalDistance) {
          /* The rook must move closer to the king! */
          k2= CheckDirRook[sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDirRook[sq_king-sq_arrival];
          }

          /* We are at the end of the line or in checking
             distance
          */
          if (k2==0)
            continue;
          if (e[sq_arrival]==vide
              || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
          {
            EndOfLine(sq_arrival,k2,sq2);
            if (sq2==sq_king)
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
}

static void GenMatingQueen(square sq_departure,
                           square sq_king,
                           ColourSpec ColourMovingPiece)
{
  square sq2;
  numvec  k, k2;
  ColourSpec ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  for (k= vec_queen_start; k<=vec_queen_end; k++) {
    sq_arrival= sq_departure+vec[k];
    while (e[sq_arrival]==vide) {
      k2= CheckDirQueen[sq_king-sq_arrival];
      if (k2) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
      sq_arrival+= vec[k];
    }
    if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
      k2= CheckDirQueen[sq_king-sq_arrival];
      if (k2) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void GenMatingBishop(square sq_departure,
                            square sq_king,
                            ColourSpec ColourMovingPiece)
{
  square    sq2;
  numvec    k, k2;
  ColourSpec    ColourCapturedPiece = advers(ColourMovingPiece);

  square sq_arrival;

  /* check if the bishop is the front piece of a battery that can
     fire
  */
  k = CheckDirRook[sq_king-sq_departure];
  if (k!=0
      && IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen))
  {
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      sq_arrival= sq_departure+vec[k];
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= vec[k];
      }
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else if (SquareCol(sq_departure)==SquareCol(sq_king)) {
    int OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];

    k2= CheckDirBishop[sq_king-sq_departure];
    if (k2) {
      /* the bishop is already on a line with the king */
      EndOfLine(sq_departure,k2,sq_arrival);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
    else {
      for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
        sq_arrival= sq_departure+vec[k];
        if (e[sq_arrival]==obs)
          continue;
        if (move_diff_code[abs(sq_arrival-sq_king)]
            <OriginalDistance) {
          /* The bishop must move closer to the king! */
          k2= CheckDirBishop[sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDirBishop[sq_king-sq_arrival];
          }

          /* We are at the end of the line or in checking
             distance */
          if (k2==0)
            continue;
          if (e[sq_arrival]==vide
              || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
          {
            EndOfLine(sq_arrival,k2,sq2);
            if (sq2==sq_king)
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
} /* GenMatingBishop */

void generate_move_reaching_goal(Goal goal, Side side_at_move)
{
  if (optim_orthomatingmoves)
  {
    square square_a = square_a1;
    square const OpponentsKing = side_at_move==White ? rn : rb;
    int i;

    nextply();
    trait[nbply]= side_at_move;
    init_move_generation_optimizer();

    FlagGenMatingMove = TSTFLAG(PieSpExFlags,Neutral);

    /* Don't try to "optimize" by hand. The double-loop is tested as
     * the fastest way to compute (due to compiler-optimizations!) */
    for (i = nr_rows_on_board; i>0; i--, square_a += onerow)
    {
      square sq_departure = square_a;
      int j;
      for (j = nr_files_on_board; j>0; j--, sq_departure += dir_right)
      {
        piece const p = e[sq_departure];
        if (p!=vide && TSTFLAG(spec[sq_departure],side_at_move))
        {
          if (CondFlag[gridchess] && !GridLegal(sq_departure,OpponentsKing))
          {
            if (side_at_move==White)
              gen_wh_piece(sq_departure,p);
            else
              gen_bl_piece(sq_departure,p);
          }
          else
          {
            switch(abs(p))
            {
              case King:
                GenMatingKing(goal,sq_departure,OpponentsKing,side_at_move);
                break;

              case Pawn:
                GenMatingPawn(sq_departure,OpponentsKing,side_at_move);
                break;

              case Knight:
                GenMatingKnight(sq_departure,OpponentsKing,side_at_move);
                break;

              case Rook:
                GenMatingRook(sq_departure,OpponentsKing,side_at_move);
                break;

              case Queen:
                GenMatingQueen(sq_departure,OpponentsKing,side_at_move);
                break;

              case Bishop:
                GenMatingBishop(sq_departure,OpponentsKing,side_at_move);
                break;
            }
          }
        }
      }
    }

    finish_move_generation_optimizer();
  }
  else
  {
    if (FlagMoveOrientatedStip)
    {
      if (goal==goal_ep)
      {
        if (ep[nbply]==initsquare && ep2[nbply]==initsquare)
        {
          nextply();
          return;
        }
      }
      else if (goal==goal_castling)
      {
        if (side_at_move==White
            ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)<=ke1_cancastle
            : TSTFLAGMASK(castling_flag[nbply],bl_castlings)<=ke8_cancastle)
        {
          nextply();
          return;
        }
      }

      FlagGenMatingMove = !(side_at_move==White
                            ? flagwhitemummer
                            : flagblackmummer);
    }

    genmove(side_at_move);
  }

  FlagGenMatingMove= False;
} /* generate_move_reaching_goal */
