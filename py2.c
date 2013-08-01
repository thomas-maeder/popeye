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
#include "solving/observation.h"
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
  return true;
}

boolean rcsech(numvec  k,
               numvec  k1,
               PieNam p,
               evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* ATTENTION: There is a parameter dependency between the
   *          indexes of k and of k1 !
   *              p       index of k (ik) I index of k1
   *          ----------------------------------------I------------
   *          Spiralspringer       9 to 16    I 25 - ik
   *                              I
   *          DiagonalSpiralspringer   9 to 14    I 23 - ik
   *          DiagonalSpiralspringer  15 to 16    I 27 - ik
   */

  square sq_departure = sq_target+k;
  square sq_arrival = sq_target;

  while (is_square_empty(sq_departure))
  {
    sq_departure += k1;
    if (!is_square_empty(sq_departure))
      break;
    else
      sq_departure += k;
  }

  if (get_walk_of_piece_on_square(sq_departure)==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && INVOKE_EVAL(evaluate,sq_departure,sq_arrival))
    return true;

  sq_departure = sq_target+k;
  while (is_square_empty(sq_departure)) {
    sq_departure-= k1;
    if (!is_square_empty(sq_departure))
      break;
    else
      sq_departure+= k;
  }

  if (get_walk_of_piece_on_square(sq_departure)==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && INVOKE_EVAL(evaluate,sq_departure,sq_arrival))
    return true;

  return false;
}

boolean rcspech(numvec  k,
                numvec  k1,
                PieNam p,
                evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square sq_departure = sq_target+vec[k];
  square sq_arrival = sq_target;

  while (is_square_empty(sq_departure)) {
    sq_departure += vec[k1];
    if (!is_square_empty(sq_departure))
      break;
    else
      sq_departure+= vec[k];
  }

  if (get_walk_of_piece_on_square(sq_departure)==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && INVOKE_EVAL(evaluate,sq_departure,sq_arrival))
    return true;

  return false;
}

boolean nevercheck(PieNam p, evalfunction_t *evaluate)
{
  return false;
}

boolean cscheck(PieNam p, evalfunction_t *evaluate)
{
  vec_index_type const sum = vec_knight_start+vec_knight_end;
  vec_index_type k;

  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (rcsech(vec[k], vec[sum-k], p, evaluate))
      return true;

  return false;
}

boolean bscoutcheck(PieNam p, evalfunction_t *evaluate)
{
  vec_index_type const sum = vec_bishop_start+vec_bishop_end;
  vec_index_type k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    if (rcsech(vec[k], vec[sum-k], p, evaluate))
      return true;

  return false;
}

boolean gscoutcheck(PieNam p, evalfunction_t *evaluate)
{
  vec_index_type const sum = vec_rook_start+vec_rook_end;
  vec_index_type k;

  for (k= vec_rook_end; k >= vec_rook_start; k--)
    if (rcsech(vec[k], vec[sum-k], p, evaluate))
      return true;

  return false;
}

boolean sp40check(PieNam p, evalfunction_t *evaluate)
{
  return rcspech(9, 16, p, evaluate) ||
         rcspech(10, 11, p, evaluate) ||
         rcspech(11, 10, p, evaluate) ||
         rcspech(12, 13, p, evaluate) ||
         rcspech(13, 12, p, evaluate) ||
         rcspech(14, 15, p, evaluate) ||
         rcspech(15, 14, p, evaluate) ||
         rcspech(16, 9, p, evaluate);
}

boolean sp20check(PieNam p, evalfunction_t *evaluate)
{
  return rcspech(9, 12, p, evaluate) ||
         rcspech(10, 15, p, evaluate) ||
         rcspech(11, 14, p, evaluate) ||
         rcspech(12, 9, p, evaluate) ||
         rcspech(13, 16, p, evaluate) ||
         rcspech(14, 11, p, evaluate) ||
         rcspech(15, 10, p, evaluate) ||
         rcspech(16, 13, p, evaluate);
}

boolean sp33check(PieNam p, evalfunction_t *evaluate)
{
  return rcspech(9, 10, p, evaluate) ||
         rcspech(10, 9, p, evaluate) ||
         rcspech(11, 12, p, evaluate) ||
         rcspech(12, 11, p, evaluate) ||
         rcspech(13, 14, p, evaluate) ||
         rcspech(14, 13, p, evaluate) ||
         rcspech(15, 16, p, evaluate) ||
         rcspech(16, 15, p, evaluate);
}

boolean sp11check(PieNam p, evalfunction_t *evaluate)
{
  return rcspech(9, 14, p, evaluate) ||
         rcspech(14, 9, p, evaluate) ||
         rcspech(10, 13, p, evaluate) ||
         rcspech(13, 10, p, evaluate) ||
         rcspech(11, 16, p, evaluate) ||
         rcspech(16, 11, p, evaluate) ||
         rcspech(12, 15, p, evaluate) ||
         rcspech(15, 12, p, evaluate);
}


boolean sp31check(PieNam p, evalfunction_t *evaluate)
{
  return rcspech(9, 11, p, evaluate) ||
         rcspech(11, 9, p, evaluate) ||
         rcspech(11, 13, p, evaluate) ||
         rcspech(13, 11, p, evaluate) ||
         rcspech(13, 15, p, evaluate) ||
         rcspech(15, 13, p, evaluate) ||
         rcspech(15, 9, p, evaluate) ||
         rcspech(9, 15, p, evaluate) ||
         rcspech(10, 12, p, evaluate) ||
         rcspech(12, 10, p, evaluate) ||
         rcspech(12, 14, p, evaluate) ||
         rcspech(14, 12, p, evaluate) ||
         rcspech(14, 16, p, evaluate) ||
         rcspech(16, 14, p, evaluate) ||
         rcspech(16, 10, p, evaluate) ||
         rcspech(10, 16, p, evaluate);
}


boolean rrefcech(square i1,
                 int   x,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  /* ATTENTION:   first call of rrefech: x must be 2 !!   */

  square sq_departure;

  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (x) {
      sq_departure= i1+vec[k];
      if (is_square_empty(sq_departure)) {
        if (!NoEdge(sq_departure)) {
          if (rrefcech(sq_departure,x-1,p,evaluate))
            return true;
        }
      }
      else if (get_walk_of_piece_on_square(sq_departure)==p
               && TSTFLAG(spec[sq_departure],trait[nbply])
               && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
    else
      for (k= vec_knight_start; k <= vec_knight_end; k++) {
        sq_departure= i1+vec[k];
        if (get_walk_of_piece_on_square(sq_departure)==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && INVOKE_EVAL(evaluate,sq_departure,sq_target))
          return true;
      }

  return false;
}

static boolean rrefnech(square i1,
                        PieNam p,
                        evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  if (!NoEdge(i1))
    settraversed(i1);

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    square sq_departure = i1+vec[k];

    while (is_square_empty(sq_departure))
    {
      if (!NoEdge(sq_departure) &&
          !traversed(sq_departure)) {
        if (rrefnech(sq_departure,p,evaluate))
          return true;
        break;
      }
      sq_departure += vec[k];
    }

    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }
  return false;
}

boolean nequicheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* check by non-stop equihopper? */
  numvec delta_horiz, delta_vert;
  numvec vector;
  square sq_hurdle;
  square sq_departure;

  square const coin= coinequis(sq_target);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {
      sq_hurdle= coin+delta_horiz+delta_vert;
      vector= sq_target-sq_hurdle;
      sq_departure= sq_hurdle-vector;

      if (!is_square_empty(sq_hurdle)
          && get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_target!=sq_departure
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }

  return false;
}

boolean norixcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* check by non-stop orix? */
  numvec delta_horiz, delta_vert;

  square const coin = coinequis(sq_target);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down)
    {
      square const sq_hurdle= coin+delta_horiz+delta_vert;
      numvec const vector = sq_target-sq_hurdle;
      numvec const delta = abs(vector);
      boolean const queenlike = (delta <= 3*dir_right)
                                 || (delta % onerow == 0)
                                 || (delta % (onerow + dir_right) == 0)
                                 || (delta % (onerow + dir_left) == 0);
      square const sq_departure = sq_hurdle-vector;

      if (queenlike
          && !is_square_empty(sq_hurdle)
          && get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_target!=sq_departure
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }

  return false;
}

boolean equifracheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* check by non-stop equistopper? */
  square sq_hurdle;
  square const *bnp;
  numvec vector;
  square sq_departure;

  for (bnp= boardnum; *bnp; bnp++)
  {
    sq_departure= *bnp;
    vector= sq_target-sq_departure;
    sq_hurdle= sq_target+vector;
    if (!is_square_empty(sq_hurdle)
        && !is_square_blocked(sq_hurdle)
        && get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && sq_target!=sq_departure
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean vizircheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_rook_start, vec_rook_end, p, evaluate);
}

boolean dabcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_dabbaba_start, vec_dabbaba_end, p, evaluate);
}

boolean ferscheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_bishop_start, vec_bishop_end, p, evaluate);
}


boolean alfilcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_alfil_start, vec_alfil_end, p, evaluate);
}

boolean rccinqcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_rccinq_start, vec_rccinq_end, p, evaluate);
}


boolean bucheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_bucephale_start, vec_bucephale_end, p, evaluate);
}


boolean gicheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_girafe_start, vec_girafe_end, p, evaluate);
}

boolean chcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_chameau_start, vec_chameau_end, p, evaluate);
}


boolean zcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean leap16check(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_leap16_start, vec_leap16_end, p, evaluate);
}

boolean leap24check(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_leap24_start, vec_leap24_end, p, evaluate);
}

boolean leap35check(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_leap35_start, vec_leap35_end, p, evaluate);
}

boolean leap37check(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_leap37_start, vec_leap37_end, p, evaluate);
}

boolean okapicheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_okapi_start, vec_okapi_end, p, evaluate);   /* knight+zebra */
}

boolean bisoncheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_bison_start, vec_bison_end, p, evaluate);    /* camel+zebra */
}

boolean zebucheck(PieNam p, evalfunction_t *evaluate)
{
  return (leapcheck(vec_chameau_start,vec_chameau_end,p,evaluate)
          || leapcheck(vec_girafe_start,vec_girafe_end,p,evaluate));
}

boolean elephantcheck(PieNam p, evalfunction_t *evaluate)
{
    return ridcheck(vec_elephant_start, vec_elephant_end, p, evaluate);    /* queen+nightrider  */
}

boolean ncheck(PieNam p, evalfunction_t *evaluate)
{
    return ridcheck(vec_knight_start, vec_knight_end, p, evaluate);
}

static boolean rider_hoppers_check(vec_index_type kanf, vec_index_type kend,
                                   PieNam p,
                                   evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation_vector_index[observation_context] = kanf;
       interceptable_observation_vector_index[observation_context]<=kend;
       interceptable_observation_vector_index[observation_context]++)
  {
    square const sq_hurdle = sq_target+vec[interceptable_observation_vector_index[observation_context]];

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation_vector_index[observation_context]]);

      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

static boolean lions_check(vec_index_type kanf, vec_index_type kend,
                                  PieNam p,
                                  evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  for (k = kanf; k <= kend; k++)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[k]);

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[k]);

      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

static boolean leaper_hoppers_check(vec_index_type kanf, vec_index_type kend,
                                    PieNam p,
                                    evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation_vector_index[observation_context] = kanf;
       interceptable_observation_vector_index[observation_context] <= kend;
       interceptable_observation_vector_index[observation_context]++)
  {
    square const sq_hurdle = sq_target+vec[interceptable_observation_vector_index[observation_context]];

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = sq_hurdle+vec[interceptable_observation_vector_index[observation_context]];

      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean scheck(PieNam p, evalfunction_t *evaluate)
{
    return rider_hoppers_check(vec_queen_start, vec_queen_end, p, evaluate);
}

static square grasshoppers_n_find_hurdle(square sq_target,
                                         numvec dir,
                                         unsigned int dist_hurdle_target)
{
  square result = sq_target;
  unsigned int dist_remaining = dist_hurdle_target;
  while (--dist_remaining>0)
  {
    result += dir;
    if (!is_square_empty(result))
      return initsquare;
  }

  result += dir;

  return result;
}

static boolean grasshoppers_n_check(vec_index_type kanf, vec_index_type kend,
                                    PieNam p,
                                    unsigned int dist_hurdle_target,
                                    evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation_vector_index[observation_context] = kanf;
       interceptable_observation_vector_index[observation_context]<=kend;
       interceptable_observation_vector_index[observation_context]++)
  {
    square const sq_hurdle = grasshoppers_n_find_hurdle(sq_target,vec[interceptable_observation_vector_index[observation_context]],dist_hurdle_target);

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation_vector_index[observation_context]]);

      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean grasshop2check(PieNam p, evalfunction_t *evaluate)
{
    return grasshoppers_n_check(vec_queen_start, vec_queen_end, p, 2, evaluate);
}

boolean grasshop3check(PieNam p, evalfunction_t *evaluate)
{
    return grasshoppers_n_check(vec_queen_start, vec_queen_end, p, 3, evaluate);
}

boolean kinghopcheck(PieNam p, evalfunction_t *evaluate)
{
    return leaper_hoppers_check(vec_queen_start, vec_queen_end, p, evaluate);
}

boolean knighthoppercheck(PieNam p, evalfunction_t *evaluate)
{
    return leaper_hoppers_check(vec_knight_start, vec_knight_end, p, evaluate);
}

static boolean doublehoppercheck(PieNam p,
                                 vec_index_type vec_start, vec_index_type vec_end,
                                 evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  for (k=vec_end; k>=vec_start; k--)
  {
    square sq_hurdle2 = sq_target+vec[k];
    if (!is_square_empty(sq_hurdle2) && !is_square_blocked(sq_hurdle2))
    {
      square sq_intermediate;
      for (sq_intermediate = sq_hurdle2+vec[k]; is_square_empty(sq_intermediate); sq_intermediate += vec[k])
      {
        vec_index_type k1;
        for (k1 = vec_end; k1>=vec_start; k1--)
        {
          square const sq_hurdle1 = sq_intermediate+vec[k1];
          if (!is_square_empty(sq_hurdle1) && !is_square_blocked(sq_hurdle1))
          {
            square const sq_departure = find_end_of_line(sq_hurdle1,vec[k1]);
            PieNam const double_hopper = get_walk_of_piece_on_square(sq_departure);
            if (double_hopper==p
                && TSTFLAG(spec[sq_departure],trait[nbply])
                && INVOKE_EVAL(evaluate,sq_departure,sq_target))
              return true;
          }
        }
      }
    }
  }

  return false;
}

boolean doublegrasshoppercheck(PieNam p, evalfunction_t *evaluate)
{
  /* W.B.Trumper feenschach 1968 - but null moves will not be allowed by Popeye
   */
  return doublehoppercheck(p,vec_queen_start,vec_queen_end,evaluate);
}

boolean doublerookhoppercheck(PieNam p, evalfunction_t *evaluate)
{
  return doublehoppercheck(p,vec_rook_start,vec_rook_end,evaluate);
}

boolean doublebishoppercheck(PieNam p, evalfunction_t *evaluate)
{
  return doublehoppercheck(p,vec_bishop_start,vec_bishop_end,evaluate);
}

boolean contragrascheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation_vector_index[observation_context] = vec_queen_start;
       interceptable_observation_vector_index[observation_context]<=vec_queen_end;
       interceptable_observation_vector_index[observation_context]++)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation_vector_index[observation_context]]);

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      square const sq_departure = sq_hurdle+vec[interceptable_observation_vector_index[observation_context]];

      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean nightlocustcheck(PieNam p, evalfunction_t *evaluate)
{
    return marine_rider_check(vec_knight_start, vec_knight_end, p, evaluate);
}

boolean loccheck(PieNam p, evalfunction_t *evaluate)
{
    return marine_rider_check(vec_queen_start, vec_queen_end, p, evaluate);
}

boolean tritoncheck(PieNam p, evalfunction_t *evaluate)
{
    return marine_rider_check(vec_rook_start, vec_rook_end, p, evaluate);
}

boolean nereidecheck(PieNam p, evalfunction_t *evaluate)
{
    return marine_rider_check(vec_bishop_start, vec_bishop_end, p, evaluate);
}

boolean marine_knight_check(PieNam p, evalfunction_t *evaluate)
{
  return marine_leaper_check(vec_knight_start,vec_knight_end,p,evaluate);
}

boolean poseidon_check(PieNam p, evalfunction_t *evaluate)
{
  return marine_leaper_check(vec_queen_start,vec_queen_end,p,evaluate);
}

boolean nightriderlioncheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_knight_start, vec_knight_end, p, evaluate);
}

boolean lioncheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_queen_start,vec_queen_end, p, evaluate);
}

boolean t_lioncheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_rook_start,vec_rook_end, p, evaluate);
}

boolean f_lioncheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_bishop_start,vec_bishop_end, p, evaluate);
}

/* see comment in py4.c on how rose and rose based pieces are
 * handled */
boolean detect_rosecheck_on_line(PieNam p,
                                 vec_index_type k, vec_index_type k1,
                                 numvec delta_k,
                                 evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square sq_departure= find_end_of_circle_line(sq_target,k,&k1,delta_k);
  return (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure!=sq_target /* pieces don't give check to themselves */
          && INVOKE_EVAL(evaluate,sq_departure,sq_target));
}

boolean rosecheck(PieNam p, evalfunction_t *evaluate)
{
  vec_index_type k;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    if (detect_rosecheck_on_line(p,
                                 k,0,rose_rotation_clockwise,
                                 evaluate))
      return true;
    if (detect_rosecheck_on_line(p,
                                 k,vec_knight_end-vec_knight_start+1,rose_rotation_counterclockwise,
                                 evaluate))
      return true;
  }

  return false;
}

boolean detect_roselioncheck_on_line(PieNam p,
                                     vec_index_type k, vec_index_type k1,
                                     numvec delta_k,
                                     evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square sq_hurdle= find_end_of_circle_line(sq_target,k,&k1,delta_k);
  if (sq_hurdle!=sq_target && !is_square_blocked(sq_hurdle))
  {
    square sq_departure= find_end_of_circle_line(sq_hurdle,k,&k1,delta_k);

#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    /* cf. issue 1747928 */
    if (sq_departure==sq_target && e[sq_hurdle]==p) {
      /* special case: king and rose lion are the only pieces on the
       * line -> king is hurdle, and what we thought to be the hurdle
       * is in fact the rose lion! */
      if (INVOKE_EVAL(evaluate,sq_hurdle,sq_target,sq_target))
        return true;
    }
#endif

    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && sq_departure!=sq_target /* pieces don't give check to themselves */
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean roselioncheck(PieNam p, evalfunction_t *evaluate)
{
  /* detects check by a rose lion */
  vec_index_type k;
  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (detect_roselioncheck_on_line(p,
                                     k,0,rose_rotation_clockwise,
                                     evaluate)
        || detect_roselioncheck_on_line(p,k,vec_knight_end-vec_knight_start+1,rose_rotation_counterclockwise,
                                        evaluate))
      return true;

  return false;
}

boolean detect_rosehoppercheck_on_line(square sq_hurdle,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square sq_departure= find_end_of_circle_line(sq_hurdle,k,&k1,delta_k);
  return (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure!=sq_target
          && INVOKE_EVAL(evaluate,sq_departure,sq_target));
}

boolean rosehoppercheck(PieNam p, evalfunction_t *evaluate) {
  /* detects check by a rose hopper */
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;
  square sq_hurdle;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_hurdle= sq_target+vec[k];
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      /* k1==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_rosehoppercheck_on_line(sq_hurdle,p,
                                         k,1,rose_rotation_clockwise,
                                         evaluate))
        return true;
      if (detect_rosehoppercheck_on_line(sq_hurdle,p,
                                         k,vec_knight_end-vec_knight_start,rose_rotation_counterclockwise,
                                         evaluate))
        return true;
    }
  }

  return false;
}

boolean detect_roselocustcheck_on_line(square sq_arrival,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square sq_departure= find_end_of_circle_line(sq_target,k,&k1,delta_k);
  return (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure!=sq_target
          && INVOKE_EVAL(evaluate,sq_departure,sq_arrival));
}

boolean roselocustcheck(PieNam p, evalfunction_t *evaluate)
{
  /* detects check by a rose locust */
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;
  square sq_arrival;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_arrival= sq_target-vec[k];
    if (is_square_empty(sq_arrival))
    {
      /* k1==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_roselocustcheck_on_line(sq_arrival,p,
                                         k,1,rose_rotation_clockwise,
                                         evaluate))
        return true;
      if (detect_roselocustcheck_on_line(sq_arrival,p,
                                         k,vec_knight_end-vec_knight_start,rose_rotation_counterclockwise,
                                         evaluate))
        return true;
    }
  }

  return false;
}

static boolean maooacheck_onedir(square sq_pass,
                                 vec_index_type vec_index_angle_departure_pass,
                                 PieNam p,
                                 evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  numvec const vec_departure_pass = angle_vectors[angle_45][vec_index_angle_departure_pass];
  square const sq_departure = sq_pass+vec_departure_pass;

  return (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target));
}

static boolean maooacheck(vec_index_type vec_index_pass_target_begin,
                          vec_index_type vec_index_pass_target_end,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation_vector_index[observation_context] = vec_index_pass_target_end;
       interceptable_observation_vector_index[observation_context]>=vec_index_pass_target_begin;
       --interceptable_observation_vector_index[observation_context])
  {
    numvec const vec_pass_target = vec[interceptable_observation_vector_index[observation_context]];
    square const sq_pass = sq_target+vec_pass_target;

    if (is_square_empty(sq_pass)
        && (maooacheck_onedir(sq_pass,2*interceptable_observation_vector_index[observation_context],p,evaluate)
            || maooacheck_onedir(sq_pass,2*interceptable_observation_vector_index[observation_context]-1,p,evaluate)))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

boolean maocheck(PieNam p, evalfunction_t *evaluate)
{
  return maooacheck(vec_bishop_start,vec_bishop_end,p,evaluate);
}

boolean moacheck(PieNam p, evalfunction_t *evaluate)
{
  return maooacheck(vec_rook_start,vec_rook_end,p,evaluate);
}

boolean paocheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_rook_start,vec_rook_end, p, evaluate);
}

boolean vaocheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean naocheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_knight_start,vec_knight_end, p, evaluate);
}

boolean leocheck(PieNam p, evalfunction_t *evaluate)
{
    return lions_check(vec_queen_start,vec_queen_end, p, evaluate);
}

boolean berolina_pawn_check(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec[sq_target],capturable) || p==Orphan || p>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;

    if (pawn_test_check(sq_target-dir_forward,sq_target,p,evaluate))
      return true;
    if (en_passant_test_check(dir_forward,&pawn_test_check,p,evaluate))
      return true;
  }

  return false;
}

boolean bspawncheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec[sq_target],base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;
    square const sq_departure = find_end_of_line(sq_target,dir_backward);
    PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
    if (p1==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean spawncheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec[sq_target],base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_left);
      PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
      if (p1==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_right);
      PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
      if (p1==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

boolean amazcheck(PieNam p, evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(vec_queen_start,vec_queen_end, p, evaluate);
}

boolean impcheck(PieNam p, evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(vec_rook_start,vec_rook_end, p, evaluate);
}

boolean princcheck(PieNam p, evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean gnoucheck(PieNam p, evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, p, evaluate)
      || leapcheck(vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean antilcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_antilope_start, vec_antilope_end, p, evaluate);
}

boolean ecurcheck(PieNam p, evalfunction_t *evaluate)
{
  return  leapcheck(vec_knight_start,vec_knight_end, p, evaluate)
      || leapcheck(vec_ecureuil_start, vec_ecureuil_end, p, evaluate);
}

boolean warancheck(PieNam p, evalfunction_t *evaluate)
{
  return  ridcheck(vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(vec_rook_start,vec_rook_end, p, evaluate);
}

boolean dragoncheck(PieNam p, evalfunction_t *evaluate)
{
  if (leapcheck(vec_knight_start,vec_knight_end,p,evaluate))
    return true;

  return pawnedpiececheck(p,evaluate);
}

boolean gryphoncheck(PieNam p, evalfunction_t *evaluate)
{
  return ridcheck(vec_bishop_start,vec_bishop_end,p,evaluate);
}

boolean shipcheck(PieNam p, evalfunction_t *evaluate)
{
  if (ridcheck(vec_rook_start,vec_rook_end,p,evaluate))
    return true;

  return pawnedpiececheck(p, evaluate);
}

boolean pawnedpiececheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec[sq_target],capturable))
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_target-dir_forward_right,sq_target,p,evaluate))
      result = true;
    else if (pawn_test_check(sq_target-dir_forward_left,sq_target,p,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_right,&pawn_test_check,p,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_left,&pawn_test_check,p,evaluate))
      result = true;
  }

  return result;
}

boolean kangoucheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = sq_target+vec[k];
    if (!is_square_empty(sq_hurdle1) && !is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_departure = find_end_of_line(sq_hurdle2,vec[k]);
        PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
        if (p1==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && INVOKE_EVAL(evaluate,sq_departure,sq_target))
          return true;
      }
    }
  }

  return false;
}

boolean kanglioncheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(sq_target,vec[k]);
    if (!is_square_empty(sq_hurdle1) && !is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_departure = find_end_of_line(sq_hurdle2,vec[k]);
        PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
        if (p1==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && INVOKE_EVAL(evaluate,sq_departure,sq_target))
          return true;
      }
    }
  }

  return false;
}

boolean bobcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* 4 hurdle lion */
  numvec  k;

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle1 = find_end_of_line(sq_target,vec[k]);
    if (!is_square_empty(sq_hurdle1) && !is_square_blocked(sq_hurdle1))
    {
      square const sq_hurdle2 = find_end_of_line(sq_hurdle1,vec[k]);
      if (!is_square_blocked(sq_hurdle2))
      {
        square const sq_hurdle3 = find_end_of_line(sq_hurdle2,vec[k]);
        if (!is_square_blocked(sq_hurdle3))
        {
          square const sq_hurdle4 = find_end_of_line(sq_hurdle3,vec[k]);
          if (!is_square_blocked(sq_hurdle4))
          {
            square const sq_departure = find_end_of_line(sq_hurdle4,vec[k]);
            PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
            if (p1==p
                && TSTFLAG(spec[sq_departure],trait[nbply])
                && INVOKE_EVAL(evaluate,sq_departure,sq_target))
              return true;
          }
        }
      }
    }
  }

  return false;
}

boolean moosecheck(PieNam p, evalfunction_t *evaluate)
{
  return angle_hoppers_is_square_observed(vec_queen_start,vec_queen_end, angle_45, p, evaluate);
}

boolean eaglecheck(PieNam p, evalfunction_t *evaluate)
{
  return angle_hoppers_is_square_observed(vec_queen_start,vec_queen_end, angle_90, p, evaluate);
}

boolean sparrcheck(PieNam p, evalfunction_t *evaluate)
{
  return angle_hoppers_is_square_observed(vec_queen_start,vec_queen_end, angle_135, p, evaluate);
}

boolean margueritecheck(PieNam p, evalfunction_t *evaluate)
{
  return  sparrcheck(p, evaluate)
      || eaglecheck(p, evaluate)
      || moosecheck(p, evaluate)
      || scheck(p, evaluate);
}

boolean leap36check(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_leap36_start, vec_leap36_end, p, evaluate);
}

boolean rookmoosecheck(PieNam p, evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_bishop_start,vec_bishop_end, angle_45, p, evaluate);
}

boolean rookeaglecheck(PieNam p, evalfunction_t *evaluate)
{
  return angle_hoppers_is_square_observed(vec_rook_start,vec_rook_end, angle_90, p, evaluate);
}

boolean rooksparrcheck(PieNam p, evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_bishop_start,vec_bishop_end, angle_135, p, evaluate);
}

boolean bishopmoosecheck(PieNam p, evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_rook_start,vec_rook_end, angle_45, p, evaluate);
}

boolean bishopeaglecheck(PieNam p, evalfunction_t *evaluate)
{
  return angle_hoppers_is_square_observed(vec_bishop_start,vec_bishop_end, angle_90, p, evaluate);
}

boolean bishopsparrcheck(PieNam p, evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_rook_start,vec_rook_end, angle_135, p, evaluate);
}

boolean archcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(sq_target, vec[k], p, 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean reffoucheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(sq_target, vec[k], p, 4, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean cardcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rcardech(sq_target, vec[k], p, 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean nsautcheck(PieNam p, evalfunction_t *evaluate)
{
    return rider_hoppers_check(vec_knight_start,vec_knight_end, p, evaluate);
}

boolean camridcheck(PieNam p, evalfunction_t *evaluate)
{
    return ridcheck(vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean zebridcheck(PieNam p, evalfunction_t *evaluate)
{
    return ridcheck(vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean gnuridcheck(PieNam p, evalfunction_t *evaluate)
{
  return  ridcheck(vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean camhopcheck(PieNam p, evalfunction_t *evaluate)
{
    return rider_hoppers_check(vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean zebhopcheck(PieNam p, evalfunction_t *evaluate)
{
    return rider_hoppers_check(vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean gnuhopcheck(PieNam p, evalfunction_t *evaluate)
{
  return  rider_hoppers_check(vec_knight_start,vec_knight_end, p, evaluate)
      || rider_hoppers_check(vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean dcscheck(PieNam p, evalfunction_t *evaluate)
{
  vec_index_type  k;

  for (k= vec_knight_start; k <= 14; k++) {
    if (rcsech(vec[k], vec[23 - k], p, evaluate)) {
      return true;
    }
  }
  for (k= 15; k <= vec_knight_end; k++) {
    if (rcsech(vec[k], vec[27 - k], p, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean refccheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  return rrefcech(sq_target, 2, p, evaluate);
}

boolean refncheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  clearedgestraversed();
  return rrefnech(sq_target, p, evaluate);
}

boolean equicheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  if (orixcheck(p,evaluate))
    return true;

  interceptable_observation_vector_index[observation_context] = 0;

  {
    vec_index_type  k;
    for (k = vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)      /* 2,4; 2,6; 4,6; */
    {
      square const sq_hurdle = sq_target+vec[k];
      square const sq_departure = sq_hurdle+vec[k];
      if (!is_square_empty(sq_hurdle)
          && !is_square_blocked(sq_hurdle)
          && get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

boolean equiengcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_target,-vec[k]);
      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure-sq_target==sq_target-sq_hurdle
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)      /* 2,4; 2,6; 4,6; */
  {
    square const sq_departure = sq_target-vec[k];
    square const sq_hurdle = sq_target+vec[k];
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle)
        && get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean catcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  if (leapcheck(vec_knight_start,vec_knight_end,p,evaluate))
    return true;
  else
  {
    vec_index_type  k;
    for (k= vec_dabbaba_start; k<=vec_dabbaba_end; k++)
    {
      square middle_square= sq_target+vec[k];
      while (is_square_empty(middle_square))
      {
        {
          square const sq_departure= middle_square+cat_vectors[k-60];
          if (get_walk_of_piece_on_square(sq_departure)==p
              && TSTFLAG(spec[sq_departure],trait[nbply])
              && INVOKE_EVAL(evaluate,sq_departure,sq_target))
            return true;
        }

        {
          square const sq_departure= middle_square+cat_vectors[k-56];
          if (get_walk_of_piece_on_square(sq_departure)==p
              && TSTFLAG(spec[sq_departure],trait[nbply])
              && INVOKE_EVAL(evaluate,sq_departure,sq_target))
            return true;
        }

        middle_square+= vec[k];
      }
    }

    return false;
  }
}

boolean roicheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_queen_start,vec_queen_end, p, evaluate);
}

boolean cavcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_knight_start,vec_knight_end, p, evaluate);
}

boolean damecheck(PieNam p, evalfunction_t *evaluate)
{
    return ridcheck(vec_queen_start,vec_queen_end, p, evaluate);
}

boolean tourcheck(PieNam p, evalfunction_t *evaluate)
{
    return ridcheck(vec_rook_start,vec_rook_end, p, evaluate);
}

boolean foucheck(PieNam p, evalfunction_t *evaluate)
{
    return ridcheck(vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean pioncheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec[sq_target],capturable) || p==Orphan || p>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_target-dir_forward_right,sq_target,p,evaluate))
      result = true;
    else if (pawn_test_check(sq_target-dir_forward_left,sq_target,p,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_right,&pawn_test_check,p,evaluate))
      result = true;
    else if (en_passant_test_check(dir_forward_left,&pawn_test_check,p,evaluate))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean reversepcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByBlPawnSq : CapturableByWhPawnSq;

  if (TSTFLAG(sq_spec[sq_target],capturable) || p==Orphan || p>=Hunter0)
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;
    numvec const dir_backward_right = dir_backward+dir_right;
    numvec const dir_backward_left = dir_backward+dir_left;

    if (pawn_test_check(sq_target-dir_backward_right,sq_target,p,evaluate))
      return true;
    else if (pawn_test_check(sq_target-dir_backward_left,sq_target,p,evaluate))
      return true;
    else if (en_passant_test_check(dir_backward_right,&pawn_test_check,p,evaluate))
      return true;
    else if (en_passant_test_check(dir_backward_left,&pawn_test_check,p,evaluate))
      return true;
  }

  return false;
}

boolean edgehcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* detect "check" of edgehog p */
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_departure = find_end_of_line(sq_target,vec[k]);
    PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
    if (p1==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && NoEdge(sq_target)!=NoEdge(sq_departure)
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

static boolean maooaridercheck(PieNam p,
                               numvec  fir,
                               numvec  sec,
                               evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square  middle_square;

  square sq_departure= sq_target+sec;

  middle_square = sq_target+fir;
  while (is_square_empty(middle_square) && is_square_empty(sq_departure))
  {
    middle_square+= sec;
    sq_departure+= sec;
  }

  return (is_square_empty(middle_square)
          && get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_target));
}

boolean moaridercheck(PieNam p, evalfunction_t *evaluate)
{
  if (maooaridercheck(p, +dir_up,+2*dir_up+dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(p, +dir_up,+2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_down,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_down,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(p, +dir_right,+dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p, +dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_left,+dir_up+2*dir_left, evaluate)) {
    return true;
  }
  return false;
}

boolean maoridercheck(PieNam p, evalfunction_t *evaluate)
{
  if (maooaridercheck(p,+dir_up+dir_right,+2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_up+dir_right,+dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_down+dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_down+dir_right,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_down+dir_left,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,+dir_down+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,  dir_up+dir_left,+dir_up+2*dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(p,  dir_up+dir_left,+2*dir_up+dir_left, evaluate)) {
    return true;
  }
  return false;
}

static boolean maooariderlioncheck(PieNam p,
                                   numvec  fir,
                                   numvec  sec,
                                   evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square middle_square= sq_target+fir;

  square sq_departure= sq_target+sec;

  while (is_square_empty(middle_square) && is_square_empty(sq_departure))
  {
    middle_square+= sec;
    sq_departure+= sec;
  }
  if (!is_square_empty(middle_square)
      && get_walk_of_piece_on_square(sq_departure)==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && INVOKE_EVAL(evaluate,sq_departure,sq_target))
    return true;

  if (!is_square_blocked(middle_square)
      && !is_square_blocked(sq_departure)
      && (is_square_empty(middle_square) || is_square_empty(sq_departure)))
  {
    middle_square+= sec;
    sq_departure+= sec;
    while (is_square_empty(middle_square) && is_square_empty(sq_departure))
    {
      middle_square+= sec;
      sq_departure+= sec;
    }
    if (is_square_empty(middle_square)
        && get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean maoriderlioncheck(PieNam p, evalfunction_t *evaluate)
{
  if (maooariderlioncheck(p,+dir_up+dir_right,   +2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_up+dir_right,   +dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_down+dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_down+dir_right,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_down+dir_left,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_down+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,  dir_up+dir_left,   +dir_up+2*dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,  dir_up+dir_left,   +2*dir_up+dir_left, evaluate)) {
    return true;
  }
  return false;
}

boolean moariderlioncheck(PieNam p, evalfunction_t *evaluate)
{
  if (maooariderlioncheck(p, +dir_up,+2*dir_up+dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p, +dir_up,+2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_down,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_down,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_right,+dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(p,+dir_left,+dir_up+2*dir_left, evaluate)) {
    return true;
  }
  return false;
}

boolean r_hopcheck(PieNam p, evalfunction_t *evaluate)
{
    return rider_hoppers_check(vec_rook_start,vec_rook_end, p, evaluate);
}

boolean b_hopcheck(PieNam p, evalfunction_t *evaluate)
{
    return rider_hoppers_check(vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean orixcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation_vector_index[observation_context] = vec_queen_end;
       interceptable_observation_vector_index[observation_context]>=vec_queen_start;
       interceptable_observation_vector_index[observation_context]--)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation_vector_index[observation_context]]);
    if (!is_square_blocked(sq_hurdle))
    {
      square const sq_departure = find_end_of_line(sq_hurdle,vec[interceptable_observation_vector_index[observation_context]]);
      if (get_walk_of_piece_on_square(sq_departure)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure-sq_hurdle==sq_hurdle-sq_target
          && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

boolean leap15check(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_leap15_start, vec_leap15_end, p, evaluate);
}

boolean leap25check(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_leap25_start, vec_leap25_end, p, evaluate);
}

boolean gralcheck(PieNam p, evalfunction_t *evaluate)
{
  return leapcheck(vec_alfil_start, vec_alfil_end, p, evaluate)
      || rider_hoppers_check(vec_rook_start,vec_rook_end, p, evaluate);
}


boolean scorpioncheck(PieNam p, evalfunction_t *evaluate)
{
  return  leapcheck(vec_queen_start,vec_queen_end, p, evaluate)
      || rider_hoppers_check(vec_queen_start,vec_queen_end, p, evaluate);
}

boolean dolphincheck(PieNam p, evalfunction_t *evaluate)
{
  return  rider_hoppers_check(vec_queen_start,vec_queen_end, p, evaluate)
      || kangoucheck(p, evaluate);
}

boolean querquisitecheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  for (k = vec_rook_start; k<=vec_rook_end; k++)
  {
    square const sq_departure = find_end_of_line(sq_target,vec[k]);
    PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
    int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if ((file_departure==file_rook_queenside
         || file_departure==file_queen
         || file_departure==file_rook_kingside)
        && p1==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  for (k= vec_bishop_start; k<=vec_bishop_end; k++)
  {
    square sq_departure = find_end_of_line(sq_target,vec[k]);
    PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
    int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if ((file_departure==file_bishop_queenside
         || file_departure==file_queen
         || file_departure==file_bishop_kingside)
        && p1==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    square const sq_departure= sq_target+vec[k];
    int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && (file_departure==file_knight_queenside
            || file_departure==file_knight_kingside)
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  for (k= vec_queen_start; k<=vec_queen_end; k++)
  {
    square const sq_departure= sq_target+vec[k];
    int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && file_departure==file_king
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

static boolean bouncerfamilycheck(vec_index_type kbeg, vec_index_type kend,
                                  PieNam p,
                                  evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;

  for (k= kend; k>=kbeg; k--)
  {
    square const sq_departure = find_end_of_line(sq_target,vec[k]);
    PieNam const p1 = get_walk_of_piece_on_square(sq_departure);
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    if (sq_departure-sq_target==sq_hurdle-sq_departure
        && p1==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean bouncercheck(PieNam p, evalfunction_t *evaluate)
{
  return bouncerfamilycheck(vec_queen_start,vec_queen_end, p, evaluate);
}

boolean rookbouncercheck(PieNam p, evalfunction_t *evaluate)
{
  return bouncerfamilycheck(vec_rook_start,vec_rook_end, p, evaluate);
}

boolean bishopbouncercheck(PieNam p, evalfunction_t *evaluate)
{
  return bouncerfamilycheck(vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean pchincheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  square sq_departure;
  numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

  sq_departure= sq_target+dir_backward;
  if (get_walk_of_piece_on_square(sq_departure)==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && INVOKE_EVAL(evaluate,sq_departure,sq_target))
    return true;

  /* chinese pawns can capture side-ways if standing on the half of
   * the board farther away from their camp's base line (i.e. if
   * black, on the lower half, if white on the upper half) */
  if ((sq_target*2<(square_h8+square_a1)) == (trait[nbply]==Black))
  {
    sq_departure= sq_target+dir_right;
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;

    sq_departure= sq_target+dir_left;
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}


square fromspecificsquare;

boolean eval_fromspecificsquare(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = sq_departure==fromspecificsquare && validate_observation();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean qlinesradialcheck(PieNam p,
                          evalfunction_t *evaluate,
                          int hurdletype,
                          boolean leaf)

{
  return leapleapcheck(vec_rook_start, vec_rook_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_dabbaba_start, vec_dabbaba_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap03_start,vec_leap03_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap04_start,vec_leap04_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap05_start,vec_leap05_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap06_start,vec_leap06_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap07_start,vec_leap07_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_bishop_start,vec_bishop_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_alfil_start,vec_alfil_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap33_start,vec_leap33_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap44_start,vec_leap44_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap55_start,vec_leap55_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap66_start,vec_leap66_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap77_start,vec_leap77_end, hurdletype, leaf, p, evaluate);
}

boolean radialcheck(PieNam p,
                    evalfunction_t *evaluate,
                    int hurdletype,
                    boolean leaf)

{
  return leapleapcheck(vec_rook_start,vec_rook_end,hurdletype, leaf,p,evaluate)
      || leapleapcheck(vec_dabbaba_start,vec_dabbaba_end,hurdletype, leaf,p,evaluate)
      || leapleapcheck(vec_leap03_start,vec_leap03_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap04_start,vec_leap04_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_bucephale_start,vec_bucephale_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap06_start,vec_leap06_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap07_start,vec_leap07_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_bishop_start,vec_bishop_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_knight_start,vec_knight_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_chameau_start,vec_chameau_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_girafe_start,vec_girafe_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap15_start,vec_leap15_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap16_start,vec_leap16_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_rccinq_start,vec_rccinq_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_alfil_start,vec_alfil_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_zebre_start,vec_zebre_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap24_start,vec_leap24_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap25_start,vec_leap25_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap26_start,vec_leap26_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap27_start,vec_leap27_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap33_start,vec_leap33_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap35_start,vec_leap35_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap36_start,vec_leap36_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap37_start,vec_leap37_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap44_start,vec_leap44_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap45_start,vec_leap45_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap46_start,vec_leap46_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap47_start,vec_leap47_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap56_start,vec_leap56_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap57_start,vec_leap57_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap66_start,vec_leap66_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap67_start,vec_leap67_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(vec_leap77_start,vec_leap77_end, hurdletype, leaf, p, evaluate);
}

boolean radialknightcheck(PieNam p, evalfunction_t *evaluate)
{
  return radialcheck(p, evaluate, 0, false);
}

boolean treehoppercheck(PieNam p, evalfunction_t *evaluate)
{
  return qlinesradialcheck(p, evaluate, 1, false);
}

boolean leafhoppercheck(PieNam p, evalfunction_t *evaluate)
{
  return qlinesradialcheck(p, evaluate, 1, true);
}

boolean greatertreehoppercheck(PieNam p, evalfunction_t *evaluate)
{
  return radialcheck(p, evaluate, 1, false);
}

boolean greaterleafhoppercheck(PieNam p, evalfunction_t *evaluate)
{
  return radialcheck(p, evaluate, 1, true);
}
