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
#include "trace.h"

boolean rubiech(ply ply_id,
                square  intermediate_square,
                square  sq_king,
                piece   p,
                piece   *e_ub,
                evalfunction_t *evaluate)
{
  numvec k;

  square sq_departure;
    
  e_ub[intermediate_square]= obs;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_departure= intermediate_square+vec[k];
    if (e_ub[sq_departure]==vide) {
      if (rubiech(ply_id,sq_departure,sq_king,p,e_ub,evaluate))
        return true;
    }
    else {
      if (e_ub[sq_departure]==p
          && evaluate(ply_id,sq_departure,sq_king,sq_king))
        return true;
    }
  }

  return false;
}

boolean rrfouech(ply ply_id,
                 square intermediate_square,
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
    if (evaluate(ply_id,sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs) {
    sq_departure-= k;
    k1= 5;
    while (vec[k1]!=k)
      k1++;
    
    k1*= 2;
    if (rrfouech(ply_id,
                 sq_departure,
                 sq_king,
                 mixhopdata[1][k1],
                 p,
                 x-1,
                 evaluate))
      
      return true;
    
    k1--;
    if (rrfouech(ply_id,
                 sq_departure,
                 sq_king,
                 mixhopdata[1][k1],
                 p,
                 x-1,
                 evaluate))
      return true;
  }
  
  return false;
}

boolean rcardech(ply ply_id,
                 square intermediate_square,
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
    if (evaluate(ply_id,sq_departure,sq_king,sq_king))
      return true;
  }
  else if (x && p1==obs) {
    for (k1= 1; k1<=4; k1++)
      if (e[sq_departure+vec[k1]]!=obs)
        break;

    if (k1<=4) {
      sq_departure+= vec[k1];
      if (e[sq_departure]==p) {
        if (evaluate(ply_id,sq_departure,sq_king,sq_king))
          return true;
      }
      else if (e[sq_departure]==vide) {
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (e[sq_departure+mixhopdata[1][k1]]==obs)
          k1--;
        if (rcardech(ply_id,
                     sq_departure,
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


boolean feebechec(ply ply_id, evalfunction_t *evaluate)
{
  piece   *pcheck;

  for (pcheck= checkpieces; *pcheck; pcheck++) {
    if (nbpiece[-*pcheck]>0
        && (*checkfunctions[*pcheck])(ply_id, rb, -*pcheck, evaluate))
    {
      return true;
    }
  }
  return false;
}

boolean feenechec(ply ply_id, evalfunction_t *evaluate) {
  piece   *pcheck;

  for (pcheck= checkpieces; *pcheck; pcheck++) {
    if (nbpiece[*pcheck]>0
        && (*checkfunctions[*pcheck])(ply_id, rn, *pcheck, evaluate))
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
           || (e[z]!=e[rb] && e[z]!=e[rn])
           || rex_phan)
          && ((e[z]!=e[rb] || e[rb]!=e[rn]))   /* exclude nK */
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
                (ply_id, sq_rebirth, camp ? e[rn] : e[rb], evaluate);
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
boolean orig_rnechec(ply ply_id, evalfunction_t *evaluate)
{
  Side neutcoul_save = neutcoul;
  boolean flag;
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(White);
  flag = calc_rnechec(ply_id,evaluate);
  initneutre(neutcoul_save);
  return flag;    
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
    boolean const mummer_sic = flagblackmummer;
    boolean k_sq_checked = false;

    if (CondFlag[strictSAT])
      k_sq_checked = BlackStrictSAT[parent_ply[ply_id]];

    flagblackmummer = false;
    dont_generate_castling = true;

    if ((satXY || k_sq_checked) && !echecc_normal(ply_id,Black))
      nr_flights--;

    nextply(ply_id);

    current_killer_state = null_killer_state;
    trait[nbply]= Black;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(White);

    gen_bl_piece(rn,-abs(e[rn]));

    dont_generate_castling = false;
    flagblackmummer = mummer_sic;

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
    if (calc_whrefl_king) {
      piece   *ptrans;
      boolean flag = true;

      /* attempted bug fix - wrong eval function used to detect 
         if wK is checked; this code is a bit hacky but best attempt to 
         guess correct eval function to use, though only one is passed in*/
      evalfunction_t *eval_ad = evaluate;
      if (eval_white != eval_black) 
        eval_ad= (evaluate == eval_white) ? eval_black :
            (evaluate == eval_black) ? eval_white : evaluate;

      calc_whrefl_king = false;

      if (!whitenormaltranspieces && echecc(ply_id,White))
      {
        flag= false;
        for (ptrans= whitetransmpieces; *ptrans; ptrans++) {
          if ((*checkfunctions[*ptrans])(ply_id, rn, roib, evaluate)) {
            calc_whrefl_king = true;
            return true;
          }
        }
      }
      else if (whitenormaltranspieces)
      {
        for (ptrans= whitetransmpieces; *ptrans; ptrans++) {
          if (nbpiece[-*ptrans]>0
              && (*checkfunctions[*ptrans])(ply_id, rb, -*ptrans, eval_ad))
          {
            flag= false;
            if ((*checkfunctions[*ptrans])(ply_id, rn, roib, evaluate)) {
              calc_whrefl_king = true;
              return true;
            }
          }
        }
      }
      
      calc_whrefl_king = true;

      if (!calc_whtrans_king || flag) {
        for (k= vec_queen_end; k>=vec_queen_start; k--) {
          sq_departure= rn+vec[k];
          if (e[sq_departure]==roib
              && evaluate(ply_id,sq_departure,rn,rn))
            if (imcheck(sq_departure,rn))
              return true;
        }
      }
    }
    else {
      if (CondFlag[sting]
          && (*checkfunctions[sb])(ply_id, rn, roib, evaluate))
        return true;

      for (k= vec_queen_end; k>=vec_queen_start; k--) {
        sq_departure= rn+vec[k];
        if (e[sq_departure]==roib
            && evaluate(ply_id,sq_departure,rn,rn))
          if (imcheck(sq_departure,rn))
            return true;
      }
    }
  }
  
  if (nbpiece[pb]>0) {
    if (rn>=square_a3
        || CondFlag[parrain]
        || CondFlag[normalp]
        || CondFlag[einstein])
    {
      sq_departure= rn+dir_down+dir_right;
      if (e[sq_departure]==pb
          && evaluate(ply_id,sq_departure,rn,rn))
        if (imcheck(sq_departure,rn))
          return true;
      
      sq_departure= rn+dir_down+dir_left;
      if (e[sq_departure]==pb
          && evaluate(ply_id,sq_departure,rn,rn))
        if (imcheck(sq_departure,rn))
          return true;

      sq_arrival= ep[nbply];
      if (sq_arrival!=initsquare && rn==sq_arrival+dir_down) {
        /* ep captures of royal pawns */
        /* ep[nbply] != initsquare --> a pawn has made a
           double/triple step.
           RN_[nbply] != rn --> the black king has moved
        */
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb
            && evaluate(ply_id,sq_departure,sq_arrival,rn))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb
            && evaluate(ply_id,sq_departure,sq_arrival,rn))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }

      sq_arrival= ep2[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && rn==sq_arrival+dir_down) {
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb && evaluate(ply_id,sq_departure,sq_arrival,rn))
          if (imcheck(sq_departure,sq_arrival))
            return true;
        
        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb && evaluate(ply_id,sq_departure,sq_arrival,rn))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }
    }
  }


  if (nbpiece[cb]>0)
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_departure= rn+vec[k];
      if (e[sq_departure]==cb
          && evaluate(ply_id,sq_departure,rn,rn))
        if (imcheck(sq_departure,rn))
          return true;
    }

  if (nbpiece[db]>0 || nbpiece[tb]>0)
    for (k= vec_rook_end; k>=vec_rook_start; k--) {
      finligne(rn,vec[k],p,sq_departure);
      if ((p==tb || p==db)
          && evaluate(ply_id,sq_departure,rn,rn))
        if (ridimcheck(sq_departure,rn,vec[k]))
          return true;
    }
  
  if (nbpiece[db]>0 || nbpiece[fb]>0)
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(rn,vec[k],p,sq_departure);
      if ((p==fb || p==db)
          && evaluate(ply_id,sq_departure,rn,rn))
        if (ridimcheck(sq_departure,rn,vec[k]))
          return true;
    }
  
  if (flagfee)
    return feenechec(ply_id,evaluate);
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
boolean orig_rbechec(ply ply_id, evalfunction_t *evaluate)
{
  Side neutcoul_save = neutcoul;
  boolean result;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(Black);
  result = calc_rbechec(ply_id,evaluate);
  initneutre(neutcoul_save);

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
    boolean mummer_sic = flagwhitemummer;
    boolean k_sq_checked = false;  


    if (CondFlag[strictSAT])
      k_sq_checked = WhiteStrictSAT[parent_ply[ply_id]];
    
    flagwhitemummer = false;
    dont_generate_castling= true;

    if ((satXY || k_sq_checked) && !echecc_normal(ply_id,White))
      nr_flights--;

    nextply(ply_id);

    current_killer_state= null_killer_state;
    trait[nbply]= White;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(Black);

    gen_wh_piece(rb,abs(e[rb]));

    dont_generate_castling= false;
    flagwhitemummer = mummer_sic;

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
    if (calc_blrefl_king)
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

      calc_blrefl_king = false;

      if (!blacknormaltranspieces && echecc(ply_id,Black))
      {
        flag= false;
        for (ptrans= blacktransmpieces; *ptrans; ptrans++) {
          if ((*checkfunctions[*ptrans])(ply_id, rb, roin, evaluate)) {
            calc_blrefl_king = true;
            return true;
          }
        }
      }
      else if (blacknormaltranspieces)
      {
        for (ptrans= blacktransmpieces; *ptrans; ptrans++) {
          if (nbpiece[*ptrans]>0
              && (*checkfunctions[*ptrans])(ply_id, rn, *ptrans, eval_ad)) {
            flag= false;
            if ((*checkfunctions[*ptrans])(ply_id, rb, roin, evaluate)) {
              calc_blrefl_king = true;
              return true;
            }
          }
        }
      }
      
      calc_blrefl_king = true;

      if (!calc_bltrans_king || flag) {
        for (k= vec_queen_end; k>=vec_queen_start; k--) {
          sq_departure= rb+vec[k];
          if (e[sq_departure]==roin
              && evaluate(ply_id,sq_departure,rb,rb))
            if (imcheck(sq_departure,rb))
              return true;
        }
      }
    }
    else {
      if ( CondFlag[sting]
           && (*checkfunctions[sb])(ply_id, rb, roin, evaluate))
      {
        return true;
      }
      for (k= vec_queen_end; k>=vec_queen_start; k--)
      {
        sq_departure= rb+vec[k];
        if (e[sq_departure]==roin
            && evaluate(ply_id,sq_departure,rb,rb))
          if (imcheck(sq_departure,rb))
            return true;
      }
    }
  }

  if (nbpiece[pn]>0) {
    if (rb<=square_h6
        || CondFlag[parrain]
        || CondFlag[normalp]
        || CondFlag[einstein])
    {
      sq_departure= rb+dir_up+dir_left;
      if (e[sq_departure]==pn
          && evaluate(ply_id,sq_departure,rb,rb))
        if (imcheck(sq_departure,rb))
          return true;

      sq_departure= rb+dir_up+dir_right;
      if (e[sq_departure]==pn
          && evaluate(ply_id,sq_departure,rb,rb))
        if (imcheck(sq_departure,rb))
          return true;

      sq_arrival= ep[nbply];
      if (sq_arrival!=initsquare && rb==sq_arrival+dir_up) {
        /* ep captures of royal pawns.
           ep[nbply] != initsquare
           --> a pawn has made a double/triple step.
           RB_[nbply] != rb
           --> the white king has moved
        */
        sq_departure= sq_arrival+dir_up+dir_left;
        if (e[sq_departure]==pn
            && evaluate(ply_id,sq_departure,sq_arrival,rb))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn
            && evaluate(ply_id,sq_departure,sq_arrival,rb))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }

      sq_arrival= ep2[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && rb==sq_arrival+dir_up) {
        sq_departure= sq_arrival+dir_up+dir_left;
        if (e[sq_departure]==pn && evaluate(ply_id,sq_departure,sq_arrival,rb))
          if (imcheck(sq_departure,sq_arrival))
            return true;

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn && evaluate(ply_id,sq_departure,sq_arrival,rb))
          if (imcheck(sq_departure,sq_arrival))
            return true;
      }
    }
  }

  if (nbpiece[cn]>0)
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_departure= rb+vec[k];
      if (e[sq_departure]==cn
          && evaluate(ply_id,sq_departure,rb,rb))
        if (imcheck(sq_departure,rb))
          return true;
    }

  if (nbpiece[dn]>0 || nbpiece[tn]>0)
    for (k= vec_rook_end; k>=vec_rook_start; k--) {
      finligne(rb,vec[k],p,sq_departure);
      if ((p==tn || p==dn)
          && evaluate(ply_id,sq_departure,rb,rb))
        if (ridimcheck(sq_departure,rb,vec[k]))
          return true;
    }

  if (nbpiece[dn]>0 || nbpiece[fn]>0)
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(rb,vec[k],p,sq_departure);
      if ((p==fn || p==dn)
          && evaluate(ply_id,sq_departure,rb,rb))
        if (ridimcheck(sq_departure,rb,vec[k]))
          return true;
    }

  if (flagfee)
    return feebechec(ply_id,evaluate);
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


boolean rncircech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  if (sq_departure == (*circerenai)(ply_id, e[rn], spec[rn], sq_capture, sq_departure, sq_arrival, White)) {
    return false;
  }
  else
    return eval_2(ply_id,sq_departure,sq_arrival,sq_capture);
}

boolean rbcircech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  if (sq_departure == (*circerenai)(ply_id, e[rb], spec[rb], sq_capture, sq_departure, sq_arrival, Black)) {
    return false;
  }
  else {
    return eval_2(ply_id,sq_departure,sq_arrival,sq_capture);
  }
}

boolean rnimmunech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  immrenroin= (*immunrenai)(ply_id, e[rn], spec[rn], sq_capture, sq_departure, sq_arrival, White);

  if ((e[immrenroin] != vide && sq_departure != immrenroin)) {
    return false;
  }
  else {
    return eval_2(ply_id,sq_departure,sq_arrival,sq_capture);
  }
}

boolean rbimmunech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  immrenroib= (*immunrenai)(ply_id, e[rb], spec[rb], sq_capture, sq_departure, sq_arrival, Black);

  if ((e[immrenroib] != vide && sq_departure != immrenroib)) {
    return false;
  }
  else {
    return eval_2(ply_id,sq_departure,sq_arrival,sq_capture);
  }
}

boolean echecc(ply ply_id, Side camp)
{
  if ((camp==White) != CondFlag[vogt]) {
    
    if (CondFlag[extinction]) {
      piece p;
      square *bnp;
      for (p=roib; p<derbla; p++) {
        if (!exist[p] || nbpiece[p]!=1)
          continue;
        for (bnp= boardnum; *bnp; bnp++) {
          if (e[*bnp]==p)
            break;
        }
        rb= *bnp;
        if (rbechec(ply_id,eval_white)) {
          return true;
        }
      }
      return false;
    }
    if (rb == initsquare) {
      return false;
    }
    if (rex_circe
        && (CondFlag[pwc]
            || e[(*circerenai)
                 (ply_id, e[rb], spec[rb], rb, initsquare, initsquare, Black)] == vide))
    {
      return false;
    }
    if (TSTFLAG(PieSpExFlags,Neutral)) {
      initneutre(Black);
    }
    if (flagAssassin) {
      boolean flag;
      piece p;
      square *bnp;
      if (rbechec(ply_id,eval_white)) {
        return true;
      }
      for (bnp= boardnum; *bnp; bnp++) {
        p = e[*bnp];
        if (p!=vide
            && p>roib
            && (*circerenai)(ply_id, p,spec[*bnp],*bnp,initsquare,initsquare,Black)==rb)
        {
          square rb_sic = rb;
          rb = *bnp;
          flagAssassin=false;
          flag=rbechec(ply_id,eval_white);
          flagAssassin=true;
          rb = rb_sic;
          if (flag) {
            return true;
          }
        }
      }
    }
    if (CondFlag[bicolores]) {
      boolean flag = rbechec(ply_id,eval_white);
      if (!flag) {
        square  rn_sic = rn;
        rn = rb;
        CondFlag[bicolores] = false;
        flag = rnechec(ply_id,eval_black);
        CondFlag[bicolores] = true;
        rn = rn_sic;
      }
      return flag;
    }
    else {
      return  CondFlag[antikings] != rbechec(ply_id,eval_white);
    }
  }
  else {      /* camp == Black */
    if (CondFlag[extinction]) {
      piece p;
      square *bnp;
      for (p=roib; p<derbla; p++) {
        if (!exist[p] || nbpiece[-p]!=1)
          continue;
        for (bnp= boardnum; *bnp; bnp++) {
          if (e[*bnp]==-p)
            break;
        }
        rn= *bnp;
        if (rnechec(ply_id,eval_black)) {
          return true;
        }
      }
      return false;
    }
    if (rn == initsquare)
      return false;
    if (rex_circe
        && (CondFlag[pwc]
            || e[(*circerenai)
                 (ply_id, e[rn], spec[rn], rn, initsquare, initsquare, White)] == vide))
    {
      return false;
    }

    if (TSTFLAG(PieSpExFlags,Neutral)) {
      initneutre(White);
    }
    if (flagAssassin) {
      boolean flag;
      piece p;
      square *bnp;
      if (rnechec(ply_id,eval_black)) {
        return true;
      }
      for (bnp= boardnum; *bnp; bnp++) {
        p= e[*bnp];
        if (p!=vide
            && p<roin
            && ((*circerenai)(ply_id,
                              p,
                              spec[*bnp],
                              *bnp,
                              initsquare,
                              initsquare,
                              White)
                ==rn))
        {
          square rn_sic = rn;
          rn = *bnp;
          flagAssassin=false;
          flag=rnechec(ply_id,eval_black);
          flagAssassin=true;
          rn = rn_sic;
          if (flag) {
            return true;
          }
        }
      }
    }
    if (CondFlag[bicolores]) {
      boolean flag = rnechec(ply_id,eval_black);
      if (!flag) {
        square  rb_sic = rb;
        rb = rn;
        CondFlag[bicolores] = false;
        flag = rbechec(ply_id,eval_white);
        CondFlag[bicolores] = true;
        rb = rb_sic;
      }
      return flag;
    }
    else {
      return  CondFlag[antikings] != rnechec(ply_id,eval_black);
    }
  }
} /* end of echecc */

boolean testparalyse(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  if (flaglegalsquare && !legalsquare(ply_id,sq_departure,sq_arrival,sq_capture))
    return false;
  else
    return TSTFLAG(spec[sq_departure], Paralyse);
}

boolean paraechecc(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  if (TSTFLAG(spec[sq_departure], Paralyse)
      || (flaglegalsquare && !legalsquare(ply_id,sq_departure,sq_arrival,sq_capture)))
  {
    return false;
  }
  else {
    return (!paralysiert(sq_departure));
  }
}

boolean paralysiert(square i) {
  square  roi;
  boolean flag;

  if (e[i] > obs) {
    roi = rb;
    rb = i;
    flag = rbechec(nbply,testparalyse);
    rb = roi;
  }
  else {
    roi = rn;
    rn = i;
    flag = rnechec(nbply,testparalyse);
    rn = roi;
  }
  return flag;
}


static evalfunction_t *next_evaluate;

static boolean eval_up(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  return sq_arrival-sq_departure>8
      && next_evaluate(ply_id,sq_departure,sq_arrival,sq_capture);
}

static boolean eval_down(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  return sq_arrival-sq_departure<-8
      && next_evaluate(ply_id,sq_departure,sq_arrival,sq_capture);
}

boolean huntercheck(ply ply_id,
                    square i,
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
  return (*checkfunctions[huntertype->home])(ply_id, i,p,eval_home)
      || (*checkfunctions[huntertype->away])(ply_id, i,p,eval_away);
}

boolean rhuntcheck(
    ply ply_id,
    square    i,
    piece p,
    evalfunction_t *evaluate)
{
  /* detect check of a rook/bishop-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (rook), down (bishop) !! */
  return ridcheck(ply_id, i, 4, 4, p, evaluate)
      || ridcheck(ply_id, i, 5, 6, p, evaluate);
}

boolean bhuntcheck(
    ply ply_id,
    square    i,
    piece p,
    evalfunction_t *evaluate)
{
  /* detect check of a bishop/rook-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (bishop), down (rook) !! */
  return ridcheck(ply_id, i, 2, 2, p, evaluate)
      || ridcheck(ply_id, i, 7, 8, p, evaluate);
}

boolean AntiCirceEch(ply ply_id,
                     square sq_departure,
                     square sq_arrival,
                     square sq_capture,
                     Side    camp)
{
  if (CondFlag[antisuper])
  {
    square *bnp= boardnum; 
    while (!LegalAntiCirceMove(*bnp, sq_capture, sq_departure) && *bnp) bnp++;
    if (!(*bnp && LegalAntiCirceMove(*bnp, sq_capture, sq_departure)))
      return false;
  }
  else
    if ((is_forwardpawn(e[sq_departure])
         && PromSq(advers(camp),sq_capture))
        || (is_reversepawn(e[sq_departure])
            && ReversePromSq(advers(camp),sq_capture))) {
      /* Pawn checking on last rank */
      piece pprom= getprompiece[vide];
      square    cren;
      do {
        cren= (*antirenai)(ply_id, pprom, spec[sq_departure], sq_capture, sq_departure, sq_arrival, camp);
        pprom= getprompiece[pprom];
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

  return eval_2(ply_id,sq_departure,sq_arrival,sq_capture);
} /* AntiCirceEch */

boolean rnanticircech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  return AntiCirceEch(ply_id, sq_departure, sq_arrival, sq_capture, Black);
}

boolean rbanticircech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  return AntiCirceEch(ply_id, sq_departure, sq_arrival, sq_capture, White);
}

boolean rnsingleboxtype1ech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  if (is_forwardpawn(e[sq_departure]) && PromSq(White, sq_capture)) {
    /* Pawn checking on last rank */
    return next_singlebox_prom(vide,White)!=vide;
  }
  else {
    return eval_2(ply_id,sq_departure,sq_arrival,sq_capture);
  }
}

boolean rbsingleboxtype1ech(ply ply_id, square sq_departure, square sq_arrival, square sq_capture) {
  if (is_forwardpawn(e[sq_departure]) && PromSq(Black, sq_capture)) {
    /* Pawn checking on last rank */
    return next_singlebox_prom(vide,Black)!=vide;
  }
  else {
    return eval_2(ply_id,sq_departure,sq_arrival,sq_capture);
  }
}


boolean rbultraech(ply ply_id,
                   square sq_departure, square sq_arrival, square sq_capture) {
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
  nextply(ply_id);
  current_killer_state.move.departure = sq_departure;
  current_killer_state.move.arrival = sq_arrival;
  current_killer_state.found = false;
  move_generation_mode = move_generation_optimized_by_killer_move;
  trait[nbply]= Black;
  we_generate_exact = true;
  gen_bl_ply();
  finply();
  check = current_killer_state.found;
  we_generate_exact = false;
  move_generation_mode = save_move_generation_mode;
  current_killer_state = save_killer_state;

  return  check ? eval_2(ply_id,sq_departure,sq_arrival,sq_capture) : false;
}

boolean rnultraech(ply ply_id,
                   square sq_departure, square sq_arrival, square sq_capture) {
  killer_state const save_killer_state = current_killer_state;
  move_generation_mode_type const save_move_generation_mode
      = move_generation_mode;
  boolean check;

  nextply(ply_id);
  current_killer_state.move.departure = sq_departure;
  current_killer_state.move.arrival = sq_arrival;
  current_killer_state.found = false;
  move_generation_mode = move_generation_optimized_by_killer_move;
  trait[nbply]= White;
  we_generate_exact = true;
  gen_wh_ply();
  finply();
  check = current_killer_state.found;
  we_generate_exact = false;
  move_generation_mode = save_move_generation_mode;
  current_killer_state = save_killer_state;

  return check ? eval_2(ply_id,sq_departure,sq_arrival,sq_capture) : false;
}

boolean skycharcheck(ply ply_id,
                     piece  p,
                     square sq_king,
                     square chp,
                     square sq_arrival1,
                     square sq_arrival2,
                     evalfunction_t *evaluate)
{
  if (e[chp] == p) {
    if (e[sq_arrival1]==vide
        && evaluate(ply_id,chp,sq_arrival1,sq_king)) {
      return  true;
    }

    if (e[sq_arrival2]==vide
        && evaluate(ply_id,chp,sq_arrival2,sq_king)) {
      return  true;
    }
  }
  
  return  false;
}

boolean skyllacheck(
    ply ply_id,
    square    i,
    piece p,
    evalfunction_t *evaluate)
{
  return  skycharcheck(ply_id, p, i, i+dir_right, i+dir_up+dir_left, i+dir_down+dir_left, evaluate)
      || skycharcheck(ply_id, p, i, i+dir_left, i+dir_up+dir_right, i+dir_down+dir_right, evaluate)
      || skycharcheck(ply_id, p, i, i+dir_up, i+dir_down+dir_right, i+dir_down+dir_left, evaluate)
      || skycharcheck(ply_id, p, i, i+dir_down, i+dir_up+dir_left, i+dir_up+dir_right, evaluate);
}

boolean charybdischeck(
    ply ply_id,
    square    i,
    piece p,
    evalfunction_t *evaluate)
{
  return  skycharcheck(ply_id, p, i, i+dir_up+dir_right, i+dir_left, i - 24, evaluate)
      || skycharcheck(ply_id, p, i, i+dir_down+dir_left, i+dir_right, i + 24, evaluate)
      || skycharcheck(ply_id, p, i, i+dir_up+dir_left, i+dir_right, i - 24, evaluate)
      || skycharcheck(ply_id, p, i, i+dir_down+dir_right, i+dir_left, i + 24, evaluate);
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

