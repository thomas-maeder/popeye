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
  Side traitnbply;

  INCREMENT_COUNTER(empile);

  if (sq_departure==sq_arrival
      && (!CondFlag[schwarzschacher] || sq_arrival != nullsquare))
    return true;

  TraceValue("%u\n",empilegenre);
  if (empilegenre)
  {
    if (TSTFLAG(spec[sq_arrival],Uncapturable))
      return false;

    if (CondFlag[messigny]
        && sq_capture == messigny_exchange
        /* a swapping move */
        && (move_generation_stack[current_move[parent_ply[nbply]]].capture==messigny_exchange)
        /* last move was a swapping one too */
        && (sq_departure == move_generation_stack[current_move[parent_ply[nbply]]].arrival
            || sq_departure == move_generation_stack[current_move[parent_ply[nbply]]].departure
            || sq_arrival == move_generation_stack[current_move[parent_ply[nbply]]].arrival
            || sq_arrival == move_generation_stack[current_move[parent_ply[nbply]]].departure))
      /* No single piece must be involved in
       * two consecutive swappings, so reject move.
       */
    {
      return false;
    }

    if (anymars || anyantimars)
    {
      if (CondFlag[phantom])
      {
        if (flagactive)
        {
          sq_departure = marsid;
          if (sq_departure==sq_arrival)
            return true;
        }
      }
      else
      {
        if (flagpassive!=anyantimars && e[sq_capture]!=vide)
          return true;
        if (flagcapture!=anyantimars && e[sq_capture]==vide)
          return true;
        if (flagcapture)
          sq_departure = marsid;
      }
    }

    traitnbply= trait[nbply];
    TraceEnumerator(Side,traitnbply,"\n");

    if (CondFlag[takemake])
    {
      /* Take & make is implemented as follows: When the take part of
       * a move is attempted to be written into the moves table (ca,
       * cd, cp) by empile(), empile() causes moves to be generated
       * from the arrival square of the take part, but according to
       * the movement of the captured piece. This will cause empile()
       * to be invoked recursively.
       *
       * The actual take&make moves are empile()d in the nested
       * invocation; empile()ing them in the outer invocation would
       * break more or less everything that follows the if
       * (CondFlag[takemake]) clause, including Maximummer.
       *
       * The global variable takemake_takedeparturesquare is used to
       * distinguish between the two cases; if its equal to
       * initsquare, we are generating a non-capturing move or the
       * take part of a capturing move; otherwise, we are generating
       * the make part of a capture where the capturing piece came
       * from takemake_takedeparturesquare.
       */
      if (takemake_takedeparturesquare==initsquare)
      {
        /* We are not generating the make part */

        takemake_takenpiece= e[sq_capture];
        if (takemake_takenpiece == vide) {
          /* Non-capturing move - go on as in orthodox chess */
        }
        else
        {
          /* this is the take part */
          takemake_takenspec= spec[sq_capture];

          /* Save the departure square for signaling the we are now
           * generating the make part and for using the value when we
           * will be recursively invoked during the generation of the
           * make part. */
          takemake_takedeparturesquare= sq_departure;

          /* At first, it may seem that saving ip isn't necessary
           * because the arrival square of the take part is the
           * departure square of the make part. Yet there are
           * situations where capture square and arrival square are
           * different (the raison d'etre of ip, after all) - most
           * notably en passant captures. */
          takemake_takecapturesquare= sq_capture;

          /* needed in m*ummer */
          takemake_takearrivalsquare = sq_arrival;

          /* Execute the take part. The order ip, ia, id avoids losing
           * information and elegantly deals with the case where
           * ip==ia. */
          e[sq_capture]= vide;
          spec[sq_capture]= EmptySpec;

          e[sq_arrival]= e[sq_departure];
          spec[sq_arrival]= spec[sq_departure];

          e[sq_departure]= vide;    /* for sentinelles, need to calculate... */
          spec[sq_departure]= EmptySpec;

          if (traitnbply == White)
            gen_bl_piece_aux(sq_arrival, takemake_takenpiece);
          else
            gen_wh_piece_aux(sq_arrival, takemake_takenpiece);

          /* Take back the take part, reverse order of executing
           * it. */
          e[sq_departure]= e[sq_arrival];
          spec[sq_departure]= spec[sq_arrival];

          e[sq_arrival]= vide;
          spec[sq_arrival]= EmptySpec;

          e[sq_capture]= takemake_takenpiece;
          spec[sq_capture]= takemake_takenspec;

          takemake_takecapturesquare= initsquare;
          takemake_takedeparturesquare= initsquare;

          /* This is the take part - actual moves were generated
           * during the recursive invokation for the make part, so
           * let's bail out.
           */
          return true;
        }
      }
      else
      {
        /* We are generating the make part */
        if (e[sq_capture]==vide)
        {
          /* Extra rule: pawns must not 'make' to their base line */
          if (is_pawn(abs(e[sq_departure]))
              && !CondFlag[normalp]
              && ((traitnbply == White && sq_arrival<=square_h1)
                  || (traitnbply == Black && sq_arrival>=square_a8)))
            return true;

          sq_capture = takemake_takecapturesquare;
          sq_departure = takemake_takedeparturesquare;
        }
        else
          /* We must not capture in the make part */
          return true;
      }
    }

    if (CondFlag[imitators]
        && ((sq_capture==kingside_castling || sq_capture==queenside_castling)
            ? !castlingimok(sq_departure, sq_arrival)
            : !imok(sq_departure, sq_arrival))) {
      return false;
    }
  }

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
  if (e[middle]==vide && rightcolor(e[sq_arrival],camp))
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
        && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp)))
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
    if (e[middle]==vide && rightcolor(e[sq_arrival],camp))
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
          && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp)))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static numecoup testdebut[maxply+1];

static boolean testempile(square sq_departure,
                          square sq_arrival,
                          square sq_capture)
{
  numecoup k;

  if (!TSTFLAG(spec[sq_departure], ColourChange))
    for (k= current_move[nbply]; k > testdebut[nbply]; k--)
      if (move_generation_stack[k].arrival==sq_arrival)
        return true;

  return empile(sq_departure,sq_arrival,sq_capture);
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
        if ((rightcolor(e[sq_arrival],camp) || (e[sq_arrival]==vide))
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

        if (rightcolor(e[sq_arrival],camp)
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
           && testempile(sq_departure,sq_arrival,sq_arrival));

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
  if (rightcolor(e[sq_end],camp))
    testempile(sq_departure,sq_end,sq_end);
}

static void grao(square sq_departure,
                 numvec k1, numvec k2, numvec delta_k,
                 Side camp)
{
  square sq_hurdle= generate_moves_on_circle_segment(sq_departure,sq_departure,
                                                     k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
    square sq_arrival= fin_circle_line(sq_hurdle,k1,&k2,delta_k);
    if (rightcolor(e[sq_arrival],camp))

      testempile(sq_departure,sq_arrival,sq_arrival);
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
    if (rightcolor(e[sq_end],camp))
      testempile(sq_departure,sq_end,sq_end);
  }
}

static void grosehopper(square sq_departure,
                        numvec k1, numvec k2, numvec delta_k,
                        Side camp)
{
  square sq_hurdle= fin_circle_line(sq_departure,k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
    square sq_arrival= sq_hurdle+vec[k1+k2];
    if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
      testempile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void groselocust(square sq_departure,
                        numvec k1, numvec k2, numvec delta_k,
                        Side camp)
{
  square sq_capture= fin_circle_line(sq_departure,k1,&k2,delta_k);
  if (sq_capture!=sq_departure && e[sq_capture]!=obs && rightcolor(e[sq_capture], camp)) {
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
  numvec k, k1;

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

  /* different moves from one piece to the same square are possible.
   * so we have to use testempile !!
   */

  square sq_arrival;

  for (k= kend; k>=kanf; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);

    if (hurdle!=obs) {
      k1= k<<1;

      sq_arrival= sq_hurdle+mixhopdata[m][k1];
      if ((e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
          && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],mixhopdata[m][k1])))
      {
        testempile(sq_departure,sq_arrival,sq_arrival);
        if (TSTFLAG(spec[sq_departure],ColourChange))
          move_generation_stack[current_move[nbply]].hopper_hurdle = sq_hurdle;
      }

      sq_arrival= sq_hurdle+mixhopdata[m][k1-1];
      if ((e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
          && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],mixhopdata[m][k1-1])))
      {
        testempile(sq_departure,sq_arrival,sq_arrival);
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
  if (rightcolor(e[sq_capture],camp)) {
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
      if (rightcolor(hurdle,camp))
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
      if (rightcolor(e[sq_arrival],camp))
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
             || rightcolor(e[sq_arrival],camp))
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
               && rightcolor(at_end_of_line,camp)
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
             || rightcolor(e[sq_arrival],camp))
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
              || rightcolor(e[sq_arrival],camp)))
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
        if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
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
        if (rightcolor(e[sq_arrival],camp))
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
        if (rightcolor(e[sq_arrival],camp))
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
            if (rightcolor(e[sq_arrival],camp))
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
    testempile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k2;
    if (e[sq_arrival]==vide) {
      testempile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= k1;
    }
    else
      break;
  }
  if (rightcolor(e[sq_arrival],camp))
    testempile(sq_departure,sq_arrival,sq_arrival);

  sq_arrival= sq_departure+k1;
  while (e[sq_arrival]==vide) {
    testempile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival-= k2;
    if (e[sq_arrival]==vide) {
      testempile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= k1;
    }
    else
      break;
  }
  if (rightcolor(e[sq_arrival],camp))
    testempile(sq_departure,sq_arrival,sq_arrival);
}

static void gcsp(square sq_departure,
         numvec k1, numvec k2,
         Side camp)
{
  square sq_arrival= sq_departure+vec[k1];

  while (e[sq_arrival]==vide) {
    testempile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= vec[k2];
    if (e[sq_arrival]==vide) {
      testempile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= vec[k1];
    }
    else
      break;
  }
  if (rightcolor(e[sq_arrival],camp))
    testempile(sq_departure,sq_arrival,sq_arrival);
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
    else if (rightcolor(e_ubi[sq_arrival],camp))
      empile(sq_departure,sq_arrival,sq_arrival);
    e_ubi[sq_arrival]= obs;
  }
}

typedef boolean generatorfunction_t(square, square, square);

static void grfou(square   orig_departure,
           square   in,
           numvec   k,
           int x,
           Side  camp,
           generatorfunction_t *generate)
{
  /* ATTENTION:
     if first call of x is 1 and boolnoedge[i]
     use empile() for generate,
     else
     use testempile() for generate !!
  */
  int k1;

  square sq_departure= orig_departure;
  square sq_arrival= in+k;

  if (e[sq_arrival]==obs)
    return;

  while (e[sq_arrival]==vide) {
    (*generate)(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }

  if (rightcolor(e[sq_arrival],camp))
    (*generate)(sq_departure,sq_arrival,sq_arrival);
  else if (x && e[sq_arrival]==obs) {
    sq_arrival-= k;
    k1= 5;
    while (vec[k1]!=k)
      k1++;
    k1*= 2;
    grfou(orig_departure,sq_arrival,mixhopdata[1][k1],x-1,camp,generate);
    k1--;
    grfou(orig_departure,sq_arrival,mixhopdata[1][k1],x-1,camp,generate);
  }
}

static void gcard(square   orig_departure,
                  square   in,
                  numvec   k,
                  int x,
                  Side  camp,
                  generatorfunction_t *generate)
{
  /* ATTENTION:
     if first call of x is 1
     use    empile() for generate,
     else
     use testempile() for generate !!
  */
  int k1;

  square sq_departure= orig_departure;
  square sq_arrival= in+k;

  while (e[sq_arrival]==vide) {
    (*generate)(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }
  if (rightcolor(e[sq_arrival],camp))
    (*generate)(sq_departure,sq_arrival,sq_arrival);
  else if (x && e[sq_arrival]==obs) {
    for (k1= 1; k1<=4; k1++) {
      if (e[sq_arrival+vec[k1]]!=obs) {
        break;
      }
    }
    if (k1<=4) {
      sq_arrival+= vec[k1];
      if (rightcolor(e[sq_arrival],camp)) {
        (*generate)(sq_departure,sq_arrival,sq_arrival);
      }
      else if (e[sq_arrival]==vide) {
        (*generate)(sq_departure,sq_arrival,sq_arrival);
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (e[sq_arrival+mixhopdata[1][k1]]==obs)
          k1--;

        gcard(orig_departure,
              sq_arrival,
              mixhopdata[1][k1],
              x-1,
              camp,
              generate);
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

  square sq_departure= orig_departure;
  square sq_arrival;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    if (x) {
      sq_arrival= step_departure+vec[k];
      if (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        if (!NoEdge(sq_arrival))
          grefc(orig_departure,sq_arrival,x-1,camp);
      }
      else if (rightcolor(e[sq_arrival], camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
    else
      for (k= vec_knight_start; k<=vec_knight_end; k++) {
        sq_arrival= step_departure+vec[k];
        if (e[sq_arrival]==vide
            || rightcolor(e[sq_arrival],camp))
          testempile(sq_departure,sq_arrival,sq_arrival);
      }
  }
} /* grefc */

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

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_arrival= step_departure;

    while (e[sq_arrival+=vec[k]]==vide)
    {
      testempile(sq_departure,sq_arrival,sq_arrival);
      if (!NoEdge(sq_arrival) &&
          !traversed(sq_arrival)) {
        grefn(orig_departure,sq_arrival,camp);
        break;
      }
    }
    if (rightcolor(e[sq_arrival], camp))
      testempile(sq_departure,sq_arrival,sq_arrival);
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
          if (rightcolor(hurdle,camp)
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
        && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
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
          && rightcolor(hurdle1,camp)) {
        sq_arrival= sq_hurdle1;
        empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {     /* 2,4; 2,6; 4,6; */
    sq_arrival= sq_departure + vec[k];
    sq_hurdle1= sq_departure+2*vec[k];
    if (abs(e[sq_hurdle1])>=roib
        && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp)))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void gcat(square sq_departure, Side camp) {
  /* generate moves of a CAT */
  numvec  k;

  square sq_arrival;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_arrival= sq_departure+vec[k];
    if (rightcolor(e[sq_arrival], camp))
      empile(sq_departure,sq_arrival,sq_arrival);
    else {
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= mixhopdata[3][k];
      }

      if (rightcolor(e[sq_arrival], camp))
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
    if (e[arrival1]==vide || rightcolor(e[arrival1],camp))
      if (maooaimcheck(sq_departure,arrival1,pass))
        empile(sq_departure,arrival1,arrival1);

    if (e[arrival2]==vide || rightcolor(e[arrival2],camp))
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

/* Remove duplicate moves at the top of the move_generation_stack.
 * @param start start position of range where to look for duplicates
 */
void remove_duplicate_moves(numecoup start)
{
  numecoup l1;
  for (l1 = start+1; l1<=current_move[nbply]; ++l1)
  {
    numecoup l2 = l1+1;
    while (l2<=current_move[nbply])
      if (move_generation_stack[l1].arrival==move_generation_stack[l2].arrival)
      {
        move_generation_stack[l2] = move_generation_stack[current_move[nbply]];
        --current_move[nbply];
      }
      else
        ++l2;
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
            if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
    }
  }

  remove_duplicate_moves(save_nbcou);
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

  switch  (abs(p)) {
  case maob:
    gmao(sq_departure, camp);
    return;

  case paob:
    gchin(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case leob:
    gchin(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case vaob:
    gchin(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;
  case naob:
    gchin(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case roseb:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grose(sq_departure, k, 0,+1, camp);
      grose(sq_departure, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case nequib:
    gnequi(sq_departure, camp);
    return;

  case locb:
    glocust(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case nightlocustb:
    glocust(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case bishoplocustb:
    glocust(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case rooklocustb:
    glocust(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case kangoub:
    gkang(sq_departure, camp);
    return;

  case kanglionb:
    gkanglion(sq_departure, camp);
    return;

  case kaob:
    gchinleap(sq_departure, vec_knight_start, vec_knight_end, camp);
    return;

  case knighthopperb:
    geshop(sq_departure, vec_knight_start, vec_knight_end, camp);
    return;

  case csb:
    for (k= vec_knight_start; k<=vec_knight_end; k++)
      gcs(sq_departure, vec[k], vec[25 - k], camp);
    return;

  case hamstb:
    ghamst(sq_departure);
    return;

  case ubib:
    for (bnp= boardnum; *bnp; bnp++)
      e_ubi[*bnp]= e[*bnp];
    gubi(sq_departure, sq_departure, camp);
    return;

  case mooseb:
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 0, camp);
    return;

  case eagleb:
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 1, camp);
    return;

  case sparrb:
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 2, camp);
    return;

  case margueriteb:
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 0, camp);
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 1, camp);
    gmhop(sq_departure, vec_queen_start,vec_queen_end, 2, camp);
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    ghamst(sq_departure);
    return;

  case archb:
    if (NoEdge(sq_departure)) {
      for (k= vec_bishop_start; k <= vec_bishop_end; k++)
        grfou(sq_departure, sq_departure, vec[k], 1, camp, empile);
    }
    else {
      for (k= vec_bishop_start; k <= vec_bishop_end; k++)
        grfou(sq_departure, sq_departure, vec[k], 1, camp, testempile);
    }
    return;

  case reffoub:
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      grfou(sq_departure, sq_departure, vec[k], 4, camp, testempile);
    return;

  case cardb:
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      gcard(sq_departure, sq_departure, vec[k], 1, camp, empile);
    return;

  case dcsb:
    for (k= 9; k <= 14; k++)
      gcs(sq_departure, vec[k], vec[23 - k], camp);
    for (k= 15; k <= 16; k++)
      gcs(sq_departure, vec[k], vec[27 - k], camp);
    return;

  case refcb:
    grefc(sq_departure, sq_departure, 2, camp);
    return;

  case refnb:
    clearedgestraversed();
    grefn(sq_departure, sq_departure, camp);
    return;

  case equib:
    gequi(sq_departure, camp);
    return;

  case catb:
    gcat(sq_departure, camp);
    return;

  case sireneb:
    gmarin(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case tritonb:
    gmarin(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case nereidb:
    gmarin(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case orphanb:
    gorph(sq_departure, camp);
    return;

  case friendb:
    gfriend(sq_departure, camp);
    return;

  case edgehb:
    gedgeh(sq_departure, camp);
    return;

  case moab:
    gmoa(sq_departure, camp);
    return;

  case moaridb:
    gemoarider(sq_departure, camp);
    return;

  case maoridb:
    gemaorider(sq_departure, camp);
    return;

  case bscoutb:
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      gcs(sq_departure, vec[k], vec[13 - k], camp);
    return;

  case gscoutb:
    for (k= vec_rook_end; k >=vec_rook_start; k--)
      gcs(sq_departure, vec[k], vec[5 - k], camp);
    return;

  case skyllab:
    geskylla(sq_departure, camp);
    return;

  case charybdisb:
    gecharybdis(sq_departure, camp);
    return;

  case sb:
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case lionb:
    gelrhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case nsautb:
    gerhop(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case camhopb:
    gerhop(sq_departure, vec_chameau_start,vec_chameau_end, camp);
    return;

  case zebhopb:
    gerhop(sq_departure, vec_zebre_start,vec_zebre_end, camp);
    return;

  case gnuhopb:
    gerhop(sq_departure, vec_chameau_start,vec_chameau_end, camp);
    gerhop(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case tlionb:
    gelrhop(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case flionb:
    gelrhop(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;

  case rookhopb:
    gerhop(sq_departure, vec_rook_start,vec_rook_end, camp);
    return;

  case bishophopb:
    gerhop(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    return;
  case contragrasb:
    gecrhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case roselionb:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      groselion(sq_departure, k, 0,+1, camp);
      groselion(sq_departure, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case rosehopperb:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grosehopper(sq_departure, k, 0,+1, camp);
      grosehopper(sq_departure, k,
                  vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case roselocustb:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      groselocust(sq_departure, k, 0,+1, camp);
      groselocust(sq_departure, k,
                  vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case g2b:
    gerhop2(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case g3b:
    gerhop3(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;
  case khb:
    geshop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case doublegb:
    gdoublegrasshopper(sq_departure, camp);
    return;

  case doublerookhopperb:
    gdoublerookhopper(sq_departure, camp);
    return;

  case doublebishopperb:
    gdoublebishopper(sq_departure, camp);
    return;

  case orixb:
    gorix(sq_departure, camp);
    return;

   case norixb:
    gnorix(sq_departure, camp);
    return;

  case gralb:
    if (camp==White)
      gebleap(sq_departure, vec_alfil_start,vec_alfil_end);    /* alfilb */
    else
      genleap(sq_departure, vec_alfil_start,vec_alfil_end);    /* alfiln */
    gerhop(sq_departure, vec_rook_start,vec_rook_end, camp);      /* rookhopper */
    return;

  case rookmooseb:
    gmhop(sq_departure, vec_rook_start,vec_rook_end, 0, camp);
    return;

  case rookeagleb:
    gmhop(sq_departure, vec_rook_start,vec_rook_end, 1, camp);
    return;

  case rooksparrb:
    gmhop(sq_departure, vec_rook_start,vec_rook_end, 2, camp);
    return;

  case bishopmooseb:
    gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 0, camp);
    return;

  case bishopeagleb:
    gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 1, camp);
    return;

  case bishopsparrb:
    gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 2, camp);
    return;

  case raob:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grao(sq_departure, k, 0,+1, camp);
      grao(sq_departure, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case scorpionb:
    if (camp==White)
      gebleap(sq_departure, vec_queen_start,vec_queen_end);        /* ekingb */
    else
      genleap(sq_departure, vec_queen_start,vec_queen_end);        /* ekingn */
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);     /* grashopper */
    return;

  case nrlionb:
    gelrhop(sq_departure, vec_knight_start,vec_knight_end, camp);
    return;

  case mrlionb:
    gemaoriderlion(sq_departure, camp);
    return;

  case molionb:
    gemoariderlion(sq_departure, camp);
    return;

  case dolphinb:
    gkang(sq_departure, camp);
    gerhop(sq_departure, vec_queen_start,vec_queen_end, camp);
    return;

  case rabbitb:
    grabbit(sq_departure, camp);
    return;

  case bobb:
    gbob(sq_departure, camp);
    return;

  case equiengb:
    gequiapp(sq_departure, camp);
    return;

  case equifrab:
    gnequiapp(sq_departure, camp);
    return;

  case querqub:
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

  case bouncerb :
    genbouncer(sq_departure, vec_queen_start,vec_queen_end, camp);
    break;

  case rookbouncerb :
    genbouncer(sq_departure, vec_rook_start,vec_rook_end, camp);
    break;

  case bishopbouncerb :
    genbouncer(sq_departure, vec_bishop_start,vec_bishop_end, camp);
    break;

  case radialknightb :
    genradialknight(sq_departure, camp);
    break;

  case treehopperb :
    gentreehopper(sq_departure, camp);
    break;

  case leafhopperb :
    genleafhopper(sq_departure, camp);
    break;

  case greatertreehopperb :
    gengreatertreehopper(sq_departure, camp);
    break;

  case greaterleafhopperb :
    gengreaterleafhopper(sq_departure, camp);
    break;

  case cs40b:
    gcsp(sq_departure, 9, 16, camp);
    gcsp(sq_departure, 10, 11, camp);
    gcsp(sq_departure, 11, 10, camp);
    gcsp(sq_departure, 12, 13, camp);
    gcsp(sq_departure, 13, 12, camp);
    gcsp(sq_departure, 14, 15, camp);
    gcsp(sq_departure, 15, 14, camp);
    gcsp(sq_departure, 16, 9, camp);
    break;

  case cs20b:
    gcsp(sq_departure, 9, 12, camp);
    gcsp(sq_departure, 10, 15, camp);
    gcsp(sq_departure, 11, 14, camp);
    gcsp(sq_departure, 12, 9, camp);
    gcsp(sq_departure, 13, 16, camp);
    gcsp(sq_departure, 14, 11, camp);
    gcsp(sq_departure, 15, 10, camp);
    gcsp(sq_departure, 16, 13, camp);
    break;

  case cs33b:
    gcsp(sq_departure, 9, 10, camp);
    gcsp(sq_departure, 10, 9, camp);
    gcsp(sq_departure, 11, 12, camp);
    gcsp(sq_departure, 12, 11, camp);
    gcsp(sq_departure, 13, 14, camp);
    gcsp(sq_departure, 14, 13, camp);
    gcsp(sq_departure, 15, 16, camp);
    gcsp(sq_departure, 16, 15, camp);
    break;

  case cs11b:
    gcsp(sq_departure, 9, 14, camp);
    gcsp(sq_departure, 10, 13, camp);
    gcsp(sq_departure, 11, 16, camp);
    gcsp(sq_departure, 12, 15, camp);
    gcsp(sq_departure, 13, 10, camp);
    gcsp(sq_departure, 14, 9, camp);
    gcsp(sq_departure, 15, 12, camp);
    gcsp(sq_departure, 16, 11, camp);
    break;

  case cs31b:
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
  if (rightcolor(hurdle,camp))
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
  testdebut[nbply]= current_move[nbply];
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
        {
          /* double or single step? */
          gen_p_nocaptures(i,+dir_up, i<=square_h2 ? 2 : 1);
        }
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
  testdebut[nbply]= current_move[nbply];
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
            && (*checkfunctions[*ptrans])(sq_departure,ptrans_black,eval_white))
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
    /* testempile nicht nutzbar */
    /* VERIFY: has anf always a propper value??
     */
    remove_duplicate_moves(save_nbcou);

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
    int i,j;
    piece p;
    square z= square_a1;
    for (i= nr_rows_on_board; i > 0; i--, z+= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z++) {
      if ((p = e[z]) != vide) {
      if (TSTFLAG(spec[z], Neutral))
        p = -p;
      if (p > obs && !is_pawn(abs(p))) /* not sure if "castling" with Ps forbidden */
        empile(sq_departure,z,platzwechsel_rochade);
      }
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

void gen_wh_piece_aux(square z, piece p) {

  TraceFunctionEntry(__func__);
  TraceSquare(z);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (CondFlag[annan]) {
    piece annan_p= e[z-onerow];
    if (whannan(z-onerow, z))
      p= annan_p;
  }

  switch(p) {
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

static void orig_gen_wh_piece(square sq_departure, piece p) {
  piece pp;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (CondFlag[madras] && !madrasi_can_piece_move(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }

  if (CondFlag[eiffel] && !eiffel_can_piece_move(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }

  if (CondFlag[disparate] && !disparate_can_piece_move(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }

  if (TSTFLAG(PieSpExFlags,Paralyse) && paralysiert(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }

  if (CondFlag[ultrapatrouille] && !patrol_is_supported(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }

  if (CondFlag[central] && !central_is_supported(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }

  if (TSTFLAG(spec[sq_departure],Beamtet) && !beamten_is_observed(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }

  if (anymars||anyantimars) {
    square mren;
    Flags psp;

    if (CondFlag[phantom]) {
      numecoup const anf1 = current_move[nbply];
      numecoup l1;
      /* generate standard moves first */
      flagactive= false;
      flagpassive= false;
      flagcapture= false;

      gen_wh_piece_aux(sq_departure,p);

      /* Kings normally don't move from their rebirth-square */
      if (p == e[king_square[White]] && !rex_phan)
      {
        TraceFunctionExit(__func__);
        TraceFunctionResultEnd();
        return;
      }
      /* generate moves from rebirth square */
      flagactive= true;
      psp= spec[sq_departure];
      mren= (*marsrenai)(p,psp,sq_departure,initsquare,initsquare,Black);
      /* if rebirth square is where the piece stands,
         we've already generated all the relevant moves.
      */
      if (mren==sq_departure)
      {
        TraceFunctionExit(__func__);
        TraceFunctionResultEnd();
        return;
      }
      if (e[mren] == vide) {
        numecoup const anf2 = current_move[nbply];
        pp=e[sq_departure];      /* Mars/Neutral bug */
        e[sq_departure]= vide;
        spec[sq_departure]= EmptySpec;
        spec[mren]= psp;
        e[mren]= p;
        marsid= sq_departure;

        gen_wh_piece_aux(mren, p);

        e[mren]= vide;
        spec[sq_departure]= psp;
        e[sq_departure]= pp;
        flagactive= false;
        /* Unfortunately we have to check for
           duplicate generated moves now.
           there's only ONE duplicate per arrival square
           possible !

           TODO: avoid entries with arrival==initsquare by moving
           the non-duplicate entries forward and reducing current_move[nbply]
        */
        for (l1= anf1 + 1; l1 <= anf2; l1++)
        {
          numecoup l2= anf2 + 1;
          while (l2 <= current_move[nbply])
            if (move_generation_stack[l1].arrival
                ==move_generation_stack[l2].arrival)
            {
              move_generation_stack[l2] = move_generation_stack[current_move[nbply]];
              --current_move[nbply];
              break;  /* remember: ONE duplicate ! */
            }
            else
              l2++;
        }
      }
    }
    else {
      /* generate noncapturing moves first */
      flagpassive= true;
      flagcapture= false;

      gen_wh_piece_aux(sq_departure, p);

      /* generate capturing moves now */
      flagpassive= false;
      flagcapture= true;
      mars_circe_rebirth_state = 0;
      do {    /* Echecs Plus */
        psp=spec[sq_departure];
        mren= (*marsrenai)(p,psp,sq_departure,initsquare,initsquare,Black);
        if (mren==sq_departure || e[mren]==vide) {
          pp= e[sq_departure];      /* Mars/Neutral bug */
          e[sq_departure]= vide;
          spec[sq_departure]= EmptySpec;
          spec[mren]= psp;
          e[mren]= p;
          marsid= sq_departure;

          gen_wh_piece_aux(mren,p);

          e[mren]= vide;
          spec[sq_departure]= psp;
          e[sq_departure]= pp;
        }
      } while (mars_circe_rebirth_state);
      flagcapture= false;
    }
  }
  else
    gen_wh_piece_aux(sq_departure,p);

  if (CondFlag[messigny] && !(king_square[White]==sq_departure && rex_mess_ex))
  {
    square const *bnp;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-p)
        empile(sq_departure,*bnp,messigny_exchange);
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
        if (ooorphancheck(i,-*porph,orphann,eval_white))
          gen_wh_piece(i,*porph);
      }
      else
      {
        if (ooorphancheck(i,*porph,orphanb,eval_black))
          gen_bl_piece(i,-*porph);
      }
    }

  remove_duplicate_moves(save_nbcou);
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
        if (fffriendcheck(i,*pfr,friendb,eval_white))
          gen_wh_piece(i, *pfr);
      }
      else
      {
        if (fffriendcheck(i,-*pfr,friendn,eval_black))
          gen_bl_piece(i, -*pfr);
      }
    }

  remove_duplicate_moves(save_nbcou);
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

    if (rightcolor(e[sq_arrival],camp))
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void geskylchar(square sq_departure, square sq_arrival, square sq_capture,
                Side camp) {
  if (e[sq_arrival] == vide) {
    if (e[sq_capture]==vide)
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (rightcolor(e[sq_capture],camp))
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
static void gen_p_captures(square sq_departure, square sq_arrival, Side camp) {
  /* generates move of a pawn of colour camp on departure capturing a
     piece on arrival
  */
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceEnumerator(Side,camp,"");
  TraceFunctionParamListEnd();

  if (rightcolor(e[sq_arrival],camp))
    /* normal capture */
    empile(sq_departure,sq_arrival,sq_arrival);
  else {
    /* ep capture */
    if ((abs(e[sq_departure])!=Orphan)   /* orphans cannot capture ep */
        && (sq_arrival==ep[parent_ply[nbply]] || sq_arrival==ep2[nbply - 1])
        /* a pawn has just done a critical move */
        && trait[parent_ply[nbply]]!=camp)     /* the opp. moved before */
    {
      square prev_arrival;

      if (nbply==2) {    /* ep.-key  standard pawn */
        if (camp==White)
          move_generation_stack[current_move[1]].arrival= sq_arrival+dir_down;
        else
          move_generation_stack[current_move[1]].arrival= sq_arrival+dir_up;
      }

      prev_arrival= move_generation_stack[current_move[parent_ply[nbply]]].arrival;
      if (rightcolor(e[prev_arrival],camp))
        /* the pawn has the right color */
        empile(sq_departure,sq_arrival,prev_arrival);
    }
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
        || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, White);
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, White);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_up, CondFlag[einstein] ? 3 : 1);
    }
  }
  else
  {
    /* not first rank */
    if (CondFlag[singlebox] && SingleBoxType==singlebox_type1
        && PromSq(White,sq_departure+dir_up)
        && next_singlebox_prom(Empty,White)==Empty)
    {
      /* nothing */
    }
    else
    {
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, White);
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, White);
      /* double or single step? */
      gen_p_nocaptures(sq_departure,+dir_up, sq_departure<=square_h2 ? 2 : 1);
    }
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
        || abs(e[sq_departure])==orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, Black);
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, Black);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,dir_down, CondFlag[einstein] ? 3 : 1);
    }
  }
  else
  {
    /* not last rank */
    if (CondFlag[singlebox] && SingleBoxType==singlebox_type1
        && PromSq(Black,sq_departure+dir_down)
        && next_singlebox_prom(Empty,Black)==Empty)
    {
      /* nothing */
    }
    else
    {
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, Black);
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, Black);
      /* double or single step? */
      gen_p_nocaptures(sq_departure,dir_down, sq_departure>=square_a7 ? 2 : 1);
    }
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

  for (k= kanf; k <= kend; k++) {
    sq_hurdle= sq_departure + vec[k];
    if (hurdletype == 0 && rightcolor(e[sq_hurdle], camp) ||
        hurdletype == 1 && abs(e[sq_hurdle]) >= roib)
    {
      for (k1= kanf; k1 <= kend; k1++) {
        sq_arrival = (leaf ? sq_departure : sq_hurdle) + vec[k1];
        if ((sq_arrival != sq_hurdle) && (e[sq_arrival] == vide || rightcolor(e[sq_arrival], camp)))
          testempile(sq_departure, sq_arrival, sq_arrival);
      }
    }
  }
}

void genqlinesradial(square sq_departure, Side camp, int hurdletype, boolean leaf)
{
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
}

void genradial(square sq_departure, Side camp, int hurdletype, boolean leaf)
{
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
