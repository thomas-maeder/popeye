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
#include "solving/en_passant.h"
#include "solving/observation.h"
#include "stipulation/temporary_hacks.h"
#include "pieces/pawns/pawn.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/roses.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

boolean eval_ortho(square sq_departure, square sq_arrival, square sq_capture) {
  return true;
}

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
                 vec_index_type kend, vec_index_type kanf,
                 angle_t angle,
                 PieNam p,
                 evalfunction_t *evaluate)
{
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

  vec_index_type k;
  square sq_departure;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParam ("%u",kend);
  TraceFunctionParam ("%u",kanf);
  TraceFunctionParam ("%u",angle);
  TracePiece(p);
  TraceFunctionParamListEnd();

  for (k = kend; k>=kanf; --k)
  {
    numvec const v = vec[k];
    square const sq_hurdle = sq_king+v;
    TraceValue("%d",v);
    TraceSquare(sq_hurdle);
    TracePiece(e[sq_hurdle]);
    TraceText("\n");
    if (abs(e[sq_hurdle])>=King)
    {
      vec_index_type k1 = 2*k;
      piece hopper;
      numvec v1 = mixhopdata[angle][k1];

      finligne(sq_hurdle,v1,hopper,sq_departure);
      TraceSquare(sq_departure);
      TracePiece(hopper);
      TraceValue("%d\n",v1);
      if (abs(hopper)==p && TSTFLAG(spec[sq_departure],trait[nbply]))
      {
        if (evaluate(sq_departure,sq_king,sq_king)
            && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-v1,-v)))
        {
          result = true;
          break;
        }
      }

      v1 = mixhopdata[angle][k1-1];
      finligne(sq_hurdle,v1,hopper,sq_departure);
      TraceSquare(sq_departure);
      TracePiece(hopper);
      TraceValue("%d\n",v1);
      if (abs(hopper)==p && TSTFLAG(spec[sq_departure],trait[nbply])) {
        if (evaluate(sq_departure,sq_king,sq_king)
            && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-v1,-v)))
        {
          result = true;
          break;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean rcsech(square  sq_king,
               numvec  k,
               numvec  k1,
               PieNam p,
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

  if (abs(e[sq_departure])==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
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

  if (abs(e[sq_departure])==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && evaluate(sq_departure,sq_arrival,sq_capture))
    return true;

  return false;
}

boolean rcspech(square  sq_king,
               numvec  k,
               numvec  k1,
               PieNam p,
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

  if (abs(e[sq_departure])==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && evaluate(sq_departure,sq_arrival,sq_capture))
    return true;

  return false;
}

boolean nevercheck(square  sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return false;
}

boolean cscheck(square  sq_king,
                PieNam p,
                evalfunction_t *evaluate)
{
  vec_index_type const sum = vec_knight_start+vec_knight_end;
  vec_index_type k;

  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (rcsech(sq_king, vec[k], vec[sum-k], p, evaluate))
      return true;

  return false;
}

boolean bscoutcheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  vec_index_type const sum = vec_bishop_start+vec_bishop_end;
  vec_index_type k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    if (rcsech(sq_king, vec[k], vec[sum-k], p, evaluate))
      return true;

  return false;
}

boolean gscoutcheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  vec_index_type const sum = vec_rook_start+vec_rook_end;
  vec_index_type k;

  for (k= vec_rook_end; k >= vec_rook_start; k--)
    if (rcsech(sq_king, vec[k], vec[sum-k], p, evaluate))
      return true;

  return false;
}

boolean sp40check(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return rcspech(sq_king, 9, 16, p, evaluate) ||
         rcspech(sq_king, 10, 11, p, evaluate) ||
         rcspech(sq_king, 11, 10, p, evaluate) ||
         rcspech(sq_king, 12, 13, p, evaluate) ||
         rcspech(sq_king, 13, 12, p, evaluate) ||
         rcspech(sq_king, 14, 15, p, evaluate) ||
         rcspech(sq_king, 15, 14, p, evaluate) ||
         rcspech(sq_king, 16, 9, p, evaluate);
}

boolean sp20check(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return rcspech(sq_king, 9, 12, p, evaluate) ||
         rcspech(sq_king, 10, 15, p, evaluate) ||
         rcspech(sq_king, 11, 14, p, evaluate) ||
         rcspech(sq_king, 12, 9, p, evaluate) ||
         rcspech(sq_king, 13, 16, p, evaluate) ||
         rcspech(sq_king, 14, 11, p, evaluate) ||
         rcspech(sq_king, 15, 10, p, evaluate) ||
         rcspech(sq_king, 16, 13, p, evaluate);
}

boolean sp33check(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return rcspech(sq_king, 9, 10, p, evaluate) ||
         rcspech(sq_king, 10, 9, p, evaluate) ||
         rcspech(sq_king, 11, 12, p, evaluate) ||
         rcspech(sq_king, 12, 11, p, evaluate) ||
         rcspech(sq_king, 13, 14, p, evaluate) ||
         rcspech(sq_king, 14, 13, p, evaluate) ||
         rcspech(sq_king, 15, 16, p, evaluate) ||
         rcspech(sq_king, 16, 15, p, evaluate);
}

boolean sp11check(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return rcspech(sq_king, 9, 14, p, evaluate) ||
         rcspech(sq_king, 14, 9, p, evaluate) ||
         rcspech(sq_king, 10, 13, p, evaluate) ||
         rcspech(sq_king, 13, 10, p, evaluate) ||
         rcspech(sq_king, 11, 16, p, evaluate) ||
         rcspech(sq_king, 16, 11, p, evaluate) ||
         rcspech(sq_king, 12, 15, p, evaluate) ||
         rcspech(sq_king, 15, 12, p, evaluate);
}


boolean sp31check(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return rcspech(sq_king, 9, 11, p, evaluate) ||
         rcspech(sq_king, 11, 9, p, evaluate) ||
         rcspech(sq_king, 11, 13, p, evaluate) ||
         rcspech(sq_king, 13, 11, p, evaluate) ||
         rcspech(sq_king, 13, 15, p, evaluate) ||
         rcspech(sq_king, 15, 13, p, evaluate) ||
         rcspech(sq_king, 15, 9, p, evaluate) ||
         rcspech(sq_king, 9, 15, p, evaluate) ||
         rcspech(sq_king, 10, 12, p, evaluate) ||
         rcspech(sq_king, 12, 10, p, evaluate) ||
         rcspech(sq_king, 12, 14, p, evaluate) ||
         rcspech(sq_king, 14, 12, p, evaluate) ||
         rcspech(sq_king, 14, 16, p, evaluate) ||
         rcspech(sq_king, 16, 14, p, evaluate) ||
         rcspech(sq_king, 16, 10, p, evaluate) ||
         rcspech(sq_king, 10, 16, p, evaluate);
}


boolean rrefcech(square sq_king,
                 square i1,
                 int   x,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  vec_index_type k;

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
      else if (abs(e[sq_departure])==p
               && TSTFLAG(spec[sq_departure],trait[nbply])
               && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
    else
      for (k= vec_knight_start; k <= vec_knight_end; k++) {
        sq_departure= i1+vec[k];
        if (abs(e[sq_departure])==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }

  return false;
}

static boolean rrefnech(square sq_king,
                        square i1,
                        PieNam p,
                        evalfunction_t *evaluate)
{
  vec_index_type k;

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
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  return false;
}

boolean nequicheck(square   sq_king,
                   PieNam p,
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
          && abs(e[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_king!=sq_departure
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,vector,vector)))
        return true;
    }

  return false;
}

boolean norixcheck(square   sq_king,
                   PieNam p,
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
          && abs(e[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_king!=sq_departure
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,vector,vector)))
        return true;
    }

  return false;
}

boolean equifracheck(square sq_king,
                     PieNam p,
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
        && abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && sq_king!=sq_departure
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean vizircheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_rook_start, vec_rook_end, p, evaluate);
}

boolean dabcheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_dabbaba_start, vec_dabbaba_end, p, evaluate);
}

boolean ferscheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_bishop_start, vec_bishop_end, p, evaluate);
}


boolean alfilcheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_alfil_start, vec_alfil_end, p, evaluate);
}

boolean rccinqcheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_rccinq_start, vec_rccinq_end, p, evaluate);
}


boolean bucheck(square    sq_king,
                PieNam p,
                evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_bucephale_start, vec_bucephale_end, p, evaluate);
}


boolean gicheck(square    sq_king,
                PieNam p,
                evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_girafe_start, vec_girafe_end, p, evaluate);
}

boolean chcheck(square    sq_king,
                PieNam p,
                evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_chameau_start, vec_chameau_end, p, evaluate);
}


boolean zcheck(square    sq_king,
               PieNam p,
               evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean leap16check(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_leap16_start, vec_leap16_end, p, evaluate);
}

boolean leap24check(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_leap24_start, vec_leap24_end, p, evaluate);
}

boolean leap35check(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_leap35_start, vec_leap35_end, p, evaluate);
}

boolean leap37check(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_leap37_start, vec_leap37_end, p, evaluate);
}

boolean okapicheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_okapi_start, vec_okapi_end, p, evaluate);   /* knight+zebra */
}

boolean bisoncheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_bison_start, vec_bison_end, p, evaluate);    /* camel+zebra */
}

boolean zebucheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return (leapcheck(sq_king,vec_chameau_start,vec_chameau_end,p,evaluate)
          || leapcheck(sq_king,vec_girafe_start,vec_girafe_end,p,evaluate));
}

boolean elephantcheck(square    sq_king,
                      PieNam p,
                      evalfunction_t *evaluate)
{
  return ridcheck(sq_king, vec_elephant_start, vec_elephant_end, p, evaluate);    /* queen+nightrider  */
}

boolean ncheck(square    sq_king,
               PieNam p,
               evalfunction_t *evaluate)
{
  return ridcheck(sq_king, vec_knight_start, vec_knight_end, p, evaluate);
}

boolean scheck(square    sq_king,
               PieNam p,
               evalfunction_t *evaluate)
{
  return rhopcheck(sq_king, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean grasshop2check(square    sq_king,
                       PieNam p,
                       evalfunction_t *evaluate)
{
  return rhop2check(sq_king, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean grasshop3check(square    sq_king,
                       PieNam p,
                       evalfunction_t *evaluate)
{
  return rhop3check(sq_king, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean kinghopcheck(square    sq_king,
                     PieNam p,
                     evalfunction_t *evaluate)
{
  return shopcheck(sq_king, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean knighthoppercheck(square    sq_king,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  return shopcheck(sq_king, vec_knight_start, vec_knight_end, p, evaluate);
}

static boolean doublehoppercheck(square sq_king,
                                 PieNam p,
                                 vec_index_type vec_start, vec_index_type vec_end,
                                 evalfunction_t *evaluate)
{
  piece double_hopper;
  square    sq_hurdle2, sq_hurdle1;
  vec_index_type k;
  vec_index_type k1;

  square sq_departure;

  for (k=vec_end; k>=vec_start; k--) {
    sq_hurdle2= sq_king+vec[k];
    if (abs(e[sq_hurdle2])>=roib) {
      sq_hurdle2+= vec[k];
      while (e[sq_hurdle2]==vide) {
        for (k1= vec_end; k1>=vec_start; k1--) {
          sq_hurdle1= sq_hurdle2+vec[k1];
          if (abs(e[sq_hurdle1]) >= roib)
          {
            finligne(sq_hurdle1,vec[k1],double_hopper,sq_departure);
            if (abs(double_hopper)==p
                && TSTFLAG(spec[sq_departure],trait[nbply])
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
                               PieNam p,
                               evalfunction_t *evaluate)
{
  /* W.B.Trumper feenschach 1968 - but null moves will not be allowed by Popeye
   */
  return doublehoppercheck(sq_king,p,vec_queen_start,vec_queen_end,evaluate);
}

boolean doublerookhoppercheck(square  sq_king,
                              PieNam p,
                              evalfunction_t *evaluate)
{
  return doublehoppercheck(sq_king,p,vec_rook_start,vec_rook_end,evaluate);
}

boolean doublebishoppercheck(square  sq_king,
                             PieNam p,
                             evalfunction_t *evaluate)
{
  return doublehoppercheck(sq_king,p,vec_bishop_start,vec_bishop_end,evaluate);
}

boolean contragrascheck(square    sq_king,
                        PieNam p,
                        evalfunction_t *evaluate)
{
  return crhopcheck(sq_king, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean nightlocustcheck(square    sq_king,
                         PieNam p,
                         evalfunction_t *evaluate)
{
  return marine_rider_check(sq_king, vec_knight_start, vec_knight_end, p, evaluate);
}

boolean loccheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return marine_rider_check(sq_king, vec_queen_start, vec_queen_end, p, evaluate);
}

boolean tritoncheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return marine_rider_check(sq_king, vec_rook_start, vec_rook_end, p, evaluate);
}

boolean nereidecheck(square    sq_king,
                     PieNam p,
                     evalfunction_t *evaluate)
{
  return marine_rider_check(sq_king, vec_bishop_start, vec_bishop_end, p, evaluate);
}

boolean marine_knight_check(square sq_king,
                            PieNam p,
                            evalfunction_t *evaluate)
{
  return marine_leaper_check(sq_king,vec_knight_start,vec_knight_end,p,evaluate);
}

boolean poseidon_check(square sq_king,
                       PieNam p,
                       evalfunction_t *evaluate)
{
  return marine_leaper_check(sq_king,vec_queen_start,vec_queen_end,p,evaluate);
}

boolean nightriderlioncheck(square    sq_king,
                            PieNam p,
                            evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_knight_start, vec_knight_end, p, evaluate);
}

boolean lioncheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean t_lioncheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean f_lioncheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_bishop_start,vec_bishop_end, p, evaluate);
}

/* see comment in py4.c on how rose and rose based pieces are
 * handled */
boolean detect_rosecheck_on_line(square sq_king,
                                 PieNam p,
                                 vec_index_type k, vec_index_type k1,
                                 numvec delta_k,
                                 evalfunction_t *evaluate) {
  square sq_departure= fin_circle_line(sq_king,k,&k1,delta_k);
  return (abs(e[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure!=sq_king /* pieces don't give check to themselves */
          && evaluate(sq_departure,sq_king,sq_king));
}

boolean rosecheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  vec_index_type k;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    if (detect_rosecheck_on_line(sq_king,p,
                                 k,0,rose_rotation_clockwise,
                                 evaluate))
      return true;
    if (detect_rosecheck_on_line(sq_king,p,
                                 k,vec_knight_end-vec_knight_start+1,rose_rotation_counterclockwise,
                                 evaluate))
      return true;
  }

  return false;
}

boolean detect_roselioncheck_on_line(square sq_king,
                                     PieNam p,
                                     vec_index_type k, vec_index_type k1,
                                     numvec delta_k,
                                     evalfunction_t *evaluate) {
  square sq_hurdle= fin_circle_line(sq_king,k,&k1,delta_k);
  if (sq_hurdle!=sq_king && e[sq_hurdle]!=obs)
  {
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

    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && sq_departure!=sq_king /* pieces don't give check to themselves */
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean roselioncheck(square    sq_king,
                      PieNam p,
                      evalfunction_t *evaluate)
{
  /* detects check by a rose lion */
  vec_index_type k;
  for (k= vec_knight_start; k <= vec_knight_end; k++)
    if (detect_roselioncheck_on_line(sq_king,p,
                                     k,0,rose_rotation_clockwise,
                                     evaluate)
        || detect_roselioncheck_on_line(sq_king,p,
                                        k,vec_knight_end-vec_knight_start+1,rose_rotation_counterclockwise,
                                        evaluate))
      return true;

  return false;
}

boolean detect_rosehoppercheck_on_line(square sq_king,
                                       square sq_hurdle,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate) {
  square sq_departure= fin_circle_line(sq_hurdle,k,&k1,delta_k);
  return (abs(e[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure!=sq_king
          && evaluate(sq_departure,sq_king,sq_king));
}

boolean rosehoppercheck(square  sq_king,
                        PieNam p,
                        evalfunction_t *evaluate) {
  /* detects check by a rose hopper */
  vec_index_type k;
  square sq_hurdle;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_hurdle= sq_king+vec[k];
    if (e[sq_hurdle]!=vide && e[sq_hurdle]!=obs) {
      /* k1==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_rosehoppercheck_on_line(sq_king,sq_hurdle,p,
                                         k,1,rose_rotation_clockwise,
                                         evaluate))
        return true;
      if (detect_rosehoppercheck_on_line(sq_king,sq_hurdle,p,
                                         k,vec_knight_end-vec_knight_start,rose_rotation_counterclockwise,
                                         evaluate))
        return true;
    }
  }

  return false;
}

boolean detect_roselocustcheck_on_line(square sq_king,
                                       square sq_arrival,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate) {
  square sq_departure= fin_circle_line(sq_king,k,&k1,delta_k);
  return (abs(e[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure!=sq_king
          && evaluate(sq_departure,sq_arrival,sq_king));
}

boolean roselocustcheck(square  sq_king,
                        PieNam p,
                        evalfunction_t *evaluate) {
  /* detects check by a rose locust */
  vec_index_type k;
  square sq_arrival;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_arrival= sq_king-vec[k];
    if (e[sq_arrival]==vide) {
      /* k1==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_roselocustcheck_on_line(sq_king,sq_arrival,p,
                                         k,1,rose_rotation_clockwise,
                                         evaluate))
        return true;
      if (detect_roselocustcheck_on_line(sq_king,sq_arrival,p,
                                         k,vec_knight_end-vec_knight_start,rose_rotation_counterclockwise,
                                         evaluate))
        return true;
    }
  }

  return false;
}

boolean maocheck(square sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  square sq_departure;

  if (e[sq_king+dir_up+dir_right]==vide) {
    sq_departure= sq_king+dir_up+2*dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_right,
                            sq_king,
                            sq_king+dir_up+dir_right);
    }
    sq_departure= sq_king+2*dir_up+dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_right,
                            sq_king,
                            sq_king+dir_up+dir_right);
    }
  }

  if (e[sq_king+dir_down+dir_left]==vide) {
    sq_departure= sq_king+dir_down+2*dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_left,
                            sq_king,
                            sq_king+dir_down+dir_left);
    }
    sq_departure= sq_king+2*dir_down+dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_left,
                            sq_king,
                            sq_king+dir_down+dir_left);
    }
  }

  if (e[sq_king+dir_up+dir_left]==vide) {
    sq_departure= sq_king+dir_up+2*dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_left,
                            sq_king,
                            sq_king+dir_up+dir_left);
    }
    sq_departure= sq_king+2*dir_up+dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_left,
                            sq_king,
                            sq_king+dir_up+dir_left);
    }
  }

  if (e[sq_king+dir_down+dir_right]==vide) {
    sq_departure= sq_king+2*dir_down+dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_right,
                            sq_king,
                            sq_king+dir_down+dir_right);
    }
    sq_departure= sq_king+dir_down+2*dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])) {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_right,
                            sq_king,
                            sq_king+dir_down+dir_right);
    }
  }

  return false;
}

boolean moacheck(square sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  square sq_departure;

  if (e[sq_king+dir_up]==vide) {
    sq_departure= sq_king+2*dir_up+dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_left, sq_king, sq_king+dir_up);
    }
    sq_departure= sq_king+2*dir_up+dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_up+dir_right, sq_king, sq_king+dir_up);
    }
  }
  if (e[sq_king+dir_down]==vide) {
    sq_departure= sq_king+2*dir_down+dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_right, sq_king, sq_king+dir_down);
    }
    sq_departure= sq_king+2*dir_down+dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+2*dir_down+dir_left, sq_king, sq_king+dir_down);
    }
  }
  if (e[sq_king+dir_right]==vide) {
    sq_departure= sq_king+dir_up+2*dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_right, sq_king, sq_king+dir_right);
    }
    sq_departure= sq_king+dir_down+2*dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_right, sq_king, sq_king+dir_right);
    }
  }
  if (e[sq_king+dir_left]==vide) {
    sq_departure= sq_king+dir_down+2*dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_down+2*dir_left, sq_king, sq_king+dir_left);
    }
    sq_departure= sq_king+dir_up+2*dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      if (evaluate(sq_departure,sq_king,sq_king))
        return maooaimcheck(sq_king+dir_up+2*dir_left, sq_king, sq_king+dir_left);
    }
  }

  return false;
}

boolean paocheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean vaocheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean naocheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate);
}

boolean leocheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return lrhopcheck(sq_king, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean berolina_pawn_check(square sq_king,
                            PieNam p,
                            evalfunction_t *evaluate)
{
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec[sq_king],capturable) || p==Orphan || p>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;

    if (pawn_test_check(sq_king-dir_forward,sq_king,sq_king,p,evaluate))
      return true;
    if (en_passant_test_check(sq_king,dir_forward,&pawn_test_check,p,evaluate))
      return true;
  }

  return false;
}

boolean bspawncheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  piece   p1;
  square sq_departure;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;
  numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

  if (!TSTFLAG(sq_spec[sq_king],base))
  {
    finligne(sq_king,dir_backward,p1,sq_departure);
    if (abs(p1)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean spawncheck(square   sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;
  numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

  if (!TSTFLAG(sq_spec[sq_king],base))
  {
    square sq_departure;
    piece p1;
    finligne(sq_king,dir_backward+dir_left,p1,sq_departure);
    if (abs(p1)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;

    finligne(sq_king,dir_backward+dir_right,p1,sq_departure);
    if (abs(p1)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean amazcheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return  leapcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(sq_king, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean impcheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return  leapcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(sq_king, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean princcheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return  leapcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(sq_king, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean gnoucheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return  leapcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || leapcheck(sq_king, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean antilcheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_antilope_start, vec_antilope_end, p, evaluate);
}

boolean ecurcheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return  leapcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || leapcheck(sq_king, vec_ecureuil_start, vec_ecureuil_end, p, evaluate);
}

boolean warancheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return  ridcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(sq_king, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean dragoncheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  if (leapcheck(sq_king,vec_knight_start,vec_knight_end,p,evaluate))
    return true;

  return pawnedpiececheck(sq_king, p, evaluate);
}

boolean gryphoncheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return ridcheck(sq_king,vec_bishop_start,vec_bishop_end,p,evaluate);
}

boolean shipcheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  if (ridcheck(sq_king,vec_rook_start,vec_rook_end,p,evaluate))
    return true;

  return pawnedpiececheck(sq_king, p, evaluate);
}

boolean pawnedpiececheck(square sq_king,
                         PieNam p,
                         evalfunction_t *evaluate)
{
  boolean result = false;
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;

  if (TSTFLAG(sq_spec[sq_king],capturable))
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_king-dir_forward_right,sq_king,sq_king,p,evaluate))
      result = true;
    else if (pawn_test_check(sq_king-dir_forward_left,sq_king,sq_king,p,evaluate))
      result = true;
    else if (en_passant_test_check(sq_king,dir_forward_right,&pawn_test_check,p,evaluate))
      result = true;
    else if (en_passant_test_check(sq_king,dir_forward_left,&pawn_test_check,p,evaluate))
      result = true;
  }

  return result;
}

boolean kangoucheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    square sq_hurdle= sq_king+vec[k];
    if (abs(e[sq_hurdle])>=roib)
    {
      piece p1;
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs)
      {
        square sq_departure;
        finligne(sq_hurdle,vec[k],p1,sq_departure);
        if (abs(p1)==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean kanglioncheck(square  sq_king,
                      PieNam p,
                      evalfunction_t *evaluate)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece p1;
    square sq_hurdle;
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (abs(e[sq_hurdle])>=roib)
    {
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs)
      {
        square sq_departure;
        finligne(sq_hurdle,vec[k],p1,sq_departure);
        if (abs(p1)==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean rabbitcheck(square  sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  /* 2 hurdle lion */
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece p1;
    square sq_hurdle;
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (abs(p1)>=roib)
    {
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs)
      {
        square sq_departure;
        finligne(sq_hurdle,vec[k],p1,sq_departure);
        if (abs(p1)==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean bobcheck(square sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  /* 4 hurdle lion */
  numvec  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece p1;
    square sq_hurdle;
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (abs(p1)>=roib)
    {
      finligne(sq_hurdle,vec[k],p1,sq_hurdle);
      if (p1!=obs)
      {
        finligne(sq_hurdle,vec[k],p1,sq_hurdle);
        if (p1!=obs)
        {
          finligne(sq_hurdle,vec[k],p1,sq_hurdle);
          if (p1!=obs)
          {
            square sq_departure;
            finligne(sq_hurdle,vec[k],p1,sq_departure);
            if (abs(p1)==p
                && TSTFLAG(spec[sq_departure],trait[nbply])
                && evaluate(sq_departure,sq_king,sq_king))
              return true;
          }
        }
      }
    }
  }

  return false;
}

boolean ubicheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  square const *bnp;

  if (CondFlag[madras])
  {
    for (bnp= boardnum; *bnp; bnp++)
      e_ubi_mad[*bnp]= e[*bnp];
    return rubiech(sq_king, sq_king, p, e_ubi_mad, evaluate);
  }
  else
  {
    for (bnp= boardnum; *bnp; bnp++)
      e_ubi[*bnp]= e[*bnp];
    return rubiech(sq_king, sq_king, p, e_ubi, evaluate);
  }
}

boolean moosecheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return rmhopech(sq_king, vec_queen_end,vec_queen_start, angle_45, p, evaluate);
}

boolean eaglecheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return rmhopech(sq_king, vec_queen_end,vec_queen_start, angle_90, p, evaluate);
}

boolean sparrcheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return rmhopech(sq_king, vec_queen_end,vec_queen_start, angle_135, p, evaluate);
}

boolean margueritecheck(square    sq_king,
                        PieNam p,
                        evalfunction_t *evaluate)
{
  return  sparrcheck(sq_king, p, evaluate)
      || eaglecheck(sq_king, p, evaluate)
      || moosecheck(sq_king, p, evaluate)
      || scheck(sq_king, p, evaluate);
}

boolean leap36check(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_leap36_start, vec_leap36_end, p, evaluate);
}

boolean rookmoosecheck(square    sq_king,
                       PieNam p,
                       evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return rmhopech(sq_king, vec_bishop_end,vec_bishop_start, angle_45, p, evaluate);
}

boolean rookeaglecheck(square    sq_king,
                       PieNam p,
                       evalfunction_t *evaluate)
{
  return rmhopech(sq_king, vec_rook_end,vec_rook_start, angle_90, p, evaluate);
}

boolean rooksparrcheck(square    sq_king,
                       PieNam p,
                       evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return rmhopech(sq_king, vec_bishop_end,vec_bishop_start, angle_135, p, evaluate);
}

boolean bishopmoosecheck(square    sq_king,
                         PieNam p,
                         evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return rmhopech(sq_king, vec_rook_end,vec_rook_start, angle_45, p, evaluate);
}

boolean bishopeaglecheck(square    sq_king,
                         PieNam p,
                         evalfunction_t *evaluate)
{
  return rmhopech(sq_king, vec_bishop_end,vec_bishop_start, angle_90, p, evaluate);
}

boolean bishopsparrcheck(square    sq_king,
                         PieNam p,
                         evalfunction_t *evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return rmhopech(sq_king, vec_rook_end,vec_rook_start, angle_135, p, evaluate);
}

boolean archcheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(sq_king, sq_king, vec[k], p, 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean reffoucheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rrfouech(sq_king, sq_king, vec[k], p, 4, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean cardcheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  vec_index_type  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    if (rcardech(sq_king, sq_king, vec[k], p, 1, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean nsautcheck(square    sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return rhopcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate);
}

boolean camridcheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return ridcheck(sq_king, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean zebridcheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return ridcheck(sq_king, vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean gnuridcheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return  ridcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || ridcheck(sq_king, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean camhopcheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return rhopcheck(sq_king, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean zebhopcheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return rhopcheck(sq_king, vec_zebre_start, vec_zebre_end, p, evaluate);
}

boolean gnuhopcheck(square    sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return  rhopcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate)
      || rhopcheck(sq_king, vec_chameau_start, vec_chameau_end, p, evaluate);
}

boolean dcscheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  vec_index_type  k;

  for (k= vec_knight_start; k <= 14; k++) {
    if (rcsech(sq_king, vec[k], vec[23 - k], p, evaluate)) {
      return true;
    }
  }
  for (k= 15; k <= vec_knight_end; k++) {
    if (rcsech(sq_king, vec[k], vec[27 - k], p, evaluate)) {
      return true;
    }
  }
  return false;
}

boolean refccheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return rrefcech(sq_king, sq_king, 2, p, evaluate);
}

boolean refncheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  clearedgestraversed();
  return rrefnech(sq_king, sq_king, p, evaluate);
}

boolean equicheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece p1;
    square sq_hurdle;
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (p1!=obs) {
      square sq_departure;
      finligne(sq_hurdle,vec[k],p1,sq_departure);
      if (abs(p1)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure-sq_hurdle==sq_hurdle-sq_king
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-vec[k],-vec[k])))
        return true;
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {      /* 2,4; 2,6; 4,6; */
    square const sq_departure= sq_king+2*vec[k];
    if (abs(e[sq_king+vec[k]])>=roib
        && abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king)
        && (!checkhopim || hopimok(sq_departure,sq_king,sq_departure-vec[k],-vec[k],-vec[k])))
      return true;
  }

  return false;
}

boolean equiengcheck(square sq_king,
                     PieNam p,
                     evalfunction_t *evaluate)
{
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece p1;
    square sq_hurdle;
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (p1!=obs)
    {
      square sq_departure;
      finligne(sq_king,-vec[k],p1,sq_departure);
      if (abs(p1)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure-sq_king==sq_king-sq_hurdle
          && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {      /* 2,4; 2,6; 4,6; */
    square const sq_departure = sq_king-vec[k];
    square const sq_hurdle = sq_king+vec[k];
    if (abs(e[sq_hurdle])>=roib
        && abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean catcheck(square sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  if (leapcheck(sq_king,vec_knight_start,vec_knight_end,p,evaluate))
    return true;
  else
  {
    vec_index_type  k;
    for (k= vec_dabbaba_start; k<=vec_dabbaba_end; k++)
    {
      square middle_square= sq_king+vec[k];
      while (e[middle_square]==vide)
      {
        {
          square const sq_departure= middle_square+mixhopdata[3][k-60];
          if (abs(e[sq_departure])==p
              && TSTFLAG(spec[sq_departure],trait[nbply])
              && evaluate(sq_departure,sq_king,sq_king))
            return true;
        }

        {
          square const sq_departure= middle_square+mixhopdata[3][k-56];
          if (abs(e[sq_departure])==p
              && TSTFLAG(spec[sq_departure],trait[nbply])
              && evaluate(sq_departure,sq_king,sq_king))
            return true;
        }

        middle_square+= vec[k];
      }
    }

    return false;
  }
}

boolean roicheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean cavcheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return leapcheck(sq_king, vec_knight_start,vec_knight_end, p, evaluate);
}

boolean damecheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return ridcheck(sq_king, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean tourcheck(square    sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return ridcheck(sq_king, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean foucheck(square    sq_king,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  return ridcheck(sq_king, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean pioncheck(square sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec[sq_king],capturable) || p==Orphan || p>=Hunter0)
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check(sq_king-dir_forward_right,sq_king,sq_king,p,evaluate))
      result = true;
    else if (pawn_test_check(sq_king-dir_forward_left,sq_king,sq_king,p,evaluate))
      result = true;
    else if (en_passant_test_check(sq_king,dir_forward_right,&pawn_test_check,p,evaluate))
      result = true;
    else if (en_passant_test_check(sq_king,dir_forward_left,&pawn_test_check,p,evaluate))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean reversepcheck(square sq_king,
                      PieNam p,
                      evalfunction_t *evaluate)
{
  SquareFlags const capturable = trait[nbply]==White ? CapturableByBlPawnSq : CapturableByWhPawnSq;

  if (TSTFLAG(sq_spec[sq_king],capturable) || p==Orphan || p>=Hunter0)
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;
    numvec const dir_backward_right = dir_backward+dir_right;
    numvec const dir_backward_left = dir_backward+dir_left;

    if (pawn_test_check(sq_king-dir_backward_right,sq_king,sq_king,p,evaluate))
      return true;
    else if (pawn_test_check(sq_king-dir_backward_left,sq_king,sq_king,p,evaluate))
      return true;
    else if (en_passant_test_check(sq_king,dir_backward_right,&pawn_test_check,p,evaluate))
      return true;
    else if (en_passant_test_check(sq_king,dir_backward_left,&pawn_test_check,p,evaluate))
      return true;
  }

  return false;
}

boolean edgehcheck(square   sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  /* detect "check" of edgehog p */
  vec_index_type k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece p1;
    square sq_departure;
    finligne(sq_king,vec[k],p1,sq_departure);
    if (abs(p1)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && NoEdge(sq_king)!=NoEdge(sq_departure)
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

static boolean maooaridercheck(square  sq_king,
                               PieNam p,
                               numvec  fir,
                               numvec  sec,
                               evalfunction_t *evaluate)
{
  square  middle_square;

  square sq_departure= sq_king+sec;

  middle_square = sq_king+fir;
  while (e[middle_square]==vide && e[sq_departure]==vide)
  {
    middle_square+= sec;
    sq_departure+= sec;
  }

  return (e[middle_square]==vide
          && abs(e[sq_departure])==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && evaluate(sq_departure,sq_king,sq_king));
}

boolean moaridercheck(square    i,
                      PieNam p,
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
                      PieNam p,
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
                                   PieNam p,
                                   numvec  fir,
                                   numvec  sec,
                                   evalfunction_t *evaluate)
{
  square middle_square= sq_king+fir;

  square sq_departure= sq_king+sec;

  while (e[middle_square]==vide && e[sq_departure]==vide)
  {
    middle_square+= sec;
    sq_departure+= sec;
  }
  if (e[middle_square]!=vide
      && abs(e[sq_departure])==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && evaluate(sq_departure,sq_king,sq_king))
    return true;

  if (e[middle_square]!=obs
      && e[sq_departure]!=obs
      && (e[middle_square]==vide || e[sq_departure]==vide))
  {
    middle_square+= sec;
    sq_departure+= sec;
    while (e[middle_square]==vide && e[sq_departure]==vide)
    {
      middle_square+= sec;
      sq_departure+= sec;
    }
    if (e[middle_square]==vide
        && abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean maoriderlioncheck(square    i,
                          PieNam p,
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
                          PieNam p,
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
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean b_hopcheck(square    i,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  return rhopcheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean orixcheck(square sq_king,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  vec_index_type  k;

  for (k= vec_queen_end; k>=vec_queen_start; k--)
  {
    piece p1;
    square sq_hurdle;
    finligne(sq_king,vec[k],p1,sq_hurdle);
    if (p1!=obs)
    {
      square sq_departure;
      finligne(sq_hurdle,vec[k],p1,sq_departure);
      if (abs(p1)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && sq_departure-sq_hurdle==sq_hurdle-sq_king
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-vec[k],-vec[k])))
        return true;
    }
  }

  return false;
}

boolean leap15check(square    i,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap15_start, vec_leap15_end, p, evaluate);
}

boolean leap25check(square    i,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  return leapcheck(i, vec_leap25_start, vec_leap25_end, p, evaluate);
}

boolean gralcheck(square    i,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  return leapcheck(i, vec_alfil_start, vec_alfil_end, p, evaluate)
      || rhopcheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}


boolean scorpioncheck(square    i,
                      PieNam p,
                      evalfunction_t *evaluate)
{
  return  leapcheck(i, vec_queen_start,vec_queen_end, p, evaluate)
      || rhopcheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean dolphincheck(square    i,
                     PieNam p,
                     evalfunction_t *evaluate)
{
  return  rhopcheck(i, vec_queen_start,vec_queen_end, p, evaluate)
      || kangoucheck(i, p, evaluate);
}

boolean querquisitecheck(square sq_king,
                         PieNam p,
                         evalfunction_t *evaluate)
{
  vec_index_type k;

  for (k= vec_rook_start; k<=vec_rook_end; k++)
  {
    piece p1;
    square sq_departure;
    finligne(sq_king,vec[k],p1,sq_departure);
    {
      int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
      if ((file_departure==file_rook_queenside
           || file_departure==file_queen
           || file_departure==file_rook_kingside)
          && abs(p1)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && evaluate(sq_departure,sq_king,sq_king)
          && ridimcheck(sq_departure,sq_king,vec[k]))
        return true;
    }
  }

  for (k= vec_bishop_start; k<=vec_bishop_end; k++)
  {
    piece p1;
    square sq_departure;
    finligne(sq_king,vec[k],p1,sq_departure);
    {
      int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
      if ((file_departure==file_bishop_queenside
           || file_departure==file_queen
           || file_departure==file_bishop_kingside)
          && abs(p1)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && evaluate(sq_departure,sq_king,sq_king)
          && ridimcheck(sq_departure,sq_king,vec[k]))
        return true;
    }
  }

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    square const sq_departure= sq_king+vec[k];
    int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && (file_departure==file_knight_queenside
            || file_departure==file_knight_kingside)
        && evaluate(sq_departure,sq_king,sq_king)
        && imcheck(sq_departure,sq_king))
      return true;
  }

  for (k= vec_queen_start; k<=vec_queen_end; k++)
  {
    square const sq_departure= sq_king+vec[k];
    int const file_departure= sq_departure%onerow - nr_of_slack_files_left_of_board;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && file_departure==file_king
        && evaluate(sq_departure,sq_king,sq_king)
        && imcheck(sq_departure,sq_king))
      return true;
  }

  return false;
}

static boolean bouncerfamilycheck(square sq_king,
                                  vec_index_type kbeg, vec_index_type kend,
                                  PieNam p,
                                  evalfunction_t *evaluate)
{
  vec_index_type k;

  for (k= kend; k>=kbeg; k--)
  {
    piece p1;
    square sq_departure;
    piece p2;
    square sq_hurdle;
    finligne(sq_king,vec[k],p1,sq_departure);
    finligne(sq_departure,vec[k],p2,sq_hurdle);  /* p2 can be obs - bounces off edges */
    if (sq_departure-sq_king==sq_hurdle-sq_departure
        && abs(p1)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}

boolean bouncercheck(square    i,
                     PieNam p,
                     evalfunction_t *evaluate)
{
  return bouncerfamilycheck(i, vec_queen_start,vec_queen_end, p, evaluate);
}

boolean rookbouncercheck(square    i,
                         PieNam p,
                         evalfunction_t *evaluate)
{
  return bouncerfamilycheck(i, vec_rook_start,vec_rook_end, p, evaluate);
}

boolean bishopbouncercheck(square    i,
                           PieNam p,
                           evalfunction_t *evaluate)
{
  return bouncerfamilycheck(i, vec_bishop_start,vec_bishop_end, p, evaluate);
}

boolean pchincheck(square sq_king,
                   PieNam p,
                   evalfunction_t *evaluate)
{
  square sq_departure;
  numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

  sq_departure= sq_king+dir_backward;
  if (abs(e[sq_departure])==p
      && TSTFLAG(spec[sq_departure],trait[nbply])
      && evaluate(sq_departure,sq_king,sq_king))
    return true;

  /* chinese pawns can capture side-ways if standing on the half of
   * the board farther away from their camp's base line (i.e. if
   * black, on the lower half, if white on the upper half) */
  if ((sq_king*2<(square_h8+square_a1)) == (trait[nbply]==Black))
  {
    sq_departure= sq_king+dir_right;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;

    sq_departure= sq_king+dir_left;
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king))
      return true;
  }

  return false;
}


square fromspecificsquare;

boolean eval_fromspecificsquare(square sq_departure, square sq_arrival, square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = (sq_departure==fromspecificsquare
            && validate_observation(sq_departure,sq_arrival,sq_capture));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean qlinesradialcheck(square    sq_king,
                          PieNam p,
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
                    PieNam p,
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
                          PieNam p,
                          evalfunction_t *evaluate)
{
  return radialcheck(sq_king, p, evaluate, 0, false);
}

boolean treehoppercheck(square    sq_king,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  return qlinesradialcheck(sq_king, p, evaluate, 1, false);
}

boolean leafhoppercheck(square    sq_king,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  return qlinesradialcheck(sq_king, p, evaluate, 1, true);
}

boolean greatertreehoppercheck(square    sq_king,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  return radialcheck(sq_king, p, evaluate, 1, false);
}

boolean greaterleafhoppercheck(square    sq_king,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  return radialcheck(sq_king, p, evaluate, 1, true);
}
