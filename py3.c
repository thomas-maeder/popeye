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
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/hunters.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/walks/angle/angles.h"
#include "pieces/walks/riders.h"
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
#include "stipulation/temporary_hacks.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "conditions/annan.h"
#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

#include <assert.h>
#include <stdio.h>

boolean rrfouech(square intermediate_square,
                 numvec k,
                 int    x,
                 evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  if (is_square_blocked(intermediate_square+k))
    return false;
  else
  {
    numvec k1;
    square const sq_reflection = find_end_of_line(intermediate_square,k);
    PieNam const p1 = get_walk_of_piece_on_square(sq_reflection);

    if (x && p1==Invalid)
    {
      square const sq_departure = sq_reflection-k;

      k1= 5;
      while (vec[k1]!=k)
        k1++;

      k1 *= 2;
      if (rrfouech(sq_departure,
                   angle_vectors[angle_90][k1],
                   x-1,
                   evaluate))

        return true;

      k1--;
      if (rrfouech(sq_departure,
                   angle_vectors[angle_90][k1],
                   x-1,
                   evaluate))
        return true;
    }
    else if (INVOKE_EVAL(evaluate,sq_reflection,sq_target))
      return true;

    return false;
  }
}

boolean rcardech(square intermediate_square,
                 numvec k,
                 int    x,
                 evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  square sq_departure = find_end_of_line(intermediate_square,k);

  if (x && is_square_blocked(sq_departure))
  {
    numvec k1;
    for (k1 = 1; k1<=4; k1++)
      if (!is_square_blocked(sq_departure+vec[k1]))
        break;

    if (k1<=4)
    {
      sq_departure += vec[k1];
      if (is_square_empty(sq_departure))
      {
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (is_square_blocked(sq_departure+angle_vectors[angle_90][k1]))
          k1--;
        if (rcardech(sq_departure,
                     angle_vectors[angle_90][k1],
                     x-1,
                     evaluate))
          return true;
      }
      else if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }
  }
  else if (INVOKE_EVAL(evaluate,sq_departure,sq_target ))
    return true;

  return false;
}

boolean echecc(Side side_in_check)
{
  return is_in_check(slices[temporary_hack_check_tester].next2,side_in_check);
}

static evalfunction_t *next_evaluate;

static boolean eval_up(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;
  return (sq_arrival-sq_departure>8
          && INVOKE_EVAL(next_evaluate,sq_departure,sq_arrival));
}

static boolean eval_down(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;
  return (sq_arrival-sq_departure<-8
          && INVOKE_EVAL(next_evaluate,sq_departure,sq_arrival));
}

boolean huntercheck(evalfunction_t *evaluate)
{
  /* detect check by a hunter */
  boolean result;
  evalfunction_t * const eval_away = trait[nbply]==Black ? &eval_down : &eval_up;
  evalfunction_t * const eval_home = trait[nbply]==Black ? &eval_up : &eval_down;
  unsigned int const typeofhunter = observing_walk[nbply]-Hunter0;
  HunterType const * const huntertype = huntertypes+typeofhunter;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(typeofhunter<maxnrhuntertypes);
  next_evaluate = evaluate;
  result = ((*checkfunctions[huntertype->home])(eval_home)
            || (*checkfunctions[huntertype->away])(eval_away));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean rhuntcheck(evalfunction_t *evaluate)
{
  /* detect check of a rook/bishop-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (rook), down (bishop) !! */
  return riders_check(4, 4, evaluate) || riders_check(5, 6, evaluate);
}

boolean bhuntcheck(evalfunction_t *evaluate)
{
  /* detect check of a bishop/rook-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (bishop), down (rook) !! */
  return riders_check(2, 2, evaluate)
      || riders_check(7, 8, evaluate);
}

static boolean skycharcheck(square chp,
                            square sq_arrival1,
                            square sq_arrival2,
                            evalfunction_t *evaluate)
{
  if (is_square_empty(sq_arrival1) && INVOKE_EVAL(evaluate,chp,sq_arrival1))
    return  true;

  if (is_square_empty(sq_arrival2) && INVOKE_EVAL(evaluate,chp,sq_arrival2))
    return  true;

  return  false;
}

boolean skyllacheck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  return  skycharcheck(sq_target+dir_right, sq_target+dir_up+dir_left, sq_target+dir_down+dir_left, evaluate)
       || skycharcheck(sq_target+dir_left, sq_target+dir_up+dir_right, sq_target+dir_down+dir_right, evaluate)
       || skycharcheck(sq_target+dir_up, sq_target+dir_down+dir_right, sq_target+dir_down+dir_left, evaluate)
       || skycharcheck(sq_target+dir_down, sq_target+dir_up+dir_left, sq_target+dir_up+dir_right, evaluate);
}

boolean charybdischeck(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  return  skycharcheck(sq_target+dir_up+dir_right, sq_target+dir_left, sq_target - 24, evaluate)
       || skycharcheck(sq_target+dir_down+dir_left, sq_target+dir_right, sq_target + 24, evaluate)
       || skycharcheck(sq_target+dir_up+dir_left, sq_target+dir_right, sq_target - 24, evaluate)
       || skycharcheck(sq_target+dir_down+dir_right, sq_target+dir_left, sq_target + 24, evaluate);
}
