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
#include "pieces/pawns/pawn.h"
#include "pieces/hunters.h"
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
                PieNam p,
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
      if (abs(e_ub[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  return false;
}

boolean rrfouech(square intermediate_square,
                 square sq_king,
                 numvec k,
                 PieNam p,
                 int    x,
                 evalfunction_t *evaluate)
{
  numvec k1;
  piece p1;

  square sq_departure;

  if (e[intermediate_square+k] == obs)
    return false;

  finligne(intermediate_square,k,p1,sq_departure);
  if (abs(p1)==p && TSTFLAG(spec[sq_departure],trait[nbply]))
  {
    if (evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs)
  {
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
                 PieNam p,
                 int    x,
                 evalfunction_t *evaluate)
{
  numvec k1;
  piece p1;

  square sq_departure;

  finligne(intermediate_square,k,p1,sq_departure);
  if (abs(p1)==p && TSTFLAG(spec[sq_departure],trait[nbply]))
  {
    if (evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs)
  {
    for (k1= 1; k1<=4; k1++)
      if (e[sq_departure+vec[k1]]!=obs)
        break;

    if (k1<=4) {
      sq_departure+= vec[k1];
      if (abs(e[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply]))
      {
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

boolean(*is_king_square_attacked[nr_sides])(evalfunction_t *evaluate);

static boolean is_king_square_attacked_impl(Side side_in_check,
                                            evalfunction_t *evaluate)
{
  piece p;
  square sq_departure;
  Side const side_checking = advers(side_in_check);

  if (SATCheck)
    return echecc_SAT(side_in_check);

  if (CondFlag[plus])
    return plusechecc(side_in_check,evaluate);
  else if (CondFlag[phantom])
  {
    if (phantom_echecc(side_in_check,evaluate))
      return true;
  }
  else if (anymars)
    return marsechecc(side_in_check,evaluate);

  if (number_of_pieces[side_checking][King]>0)
  {
    if (calc_reflective_king[side_checking])
    {
      boolean transmutation_of_king_of_checking_side_found = false;

      calc_reflective_king[side_checking] = false;

      if (!normaltranspieces[side_checking] && echecc(side_checking))
      {
        PieNam *ptrans;

        transmutation_of_king_of_checking_side_found = true;

        for (ptrans= transmpieces[side_checking]; *ptrans; ptrans++)
          if ((*checkfunctions[*ptrans])(king_square[side_in_check], King, evaluate))
          {
            calc_reflective_king[side_checking] = true;
            return true;
          }
      }
      else if (normaltranspieces[side_checking])
      {
        PieNam *ptrans;
        for (ptrans = transmpieces[side_checking]; *ptrans; ptrans++)
          if (number_of_pieces[side_in_check][*ptrans]>0)
          {
            boolean is_king_transmuted;

            trait[nbply] = advers(trait[nbply]);
            is_king_transmuted = (*checkfunctions[*ptrans])(king_square[side_checking],*ptrans,evaluate);
            trait[nbply] = advers(trait[nbply]);

            if (is_king_transmuted)
            {
              transmutation_of_king_of_checking_side_found = true;

              if ((*checkfunctions[*ptrans])(king_square[side_in_check], King, evaluate))
              {
                calc_reflective_king[side_checking] = true;
                return true;
              }
            }
          }
      }

      calc_reflective_king[side_checking] = true;

      if (!calc_transmuting_king[side_checking]
          || !transmutation_of_king_of_checking_side_found )
      {
        vec_index_type k;
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq_departure= king_square[side_in_check]+vec[k];
          if (abs(e[sq_departure])==King && TSTFLAG(spec[sq_departure],side_checking)
              && evaluate(sq_departure,king_square[side_in_check],king_square[side_in_check]))
            if (imcheck(sq_departure,king_square[side_in_check]))
              return true;
        }
      }
    }
    else
    {
      if (CondFlag[sting]
          && (*checkfunctions[Grasshopper])(king_square[side_in_check], King, evaluate))
        return true;
      else
      {
        vec_index_type k;
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq_departure= king_square[side_in_check]+vec[k];
          if (abs(e[sq_departure])==King && TSTFLAG(spec[sq_departure],side_checking)
              && evaluate(sq_departure,king_square[side_in_check],king_square[side_in_check]))
            if (imcheck(sq_departure,king_square[side_in_check]))
              return true;
        }
      }
    }
  }

  if (number_of_pieces[side_checking][Pawn]>0
      && pioncheck(king_square[side_in_check],Pawn,evaluate))
    return true;

  if (number_of_pieces[side_checking][Knight]>0)
  {
    vec_index_type k;
    for (k= vec_knight_start; k<=vec_knight_end; k++)
    {
      sq_departure= king_square[side_in_check]+vec[k];
      if (abs(e[sq_departure])==Knight && TSTFLAG(spec[sq_departure],side_checking)
          && evaluate(sq_departure,king_square[side_in_check],king_square[side_in_check]))
        if (imcheck(sq_departure,king_square[side_in_check]))
          return true;
    }
  }

  if (number_of_pieces[side_checking][Queen]>0 || number_of_pieces[side_checking][Rook]>0)
  {
    vec_index_type k;
    for (k= vec_rook_end; k>=vec_rook_start; k--)
    {
      finligne(king_square[side_in_check],vec[k],p,sq_departure);
      if ((abs(p)==Rook || abs(p)==Queen) && TSTFLAG(spec[sq_departure],side_checking)
          && evaluate(sq_departure,king_square[side_in_check],king_square[side_in_check]))
        if (ridimcheck(sq_departure,king_square[side_in_check],vec[k]))
          return true;
    }
  }

  if (number_of_pieces[side_checking][Queen]>0 || number_of_pieces[side_checking][Bishop]>0)
  {
    vec_index_type k;
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(king_square[side_in_check],vec[k],p,sq_departure);
      if ((abs(p)==Bishop || abs(p)==Queen) && TSTFLAG(spec[sq_departure],side_checking)
          && evaluate(sq_departure,king_square[side_in_check],king_square[side_in_check]))
        if (ridimcheck(sq_departure,king_square[side_in_check],vec[k]))
          return true;
    }
  }

  if (flagfee)
  {
    PieNam const *pcheck;
    boolean result = false;

    for (pcheck = checkpieces; *pcheck; ++pcheck)
      if (number_of_pieces[side_checking][*pcheck]>0
          && (*checkfunctions[*pcheck])(king_square[side_in_check], *pcheck, evaluate))
      {
        result = true;
        break;
      }

    return result;
  }
  else
    return false;
}

DEFINE_COUNTER(is_white_king_square_attacked)

boolean is_white_king_square_attacked(evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(is_white_king_square_attacked);

  nextply();
  trait[nbply] = Black;

  if (TSTFLAG(some_pieces_flags,Neutral))
  {
    Side const neutcoul_save = neutral_side;
    initialise_neutrals(Black);
    result = is_king_square_attacked_impl(White,evaluate);
    initialise_neutrals(neutcoul_save);
  }
  else
    result = is_king_square_attacked_impl(White,evaluate);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

DEFINE_COUNTER(is_black_king_square_attacked)

boolean is_black_king_square_attacked(evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  INCREMENT_COUNTER(is_black_king_square_attacked);

  nextply();
  trait[nbply] = White;

  if (TSTFLAG(some_pieces_flags,Neutral))
  {
    Side const neutcoul_save = neutral_side;
    initialise_neutrals(White);
    result = is_king_square_attacked_impl(Black,evaluate);
    initialise_neutrals(neutcoul_save);
  }
  else
    result = is_king_square_attacked_impl(Black,evaluate);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

boolean huntercheck(square sq_target,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  /* detect check by a hunter */
  boolean result;
  evalfunction_t * const eval_away = trait[nbply]==Black ? &eval_down : &eval_up;
  evalfunction_t * const eval_home = trait[nbply]==Black ? &eval_up : &eval_down;
  unsigned int const typeofhunter = p-Hunter0;
  HunterType const * const huntertype = huntertypes+typeofhunter;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(p);
  TraceFunctionParamListEnd();

  assert(typeofhunter<maxnrhuntertypes);
  next_evaluate = evaluate;
  result = ((*checkfunctions[huntertype->home])(sq_target,p,eval_home)
            || (*checkfunctions[huntertype->away])(sq_target,p,eval_away));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean rhuntcheck(square    i,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  /* detect check of a rook/bishop-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (rook), down (bishop) !! */
  return ridcheck(i, 4, 4, p, evaluate)
      || ridcheck(i, 5, 6, p, evaluate);
}

boolean bhuntcheck(square    i,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  /* detect check of a bishop/rook-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (bishop), down (rook) !! */
  return ridcheck(i, 2, 2, p, evaluate)
      || ridcheck(i, 7, 8, p, evaluate);
}

static boolean skycharcheck(PieNam p,
                            square sq_king,
                            square chp,
                            square sq_arrival1,
                            square sq_arrival2,
                            evalfunction_t *evaluate)
{
  if (abs(e[chp])==p && TSTFLAG(sq_spec[chp],trait[nbply]))
  {
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
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_right, i+dir_up+dir_left, i+dir_down+dir_left, evaluate)
      || skycharcheck(p, i, i+dir_left, i+dir_up+dir_right, i+dir_down+dir_right, evaluate)
      || skycharcheck(p, i, i+dir_up, i+dir_down+dir_right, i+dir_down+dir_left, evaluate)
      || skycharcheck(p, i, i+dir_down, i+dir_up+dir_left, i+dir_up+dir_right, evaluate);
}

boolean charybdischeck(square    i,
                       PieNam p,
                       evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_up+dir_right, i+dir_left, i - 24, evaluate)
      || skycharcheck(p, i, i+dir_down+dir_left, i+dir_right, i + 24, evaluate)
      || skycharcheck(p, i, i+dir_up+dir_left, i+dir_right, i - 24, evaluate)
      || skycharcheck(p, i, i+dir_down+dir_right, i+dir_left, i + 24, evaluate);
}
