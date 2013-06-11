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
#include "conditions/singlebox/type2.h"
#include "conditions/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/pawns/pawns.h"
#include "pieces/pawns/pawn.h"
#include "pieces/hunters.h"
#include "pieces/roses.h"
#include "pieces/spiral_springers.h"
#include "pieces/marine.h"
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
    switch (get_walk_of_piece_on_square(sq_departure)) {

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

int len_capt(square sq_departure, square sq_arrival, square sq_capture)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  return move_effect_journal[capture].type==move_effect_piece_removal;
}

int len_follow(square sq_departure, square sq_arrival, square sq_capture)
{
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
    return sq_arrival==move_effect_journal[movement].u.piece_movement.from;
  else
    return true;
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

int len_synchron(square sq_departure, square sq_arrival, square sq_capture)
{
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
  {
    square const sq_parent_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
    numvec const diff = sq_departure-sq_arrival;
    return diff==parent_diff;
  }
  else
    return true;
}

int len_antisynchron(square sq_departure, square sq_arrival, square sq_capture)
{
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
  {
    square const sq_parent_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
    numvec const diff = sq_departure-sq_arrival;
    return diff==-parent_diff;
  }
  else
    return true;
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
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  return move_effect_journal[capture].type==move_effect_piece_removal;
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
  move_generation_stack[current_move[nbply]].auxiliary = initsquare;

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
  while (is_square_empty(middle)
         && is_square_empty(sq_arrival)
         && empile(sq_departure,sq_arrival,sq_arrival)) {
    middle += todest;
    sq_arrival += todest;
  }
  if (is_square_empty(middle) && piece_belongs_to_opponent(sq_arrival,camp))
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

  while (is_square_empty(middle) && is_square_empty(sq_arrival)) {
    middle += todest;
    sq_arrival += todest;
  }
  if (e[middle] != obs && e[sq_arrival] != obs) {
    if (!is_square_empty(middle)
        && (is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,camp)))
      empile(sq_departure,sq_arrival,sq_arrival);
    if (is_square_empty(middle) || is_square_empty(sq_arrival)) {
      middle += todest;
      sq_arrival += todest;
      while (is_square_empty(middle) && is_square_empty(sq_arrival)
             && empile(sq_departure,sq_arrival,sq_arrival)) {
        middle += todest;
        sq_arrival += todest;
      }
    }
    if (is_square_empty(middle) && piece_belongs_to_opponent(sq_arrival,camp))
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

void leaper_generate_moves(Side side, square sq_departure,
                           vec_index_type kbeg, vec_index_type kend)
{
  /* generate leaper moves from vec[kbeg] to vec[kend] */
  vec_index_type k;

  for (k= kbeg; k<= kend; ++k)
  {
    square const sq_arrival = sq_departure+vec[k];
    if (is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,side))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

square generate_moves_on_line_segment(square sq_departure,
                                      square sq_base,
                                      vec_index_type k)
{
  square sq_arrival = sq_base+vec[k];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_base);
  TraceFunctionParamListEnd();

  while (is_square_empty(sq_arrival) && empile(sq_departure,sq_arrival,sq_arrival))
    sq_arrival += vec[k];

  TraceFunctionExit(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionResultEnd();
  return sq_arrival;
}

void rider_generate_moves(Side side, square sq_departure,
                          vec_index_type kbeg, vec_index_type kend)
{
  /* generate rider moves from vec[kbeg] to vec[kend] */
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k = kbeg; k<=kend; ++k)
  {
    square const sq_arrival = generate_moves_on_line_segment(sq_departure,sq_departure,k);
    if (piece_belongs_to_opponent(sq_arrival,side))
      empile(sq_departure,sq_arrival,sq_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void bouncer_generate_moves(Side side,
                                   square sq_departure,
                                   vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type  k;
  for (k = kend; k>=kbeg; k--)
  {
    piece   p1;
    square  bounce_where;
    finligne(sq_departure,vec[k],p1,bounce_where);

    {
      square const bounce_to = 2*sq_departure-bounce_where;

      square sq_arrival = sq_departure-vec[k];
      while (sq_arrival!=bounce_to && is_square_empty(sq_arrival))
        sq_arrival -= vec[k];

      if (sq_arrival==bounce_to
          && (is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,side)))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

void geriderhopper(square sq_departure,
                   vec_index_type kbeg, vec_index_type kend,
                   int run_up,
                   int jump,
                   Side  camp)
{
  /* generate rider-hopper moves from vec[kbeg] to vec[kend] */

  piece   hurdle;
  square  sq_hurdle;

  square sq_arrival;

  vec_index_type k;
  for (k= kbeg; k <= kend; k++)
  {
    if (run_up)
    {
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
          if (!is_square_empty(sq_hurdle))
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
            if (!is_square_empty(sq_arrival))
              break;
          }
          if (jumped) {
            /* there is an obstacle -> next line */
            continue;
          }
        }
        sq_arrival+= vec[k];
        if ((piece_belongs_to_opponent(sq_arrival,camp) || (is_square_empty(sq_arrival)))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }
      else {
        /* jump of flexible length */
        /* lion, contragrashopper type */
        sq_arrival+= vec[k];
        while (is_square_empty(sq_arrival)) {
          if (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k]))
          {
            empile(sq_departure,sq_arrival,sq_arrival);
            move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
          }
          sq_arrival+= vec[k];
        }

        if (piece_belongs_to_opponent(sq_arrival,camp)
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }
    }
  }
}

static void ghamst(square sq_departure)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece hurdle;
    square sq_arrival;
    finligne(sq_departure,vec[k],hurdle,sq_arrival);
    if (hurdle!=obs)
    {
      sq_arrival-= vec[k];
      if (sq_arrival!=sq_departure)
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static void gmhop(square   sq_departure,
                  vec_index_type kanf, vec_index_type kend,
                  int m,
                  Side camp)
{
  vec_index_type k;

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

  for (k = kend; k>=kanf; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);

    if (hurdle!=obs)
    {
      vec_index_type const k1 = 2*k;

      {
        square const sq_arrival= sq_hurdle+mixhopdata[m][k1];
        if ((is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,camp))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],mixhopdata[m][k1])))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }

      {
        square const sq_arrival= sq_hurdle+mixhopdata[m][k1-1];
        if ((is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,camp))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],mixhopdata[m][k1-1])))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }
    }
  }
}

void generate_locust_capture(square sq_departure, square sq_capture,
                             vec_index_type k,
                             Side camp)
{
  if (piece_belongs_to_opponent(sq_capture,camp))
  {
    square const sq_arrival= sq_capture+vec[k];
    if (is_square_empty(sq_arrival))
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

static void glocust(square sq_departure,
                    vec_index_type kbeg, vec_index_type kend,
                    Side    camp)
{
  vec_index_type k;
  for (k= kbeg; k <= kend; k++)
  {
    piece hurdle;
    square sq_capture;
    finligne(sq_departure,vec[k],hurdle,sq_capture);
    generate_locust_capture(sq_departure,sq_capture,k,camp);
  }
} /* glocust */

static void gchin(square sq_departure,
                  vec_index_type kbeg, vec_index_type kend,
                  Side camp)
{
  /* generate chinese-rider moves from vec[kbeg] to vec[kend] */

  vec_index_type k;

  for (k= kbeg; k<=kend; k++)
  {
    square sq_arrival = generate_moves_on_line_segment(sq_departure,sq_departure,k);

    if (e[sq_arrival]!=obs)
    {
      piece   hurdle;
      finligne(sq_arrival,vec[k],hurdle,sq_arrival);
      if (piece_belongs_to_opponent(sq_arrival,camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static void gchinleap(square sq_departure,
                      vec_index_type kbeg, vec_index_type kend,
                      Side camp)
{
  /* generate chinese-leaper moves from vec[kbeg] to vec[kend] */

  vec_index_type k;

  for (k= kbeg; k<=kend; k++)
  {
    square sq_arrival= sq_departure + vec[k];

    if (get_walk_of_piece_on_square(sq_arrival)>=King)
    {
      sq_arrival += vec[k];
      if (piece_belongs_to_opponent(sq_arrival,camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
    else if (is_square_empty(sq_arrival))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void gnequi(square sq_departure, Side camp)
{
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

      if (!is_square_empty(sq_hurdle)) {

        vector= sq_hurdle-sq_departure;
        sq_arrival= sq_hurdle+vector;

        if ((is_square_empty(sq_arrival)
             || piece_belongs_to_opponent(sq_arrival,camp))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vector,vector)))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }
    }
}

static void gorix(square sq_departure, Side camp)
{
  /* Orix */
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs)
    {
      piece at_end_of_line;
      square end_of_line;
      finligne(sq_hurdle,vec[k],at_end_of_line,end_of_line);
      {
        square const sq_arrival = sq_hurdle+sq_hurdle-sq_departure;
        if (abs(end_of_line-sq_hurdle) > abs(sq_hurdle-sq_departure)
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
        else if (abs(end_of_line-sq_hurdle) == abs(sq_hurdle-sq_departure)
                 && piece_belongs_to_opponent(end_of_line,camp)
                 && (!checkhopim || hopimok(sq_departure,end_of_line,sq_hurdle,vec[k],vec[k])))
        {
          square const sq_arrival= end_of_line;
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }
    }
  }
}

static void gnorix(square sq_departure, Side camp)
{
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

      if (queenlike && !is_square_empty(sq_hurdle)) {

        vector= sq_hurdle-sq_departure;
        sq_arrival= sq_hurdle+vector;

        if ((is_square_empty(sq_arrival)
             || piece_belongs_to_opponent(sq_arrival,camp))
            && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vector,vector)))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
        }
      }
    }
}

static void gnequiapp(square sq_departure, Side camp)
{
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
          && !is_square_empty(sq_hurdle)
          && (is_square_empty(sq_arrival)
              || piece_belongs_to_opponent(sq_arrival,camp)))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
}

static void gkang(square sq_departure, Side camp)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs)
    {
      square sq_arrival;
      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs)
      {
        sq_arrival+= vec[k];
        if (is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void gkanglion(square sq_departure, Side camp)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs)
    {
      square sq_arrival;
      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs)
      {
        sq_arrival += vec[k];
        while (is_square_empty(sq_arrival))
        {
          empile(sq_departure,sq_arrival,sq_arrival);
          sq_arrival += vec[k];
        }
        if (piece_belongs_to_opponent(sq_arrival,camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void grabbit(square sq_departure, Side camp)
{
  vec_index_type k;

  for (k= vec_queen_end; k >=vec_queen_start; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs)
    {
      square sq_arrival;
      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs)
      {
        sq_arrival= generate_moves_on_line_segment(sq_departure,sq_arrival,k);
        if (piece_belongs_to_opponent(sq_arrival,camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

static void gbob(square sq_departure, Side camp) {
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs)
    {
      finligne(sq_hurdle,vec[k],hurdle,sq_hurdle);
      if (hurdle!=obs)
      {
        finligne(sq_hurdle,vec[k],hurdle,sq_hurdle);
        if (hurdle!=obs)
        {
          square sq_arrival;
          finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
          if (hurdle!=obs)
          {
            sq_arrival= generate_moves_on_line_segment(sq_departure,
                                                       sq_arrival,
                                                       k);
            if (piece_belongs_to_opponent(sq_arrival,camp))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
}

static void gcsp(square sq_departure,
                 numvec k1, numvec k2,
                 Side camp)
{
  square sq_arrival= sq_departure+vec[k1];

  while (is_square_empty(sq_arrival)) {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= vec[k2];
    if (is_square_empty(sq_arrival)) {
      empile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= vec[k1];
    }
    else
      break;
  }
  if (piece_belongs_to_opponent(sq_arrival,camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

static void gubi(square orig_departure,
                 square step_departure,
                 Side camp)
{
  vec_index_type k;

  square sq_departure = orig_departure;

  e_ubi[step_departure]= obs;

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    square const sq_arrival = step_departure+vec[k];
    if (e_ubi[sq_arrival]==vide)
    {
      empile(sq_departure,sq_arrival,sq_arrival);
      gubi(orig_departure,sq_arrival,camp);
    }
    else if (camp==White ? e_ubi[sq_arrival]<=roin : e_ubi[sq_arrival]>=roib)
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

  while (is_square_empty(sq_arrival))
  {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }

  if (piece_belongs_to_opponent(sq_arrival,camp))
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

  while (is_square_empty(sq_arrival))
  {
    empile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }

  if (piece_belongs_to_opponent(sq_arrival,camp))
    empile(sq_departure,sq_arrival,sq_arrival);
  else if (x && e[sq_arrival]==obs)
  {
    for (k1= 1; k1<=4; k1++)
      if (e[sq_arrival+vec[k1]]!=obs)
        break;

    if (k1<=4)
    {
      sq_arrival+= vec[k1];
      if (piece_belongs_to_opponent(sq_arrival,camp))
        empile(sq_departure,sq_arrival,sq_arrival);
      else if (is_square_empty(sq_arrival))
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
  vec_index_type k;

  /* ATTENTION:   first call of grefcn: x must be 2 !!   */

  square sq_departure = orig_departure;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    square const sq_arrival = step_departure+vec[k];
    if (is_square_empty(sq_arrival))
    {
      empile(sq_departure,sq_arrival,sq_arrival);
      if (x>0 && !NoEdge(sq_arrival))
        grefc(orig_departure,sq_arrival,x-1,camp);
    }
    else if (piece_belongs_to_opponent(sq_arrival,camp))
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
  vec_index_type k;

  square sq_departure= orig_departure;

  if (!NoEdge(step_departure))
    settraversed(step_departure);

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    square sq_arrival = step_departure+vec[k];

    while (is_square_empty(sq_arrival))
    {
      empile(sq_departure,sq_arrival,sq_arrival);
      if (!NoEdge(sq_arrival) && !traversed(sq_arrival))
      {
        grefn(orig_departure,sq_arrival,camp);
        break;
      }
      else
        sq_arrival += vec[k];
    }

    if (piece_belongs_to_opponent(sq_arrival,camp))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
} /* grefc */

static void gequi(square sq_departure, Side camp)
{
  /* Equihopper */
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs)
    {
      square  end_of_line;
      finligne(sq_hurdle,vec[k],hurdle,end_of_line);
      {
        int const dist_hurdle_end= abs(end_of_line-sq_hurdle);
        int const dist_hurdle_dep= abs(sq_hurdle-sq_departure);
        if (dist_hurdle_end>dist_hurdle_dep)
        {
          square const sq_arrival= sq_hurdle+sq_hurdle-sq_departure;
          if (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k]))
          {
            empile(sq_departure,sq_arrival,sq_arrival);
            move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
          }
        }
        else if (dist_hurdle_end==dist_hurdle_dep)
        {
          square const sq_arrival= end_of_line;
          if (piece_belongs_to_opponent(sq_arrival,camp)
              && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
          {
            empile(sq_departure,sq_arrival,sq_arrival);
            move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
          }
        }
      }
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)
  {
    square const sq_hurdle= sq_departure+vec[k];
    square const sq_arrival= sq_departure + 2*vec[k];
    if (get_walk_of_piece_on_square(sq_hurdle)>=King
        && (is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,camp))
        && (!checkhopim || hopimok(sq_departure,sq_arrival,sq_hurdle,vec[k],vec[k])))
    {
      empile(sq_departure,sq_arrival,sq_arrival);
      move_generation_stack[current_move[nbply]].auxiliary = sq_hurdle;
    }
  }
}

static void gequiapp(square sq_departure, Side camp)
{
  /* (interceptable) Equistopper */
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece hurdle1;
    square sq_hurdle1;
    finligne(sq_departure,vec[k],hurdle1,sq_hurdle1);
    if (hurdle1!=obs)
    {
      square const sq_arrival= (sq_hurdle1+sq_departure)/2;
      if (!((sq_hurdle1/onerow+sq_departure/onerow)%2
            || (sq_hurdle1%onerow+sq_departure%onerow)%2)) /* is sq_arrival a square? */
        empile(sq_departure,sq_arrival,sq_arrival);

      {
        piece hurdle2;
        square sq_hurdle2;
        finligne(sq_hurdle1,vec[k],hurdle2,sq_hurdle2);
        if (hurdle2!=obs
            && abs(sq_hurdle2-sq_hurdle1)==abs(sq_hurdle1-sq_departure)
            && piece_belongs_to_opponent(sq_hurdle1,camp))
        {
          square const sq_arrival = sq_hurdle1;
          empile(sq_departure,sq_arrival,sq_arrival);
        }
      }
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)
  {
    square const sq_arrival = sq_departure + vec[k];
    square const sq_hurdle1 = sq_departure+2*vec[k];
    if (get_walk_of_piece_on_square(sq_hurdle1)>=King
        && (is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,camp)))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void gcat(square sq_departure, Side camp)
{
  /* generate moves of a CAT */
  vec_index_type k;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    square sq_arrival = sq_departure+vec[k];
    if (piece_belongs_to_opponent(sq_arrival,camp))
      empile(sq_departure,sq_arrival,sq_arrival);
    else
    {
      while (is_square_empty(sq_arrival))
      {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= mixhopdata[3][k];
      }

      if (piece_belongs_to_opponent(sq_arrival,camp))
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
  if (is_square_empty(pass)) {
    if (is_square_empty(arrival1) || piece_belongs_to_opponent(arrival1,camp))
      if (maooaimcheck(sq_departure,arrival1,pass))
        empile(sq_departure,arrival1,arrival1);

    if (is_square_empty(arrival2) || piece_belongs_to_opponent(arrival2,camp))
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
                          vec_index_type vec_start, vec_index_type vec_end)
{
  numecoup const save_nbcou = current_move[nbply];

  vec_index_type k;
  for (k = vec_end; k>=vec_start; k--)
  {
    piece hurdle;
    square sq_hurdle;
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs)
    {
      square const past_sq_hurdle= sq_hurdle+vec[k];
      if (is_square_empty(past_sq_hurdle))
      {
        vec_index_type k1;
        for (k1=vec_end; k1>=vec_start; k1--)
        {
          finligne(past_sq_hurdle,vec[k1],hurdle,sq_hurdle);
          if (hurdle!=obs)
          {
            square const sq_arrival= sq_hurdle+vec[k1];
            if (is_square_empty(sq_arrival) || piece_belongs_to_opponent(sq_arrival,camp))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }

  remove_duplicate_moves_of_single_piece(save_nbcou);
}

static void gdoublegrasshopper(square sq_departure, Side camp)
{
  gdoublehopper(sq_departure,camp,vec_queen_start,vec_queen_end);
}

static void gdoublerookhopper(square sq_departure, Side camp)
{
  gdoublehopper(sq_departure,camp,vec_rook_start,vec_rook_end);
}

static void gdoublebishopper(square sq_departure, Side camp)
{
  gdoublehopper(sq_departure,camp,vec_bishop_start,vec_bishop_end);
}

/* Two auxiliary functions for generating super pawn moves */
static void gen_sp_nocaptures(square sq_departure, numvec dir)
{
  /* generates non capturing moves of a super pawn in direction dir */

  square sq_arrival= sq_departure+dir;

  /* it can move from first rank */
  for (; is_square_empty(sq_arrival); sq_arrival+= dir)
    empile(sq_departure,sq_arrival,sq_arrival);
}

static void gen_sp_captures(square sq_departure, numvec dir, Side camp) {
  /* generates capturing moves of a super pawn of colour camp in
     direction dir.  */

  piece   hurdle;

  square sq_arrival;

  /* it can move from first rank */
  finligne(sq_departure,dir,hurdle,sq_arrival);
  if (piece_belongs_to_opponent(sq_arrival,camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

void chinese_pawn_generate_moves(Side side, square sq_departure)
{
  Side const opponent = advers(side);
  boolean const past_river = (side==White
                              ? 2*sq_departure>square_h8+square_a1
                              : 2*sq_departure<square_h8+square_a1);

  {
    int const dir_forward = side==White ? dir_up : dir_down;
    square const sq_arrival = sq_departure+dir_forward;
    if (is_square_empty(sq_arrival) || TSTFLAG(spec[sq_arrival],opponent))
      empile(sq_departure,sq_arrival,sq_arrival);
  }

  if (past_river)
  {
    {
      square const sq_arrival = sq_departure+dir_right;
      if (is_square_empty(sq_arrival) || TSTFLAG(spec[sq_arrival],opponent))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
    {
      square const sq_arrival = sq_departure+dir_left;
      if (is_square_empty(sq_arrival) || TSTFLAG(spec[sq_arrival],opponent))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

void piece_generate_moves(Side side, square sq_departure, PieNam p)
{
  switch (p)
  {
    case King:
      king_generate_moves(side,sq_departure);
      break;

    case Pawn:
      pawn_generate_moves(side,sq_departure);
      break;

    case Knight:
      leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Rook:
      rider_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Queen:
      rider_generate_moves(side,sq_departure, vec_queen_start,vec_queen_end);
      break;

    case Bishop:
      rider_generate_moves(side,sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case NightRider:
      rider_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Zebra:
      leaper_generate_moves(side,sq_departure, vec_zebre_start,vec_zebre_end);
      return;

    case Camel:
      leaper_generate_moves(side,sq_departure, vec_chameau_start,vec_chameau_end);
      return;

    case Giraffe:
      leaper_generate_moves(side,sq_departure, vec_girafe_start,vec_girafe_end);
      return;

    case RootFiftyLeaper:
      leaper_generate_moves(side,sq_departure, vec_rccinq_start,vec_rccinq_end);
      return;

    case Bucephale:
      leaper_generate_moves(side,sq_departure, vec_bucephale_start,vec_bucephale_end);
      return;

    case Wesir:
      leaper_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Alfil:
      leaper_generate_moves(side,sq_departure, vec_alfil_start,vec_alfil_end);
      return;

    case Fers:
      leaper_generate_moves(side,sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case Dabbaba:
      leaper_generate_moves(side,sq_departure, vec_dabbaba_start,vec_dabbaba_end);
      return;

    case BerolinaPawn:
      berolina_pawn_generate_moves(side,sq_departure);
      return;

    case ReversePawn:
      reverse_pawn_generate_moves(side,sq_departure);
      return;

    case Amazone:
      rider_generate_moves(side,sq_departure, vec_queen_start,vec_queen_end);
      leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Empress:
      rider_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
      leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Princess:
      rider_generate_moves(side,sq_departure, vec_bishop_start,vec_bishop_end);
      leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Gnu:
      leaper_generate_moves(side,sq_departure, vec_chameau_start,vec_chameau_end);
      leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Antilope:
      leaper_generate_moves(side,sq_departure, vec_antilope_start,vec_antilope_end);
      return;

    case Squirrel:
      leaper_generate_moves(side,sq_departure, vec_ecureuil_start,vec_ecureuil_end);
      leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Waran:
      rider_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      rider_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Dragon:
      pawn_generate_moves(side,sq_departure);
      leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Gryphon:
    {
      unsigned int const no_capture_length = pawn_get_no_capture_length(side,sq_departure);

      if (no_capture_length>0)
      {
        int const dir_forward = side==White ? dir_up : dir_down;
        pawns_generate_nocapture_moves(sq_departure,dir_forward,no_capture_length);
      }

      rider_generate_moves(side,sq_departure, vec_bishop_start,vec_bishop_end);
      return;
    }

    case Ship:
      if (pawn_get_no_capture_length(side,sq_departure)>0)
      {
        int const dir_forward = side==White ? dir_up : dir_down;
        pawns_generate_capture_move(side,sq_departure,dir_forward+dir_left);
        pawns_generate_capture_move(side,sq_departure,dir_forward+dir_right);
      }

      rider_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Camelrider:
      rider_generate_moves(side,sq_departure, vec_chameau_start,vec_chameau_end);
      return;

    case Zebrarider:
      rider_generate_moves(side,sq_departure, vec_zebre_start,vec_zebre_end);
      return;

    case Gnurider:
      rider_generate_moves(side,sq_departure, vec_chameau_start,vec_chameau_end);
      rider_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
      return;

    case SuperBerolinaPawn:
    {
      int const dir_forward = side==White ? dir_up : dir_down;
      gen_sp_nocaptures(sq_departure,dir_forward+dir_left);
      gen_sp_nocaptures(sq_departure,dir_forward+dir_right);
      gen_sp_captures(sq_departure,dir_forward,side);
      return;
    }

    case SuperPawn:
    {
      int const dir_forward = side==White ? dir_up : dir_down;
      gen_sp_nocaptures(sq_departure,dir_forward);
      gen_sp_captures(sq_departure,dir_forward+dir_left,side);
      gen_sp_captures(sq_departure,dir_forward+dir_right,side);
      return;
    }

    case RookHunter:
      rook_hunter_generate_moves(side,sq_departure);
      return;

    case BishopHunter:
      bishop_hunter_generate_moves(side,sq_departure);
      return;

    case ErlKing:
      leaper_generate_moves(side,sq_departure, vec_queen_start,vec_queen_end);
      return;

    case Okapi:
      leaper_generate_moves(side,sq_departure, vec_okapi_start,vec_okapi_end);
      return;

    case Leap37:
      leaper_generate_moves(side,sq_departure, vec_leap37_start,vec_leap37_end);
      return;

    case Leap16:
      leaper_generate_moves(side,sq_departure, vec_leap16_start,vec_leap16_end);
      return;

    case Leap24:
      leaper_generate_moves(side,sq_departure, vec_leap24_start,vec_leap24_end);
      return;

    case Leap35:
      leaper_generate_moves(side,sq_departure, vec_leap35_start,vec_leap35_end);
      return;

    case Leap15:
      leaper_generate_moves(side,sq_departure, vec_leap15_start,vec_leap15_end);
      return;

    case Leap25:
      leaper_generate_moves(side,sq_departure, vec_leap25_start,vec_leap25_end);
      return;

    case WesirRider:
      rider_generate_moves(side,sq_departure,   vec_rook_start,vec_rook_end);
      return;

    case FersRider:
      rider_generate_moves(side,sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case Bison:
      leaper_generate_moves(side,sq_departure, vec_bison_start,vec_bison_end);
      return;

    case Zebu:
      leaper_generate_moves(side,sq_departure, vec_chameau_start,vec_chameau_end);
      leaper_generate_moves(side,sq_departure, vec_girafe_start,vec_girafe_end);
      return;

    case Elephant:
      rider_generate_moves(side,sq_departure, vec_elephant_start,vec_elephant_end);
      return;

    case Leap36:
      leaper_generate_moves(side,sq_departure, vec_leap36_start,vec_leap36_end);
      return;

    case ChinesePawn:
      chinese_pawn_generate_moves(side,sq_departure);
      return;

    case Mao:
      gmao(sq_departure, side);
      return;

    case Pao:
      gchin(sq_departure, vec_rook_start,vec_rook_end, side);
      return;

    case Leo:
      gchin(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case Vao:
      gchin(sq_departure, vec_bishop_start,vec_bishop_end, side);
      return;

    case Nao:
      gchin(sq_departure, vec_knight_start,vec_knight_end, side);
      return;

    case Rose:
      rose_generate_moves(side,sq_departure,vec_knight_start,vec_knight_end);
      return;

    case NonStopEquihopper:
      gnequi(sq_departure, side);
      return;

    case Locust:
      glocust(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case NightLocust:
      glocust(sq_departure, vec_knight_start,vec_knight_end, side);
      return;

    case BishopLocust:
      glocust(sq_departure, vec_bishop_start,vec_bishop_end, side);
      return;

    case RookLocust:
      glocust(sq_departure, vec_rook_start,vec_rook_end, side);
      return;

    case Kangaroo:
      gkang(sq_departure, side);
      return;

    case KangarooLion:
      gkanglion(sq_departure, side);
      return;

    case Kao:
      gchinleap(sq_departure, vec_knight_start, vec_knight_end, side);
      return;

    case KnightHopper:
      geshop(sq_departure, vec_knight_start, vec_knight_end, side);
      return;

    case SpiralSpringer:
      spiralspringer_generate_moves(side,sq_departure);
      return;

    case DiagonalSpiralSpringer:
      diagonalspiralspringer_generate_moves(side,sq_departure);
      return;

    case BoyScout:
      boyscout_generate_moves(side,sq_departure);
      return;

    case GirlScout:
      girlscout_generate_moves(side,sq_departure);
      return;

    case Hamster:
      ghamst(sq_departure);
      return;

    case UbiUbi:
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
        e_ubi[*bnp]= e[*bnp];
      gubi(sq_departure, sq_departure, side);
      return;
    }

    case Elk:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_queen_start,vec_queen_end, 0, side);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case Eagle:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_queen_start,vec_queen_end, 1, side);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case Sparrow:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_queen_start,vec_queen_end, 2, side);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case Marguerite:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_queen_start,vec_queen_end, 0, side);
      gmhop(sq_departure, vec_queen_start,vec_queen_end, 1, side);
      gmhop(sq_departure, vec_queen_start,vec_queen_end, 2, side);
      gerhop(sq_departure, vec_queen_start,vec_queen_end, side);
      ghamst(sq_departure);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case Archbishop:
    {
      numecoup const save_current_move = current_move[nbply];
      vec_index_type k;
      for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
        grfou(sq_departure,sq_departure,vec[k],1,side);
      if (!NoEdge(sq_departure))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case ReflectBishop:
    {
      numecoup const save_current_move = current_move[nbply];
      vec_index_type k;
      for (k= vec_bishop_start; k <= vec_bishop_end; k++)
        grfou(sq_departure,sq_departure,vec[k],4,side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case Cardinal:
    {
      vec_index_type k;
      for (k= vec_bishop_start; k <= vec_bishop_end; k++)
        gcard(sq_departure, sq_departure, vec[k], 1, side);
      return;
    }

    case BouncyKnight:
    {
      numecoup const save_current_move = current_move[nbply];
      grefc(sq_departure, sq_departure, 2, side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case BouncyNightrider:
    {
      numecoup const save_current_move = current_move[nbply];
      clearedgestraversed();
      grefn(sq_departure, sq_departure, side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case EquiHopper:
      gequi(sq_departure, side);
      return;

    case CAT:
      gcat(sq_departure, side);
      return;

    case Sirene:
      marine_rider_generate_moves(side, sq_departure, vec_queen_start,vec_queen_end);
      return;

    case Triton:
      marine_rider_generate_moves(side, sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Nereide:
      marine_rider_generate_moves(side, sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case Orphan:
      gorph(sq_departure, side);
      return;

    case Friend:
      gfriend(sq_departure, side);
      return;

    case EdgeHog:
      gedgeh(sq_departure, side);
      return;

    case Moa:
      gmoa(sq_departure, side);
      return;

    case MoaRider:
      gemoarider(sq_departure, side);
      return;

    case MaoRider:
      gemaorider(sq_departure, side);
      return;

    case Skylla:
      geskylla(sq_departure, side);
      return;

    case Charybdis:
      gecharybdis(sq_departure, side);
      return;

    case Grasshopper:
      gerhop(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case Lion:
      gelrhop(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case NightriderHopper:
      gerhop(sq_departure, vec_knight_start,vec_knight_end, side);
      return;

    case CamelHopper:
      gerhop(sq_departure, vec_chameau_start,vec_chameau_end, side);
      return;

    case ZebraHopper:
      gerhop(sq_departure, vec_zebre_start,vec_zebre_end, side);
      return;

    case GnuHopper:
      gerhop(sq_departure, vec_chameau_start,vec_chameau_end, side);
      gerhop(sq_departure, vec_knight_start,vec_knight_end, side);
      return;

    case RookLion:
      gelrhop(sq_departure, vec_rook_start,vec_rook_end, side);
      return;

    case BishopLion:
      gelrhop(sq_departure, vec_bishop_start,vec_bishop_end, side);
      return;

    case RookHopper:
      gerhop(sq_departure, vec_rook_start,vec_rook_end, side);
      return;

    case BishopHopper:
      gerhop(sq_departure, vec_bishop_start,vec_bishop_end, side);
      return;

    case ContraGras:
      gecrhop(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case RoseLion:
      roselion_generate_moves(side,sq_departure,vec_knight_start,vec_knight_end);
      return;

    case RoseHopper:
      rosehopper_generate_moves(side,sq_departure,vec_knight_start,vec_knight_end);
      return;

    case RoseLocust:
      roselocust_generate_moves(side,sq_departure,vec_knight_start,vec_knight_end);
      return;

    case GrassHopper2:
      gerhop2(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case GrassHopper3:
      gerhop3(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case KingHopper:
      geshop(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case DoubleGras:
      gdoublegrasshopper(sq_departure, side);
      return;

    case DoubleRookHopper:
      gdoublerookhopper(sq_departure, side);
      return;

    case DoubleBishopper:
      gdoublebishopper(sq_departure, side);
      return;

    case Orix:
      gorix(sq_departure, side);
      return;

     case NonStopOrix:
      gnorix(sq_departure, side);
      return;

    case Gral:
      leaper_generate_moves(side,sq_departure, vec_alfil_start,vec_alfil_end);
      gerhop(sq_departure, vec_rook_start,vec_rook_end, side);      /* rookhopper */
      return;

    case RookMoose:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_rook_start,vec_rook_end, 0, side);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case RookEagle:
      gmhop(sq_departure, vec_rook_start,vec_rook_end, 1, side);
      return;

    case RookSparrow:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_rook_start,vec_rook_end, 2, side);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case BishopMoose:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 0, side);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case BishopEagle:
      gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 1, side);
      return;

    case BishopSparrow:
    {
      numecoup const save_current_move = current_move[nbply];
      gmhop(sq_departure, vec_bishop_start,vec_bishop_end, 2, side);
      if (!TSTFLAG(spec[sq_departure],ColourChange))
        remove_duplicate_moves_of_single_piece(save_current_move);
      return;
    }

    case Rao:
      rao_generate_moves(side,sq_departure,vec_knight_start,vec_knight_end);
      return;

    case Scorpion:
      leaper_generate_moves(side,sq_departure, vec_queen_start,vec_queen_end); /* eking */
      gerhop(sq_departure, vec_queen_start,vec_queen_end, side);     /* grashopper */
      return;

    case NightRiderLion:
      gelrhop(sq_departure, vec_knight_start,vec_knight_end, side);
      return;

    case MaoRiderLion:
      gemaoriderlion(sq_departure, side);
      return;

    case MoaRiderLion:
      gemoariderlion(sq_departure, side);
      return;

    case Dolphin:
      gkang(sq_departure, side);
      gerhop(sq_departure, vec_queen_start,vec_queen_end, side);
      return;

    case Rabbit:
      grabbit(sq_departure, side);
      return;

    case Bob:
      gbob(sq_departure, side);
      return;

    case EquiEnglish:
      gequiapp(sq_departure, side);
      return;

    case EquiFrench:
      gnequiapp(sq_departure, side);
      return;

    case Querquisite:
      switch (sq_departure%onerow - nr_of_slack_files_left_of_board) {
      case file_rook_queenside:
      case file_rook_kingside:
        rider_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
        break;
      case file_bishop_queenside:
      case file_bishop_kingside:
        rider_generate_moves(side,sq_departure, vec_bishop_start,vec_bishop_end);
        break;
      case file_queen:
        rider_generate_moves(side,sq_departure, vec_queen_start,vec_queen_end);
        break;
      case file_knight_queenside:
      case file_knight_kingside:
        leaper_generate_moves(side,sq_departure, vec_knight_start,vec_knight_end);
        break;
      case file_king:
        leaper_generate_moves(side,sq_departure, vec_queen_start,vec_queen_end);
        break;
      }
      break;

    case Bouncer :
      bouncer_generate_moves(side, sq_departure, vec_queen_start,vec_queen_end);
      break;

    case RookBouncer:
      bouncer_generate_moves(side, sq_departure, vec_rook_start,vec_rook_end);
      break;

    case BishopBouncer :
      bouncer_generate_moves(side, sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case RadialKnight:
      genradialknight(sq_departure, side);
      break;

    case Treehopper:
      gentreehopper(sq_departure, side);
      break;

    case Leafhopper :
      genleafhopper(sq_departure, side);
      break;

    case GreaterTreehopper:
      gengreatertreehopper(sq_departure, side);
      break;

    case GreaterLeafhopper:
      gengreaterleafhopper(sq_departure, side);
      break;

    case SpiralSpringer40:
    {
      numecoup const save_current_move = current_move[nbply];
      gcsp(sq_departure, 9, 16, side);
      gcsp(sq_departure, 10, 11, side);
      gcsp(sq_departure, 11, 10, side);
      gcsp(sq_departure, 12, 13, side);
      gcsp(sq_departure, 13, 12, side);
      gcsp(sq_departure, 14, 15, side);
      gcsp(sq_departure, 15, 14, side);
      gcsp(sq_departure, 16, 9, side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      break;
    }

    case SpiralSpringer20:
    {
      numecoup const save_current_move = current_move[nbply];
      gcsp(sq_departure, 9, 12, side);
      gcsp(sq_departure, 10, 15, side);
      gcsp(sq_departure, 11, 14, side);
      gcsp(sq_departure, 12, 9, side);
      gcsp(sq_departure, 13, 16, side);
      gcsp(sq_departure, 14, 11, side);
      gcsp(sq_departure, 15, 10, side);
      gcsp(sq_departure, 16, 13, side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      break;
    }

    case SpiralSpringer33:
    {
      numecoup const save_current_move = current_move[nbply];
      gcsp(sq_departure, 9, 10, side);
      gcsp(sq_departure, 10, 9, side);
      gcsp(sq_departure, 11, 12, side);
      gcsp(sq_departure, 12, 11, side);
      gcsp(sq_departure, 13, 14, side);
      gcsp(sq_departure, 14, 13, side);
      gcsp(sq_departure, 15, 16, side);
      gcsp(sq_departure, 16, 15, side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      break;
    }

    case SpiralSpringer11:
    {
      numecoup const save_current_move = current_move[nbply];
      gcsp(sq_departure, 9, 14, side);
      gcsp(sq_departure, 10, 13, side);
      gcsp(sq_departure, 11, 16, side);
      gcsp(sq_departure, 12, 15, side);
      gcsp(sq_departure, 13, 10, side);
      gcsp(sq_departure, 14, 9, side);
      gcsp(sq_departure, 15, 12, side);
      gcsp(sq_departure, 16, 11, side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      break;
    }

    case Quintessence:
    {
      numecoup const save_current_move = current_move[nbply];
      gcsp(sq_departure, 9, 11, side);
      gcsp(sq_departure, 11, 9, side);
      gcsp(sq_departure, 11, 13, side);
      gcsp(sq_departure, 13, 11, side);
      gcsp(sq_departure, 13, 15, side);
      gcsp(sq_departure, 15, 13, side);
      gcsp(sq_departure, 15, 9, side);
      gcsp(sq_departure, 9, 15, side);
      gcsp(sq_departure, 10, 12, side);
      gcsp(sq_departure, 12, 10, side);
      gcsp(sq_departure, 12, 14, side);
      gcsp(sq_departure, 14, 12, side);
      gcsp(sq_departure, 14, 16, side);
      gcsp(sq_departure, 16, 14, side);
      gcsp(sq_departure, 16, 10, side);
      gcsp(sq_departure, 10, 16, side);
      remove_duplicate_moves_of_single_piece(save_current_move);
      break;
    }

    case MarineKnight:
      marine_knight_generate_moves(side,sq_departure);
      break;

    case Poseidon:
      poseidon_generate_moves(side,sq_departure);
      break;

    case MarinePawn:
      marine_pawn_generate_moves(side,sq_departure);
      break;

    case MarineShip:
      marine_ship_generate_moves(side,sq_departure,vec_rook_start,vec_rook_end);
      return;

    default:
      /* Since pieces like DUMMY fall through 'default', we have */
      /* to check exactly if there is something to generate ...  */
      if (p>=Hunter0 && p<Hunter0+maxnrhuntertypes)
        hunter_generate_moves(side,sq_departure,p);
      break;
  }
}

void king_generate_moves(Side side_moving, square sq_departure)
{
  if (calc_reflective_king[side_moving] && !transmuting_kings_lock_recursion)
  {
    boolean generated_by_transmutation = false;
    numecoup const save_nbcou = current_move[nbply];

    if (CondFlag[side_moving==White ? blvault_king : whvault_king])
    {
      generated_by_transmutation = vaulting_kings_generate_moves(side_moving,sq_departure);
      if (generated_by_transmutation && calc_transmuting_king[side_moving])
        return;
    }
    else if (CondFlag[side_moving==White ? whtrans_king : bltrans_king]
             || CondFlag[side_moving==White ? whsupertrans_king : blsupertrans_king])
    {
      if (transmuting_kings_generate_moves(side_moving,sq_departure))
        return;
    }
    else if (CondFlag[side_moving==White ? whrefl_king : blrefl_king])
      generated_by_transmutation = transmuting_kings_generate_moves(side_moving,sq_departure);
    else
    {
      assert(0);
    }

    if (CondFlag[sting])
      gerhop(sq_departure,vec_queen_start,vec_queen_end,side_moving);

    leaper_generate_moves(side_moving,sq_departure,vec_queen_start,vec_queen_end);

    if (generated_by_transmutation)
      remove_duplicate_moves_of_single_piece(save_nbcou);
  }
  else
  {
    if (CondFlag[sting])
      gerhop(sq_departure,vec_queen_start,vec_queen_end,side_moving);

    leaper_generate_moves(side_moving,sq_departure,vec_queen_start,vec_queen_end);
  }

  if (castling_supported)
    generate_castling(side_moving);

  if (CondFlag[castlingchess] && !echecc(side_moving))
  {
    vec_index_type k;
    for (k = vec_queen_end; k>= vec_queen_start; --k)
    {
      square const sq_passed = sq_departure+vec[k];
      square const sq_arrival = sq_passed+vec[k];
      square sq_castler;
      piece p;

      finligne(sq_departure,vec[k],p,sq_castler);
      if (sq_castler!=sq_passed && sq_castler!=sq_arrival && p!=obs
          && castling_is_intermediate_king_move_legal(side_moving,sq_departure,sq_passed))
        empile(sq_departure,sq_arrival,maxsquare+sq_castler);
    }
  }

  if (CondFlag[platzwechselrochade] && platzwechsel_rochade_allowed[side_moving][nbply])
  {
    int i;
    square square_a = square_a1;
    for (i = nr_rows_on_board; i>0; --i, square_a += onerow)
    {
      int j;
      square pos_partner = square_a;
      for (j = nr_files_on_board; j>0; --j, pos_partner += dir_right)
        if (pos_partner!=sq_departure
            && TSTFLAG(spec[pos_partner],side_moving)
            && !is_pawn(get_walk_of_piece_on_square(pos_partner))) /* not sure if "castling" with Ps forbidden */
          empile(sq_departure,pos_partner,platzwechsel_rochade);
    }
  }
}

void gen_wh_ply(void)
{
  unsigned int i;
  square z = square_a1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* Don't try to "optimize" by hand. The double-loop is tested as
     the fastest way to compute (due to compiler-optimizations !)
     V3.14  NG
  */
  for (i = nr_rows_on_board; i > 0; i--, z+= onerow-nr_files_on_board)
  {
    square j;
    for (j = nr_files_on_board; j > 0; j--, z++)
      if (TSTFLAG(spec[z],White))
        generate_moves_for_piece(White,z,get_walk_of_piece_on_square(z));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void gen_piece_aux(Side side, square sq_departure, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (CondFlag[annan])
  {
    int const annaniser_dir = side==White ? -onerow : +onerow;
    square const annaniser_pos = sq_departure+annaniser_dir;
    if (annanises(side,annaniser_pos,sq_departure))
    {
      boolean const save_castling_supported = castling_supported;
      castling_supported = false;
      piece_generate_moves(side,sq_departure,get_walk_of_piece_on_square(annaniser_pos));
      castling_supported = save_castling_supported;
    }
    else
      piece_generate_moves(side,sq_departure,p);
  }
  else
    piece_generate_moves(side,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void orig_generate_moves_for_piece(Side side, square sq_departure, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
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
      phantom_chess_generate_moves(side,p,sq_departure);
    else if (CondFlag[plus])
      plus_generate_moves(side,p,sq_departure);
    else if (anymars)
      marscirce_generate_moves(side,p,sq_departure);
    else if (anyantimars)
      antimars_generate_moves(side,p,sq_departure);
    else
      gen_piece_aux(side,sq_departure,p);

    if (CondFlag[messigny] && !(king_square[side]==sq_departure && rex_mess_ex))
    {
      Side const opponent = advers(side);
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
        if (TSTFLAG(spec[*bnp],opponent) && get_walk_of_piece_on_square(*bnp)==p)
          empile(sq_departure,*bnp,messigny_exchange);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void (*generate_moves_for_piece)(Side side, square z, PieNam p) = &orig_generate_moves_for_piece;

void gorph(square sq_departure, Side camp)
{
  numecoup const save_nbcou = current_move[nbply];

  PieNam const *orphan_observer;
  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; ++orphan_observer)
    if (number_of_pieces[White][*orphan_observer]+number_of_pieces[Black][*orphan_observer]>0
        && orphan_find_observation_chain(sq_departure,*orphan_observer,&validate_observation))
      generate_moves_for_piece(camp,sq_departure,*orphan_observer);

  remove_duplicate_moves_of_single_piece(save_nbcou);
}

void gfriend(square i, Side camp)
{
  numecoup const save_nbcou = current_move[nbply];

  PieNam const *friend_observer;
  for (friend_observer = orphanpieces; *friend_observer!=Empty; ++friend_observer)
    if (number_of_pieces[camp][*friend_observer]>0
        && find_next_friend_in_chain(i,*friend_observer,Friend,&validate_observation))
      generate_moves_for_piece(camp, i,*friend_observer);

  remove_duplicate_moves_of_single_piece(save_nbcou);
}


void gedgeh(square sq_departure, Side camp) {
  vec_index_type k;

  for (k= vec_queen_end; k >=vec_queen_start; k--) {
    square sq_arrival= sq_departure+vec[k];
    while (is_square_empty(sq_arrival))
    {
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        empile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= vec[k];
    }

    if (piece_belongs_to_opponent(sq_arrival,camp))
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        empile(sq_departure,sq_arrival,sq_arrival);
  }
}

static void geskylchar(square sq_departure, square sq_arrival, square sq_capture,
                Side camp) {
  if (is_square_empty(sq_arrival))
  {
    if (is_square_empty(sq_capture))
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (piece_belongs_to_opponent(sq_capture,camp))
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

void reverse_pawn_generate_moves(Side side, square sq_departure)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(advers(side),
                                                                    sq_departure);

  if (no_capture_length>0)
  {
    int const dir_backward = side==White ? dir_down : dir_up;

    pawns_generate_capture_move(side,sq_departure,dir_backward+dir_right);
    pawns_generate_capture_move(side,sq_departure,dir_backward+dir_left);
    pawns_generate_nocapture_moves(sq_departure,dir_backward,no_capture_length);
  }
}

/************************  white berolina pawn  ***********************/
void berolina_pawn_generate_moves(Side side, square sq_departure)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(side,sq_departure);

  if (no_capture_length>0)
  {
    int const dir_forward = side==White ? dir_up : dir_down;

    pawns_generate_capture_move(side,sq_departure,dir_forward);
    pawns_generate_nocapture_moves(sq_departure,dir_forward+dir_left,no_capture_length);
    pawns_generate_nocapture_moves(sq_departure,dir_forward+dir_right,no_capture_length);
  }
}

static void genleapleap(square sq_departure,
                        vec_index_type kanf, vec_index_type kend,
                        int hurdletype, Side camp, boolean leaf)
{
  vec_index_type k;

  for (k= kanf; k<=kend; ++k)
  {
    square const sq_hurdle = sq_departure + vec[k];
    if ((hurdletype==0 && piece_belongs_to_opponent(sq_hurdle,camp))
        || (hurdletype==1 && get_walk_of_piece_on_square(sq_hurdle)>=King))
    {
      vec_index_type k1;
      for (k1= kanf; k1 <= kend; k1++)
      {
        square const sq_arrival = (leaf ? sq_departure : sq_hurdle) + vec[k1];
        if ((sq_arrival != sq_hurdle) && (e[sq_arrival] == vide || piece_belongs_to_opponent(sq_arrival,camp)))
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
