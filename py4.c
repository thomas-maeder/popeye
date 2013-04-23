/********************* MODIFICATIONS to py4.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/09 SE   New pieces Bouncer, Rookbouncer, Bishopbouncer (invented P.Wong)
 **
 ** 2006/05/14 SE   New Condition: TakeMake (invented H.Laue)
 **
 ** 2006/05/17 SE   Bug fix: querquisite
 **                 P moves to 1st rank disallowed for Take&Make on request of inventor
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: NormalPawn
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/05/01 SE   Extended Chopper types to eagles, mooses and sparrows
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
 ** 2008/01/01 SE   Bug fix: Isardam + Maximummer (reported V.Crisan)
 **
 ** 2008/02/24 SE   Bug fix: Gridchess
 **
 ** 2008/02/19 SE   New piece: RoseLocust
 **
 ** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)
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
 ** 2012/02/04 NG   New condition: Chess 8/1 (invented: Werner Keym, 5/2011)
 **
 **************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGM2
#   include "platform/unix/mac.h"
#endif

#include "py.h"
#include "stipulation/stipulation.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "solving/castling.h"
#include "solving/en_passant.h"
#include "solving/observation.h"
#include "conditions/einstein/en_passant.h"
#include "conditions/disparate.h"
#include "conditions/eiffel.h"
#include "conditions/madrasi.h"
#include "conditions/sat.h"
#include "conditions/duellists.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/beamten.h"
#include "conditions/patrol.h"
#include "conditions/central.h"
#include "conditions/koeko/koeko.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/plus.h"
#include "conditions/annan.h"
#include "conditions/wormhole.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int len_max(square sq_departure, square sq_arrival, square sq_capture)
{
  switch (sq_capture) {
  case messigny_exchange:
    return 0;

  case kingside_castling:
    return 16;

  case queenside_castling:
    return 25;

  default:  /* "ordinary move" */
    switch (abs(e[sq_departure])) {

    case Mao:    /* special MAO move.*/
      return 6;

    case Moa:    /* special MOA move.*/
      return 6;

    default:
      if (CondFlag[castlingchess] && sq_capture > platzwechsel_rochade) {
        return (move_diff_code[abs(sq_arrival-sq_departure)]) +
          (move_diff_code[abs((sq_capture-maxsquare)-(sq_departure+sq_arrival)/2)]);
      }
      if (CondFlag[castlingchess] && sq_capture == platzwechsel_rochade) {
        return 2 * (move_diff_code[abs(sq_arrival-sq_departure)]);
      }
      else
       return (move_diff_code[abs(sq_arrival-sq_departure)]);
    }
    break;
  }
}

int len_min(square sq_departure, square sq_arrival, square sq_capture) {
  return -len_max(sq_departure,sq_arrival,sq_capture);
}

int len_capt(square sq_departure, square sq_arrival, square sq_capture) {
  return pprise[nbply]!=vide;
}

int len_follow(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_arrival == move_generation_stack[current_move[parent_ply[nbply]]].departure);
}

int len_whduell(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure == duellists[White][parent_ply[nbply]]);
}

int len_blduell(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure == duellists[Black][parent_ply[nbply]]);
}

int len_alphabetic(square sq_departure, square sq_arrival, square sq_capture) {
  return -((sq_departure/onerow) + onerow*(sq_departure%onerow));
}

int len_synchron(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure-sq_arrival
          == (move_generation_stack[current_move[parent_ply[nbply]]].departure
              - move_generation_stack[current_move[parent_ply[nbply]]].arrival));
}

int len_antisynchron(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_arrival-sq_departure
          == (move_generation_stack[current_move[parent_ply[nbply]]].departure
              - move_generation_stack[current_move[parent_ply[nbply]]].arrival));
}

int len_whforcedsquare(square sq_departure,
                       square sq_arrival,
                       square sq_capture) {
  if (we_generate_exact)
  {
    if (TSTFLAG(sq_spec[sq_arrival], WhConsForcedSq))
    {
      there_are_consmoves = true;
      return 1;
    }
    else
      return 0;
  }
  else
    return (TSTFLAG(sq_spec[sq_arrival], WhForcedSq));
}

int len_blforcedsquare(square sq_departure, square sq_arrival, square sq_capture) {
  if (we_generate_exact) {
    if (TSTFLAG(sq_spec[sq_arrival], BlConsForcedSq)) {
      there_are_consmoves = true;
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return (TSTFLAG(sq_spec[sq_arrival], BlForcedSq));
  }
}

int len_schwarzschacher(square sq_departure, square sq_arrival, square sq_capture)
{
   return sq_arrival==nullsquare ? 0 : 1;
}

int len_losingchess(square sq_departure, square sq_arrival, square sq_capture)
{
  return pprise[nbply]!=vide ? 1 : 0;
}

void add_to_move_generation_stack(square sq_departure,
                                  square sq_arrival,
                                  square sq_capture)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  current_move[nbply]++;
  TraceValue("%u\n",current_move[nbply]);
  move_generation_stack[current_move[nbply]].departure= sq_departure;
  move_generation_stack[current_move[nbply]].arrival= sq_arrival;
  move_generation_stack[current_move[nbply]].capture= sq_capture;
  move_generation_stack[current_move[nbply]].current_transmutation = current_trans_gen;
  move_generation_stack[current_move[nbply]].hopper_hurdle = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

DEFINE_COUNTER(empile)

boolean empile(square sq_departure, square sq_arrival, square sq_capture)
{
  INCREMENT_COUNTER(empile);

  if (CondFlag[imitators]
      && ((sq_capture==kingside_castling || sq_capture==queenside_castling)
          ? !castlingimok(sq_departure, sq_arrival)
          : !imok(sq_departure, sq_arrival)))
    return false;

  add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture);

  return true;
}

static void gemaooarider(square sq_departure,
                         numvec tomiddle,
                         numvec todest,
                         Side camp)
{
  square middle= sq_departure+tomiddle;
  square sq_arrival= sq_departure+todest;
  while (e[middle]==vide
         && e[sq_arrival]==vide
         && empile(sq_departure,sq_arrival,sq_arrival)) {
    middle += todest;
    sq_arrival += todest;
  }
  if (e[middle]==vide && piece_belongs_to_opponent(e[sq_arrival],camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

void gemoarider(square i, Side camp) {
  gemaooarider(i,+dir_up+dir_left,+2*dir_up+dir_left, camp);
  gemaooarider(i,+dir_up+dir_left,+dir_up+2*dir_left, camp);
  gemaooarider(i,+dir_down+dir_right,+2*dir_down+dir_right, camp);
  gemaooarider(i,+dir_down+dir_right,+dir_down+2*dir_right, camp);
  gemaooarider(i,+dir_up+dir_right,+dir_up+2*dir_right, camp);
  gemaooarider(i,+dir_up+dir_right,+2*dir_up+dir_right, camp);
  gemaooarider(i,+dir_down+dir_left,+dir_down+2*dir_left, camp);
  gemaooarider(i,+dir_down+dir_left,+2*dir_down+dir_left, camp);
}

void gemaorider(square i, Side camp) {
  gemaooarider(i,+dir_right,+dir_up+2*dir_right, camp);
  gemaooarider(i,+dir_right,+dir_down+2*dir_right, camp);
  gemaooarider(i,+dir_down,+2*dir_down+dir_right, camp);
  gemaooarider(i,+dir_down,+2*dir_down+dir_left, camp);
  gemaooarider(i,+dir_left,+dir_down+2*dir_left, camp);
  gemaooarider(i,+dir_left,+dir_up+2*dir_left, camp);
  gemaooarider(i,+dir_up,+2*dir_up+dir_left, camp);
  gemaooarider(i,+dir_up,+2*dir_up+dir_right, camp);
}

static void gemaooariderlion(square sq_departure,
                             numvec tomiddle,
                             numvec todest,
                             Side camp)
{
  square middle= sq_departure + tomiddle;
  square sq_arrival= sq_departure+todest;

  while (e[middle]==vide && e[sq_arrival]==vide) {
    middle += todest;
    sq_arrival += todest;
  }
  if (e[middle] != obs && e[sq_arrival] != obs) {
    if (e[middle]!=vide
        && (e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp)))
      empile(sq_departure,sq_arrival,sq_arrival);
    if (e[middle]==vide || e[sq_arrival]==vide) {
      middle += todest;
      sq_arrival += todest;
      while (e[middle]==vide && e[sq_arrival]==vide
             && empile(sq_departure,sq_arrival,sq_arrival)) {
        middle += todest;
        sq_arrival += todest;
      }
    }
    if (e[middle]==vide && piece_belongs_to_opponent(e[sq_arrival],camp))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void gemaoriderlion(square i, Side camp)
{
  gemaooariderlion(i,+dir_right,+dir_up+2*dir_right, camp);
  gemaooariderlion(i,+dir_right,+dir_down+2*dir_right, camp);
  gemaooariderlion(i,+dir_down,+2*dir_down+dir_right, camp);
  gemaooariderlion(i,+dir_down,+2*dir_down+dir_left, camp);
  gemaooariderlion(i,+dir_left,+dir_down+2*dir_left, camp);
  gemaooariderlion(i,+dir_left,+dir_up+2*dir_left, camp);
  gemaooariderlion(i,+dir_up,+2*dir_up+dir_left, camp);
  gemaooariderlion(i,+dir_up,+2*dir_up+dir_right, camp);
}

static void gemoariderlion(square i, Side camp)
{
  gemaooariderlion(i,+dir_up+dir_left,+2*dir_up+dir_left, camp);
  gemaooariderlion(i,+dir_up+dir_left,+dir_up+2*dir_left, camp);
  gemaooariderlion(i,+dir_down+dir_right,+2*dir_down+dir_right, camp);
  gemaooariderlion(i,+dir_down+dir_right,+dir_down+2*dir_right, camp);
  gemaooariderlion(i,+dir_up+dir_right,+dir_up+2*dir_right, camp);
  gemaooariderlion(i,+dir_up+dir_right,+2*dir_up+dir_right, camp);
  gemaooariderlion(i,+dir_down+dir_left,+dir_down+2*dir_left, camp);
  gemaooariderlion(i,+dir_down+dir_left,+2*dir_down+dir_left, camp);
}

static square generate_moves_on_line_segment(square sq_departure,
                                             square sq_base,
                                             int k) {
  square arr= sq_base+vec[k];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_base);
  TraceFunctionParamListEnd();

  while (e[arr]==vide && empile(sq_departure,arr,arr))
    arr+= vec[k];

  TraceFunctionExit(__func__);
  TraceSquare(arr);
  TraceFunctionResultEnd();
  return arr;
}

void gebrid(square sq_departure, numvec kbeg, numvec kend) {
  /* generate white rider moves from vec[kbeg] to vec[kend] */
  numvec k;
  square sq_arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k= kbeg; k<= kend; k++) {
    sq_arrival= generate_moves_on_line_segment(sq_departure,sq_departure,k);
    if (e[sq_arrival]<=roin)
      empile(sq_departure,sq_arrival,sq_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void genrid(square sq_departure, numvec kbeg, numvec kend) {
  /* generate black rider moves from vec[kbeg] to vec[kend] */
  numvec k;
  square sq_arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k= kbeg; k<= kend; k++) {
    sq_arrival= generate_moves_on_line_segment(sq_departure,sq_departure,k);
    if (e[sq_arrival]>=roib)
      empile(sq_departure,sq_arrival,sq_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void genbouncer(square sq_departure,
                       numvec kbeg, numvec kend,
                       Side camp)
{
  square sq_arrival;

  numvec  k;
  for (k= kend; k >= kbeg; k--) {
    piece   p1;
    square  bounce_where;
    finligne(sq_departure,vec[k],p1,bounce_where);

    {
      square const bounce_to= 2*sq_departure-bounce_where;

      sq_arrival= sq_departure-vec[k];
      while (sq_arrival!=bounce_to && e[sq_arrival]==vide)
        sq_arrival-= vec[k];

      if (sq_arrival==bounce_to
          && (e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp)))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

void gebleap(square sq_departure, numvec kbeg, numvec kend) {
  /* generate white leaper moves from vec[kbeg] to vec[kend] */
  square sq_arrival;
  numvec  k;

  for (k= kbeg; k<= kend; k++) {
    sq_arrival= sq_departure+vec[k];
    if (e[sq_arrival]<=vide)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void genleap(square sq_departure, numvec kbeg, numvec kend) {
  /* generate black leaper moves from vec[kbeg] to vec[kend] */

  square sq_arrival;

  numvec  k;
  for (k= kbeg; k<= kend; k++) {
    sq_arrival= sq_departure+vec[k];
    if (e[sq_arrival]==vide || e[sq_arrival] >= roib)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void geriderhopper(square   sq_departure,
                   numvec   kbeg, numvec    kend,
                   int run_up,
                   int jump,
                   Side  camp)
{
  /* generate rider-hopper moves from vec[kbeg] to vec[kend] */

  piece   hurdle;
  square  sq_hurdle;

  square sq_arrival;

  numvec  k;
  for (k= kbeg; k <= kend; k++) {
    if (run_up) {
      /* run up of fixed length */
      /* contragrashopper type */
      sq_hurdle= sq_departure;
      if (run_up>1) {
        /* The run up is longer.
           Check if there is an obstacle between the hopper
           and the hurdle
        */
        int ran_up= run_up;
        while (--ran_up) {
          sq_hurdle += vec[k];
          if (e[sq_hurdle]!=vide)
            break;
        }
        if (ran_up) {
          /* there is an obstacle -> next line */
          continue;
        }
      }
      sq_hurdle+= vec[k];
      hurdle= e[sq_hurdle];
      if (hurdle==vide) {
        /* there is no hurdle -> next line */
        continue;
      }
    }
    else
      /* run up of flexible length
       * lion, grashopper type
       */
      finligne(sq_departure,vec[k],hurdle,sq_hurdle);

    if (hurdle!=obs) {
      sq_arrival= sq_hurdle;
      if (jump) {
        /* jump of fixed length */
        /* grashopper or grashopper-2 type */
        if (jump>1) {
          /* The jump up is a longer one.
             Check if there is an obstacle between
             the hurdle and the target square
          */
          int jumped= jump;
          while (--jumped) {
            sq_arrival+= vec[k];
            if (e[sq_arrival]!=vide)
              break;
          }
          if (jumped) {
            /* there is an obstacle -> next line */
            continue;
          }
        }
        sq_arrival+= vec[k];
        if ((piece_belongs_to_opponent(e[sq_arrival],camp) || (e[sq_arrival]==vide))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          if (TSTFLAG(spec[sq_departure],ColourChange))
            move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
        }
      }
      else {
        /* jump of flexible length */
        /* lion, contragrashopper type */
        sq_arrival+= vec[k];
        while (e[sq_arrival]==vide) {
          if (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k]))
          {
            empile(sq_departure,sq_arrival,sq_arrival);
            if (TSTFLAG(spec[sq_departure],ColourChange))
              move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
          }
          sq_arrival+= vec[k];
        }

        if (piece_belongs_to_opponent(e[sq_arrival],camp)
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          if (TSTFLAG(spec[sq_departure],ColourChange))
            move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
        }
      }
    }
  }
}

/* Generation of moves for Rose and derived pieces.
 * The circle lines are generated in a clever way (which leads to
 * simple code):
 *
 * At the positions vec_knight_start to
 * vec_knight_start+(vec_knight_end-vec_knight_start) of the array
 * vec, we have the knight vectors twice in a row, in the order of the
 * knight's wheel. Each rose line is generated by sequentally adding
 * vec[k1], vec[k1+1] ... vec[k1+(vec_knight_end-vec_knight_start)]
 * (or the same vectors in opposite order) to the initial square.
 *
 * In calls to the generator functions, k2 should either be 0 or
 * (vec_knight_end-vec_knight_start)+1, and delta_k +1 or -1,
 * respectively, to cause k2, k2+delta_k, ... to assume values 0 ...
 * (vec_knight_end-vec_knight_start) or
 * (vec_knight_end-vec_knight_start)+1 ... 1.
 */

static square generate_moves_on_circle_segment(square sq_departure,
                                               square sq_base,
                                               int k1,
                                               int *k2,
                                               int delta_k) {
  square sq_arrival= sq_base;
  do {
    sq_arrival+= vec[k1+*k2];
    *k2+= delta_k;
  } while (e[sq_arrival]==vide
           && empile(sq_departure,sq_arrival,sq_arrival));

  return sq_arrival;
}

square fin_circle_line(square sq_departure,
                       numvec k1, numvec *k2, numvec delta_k) {
  square sq_result= sq_departure;
  do {
    sq_result+= vec[k1+*k2];
    *k2+= delta_k;
  } while (e[sq_result]==vide);

  return sq_result;
}

void grose(square sq_departure,
           numvec k1, numvec k2, numvec delta_k,
           Side camp) {
  square sq_end= generate_moves_on_circle_segment(sq_departure,sq_departure,
                                                  k1,&k2,delta_k);
  if (piece_belongs_to_opponent(e[sq_end],camp))
    empile(sq_departure,sq_end,sq_end);
}

static void grao(square sq_departure,
                 numvec k1, numvec k2, numvec delta_k,
                 Side camp)
{
  square sq_hurdle= generate_moves_on_circle_segment(sq_departure,sq_departure,
                                                     k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
    square sq_arrival= fin_circle_line(sq_hurdle,k1,&k2,delta_k);
    if (piece_belongs_to_opponent(e[sq_arrival],camp))

      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void groselion(square sq_departure,
                      numvec k1, numvec k2, numvec delta_k,
                      Side camp)
{
  square sq_end;
  square sq_hurdle= fin_circle_line(sq_departure,k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    /* cf. issue 1747928 */
    /* temporarily remove the moving piece to prevent it from blocking
     * itself */
    piece save_piece = e[sq_departure];
    e[sq_departure] = vide;
    /* could be going for another 8 steps
    let's make sure we don't run out of S vectors */
    if (delta_k > 0)
    {
      if (k1+k2 > vec_knight_end)
        k2-=8;
    }
    else
    {
      if (k1+k2 <= vec_knight_end)
        k2+=8;
    }
#endif
    sq_end= generate_moves_on_circle_segment(sq_departure,sq_hurdle,
                                                    k1,&k2,delta_k);
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    e[sq_departure] = save_piece;
#endif
    if (piece_belongs_to_opponent(e[sq_end],camp))
      empile(sq_departure,sq_end,sq_end);
  }
}

static void grosehopper(square sq_departure,
                        numvec k1, numvec k2, numvec delta_k,
                        Side camp)
{
  square sq_hurdle= fin_circle_line(sq_departure,k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
    square sq_arrival= sq_hurdle+vec[k1+k2];
    if (e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void groselocust(square sq_departure,
                        numvec k1, numvec k2, numvec delta_k,
                        Side camp)
{
  square sq_capture= fin_circle_line(sq_departure,k1,&k2,delta_k);
  if (sq_capture!=sq_departure && e[sq_capture]!=obs && piece_belongs_to_opponent(e[sq_capture], camp)) {
    square sq_arrival= sq_capture+vec[k1+k2];
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

static void ghamst(square sq_departure)
{
  piece hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_arrival);
    if (hurdle!=obs) {
      sq_arrival-= vec[k];
      if (sq_arrival!=sq_departure)
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static void gmhop(square   sq_departure,
                  numvec   kanf,
                  numvec   kend,
                  int m,
                  Side  camp)
{
  piece hurdle;
  square sq_hurdle;
  numvec k;

  /* ATTENTION:
   *    m == 0: moose    45 degree hopper
   *    m == 1: eagle    90 degree hopper
   *    m == 2: sparrow 135 degree hopper
   *
   *    kend == 8, kanf == 1: queen types  (moose, eagle, sparrow)
   *    kend == 8, kanf == 5: bishop types
   *                  (bishopmoose, bishopeagle, bishopsparrow)
   *    kend == 4, kanf == 1: rook types
   *                  (rookmoose, rookeagle, rooksparrow)
   */

  square sq_arrival;

  for (k = kend; k>=kanf; k--)
  {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);

    if (hurdle!=obs)
    {
      numvec const k1 = 2*k;

      sq_arrival= sq_hurdle+mixhopdata[m][k1];
      if ((e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp))
          && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],mixhopdata[m][k1])))
      {
        empile(sq_departure,sq_arrival,sq_arrival);
        if (TSTFLAG(spec[sq_departure],ColourChange))
          move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
      }

      sq_arrival= sq_hurdle+mixhopdata[m][k1-1];
      if ((e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp))
          && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],mixhopdata[m][k1-1])))
      {
        empile(sq_departure,sq_arrival,sq_arrival);
        if (TSTFLAG(spec[sq_departure],ColourChange))
          move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
      }
    }
  }
}

static void generate_locust_capture(square sq_departure, square sq_capture,
                                    int k,
                                    Side camp) {
  square sq_arrival;
  if (piece_belongs_to_opponent(e[sq_capture],camp)) {
    sq_arrival= sq_capture+vec[k];
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

static void glocust(square sq_departure,
                    numvec kbeg,
                    numvec kend,
                    Side    camp)
{
  piece hurdle;
  square sq_capture;

  numvec k;
  for (k= kbeg; k <= kend; k++) {
    finligne(sq_departure,vec[k],hurdle,sq_capture);
    generate_locust_capture(sq_departure,sq_capture,k,camp);
  }
} /* glocust */

static void gmarin(square  sq_departure,
                   numvec  kbeg,
                   numvec  kend,
                   Side camp)
{
  /* generate marin-pieces moves from vec[kbeg] to vec[kend] */
  numvec k;
  square sq_capture;

  for (k= kbeg; k<=kend; k++) {
    sq_capture= generate_moves_on_line_segment(sq_departure,sq_departure,k);
    generate_locust_capture(sq_departure,sq_capture,k,camp);
  }
}

static void gchin(square   sq_departure,
           numvec   kbeg, numvec    kend,
           Side  camp)
{
  /* generate chinese-rider moves from vec[kbeg] to vec[kend] */

  piece   hurdle;
  numvec  k;

  square sq_arrival;

  for (k= kbeg; k<=kend; k++) {
    sq_arrival= generate_moves_on_line_segment(sq_departure,sq_departure,k);

    if (e[sq_arrival]!=obs) {
      finligne(sq_arrival,vec[k],hurdle,sq_arrival);
      if (piece_belongs_to_opponent(hurdle,camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static void gchinleap(square   sq_departure,
           numvec   kbeg, numvec    kend,
           Side  camp)
{
  /* generate chinese-leaper moves from vec[kbeg] to vec[kend] */

  numvec  k;

  square sq_arrival;

  for (k= kbeg; k<=kend; k++) {
    sq_arrival= sq_departure + vec[k];

    if (abs(e[sq_arrival])>=roib) {
      sq_arrival += vec[k];
      if (piece_belongs_to_opponent(e[sq_arrival],camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
    else if (e[sq_arrival] == vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static void gnequi(square sq_departure, Side camp) {
  /* Non-Stop-Equihopper */
  square sq_hurdle;
  square sq_arrival;
  numvec delta_horiz, delta_vert;
  numvec vector;

  square const coin= coinequis(sq_departure);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {

      sq_hurdle= coin+delta_horiz+delta_vert;

      if (e[sq_hurdle]!=vide) {

        vector= sq_hurdle-sq_departure;
        sq_arrival= sq_hurdle+vector;

        if ((e[sq_arrival]==vide
             || piece_belongs_to_opponent(e[sq_arrival],camp))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vector,vector)))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          if (TSTFLAG(spec[sq_departure],ColourChange))
            move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
        }
      }
    }
}

static void gorix(square sq_departure, Side camp) {
  /* Orix */
  numvec  k;
  piece   hurdle, at_end_of_line;
  square  sq_hurdle, end_of_line;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      finligne(sq_hurdle,vec[k],at_end_of_line,end_of_line);
      sq_arrival= sq_hurdle+sq_hurdle-sq_departure;
      if (abs(end_of_line-sq_hurdle) > abs(sq_hurdle-sq_departure)
          && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
      {
        empile(sq_departure,sq_arrival,sq_arrival);
        if (TSTFLAG(spec[sq_departure],ColourChange))
          move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
      }
      else if (abs(end_of_line-sq_hurdle) == abs(sq_hurdle-sq_departure)
               && piece_belongs_to_opponent(at_end_of_line,camp)
               && (!checkhopim || hopimok(sq_departure,end_of_line,sq_hurdle,vec[k],vec[k])))
      {
        sq_arrival= end_of_line;
        empile(sq_departure,sq_arrival,sq_arrival);
        if (TSTFLAG(spec[sq_departure],ColourChange))
          move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
      }
    }
  }
}

static void gnorix(square sq_departure, Side camp) {
  /* Non-Stop-Orix */
  square sq_hurdle;
  square sq_arrival;
  numvec delta_horiz, delta_vert, delta;
  numvec vector;
  boolean queenlike;

  square const coin= coinequis(sq_departure);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {

      sq_hurdle= coin+delta_horiz+delta_vert;
      delta= abs(sq_hurdle - sq_departure);
      queenlike= (delta <= 3*dir_right)
      		|| (delta % onerow == 0)
		|| (delta % (onerow + dir_right) == 0)
		|| (delta % (onerow + dir_left) == 0);

      if (queenlike && e[sq_hurdle]!=vide) {

        vector= sq_hurdle-sq_departure;
        sq_arrival= sq_hurdle+vector;

        if ((e[sq_arrival]==vide
             || piece_belongs_to_opponent(e[sq_arrival],camp))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vector,vector)))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          if (TSTFLAG(spec[sq_departure],ColourChange))
            move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
        }
      }
    }
}

static void gnequiapp(square sq_departure, Side camp) {
  /* Non-Stop-Equistopper */
  square sq_hurdle;
  square sq_arrival;
  numvec delta_horiz, delta_vert;
  numvec vector;

  square const coin= coinequis(sq_departure);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {

      sq_arrival= coin+delta_horiz+delta_vert;
      vector= sq_arrival-sq_departure;
      sq_hurdle= sq_arrival+vector;

      if (sq_arrival!=sq_departure
          && e[sq_hurdle]!=vide
          && (e[sq_arrival]==vide
              || piece_belongs_to_opponent(e[sq_arrival],camp)))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
}

static void gkang(square sq_departure, Side camp) {
  piece hurdle;
  square sq_hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs) {
        sq_arrival+= vec[k];
        if (e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void gkanglion(square sq_departure, Side camp) {
  piece hurdle;
  square sq_hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs) {
        while (e[sq_arrival+= vec[k]] == vide)
          empile(sq_departure,sq_arrival,sq_arrival);
        if (piece_belongs_to_opponent(e[sq_arrival],camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void grabbit(square sq_departure, Side camp) {
  piece hurdle;
  square sq_hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k >=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {

      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs) {
        sq_arrival= generate_moves_on_line_segment(sq_departure,sq_arrival,k);
        if (piece_belongs_to_opponent(e[sq_arrival],camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void gbob(square sq_departure, Side camp) {
  piece hurdle;
  square sq_hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {

      finligne(sq_hurdle,vec[k],hurdle,sq_hurdle);
      if (hurdle!=obs) {

        finligne(sq_hurdle,vec[k],hurdle,sq_hurdle);
        if (hurdle!=obs) {

          finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
          if (hurdle!=obs) {

            sq_arrival= generate_moves_on_line_segment(sq_departure,
                                                       sq_arrival,
                                                       k);
            if (piece_belongs_to_opponent(e[sq_arrival],camp))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
}

static void gcs(square sq_departure,
                numvec k1, numvec k2,
                Side camp)
{
  square sq_arrival= sq_departure+k1;

  while (e[sq_arrival]==vide) {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k2;
    if (e[sq_arrival]==vide) {
      empile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= k1;
    }
    else
      break;
  }
  if (piece_belongs_to_opponent(e[sq_arrival],camp))
    empile(sq_departure,sq_arrival,sq_arrival);

  sq_arrival= sq_departure+k1;
  while (e[sq_arrival]==vide) {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival-= k2;
    if (e[sq_arrival]==vide) {
      empile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= k1;
    }
    else
      break;
  }
  if (piece_belongs_to_opponent(e[sq_arrival],camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

static void gcsp(square sq_departure,
         numvec k1, numvec k2,
         Side camp)
{
  square sq_arrival= sq_departure+vec[k1];

  while (e[sq_arrival]==vide) {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= vec[k2];
    if (e[sq_arrival]==vide) {
      empile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= vec[k1];
    }
    else
      break;
  }
  if (piece_belongs_to_opponent(e[sq_arrival],camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

static void gubi(square orig_departure,
          square step_departure,
          Side camp) {
  numvec k;

  square sq_departure= orig_departure;
  square sq_arrival;

  e_ubi[step_departure]= obs;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_arrival= step_departure+vec[k];
    if (e_ubi[sq_arrival]==vide) {
      empile(sq_departure,sq_arrival,sq_arrival);
      gubi(orig_departure,sq_arrival,camp);
    }
    else if (piece_belongs_to_opponent(e_ubi[sq_arrival],camp))
      empile(sq_departure,sq_arrival,sq_arrival);
    e_ubi[sq_arrival]= obs;
  }
}

static void grfou(square   orig_departure,
                  square   in,
                  numvec   k,
                  int x,
                  Side  camp)
{
  int k1;

  square sq_departure= orig_departure;
  square sq_arrival= in+k;

  if (e[sq_arrival]==obs)
    return;

  while (e[sq_arrival]==vide)
  {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }

  if (piece_belongs_to_opponent(e[sq_arrival],camp))
    empile(sq_departure,sq_arrival,sq_arrival);
  else if (x && e[sq_arrival]==obs)
  {
    sq_arrival-= k;
    k1= 5;
    while (vec[k1]!=k)
      k1++;
    k1*= 2;
    grfou(orig_departure,sq_arrival,mixhopdata[1][k1],x-1,camp);
    k1--;
    grfou(orig_departure,sq_arrival,mixhopdata[1][k1],x-1,camp);
  }
}

static void gcard(square   orig_departure,
                  square   in,
                  numvec   k,
                  int x,
                  Side  camp)
{
  int k1;

  square sq_departure= orig_departure;
  square sq_arrival= in+k;

  while (e[sq_arrival]==vide)
  {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }

  if (piece_belongs_to_opponent(e[sq_arrival],camp))
    empile(sq_departure,sq_arrival,sq_arrival);
  else if (x && e[sq_arrival]==obs)
  {
    for (k1= 1; k1<=4; k1++)
      if (e[sq_arrival+vec[k1]]!=obs)
        break;

    if (k1<=4)
    {
      sq_arrival+= vec[k1];
      if (piece_belongs_to_opponent(e[sq_arrival],camp))
        empile(sq_departure,sq_arrival,sq_arrival);
      else if (e[sq_arrival]==vide)
      {
        empile(sq_departure,sq_arrival,sq_arrival);
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (e[sq_arrival+mixhopdata[1][k1]]==obs)
          k1--;

        gcard(orig_departure,sq_arrival,mixhopdata[1][k1],x-1,camp);
      }
    }
  }
}

static void grefc(square orig_departure,
                  square step_departure,
                  int x,
                  Side camp) {
  numvec k;

  /* ATTENTION:   first call of grefcn: x must be 2 !!   */

  square sq_departure = orig_departure;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    square const sq_arrival = step_departure+vec[k];
    if (e[sq_arrival]==vide)
    {
      empile(sq_departure,sq_arrival,sq_arrival);
      if (x>0 && !NoEdge(sq_arrival))
        grefc(orig_departure,sq_arrival,x-1,camp);
    }
    else if (piece_belongs_to_opponent(e[sq_arrival], camp))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

enum
{
  nr_edge_squares_row = nr_files_on_board,
  nr_edge_squares_file = nr_rows_on_board-2, /* count corners once only */

  nr_edge_squares = 2*nr_edge_squares_row + 2*nr_edge_squares_file
};

typedef square edge_square_index;

/* remember edge traversals by reflecting pieces
 */
static boolean edgestraversed[nr_edge_squares];

/* clear edge traversal memory
 */
void clearedgestraversed()
{
  int i;
  for (i=0; i<nr_edge_squares; i++)
    edgestraversed[i] = false;
}

/* map edge squares to indices into edgestraversed
 * the mapping occurs in the order
 * - bottom row
 * - top row
 * - left file
 * - right file
 * @param edge_square square on board edge
 * @return index into edgestraversed where to remember traversal of
 *         edge_square
 */
static edge_square_index square_2_edge_square_index(square edge_square)
{
  int const row =  edge_square/onerow;
  int const file = edge_square%onerow;
  edge_square_index result;

  assert(row==bottom_row || row==top_row
         || file==left_file || file==right_file);
  if (row==bottom_row)
    result = file-left_file;
  else if (row==top_row)
    result = file-left_file + nr_edge_squares_row;
  else if (file==left_file)
    result = row-bottom_row-1 + 2*nr_edge_squares_row;
  else
    result = row-bottom_row-1 + 2*nr_edge_squares_row + nr_edge_squares_file;

  assert(result<nr_edge_squares);
  return result;
}

/* query traversal of an edge square
 * @param edge_square square on board edge
 * @return has edge_square been traversed?
 */
boolean traversed(square edge_square)
{
  return edgestraversed[square_2_edge_square_index(edge_square)];
}

/* remember traversal of an edge square
 * @param edge_square square on board edge
 */
void settraversed(square edge_square)
{
  edgestraversed[square_2_edge_square_index(edge_square)] = true;
}

static void grefn(square orig_departure,
                  square step_departure,
                  Side camp) {
  numvec k;

  square sq_departure= orig_departure;
  square sq_arrival;

  if (!NoEdge(step_departure))
    settraversed(step_departure);

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    sq_arrival= step_departure;

    while (e[sq_arrival+=vec[k]]==vide)
    {
      empile(sq_departure,sq_arrival,sq_arrival);
      if (!NoEdge(sq_arrival) &&
          !traversed(sq_arrival)) {
        grefn(orig_departure,sq_arrival,camp);
        break;
      }
    }

    if (piece_belongs_to_opponent(e[sq_arrival], camp))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
} /* grefc */

static void gequi(square sq_departure, Side camp) {
  /* Equihopper */
  numvec  k;
  piece   hurdle;
  square  sq_hurdle, end_of_line;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      finligne(sq_hurdle,vec[k],hurdle,end_of_line);
      {
        int const dist_hurdle_end= abs(end_of_line-sq_hurdle);
        int const dist_hurdle_dep= abs(sq_hurdle-sq_departure);
        if (dist_hurdle_end>dist_hurdle_dep) {
          sq_arrival= sq_hurdle+sq_hurdle-sq_departure;
          if (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k]))
          {
            empile(sq_departure,sq_arrival,sq_arrival);
            if (TSTFLAG(spec[sq_departure],ColourChange))
              move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
          }
        }
        else if (dist_hurdle_end==dist_hurdle_dep) {
          sq_arrival= end_of_line;
          if (piece_belongs_to_opponent(hurdle,camp)
              && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
          {
            empile(sq_departure,sq_arrival,sq_arrival);
            if (TSTFLAG(spec[sq_departure],ColourChange))
              move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
          }
        }
      }
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {     /* 2,4; 2,6; 4,6; */
    sq_hurdle= sq_departure+vec[k];
    sq_arrival= sq_departure + 2*vec[k];
    if (abs(e[sq_hurdle])>=roib
        && (e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp))
        && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
    {
      empile(sq_departure,sq_arrival,sq_arrival);
      if (TSTFLAG(spec[sq_departure],ColourChange))
        move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
    }
  }
}

static void gequiapp(square sq_departure, Side camp) {
  /* (interceptable) Equistopper */
  numvec  k;
  piece   hurdle1, hurdle2;
  square  sq_hurdle1, sq_hurdle2;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_departure,vec[k],hurdle1,sq_hurdle1);
    if (hurdle1!=obs) {
      sq_arrival= (sq_hurdle1+sq_departure)/2;
      if (!((sq_hurdle1/onerow+sq_departure/onerow)%2
            || (sq_hurdle1%onerow+sq_departure%onerow)%2)) /* is sq_arrival a square? */
        empile(sq_departure,sq_arrival,sq_arrival);

      finligne(sq_hurdle1,vec[k],hurdle2,sq_hurdle2);
      if (hurdle2!=obs
          && (abs(sq_hurdle2-sq_hurdle1)
              ==abs(sq_hurdle1-sq_departure))
          && piece_belongs_to_opponent(hurdle1,camp)) {
        sq_arrival= sq_hurdle1;
        empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {     /* 2,4; 2,6; 4,6; */
    sq_arrival= sq_departure + vec[k];
    sq_hurdle1= sq_departure+2*vec[k];
    if (abs(e[sq_hurdle1])>=roib
        && (e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp)))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void gcat(square sq_departure, Side camp) {
  /* generate moves of a CAT */
  numvec  k;

  square sq_arrival;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_arrival= sq_departure+vec[k];
    if (piece_belongs_to_opponent(e[sq_arrival], camp))
      empile(sq_departure,sq_arrival,sq_arrival);
    else {
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= mixhopdata[3][k];
      }

      if (piece_belongs_to_opponent(e[sq_arrival], camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static void gmaooa(square  sq_departure,
            square  pass,
            square  arrival1,
            square  arrival2,
            Side camp)
{
  if (e[pass] == vide) {
    if (e[arrival1]==vide || piece_belongs_to_opponent(e[arrival1],camp))
      if (maooaimcheck(sq_departure,arrival1,pass))
        empile(sq_departure,arrival1,arrival1);

    if (e[arrival2]==vide || piece_belongs_to_opponent(e[arrival2],camp))
      if (maooaimcheck(sq_departure,arrival2,pass))
        empile(sq_departure,arrival2,arrival2);
  }
}

static void gmao(square i, Side camp) {
  gmaooa(i, i+dir_up, i+2*dir_up+dir_left, i+2*dir_up+dir_right, camp);
  gmaooa(i, i+dir_down, i+2*dir_down+dir_right, i+2*dir_down+dir_left, camp);
  gmaooa(i, i+dir_right, i+dir_up+2*dir_right, i+dir_down+2*dir_right, camp);
  gmaooa(i, i+dir_left, i+dir_down+2*dir_left, i+dir_up+2*dir_left, camp);
}

static void gmoa(square i, Side camp) {
  gmaooa(i, i+dir_up+dir_left, i+2*dir_up+dir_left, i+dir_up+2*dir_left, camp);
  gmaooa(i, i+dir_down+dir_right, i+2*dir_down+dir_right, i+dir_down+2*dir_right, camp);
  gmaooa(i, i+dir_up+dir_right, i+dir_up+2*dir_right, i+2*dir_up+dir_right, camp);
  gmaooa(i, i+dir_down+dir_left, i+dir_down+2*dir_left, i+2*dir_down+dir_left, camp);
}

typedef unsigned int mark_type;

static mark_type square_marks[square_h8+1] = { 0 };
static mark_type current_mark = 0;

/* Remove duplicate moves generated for a single piece.
 * @param last_move_of_prev_piece index of last move of previous piece
 */
void remove_duplicate_moves_of_single_piece(numecoup last_move_of_prev_piece)
{
  if (current_mark==UINT_MAX)
  {
    square i;
    for (i = square_a1; i!=square_h8; ++i)
      square_marks[i] = 0;

    current_mark = 1;
  }
  else
    ++current_mark;

  {
    numecoup curr_move;
    numecoup last_unique_move = last_move_of_prev_piece;
    for (curr_move = last_move_of_prev_piece+1;
         curr_move<=current_move[nbply];
         ++curr_move)
    {
      square const sq_arrival = move_generation_stack[curr_move].arrival;
      if (square_marks[sq_arrival]==current_mark)
      {
        // skip over duplicate move
      }
      else
      {
        ++last_unique_move;
        move_generation_stack[last_unique_move] = move_generation_stack[curr_move];
        square_marks[sq_arrival] = current_mark;
      }
    }

    current_move[nbply] = last_unique_move;
  }
}

static void gdoublehopper(square sq_departure, Side camp,
                          numvec vec_start, numvec vec_end)
{
  numvec k,k1;
  piece hurdle;
  square sq_hurdle, past_sq_hurdle;

  square sq_arrival;

  numecoup save_nbcou = current_move[nbply];

  for (k=vec_end; k>=vec_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      past_sq_hurdle= sq_hurdle+vec[k];
      if (e[past_sq_hurdle]==vide)
        for (k1=vec_end; k1>=vec_start; k1--) {
          finligne(past_sq_hurdle,vec[k1],hurdle,sq_hurdle);
          if (hurdle!=obs) {
            sq_arrival= sq_hurdle+vec[k1];
            if (e[sq_arrival]==vide || piece_belongs_to_opponent(e[sq_arrival],camp))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
    }
  }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}

static void gdoublegrasshopper(square sq_departure, Side camp) {
  gdoublehopper(sq_departure,camp,vec_queen_start,vec_queen_end);
}

static void gdoublerookhopper(square sq_departure, Side camp) {
  gdoublehopper(sq_departure,camp,vec_rook_start,vec_rook_end);
}

static void gdoublebishopper(square sq_departure, Side camp) {
  gdoublehopper(sq_departure,camp,vec_bishop_start,vec_bishop_end);
}

typedef enum
{
  UP,
  DOWN
} UPDOWN;

static void filter(square i, numecoup prevnbcou, UPDOWN u)
{
  numecoup s = prevnbcou+1;

  while (s<=current_move[nbply])
    if ((u==DOWN && move_generation_stack[s].arrival-i>-8)
        || (u==UP && move_generation_stack[s].arrival-i<8))
    {
      memmove(move_generation_stack+s,
              move_generation_stack+s+1,
              (current_move[nbply]-s) * sizeof move_generation_stack[s]);
      --current_move[nbply];
    }
    else
      ++s;
}

static void genhunt(square i, piece p, PieNam pabs)
{
  TraceFunctionEntry(__func__);
  TraceSquare(i);
  TracePiece(p);
  TraceFunctionParam("%d",p);
  TraceFunctionParamListEnd();

  /*   PieNam const pabs = abs(p);  */
  assert(pabs>=Hunter0);
  assert(pabs<Hunter0+maxnrhuntertypes);

  {
    unsigned int const typeofhunter = pabs-Hunter0;
    HunterType const * const huntertype = huntertypes+typeofhunter;

    if (p>0) {
      numecoup savenbcou = current_move[nbply];
      gen_wh_piece(i,huntertype->home);
      filter(i,savenbcou,DOWN);

      savenbcou = current_move[nbply];
      gen_wh_piece(i,huntertype->away);
      filter(i,savenbcou,UP);
    }
    else {
      numecoup savenbcou = current_move[nbply];
      gen_bl_piece(i,-huntertype->away);
      filter(i,savenbcou,DOWN);

      savenbcou = current_move[nbply];
      gen_bl_piece(i,-huntertype->home);
      filter(i,savenbcou,UP);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void gfeerrest(square sq_departure, piece p, Side camp)
{
  numvec k;
  square const *bnp;
  PieNam const pabs = abs(p);

  switch (pabs)
  {
  case Mao:
    gmao(sq_departure, camp);
    return;

  case Pao:
    gchin(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case Leo:
    gchin(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case Vao:
    gchin(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case Nao:
    gchin(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case Rose:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_knight_start; k<=vec_knight_end; k++)
    {
      grose(sq_departure, k, 0,+1, camp);
      grose(sq_departure, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case NonStopEquihopper:
    gnequi(sq_departure, camp);
    return;

  case Locust:
    glocust(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case NightLocust:
    glocust(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case BishopLocust:
    glocust(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case RookLocust:
    glocust(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case Kangaroo:
    gkang(sq_departure, camp);
    return;

  case KangarooLion:
    gkanglion(sq_departure, camp);
    return;

  case Kao:
    gchinleap(sq_departure, vec_knight_start, vec_knight_end, camp);
    return;

  case KnightHopper:
    geshop(sq_departure, vec_knight_start, vec_knight_end, camp);
    return;

  case SpiralSpringer:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_knight_start; k<=vec_knight_end; k++)
      gcs(sq_departure, vec[k], vec[25 - k], camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Hamster:
    ghamst(sq_departure);
    return;

  case UbiUbi:
    for (bnp= boardnum; *bnp; bnp++)
      e_ubi[*bnp]= e[*bnp];
    gubi(sq_departure, sq_departure, camp);
    return;

  case Elk:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 0, camp);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Eagle:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 1, camp);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Sparrow:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 2, camp);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Marguerite:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 0, camp);
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 1, camp);
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 2, camp);
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    ghamst(sq_departure);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Archbishop:
    {
      numecoup const save_current_move = current_move[nbply];
      for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
        grfou(sq_departure,sq_departure,vec[k],1,camp);
      if (!NoEdge(sq_departure))
        remove_duplicate_moves_of_single_piece(save_current_move);
    }
    return;

  case ReflectBishop:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      grfou(sq_departure,sq_departure,vec[k],4,camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Cardinal:
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      gcard(sq_departure, sq_departure, vec[k], 1, camp);
    return;

  case DiagonalSpiralSpringer:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= 9; k <= 14; k++)
      gcs(sq_departure, vec[k], vec[23 - k], camp);
    for (k= 15; k <= 16; k++)
      gcs(sq_departure, vec[k], vec[27 - k], camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case BouncyKnight:
  {
    numecoup const save_current_move = current_move[nbply];
    grefc(sq_departure, sq_departure, 2, camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case BouncyNightrider:
  {
    numecoup const save_current_move = current_move[nbply];
    clearedgestraversed();
    grefn(sq_departure, sq_departure, camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case EquiHopper:
    gequi(sq_departure, camp);
    return;

  case CAT:
    gcat(sq_departure, camp);
    return;

  case Sirene:
    gmarin(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case Triton:
    gmarin(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case Nereide:
    gmarin(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case Orphan:
    gorph(sq_departure, camp);
    return;

  case Friend:
    gfriend(sq_departure, camp);
    return;

  case EdgeHog:
    gedgeh(sq_departure, camp);
    return;

  case Moa:
    gmoa(sq_departure, camp);
    return;

  case MoaRider:
    gemoarider(sq_departure, camp);
    return;

  case MaoRider:
    gemaorider(sq_departure, camp);
    return;

  case BoyScout:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      gcs(sq_departure,vec[k],vec[13-k],camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case GirlScout:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_rook_end; k >=vec_rook_start; k--)
      gcs(sq_departure,vec[k],vec[5-k],camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Skylla:
    geskylla(sq_departure, camp);
    return;

  case Charybdis:
    gecharybdis(sq_departure, camp);
    return;

  case Grasshopper:
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case Lion:
    gelrhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case NightriderHopper:
    gerhop(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case CamelHopper:
    gerhop(sq_departure, vec_chameau_start,vec_chameau_end, camp);
    return;

  case ZebraHopper:
    gerhop(sq_departure, vec_zebre_start,vec_zebre_end, camp);
    return;

  case GnuHopper:
    gerhop(sq_departure, vec_chameau_start,vec_chameau_end, camp);
    gerhop(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case RookLion:
    gelrhop(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case BishopLion:
    gelrhop(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case RookHopper:
    gerhop(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case BishopHopper:
    gerhop(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case ContraGras:
    gecrhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case RoseLion:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_knight_start; k<=vec_knight_end; k++)
    {
      groselion(sq_departure, k, 0,+1, camp);
      groselion(sq_departure, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case RoseHopper:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grosehopper(sq_departure, k, 0,+1, camp);
      grosehopper(sq_departure, k,
                  vec_knight_end-vec_knight_start+1,-1, camp);
    }
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case RoseLocust:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      groselocust(sq_departure, k, 0,+1, camp);
      groselocust(sq_departure, k,
                  vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case GrassHopper2:
    gerhop2(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case GrassHopper3:
    gerhop3(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case KingHopper:
    geshop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case DoubleGras:
    gdoublegrasshopper(sq_departure, camp);
    return;

  case DoubleRookHopper:
    gdoublerookhopper(sq_departure, camp);
    return;

  case DoubleBishopper:
    gdoublebishopper(sq_departure, camp);
    return;

  case Orix:
    gorix(sq_departure, camp);
    return;

   case NonStopOrix:
    gnorix(sq_departure, camp);
    return;

  case Gral:
    if (camp==White)
      gebleap(sq_departure, vec_alfil_start,vec_alfil_end);    /* alfilb */
    else
      genleap(sq_departure, vec_alfil_start,vec_alfil_end);    /* alfiln */
    gerhop(sq_departure, vec_rook_start,vec_rook_end, camp);      /* rookhopper */
    return;

  case RookMoose:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_rook_start,vec_rook_end, 0, camp);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case RookEagle:
    gmhop(sq_departure, vec_rook_start,vec_rook_end, 1, camp);
    return;

  case RookSparrow:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_rook_start,vec_rook_end, 2, camp);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case BishopMoose:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 0, camp);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case BishopEagle:
    gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 1, camp);
    return;

  case BishopSparrow:
  {
    numecoup const save_current_move = current_move[nbply];
    gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 2, camp);
    if (!TSTFLAG(spec[sq_departure],ColourChange))
      remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Rao:
  {
    numecoup const save_current_move = current_move[nbply];
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grao(sq_departure, k, 0,+1, camp);
      grao(sq_departure, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    remove_duplicate_moves_of_single_piece(save_current_move);
    return;
  }

  case Scorpion:
    if (camp==White)
      gebleap(sq_departure, vec_queen_start,vec_queen_end);        /* ekingb */
    else
      genleap(sq_departure, vec_queen_start,vec_queen_end);        /* ekingn */
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);     /* grashopper */
    return;

  case NightRiderLion:
    gelrhop(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case MaoRiderLion:
    gemaoriderlion(sq_departure, camp);
    return;

  case MoaRiderLion:
    gemoariderlion(sq_departure, camp);
    return;

  case Dolphin:
    gkang(sq_departure, camp);
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case Rabbit:
    grabbit(sq_departure, camp);
    return;

  case Bob:
    gbob(sq_departure, camp);
    return;

  case EquiEnglish:
    gequiapp(sq_departure, camp);
    return;

  case EquiFrench:
    gnequiapp(sq_departure, camp);
    return;

  case Querquisite:
    switch (sq_departure%onerow - nr_of_slack_files_left_of_board) {
    case file_rook_queenside:
    case file_rook_kingside:
      if (camp == White)
        gebrid(sq_departure, vec_rook_start,vec_rook_end);
      else
        genrid(sq_departure, vec_rook_start,vec_rook_end);
      break;
    case file_bishop_queenside:
    case file_bishop_kingside:  if (camp == White)
        gebrid(sq_departure, vec_bishop_start,vec_bishop_end);
      else
        genrid(sq_departure, vec_bishop_start,vec_bishop_end);
      break;
    case file_queen:    if (camp == White)
        gebrid(sq_departure, vec_queen_start,vec_queen_end);
      else
        genrid(sq_departure, vec_queen_start,vec_queen_end);
      break;
    case file_knight_queenside:
    case file_knight_kingside:  if (camp == White)
        gebleap(sq_departure, vec_knight_start,vec_knight_end);
      else
        genleap(sq_departure, vec_knight_start,vec_knight_end);
      break;
    case file_king: if (camp == White)
        gebleap(sq_departure, vec_queen_start,vec_queen_end);
      else
        genleap(sq_departure, vec_queen_start,vec_queen_end);
      break;
    }
    break;

  case Bouncer :
    genbouncer(sq_departure, vec_queen_start,vec_queen_end, camp);
    break;

  case RookBouncer:
    genbouncer(sq_departure, vec_rook_start,vec_rook_end, camp);
    break;

  case BishopBouncer :
    genbouncer(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    break;

  case RadialKnight:
    genradialknight(sq_departure, camp);
    break;

  case Treehopper:
    gentreehopper(sq_departure, camp);
    break;

  case Leafhopper :
    genleafhopper(sq_departure, camp);
    break;

  case GreaterTreehopper:
    gengreatertreehopper(sq_departure, camp);
    break;

  case GreaterLeafhopper:
    gengreaterleafhopper(sq_departure, camp);
    break;

  case SpiralSpringer40:
  {
    numecoup const save_current_move = current_move[nbply];
    gcsp(sq_departure, 9, 16, camp);
    gcsp(sq_departure, 10, 11, camp);
    gcsp(sq_departure, 11, 10, camp);
    gcsp(sq_departure, 12, 13, camp);
    gcsp(sq_departure, 13, 12, camp);
    gcsp(sq_departure, 14, 15, camp);
    gcsp(sq_departure, 15, 14, camp);
    gcsp(sq_departure, 16, 9, camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    break;
  }

  case SpiralSpringer20:
  {
    numecoup const save_current_move = current_move[nbply];
    gcsp(sq_departure, 9, 12, camp);
    gcsp(sq_departure, 10, 15, camp);
    gcsp(sq_departure, 11, 14, camp);
    gcsp(sq_departure, 12, 9, camp);
    gcsp(sq_departure, 13, 16, camp);
    gcsp(sq_departure, 14, 11, camp);
    gcsp(sq_departure, 15, 10, camp);
    gcsp(sq_departure, 16, 13, camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    break;
  }

  case SpiralSpringer33:
  {
    numecoup const save_current_move = current_move[nbply];
    gcsp(sq_departure, 9, 10, camp);
    gcsp(sq_departure, 10, 9, camp);
    gcsp(sq_departure, 11, 12, camp);
    gcsp(sq_departure, 12, 11, camp);
    gcsp(sq_departure, 13, 14, camp);
    gcsp(sq_departure, 14, 13, camp);
    gcsp(sq_departure, 15, 16, camp);
    gcsp(sq_departure, 16, 15, camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    break;
  }

  case SpiralSpringer11:
  {
    numecoup const save_current_move = current_move[nbply];
    gcsp(sq_departure, 9, 14, camp);
    gcsp(sq_departure, 10, 13, camp);
    gcsp(sq_departure, 11, 16, camp);
    gcsp(sq_departure, 12, 15, camp);
    gcsp(sq_departure, 13, 10, camp);
    gcsp(sq_departure, 14, 9, camp);
    gcsp(sq_departure, 15, 12, camp);
    gcsp(sq_departure, 16, 11, camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    break;
  }

  case Quintessence:
  {
    numecoup const save_current_move = current_move[nbply];
    gcsp(sq_departure, 9, 11, camp);
    gcsp(sq_departure, 11, 9, camp);
    gcsp(sq_departure, 11, 13, camp);
    gcsp(sq_departure, 13, 11, camp);
    gcsp(sq_departure, 13, 15, camp);
    gcsp(sq_departure, 15, 13, camp);
    gcsp(sq_departure, 15, 9, camp);
    gcsp(sq_departure, 9, 15, camp);
    gcsp(sq_departure, 10, 12, camp);
    gcsp(sq_departure, 12, 10, camp);
    gcsp(sq_departure, 12, 14, camp);
    gcsp(sq_departure, 14, 12, camp);
    gcsp(sq_departure, 14, 16, camp);
    gcsp(sq_departure, 16, 14, camp);
    gcsp(sq_departure, 16, 10, camp);
    gcsp(sq_departure, 10, 16, camp);
    remove_duplicate_moves_of_single_piece(save_current_move);
    break;
  }

  case MarineKnight:
    generate_marine_knight(sq_departure,camp);
    break;

  case Poseidon:
    generate_poseidon(sq_departure,camp);
    break;

  case MarinePawn:
    generate_marine_pawn(sq_departure,camp);
    break;

  default:
    /* Since pieces like DUMMY fall through 'default', we have */
    /* to check exactly if there is something to generate ...  */
    if ((pabs>=Hunter0) && (pabs<Hunter0+maxnrhuntertypes))
      genhunt(sq_departure,p,pabs);
    break;
  }
} /* gfeerrest */

/* Two auxiliary functions for generating super pawn moves */
static void gen_sp_nocaptures(square sq_departure, numvec dir) {
  /* generates non capturing moves of a super pawn in direction dir */

  square sq_arrival= sq_departure+dir;

  /* it can move from first rank */
  for (; e[sq_arrival]==vide; sq_arrival+= dir)
    empile(sq_departure,sq_arrival,sq_arrival);
}

static void gen_sp_captures(square sq_departure, numvec dir, Side camp) {
  /* generates capturing moves of a super pawn of colour camp in
     direction dir.  */

  piece   hurdle;

  square sq_arrival;

  /* it can move from first rank */
  finligne(sq_departure,dir,hurdle,sq_arrival);
  if (piece_belongs_to_opponent(hurdle,camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

static void gencpn(square i) {
  genleap(i, 4, 4);
  if (2*i < square_h8+square_a1) {
    genleap(i, 1, 1);
    genleap(i, 3, 3);
  }
}

static void gencpb(square i) {
  gebleap(i, 2, 2);
  if (2*i > square_h8+square_a1) {
    gebleap(i, 1, 1);
    gebleap(i, 3, 3);
  }
}

static void gen_p_captures(square sq_departure, square sq_arrival, Side camp);
static void gen_p_nocaptures(square sq_departure, numvec dir, int steps);

void gfeerblanc(square i, piece p) {
  switch(p) {
  case nb:
    gebrid(i, vec_knight_start,vec_knight_end);
    return;

  case zb:
    gebleap(i, vec_zebre_start,vec_zebre_end);
    return;

  case chb:
    gebleap(i, vec_chameau_start,vec_chameau_end);
    return;

  case gib:
    gebleap(i, vec_girafe_start,vec_girafe_end);
    return;

  case rccinqb:
    gebleap(i, vec_rccinq_start,vec_rccinq_end);
    return;

  case bub:
    gebleap(i, vec_bucephale_start,vec_bucephale_end);
    return;

  case vizirb:
    gebleap(i, vec_rook_start,vec_rook_end);
    return;

  case alfilb:
    gebleap(i, vec_alfil_start,vec_alfil_end);
    return;

  case fersb:
    gebleap(i, vec_bishop_start,vec_bishop_end);
    return;

  case dabb:
    gebleap(i, vec_dabbaba_start,vec_dabbaba_end);
    return;

  case pbb:
    genpbb(i);
    return;

  case reversepb:
    genreversepb(i);
    return;

  case amazb:
    gebrid(i, vec_queen_start,vec_queen_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case impb:
    gebrid(i, vec_rook_start,vec_rook_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case princb:
    gebrid(i, vec_bishop_start,vec_bishop_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case gnoub:
    gebleap(i, vec_chameau_start,vec_chameau_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case antilb:
    gebleap(i, vec_antilope_start,vec_antilope_end);
    return;

  case ecurb:
    gebleap(i, vec_ecureuil_start,vec_ecureuil_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case waranb:
    gebrid(i, vec_knight_start,vec_knight_end);
    gebrid(i, vec_rook_start,vec_rook_end);
    return;

  case dragonb:
    genpb(i);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case gryphonb:
    if (i<=square_h1)
    {
      /* pawn on first rank */
      if (anyparrain
          || CondFlag[einstein]
          || CondFlag[normalp]
          || CondFlag[circecage]
          || abs(e[i]) == orphanb)
      {
        /* triple or single step? */
        gen_p_nocaptures(i,+dir_up, CondFlag[einstein] ? 3 : 1);
      }
    }
    else
    {
      /* double or single step? */
      gen_p_nocaptures(i,+dir_up, i<=square_h2 ? 2 : 1);
    }
    gebrid(i, vec_bishop_start,vec_bishop_end);
    return;

  case shipb:
    if (i >= square_a2
      || anyparrain
      || CondFlag[einstein]
      || CondFlag[normalp]
      || CondFlag[circecage]
      || abs(e[i]) == orphanb)
    {
      gen_p_captures(i, i+dir_up+dir_left, White);
      gen_p_captures(i, i+dir_up+dir_right, White);
    }
    gebrid(i, vec_rook_start,vec_rook_end);
    return;

  case camridb:
    gebrid(i, vec_chameau_start,vec_chameau_end);
    return;

  case zebridb:
    gebrid(i, vec_zebre_start,vec_zebre_end);
    return;

  case gnuridb:
    gebrid(i, vec_chameau_start,vec_chameau_end);
    gebrid(i, vec_knight_start,vec_knight_end);
    return;

  case bspawnb:
    gen_sp_nocaptures(i,+dir_up+dir_left);
    gen_sp_nocaptures(i,+dir_up+dir_right);
    gen_sp_captures(i,+dir_up, White);
    return;

  case spawnb:
    gen_sp_nocaptures(i,+dir_up);
    gen_sp_captures(i,+dir_up+dir_left, White);
    gen_sp_captures(i,+dir_up+dir_right, White);
    return;

  case rhuntb:
    gebrid(i, 2, 2);
    gebrid(i, 7, 8);
    return;

  case bhuntb:
    gebrid(i, 4, 4);
    gebrid(i, 5, 6);
    return;

  case ekingb:
    gebleap(i, vec_queen_start,vec_queen_end);
    return;

  case okapib:
    gebleap(i, vec_okapi_start,vec_okapi_end);
    return;

  case leap37b:
    gebleap(i, vec_leap37_start,vec_leap37_end);
    return;

  case leap16b:
    gebleap(i, vec_leap16_start,vec_leap16_end);
    return;

  case leap24b:
    gebleap(i, vec_leap24_start,vec_leap24_end);
    return;

  case leap35b:
    gebleap(i, vec_leap35_start,vec_leap35_end);
    return;

  case leap15b:
    gebleap(i, vec_leap15_start,vec_leap15_end);
    return;

  case leap25b:
    gebleap(i, vec_leap25_start,vec_leap25_end);
    return;

  case vizridb:
    gebrid(i,   vec_rook_start,vec_rook_end);
    return;

  case fersridb:
    gebrid(i, vec_bishop_start,vec_bishop_end);
    return;

  case bisonb:
    gebleap(i, vec_bison_start,vec_bison_end);
    return;

  case zebub:
    gebleap(i, vec_chameau_start,vec_chameau_end);
    gebleap(i, vec_girafe_start,vec_girafe_end);
    return;

  case elephantb:
    gebrid(i, vec_elephant_start,vec_elephant_end);
    return;

  case leap36b:
    gebleap(i, vec_leap36_start,vec_leap36_end);
    return;

  case chinesepawnb:
    gencpb(i);
    return;

  default:
    gfeerrest(i, p, White);
    break;
  }
}

void gfeernoir(square i, piece p) {
  switch(p) {
  case nn:
    genrid(i, vec_knight_start,vec_knight_end);
    return;

  case zn:
    genleap(i, vec_zebre_start,vec_zebre_end);
    return;

  case chn:
    genleap(i, vec_chameau_start,vec_chameau_end);
    return;

  case gin:
    genleap(i, vec_girafe_start,vec_girafe_end);
    return;

  case rccinqn:
    genleap(i, vec_rccinq_start,vec_rccinq_end);
    return;

  case bun:
    genleap(i, vec_bucephale_start,vec_bucephale_end);
    return;

  case vizirn:
    genleap(i, vec_rook_start,vec_rook_end);
    return;

  case alfiln:
    genleap(i, vec_alfil_start,vec_alfil_end);
    return;

  case fersn:
    genleap(i, vec_bishop_start,vec_bishop_end);
    return;

  case dabn:
    genleap(i, vec_dabbaba_start,vec_dabbaba_end);
    return;

  case pbn:
    genpbn(i);
    return;

  case reversepn:
    genreversepn(i);
    return;

  case amazn:
    genrid(i, vec_queen_start,vec_queen_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case impn:
    genrid(i, vec_rook_start,vec_rook_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case princn:
    genrid(i, vec_bishop_start,vec_bishop_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case gnoun:
    genleap(i, vec_chameau_start,vec_chameau_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case antiln:
    genleap(i, vec_antilope_start,vec_antilope_end);
    return;

  case ecurn:
    genleap(i, vec_ecureuil_start,vec_ecureuil_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case warann:
    genrid(i, vec_knight_start,vec_knight_end);
    genrid(i, vec_rook_start,vec_rook_end);
    return;

  case dragonn:
    genpn(i);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case gryphonn:
      if (i>=square_a8)
      {
        /* pawn on first rank */
        if (anyparrain
            || CondFlag[einstein]
            || CondFlag[normalp]
            || CondFlag[circecage]
            || abs(e[i]) == orphanb)
        {
          /* triple or single step? */
            gen_p_nocaptures(i,dir_down, CondFlag[einstein] ? 3 : 1);
        }
      }
      else
      {
        {
          /* double or single step? */
            gen_p_nocaptures(i,dir_down, CondFlag[einstein] ? 3 : 1);
        }
      }
    genrid(i, vec_bishop_start,vec_bishop_end);
    return;

  case shipn:
    if (i<=square_h7
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || abs(e[i])==orphanb)
    {
      gen_p_captures(i, i+dir_down+dir_right, Black);
      gen_p_captures(i, i+dir_down+dir_left, Black);
    }
    genrid(i, vec_rook_start,vec_rook_end);
    return;

  case camridn:
    genrid(i, vec_chameau_start,vec_chameau_end);
    return;

  case zebridn:
    genrid(i, vec_zebre_start,vec_zebre_end);
    return;

  case gnuridn:
    genrid(i, vec_chameau_start,vec_chameau_end);
    genrid(i, vec_knight_start,vec_knight_end);
    return;

  case bspawnn:
    gen_sp_nocaptures(i,+dir_down+dir_right);
    gen_sp_nocaptures(i,+dir_down+dir_left);
    gen_sp_captures(i,+dir_down, Black);
    return;

  case spawnn:
    gen_sp_nocaptures(i,+dir_down);
    gen_sp_captures(i,+dir_down+dir_right, Black);
    gen_sp_captures(i,+dir_down+dir_left, Black);
    return;

  case rhuntn:
    genrid(i, 2, 2);
    genrid(i, 7, 8);
    return;

  case bhuntn:
    genrid(i, 4, 4);
    genrid(i, 5, 6);
    return;

  case ekingn:
    genleap(i, vec_queen_start,vec_queen_end);
    return;

  case okapin:
    genleap(i, vec_okapi_start,vec_okapi_end);
    return;

  case leap37n:
    genleap(i, vec_leap37_start,vec_leap37_end);
    return;

  case leap16n:
    genleap(i, vec_leap16_start,vec_leap16_end);
    return;

  case leap24n:
    genleap(i, vec_leap24_start,vec_leap24_end);
    return;

  case leap35n:
    genleap(i, vec_leap35_start,vec_leap35_end);
    return;

  case leap15n:
    genleap(i, vec_leap15_start,vec_leap15_end);
    return;

  case leap25n:
    genleap(i, vec_leap25_start,vec_leap25_end);
    return;

  case vizridn:
    genrid(i, vec_rook_start,vec_rook_end);
    return;

  case fersridn:
    genrid(i, vec_bishop_start,vec_bishop_end);
    return;

  case bisonn:
    genleap(i, vec_bison_start,vec_bison_end);
    return;

  case zebun:
    genleap(i, vec_chameau_start,vec_chameau_end);
    genleap(i, vec_girafe_start,vec_girafe_end);
    return;

  case elephantn:
    genrid(i, vec_elephant_start,vec_elephant_end);
    return;

  case leap36n:
    genleap(i, vec_leap36_start,vec_leap36_end);
    return;

  case chinesepawnn:
    gencpn(i);
    return;

  default:
    gfeerrest(i, p, Black);
    break;
  }
} /* end of gfeernoir */

void genrb(square sq_departure)
{
  Side const side = White;
  boolean   flag = false;       /* K im Schach ? */
  numecoup const save_nbcou = current_move[nbply];

  if (calc_reflective_king[side] && !calctransmute)
  {
    /* K im Schach zieht auch */
    calctransmute = true;
    if (!normaltranspieces[side] && echecc(side))
    {
      PieNam const *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=Empty; ++ptrans)
      {
        flag = true;
        current_trans_gen = *ptrans;
        gen_wh_piece(sq_departure,*ptrans);
        current_trans_gen = vide;
      }
    }
    else if (normaltranspieces[side])
    {
      PieNam const *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=Empty; ++ptrans)
      {
        piece const ptrans_black = -*ptrans;
        if (nbpiece[ptrans_black]
            && (*checkfunctions[*ptrans])(sq_departure,ptrans_black,&validate_observation))
        {
          flag = true;
          current_trans_gen = *ptrans;
          gen_wh_piece(sq_departure,*ptrans);
          current_trans_gen = vide;
        }
      }
    }
    calctransmute= false;

    if (flag && nbpiece[orphann]>0)
    {
      piece const king = e[king_square[side]];
      e[king_square[side]] = dummyb;
      if (!echecc(side))
        /* side's king checked only by an orphan empowered by the king */
        flag= false;
      e[king_square[side]] = king;
    }


    /* K im Schach zieht nur */
    if (calc_transmuting_king[side] && flag)
      return;
  }

  if (CondFlag[sting])
    gerhop(sq_departure,vec_queen_start,vec_queen_end,side);

  {
    numvec k;
    for (k= vec_queen_end; k >=vec_queen_start; k--)
    {
      square const sq_arrival = sq_departure+vec[k];
      if (e[sq_arrival] <= vide)
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }

  if (flag)
    remove_duplicate_moves_of_single_piece(save_nbcou);

  /* Now we test castling */
  if (castling_supported)
    generate_castling(side);

  if (CondFlag[castlingchess] && !echecc(side))
  {
    numvec k;
    for (k = vec_queen_end; k>= vec_queen_start; --k)
    {
      square const sq_passed = sq_departure+vec[k];
      square const sq_arrival = sq_passed+vec[k];
      square sq_castler;
      piece p;

      finligne(sq_departure,vec[k],p,sq_castler);
      if (sq_castler!=sq_passed && sq_castler!=sq_arrival && abs(p)>=roib
          && castling_is_intermediate_king_move_legal(side,sq_departure,sq_passed))
        empile(sq_departure,sq_arrival,maxsquare+sq_castler);
    }
  }

  if (CondFlag[platzwechselrochade] && platzwechsel_rochade_allowed[White][nbply])
  {
    int i;
    square square_a = square_a1;
    for (i = nr_rows_on_board; i>0; --i, square_a += onerow)
    {
      int j;
      square pos_partner = square_a;
      for (j = nr_files_on_board; j>0; --j, pos_partner += dir_right)
        if (pos_partner!=sq_departure
            && TSTFLAG(spec[pos_partner],White)
            && !is_pawn(abs(e[pos_partner]))) /* not sure if "castling" with Ps forbidden */
          empile(sq_departure,pos_partner,platzwechsel_rochade);
    }
  }
}

void gen_wh_ply(void)
{
  square i, j, z;
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* Don't try to "optimize" by hand. The double-loop is tested as
     the fastest way to compute (due to compiler-optimizations !)
     V3.14  NG
  */
  z= square_a1;
  for (i= nr_rows_on_board; i > 0; i--, z+= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z++) {
      if ((p = e[z]) != vide) {
        if (TSTFLAG(spec[z], Neutral))
          p = -p;
        if (p > obs)
          gen_wh_piece(z, p);
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void gen_piece_aux(Side side, square z, piece p)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(z);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (CondFlag[annan])
  {
    int const annaniser_dir = side==White ? -onerow : +onerow;
    square const annaniser_pos = z+annaniser_dir;
    piece const annaniser_walk = e[annaniser_pos];
    if (annanises(side,annaniser_pos,z))
    {
      boolean const save_castling_supported = castling_supported;

      castling_supported = false;

      if (side==White)
        gen_wh_piece_aux(z,annaniser_walk);
      else
        gen_bl_piece_aux(z,annaniser_walk);

      castling_supported = save_castling_supported;
    }
    else
    {
      if (side==White)
        gen_wh_piece_aux(z,p);
      else
        gen_bl_piece_aux(z,p);
    }
  }
  else
  {
    if (side==White)
      gen_wh_piece_aux(z,p);
    else
      gen_bl_piece_aux(z,p);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void gen_wh_piece_aux(square z, piece p) {

  TraceFunctionEntry(__func__);
  TraceSquare(z);
  TracePiece(p);
  TraceFunctionParamListEnd();

  switch(p)
  {
    case roib:
      genrb(z);
      break;

    case pb:
      genpb(z);
      break;

    case cb:
      gebleap(z, vec_knight_start,vec_knight_end);
      break;

    case tb:
      gebrid(z, vec_rook_start,vec_rook_end);
      break;

    case db:
      gebrid(z, vec_queen_start,vec_queen_end);
      break;

    case fb:
      gebrid(z, vec_bishop_start,vec_bishop_end);
      break;

    default:
      gfeerblanc(z, p);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void orig_gen_wh_piece(square sq_departure, piece p)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (!(CondFlag[madras] && !madrasi_can_piece_move(sq_departure))
      && !(CondFlag[eiffel] && !eiffel_can_piece_move(sq_departure))
      && !(CondFlag[disparate] && !disparate_can_piece_move(sq_departure))
      && !(TSTFLAG(some_pieces_flags,Paralysing) && is_piece_paralysed_on(sq_departure))
      && !(CondFlag[ultrapatrouille] && !patrol_is_supported(sq_departure))
      && !(CondFlag[central] && !central_can_piece_move_from(sq_departure))
      && !(TSTFLAG(spec[sq_departure],Beamtet) && !beamten_is_observed(sq_departure)))
  {
    if (CondFlag[phantom])
      phantom_chess_generate_moves(White,p,sq_departure);
    else if (CondFlag[plus])
      plus_generate_moves(White,p,sq_departure);
    else if (anymars)
      marscirce_generate_moves(White,p,sq_departure);
    else if (anyantimars)
      antimars_generate_moves(White,p,sq_departure);
    else
      gen_piece_aux(White ,sq_departure,p);

    if (CondFlag[messigny] && !(king_square[White]==sq_departure && rex_mess_ex))
    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
        if (e[*bnp]==-p)
          empile(sq_departure,*bnp,messigny_exchange);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* orig_gen_wh_piece */

void singleboxtype3_gen_wh_piece(square z, piece p)
{
  numecoup save_nbcou = current_move[nbply];
  unsigned int latent_prom = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,White);
       sq!=initsquare;
       sq = next_latent_pawn(sq,White))
  {
    PieNam pprom;
    for (pprom = next_singlebox_prom(Empty,White);
         pprom!=Empty;
         pprom = next_singlebox_prom(pprom,White))
    {
      numecoup prev_nbcou = current_move[nbply];
      ++latent_prom;
      e[sq] = pprom;
      orig_gen_wh_piece(z, sq==z ? (piece)pprom : p);
      e[sq] = pb;
      for (++prev_nbcou; prev_nbcou<=current_move[nbply]; ++prev_nbcou)
      {
        move_generation_stack[prev_nbcou].singlebox_type3_promotion_where = sq;
        move_generation_stack[prev_nbcou].singlebox_type3_promotion_what = pprom;
      }
    }
  }

  if (latent_prom==0)
  {
    orig_gen_wh_piece(z,p);

    for (++save_nbcou; save_nbcou<=current_move[nbply]; ++save_nbcou)
    {
      move_generation_stack[save_nbcou].singlebox_type3_promotion_where = initsquare;
      move_generation_stack[save_nbcou].singlebox_type3_promotion_what = vide;
    }
  }
}

void (*gen_wh_piece)(square z, piece p)
  = &orig_gen_wh_piece;


void gorph(square i, Side camp)
{
  numecoup const save_nbcou = current_move[nbply];

  PieNam const *porph;
  for (porph = orphanpieces; *porph!=Empty; ++porph)
    if (nbpiece[*porph]>0 || nbpiece[-(piece)*porph]>0)
    {
      if (camp == White)
      {
        if (ooorphancheck(i,-*porph,orphann,&validate_observation))
          gen_wh_piece(i,*porph);
      }
      else
      {
        if (ooorphancheck(i,*porph,orphanb,&validate_observation))
          gen_bl_piece(i,-*porph);
      }
    }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}

void gfriend(square i, Side camp)
{
  numecoup const save_nbcou = current_move[nbply];

  PieNam const *pfr;
  for (pfr = orphanpieces; *pfr!=Empty; ++pfr)
    if (nbpiece[*pfr]>0)
    {
      if (camp==White)
      {
        if (fffriendcheck(i,*pfr,friendb,&validate_observation))
          gen_wh_piece(i, *pfr);
      }
      else
      {
        if (fffriendcheck(i,-*pfr,friendn,&validate_observation))
          gen_bl_piece(i, -*pfr);
      }
    }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}


void gedgeh(square sq_departure, Side camp) {
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k >=vec_queen_start; k--) {
    sq_arrival= sq_departure+vec[k];
    while (e[sq_arrival]==vide) {
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        empile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= vec[k];
    }

    if (piece_belongs_to_opponent(e[sq_arrival],camp))
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void geskylchar(square sq_departure, square sq_arrival, square sq_capture,
                Side camp) {
  if (e[sq_arrival] == vide) {
    if (e[sq_capture]==vide)
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (piece_belongs_to_opponent(e[sq_capture],camp))
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

void geskylla(square i, Side camp) {
  geskylchar(i, i+dir_up+2*dir_right, i+dir_right, camp);
  geskylchar(i, i+2*dir_up+dir_right, i+dir_up, camp);
  geskylchar(i, i+2*dir_up+dir_left, i+dir_up, camp);
  geskylchar(i, i+dir_up+2*dir_left, i+dir_left, camp);
  geskylchar(i, i+dir_down+2*dir_left, i+dir_left, camp);
  geskylchar(i, i+2*dir_down+dir_left, i+dir_down, camp);
  geskylchar(i, i+2*dir_down+dir_right, i+dir_down, camp);
  geskylchar(i, i+dir_down+2*dir_right, i+dir_right, camp);
}

void gecharybdis(square i, Side camp) {
  geskylchar(i, i+dir_up+2*dir_right, i+dir_up+dir_right, camp);
  geskylchar(i, i+2*dir_up+dir_right, i+dir_up+dir_right, camp);
  geskylchar(i, i+2*dir_up+dir_left, i+dir_up+dir_left, camp);
  geskylchar(i, i+dir_up+2*dir_left, i+dir_up+dir_left, camp);
  geskylchar(i, i+dir_down+2*dir_left, i+dir_down+dir_left, camp);
  geskylchar(i, i+2*dir_down+dir_left, i+dir_down+dir_left, camp);
  geskylchar(i, i+2*dir_down+dir_right, i+dir_down+dir_right, camp);
  geskylchar(i, i+dir_down+2*dir_right, i+dir_down+dir_right, camp);
}

/***********************************************************************
 **                                    **
 **       generating functions for pawns               **
 **                                    **
 ***********************************************************************/

/* Two auxiliary function for generating pawn moves */
static void gen_p_captures(square sq_departure, square sq_arrival, Side camp)
{
  /* generates move of a pawn of side camp on departure capturing a
     piece on arrival
  */
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceEnumerator(Side,camp,"");
  TraceFunctionParamListEnd();

  if (piece_belongs_to_opponent(e[sq_arrival],camp))
    /* normal capture */
    empile(sq_departure,sq_arrival,sq_arrival);
  else if ((abs(e[sq_departure])!=Orphan)   /* orphans cannot capture ep */
           && (sq_arrival==ep[parent_ply[nbply]] || sq_arrival==einstein_ep[parent_ply[nbply]])
           /* a pawn has just done a critical move */
           && trait[parent_ply[nbply]]!=camp) /* the opponent has just moved */
  {
    /* ep capture */
    square prev_arrival;

    if (nbply==2)
    {    /* ep.-key  standard pawn */
      if (camp==White)
        move_generation_stack[current_move[1]].arrival = sq_arrival+dir_down;
      else
        move_generation_stack[current_move[1]].arrival = sq_arrival+dir_up;
    }

    prev_arrival = move_generation_stack[current_move[parent_ply[nbply]]].arrival;

    if (TSTFLAG(sq_spec[prev_arrival],Wormhole))
      prev_arrival = wormhole_positions[wormhole_next_transfer[parent_ply[nbply]]-1];

    if (piece_belongs_to_opponent(e[prev_arrival],camp))
      empile(sq_departure,sq_arrival,prev_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* end gen_p_captures */

static void gen_p_nocaptures(square sq_departure, numvec dir, int steps)
{
  /* generates moves of a pawn in direction dir where steps single
     steps are possible.
  */

  square sq_arrival= sq_departure+dir;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",steps);
  TraceFunctionParamListEnd();

  while (steps--)
  {
    TraceSquare(sq_arrival);
    TracePiece(e[sq_arrival]);
    TraceText("\n");
    if (e[sq_arrival]==vide && empile(sq_departure,sq_arrival,sq_arrival))
      sq_arrival+= dir;
    else
      break;
    TraceValue("%d\n",steps);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/****************************  white pawn  ****************************/
void genpb(square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (sq_departure<=square_h1)
  {
    /* pawn on first rank */
    if (anyparrain
        || CondFlag[einstein]
        || CondFlag[normalp]
        || CondFlag[circecage]
        || abs(e[sq_departure])==Orphan
        || TSTFLAG(sq_spec[sq_departure],Wormhole))
    {
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, White);
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, White);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_up, CondFlag[einstein] ? 3 : 1);
    }
  }
  else
  {
    gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, White);
    gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, White);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,+dir_up, sq_departure<=square_h2 ? 2 : 1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* end of genpb */

/****************************  black pawn  ****************************/
void genpn(square sq_departure)
{
  if (sq_departure>=square_a8)
  {
    /* pawn on last rank */
    if (anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || abs(e[sq_departure])==Orphan
        || TSTFLAG(sq_spec[sq_departure],Wormhole))
    {
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, Black);
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, Black);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,dir_down, CondFlag[einstein] ? 3 : 1);
    }
  }
  else
  {
    gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, Black);
    gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, Black);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,dir_down, sq_departure>=square_a7 ? 2 : 1);
  }
}

void genreversepb(square sq_departure)
{
  if (sq_departure>=square_a8)
  {
    /* pawn on last rank */
    if (anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, White);
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, White);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,dir_down, CondFlag[einstein] ? 3 : 1);
    }
  }
  else
  {
    /* not last rank */
    gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, White);
    gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, White);
    /* double or single step? */
    gen_p_nocaptures(sq_departure, dir_down, sq_departure>=square_a7 ? 2 : 1);
  }
}

void genreversepn(square sq_departure)
{
  if (sq_departure<=square_h1)
  {
    /* pawn on last rank */
    if (anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, Black);
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, Black);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure, dir_up, CondFlag[einstein] ? 3 : 1);
    }
  }
  else
  {
    gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, Black);
    gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, Black);
    /* double or single step? */
    gen_p_nocaptures(sq_departure, dir_up, sq_departure<=square_h1 ? 2 : 1);
  }
}

/************************  white berolina pawn  ***********************/
void genpbb(square sq_departure)
{
  if (sq_departure<=square_h1)
  {
    /* pawn on first rank */
    if ( anyparrain
         || CondFlag[normalp]
         || CondFlag[einstein]
         || CondFlag[circecage]
         || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_up, White);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_up+dir_left, CondFlag[einstein] ? 3 : 1);
      gen_p_nocaptures(sq_departure,+dir_up+dir_right, CondFlag[einstein] ? 3 : 1);
    }
  }
  else {
    /* not first rank */
    gen_p_captures(sq_departure, sq_departure+dir_up, White);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,+dir_up+dir_left, sq_departure<=square_h2 ? 2 : 1);
    gen_p_nocaptures(sq_departure,+dir_up+dir_right, sq_departure<=square_h2 ? 2 : 1);
  }
}

/************************  black berolina pawn  ***********************/
void genpbn(square sq_departure) {
  if (sq_departure>=square_a8)
  {
    /* pawn on last rank */
    if ( anyparrain
         || CondFlag[normalp]
         || CondFlag[einstein]
         || CondFlag[circecage]
         || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_down, Black);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_down+dir_right, CondFlag[einstein] ? 3 : 1);
      gen_p_nocaptures(sq_departure,+dir_down+dir_left, CondFlag[einstein] ? 3 : 1);
    }
    else {
      return;
    }
  }
  else {
    /* not last rank */
    gen_p_captures(sq_departure, sq_departure+dir_down, Black);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,+dir_down+dir_right,
                     sq_departure>=square_a7 ? 2 : 1);
    gen_p_nocaptures(sq_departure,+dir_down+dir_left,
                     sq_departure>=square_a7 ? 2 : 1);
  }
}


static void genleapleap(square sq_departure, numvec kanf, numvec kend, int hurdletype, Side camp, boolean leaf)
{
  square  sq_arrival, sq_hurdle;
  numvec  k, k1;

  for (k= kanf; k<=kend; ++k)
  {
    sq_hurdle= sq_departure + vec[k];
    if ((hurdletype==0 && piece_belongs_to_opponent(e[sq_hurdle],camp))
        || (hurdletype==1 && abs(e[sq_hurdle])>=roib))
    {
      for (k1= kanf; k1 <= kend; k1++) {
        sq_arrival = (leaf ? sq_departure : sq_hurdle) + vec[k1];
        if ((sq_arrival != sq_hurdle) && (e[sq_arrival] == vide || piece_belongs_to_opponent(e[sq_arrival], camp)))
          empile(sq_departure, sq_arrival, sq_arrival);
      }
    }
  }
}

void genqlinesradial(square sq_departure, Side camp, int hurdletype, boolean leaf)
{
  numecoup const save_current_move = current_move[nbply];

  genleapleap(sq_departure, vec_rook_start, vec_rook_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_dabbaba_start, vec_dabbaba_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap03_start, vec_leap03_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap04_start, vec_leap04_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap05_start, vec_leap05_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap06_start, vec_leap06_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap07_start, vec_leap07_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_bishop_start, vec_bishop_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_alfil_start, vec_alfil_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap33_start, vec_leap33_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap44_start, vec_leap44_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap55_start, vec_leap55_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap66_start, vec_leap66_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap77_start, vec_leap77_end, hurdletype, camp, leaf);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void genradial(square sq_departure, Side camp, int hurdletype, boolean leaf)
{
  numecoup const save_current_move = current_move[nbply];

  genleapleap(sq_departure, vec_rook_start, vec_rook_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_dabbaba_start, vec_dabbaba_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap03_start, vec_leap03_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap04_start, vec_leap04_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_bucephale_start, vec_bucephale_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap06_start, vec_leap06_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap07_start, vec_leap07_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_bishop_start, vec_bishop_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_knight_start, vec_knight_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_chameau_start, vec_chameau_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_girafe_start, vec_girafe_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap15_start, vec_leap15_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap16_start, vec_leap16_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_rccinq_start, vec_rccinq_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_alfil_start, vec_alfil_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_zebre_start, vec_zebre_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap24_start, vec_leap24_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap25_start, vec_leap25_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap26_start, vec_leap26_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap27_start, vec_leap27_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap33_start, vec_leap33_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap35_start, vec_leap35_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap36_start, vec_leap36_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap37_start, vec_rccinq_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap44_start, vec_leap44_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap45_start, vec_leap45_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap46_start, vec_leap46_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap47_start, vec_leap47_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap56_start, vec_leap56_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap57_start, vec_leap57_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap66_start, vec_leap66_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap67_start, vec_leap67_end, hurdletype, camp, leaf);
  genleapleap(sq_departure, vec_leap77_start, vec_leap77_end, hurdletype, camp, leaf);

  remove_duplicate_moves_of_single_piece(save_current_move);
}

void genradialknight(square sq_departure, Side camp)
{
  genradial(sq_departure, camp, 0, false);
}

void gentreehopper(square sq_departure, Side camp)
{
  genqlinesradial(sq_departure, camp, 1, false);
}

void gengreatertreehopper(square sq_departure, Side camp)
{
  genradial(sq_departure, camp, 1, false);
}

void genleafhopper(square sq_departure, Side camp)
{
  genqlinesradial(sq_departure, camp, 1, true);
}

void gengreaterleafhopper(square sq_departure, Side camp)
{
  genradial(sq_departure, camp, 1, true);
}

void generate_marine_knight(square sq_departure, Side moving)
{
  Side const opponent = advers(moving);
  numvec  k;
  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    square sq_arrival = sq_departure+vec[k];
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (TSTFLAG(spec[sq_arrival],opponent))
    {
      square const sq_capture = sq_arrival;
      sq_arrival += vec[k];
      if (e[sq_arrival]==vide)
        empile(sq_departure,sq_arrival,sq_capture);
    }
  }
}

void generate_poseidon(square sq_departure, Side moving)
{
  Side const opponent = advers(moving);
  numvec  k;
  for (k = vec_queen_start; k<=vec_queen_end; ++k)
  {
    square sq_arrival = sq_departure+vec[k];
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (TSTFLAG(spec[sq_arrival],opponent))
    {
      square const sq_capture = sq_arrival;
      sq_arrival += vec[k];
      if (e[sq_arrival]==vide)
        empile(sq_departure,sq_arrival,sq_capture);
    }
  }
}

void generate_marine_pawn(square sq_departure, Side moving)
{
  Side const opponent = advers(moving);
  int dir_vertical;
  unsigned nr_steps;

  if (moving==White)
  {
    dir_vertical = dir_up;

    if (sq_departure<=square_h1)
    {
      if (CondFlag[einstein])
        nr_steps = 3;
      else
        nr_steps = 1;
    }
    else if (sq_departure<=square_h2)
      nr_steps = 2;
    else
      nr_steps = 1;
  }
  else
  {
    dir_vertical = dir_down;

    if (sq_departure>=square_a8)
    {
      if (CondFlag[einstein])
        nr_steps = 3;
      else
        nr_steps = 1;
    }
    else if (sq_departure>=square_a7)
      nr_steps = 2;
    else
      nr_steps = 1;
  }

  gen_p_nocaptures(sq_departure,dir_vertical,nr_steps);

  {
    int const vec_left = dir_vertical+dir_left;
    square const sq_capture = sq_departure+vec_left;
    square const sq_arrival = sq_capture+vec_left;
    if (e[sq_arrival]==vide && TSTFLAG(spec[sq_capture],opponent))
      empile(sq_departure,sq_arrival,sq_capture);
  }

  {
    int const vec_right = dir_vertical+dir_right;
    square const sq_capture = sq_departure+vec_right;
    square const sq_arrival = sq_capture+vec_right;
    if (e[sq_arrival]==vide && TSTFLAG(spec[sq_capture],opponent))
      empile(sq_departure,sq_arrival,sq_capture);
  }
}
