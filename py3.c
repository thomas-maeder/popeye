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
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/vaulting_kings.h"
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
  if (is_square_blocked(intermediate_square+k))
    return false;
  else
  {
    numvec k1;
    square const sq_reflection = find_end_of_line(intermediate_square,k);
    PieNam const p1 = get_walk_of_piece_on_square(sq_reflection);

    if (p1==p && TSTFLAG(spec[sq_reflection],trait[nbply]))
    {
      if (evaluate(sq_reflection,sq_king,sq_king))
        return true;
    }
    else if (x && p1==Invalid)
    {
      square const sq_departure = sq_reflection-k;

      k1= 5;
      while (vec[k1]!=k)
        k1++;

      k1 *= 2;
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
}

boolean rcardech(square intermediate_square,
                 square sq_king,
                 numvec k,
                 PieNam p,
                 int    x,
                 evalfunction_t *evaluate)
{
  square sq_departure = find_end_of_line(intermediate_square,k);
  PieNam const p1 = get_walk_of_piece_on_square(sq_departure);

  if (p1==p && TSTFLAG(spec[sq_departure],trait[nbply]))
  {
    if (evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && is_square_blocked(sq_departure))
  {
    numvec k1;
    for (k1 = 1; k1<=4; k1++)
      if (!is_square_blocked(sq_departure+vec[k1]))
        break;

    if (k1<=4)
    {
      sq_departure += vec[k1];
      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply]))
      {
        if (evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
      else if (is_square_empty(sq_departure))
      {
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (is_square_blocked(sq_departure+mixhopdata[1][k1]))
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

boolean (*is_square_attacked)(Side side_attacking,
                              square sq_target,
                              evalfunction_t *evaluate);

static boolean does_observe_square_impl(square sq_target,
                                        evalfunction_t *evaluate)
{
  Side const side_checking = trait[nbply];
  Side const side_in_check = advers(side_checking);

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
    if (calc_reflective_king[side_checking] && !transmuting_kings_lock_recursion)
    {
      if (CondFlag[side_checking==White ? whvault_king : blvault_king])
      {
        if (vaulting_kings_is_square_attacked_by_king(sq_target,evaluate))
          return true;
      }
      else if (CondFlag[side_checking==White ? whtrans_king : bltrans_king]
               || CondFlag[side_checking==White ? whsupertrans_king : blsupertrans_king])
      {
        if (transmuting_kings_is_square_attacked_by_king(sq_target,evaluate))
          return true;
      }
      else if (CondFlag[side_checking==White ? whrefl_king : blrefl_king])
      {
        if (reflective_kings_is_square_attacked_by_king(sq_target,evaluate))
          return true;
      }
      else
      {
        assert(0);
      }
    }
    else if (CondFlag[sting]
        && (*checkfunctions[Grasshopper])(sq_target, King, evaluate))
      return true;
    else if (roicheck(sq_target,King,evaluate))
      return true;
  }

  if (number_of_pieces[side_checking][Pawn]>0
      && pioncheck(sq_target,Pawn,evaluate))
    return true;

  if (number_of_pieces[side_checking][Knight]>0
      && cavcheck(sq_target,Knight,evaluate))
    return true;

  if (number_of_pieces[side_checking][Queen]>0
      || number_of_pieces[side_checking][Rook]>0)
  {
    vec_index_type k;
    for (k= vec_rook_end; k>=vec_rook_start; k--)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      PieNam const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Rook || p==Queen) && TSTFLAG(spec[sq_departure],side_checking)
          && evaluate(sq_departure,sq_target,sq_target))
        if (ridimcheck(sq_departure,sq_target,vec[k]))
          return true;
    }
  }

  if (number_of_pieces[side_checking][Queen]>0
      || number_of_pieces[side_checking][Bishop]>0)
  {
    vec_index_type k;
    for (k= vec_bishop_start; k<=vec_bishop_end; k++)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      PieNam const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Bishop || p==Queen) && TSTFLAG(spec[sq_departure],side_checking)
          && evaluate(sq_departure,sq_target,sq_target))
        if (ridimcheck(sq_departure,sq_target,vec[k]))
          return true;
    }
  }

  if (flagfee)
  {
    PieNam const *pcheck;
    boolean result = false;

    for (pcheck = checkpieces; *pcheck; ++pcheck)
      if (number_of_pieces[side_checking][*pcheck]>0
          && (*checkfunctions[*pcheck])(sq_target, *pcheck, evaluate))
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
DEFINE_COUNTER(is_black_king_square_attacked)

boolean is_square_observed(Side side_checking,
                           square sq_target,
                           evalfunction_t *evaluate)
{
  boolean result;
  Side const side_in_check = advers(side_checking);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check,"");
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  if (side_in_check==White)
  {
    INCREMENT_COUNTER(is_white_king_square_attacked);
  }
  else
  {
    INCREMENT_COUNTER(is_black_king_square_attacked);
  }

  nextply();
  trait[nbply] = side_checking;

  if (TSTFLAG(some_pieces_flags,Neutral))
  {
    Side const neutcoul_save = neutral_side;
    initialise_neutrals(side_checking);
    result = does_observe_square_impl(sq_target,evaluate);
    initialise_neutrals(neutcoul_save);
  }
  else
    result = does_observe_square_impl(sq_target,evaluate);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean echecc_extinction(Side side_in_check)
{
  Side const side_checking = advers(side_in_check);
  boolean result = false;

  PieNam p;
  for (p = King; p<PieceCount; ++p)
    if (exist[p] && number_of_pieces[side_in_check][p]==1)
    {
      square const *bnp;
      for (bnp  = boardnum; *bnp; ++bnp)
        if (get_walk_of_piece_on_square(*bnp)==p && TSTFLAG(spec[*bnp],side_in_check))
          break;

      if (is_square_attacked(side_checking,*bnp,&validate_observation))
      {
        result = true;
        break;
      }
    }

  return result;
}

static boolean echecc_assassin(Side side_in_check)
{
  Side const side_checking = advers(side_in_check);

  if (is_square_attacked(side_checking,king_square[side_in_check],&validate_observation))
    return true;
  else
  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
    {
      PieNam const p = get_walk_of_piece_on_square(*bnp);

      if (p!=Empty
          && p!=King && TSTFLAG(spec[*bnp],side_in_check)
          && (*circerenai)(p,spec[*bnp],*bnp,initsquare,initsquare,side_checking)==king_square[side_in_check]
          && is_square_attacked(side_checking,*bnp,&validate_observation))
        return true;
    }
  }

  return false;
}

static boolean echecc_bicolores(Side side_in_check)
{
  Side const side_checking = advers(side_in_check);

  return (is_square_attacked(side_checking,king_square[side_in_check],&validate_observation)
          || is_square_attacked(side_in_check,king_square[side_in_check],&validate_observation));
}

boolean echecc(Side side_in_check)
{
  boolean result;
  Side const side_king_attacked = CondFlag[vogt] ? advers(side_in_check) : side_in_check;
  Side const side_attacking_king = advers(side_king_attacked);

  if (CondFlag[extinction])
    result = echecc_extinction(side_king_attacked);
  else if (king_square[side_king_attacked]==initsquare)
    result = false;
  else if (SATCheck)
    return echecc_SAT(side_in_check);
  else if (rex_circe
           && (CondFlag[pwc]
               || (e[(*circerenai)(get_walk_of_piece_on_square(king_square[side_king_attacked]),
                                   spec[king_square[side_king_attacked]],
                                   king_square[side_king_attacked],
                                   initsquare,
                                   initsquare,
                                   side_attacking_king)]
                   == vide)))
    result = false;
  else
  {
    if (TSTFLAG(some_pieces_flags,Neutral))
      initialise_neutrals(side_attacking_king);
    if (CondFlag[circeassassin] && echecc_assassin(side_king_attacked))
      result = true;
    else if (CondFlag[bicolores])
      result = echecc_bicolores(side_king_attacked);
    else
      result = CondFlag[antikings]!=is_square_attacked(side_attacking_king,
                                                       king_square[side_king_attacked],
                                                       &validate_observation);
  }

  return result;
}

static evalfunction_t *next_evaluate;

static boolean eval_up(square sq_departure, square sq_arrival, square sq_capture)
{
  return sq_arrival-sq_departure>8
      && next_evaluate(sq_departure,sq_arrival,sq_capture);
}

static boolean eval_down(square sq_departure, square sq_arrival, square sq_capture)
{
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
  if (get_walk_of_piece_on_square(chp)==p && TSTFLAG(sq_spec[chp],trait[nbply]))
  {
    if (is_square_empty(sq_arrival1)
        && evaluate(chp,sq_arrival1,sq_king)) {
      return  true;
    }

    if (is_square_empty(sq_arrival2)
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
