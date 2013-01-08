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
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "solving/single_move_generator.h"
#include "solving/en_passant.h"
#include "stipulation/temporary_hacks.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

boolean eval_ortho(square sq_departure, square sq_arrival, square sq_capture) {
  return true;
}

boolean legalsquare(square sq_departure, square sq_arrival, square sq_capture) {
  if (CondFlag[koeko]) {
    if (nocontact(sq_departure,sq_arrival,sq_capture, koeko_nocontact))
      return(false);
  }
  if (CondFlag[antikoeko]) {
    if (!nocontact(sq_departure,sq_arrival,sq_capture, antikoeko_nocontact))
      return(false);
  }
  if (CondFlag[gridchess]) {
    if (!GridLegal(sq_departure, sq_arrival))
      return(false);
  }
  if (CondFlag[blackedge]) {
    if (e[sq_departure] <= roin)
      if (NoEdge(sq_arrival))
        return(false);
  }
  if (CondFlag[whiteedge]) {
    if (e[sq_departure] >= roib)
      if (NoEdge(sq_arrival))
        return(false);
  }
  if (CondFlag[bichro]) {
    if (SquareCol(sq_departure) == SquareCol(sq_arrival))
      return(false);
  }
  if (CondFlag[monochro]) {
    if (SquareCol(sq_departure) != SquareCol(sq_arrival))
      return(false);
  }
  if (TSTFLAG(spec[sq_departure], Jigger)) {
    if (nocontact(sq_departure,sq_arrival,sq_capture,nokingcontact))
      return(false);
  }
  if (CondFlag[newkoeko]) {
    if (nocontact(sq_departure,sq_arrival,sq_capture,nokingcontact)
        != nocontact(initsquare,sq_departure,initsquare,nokingcontact))
    {
      return false;
    }
  }
  if (anygeneva) {
    if ((e[sq_capture] <= roin) && (rex_geneva || (sq_departure != king_square[White])))
      if (e[rennormal(e[sq_departure],spec[sq_departure],sq_departure,sq_departure,sq_arrival,Black)] != vide)
        return(false);
    if ((e[sq_capture] >= roib) && (rex_geneva || (sq_departure != king_square[Black])))
      if (e[rennormal(e[sq_departure],spec[sq_departure],sq_departure,sq_departure,sq_arrival,White)] != vide)
        return(false);
  }
  return(true);
} /* end of legalsquare */

boolean imok(square i, square j)
{
  /* move i->j ok? */
  unsigned int imi_idx;
  int const diff = j-i;

  for (imi_idx = number_of_imitators; imi_idx>0; imi_idx--)
  {
    square const j2 = isquare[imi_idx-1]+diff;
    if (j2!=i && e[j2]!=vide)
      return false;
  }

  return true;
}

boolean maooaimok(square i, square j, square pass) {
  boolean ret;
  piece p= e[i];
  e[i]= vide;
  ret= imok(i, pass) && imok(i, j);
  e[i]= p;
  return ret;
}

boolean ridimok(square i, square j, numvec diff) {
  /* move i->j in steps of diff ok? */
  square  i2= i;
  boolean ret;
  piece   p= e[i];

  e[i]= vide;/* an imitator might be disturbed by the moving rider! */
  do {
    i2-= diff;
  } while (imok(i, i2) && (i2 != j));

  ret= (i2 == j) && imok (i, j);
  e[i]= p;           /* only after the last call of imok! */
  return ret;
}

boolean castlingimok(square i, square j) {
  piece p= e[i];
  boolean ret= false;
  /* I think this should work - clear the K, and move the Is, but don't clear the rook. */
  /* If the Is crash into the R, the move would be illegal as the K moves first.        */
  /* The only other test here is for long castling when the Is have to be clear to move */
  /* one step right (put K back first)as well as two steps left.                        */
  /* But there won't be an I one sq to the left of a1 (a8) so no need to clear the R    */

  switch (j-i)
  {
    case 2*dir_right:  /* 00 - can short-circuit here (only follow K, if ok rest will be ok) */
      e[i]= vide;
      ret= imok(i, i+dir_right) && imok(i, i+2*dir_right);
      e[i]= p;
      break;

    case 2*dir_left:  /* 000 - follow K, (and move K as well), then follow R */
      e[i]= vide;
      ret= imok(i, i+dir_left) && imok(i, i+2*dir_left);
      e[i+2*dir_left]= p;
      ret= ret && imok(i, i+dir_left) && imok (i, i) && imok(i, i+dir_right);
      e[i+2*dir_left]= vide;
      e[i]= p;
      break;
  }
  return ret;
}



boolean hopimok(square i, square j, square k, numvec diff, numvec diff1)
{
  /* hop i->j hopping over k in steps of diff ok? */

  if (CondFlag[imitators])
  {
    square i2 = i;
    piece const p = e[i];
    boolean result = true;

    /* an imitator might be disturbed by the moving hopper! */
    e[i] = vide;

    /* Are the lines from the imitators to the square to hop over free?
     */
    do
    {
      i2 += diff;
    } while (imok(i,i2) && i2!=k);

    result = i2==k;

    if (result)
    {
      /* Are the squares the imitators have to hop over occupied? */
      unsigned int imi_idx;
      for (imi_idx = number_of_imitators; imi_idx>0; imi_idx--)
        if (e[isquare[imi_idx-1]+k-i]==vide)
        {
          result = false;
          break;
        }
    }

    if (result)
      do
      {
        i2 += diff1;
      } while (imok(i,i2) && i2!=j);

    result = result && i2==j && imok(i,j);

    e[i] = p;

    return result;
  }
  else
    return true;
}


boolean rmhopech(square sq_king,
                 numvec kend,
                 numvec kanf,
                 angle_t angle,
                 piece  p,
                 evalfunction_t *evaluate)
{
  square sq_hurdle;
  numvec k, k1;
  piece hopper;

  /* ATTENTION:
   *    angle==angle_45:  moose    45 degree hopper
   *    angle==angle_90:  eagle    90 degree hopper
   *    angle==angle_135: sparrow 135 degree hopper
   *
   *    kend==vec_queen_end, kanf==vec_queen_start: all types (moose,
   *                                                eagle, sparrow)
   *    kend==vec_bishop_end, kanf==vec_bishop_start:
   *        types arriving diagonally (rookmoose, rooksparrow, bishopeagle)
   *    kend==vec_rook_end, kanf==vec_rook_start: diagonal types
   *        types arriving orthogonally (bishopmoose, bishopsparrow, rookeagle)
   *
   *    YES, this is definitely different to generating their moves ...
   *                                     NG
   */

  square sq_departure;
  numvec v, v1;

  for (k= kend; k>=kanf; k--) {
    v= vec[k];
    sq_hurdle= sq_king+v;
    if (abs(e[sq_hurdle])>=roib) {
      k1= 2*k;
      finligne(sq_hurdle,v1=mixhopdata[angle][k1],hopper,sq_departure);
      if (hopper==p) {
        if (evaluate(sq_departure,sq_king,sq_king)
            && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-v1,-v)))
          return true;
      }
      finligne(sq_hurdle,v1=mixhopdata[angle][k1-1],hopper,sq_departure);
      if (hopper==p) {
        if (evaluate(sq_departure,sq_king,sq_king)
            && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-v1,-v)))
          return true;
      }
    }
  }
  return false;
}

boolean rcsech(square  sq_king,
               numvec  k,
               numvec  k1,
               piece   p,
               evalfunction_t *evaluate)
{
  /* ATTENTION: There is a parameter dependency between the
   *          indexes of k and of k1 !
   *              p       index of k (ik) I index of k1
   *          ----------------------------------------I------------
   *          Spiralspringer       9 to 16    I 25 - ik
   *                              I
   *          DiagonalSpiralspringer   9 to 14    I 23 - ik
   *          DiagonalSpiralspringer  15 to 16    I 27 - ik
   */

  square sq_departure= sq_king+k;
  square sq_arrival= sq_king;
  square sq_capture= sq_king;

  while (e[sq_departure] == vide) {
    sq_departure+= k1;
    if (e[sq_departure] != vide)
      break;
    else
      sq_departure+= k;
  }

  if (e[sq_departure]==p
      && evaluate(sq_departure,sq_arrival,sq_capture))
    return true;

  sq_departure = sq_king+k;
  while (e[sq_departure]==vide) {
    sq_departure-= k1;
    if (e[sq_departure]!=vide)
      break;
    else
      sq_departure+= k;
  }

  if (e[sq_departure]==p
      && evaluate(sq_departure,sq_arrival,sq_capture))
    return true;

  return false;
}

boolean rcspech(square  sq_king,
               numvec  k,
               numvec  k1,
               piece   p,
               evalfunction_t *evaluate)
{
  square sq_departure= sq_king+vec[k];
  square sq_arrival= sq_king;
  square sq_capture= sq_king;

  while (e[sq_departure] == vide) {
    sq_departure+= vec[k1];
    if (e[sq_departure] != vide)
      break;
    else
      sq_departure+= vec[k];
  }

  if (e[sq_departure]==p
      && evaluate(sq_departure,sq_arrival,sq_capture))
    return true;

  return false;
}

boolean nevercheck(square  i,
                   piece   p,
                   evalfunction_t *evaluate)
{
  return false;
}

boolean cscheck(square  i,
                piece   p,
                evalfunction_t *evaluate)
{
  numvec  k;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    if (rcsech(i, vec[k], vec[25 - k], p, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean bscoutcheck(square  i,
                    piece   p,
                    evalfunction_t *evaluate)
{
  numvec  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rcsech(i, vec[k], vec[13 - k], p, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean gscoutcheck(square  i,
                    piece   p,
                    evalfunction_t *evaluate)
{
  numvec  k;

  for (k= vec_rook_end; k >= vec_rook_start; k--) {
    if (rcsech(i, vec[k], vec[5 - k], p, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean sp40check(square  i,
                    piece   p,
                    evalfunction_t *evaluate)
{
  return rcspech(i, 9, 16, p, evaluate) ||
         rcspech(i, 10, 11, p, evaluate) ||
         rcspech(i, 11, 10, p, evaluate) ||
         rcspech(i, 12, 13, p, evaluate) ||
         rcspech(i, 13, 12, p, evaluate) ||
         rcspech(i, 14, 15, p, evaluate) ||
         rcspech(i, 15, 14, p, evaluate) ||
         rcspech(i, 16, 9, p, evaluate);
}

boolean sp20check(square  i,
                    piece   p,
                    evalfunction_t *evaluate)
{
  return rcspech(i, 9, 12, p, evaluate) ||
         rcspech(i, 10, 15, p, evaluate) ||
         rcspech(i, 11, 14, p, evaluate) ||
         rcspech(i, 12, 9, p, evaluate) ||
         rcspech(i, 13, 16, p, evaluate) ||
         rcspech(i, 14, 11, p, evaluate) ||
         rcspech(i, 15, 10, p, evaluate) ||
         rcspech(i, 16, 13, p, evaluate);
}

boolean sp33check(square  i,
                    piece   p,
                    evalfunction_t *evaluate)
{
  return rcspech(i, 9, 10, p, evaluate) ||
         rcspech(i, 10, 9, p, evaluate) ||
         rcspech(i, 11, 12, p, evaluate) ||
         rcspech(i, 12, 11, p, evaluate) ||
         rcspech(i, 13, 14, p, evaluate) ||
         rcspech(i, 14, 13, p, evaluate) ||
         rcspech(i, 15, 16, p, evaluate) ||
         rcspech(i, 16, 15, p, evaluate);
}

boolean sp11check(square  i,
                    piece   p,
                    evalfunction_t *evaluate)
{
  return rcspech(i, 9, 14, p, evaluate) ||
         rcspech(i, 14, 9, p, evaluate) ||
         rcspech(i, 10, 13, p, evaluate) ||
         rcspech(i, 13, 10, p, evaluate) ||
         rcspech(i, 11, 16, p, evaluate) ||
         rcspech(i, 16, 11, p, evaluate) ||
         rcspech(i, 12, 15, p, evaluate) ||
         rcspech(i, 15, 12, p, evaluate);
}


boolean sp31check(square  i,
                    piece   p,
                    evalfunction_t *evaluate)
{
  return rcspech(i, 9, 11, p, evaluate) ||
         rcspech(i, 11, 9, p, evaluate) ||
         rcspech(i, 11, 13, p, evaluate) ||
         rcspech(i, 13, 11, p, evaluate) ||
         rcspech(i, 13, 15, p, evaluate) ||
         rcspech(i, 15, 13, p, evaluate) ||
         rcspech(i, 15, 9, p, evaluate) ||
         rcspech(i, 9, 15, p, evaluate) ||
         rcspech(i, 10, 12, p, evaluate) ||
         rcspech(i, 12, 10, p, evaluate) ||
         rcspech(i, 12, 14, p, evaluate) ||
         rcspech(i, 14, 12, p, evaluate) ||
         rcspech(i, 14, 16, p, evaluate) ||
         rcspech(i, 16, 14, p, evaluate) ||
         rcspech(i, 16, 10, p, evaluate) ||
         rcspech(i, 10, 16, p, evaluate);
}


boolean rrefcech(square sq_king,
                 square i1,
                 int   x,
                 piece  p,
                 evalfunction_t *evaluate)
{
  numvec k;

  /* ATTENTION:   first call of rrefech: x must be 2 !!   */

  square sq_departure;

  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (x) {
      sq_departure= i1+vec[k];
      if (e[sq_departure]==vide) {
        if (!NoEdge(sq_departure)) {
          if (rrefcech(sq_king,sq_departure,x-1,p,evaluate))
            return true;
        }
      }
      else if (e[sq_departure]==p
               && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
    else
      for (k= vec_knight_start; k <= vec_knight_end; k++) {
        sq_departure= i1+vec[k];
        if (e[sq_departure]==p
            && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }

  return false;
}

static boolean rrefnech(square sq_king,
                        square i1,
                        piece  p,
                        evalfunction_t *evaluate)
{
  numvec k;

  square sq_departure;

  if (!NoEdge(i1))
    settraversed(i1);

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_departure= i1;

    while (e[sq_departure+=vec[k]]==vide)
    {
      if (!NoEdge(sq_departure) &&
          !traversed(sq_departure)) {
        if (rrefnech(sq_king,sq_departure,p,evaluate))
          return true;
        break;
      }
    }
    if (e[sq_departure] == p &&
        evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  return false;
}

boolean nequicheck(square   sq_king,
                   piece    p,
                   evalfunction_t *evaluate)
{
  /* check by non-stop equihopper? */
  numvec delta_horiz, delta_vert;
  numvec vector;
  square sq_hurdle;
  square sq_departure;

  square const coin= coinequis(sq_king);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {
      sq_hurdle= coin+delta_horiz+delta_vert;
      vector= sq_king-sq_hurdle;
      sq_departure= sq_hurdle-vector;

      if (e[sq_hurdle]!=vide
          && e[sq_departure]==p
          && sq_king!=sq_departure
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,vector,vector)))
        return true;
    }

  return false;
}

boolean norixcheck(square   sq_king,
                   piece    p,
                   evalfunction_t *evaluate)
{
  /* check by non-stop orix? */
  numvec delta_horiz, delta_vert, delta;
  numvec vector;
  square sq_hurdle;
  square sq_departure;
  boolean queenlike;

  square const coin= coinequis(sq_king);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {
      sq_hurdle= coin+delta_horiz+delta_vert;
      vector= sq_king-sq_hurdle;
      delta= abs(vector);
      queenlike= (delta <= 3*dir_right)
                  || (delta % onerow == 0)
		|| (delta % (onerow + dir_right) == 0)
		|| (delta % (onerow + dir_left) == 0);
      sq_departure= sq_hurdle-vector;

      if (queenlike
          && e[sq_hurdle]!=vide
          && e[sq_departure]==p
          && sq_king!=sq_departure
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,vector,vector)))
        return true;
    }

  return false;
}

boolean equifracheck(square sq_king,
                     piece  p,
                     evalfunction_t *evaluate)
{
  /* check by non-stop equistopper? */
  square sq_hurdle;
  square const *bnp;
  numvec vector;
  square sq_departure;

  for (bnp= boardnum; *bnp; bnp++)
  {
    sq_departure= *bnp;
    vector= sq_king-sq_departure;
    sq_hurdle= sq_king+vector;
    if (e[sq_hurdle]!=vide
        && e[sq_hurdle]!=obs
        && e[sq_departure]==p
        && sq_king!=sq_departure
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean vizircheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return leapcheck(i, vec_rook_start, vec_rook_end, p, evaluate);
}

boolean dabcheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return leapcheck(i, vec_dabbaba_start, vec_dabbaba_end, p, evaluate);
}

boolean ferscheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return leapcheck(i, vec_bishop_start, vec_bishop_end, p, evaluate);
}


boolean alfilcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return leapcheck(i, vec_alfil_start, vec_alfil_end, p, evaluate);
}

boolean rccinqcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_rccinq_start, vec_rccinq_end, p, evaluate);
}


boolean bucheck(square    i,
                piece p,
                evalfunction_t *evaluate)
{
  return leapcheck(i, vec_bucephale_start, vec_bucephale_end, p, evaluate);
}


boolean gicheck(square    i,
                piece p,
                evalfunction_t *evaluate)
{
  return leapcheck(i, vec_girafe_start, vec_girafe_end, p, evaluate);
}

boolean chcheck(square    i,
                piece p,
                evalfunction_t *evaluate)
{
  return leapcheck(i, vec_chameau_start, vec_chameau_end, p, evaluate);
}


boolean zcheck(square    i,
               piece p,
               evalfunction_t *evaluate)
{
  return leapcheck(i, vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean leap16check(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap16_start, vec_leap16_end, p, evaluate);
}

boolean leap24check(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap24_start, vec_leap24_end, p, evaluate);
}

boolean leap35check(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap35_start, vec_leap35_end, p, evaluate);
}

boolean leap37check(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap37_start, vec_leap37_end, p, evaluate);
}

boolean okapicheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return leapcheck(i, vec_okapi_start, vec_okapi_end, p, evaluate);   /* knight+zebra */
}

boolean bisoncheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return leapcheck(i, vec_bison_start, vec_bison_end, p, evaluate);    /* camel+zebra */
}

boolean zebucheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return (leapcheck(i,vec_chameau_start,vec_chameau_end,p,evaluate)
          || leapcheck(i,vec_girafe_start,vec_girafe_end,p,evaluate));
}

boolean elephantcheck(square    i,
                      piece p,
                      evalfunction_t *evaluate)
{
  return ridcheck(i, vec_elephant_start, vec_elephant_end, p, evaluate);    /* queen+nightrider  */
}

boolean ncheck(square    i,
               piece p,
               evalfunction_t *evaluate)
{
  return ridcheck(i, vec_knight_start, vec_knight_end, p, evaluate);
}

boolean scheck(square    i,
               piece p,
               evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean grasshop2check(square    i,
                       piece p,
                       evalfunction_t *evaluate)
{
  return rhop2check(i, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean grasshop3check(square    i,
                       piece p,
                       evalfunction_t *evaluate)
{
  return rhop3check(i, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean kinghopcheck(square    i,
                     piece p,
                     evalfunction_t *evaluate)
{
  return shopcheck(i, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean knighthoppercheck(square    i,
                          piece p,
                          evalfunction_t *evaluate)
{
    return shopcheck(i, vec_knight_start, vec_knight_end, p, evaluate);
}

static boolean doublehoppercheck(square  sq_king,
                                 piece   p,
                                 numvec vec_start, numvec vec_end,
                                 evalfunction_t *evaluate)
{
  piece double_hopper;
  square    sq_hurdle2, sq_hurdle1;
  numvec    k, k1;

  square sq_departure;

  for (k=vec_end; k>=vec_start; k--) {
    sq_hurdle2= sq_king+vec[k];
    if (abs(e[sq_hurdle2])>=roib) {
      sq_hurdle2+= vec[k];
      while (e[sq_hurdle2]==vide) {
        for (k1= vec_end; k1>=vec_start; k1--) {
          sq_hurdle1= sq_hurdle2+vec[k1];
          if (abs(e[sq_hurdle1]) >= roib) {
            finligne(sq_hurdle1,vec[k1],double_hopper,sq_departure);
            if (double_hopper==p
                && evaluate(sq_departure,sq_king,sq_king))
              return true;
          }
        }
        sq_hurdle2+= vec[k];
      }
    }
  }

  return false;
}

boolean doublegrasshoppercheck(square  sq_king,
                               piece   p,
                               evalfunction_t *evaluate)
{
  /* W.B.Trumper feenschach 1968 - but null moves will not be allowed by Popeye
   */
  return doublehoppercheck(sq_king,p,vec_queen_start,vec_queen_end,evaluate);
}

boolean doublerookhoppercheck(square  sq_king,
                              piece   p,
                              evalfunction_t *evaluate)
{
  return doublehoppercheck(sq_king,p,vec_rook_start,vec_rook_end,evaluate);
}

boolean doublebishoppercheck(square  sq_king,
                             piece   p,
                             evalfunction_t *evaluate)
{
  return doublehoppercheck(sq_king,p,vec_bishop_start,vec_bishop_end,evaluate);
}

boolean contragrascheck(square    i,
                        piece p,
                        evalfunction_t *evaluate)
{
  return crhopcheck(i, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean nightlocustcheck(square    i,
                         piece p,
                         evalfunction_t *evaluate)
{
  return marincheck(i, vec_knight_start, vec_knight_end, p, evaluate);
}

boolean loccheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return marincheck(i, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean tritoncheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return marincheck(i, vec_rook_start, vec_rook_end, p, evaluate);
}

boolean nereidecheck(square    i,
                     piece p,
                     evalfunction_t *evaluate)
{
  return marincheck(i, vec_bishop_start, vec_bishop_end, p, evaluate);
}

boolean nightriderlioncheck(square    i,
                            piece p,
                            evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_knight_start, vec_knight_end, p, evaluate);
}

boolean lioncheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean t_lioncheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean f_lioncheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

/* see comment in py4.c on how rose and rose based pieces are
 * handled */
boolean detect_rosecheck_on_line(square sq_king,
                                 piece p,
                                 numvec k, numvec k1,
                                 numvec delta_k,
                                 evalfunction_t *evaluate) {
  square sq_departure= fin_circle_line(sq_king,k,&k1,delta_k);
  return e[sq_departure]==p
      && sq_departure!=sq_king /* pieces don't give check to themselves */
      && evaluate(sq_departure,sq_king,sq_king);
}

boolean rosecheck(square    sq_king,
                  piece p,
                  evalfunction_t *evaluate)
{
  numvec  k;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    if (detect_rosecheck_on_line(sq_king,p,
                                 k,0,+1,
                                 evaluate))
      return true;
    if (detect_rosecheck_on_line(sq_king,p,
                                 k,vec_knight_end-vec_knight_start+1,-1,
                                 evaluate))
      return true;
  }

  return false;
}

boolean detect_roselioncheck_on_line(square sq_king,
                                     piece p,
                                     numvec k, numvec k1,
                                     numvec delta_k,
                                     evalfunction_t *evaluate) {
  square sq_hurdle= fin_circle_line(sq_king,k,&k1,delta_k);
  if (sq_hurdle!=sq_king && e[sq_hurdle]!=obs) {
    square sq_departure= fin_circle_line(sq_hurdle,k,&k1,delta_k);

#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    /* cf. issue 1747928 */
    if (sq_departure==sq_king && e[sq_hurdle]==p) {
      /* special case: king and rose lion are the only pieces on the
       * line -> king is hurdle, and what we thought to be the hurdle
       * is in fact the rose lion! */
      if (evaluate(sq_hurdle,sq_king,sq_king))
        return true;
    }
#endif

    if (e[sq_departure]==p
        && sq_departure!=sq_king /* pieces don't give check to themselves */
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean roselioncheck(square    sq_king,
                      piece p,
                      evalfunction_t *evaluate)
{
  /* detects check by a rose lion */
  numvec  k;
  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (detect_roselioncheck_on_line(sq_king,p,
                                     k,0,+1,
                                     evaluate)
        || detect_roselioncheck_on_line(sq_king,p,
                                        k,vec_knight_end-vec_knight_start+1,-1,
                                        evaluate))
      return true;

  return false;
}

boolean detect_rosehoppercheck_on_line(square sq_king,
                                       square sq_hurdle,
                                       piece p,
                                       numvec k, numvec k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate) {
  square sq_departure= fin_circle_line(sq_hurdle,k,&k1,delta_k);
  return e[sq_departure]==p
      && sq_departure!=sq_king
      && evaluate(sq_departure,sq_king,sq_king);
}

boolean rosehoppercheck(square  sq_king,
                        piece   p,
                        evalfunction_t *evaluate) {
  /* detects check by a rose hopper */
  numvec  k;
  square sq_hurdle;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_hurdle= sq_king+vec[k];
    if (e[sq_hurdle]!=vide && e[sq_hurdle]!=obs) {
      /* k1==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_rosehoppercheck_on_line(sq_king,sq_hurdle,p,
                                         k,1,+1,
                                         evaluate))
        return true;
      if (detect_rosehoppercheck_on_line(sq_king,sq_hurdle,p,
                                         k,vec_knight_end-vec_knight_start,-1,
                                         evaluate))
        return true;
    }
  }

  return false;
}

boolean detect_roselocustcheck_on_line(square sq_king,
                                       square sq_arrival,
                                       piece p,
                                       numvec k, numvec k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate) {
  square sq_departure= fin_circle_line(sq_king,k,&k1,delta_k);
  return e[sq_departure]==p
      && sq_departure!=sq_king
      && evaluate(sq_departure,sq_arrival,sq_king);
}

boolean roselocustcheck(square  sq_king,
                        piece   p,
                        evalfunction_t *evaluate) {
  /* detects check by a rose locust */
  numvec  k;
  square sq_arrival;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_arrival= sq_king-vec[k];
    if (e[sq_arrival]==vide) {
      /* k1==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_roselocustcheck_on_line(sq_king,sq_arrival,p,
                                         k,1,+1,
                                         evaluate))
        return true;
      if (detect_roselocustcheck_on_line(sq_king,sq_arrival,p,
                                         k,vec_knight_end-vec_knight_start,-1,
                                         evaluate))
        return true;
    }
  }

  return false;
}

boolean maocheck(square sq_king,
                 piece  p,
                 evalfunction_t *evaluate)
{
  square sq_departure;

  if (e[sq_king+dir_up+dir_right]==vide) {
    sq_departure= sq_king+dir_up+2*dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_right,
                            sq_king,
                            sq_king+dir_up+dir_right);
    }
    sq_departure= sq_king+2*dir_up+dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_right,
                            sq_king,
                            sq_king+dir_up+dir_right);
    }
  }

  if (e[sq_king+dir_down+dir_left]==vide) {
    sq_departure= sq_king+dir_down+2*dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_left,
                            sq_king,
                            sq_king+dir_down+dir_left);
    }
    sq_departure= sq_king+2*dir_down+dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_left,
                            sq_king,
                            sq_king+dir_down+dir_left);
    }
  }

  if (e[sq_king+dir_up+dir_left]==vide) {
    sq_departure= sq_king+dir_up+2*dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_left,
                            sq_king,
                            sq_king+dir_up+dir_left);
    }
    sq_departure= sq_king+2*dir_up+dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_left,
                            sq_king,
                            sq_king+dir_up+dir_left);
    }
  }

  if (e[sq_king+dir_down+dir_right]==vide) {
    sq_departure= sq_king+2*dir_down+dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_right,
                            sq_king,
                            sq_king+dir_down+dir_right);
    }
    sq_departure= sq_king+dir_down+2*dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_right,
                            sq_king,
                            sq_king+dir_down+dir_right);
    }
  }

  return false;
}

boolean moacheck(square sq_king,
                 piece  p,
                 evalfunction_t *evaluate)
{
  square sq_departure;

  if (e[sq_king+dir_up]==vide) {
    sq_departure= sq_king+2*dir_up+dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_left, sq_king, sq_king+dir_up);
    }
    sq_departure= sq_king+2*dir_up+dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_right, sq_king, sq_king+dir_up);
    }
  }
  if (e[sq_king+dir_down]==vide) {
    sq_departure= sq_king+2*dir_down+dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_right, sq_king, sq_king+dir_down);
    }
    sq_departure= sq_king+2*dir_down+dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_left, sq_king, sq_king+dir_down);
    }
  }
  if (e[sq_king+dir_right]==vide) {
    sq_departure= sq_king+dir_up+2*dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_right, sq_king, sq_king+dir_right);
    }
    sq_departure= sq_king+dir_down+2*dir_right;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_right, sq_king, sq_king+dir_right);
    }
  }
  if (e[sq_king+dir_left]==vide) {
    sq_departure= sq_king+dir_down+2*dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_left, sq_king, sq_king+dir_left);
    }
    sq_departure= sq_king+dir_up+2*dir_left;
    if (e[sq_departure]==p) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_left, sq_king, sq_king+dir_left);
    }
  }

  return false;
}

boolean paocheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean vaocheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean naocheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_knight_start,vec_knight_end, p, evaluate);
}

boolean leocheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean pbcheck(square  sq_king,
                piece   p,
                evalfunction_t *evaluate)
{
  if (CondFlag[phantom])
  {
    if ((p==e[king_square[White]] && e[sq_king+dir_down]==p)
        || (p==e[king_square[Black]] && e[sq_king+dir_up]==p))
      return true;
  }
  else if (anymars)
    return ((p==e[king_square[White]] && e[sq_king+dir_down]==p)
            || (p==e[king_square[Black]] && e[sq_king+dir_up]==p));

  if (p<=roin) {
    if (sq_king<=square_h6
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || p==orphann)
    {
      square sq_departure= sq_king+dir_up;

      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;

      if (ep[nbply]!=initsquare
          && prev_king_square[White][nbply]!=king_square[White]
          && (king_square[White]==ep[nbply]+dir_up+dir_left
              || king_square[White]==ep[nbply]+dir_up+dir_right)) {
        /* ep captures of royal pawns */
        sq_departure= ep[nbply]+dir_up;
        if (e[sq_departure]==pbn
            && evaluate(sq_departure,ep[nbply],sq_king))
          if (imcheck(sq_departure,ep[nbply]))
            return true;
      }
    }
  }
  else {      /* hopefully (p >= roib) */
    if (sq_king>=square_a3
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || p==orphanb)
    {
      square sq_departure= sq_king+dir_down;

      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;

      if (ep[nbply]!=initsquare
          && prev_king_square[Black][nbply]!=king_square[Black]
          && (king_square[Black]==ep[nbply]+dir_down+dir_right
              || king_square[Black]==ep[nbply]+dir_down+dir_left)) {
        /* ep captures of royal pawns */
        sq_departure= ep[nbply]+dir_down;
        if (e[sq_departure] == pbb
            && evaluate(sq_departure,ep[nbply],sq_king))
          if (imcheck(sq_departure,ep[nbply]))
            return true;
      }
    }
  }

  return false;
}

boolean bspawncheck(square  sq_king,
                    piece   p,
                    evalfunction_t *evaluate)
{
  piece   p1;

  square sq_departure;

  if (p==bspawnn
      || (calc_reflective_king[Black] && p==roin))
  {
    if (sq_king<=square_h7) {          /* it can move from eigth rank */
      finligne(sq_king,+dir_up,p1,sq_departure);
      if (p1==p && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }
  else {/* hopefully ((p == bspawnb)
           || (calc_whrefl_king && p == roib)) */
    if (sq_king>=square_a2) {          /* it can move from first rank */
      finligne(sq_king,+dir_down,p1,sq_departure);
      if (p1==p && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  return false;
}

boolean spawncheck(square   sq_king,
                   piece    p,
                   evalfunction_t *evaluate)
{
  piece   p1;

  square sq_departure;

  if (p==spawnn
      || (calc_reflective_king[Black] && p==roin))
  {
    if (sq_king<=square_h7) {          /* it can move from eigth rank */
      finligne(sq_king,dir_up+dir_left,p1,sq_departure);
      if (p1==p && evaluate(sq_departure,sq_king,sq_king))
        return true;

      finligne(sq_king,+dir_up+dir_right,p1,sq_departure);
      if (p1==p && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }
  else {/* hopefully ((p == bspawnb)
           || (calc_whrefl_king && p == roib)) */
    if (sq_king>=square_a2) {          /* it can move from first rank */
      finligne(sq_king,+dir_down+dir_right,p1,sq_departure);
      if (p1==p && evaluate(sq_departure,sq_king,sq_king))
        return true;

      finligne(sq_king,+dir_down+dir_left,p1,sq_departure);
      if (p1==p && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  return false;
}

boolean amazcheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return  leapcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean impcheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return  leapcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean princcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return  leapcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean gnoucheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return  leapcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || leapcheck(i, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean antilcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return leapcheck(i, vec_antilope_start, vec_antilope_end, p, evaluate);
}

boolean ecurcheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return  leapcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || leapcheck(i, vec_ecureuil_start, vec_ecureuil_end, p, evaluate);
}

boolean warancheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return  ridcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean dragoncheck(square  sq_king,
                    piece   p,
                    evalfunction_t *evaluate)
{
  if (leapcheck(sq_king,vec_knight_start,vec_knight_end,p,evaluate))
    return true;

  return pawnedpiececheck(sq_king, p, evaluate);
}

boolean gryphoncheck(square  sq_king,
                    piece   p,
                    evalfunction_t *evaluate)
{
  if (ridcheck(sq_king,vec_bishop_start,vec_bishop_end,p,evaluate))
    return true;

  return pawnedpiececheck(sq_king, p, evaluate);
}

boolean shipcheck(square  sq_king,
                    piece   p,
                    evalfunction_t *evaluate)
{
  if (ridcheck(sq_king,vec_rook_start,vec_rook_end,p,evaluate))
    return true;

  return pawnedpiececheck(sq_king, p, evaluate);
}

boolean pawnedpiececheck(square  sq_king,
                    piece   p,
                    evalfunction_t *evaluate)
{
  square sq_departure;

  if (p==dragonn || p==gryphonn || p==shipn
      || (calc_reflective_king[Black] && p==roin))

  {
    if (sq_king<=square_h6
        || anyparrain
        || CondFlag[normalp])
    {
      sq_departure= sq_king+dir_up+dir_left;
      if (e[sq_departure]==p) {
        if (evaluate(sq_departure,sq_king,sq_king))
          return true;
      }

      sq_departure= sq_king+dir_up+dir_right;
      if (e[sq_departure]==p) {
        if (evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }
  else {/* hopefully ((p == dragonb || p==gryphonb || p==shipb)
           || (calc_whrefl_king && p == roib)) */
    if (sq_king>=square_a3
        || anyparrain
        || CondFlag[normalp])
    {
      sq_departure= sq_king+dir_down+dir_right;
      if (e[sq_departure]==p) {
        if (evaluate(sq_departure,sq_king,sq_king))
          return true;
      }

      sq_departure= sq_king+dir_down+dir_left;
      if (e[sq_departure]==p) {
        if (evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean kangoucheck(square  sq_king,
                    piece   p,
                    evalfunction_t *evaluate)
{
  numvec  k;
  piece   p1;
  square sq_hurdle;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    sq_hurdle= sq_king+vec[k];
    if (abs(e[sq_hurdle])>=roib) {
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs) {
        finligne(sq_hurdle,vec[k],p1,sq_departure);
        if (p1==p && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean kanglioncheck(square  sq_king,
                      piece   p,
                      evalfunction_t *evaluate)
{
  numvec  k;
  piece   p1;
  square sq_hurdle;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (abs(e[sq_hurdle])>=roib) {
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs) {
        finligne(sq_hurdle,vec[k],p1,sq_departure);
        if (p1==p && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean rabbitcheck(square  sq_king,
                    piece   p,
                    evalfunction_t *evaluate)
{
  /* 2 hurdle lion */
  numvec  k;
  piece   p1;
  square sq_hurdle;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (abs(p1)>=roib) {
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs) {
        finligne(sq_hurdle,vec[k],p1,sq_departure);
        if (p1==p && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean bobcheck(square sq_king,
                 piece  p,
                 evalfunction_t *evaluate)
{
  /* 4 hurdle lion */
  numvec  k;
  piece   p1;
  square sq_hurdle;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (abs(p1)>=roib) {
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs) {
        finligne(sq_hurdle,vec[k],p1,sq_hurdle);
        if (p1!=obs) {
          finligne(sq_hurdle,vec[k],p1,sq_hurdle);
          if (p1!=obs) {
            finligne(sq_hurdle,vec[k],p1,sq_departure);
            if (p1==p && evaluate(sq_departure,sq_king,sq_king))
              return true;
          }
        }
      }
    }
  }

  return false;
}

boolean ubicheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  square const *bnp;

  if (evaluate == eval_madrasi) {
    for (bnp= boardnum; *bnp; bnp++) {
      e_ubi_mad[*bnp]= e[*bnp];
    }
    return rubiech(i, i, p, e_ubi_mad, evaluate);
  }
  else {
    for (bnp= boardnum; *bnp; bnp++) {
      e_ubi[*bnp]= e[*bnp];
    }
    return rubiech(i, i, p, e_ubi, evaluate);
  }
}

boolean moosecheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return rmhopech(i, vec_queen_end,vec_queen_start, angle_45, p, evaluate);
}

boolean eaglecheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return rmhopech(i, vec_queen_end,vec_queen_start, angle_90, p, evaluate);
}

boolean sparrcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return rmhopech(i, vec_queen_end,vec_queen_start, angle_135, p, evaluate);
}

boolean margueritecheck(square    i,
                        piece p,
                        evalfunction_t *evaluate)
{
  return  sparrcheck(i, p, evaluate)
      || eaglecheck(i, p, evaluate)
      || moosecheck(i, p, evaluate)
      || scheck(i, p, evaluate);
}

boolean leap36check(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap36_start, vec_leap36_end, p, evaluate);
}

boolean rookmoosecheck(square    i,
                       piece p,
                       evalfunction_t *evaluate)
{
  return rmhopech(i, vec_rook_end,vec_rook_start, angle_45, p, evaluate);
}

boolean rookeaglecheck(square    i,
                       piece p,
                       evalfunction_t *evaluate)
{
  return rmhopech(i, vec_rook_end,vec_rook_start, angle_90, p, evaluate);
}

boolean rooksparrcheck(square    i,
                       piece p,
                       evalfunction_t *evaluate)
{
  return rmhopech(i, vec_rook_end,vec_rook_start, angle_135, p, evaluate);
}

boolean bishopmoosecheck(square    i,
                         piece p,
                         evalfunction_t *evaluate)
{
  return rmhopech(i, vec_bishop_end,vec_bishop_start, angle_45, p, evaluate);
}

boolean bishopeaglecheck(square    i,
                         piece p,
                         evalfunction_t *evaluate)
{
  return rmhopech(i, vec_bishop_end,vec_bishop_start, angle_90, p, evaluate);
}

boolean bishopsparrcheck(square    i,
                         piece p,
                         evalfunction_t *evaluate)
{
  return rmhopech(i, vec_bishop_end,vec_bishop_start, angle_135, p, evaluate);
}

boolean archcheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  numvec  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(i, i, vec[k], p, 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean reffoucheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  numvec  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(i, i, vec[k], p, 4, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean cardcheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  numvec  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rcardech(i, i, vec[k], p, 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean nsautcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_knight_start,vec_knight_end, p, evaluate);
}

boolean camridcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return ridcheck(i, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean zebridcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return ridcheck(i, vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean gnuridcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return  ridcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(i, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean camhopcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean zebhopcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean gnuhopcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return  rhopcheck(i, vec_knight_start,vec_knight_end, p, evaluate)
      || rhopcheck(i, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean dcscheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  numvec  k;

  for (k= vec_knight_start; k <= 14; k++) {
    if (rcsech(i, vec[k], vec[23 - k], p, evaluate)) {
      return true;
    }
  }
  for (k= 15; k <= vec_knight_end; k++) {
    if (rcsech(i, vec[k], vec[27 - k], p, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean refccheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return rrefcech(i, i, 2, p, evaluate);
}

boolean refncheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  clearedgestraversed();
  return rrefnech(i, i, p, evaluate);
}

boolean equicheck(square    sq_king,
                  piece p,
                  evalfunction_t *evaluate)
{
  numvec  k;
  piece   p1;
  square  sq_hurdle;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (p1!=obs) {
      finligne(sq_hurdle,vec[k],p1,sq_departure);
      if (p1==p
          && sq_departure-sq_hurdle==sq_hurdle-sq_king
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-vec[k],-vec[k])))
        return true;
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {      /* 2,4; 2,6; 4,6; */
    sq_departure= sq_king+2*vec[k];
    if (abs(e[sq_king+vec[k]])>=roib
        && e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king)
        && (!checkhopim || hopimok(sq_departure,sq_king,sq_departure-vec[k],-vec[k],-vec[k])))
      return true;
  }

  return false;
}

boolean equiengcheck(square sq_king,
                     piece  p,
                     evalfunction_t *evaluate)
{
  numvec  k;
  piece   p1;
  square  sq_hurdle;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (p1!=obs) {
      finligne(sq_king,-vec[k],p1,sq_departure);
      if (p1==p
          && sq_departure-sq_king==sq_king-sq_hurdle
          && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {      /* 2,4; 2,6; 4,6; */
    sq_departure= sq_king-vec[k];
    sq_hurdle= sq_king+vec[k];
    if (abs(e[sq_hurdle])>=roib
        && e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean catcheck(square sq_king,
                 piece  p,
                 evalfunction_t *evaluate)
{
  numvec  k;
  square  middle_square;

  square sq_departure;

  if (leapcheck(sq_king,vec_knight_start,vec_knight_end,p,evaluate)) {
    return true;
  }

  for (k= vec_dabbaba_start; k<=vec_dabbaba_end; k++) {
    middle_square= sq_king+vec[k];
    while (e[middle_square]==vide) {
      sq_departure= middle_square+mixhopdata[3][k-60];
      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;

      sq_departure= middle_square+mixhopdata[3][k-56];
      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;

      middle_square+= vec[k];
    }
  }

  return false;
}

boolean roicheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return leapcheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean cavcheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return leapcheck(i, vec_knight_start,vec_knight_end, p, evaluate);
}

boolean damecheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return ridcheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean tourcheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return ridcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean foucheck(square    i,
                 piece p,
                 evalfunction_t *evaluate)
{
  return ridcheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean pioncheck(square sq_king,
                  piece p,
                  evalfunction_t *evaluate)
{
  square sq_departure;

  if (p<=roin) {
    if (sq_king<=square_h6
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || p==orphann
        || p<=hunter0n)
    {
      sq_departure= sq_king+dir_up+dir_left;
      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;

      sq_departure= sq_king+dir_up+dir_right;
      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }
  else {      /* hopefully (p >= roib) */
    if (sq_king>=square_a3
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || p==orphanb
        || p>=hunter0b)
    {
      sq_departure= sq_king+dir_down+dir_right;
      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;

      sq_departure= sq_king+dir_down+dir_left;
      if (e[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  return false;
}

boolean reversepcheck(square sq_king,
                      piece p,
                      evalfunction_t *evaluate)
{
  if (anymars || CondFlag[phantom]) {
    boolean anymarscheck=
        (p==e[king_square[White]]
         && (e[sq_king+dir_down+dir_right]==p
             || e[sq_king+dir_down+dir_left]==p))
        || (p==e[king_square[Black]]
            && (e[sq_king+dir_up+dir_right]==p
                || e[sq_king+dir_up+dir_left]==p));
    if (!is_phantomchess || anymarscheck) {
      return anymarscheck;
    }
  }

  if (p >= roib) {
    if (sq_king <= square_h8 - 2*onerow
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || p == orphanb
        || p>=hunter0b)
    {
      if (e[sq_king+dir_up+dir_right]==p) {
        if ((*evaluate)(sq_king+dir_up+dir_right,sq_king,sq_king)) {
          return true;
        }
      }
      if (e[sq_king+dir_up+dir_left]==p) {
        if ((*evaluate)(sq_king+dir_up+dir_left,sq_king,sq_king)) {
          return true;
        }
      }
    }
  }
  else {      /* hopefully (p <= roin) */
    if (sq_king >= square_a1 + 2*onerow
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage]
        || p == orphann
        || p<=hunter0n)
    {
      if (e[sq_king+dir_down+dir_right]==p) {
        if ((*evaluate)(sq_king+dir_down+dir_right,sq_king,sq_king)) {
          return true;
        }
      }
      if (e[sq_king+dir_down+dir_left]==p) {
        if ((*evaluate)(sq_king+dir_down+dir_left,sq_king,sq_king)) {
          return true;
        }
      }
    }
  }
  return false;
}

boolean soutenu(square sq_departure, square sq_arrival, square sq_capture)
{
  piece p= 0;       /* avoid compiler warning */
  boolean Result,
          enemyobserveok=true,
          friendobserveok=true,
          testenemyobs=false,
          testfriendobs=false,
          testfriendanti;
  evalfunction_t *evaluate;

  if (CondFlag[central]) {
    if ( sq_departure == king_square[White] || sq_departure == king_square[Black]) {
      return true;
    }
    nbpiece[p= e[sq_departure]]--;
    e[sq_departure]= (p > vide) ? dummyb : dummyn;
    evaluate= soutenu;
  }
  else if (flaglegalsquare) {
    if (!legalsquare(sq_departure,sq_arrival,sq_capture)) {
      return false;
    }
    evaluate= legalsquare;
  }
  else if (CondFlag[madras] || CondFlag[eiffel]) {
    if (!eval_madrasi(sq_departure,sq_arrival,sq_capture)) {
      return false;
    }
    evaluate= eval_madrasi;
  }
  else if (TSTFLAG(PieSpExFlags,Paralyse)) {
    if (!paraechecc(sq_departure,sq_arrival,sq_capture)) {
      return false;
    }
    evaluate= paraechecc;
  }
  else {
    evaluate= eval_ortho;
  }

  /* logic rewritten to simplify new conditions and allow combinations
   * interpretation here is:
   * A piece with a special observation variant piece type will obey
   * the types and not any conditions in force; all other pieces obey
   * global conditions in force
   * If there are both enemy-observation and friend-observation rules
   * for a piece, it has to satisfy both
   * This interpretation can be changed by altering logic here
   * New variants e.g. Anti-Provacateurs and piece types not implemented
   * yet but can be set up by setting up obs* flags in verifieposition
   * and amending the macros below (in py.h)
   *   enemy/friend determines if rule concerns observation by other/own side
   *   anti true if should NOT be observed as in Lortap
   *   ultra (see py4.c) true if observation applies also to non-capture moves
   * two other conditions (central, shielded kings) also use this code */

  if (TSTFLAG(PieSpExFlags,Beamtet) || TSTFLAG(PieSpExFlags,Patrol))
  {
    testenemyobs= TSTFLAG(spec[sq_departure], Beamtet);
    testfriendobs= TSTFLAG(spec[sq_departure], Patrol);
    if (testfriendobs)
      testfriendanti = false;
  }
  if (!testenemyobs && !testfriendobs) {
    testenemyobs= CondFlag[beamten] || CondFlag[provocateurs];
    testfriendobs= CondFlag[patrouille] || CondFlag[central] || CondFlag[ultrapatrouille] || CondFlag[lortap] || CondFlag[shieldedkings];
    testfriendanti= CondFlag[lortap];
  }

  if (testenemyobs)
  {
    Side const side = e[sq_departure]<=roin ? Black : White;
    sq_arrival = king_square[side];
    king_square[side] = sq_departure;
    enemyobserveok = rechec[side](evaluate);
    king_square[side] = sq_arrival;
  }

  if (testfriendobs)
  {
    Side const side = e[sq_departure]>=roib ? Black : White;
    sq_arrival = king_square[side];
    king_square[side] = sq_departure;
    friendobserveok = testfriendanti ^ rechec[side](evaluate);
    king_square[side] = sq_arrival;
  }

  Result = enemyobserveok && friendobserveok;

  if (CondFlag[central])
    nbpiece[e[sq_departure]= p]++;

  return(Result);
} /* soutenu */

boolean eval_shielded(square sq_departure, square sq_arrival, square sq_capture) {
  if ((sq_departure==king_square[Black] && sq_capture==king_square[White])
      || (sq_departure==king_square[White] && sq_capture==king_square[Black])) {
    return !soutenu(sq_capture,sq_departure,sq_departure);  /* won't work for locust Ks etc.*/
  }
  else {
    return true;
  }
} /* eval_shielded */

boolean edgehcheck(square   sq_king,
                   piece    p,
                   evalfunction_t *evaluate)
{
  /* detect "check" of edgehog p */
  piece p1;
  numvec  k;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_king,vec[k],p1,sq_departure);
    if (p1==p
        && NoEdge(sq_king)!=NoEdge(sq_departure)
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

static boolean maooaridercheck(square  sq_king,
                               piece   p,
                               numvec  fir,
                               numvec  sec,
                               evalfunction_t *evaluate)
{
  square  middle_square;

  square sq_departure= sq_king+sec;

  middle_square = sq_king+fir;
  while (e[middle_square]==vide && e[sq_departure]==vide) {
    middle_square+= sec;
    sq_departure+= sec;
  }

  return e[middle_square]==vide
      && e[sq_departure]==p
      && evaluate(sq_departure,sq_king,sq_king);
}

boolean moaridercheck(square    i,
                      piece p,
                      evalfunction_t *evaluate)
{
  if (maooaridercheck(i, p, +dir_up,+2*dir_up+dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p, +dir_up,+2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_down,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_down,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p, +dir_right,+dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p, +dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_left,+dir_up+2*dir_left, evaluate)) {
    return true;
  }
  return false;
}

boolean maoridercheck(square    i,
                      piece p,
                      evalfunction_t *evaluate)
{
  if (maooaridercheck(i, p,+dir_up+dir_right,+2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_up+dir_right,+dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_down+dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_down+dir_right,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_down+dir_left,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,+dir_down+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,  dir_up+dir_left,+dir_up+2*dir_left, evaluate)) {
    return true;
  }
  if (maooaridercheck(i, p,  dir_up+dir_left,+2*dir_up+dir_left, evaluate)) {
    return true;
  }
  return false;
}

static boolean maooariderlioncheck(square  sq_king,
                                   piece   p,
                                   numvec  fir,
                                   numvec  sec,
                                   evalfunction_t *evaluate)
{
  square middle_square= sq_king+fir;

  square sq_departure= sq_king+sec;

  while (e[middle_square]==vide && e[sq_departure]==vide) {
    middle_square+= sec;
    sq_departure+= sec;
  }
  if (e[middle_square]!=vide
      && e[sq_departure]==p
      && evaluate(sq_departure,sq_king,sq_king))
    return true;

  if (e[middle_square]!=obs
      && e[sq_departure]!=obs
      && (e[middle_square]==vide || e[sq_departure]==vide))
  {
    middle_square+= sec;
    sq_departure+= sec;
    while (e[middle_square]==vide && e[sq_departure]==vide) {
      middle_square+= sec;
      sq_departure+= sec;
    }
    if (e[middle_square]==vide
        && e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean maoriderlioncheck(square    i,
                          piece p,
                          evalfunction_t *evaluate)
{
  if (maooariderlioncheck(i, p,+dir_up+dir_right,   +2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_up+dir_right,   +dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_down+dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_down+dir_right,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_down+dir_left,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_down+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,  dir_up+dir_left,   +dir_up+2*dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,  dir_up+dir_left,   +2*dir_up+dir_left, evaluate)) {
    return true;
  }
  return false;
}

boolean moariderlioncheck(square    i,
                          piece p,
                          evalfunction_t *evaluate)
{
  if (maooariderlioncheck(i, p, +dir_up,+2*dir_up+dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p, +dir_up,+2*dir_up+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_down,+2*dir_down+dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_down,+2*dir_down+dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_right,+dir_up+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_right,+dir_down+2*dir_right, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_left,+dir_down+2*dir_left, evaluate)) {
    return true;
  }
  if (maooariderlioncheck(i, p,+dir_left,+dir_up+2*dir_left, evaluate)) {
    return true;
  }
  return false;
}

boolean r_hopcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean b_hopcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

static Side guess_side_at_move(square sq_departure, square sq_capture)
{
  Side result;

  /* the following does not suffice if we have neutral kings,
     but we have no chance to recover the information who is to
     move from sq_departure, sq_arrival and sq_capture.
     TLi
  */
  if (TSTFLAG(PieSpExFlags,Neutral)
      && king_square[White]!=initsquare
      && TSTFLAG(spec[king_square[White]],Neutral))
    /* will this do for neutral Ks? */
    result = neutral_side;
  else if (sq_capture==king_square[Black])
    result = White;
  else if (sq_capture==king_square[White])
    result = Black;
  else
    result = e[sq_departure]<0 ? Black : White;

  return result;
}

boolean eval_isardam(square sq_departure, square sq_arrival, square sq_capture)
{
  boolean result;
  Side side;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  side = guess_side_at_move(sq_departure,sq_capture);

  init_single_move_generator(sq_departure,sq_arrival,sq_capture);
  result = solve(slices[temporary_hack_king_capture_legality_tester[side]].next2,length_unspecified)==next_move_has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean eval_brunner(square sq_departure, square sq_arrival, square sq_capture)
{
  boolean result;
  Side side;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  side = guess_side_at_move(sq_departure,sq_capture);

  init_single_move_generator(sq_departure,sq_arrival,sq_capture);
  result = solve(slices[temporary_hack_brunner_check_defense_finder[side]].next2,length_unspecified)==next_move_has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean orixcheck(square sq_king,
                  piece p,
                  evalfunction_t *evaluate)
{
  numvec  k;
  piece   p1;
  square  sq_hurdle;

  square sq_departure;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (p1!=obs) {
      finligne(sq_hurdle,vec[k],p1,sq_departure);
      if (p1==p
          && sq_departure-sq_hurdle==sq_hurdle-sq_king
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-vec[k],-vec[k])))
        return true;
    }
  }

  return false;
}

boolean leap15check(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap15_start, vec_leap15_end, p, evaluate);
}

boolean leap25check(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap25_start, vec_leap25_end, p, evaluate);
}

boolean gralcheck(square    i,
                  piece p,
                  evalfunction_t *evaluate)
{
  return leapcheck(i, vec_alfil_start, vec_alfil_end, p, evaluate)
      || rhopcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}


boolean scorpioncheck(square    i,
                      piece p,
                      evalfunction_t *evaluate)
{
  return  leapcheck(i, vec_queen_start,vec_queen_end, p, evaluate)
      || rhopcheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean dolphincheck(square    i,
                     piece p,
                     evalfunction_t *evaluate)
{
  return  rhopcheck(i, vec_queen_start,vec_queen_end, p, evaluate)
      || kangoucheck(i, p, evaluate);
}

boolean querquisitecheck(square sq_king,
                         piece p,
                         evalfunction_t *evaluate)
{
  numvec k;
  int file_departure;
  piece p1;

  square sq_departure;

  for (k= vec_rook_start; k<=vec_rook_end; k++) {
    finligne(sq_king,vec[k],p1,sq_departure);
    file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if ((file_departure==file_rook_queenside
         || file_departure==file_queen
         || file_departure==file_rook_kingside)
        && p1==p
        && evaluate(sq_departure,sq_king,sq_king)
        && ridimcheck(sq_departure,sq_king,vec[k]))
      return true;
  }

  for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
    finligne(sq_king,vec[k],p1,sq_departure);
    file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if ((file_departure==file_bishop_queenside
         || file_departure==file_queen
         || file_departure==file_bishop_kingside)
        && p1==p
        && evaluate(sq_departure,sq_king,sq_king)
        && ridimcheck(sq_departure,sq_king,vec[k]))
      return true;
  }

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_departure= sq_king+vec[k];
    file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if (e[sq_departure]==p
        && (file_departure==file_knight_queenside
            || file_departure==file_knight_kingside)
        && evaluate(sq_departure,sq_king,sq_king)
        && imcheck(sq_departure,sq_king))
      return true;
  }

  for (k= vec_queen_start; k<=vec_queen_end; k++) {
    sq_departure= sq_king+vec[k];
    file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if (e[sq_departure]==p
        && file_departure==file_king
        && evaluate(sq_departure,sq_king,sq_king)
        && imcheck(sq_departure,sq_king))
      return true;
  }

  return false;
}

static boolean bouncerfamilycheck(square sq_king,
                                  numvec kbeg,
                                  numvec kend,
                                  piece    p,
                                  evalfunction_t *evaluate)
{
  numvec  k;
  piece   p1,p2;
  square  sq_hurdle;

  square sq_departure;

  for (k= kend; k>=kbeg; k--) {
    finligne(sq_king,vec[k],p1,sq_departure);
    finligne(sq_departure,vec[k],p2,sq_hurdle);  /* p2 can be obs - bounces off edges */
    if (sq_departure-sq_king==sq_hurdle-sq_departure
        && p1==p
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean bouncercheck(square    i,
                     piece p,
                     evalfunction_t *evaluate)
{
  return bouncerfamilycheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean rookbouncercheck(square    i,
                         piece p,
                         evalfunction_t *evaluate)
{
  return bouncerfamilycheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean bishopbouncercheck(square    i,
                           piece p,
                           evalfunction_t *evaluate)
{
  return bouncerfamilycheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean pchincheck(square sq_king,
                   piece    p,
                   evalfunction_t *evaluate)
{
  square sq_departure;

  boolean const is_black= p<=roin;

  sq_departure= sq_king + (is_black ? dir_up :dir_down);
  if (e[sq_departure]==p
      && evaluate(sq_departure,sq_king,sq_king))
    return true;

  /* chinese pawns can capture side-ways if standing on the half of
   * the board farther away from their camp's base line (i.e. if
   * black, on the lower half, if white on the upper half) */
  if ((sq_king*2<(square_h8+square_a1)) == is_black) {
    sq_departure= sq_king+dir_right;
    if (e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king))
      return true;

    sq_departure= sq_king+dir_left;
    if (e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}


square fromspecificsquare;

boolean eval_fromspecificsquare(square sq_departure, square sq_arrival, square sq_capture) {
  return
      sq_departure==fromspecificsquare
      && (e[sq_departure]>vide ? eval_white : eval_black)(sq_departure,sq_arrival,sq_capture);
}

boolean qlinesradialcheck(square    sq_king,
                          piece p,
                          evalfunction_t *evaluate,
                          int hurdletype,
                          boolean leaf)

{
  return leapleapcheck(sq_king, vec_rook_start, vec_rook_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_dabbaba_start, vec_dabbaba_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap03_start,vec_leap03_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap04_start,vec_leap04_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap05_start,vec_leap05_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap06_start,vec_leap06_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap07_start,vec_leap07_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_bishop_start,vec_bishop_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_alfil_start,vec_alfil_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap33_start,vec_leap33_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap44_start,vec_leap44_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap55_start,vec_leap55_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap66_start,vec_leap66_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap77_start,vec_leap77_end, hurdletype, leaf, p, evaluate);
}

boolean radialcheck(square    sq_king,
                          piece p,
                          evalfunction_t *evaluate,
                          int hurdletype,
                          boolean leaf)

{
  return leapleapcheck(sq_king, vec_rook_start,vec_rook_end,hurdletype, leaf,p,evaluate)
      || leapleapcheck(sq_king, vec_dabbaba_start,vec_dabbaba_end,hurdletype, leaf,p,evaluate)
      || leapleapcheck(sq_king, vec_leap03_start,vec_leap03_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap04_start,vec_leap04_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_bucephale_start,vec_bucephale_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap06_start,vec_leap06_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap07_start,vec_leap07_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_bishop_start,vec_bishop_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_knight_start,vec_knight_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_chameau_start,vec_chameau_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_girafe_start,vec_girafe_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap15_start,vec_leap15_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap16_start,vec_leap16_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_rccinq_start,vec_rccinq_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_alfil_start,vec_alfil_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_zebre_start,vec_zebre_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap24_start,vec_leap24_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap25_start,vec_leap25_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap26_start,vec_leap26_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap27_start,vec_leap27_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap33_start,vec_leap33_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap35_start,vec_leap35_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap36_start,vec_leap36_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap37_start,vec_leap37_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap44_start,vec_leap44_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap45_start,vec_leap45_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap46_start,vec_leap46_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap47_start,vec_leap47_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap56_start,vec_leap56_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap57_start,vec_leap57_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap66_start,vec_leap66_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap67_start,vec_leap67_end, hurdletype, leaf, p, evaluate)
      || leapleapcheck(sq_king, vec_leap77_start,vec_leap77_end, hurdletype, leaf, p, evaluate);
}

boolean radialknightcheck(square    sq_king,
                          piece p,
                          evalfunction_t *evaluate)
{
  return radialcheck(sq_king, p, evaluate, 0, false);
}

boolean treehoppercheck(square    sq_king,
                          piece p,
                          evalfunction_t *evaluate)
{
  return qlinesradialcheck(sq_king, p, evaluate, 1, false);
}

boolean leafhoppercheck(square    sq_king,
                          piece p,
                          evalfunction_t *evaluate)
{
  return qlinesradialcheck(sq_king, p, evaluate, 1, true);
}

boolean greatertreehoppercheck(square    sq_king,
                          piece p,
                          evalfunction_t *evaluate)
{
  return radialcheck(sq_king, p, evaluate, 1, false);
}

boolean greaterleafhoppercheck(square    sq_king,
                          piece p,
                          evalfunction_t *evaluate)
{
  return radialcheck(sq_king, p, evaluate, 1, true);
}
