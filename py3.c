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
#   include "pymac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "debugging/trace.h"
#include "debugging/measure.h"
#include "pieces/attributes/paralysing/paralysing.h"

boolean rubiech(square  intermediate_square,
                square  sq_king,
                piece   p,
                echiquier e_ub,
                evalfunction_t *evaluate)
{
  numvec k;

  square sq_departure;

  e_ub[intermediate_square]= obs;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_departure= intermediate_square+vec[k];
    if (e_ub[sq_departure]==vide) {
      if (rubiech(sq_departure,sq_king,p,e_ub,evaluate))
        return true;
    }
    else {
      if (e_ub[sq_departure]==p
          && evaluate(sq_departure,sq_king,sq_king))
        return true;
    }
  }

  return false;
}

boolean rrfouech(square intermediate_square,
                 square sq_king,
                 numvec k,
                 piece  p,
                 int    x,
                 evalfunction_t *evaluate)
{
  numvec k1;
  piece p1;

  square sq_departure;

  if (e[intermediate_square+k] == obs)
    return false;

  finligne(intermediate_square,k,p1,sq_departure);
  if (p1==p) {
    if (evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs) {
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
                 piece  p,
                 int    x,
                 evalfunction_t *evaluate)
{
  numvec k1;
  piece p1;

  square sq_departure;

  finligne(intermediate_square,k,p1,sq_departure);
  if (p1==p) {
    if (evaluate(sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs) {
    for (k1= 1; k1<=4; k1++)
      if (e[sq_departure+vec[k1]]!=obs)
        break;

    if (k1<=4) {
      sq_departure+= vec[k1];
      if (e[sq_departure]==p) {
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


boolean feebechec(evalfunction_t *evaluate)
{
  piece   *pcheck;

  for (pcheck= checkpieces; *pcheck; pcheck++) {
    if (nbpiece[-*pcheck]>0
        && (*checkfunctions[*pcheck])(king_square[White], -*pcheck, evaluate))
    {
      return true;
    }
  }
  return false;
}

boolean feenechec(evalfunction_t *evaluate) {
  piece   *pcheck;

  for (pcheck= checkpieces; *pcheck; pcheck++) {
    if (nbpiece[*pcheck]>0
        && (*checkfunctions[*pcheck])(king_square[Black], *pcheck, evaluate))
    {
      return true;
    }
  }
  return false;
}

#define marsmap(p) ((p)==maob ? moab : ((p)==moab ? maob : (p)))

static boolean marsechecc(ply ply_id, Side camp, evalfunction_t *evaluate)
{
  int i,j;
  square square_h = square_h8;

  /* detect mars circe check of k of Side camp */
  for (i= nr_rows_on_board; i>0; i--, square_h += dir_down)
  {
    square z = square_h;
    for (j= nr_files_on_board; j>0; j--, z += dir_left)
    {
      /* in marscirce the kings are included */
      /* in phantomchess the kings are not included, but with rex
         inclusif they are */
      if ((!CondFlag[phantom]
           || (e[z]!=e[king_square[White]] && e[z]!=e[king_square[Black]])
           || rex_phan)
          && ((e[z]!=e[king_square[White]] || e[king_square[White]]!=e[king_square[Black]]))   /* exclude nK */
          && rightcolor(e[z],camp))
      {
        mars_circe_rebirth_state = 0;
        do
        {
          piece const p = e[z];
          Flags const psp = spec[z];
          square const sq_rebirth = (*marsrenai)(ply_id,
                                                 p,
                                                 psp,
                                                 z,
                                                 initsquare,
                                                 initsquare,
                                                 camp);
          if (e[sq_rebirth]==vide || sq_rebirth==z)
          {
            boolean is_check;
            Flags const spec_rebirth = spec[sq_rebirth];
            e[z] = vide;
            e[sq_rebirth] = p;
            spec[sq_rebirth] = psp;
            is_check = (*checkfunctions[marsmap(abs(p))])
                (sq_rebirth, camp ? e[king_square[Black]] : e[king_square[White]], evaluate);
            e[sq_rebirth] = vide;
            spec[sq_rebirth] = spec_rebirth;
            e[z] = p;
            spec[z] = psp;
            if (is_check)
              return true;
          }
        } while (mars_circe_rebirth_state);
      }
    }
  }

  return false;
} /* marsechecc */

static boolean calc_rnechec(ply ply_id, evalfunction_t *evaluate);

DEFINE_COUNTER(orig_rnechec)

boolean orig_rnechec(ply ply_id, evalfunction_t *evaluate)
{
  boolean result;

  INCREMENT_COUNTER(orig_rnechec);

  if (TSTFLAG(PieSpExFlags,Neutral))
  {
    Side const neutcoul_save = neutcoul;
    initneutre(White);
    result = calc_rnechec(ply_id,evaluate);
    initneutre(neutcoul_save);
  }
  else
    result = calc_rnechec(ply_id,evaluate);

  return result;
}

static boolean calc_rnechec(ply ply_id, evalfunction_t *evaluate)
{
  /* detect, if black king is checked     */
  /* I didn't change this function, because it would be much (20% !)
     slower. NG
  */
  numvec k;
  piece p;

  square sq_departure;
  square sq_arrival;

  if (SATCheck)
  {
    int nr_flights = BlackSATFlights;
    boolean const mummer_sic = flagmummer[Black];
    boolean k_sq_checked = false;

    if (CondFlag[strictSAT])
      k_sq_checked = BlackStrictSAT[parent_ply[ply_id]];

    flagmummer[Black] = false;
    dont_generate_castling = true;

    if ((satXY || k_sq_checked) && !echecc_normal(ply_id,Black))
      nr_flights--;

    nextply(ply_id);

    trait[nbply]= Black;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(White);

    gen_bl_piece(king_square[Black],-abs(e[king_square[Black]]));

    dont_generate_castling = false;
    flagmummer[Black] = mummer_sic;

    SATCheck = false;

    while (nr_flights>0 && encore())
    {
      if (jouecoup_ortho_test(nbply) && !echecc_normal(ply_id,Black))
        nr_flights--;

      repcoup();
    }

    SATCheck = true;
    assert (nr_flights>=0);

    finply();

    return nr_flights==0;
  }

  if (anymars)
  {
    boolean anymarscheck = marsechecc(ply_id,Black,evaluate);
    if ( !CondFlag[phantom] || anymarscheck)
      return anymarscheck;
  }

  if (nbpiece[roib]>0) {
    if (calc_refl_king[White]) {
      piece   *ptrans;
      boolean flag = true;

      /* attempted bug fix - wrong eval function used to detect
         if wK is checked; this code is a bit hacky but best attempt to
         guess correct eval function to use, though only one is passed in*/
      evalfunction_t *eval_ad = evaluate;
      if (eval_white != eval_black)
        eval_ad= (evaluate == eval_white) ? eval_black :
            (evaluate == eval_black) ? eval_white : evaluate;

      calc_refl_king[White] = false;

      if (!normaltranspieces[White] && echecc(ply_id,White))
      {
        flag= false;
        for (ptrans= transmpieces[White]; *ptrans; ptrans++) {
          if ((*checkfunctions[*ptrans])(king_square[Black], roib, evaluate)) {
            calc_refl_king[White] = true;
            return true;
          }
        }
      }
      else if (normaltranspieces[White])
      {
        for (ptrans= transmpieces[White]; *ptrans; ptrans++) {
          if (nbpiece[-*ptrans]>0
              && (*checkfunctions[*ptrans])(king_square[White], -*ptrans, eval_ad))
          {
            flag= false;
            if ((*checkfunctions[*ptrans])(king_square[Black], roib, evaluate)) {
              calc_refl_king[White] = true;
              return true;
            }
          }
        }
      }

      calc_refl_king[White] = true;

      if (!calc_trans_king[White] || flag) {
        for (k= vec_queen_end; k>=vec_queen_start; k--) {
          sq_departure= king_square[Black]+vec[k];
          if (e[sq_departure]==roib
              && evaluate(sq_departure,king_square[Black],king_square[Black]))
            if (imcheck(sq_departure,king_square[Black]))
              return true;
        }
      }
    }
    else {
      if (CondFlag[sting]
          && (*checkfunctions[sb])(king_square[Black], roib, evaluate))
        return true;

      for (k= vec_queen_end; k>=vec_queen_start; k--) {
        sq_departure= king_square[Black]+vec[k];
        if (e[sq_departure]==roib
            && evaluate(sq_departure,king_square[Black],king_square[Black])
            && imcheck(sq_departure,king_square[Black]))
          return true;
      }
    }
  }

  if (nbpiece[pb]>0) {
    if (king_square[Black]>=square_a3
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage])
    {
      sq_departure= king_square[Black]+dir_down+dir_right;
      if (e[sq_departure]==pb
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (imcheck(sq_departure,king_square[Black]))
          return true;

      sq_departure= king_square[Black]+dir_down+dir_left;
      if (e[sq_departure]==pb
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (imcheck(sq_departure,king_square[Black]))
          return true;

      sq_arrival= ep[nbply];
      if (sq_arrival!=initsquare && king_square[Black]==sq_arrival+dir_down) {
        /* ep captures of royal pawns */
        /* ep[nbply] != initsquare --> a pawn has made a
           double/triple step.
           RN_[nbply] != king_square[Black] --> the black king has moved
        */
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb
            && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb
            && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }

      sq_arrival= ep2[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && king_square[Black]==sq_arrival+dir_down) {
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb && evaluate(sq_departure,sq_arrival,king_square[Black]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }
    }
  }


  if (nbpiece[cb]>0)
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_departure= king_square[Black]+vec[k];
      if (e[sq_departure]==cb
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (imcheck(sq_departure,king_square[Black]))
          return true;
    }

  if (nbpiece[db]>0 || nbpiece[tb]>0)
    for (k= vec_rook_end; k>=vec_rook_start; k--) {
      finligne(king_square[Black],vec[k],p,sq_departure);
      if ((p==tb || p==db)
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (ridimcheck(sq_departure,king_square[Black],vec[k]))
          return true;
    }

  if (nbpiece[db]>0 || nbpiece[fb]>0)
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(king_square[Black],vec[k],p,sq_departure);
      if ((p==fb || p==db)
          && evaluate(sq_departure,king_square[Black],king_square[Black]))
        if (ridimcheck(sq_departure,king_square[Black],vec[k]))
          return true;
    }

  if (flagfee)
    return feenechec(evaluate);
  else
    return false;
}

boolean singleboxtype3_rnechec(ply ply_id, evalfunction_t *evaluate)
{
  unsigned int promotionstried = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,White);
       sq!=vide;
       sq = next_latent_pawn(sq,White))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,White);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,White))
    {
      boolean result;
      ++promotionstried;
      e[sq] = pprom;
      ++nbpiece[pprom];
      result = orig_rnechec(ply_id, evaluate);
      --nbpiece[pprom];
      e[sq] = pb;
      if (result) {
        return true;
      }
    }
  }

  return promotionstried==0 && orig_rnechec(ply_id,evaluate);
}

boolean annan_rnechec(ply ply_id, evalfunction_t *evaluate)
{
  square annan_sq[nr_squares_on_board];
  piece annan_p[nr_squares_on_board];
  int annan_cnt= 0;
  boolean ret;

  square i,j,z,z1;
  z= square_h8;
  for (i= nr_rows_on_board-1; i > 0; i--, z-= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z--) {
      z1= z-onerow;
      if (e[z] > obs && whannan(z1,z))
      {
        annan_sq[annan_cnt]= z;
        annan_p[annan_cnt]= e[z];
        ++annan_cnt;
        e[z]=e[z1];
      }
    }
  ret= orig_rnechec(ply_id,evaluate);

  while (annan_cnt--)
    e[annan_sq[annan_cnt]]= annan_p[annan_cnt];

  return ret;
}

boolean (*rnechec)(ply ply_id, evalfunction_t *evaluate);

static boolean calc_rbechec(ply ply_id, evalfunction_t *evaluate);

DEFINE_COUNTER(orig_rbechec)

boolean orig_rbechec(ply ply_id, evalfunction_t *evaluate)
{
  boolean result;

  INCREMENT_COUNTER(orig_rbechec);

  if (TSTFLAG(PieSpExFlags,Neutral))
  {
    Side const neutcoul_save = neutcoul;
    initneutre(Black);
    result = calc_rbechec(ply_id,evaluate);
    initneutre(neutcoul_save);
  }
  else
    result = calc_rbechec(ply_id,evaluate);

  return result;
}

static boolean calc_rbechec(ply ply_id, evalfunction_t *evaluate)
{
  /* detect, if white king is checked  */
  /* I didn't change this function, because it would be much (20% !)
     slower. NG
  */

  numvec k;
  piece p;

  square sq_departure;
  square sq_arrival;

  if (SATCheck)
  {
    int nr_flights= WhiteSATFlights;
    boolean const mummer_sic = flagmummer[White];
    boolean k_sq_checked = false;


    if (CondFlag[strictSAT])
      k_sq_checked = WhiteStrictSAT[parent_ply[ply_id]];

    flagmummer[White] = false;
    dont_generate_castling= true;

    if ((satXY || k_sq_checked) && !echecc_normal(ply_id,White))
      nr_flights--;

    nextply(ply_id);

    current_killer_state= null_killer_state;
    trait[nbply]= White;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(Black);

    gen_wh_piece(king_square[White],abs(e[king_square[White]]));

    dont_generate_castling= false;
    flagmummer[White] = mummer_sic;

    SATCheck= false;

    while (nr_flights>0 && encore())
    {
      if (jouecoup_ortho_test(nbply) && ! echecc_normal(ply_id,White))
        nr_flights--;

      repcoup();
    }

    SATCheck= true;
    assert (nr_flights>=0);

    finply();

    return nr_flights==0;
  }

  if (anymars) {
    boolean anymarscheck= marsechecc(ply_id,White,evaluate);
    if ( !CondFlag[phantom] || anymarscheck) {
      return anymarscheck;
    }
  }

  if (nbpiece[roin]>0)
  {
    if (calc_refl_king[Black])
    {
      piece   *ptrans;
      boolean flag= true;

      /* attempted bug fix - wrong eval function used to detect
         if bK is checked; this code is a bit hacky but best attempt to
         guess correct eval function to use, though only one is passed in */
      evalfunction_t *eval_ad = evaluate;
      if (eval_white != eval_black)
        eval_ad= (evaluate == eval_white) ? eval_black :
            (evaluate == eval_black) ? eval_white : evaluate;

      calc_refl_king[Black] = false;

      if (!normaltranspieces[Black] && echecc(ply_id,Black))
      {
        flag= false;
        for (ptrans= transmpieces[Black]; *ptrans; ptrans++) {
          if ((*checkfunctions[*ptrans])(king_square[White], roin, evaluate)) {
            calc_refl_king[Black] = true;
            return true;
          }
        }
      }
      else if (normaltranspieces[Black])
      {
        for (ptrans= transmpieces[Black]; *ptrans; ptrans++) {
          if (nbpiece[*ptrans]>0
              && (*checkfunctions[*ptrans])(king_square[Black], *ptrans, eval_ad)) {
            flag= false;
            if ((*checkfunctions[*ptrans])(king_square[White], roin, evaluate)) {
              calc_refl_king[Black] = true;
              return true;
            }
          }
        }
      }

      calc_refl_king[Black] = true;

      if (!calc_trans_king[Black] || flag) {
        for (k= vec_queen_end; k>=vec_queen_start; k--) {
          sq_departure= king_square[White]+vec[k];
          if (e[sq_departure]==roin
              && evaluate(sq_departure,king_square[White],king_square[White]))
            if (imcheck(sq_departure,king_square[White]))
              return true;
        }
      }
    }
    else {
      if ( CondFlag[sting]
           && (*checkfunctions[sb])(king_square[White], roin, evaluate))
      {
        return true;
      }
      for (k= vec_queen_end; k>=vec_queen_start; k--)
      {
        sq_departure= king_square[White]+vec[k];
        if (e[sq_departure]==roin
            && evaluate(sq_departure,king_square[White],king_square[White]))
          if (imcheck(sq_departure,king_square[White]))
            return true;
      }
    }
  }

  if (nbpiece[pn]>0) {
    if (king_square[White]<=square_h6
        || anyparrain
        || CondFlag[normalp]
        || CondFlag[einstein]
        || CondFlag[circecage])
    {
      sq_departure= king_square[White]+dir_up+dir_left;
      if (e[sq_departure]==pn
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (imcheck(sq_departure,king_square[White]))
          return true;

      sq_departure= king_square[White]+dir_up+dir_right;
      if (e[sq_departure]==pn
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (imcheck(sq_departure,king_square[White]))
          return true;

      sq_arrival= ep[nbply];
      if (sq_arrival!=initsquare && king_square[White]==sq_arrival+dir_up) {
        /* ep captures of royal pawns.
           ep[nbply] != initsquare
           --> a pawn has made a double/triple step.
           RB_[nbply] != king_square[White]
           --> the white king has moved
        */
        sq_departure= sq_arrival+dir_up+dir_left;
        if (e[sq_departure]==pn
            && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn
            && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }

      sq_arrival= ep2[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && king_square[White]==sq_arrival+dir_up) {
        sq_departure= sq_arrival+dir_up+dir_left;
        if (e[sq_departure]==pn && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn && evaluate(sq_departure,sq_arrival,king_square[White]))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }
    }
  }

  if (nbpiece[cn]>0)
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_departure= king_square[White]+vec[k];
      if (e[sq_departure]==cn
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (imcheck(sq_departure,king_square[White]))
          return true;
    }

  if (nbpiece[dn]>0 || nbpiece[tn]>0)
    for (k= vec_rook_end; k>=vec_rook_start; k--) {
      finligne(king_square[White],vec[k],p,sq_departure);
      if ((p==tn || p==dn)
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (ridimcheck(sq_departure,king_square[White],vec[k]))
          return true;
    }

  if (nbpiece[dn]>0 || nbpiece[fn]>0)
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(king_square[White],vec[k],p,sq_departure);
      if ((p==fn || p==dn)
          && evaluate(sq_departure,king_square[White],king_square[White]))
        if (ridimcheck(sq_departure,king_square[White],vec[k]))
          return true;
    }

  if (flagfee)
    return feebechec(evaluate);
  else
    return false;
}

boolean annan_rbechec(ply ply_id, evalfunction_t *evaluate)
{
  square annan_sq[nr_squares_on_board];
  piece annan_p[nr_squares_on_board];
  int annan_cnt= 0;
  boolean ret;

  square i,j,z,z1;
  z= square_a1;
  for (i= nr_rows_on_board-1; i > 0; i--, z+= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z++) {
      z1= z+onerow;
      if (e[z] < vide && blannan(z1,z))
      {
        annan_sq[annan_cnt]= z;
        annan_p[annan_cnt]= e[z];
        ++annan_cnt;
        e[z]=e[z1];
      }
    }
  ret= orig_rbechec(ply_id,evaluate);

  while (annan_cnt--)
    e[annan_sq[annan_cnt]]= annan_p[annan_cnt];

  return ret;
}

boolean losingchess_rbnechec(ply ply_id, evalfunction_t *evaluate)
{
  return false;
}

boolean singleboxtype3_rbechec(ply ply_id, evalfunction_t *evaluate)
{
  unsigned int promotionstried = 0;
  square sq;

  for (sq = next_latent_pawn(initsquare,Black);
       sq!=vide;
       sq = next_latent_pawn(sq,Black))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,Black);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,Black))
    {
      boolean result;
      ++promotionstried;
      e[sq] = -pprom;
      ++nbpiece[-pprom];
      result = orig_rbechec(ply_id,evaluate);
      --nbpiece[-pprom];
      e[sq] = pn;
      if (result) {
        return true;
      }
    }
  }

  return promotionstried==0 && orig_rbechec(ply_id,evaluate);
}

boolean (*rbechec)(ply ply_id, evalfunction_t *evaluate);


boolean rncircech(square sq_departure, square sq_arrival, square sq_capture) {
  if (sq_departure == (*circerenai)(nbply, e[king_square[Black]], spec[king_square[Black]], sq_capture, sq_departure, sq_arrival, White)) {
    return false;
  }
  else
    return eval_2(sq_departure,sq_arrival,sq_capture);
}

boolean rbcircech(square sq_departure, square sq_arrival, square sq_capture) {
  if (sq_departure == (*circerenai)(nbply, e[king_square[White]], spec[king_square[White]], sq_capture, sq_departure, sq_arrival, Black)) {
    return false;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

boolean rnimmunech(square sq_departure, square sq_arrival, square sq_capture) {
  immrenroin= (*immunrenai)(nbply, e[king_square[Black]], spec[king_square[Black]], sq_capture, sq_departure, sq_arrival, White);

  if ((e[immrenroin] != vide && sq_departure != immrenroin)) {
    return false;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

boolean rbimmunech(square sq_departure, square sq_arrival, square sq_capture) {
  immrenroib= (*immunrenai)(nbply, e[king_square[White]], spec[king_square[White]], sq_capture, sq_departure, sq_arrival, Black);

  if ((e[immrenroib] != vide && sq_departure != immrenroib)) {
    return false;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

static boolean echecc_wh_extinction(ply ply_id)
{
  square const save_rb = king_square[White];
  piece p;
  for (p=roib; p<derbla; p++)
  {
    square const *bnp;
    if (!exist[p] || nbpiece[p]!=1)
      continue;

    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==p)
        break;

    king_square[White] = *bnp;
    if (rbechec(ply_id,eval_white))
    {
      king_square[White] = save_rb;
      return true;
    }
  }

  king_square[White] = save_rb;
  return false;
}

static boolean echecc_bl_extinction(ply ply_id)
{
  square const save_rn = king_square[Black];
  piece p;
  for (p=roib; p<derbla; p++)
  {
    square const *bnp;

    if (!exist[p] || nbpiece[-p]!=1)
      continue;

    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-p)
        break;

    king_square[Black] = *bnp;
    if (rnechec(ply_id,eval_black))
    {
      king_square[Black] = save_rn;
      return true;
    }
  }

  king_square[Black] = save_rn;
  return false;
}

static boolean echecc_wh_assassin(ply ply_id)
{
  square const *bnp;

  if (rbechec(ply_id,eval_white))
    return true;

  for (bnp= boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];

    if (p!=vide
        && p>roib
        && (*circerenai)(ply_id, p,spec[*bnp],*bnp,initsquare,initsquare,Black)==king_square[White])
    {
      boolean flag;
      square const rb_sic = king_square[White];
      king_square[White] = *bnp;
      CondFlag[circeassassin] = false;
      flag = rbechec(ply_id,eval_white);
      CondFlag[circeassassin] = true;
      king_square[White] = rb_sic;
      if (flag)
        return true;
    }
  }

  return false;
}

static boolean echecc_bl_assassin(ply ply_id)
{
  square const *bnp;

  if (rnechec(ply_id,eval_black))
    return true;

  for (bnp= boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];
    if (p!=vide
        && p<roin
        && ((*circerenai)(ply_id,
                          p,
                          spec[*bnp],
                          *bnp,
                          initsquare,
                          initsquare,
                          White)
            ==king_square[Black]))
    {
      boolean flag;
      square rn_sic = king_square[Black];
      king_square[Black] = *bnp;
      CondFlag[circeassassin] = false;
      flag = rnechec(ply_id,eval_black);
      CondFlag[circeassassin] = true;
      king_square[Black] = rn_sic;
      if (flag)
        return true;
    }
  }

  return false;
}

static boolean echecc_wh_bicolores(ply ply_id)
{
  if (rbechec(ply_id,eval_white))
    return true;
  else
  {
    boolean result;
    square rn_sic = king_square[Black];
    king_square[Black] = king_square[White];
    CondFlag[bicolores] = false;
    result = rnechec(ply_id,eval_black);
    CondFlag[bicolores] = true;
    king_square[Black] = rn_sic;
    return result;
  }
}

static boolean echecc_bl_bicolores(ply ply_id)
{
  if (rnechec(ply_id,eval_black))
    return true;
  else
  {
    boolean result;
    square rb_sic = king_square[White];
    king_square[White] = king_square[Black];
    CondFlag[bicolores] = false;
    result = rbechec(ply_id,eval_white);
    CondFlag[bicolores] = true;
    king_square[White] = rb_sic;
    return result;
  }
}

boolean echecc(ply ply_id, Side camp)
{
  boolean result;

  nextply(ply_id);

  if ((camp==White) != CondFlag[vogt])
  {
    if (CondFlag[extinction])
      result = echecc_wh_extinction(nbply);
    else if (king_square[White]==initsquare)
      result = false;
    else if (rex_circe
             && (CondFlag[pwc]
                 || e[(*circerenai)
                      (nbply, e[king_square[White]], spec[king_square[White]], king_square[White], initsquare, initsquare, Black)] == vide))
      result = false;
    else
    {
      if (TSTFLAG(PieSpExFlags,Neutral))
        initneutre(Black);
      if (CondFlag[circeassassin] && echecc_wh_assassin(nbply))
        result = true;
      else if (CondFlag[bicolores])
        result = echecc_wh_bicolores(nbply);
      else
        result = CondFlag[antikings]!=rbechec(nbply,eval_white);
    }
  }
  else /* camp==Black */
  {
    if (CondFlag[extinction])
      result = echecc_bl_extinction(nbply);
    else if (king_square[Black] == initsquare)
      result = false;
    else if (rex_circe
             && (CondFlag[pwc]
                 || e[(*circerenai)
                      (nbply, e[king_square[Black]], spec[king_square[Black]], king_square[Black], initsquare, initsquare, White)] == vide))
      result = false;
    else
    {
      if (TSTFLAG(PieSpExFlags,Neutral))
        initneutre(White);
      if (CondFlag[circeassassin] && echecc_bl_assassin(nbply))
        result = true;
      else if (CondFlag[bicolores])
        result = echecc_bl_bicolores(nbply);
      else
        result = CondFlag[antikings]!=rnechec(nbply,eval_black);
    }
  }

  finply();

  return result;
} /* end of echecc */

boolean testparalyse(square sq_departure, square sq_arrival, square sq_capture) {
  if (flaglegalsquare && !legalsquare(sq_departure,sq_arrival,sq_capture))
    return false;
  else
    return TSTFLAG(spec[sq_departure], Paralyse);
}

boolean paraechecc(square sq_departure, square sq_arrival, square sq_capture) {
  if (TSTFLAG(spec[sq_departure], Paralyse)
      || (flaglegalsquare && !legalsquare(sq_departure,sq_arrival,sq_capture)))
  {
    return false;
  }
  else {
    return (!paralysiert(sq_departure));
  }
}

static evalfunction_t *next_evaluate;

static boolean eval_up(square sq_departure, square sq_arrival, square sq_capture) {
  return sq_arrival-sq_departure>8
      && next_evaluate(sq_departure,sq_arrival,sq_capture);
}

static boolean eval_down(square sq_departure, square sq_arrival, square sq_capture) {
  return sq_arrival-sq_departure<-8
      && next_evaluate(sq_departure,sq_arrival,sq_capture);
}

boolean huntercheck(square i,
                    piece p,
                    evalfunction_t *evaluate)
{
  /* detect check by a hunter */
  evalfunction_t * const eval_away = p<0 ? &eval_down : &eval_up;
  evalfunction_t * const eval_home = p<0 ? &eval_up : &eval_down;
  unsigned int const typeofhunter = abs(p)-Hunter0;
  HunterType const * const huntertype = huntertypes+typeofhunter;
  assert(typeofhunter<maxnrhuntertypes);
  next_evaluate = evaluate;
  return (*checkfunctions[huntertype->home])(i,p,eval_home)
      || (*checkfunctions[huntertype->away])(i,p,eval_away);
}

boolean rhuntcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  /* detect check of a rook/bishop-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (rook), down (bishop) !! */
  return ridcheck(i, 4, 4, p, evaluate)
      || ridcheck(i, 5, 6, p, evaluate);
}

boolean bhuntcheck(square    i,
                   piece p,
                   evalfunction_t *evaluate)
{
  /* detect check of a bishop/rook-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (bishop), down (rook) !! */
  return ridcheck(i, 2, 2, p, evaluate)
      || ridcheck(i, 7, 8, p, evaluate);
}

static boolean AntiCirceEch(ply ply_id,
                            square sq_departure,
                            square sq_arrival,
                            square sq_capture,
                            Side    camp)
{
  if (CondFlag[antisuper])
  {
    square const *bnp= boardnum;
    while (!LegalAntiCirceMove(*bnp, sq_capture, sq_departure) && *bnp) bnp++;
    if (!(*bnp && LegalAntiCirceMove(*bnp, sq_capture, sq_departure)))
      return false;
  }
  else
  {
	piece* acprompieces= GetPromotingPieces(sq_departure,
										    e[sq_departure],
										    advers(camp),
										    spec[sq_departure],
										    sq_arrival,
										    e[sq_capture]);
    if (acprompieces) {
      /* Pawn checking on last rank or football check on a/h file */
      piece pprom= acprompieces[vide];
      square    cren;
      do {
        cren= (*antirenai)(ply_id, pprom, spec[sq_departure], sq_capture, sq_departure, sq_arrival, camp);
        pprom= acprompieces[pprom];
      } while (!LegalAntiCirceMove(cren, sq_capture, sq_departure) && pprom != vide);
      if (  !LegalAntiCirceMove(cren, sq_capture, sq_departure)
            && pprom == vide)
      {
        return false;
      }
    }
    else {
      square    cren;
      cren= (*antirenai)( ply_id, TSTFLAG(spec[sq_departure], Chameleon)
                          ? champiece(e[sq_departure])
                          : e[sq_departure],
                          spec[sq_departure], sq_capture, sq_departure, sq_arrival, camp);
      if (!LegalAntiCirceMove(cren, sq_capture, sq_departure)) {
        return false;
      }
    }
  }

  return eval_2(sq_departure,sq_arrival,sq_capture);
} /* AntiCirceEch */

boolean rnanticircech(square sq_departure, square sq_arrival, square sq_capture) {
  return AntiCirceEch(nbply, sq_departure, sq_arrival, sq_capture, Black);
}

boolean rbanticircech(square sq_departure, square sq_arrival, square sq_capture) {
  return AntiCirceEch(nbply, sq_departure, sq_arrival, sq_capture, White);
}

boolean rnsingleboxtype1ech(square sq_departure, square sq_arrival, square sq_capture) {
  if (is_forwardpawn(e[sq_departure]) && PromSq(White, sq_capture)) {
    /* Pawn checking on last rank */
    return next_singlebox_prom(vide,White)!=vide;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

boolean rbsingleboxtype1ech(square sq_departure, square sq_arrival, square sq_capture) {
  if (is_forwardpawn(e[sq_departure]) && PromSq(Black, sq_capture)) {
    /* Pawn checking on last rank */
    return next_singlebox_prom(vide,Black)!=vide;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}


boolean rbultraech(square sq_departure, square sq_arrival, square sq_capture) {
  killer_state const save_killer_state = current_killer_state;
  move_generation_mode_type const save_move_generation_mode
      = move_generation_mode;
  boolean check;

  /* if we_generate_consmoves is set this function is never called.
     Let's check this for a while.
  */
  if (we_generate_exact) {
    StdString(
        "rbultra is called while we_generate_exact is set!\n");
  }
  nextply(nbply);
  current_killer_state.move.departure = sq_departure;
  current_killer_state.move.arrival = sq_arrival;
  current_killer_state.found = false;
  move_generation_mode = move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  trait[nbply]= Black;
  we_generate_exact = true;
  gen_bl_ply();
  finply();
  check = current_killer_state.found;
  we_generate_exact = false;
  move_generation_mode = save_move_generation_mode;
  TraceValue("->%u\n",move_generation_mode);
  current_killer_state = save_killer_state;

  return  check ? eval_2(sq_departure,sq_arrival,sq_capture) : false;
}

boolean rnultraech(square sq_departure, square sq_arrival, square sq_capture) {
  killer_state const save_killer_state = current_killer_state;
  move_generation_mode_type const save_move_generation_mode
      = move_generation_mode;
  boolean check;

  nextply(nbply);
  current_killer_state.move.departure = sq_departure;
  current_killer_state.move.arrival = sq_arrival;
  current_killer_state.found = false;
  move_generation_mode = move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  trait[nbply]= White;
  we_generate_exact = true;
  gen_wh_ply();
  finply();
  check = current_killer_state.found;
  we_generate_exact = false;
  move_generation_mode = save_move_generation_mode;
  TraceValue("->%u\n",move_generation_mode);
  current_killer_state = save_killer_state;

  return check ? eval_2(sq_departure,sq_arrival,sq_capture) : false;
}

static boolean skycharcheck(piece  p,
                            square sq_king,
                            square chp,
                            square sq_arrival1,
                            square sq_arrival2,
                            evalfunction_t *evaluate)
{
  if (e[chp] == p) {
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
                    piece p,
                    evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_right, i+dir_up+dir_left, i+dir_down+dir_left, evaluate)
      || skycharcheck(p, i, i+dir_left, i+dir_up+dir_right, i+dir_down+dir_right, evaluate)
      || skycharcheck(p, i, i+dir_up, i+dir_down+dir_right, i+dir_down+dir_left, evaluate)
      || skycharcheck(p, i, i+dir_down, i+dir_up+dir_left, i+dir_up+dir_right, evaluate);
}

boolean charybdischeck(square    i,
                       piece p,
                       evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_up+dir_right, i+dir_left, i - 24, evaluate)
      || skycharcheck(p, i, i+dir_down+dir_left, i+dir_right, i + 24, evaluate)
      || skycharcheck(p, i, i+dir_up+dir_left, i+dir_right, i - 24, evaluate)
      || skycharcheck(p, i, i+dir_down+dir_right, i+dir_left, i + 24, evaluate);
}

boolean echecc_normal(ply ply_id, Side camp)
{
  /* for strict SAT - need to compute whether the K square is normally checked */
  boolean flag;
  SATCheck= false;
  flag= echecc(ply_id,camp);
  SATCheck= true;
  return flag;
}

