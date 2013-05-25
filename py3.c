/******************** MODIFICATIONS to py3.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: NormalPawn
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/04/27 SE   Bugfix: Anticirce + TransmutingK
 **
 ** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
 **
 ** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
 **
 ** 2007/09/01 SE   Bug fix: Transmuting Kings (reported: V.Crisan?)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
 **
 ** 2008/02/20 SE   Bugfixes: Annan; Neutrals
 **
 **************************** End of List ******************************/

#if defined(macintosh)  /* is always defined on macintosh's  SB */
#   define SEGM1
#   include "platform/unix/mac.h"
#endif

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "conditions/sat.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/magic_square.h"
#include "conditions/immune.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "stipulation/stipulation.h"
#include "solving/en_passant.h"
#include "solving/observation.h"
#include "conditions/einstein/en_passant.h"
#include "conditions/annan.h"
#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

boolean rubiech(square  intermediate_square,
                square  sq_king,
                piece   p,
                echiquier e_ub,
                evalfunction_t *evaluate)
{
  vec_index_type k;

  e_ub[intermediate_square]= obs;
  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    square const sq_departure= intermediate_square+vec[k];
    if (e_ub[sq_departure]==vide)
    {
      if (rubiech(sq_departure,sq_king,p,e_ub,evaluate))
        return true;
    }
    else
    {
      if (e_ub[sq_departure]==p && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  return false;
}

boolean rrfouech(square intermediate_square,
                 square sq_king,
                 numvec k,
                 piece  p,
                 int    x,
                 evalfunction_t *evaluate)
{
  numvec k1;
  piece p1;

  square sq_departure;

  if (e[intermediate_square+k] == obs)
    return false;

  finligne(intermediate_square,k,p1,sq_departure);
  if (p1==p) {
    if (evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs) {
    sq_departure-= k;
    k1= 5;
    while (vec[k1]!=k)
      k1++;

    k1*= 2;
    if (rrfouech(sq_departure,
                 sq_king,
                 mixhopdata[1][k1],
                 p,
                 x-1,
                 evaluate))

      return true;

    k1--;
    if (rrfouech(sq_departure,
                 sq_king,
                 mixhopdata[1][k1],
                 p,
                 x-1,
                 evaluate))
      return true;
  }

  return false;
}

boolean rcardech(square intermediate_square,
                 square sq_king,
                 numvec k,
                 piece  p,
                 int    x,
                 evalfunction_t *evaluate)
{
  numvec k1;
  piece p1;

  square sq_departure;

  finligne(intermediate_square,k,p1,sq_departure);
  if (p1==p) {
    if (evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs) {
    for (k1= 1; k1<=4; k1++)
      if (e[sq_departure+vec[k1]]!=obs)
        break;

    if (k1<=4) {
      sq_departure+= vec[k1];
      if (e[sq_departure]==p) {
        if (evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
      else if (e[sq_departure]==vide) {
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (e[sq_departure+mixhopdata[1][k1]]==obs)
          k1--;
        if (rcardech(sq_departure,
                     sq_king,
                     mixhopdata[1][k1],
                     p,
                     x-1,
                     evaluate))
          return true;
      }
    }
  }

  return false;
}


boolean feebechec(evalfunction_t *evaluate)
{
  PieNam const *pcheck;
  boolean result = false;

  nextply();

  for (pcheck = checkpieces; *pcheck; ++pcheck)
    if (number_of_pieces[Black][*pcheck]>0
        && (*checkfunctions[*pcheck])(king_square[White], -*pcheck, evaluate))
    {
      result = true;
      break;
    }

  finply();

  return result;
}

boolean feenechec(evalfunction_t *evaluate)
{
  PieNam const *pcheck;
  boolean result = false;

  nextply();

  for (pcheck = checkpieces; *pcheck; ++pcheck)
    if (number_of_pieces[White][*pcheck]>0
        && (*checkfunctions[*pcheck])(king_square[Black], *pcheck, evaluate))
    {
      result = true;
      break;
    }

  finply();

  return result;
}

static boolean calc_rnechec(evalfunction_t *evaluate);

DEFINE_COUNTER(is_black_king_square_attacked)

boolean is_black_king_square_attacked(evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(is_black_king_square_attacked);

  if (TSTFLAG(some_pieces_flags,Neutral))
  {
    Side const neutcoul_save = neutral_side;
    initialise_neutrals(White);
    result = calc_rnechec(evaluate);
    initialise_neutrals(neutcoul_save);
  }
  else
    result = calc_rnechec(evaluate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* detect, if black king is checked     */
static boolean calc_rnechec(evalfunction_t *evaluate)
{
  piece p;

  square sq_departure;
  square sq_arrival;

  if (SATCheck)
    return echecc_SAT(Black);

  if (CondFlag[plus])
    return plusechecc(Black,evaluate);
  else if (CondFlag[phantom])
  {
    if (phantom_echecc(Black,evaluate))
      return true;
  }
  else if (anymars)
    return marsechecc(Black,evaluate);

  if (number_of_pieces[White][King]>0)
  {
    if (calc_reflective_king[White])
    {
      boolean transmutation_of_king_of_checking_side_found = false;

      calc_reflective_king[White] = false;

      if (!normaltranspieces[White] && echecc(White))
      {
        PieNam *ptrans;
        transmutation_of_king_of_checking_side_found = true;
        for (ptrans = transmpieces[White]; *ptrans; ptrans++)
        {
          boolean king_checks;

          nextply();
          king_checks = (*checkfunctions[*ptrans])(king_square[Black], roib, evaluate);
          finply();

          if (king_checks)
          {
            calc_reflective_king[White] = true;
            return true;
          }
        }
      }
      else if (normaltranspieces[White])
      {
        PieNam *ptrans;
        for (ptrans= transmpieces[White]; *ptrans; ptrans++)
        {
          piece const ptrans_black = -*ptrans;
          boolean is_king_of_checking_side_transmuted;

          nextply();
          is_king_of_checking_side_transmuted = number_of_pieces[Black][*ptrans]>0 && (*checkfunctions[*ptrans])(king_square[White],ptrans_black,evaluate);
          finply();

          if (is_king_of_checking_side_transmuted)
          {
            boolean does_transmuted_king_deliver_check;

            transmutation_of_king_of_checking_side_found = true;

            nextply();
            does_transmuted_king_deliver_check = (*checkfunctions[*ptrans])(king_square[Black], roib, evaluate);
            finply();

            if (does_transmuted_king_deliver_check)
            {
              calc_reflective_king[White] = true;
              return true;
            }
          }
        }
      }

      calc_reflective_king[White] = true;

      if (!calc_transmuting_king[White]
          || !transmutation_of_king_of_checking_side_found)
      {
        vec_index_type k;
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq_departure= king_square[Black]+vec[k];
          if (e[sq_departure]==roib
              && evaluate(sq_departure,king_square[Black],king_square[Black]))
            if (imcheck(sq_departure,king_square[Black]))
              return true;
        }
      }
    }
    else
    {
      boolean sting_checks;

      nextply();
      sting_checks = CondFlag[sting] && (*checkfunctions[sb])(king_square[Black], roib, evaluate);
      finply();

      if (sting_checks)
        return true;
      else
      {
        vec_index_type k;
        for (k= vec_queen_end; k>=vec_queen_start; k--) {
          sq_departure= king_square[Black]+vec[k];
          if (e[sq_departure]==roib
              && evaluate(sq_departure,king_square[Black],king_square[Black])
              && imcheck(sq_departure,king_square[Black]))
            return true;
        }
      }
    }
  }

  if (number_of_pieces[White][Pawn]>0)
  {
    if (king_square[Black]>=square_a3
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || CondFlag[wormholes])
    {
      sq_departure= king_square[Black]+dir_down+dir_right;
      if (e[sq_departure]==pb
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (imcheck(sq_departure,king_square[Black]))
          return true;

      sq_departure= king_square[Black]+dir_down+dir_left;
      if (e[sq_departure]==pb
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (imcheck(sq_departure,king_square[Black]))
          return true;

      sq_arrival= ep[nbply];
      if (sq_arrival!=initsquare && king_square[Black]==sq_arrival+dir_down) {
        /* ep captures of royal pawns */
        /* ep[nbply] != initsquare --> a pawn has made a
           double/triple step.
           RN_[nbply] != king_square[Black] --> the black king has moved
        */
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb
            && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb
            && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }

      sq_arrival= einstein_ep[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && king_square[Black]==sq_arrival+dir_down) {
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }
    }
  }

  if (number_of_pieces[White][Knight]>0)
  {
    vec_index_type k;
    for (k= vec_knight_start; k<=vec_knight_end; k++)
    {
      sq_departure= king_square[Black]+vec[k];
      if (e[sq_departure]==cb
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (imcheck(sq_departure,king_square[Black]))
          return true;
    }
  }

  if (number_of_pieces[White][Queen]>0 || number_of_pieces[White][Rook]>0)
  {
    vec_index_type k;
    for (k= vec_rook_end; k>=vec_rook_start; k--)
    {
      finligne(king_square[Black],vec[k],p,sq_departure);
      if ((p==tb || p==db)
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (ridimcheck(sq_departure,king_square[Black],vec[k]))
          return true;
    }
  }

  if (number_of_pieces[White][Queen]>0 || number_of_pieces[White][Bishop]>0)
  {
    vec_index_type k;
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(king_square[Black],vec[k],p,sq_departure);
      if ((p==fb || p==db)
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (ridimcheck(sq_departure,king_square[Black],vec[k]))
          return true;
    }
  }

  if (flagfee)
    return feenechec(evaluate);
  else
    return false;
}

boolean(*is_king_square_attacked[nr_sides])(evalfunction_t *evaluate);

static boolean calc_rbechec(evalfunction_t *evaluate);

DEFINE_COUNTER(is_white_king_square_attacked)

boolean is_white_king_square_attacked(evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(is_white_king_square_attacked);

  if (TSTFLAG(some_pieces_flags,Neutral))
  {
    Side const neutcoul_save = neutral_side;
    initialise_neutrals(Black);
    result = calc_rbechec(evaluate);
    initialise_neutrals(neutcoul_save);
  }
  else
    result = calc_rbechec(evaluate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* detect, if white king is checked  */
static boolean calc_rbechec(evalfunction_t *evaluate)
{
  piece p;

  square sq_departure;
  square sq_arrival;

  if (SATCheck)
    return echecc_SAT(White);

  if (CondFlag[plus])
    return plusechecc(White,evaluate);
  else if (CondFlag[phantom])
  {
    if (phantom_echecc(White,evaluate))
      return true;
  }
  else if (anymars)
    return marsechecc(White,evaluate);

  if (number_of_pieces[Black][King]>0)
  {
    if (calc_reflective_king[Black])
    {
      boolean transmutation_of_king_of_checking_side_found = false;

      calc_reflective_king[Black] = false;

      if (!normaltranspieces[Black] && echecc(Black))
      {
        PieNam *ptrans;

        transmutation_of_king_of_checking_side_found = true;

        for (ptrans= transmpieces[Black]; *ptrans; ptrans++)
        {
          boolean king_checks;

          nextply();
          king_checks = (*checkfunctions[*ptrans])(king_square[White], roin, evaluate);
          finply();

          if (king_checks)
          {
            calc_reflective_king[Black] = true;
            return true;
          }
        }
      }
      else if (normaltranspieces[Black])
      {
        PieNam *ptrans;
        for (ptrans = transmpieces[Black]; *ptrans; ptrans++)
        {
          boolean is_king_of_checking_side_transmuted;

          nextply();
          is_king_of_checking_side_transmuted = number_of_pieces[White][*ptrans]>0 && (*checkfunctions[*ptrans])(king_square[Black], *ptrans, evaluate);
          finply();

          if (is_king_of_checking_side_transmuted)
          {
            boolean does_transmuted_king_deliver_check;

            transmutation_of_king_of_checking_side_found = true;

            nextply();
            does_transmuted_king_deliver_check = (*checkfunctions[*ptrans])(king_square[White], roin, evaluate);
            finply();

            if (does_transmuted_king_deliver_check)
            {
              calc_reflective_king[Black] = true;
              return true;
            }
          }
        }
      }

      calc_reflective_king[Black] = true;

      if (!calc_transmuting_king[Black]
          || !transmutation_of_king_of_checking_side_found )
      {
        vec_index_type k;
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq_departure= king_square[White]+vec[k];
          if (e[sq_departure]==roin
              && evaluate(sq_departure,king_square[White],king_square[White]))
            if (imcheck(sq_departure,king_square[White]))
              return true;
        }
      }
    }
    else
    {
      boolean sting_checks;

      nextply();
      sting_checks = CondFlag[sting] && (*checkfunctions[sb])(king_square[White], roin, evaluate);
      finply();

      if (sting_checks)
        return true;
      else
      {
        vec_index_type k;
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq_departure= king_square[White]+vec[k];
          if (e[sq_departure]==roin
              && evaluate(sq_departure,king_square[White],king_square[White]))
            if (imcheck(sq_departure,king_square[White]))
              return true;
        }
      }
    }
  }

  if (number_of_pieces[Black][Pawn]>0) {
    if (king_square[White]<=square_h6
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || CondFlag[wormholes])
    {
      sq_departure= king_square[White]+dir_up+dir_left;
      if (e[sq_departure]==pn
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (imcheck(sq_departure,king_square[White]))
          return true;

      sq_departure= king_square[White]+dir_up+dir_right;
      if (e[sq_departure]==pn
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (imcheck(sq_departure,king_square[White]))
          return true;

      sq_arrival= ep[nbply];
      if (sq_arrival!=initsquare && king_square[White]==sq_arrival+dir_up) {
        /* ep captures of royal pawns.
           ep[nbply] != initsquare
           --> a pawn has made a double/triple step.
           RB_[nbply] != king_square[White]
           --> the white king has moved
        */
        sq_departure= sq_arrival+dir_up+dir_left;
        if (e[sq_departure]==pn
            && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn
            && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }

      sq_arrival= einstein_ep[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && king_square[White]==sq_arrival+dir_up) {
        sq_departure= sq_arrival+dir_up+dir_left;
        if (e[sq_departure]==pn && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }
    }
  }

  if (number_of_pieces[Black][Knight]>0)
  {
    vec_index_type k;
    for (k= vec_knight_start; k<=vec_knight_end; k++)
    {
      sq_departure= king_square[White]+vec[k];
      if (e[sq_departure]==cn
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (imcheck(sq_departure,king_square[White]))
          return true;
    }
  }

  if (number_of_pieces[Black][Queen]>0 || number_of_pieces[Black][Rook]>0)
  {
    vec_index_type k;
    for (k= vec_rook_end; k>=vec_rook_start; k--)
    {
      finligne(king_square[White],vec[k],p,sq_departure);
      if ((p==tn || p==dn)
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (ridimcheck(sq_departure,king_square[White],vec[k]))
          return true;
    }
  }

  if (number_of_pieces[Black][Queen]>0 || number_of_pieces[Black][Bishop]>0)
  {
    vec_index_type k;
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(king_square[White],vec[k],p,sq_departure);
      if ((p==fn || p==dn)
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (ridimcheck(sq_departure,king_square[White],vec[k]))
          return true;
    }
  }

  if (flagfee)
    return feebechec(evaluate);
  else
    return false;
}

boolean losingchess_is_king_square_attacked(evalfunction_t *evaluate)
{
  return false;
}

static boolean echecc_wh_extinction(void)
{
  boolean result = false;

  PieNam p;
  for (p = King; p<PieceCount; ++p)
  {
    square const *bnp;
    if (!exist[p] || number_of_pieces[White][p]!=1)
      continue;

    for (bnp= boardnum; *bnp; ++bnp)
      if (e[*bnp]==(piece)p)
        break;

    king_square[White] = *bnp;
    if (is_king_square_attacked[White](&validate_observation))
    {
      result = true;
      break;
    }
  }

  king_square[White] = initsquare;

  return result;
}

static boolean echecc_bl_extinction(void)
{
  boolean result = false;

  PieNam p;
  for (p = King; p<PieceCount; ++p)
  {
    square const *bnp;

    if (!exist[p] || number_of_pieces[Black][p]!=1)
      continue;

    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-(piece)p)
        break;

    king_square[Black] = *bnp;
    if (is_king_square_attacked[Black](&validate_observation))
    {
      result = true;
      break;
    }
  }

  king_square[Black] = initsquare;

  return result;
}

static boolean echecc_wh_assassin(void)
{
  square const *bnp;

  if (is_king_square_attacked[White](&validate_observation))
    return true;

  for (bnp= boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];

    if (p!=vide
        && p>roib
        && (*circerenai)(p,spec[*bnp],*bnp,initsquare,initsquare,Black)==king_square[White])
    {
      boolean flag;
      square const rb_sic = king_square[White];
      king_square[White] = *bnp;
      CondFlag[circeassassin] = false;
      flag = is_king_square_attacked[White](&validate_observation);
      CondFlag[circeassassin] = true;
      king_square[White] = rb_sic;
      if (flag)
        return true;
    }
  }

  return false;
}

static boolean echecc_bl_assassin(void)
{
  square const *bnp;

  if (is_king_square_attacked[Black](&validate_observation))
    return true;

  for (bnp= boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];
    if (p!=vide
        && p<roin
        && ((*circerenai)(p,spec[*bnp],*bnp,initsquare,initsquare,White)
            ==king_square[Black]))
    {
      boolean flag;
      square rn_sic = king_square[Black];
      king_square[Black] = *bnp;
      CondFlag[circeassassin] = false;
      flag = is_king_square_attacked[Black](&validate_observation);
      CondFlag[circeassassin] = true;
      king_square[Black] = rn_sic;
      if (flag)
        return true;
    }
  }

  return false;
}

static boolean echecc_wh_bicolores(void)
{
  if (is_king_square_attacked[White](&validate_observation))
    return true;
  else
  {
    boolean result;
    square rn_sic = king_square[Black];
    king_square[Black] = king_square[White];
    CondFlag[bicolores] = false;
    result = is_king_square_attacked[Black](&validate_observation);
    CondFlag[bicolores] = true;
    king_square[Black] = rn_sic;
    return result;
  }
}

static boolean echecc_bl_bicolores(void)
{
  if (is_king_square_attacked[Black](&validate_observation))
    return true;
  else
  {
    boolean result;
    square rb_sic = king_square[White];
    king_square[White] = king_square[Black];
    CondFlag[bicolores] = false;
    result = is_king_square_attacked[White](&validate_observation);
    CondFlag[bicolores] = true;
    king_square[White] = rb_sic;
    return result;
  }
}

boolean echecc(Side camp)
{
  boolean result;

  nextply();

  if ((camp==White) != CondFlag[vogt])
  {
    if (CondFlag[extinction])
      result = echecc_wh_extinction();
    else if (king_square[White]==initsquare)
      result = false;
    else if (rex_circe
             && (CondFlag[pwc]
                 || (e[(*circerenai)(e[king_square[White]],
                                     spec[king_square[White]],
                                     king_square[White],
                                     initsquare,
                                     initsquare,
                                     Black)]
                     == vide)))
      result = false;
    else
    {
      if (TSTFLAG(some_pieces_flags,Neutral))
        initialise_neutrals(Black);
      if (CondFlag[circeassassin] && echecc_wh_assassin())
        result = true;
      else if (CondFlag[bicolores])
        result = echecc_wh_bicolores();
      else
        result = CondFlag[antikings]!=is_king_square_attacked[White](&validate_observation);
    }
  }
  else /* camp==Black */
  {
    if (CondFlag[extinction])
      result = echecc_bl_extinction();
    else if (king_square[Black] == initsquare)
      result = false;
    else if (rex_circe
             && (CondFlag[pwc]
                 || (e[(*circerenai)(e[king_square[Black]],
                                     spec[king_square[Black]],
                                     king_square[Black],
                                     initsquare,
                                     initsquare,
                                     White)]
                     == vide)))
      result = false;
    else
    {
      if (TSTFLAG(some_pieces_flags,Neutral))
        initialise_neutrals(White);
      if (CondFlag[circeassassin] && echecc_bl_assassin())
        result = true;
      else if (CondFlag[bicolores])
        result = echecc_bl_bicolores();
      else
        result = CondFlag[antikings]!=is_king_square_attacked[Black](&validate_observation);
    }
  }

  finply();

  return result;
} /* end of echecc */

static evalfunction_t *next_evaluate;

static boolean eval_up(square sq_departure, square sq_arrival, square sq_capture) {
  return sq_arrival-sq_departure>8
      && next_evaluate(sq_departure,sq_arrival,sq_capture);
}

static boolean eval_down(square sq_departure, square sq_arrival, square sq_capture) {
  return sq_arrival-sq_departure<-8
      && next_evaluate(sq_departure,sq_arrival,sq_capture);
}

boolean huntercheck(square i,
                    piece p,
                    evalfunction_t *evaluate)
{
  /* detect check by a hunter */
  boolean result;
  evalfunction_t * const eval_away = p<0 ? &eval_down : &eval_up;
  evalfunction_t * const eval_home = p<0 ? &eval_up : &eval_down;
  unsigned int const typeofhunter = abs(p)-Hunter0;
  HunterType const * const huntertype = huntertypes+typeofhunter;
  assert(typeofhunter<maxnrhuntertypes);
  next_evaluate = evaluate;
  nextply();
  result = ((*checkfunctions[huntertype->home])(i,p,eval_home)
            || (*checkfunctions[huntertype->away])(i,p,eval_away));
  finply();
  return result;
}

boolean rhuntcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  /* detect check of a rook/bishop-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (rook), down (bishop) !! */
  return ridcheck(i, 4, 4, p, evaluate)
      || ridcheck(i, 5, 6, p, evaluate);
}

boolean bhuntcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  /* detect check of a bishop/rook-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (bishop), down (rook) !! */
  return ridcheck(i, 2, 2, p, evaluate)
      || ridcheck(i, 7, 8, p, evaluate);
}

static boolean skycharcheck(piece  p,
                            square sq_king,
                            square chp,
                            square sq_arrival1,
                            square sq_arrival2,
                            evalfunction_t *evaluate)
{
  if (e[chp] == p) {
    if (e[sq_arrival1]==vide
        && evaluate(chp,sq_arrival1,sq_king)) {
      return  true;
    }

    if (e[sq_arrival2]==vide
        && evaluate(chp,sq_arrival2,sq_king)) {
      return  true;
    }
  }

  return  false;
}

boolean skyllacheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_right, i+dir_up+dir_left, i+dir_down+dir_left, evaluate)
      || skycharcheck(p, i, i+dir_left, i+dir_up+dir_right, i+dir_down+dir_right, evaluate)
      || skycharcheck(p, i, i+dir_up, i+dir_down+dir_right, i+dir_down+dir_left, evaluate)
      || skycharcheck(p, i, i+dir_down, i+dir_up+dir_left, i+dir_up+dir_right, evaluate);
}

boolean charybdischeck(square    i,
                       piece p,
                       evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_up+dir_right, i+dir_left, i - 24, evaluate)
      || skycharcheck(p, i, i+dir_down+dir_left, i+dir_right, i + 24, evaluate)
      || skycharcheck(p, i, i+dir_up+dir_left, i+dir_right, i - 24, evaluate)
      || skycharcheck(p, i, i+dir_down+dir_right, i+dir_left, i + 24, evaluate);
}
