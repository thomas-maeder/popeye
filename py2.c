/********************* MODIFICATIONS to py2.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/04 NG   Bugfix: wrong rrefcech evaluation
 **
 ** 2006/05/09 SE   New pieces Bouncer, Rookbouncer, Bishopbouncer (invented P.Wong)
 **
 ** 2006/06/28 SE   New condition: Masand (invented P.Petkov)
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: NormalPawn
 **
 ** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
 **
 ** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
 **
 ** 2008/01/02 NG   New condition: Geneva Chess
 **
 ** 2008/01/24 SE   New variant: Gridlines
 **
 ** 2008/02/19 SE   New condition: AntiKoeko
 **
 ** 2008/02/19 SE   New piece: RoseLocust
 **
 ** 2008/02/25 SE   New piece type: Magic
 **                 Adjusted Masand code
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 ** 2009/02/24 SE   New pieces: 2,0-Spiralknight
 **                             4,0-Spiralknight
 **                             1,1-Spiralknight
 **                             3,3-Spiralknight
 **                             Quintessence (invented Joerg Knappen)
 **
 ** 2009/04/25 SE   New condition: Provacateurs
 **                 New piece type: Patrol pieces
 **
 ** 2009/06/27 SE   Extended imitators/hurdlecolorchanging to moose etc.
 **
 **************************** End of List ******************************/

#if defined(macintosh)  /* is always defined on macintosh's  SB */
#  define SEGM1
#  include "platform/unix/mac.h"
#endif

#include "py.h"
#include "stipulation/stipulation.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "conditions/exclusive.h"
#include "conditions/madrasi.h"
#include "conditions/geneva.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/imitator.h"
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "solving/single_move_generator.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/lions.h"
#include "pieces/walks/hoppers.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/temporary_hacks.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/roses.h"
#include "pieces/walks/angle/angles.h"
#include "pieces/walks/angle/hoppers.h"
#include "pieces/walks/bouncy.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

boolean eval_ortho(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  return (get_walk_of_piece_on_square(sq_departure)==observing_walk[nbply]
          && TSTFLAG(spec[sq_departure],trait[nbply]));
}

boolean nevercheck(evalfunction_t *evaluate)
{
  return false;
}

boolean rrefcech(square i1, int x, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  vec_index_type k;

  /* ATTENTION:   first call of rrefech: x must be 2 !!   */

  square sq_departure;

  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (x) {
      sq_departure= i1+vec[k];
      if (is_square_empty(sq_departure)) {
        if (!NoEdge(sq_departure)) {
          if (rrefcech(sq_departure,x-1,evaluate))
            return true;
        }
      }
      else if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
    else
      for (k= vec_knight_start; k <= vec_knight_end; k++) {
        sq_departure= i1+vec[k];
        if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
          return true;
      }

  return false;
}

static boolean rrefnech(square i1, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  vec_index_type k;

  if (!NoEdge(i1))
    settraversed(i1);

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    square sq_departure = i1+vec[k];

    while (is_square_empty(sq_departure))
    {
      if (!NoEdge(sq_departure) &&
          !traversed(sq_departure)) {
        if (rrefnech(sq_departure,evaluate))
          return true;
        break;
      }
      sq_departure += vec[k];
    }

    if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }
  return false;
}

boolean vizircheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_rook_start, vec_rook_end, evaluate);
}

boolean dabcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_dabbaba_start, vec_dabbaba_end, evaluate);
}

boolean ferscheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_bishop_start, vec_bishop_end, evaluate);
}


boolean alfilcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_alfil_start, vec_alfil_end, evaluate);
}

boolean rccinqcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_rccinq_start, vec_rccinq_end, evaluate);
}


boolean bucheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_bucephale_start, vec_bucephale_end, evaluate);
}


boolean gicheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_girafe_start, vec_girafe_end, evaluate);
}

boolean chcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_chameau_start, vec_chameau_end, evaluate);
}


boolean zcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_zebre_start, vec_zebre_end, evaluate);
}

boolean leap16check(evalfunction_t *evaluate)
{
  return leapcheck(vec_leap16_start, vec_leap16_end, evaluate);
}

boolean leap24check(evalfunction_t *evaluate)
{
  return leapcheck(vec_leap24_start, vec_leap24_end, evaluate);
}

boolean leap35check(evalfunction_t *evaluate)
{
  return leapcheck(vec_leap35_start, vec_leap35_end, evaluate);
}

boolean leap37check(evalfunction_t *evaluate)
{
  return leapcheck(vec_leap37_start, vec_leap37_end, evaluate);
}

boolean okapicheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_okapi_start, vec_okapi_end, evaluate);   /* knight+zebra */
}

boolean bisoncheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_bison_start, vec_bison_end, evaluate);    /* camel+zebra */
}

boolean zebucheck(evalfunction_t *evaluate)
{
  return (leapcheck(vec_chameau_start,vec_chameau_end,evaluate)
          || leapcheck(vec_girafe_start,vec_girafe_end,evaluate));
}

boolean elephantcheck(evalfunction_t *evaluate)
{
    return riders_check(vec_elephant_start, vec_elephant_end, evaluate);    /* queen+nightrider  */
}

boolean ncheck(evalfunction_t *evaluate)
{
    return riders_check(vec_knight_start, vec_knight_end, evaluate);
}

static boolean leaper_hoppers_check(vec_index_type kanf, vec_index_type kend,
                                    evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kanf;
       interceptable_observation[observation_context].vector_index1 <= kend;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_hurdle = sq_target+vec[interceptable_observation[observation_context].vector_index1];

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = sq_hurdle+vec[interceptable_observation[observation_context].vector_index1];

      if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean kinghopcheck(evalfunction_t *evaluate)
{
    return leaper_hoppers_check(vec_queen_start, vec_queen_end, evaluate);
}

boolean knighthoppercheck(evalfunction_t *evaluate)
{
    return leaper_hoppers_check(vec_knight_start, vec_knight_end, evaluate);
}

boolean nightlocustcheck(evalfunction_t *evaluate)
{
    return marine_rider_check(vec_knight_start, vec_knight_end, evaluate);
}

boolean loccheck(evalfunction_t *evaluate)
{
    return marine_rider_check(vec_queen_start, vec_queen_end, evaluate);
}

boolean tritoncheck(evalfunction_t *evaluate)
{
    return marine_rider_check(vec_rook_start, vec_rook_end, evaluate);
}

boolean nereidecheck(evalfunction_t *evaluate)
{
    return marine_rider_check(vec_bishop_start, vec_bishop_end, evaluate);
}

boolean marine_knight_check(evalfunction_t *evaluate)
{
  return marine_leaper_check(vec_knight_start,vec_knight_end,evaluate);
}

boolean poseidon_check(evalfunction_t *evaluate)
{
  return marine_leaper_check(vec_queen_start,vec_queen_end,evaluate);
}

boolean paocheck(evalfunction_t *evaluate)
{
    return lions_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean vaocheck(evalfunction_t *evaluate)
{
    return lions_check(vec_bishop_start,vec_bishop_end, evaluate);
}

boolean naocheck(evalfunction_t *evaluate)
{
    return lions_check(vec_knight_start,vec_knight_end, evaluate);
}

boolean leocheck(evalfunction_t *evaluate)
{
    return lions_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean berolina_pawn_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec[sq_target],capturable) || observing_walk[nbply]==Orphan || observing_walk[nbply]>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;

    if (pawn_test_check(sq_target-dir_forward,sq_target,evaluate))
      return true;
    if (en_passant_test_check(dir_forward,&pawn_test_check,evaluate))
      return true;
  }

  return false;
}

boolean bspawncheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec[sq_target],base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;
    square const sq_departure = find_end_of_line(sq_target,dir_backward);
    if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean spawncheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec[sq_target],base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_left);
      if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_right);
      if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

boolean amazcheck(evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean impcheck(evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean princcheck(evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_bishop_start,vec_bishop_end, evaluate);
}

boolean gnoucheck(evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, evaluate)
      || leapcheck(vec_chameau_start, vec_chameau_end, evaluate);
}

boolean antilcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_antilope_start, vec_antilope_end, evaluate);
}

boolean ecurcheck(evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, evaluate)
      || leapcheck(vec_ecureuil_start, vec_ecureuil_end, evaluate);
}

boolean warancheck(evalfunction_t *evaluate)
{
  return  riders_check(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean dragoncheck(evalfunction_t *evaluate)
{
  if (leapcheck(vec_knight_start,vec_knight_end,evaluate))
    return true;

  return pawnedpiececheck(evaluate);
}

boolean gryphoncheck(evalfunction_t *evaluate)
{
  return riders_check(vec_bishop_start,vec_bishop_end,evaluate);
}

boolean shipcheck(evalfunction_t *evaluate)
{
  if (riders_check(vec_rook_start,vec_rook_end,evaluate))
    return true;

  return pawnedpiececheck(evaluate);
}

boolean pawnedpiececheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec[sq_target],capturable))
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_target-dir_forward_right,sq_target,evaluate))
      result = true;
    else if (pawn_test_check(sq_target-dir_forward_left,sq_target,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_right,&pawn_test_check,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_left,&pawn_test_check,evaluate))
      result = true;
  }

  return result;
}

boolean leap36check(evalfunction_t *evaluate)
{
  return leapcheck(vec_leap36_start, vec_leap36_end, evaluate);
}

boolean archcheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(sq_target, vec[k], 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean reffoucheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(sq_target, vec[k], 4, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean cardcheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rcardech(sq_target, vec[k], 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean camel_rider_check(evalfunction_t *evaluate)
{
  return riders_check(vec_chameau_start, vec_chameau_end, evaluate);
}

boolean zebra_rider_check(evalfunction_t *evaluate)
{
  return riders_check(vec_zebre_start, vec_zebre_end, evaluate);
}

boolean gnu_rider_check(evalfunction_t *evaluate)
{
  return  riders_check(vec_knight_start,vec_knight_end, evaluate)
      || riders_check(vec_chameau_start, vec_chameau_end, evaluate);
}

boolean refccheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  return rrefcech(sq_target, 2, evaluate);
}

boolean refncheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  clearedgestraversed();
  return rrefnech(sq_target, evaluate);
}

boolean roicheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_queen_start,vec_queen_end, evaluate);
}

boolean cavcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_knight_start,vec_knight_end, evaluate);
}

boolean damecheck(evalfunction_t *evaluate)
{
    return riders_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean tourcheck(evalfunction_t *evaluate)
{
    return riders_check(vec_rook_start,vec_rook_end, evaluate);
}

boolean foucheck(evalfunction_t *evaluate)
{
    return riders_check(vec_bishop_start,vec_bishop_end, evaluate);
}

boolean pioncheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec[sq_target],capturable) || observing_walk[nbply]==Orphan || observing_walk[nbply]>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_target-dir_forward_right,sq_target,evaluate))
      result = true;
    else if (pawn_test_check(sq_target-dir_forward_left,sq_target,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_right,&pawn_test_check,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_left,&pawn_test_check,evaluate))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean reversepcheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByBlPawnSq : CapturableByWhPawnSq;

  if (TSTFLAG(sq_spec[sq_target],capturable) || observing_walk[nbply]==Orphan || observing_walk[nbply]>=Hunter0)
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;
    numvec const dir_backward_right = dir_backward+dir_right;
    numvec const dir_backward_left = dir_backward+dir_left;

    if (pawn_test_check(sq_target-dir_backward_right,sq_target,evaluate))
      return true;
    else if (pawn_test_check(sq_target-dir_backward_left,sq_target,evaluate))
      return true;
    else if (en_passant_test_check(dir_backward_right,&pawn_test_check,evaluate))
      return true;
    else if (en_passant_test_check(dir_backward_left,&pawn_test_check,evaluate))
      return true;
  }

  return false;
}

boolean leap15check(evalfunction_t *evaluate)
{
  return leapcheck(vec_leap15_start, vec_leap15_end, evaluate);
}

boolean leap25check(evalfunction_t *evaluate)
{
  return leapcheck(vec_leap25_start, vec_leap25_end, evaluate);
}

boolean gralcheck(evalfunction_t *evaluate)
{
  return leapcheck(vec_alfil_start, vec_alfil_end, evaluate)
      || rider_hoppers_check(vec_rook_start,vec_rook_end, evaluate);
}


boolean scorpioncheck(evalfunction_t *evaluate)
{
  return  leapcheck(vec_queen_start,vec_queen_end, evaluate)
      || rider_hoppers_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean dolphincheck(evalfunction_t *evaluate)
{
  return  rider_hoppers_check(vec_queen_start,vec_queen_end, evaluate)
      || kangoucheck(evaluate);
}

boolean pchincheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  square sq_departure;
  numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

  sq_departure= sq_target+dir_backward;
  if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
    return true;

  /* chinese pawns can capture side-ways if standing on the half of
   * the board farther away from their camp's base line (i.e. if
   * black, on the lower half, if white on the upper half) */
  if ((sq_target*2<(square_h8+square_a1)) == (trait[nbply]==Black))
  {
    sq_departure= sq_target+dir_right;
    if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;

    sq_departure= sq_target+dir_left;
    if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}
