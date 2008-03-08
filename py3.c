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

boolean rubiech(square  intermediate_square,
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


boolean feebechec(
  evalfunction_t *evaluate)
{
  piece   *pcheck;

  for (pcheck= checkpieces; *pcheck; pcheck++) {
    if (   nbpiece[-*pcheck]
           && (*checkfunctions[*pcheck])(rb, -*pcheck, evaluate))
    {
      return true;
    }
  }
  return false;
}

boolean feenechec(evalfunction_t *evaluate) {
  piece   *pcheck;

  for (pcheck= checkpieces; *pcheck; pcheck++) {
    if (   nbpiece[*pcheck]
           && (*checkfunctions[*pcheck])(rn, *pcheck, evaluate))
    {
      return true;
    }
  }
  return false;
}

#define marsmap(p) ((p)==maob ? moab : ((p)==moab ? maob : (p)))

boolean marsechecc(
  couleur   camp,
  evalfunction_t *evaluate)
{
  piece p;
  square i,z;
  int ii,jj;
  Flags psp;
  boolean ch;

  /* detect mars circe check of k of couleur camp */

  for (ii= 8, z= haut; ii > 0; ii--, z-= 16) {
    for (jj= 8; jj > 0; jj--, z--) {
      /* in marscirce the kings are included */
      /* in phantomchess the kings are not included, but with rex
         inclusif they are */
      if ( (!CondFlag[phantom]
            || (e[z] != e[rb] && e[z] != e[rn])
            || rex_phan)
           && ( (e[z] != e[rb]
                 || e[rb] != e[rn]) )   /* exclude nK */
           && rightcolor(e[z],camp))
      {
        more_ren=0;
        do {
          i= (*marsrenai)(p=e[z],
                          psp=spec[z], z, initsquare, initsquare,camp);
          if ((e[i]==vide) || (i==z)) {
            e[z]=vide;
            e[i]=p;
            spec[i]=psp;
            ch=(*checkfunctions[marsmap((p > 0)?p:-p)])
              (i, camp ? e[rn] : e[rb], evaluate);
            e[i]=vide;
            e[z]=p;
            spec[z]=psp;
            if (ch) {
              return true;
            }
          } /* if */
        } while (more_ren);
      }
    }
  }
  return false;
} /* marsechecc */

static boolean calc_rnechec(evalfunction_t *evaluate);
boolean orig_rnechec(evalfunction_t *evaluate)
{
  couleur neutcoul_save = neutcoul;
  boolean flag;
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(blanc);
  flag = calc_rnechec(evaluate);
  initneutre(neutcoul_save);
  return flag;    
}

static boolean calc_rnechec(evalfunction_t *evaluate)
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
    int flag= BlackSATFlights;
    boolean mummer_sic = flagblackmummer;
    boolean k_sq_checked = false;    
    int ply;
    if (CondFlag[strictSAT])
      for (ply= flag_writinglinesolution?tempply:nbply-1; ply; ply--)
        if (BlackStrictSAT[ply])
          k_sq_checked= true; 
    flagblackmummer = false;
    if ((satXY || k_sq_checked) && !echecc_normal(noir))
      flag--;
    nextply();
    current_killer_state= null_killer_state;
    trait[nbply]= noir;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(blanc);
    dont_generate_castling=true;
    gen_bl_piece(rn, -abs(e[rn]));
    flagblackmummer = mummer_sic;
    dont_generate_castling=false;
    while (flag && encore()) {
      SATCheck = false;
      if (jouecoup_ortho_test())
        if (! echecc_normal(noir))
          flag--;
      repcoup();
      SATCheck = true;
      if (!flag) break;
    }
    finply();
    return !flag;
  }

  if (anymars) {
    boolean anymarscheck= marsechecc(noir, evaluate);
    if ( !CondFlag[phantom] || anymarscheck) {
      return anymarscheck;
    }
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

      if (!whitenormaltranspieces && echecc(blanc))
      {
        flag= false;
        for (ptrans= whitetransmpieces; *ptrans; ptrans++) {
          if ((*checkfunctions[*ptrans])(rn, roib, evaluate)) {
            calc_whrefl_king = true;
            return true;
          }
        }
      }
      else if (whitenormaltranspieces)
      {
        for (ptrans= whitetransmpieces; *ptrans; ptrans++) {
          if (nbpiece[-*ptrans]
              && (*checkfunctions[*ptrans])(rb, -*ptrans, eval_ad))
          {
            flag= false;
            if ((*checkfunctions[*ptrans])(rn, roib, evaluate)) {
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
              && evaluate(sq_departure,rn,rn))
            imech(sq_departure,rn);
        }
      }
    }
    else {
      if (CondFlag[sting]
          && (*checkfunctions[sb])(rn, roib, evaluate))
        return true;

      for (k= vec_queen_end; k>=vec_queen_start; k--) {
        sq_departure= rn+vec[k];
        if (e[sq_departure]==roib
            && evaluate(sq_departure,rn,rn))
          imech(sq_departure,rn);
      }
    }
  }
  
  if (nbpiece[pb]) {
    if (rn>=square_a3
        || CondFlag[parrain]
        || CondFlag[normalp]
        || CondFlag[einstein])
    {
      sq_departure= rn+dir_down+dir_right;
      if (e[sq_departure]==pb
          && evaluate(sq_departure,rn,rn))
        imech(sq_departure,rn);
      
      sq_departure= rn+dir_down+dir_left;
      if (e[sq_departure]==pb
          && evaluate(sq_departure,rn,rn))
        imech(sq_departure,rn);

      sq_arrival= ep[nbply];
      if (sq_arrival!=initsquare && rn==sq_arrival+dir_down) {
        /* ep captures of royal pawns */
        /* ep[nbply] != initsquare --> a pawn has made a
           double/triple step.
           RN_[nbply] != rn --> the black king has moved
        */
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb
            && evaluate(sq_departure,sq_arrival,rn))
          imech(sq_departure,sq_arrival);

        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb
            && evaluate(sq_departure,sq_arrival,rn))
          imech(sq_departure,sq_arrival);
      }

      sq_arrival= ep2[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && rn==sq_arrival+dir_down) {
        sq_departure= sq_arrival+dir_down+dir_right;
        if (e[sq_departure]==pb && evaluate(sq_departure,sq_arrival,rn))
          imech(sq_departure,sq_arrival);
        
        sq_departure= sq_arrival+dir_down+dir_left;
        if (e[sq_departure]==pb && evaluate(sq_departure,sq_arrival,rn))
          imech(sq_departure,sq_arrival);
      }
    }
  }


  if (nbpiece[cb])
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_departure= rn+vec[k];
      if (e[sq_departure]==cb
          && evaluate(sq_departure,rn,rn))
        imech(sq_departure,rn);
    }

  if (nbpiece[db] || nbpiece[tb])
    for (k= vec_rook_end; k>=vec_rook_start; k--) {
      finligne(rn,vec[k],p,sq_departure);
      if ((p==tb || p==db)
          && evaluate(sq_departure,rn,rn))
        ridimech(sq_departure,rn,vec[k]);
    }
  
  if (nbpiece[db] || nbpiece[fb])
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(rn,vec[k],p,sq_departure);
      if ((p==fb || p==db)
          && evaluate(sq_departure,rn,rn))
        ridimech(sq_departure,rn,vec[k]);
    }
  
  if (flagfee)
    return feenechec(evaluate);
  else
    return false;
}

boolean singleboxtype3_rnechec(
  evalfunction_t *evaluate)
{
  unsigned int promotionstried = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,blanc);
       sq!=vide;
       sq = next_latent_pawn(sq,blanc))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,blanc);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,blanc))
    {
      boolean result;
      ++promotionstried;
      e[sq] = pprom;
      ++nbpiece[pprom];
      result = orig_rnechec(evaluate);
      --nbpiece[pprom];
      e[sq] = pb;
      if (result) {
        return true;
      }
    }
  }

  return promotionstried==0 && orig_rnechec(evaluate);
}

boolean annan_rnechec(evalfunction_t *evaluate) 
{
  square annan_sq[64];
  piece annan_p[64];
  int annan_cnt= 0;
  boolean ret;

  square i,j,z,z1;
  z= haut;
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
  ret= orig_rnechec(evaluate);

  while (annan_cnt--)
    e[annan_sq[annan_cnt]]= annan_p[annan_cnt];

  return ret;
}

boolean (*rnechec)(evalfunction_t *evaluate);

static boolean calc_rbechec(evalfunction_t *evaluate);
boolean orig_rbechec(evalfunction_t *evaluate)
{
  couleur neutcoul_save = neutcoul;
  boolean flag;
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(noir);
  flag = calc_rbechec(evaluate);
  initneutre(neutcoul_save);
  return flag;    
}

static boolean calc_rbechec(evalfunction_t *evaluate)
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
    int flag= WhiteSATFlights;
    boolean mummer_sic = flagwhitemummer;
    boolean k_sq_checked = false;  
    int ply;
    if (CondFlag[strictSAT])
      for (ply= flag_writinglinesolution?tempply:nbply-1; ply; ply--)
        if (WhiteStrictSAT[ply])
          k_sq_checked= true; 
    flagwhitemummer = false;
    if ((satXY || k_sq_checked) && !echecc_normal(blanc))
      flag--;
    nextply();
    current_killer_state= null_killer_state;
    trait[nbply]= blanc;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(noir);
    flagwhitemummer = mummer_sic;
    dont_generate_castling= true;
    gen_wh_piece(rb, abs(e[rb]));
    dont_generate_castling= false;
    while (flag && encore()) {
      SATCheck= false;
      if (jouecoup_ortho_test())
        if (! echecc_normal(blanc))
          flag--;
      repcoup();
      SATCheck= true;
      if (!flag) break;
    }
    assert (flag >= 0);
    finply();
    return !flag;
  }

  if (anymars) {
    boolean anymarscheck= marsechecc(blanc, evaluate);
    if ( !CondFlag[phantom] || anymarscheck) {
      return anymarscheck;
    }
  }

  if (nbpiece[roin]>0) {
    if (calc_blrefl_king) {
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

      if (!blacknormaltranspieces && echecc(noir))
      {
        flag= false;
        for (ptrans= blacktransmpieces; *ptrans; ptrans++) {
          if ((*checkfunctions[*ptrans])(rb, roin, evaluate)) {
            calc_blrefl_king = true;
            return true;
          }
        }
      }
      else if (blacknormaltranspieces)
      {
        for (ptrans= blacktransmpieces; *ptrans; ptrans++) {
          if (nbpiece[*ptrans]
              && (*checkfunctions[*ptrans])(rn, *ptrans, eval_ad)) {
            flag= false;
            if ((*checkfunctions[*ptrans])(rb, roin, evaluate)) {
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
              && evaluate(sq_departure,rb,rb))
            imech(sq_departure,rb);
        }
      }
    }
    else {
      if ( CondFlag[sting]
           && (*checkfunctions[sb])(rb, roin, evaluate))
      {
        return true;
      }
      for (k= vec_queen_end; k>=vec_queen_start; k--) {
        sq_departure= rb+vec[k];
        if (e[sq_departure]==roin
            && evaluate(sq_departure,rb,rb))
          imech(sq_departure,rb);
      }
    }
  }

  if (nbpiece[pn]) {
    if (rb<=square_h6
        || CondFlag[parrain]
        || CondFlag[normalp]
        || CondFlag[einstein])
    {
      sq_departure= rb+dir_up+dir_left;
      if (e[sq_departure]==pn
          && evaluate(sq_departure,rb,rb))
        imech(sq_departure,rb);

      sq_departure= rb+dir_up+dir_right;
      if (e[sq_departure]==pn
          && evaluate(sq_departure,rb,rb))
        imech(sq_departure,rb);

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
            && evaluate(sq_departure,sq_arrival,rb))
          imech(sq_departure,sq_arrival);

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn
            && evaluate(sq_departure,sq_arrival,rb))
          imech(sq_departure,sq_arrival);
      }

      sq_arrival= ep2[nbply]; /* Einstein triple step */
      if (sq_arrival!=initsquare && rb==sq_arrival+dir_up) {
        sq_departure= sq_arrival+dir_up+dir_left;
        if (e[sq_departure]==pn && evaluate(sq_departure,sq_arrival,rb))
          imech(sq_departure,sq_arrival);

        sq_departure= sq_arrival+dir_up+dir_right;
        if (e[sq_departure]==pn && evaluate(sq_departure,sq_arrival,rb))
          imech(sq_departure,sq_arrival);
      }
    }
  }

  if (nbpiece[cn])
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_departure= rb+vec[k];
      if (e[sq_departure]==cn
          && evaluate(sq_departure,rb,rb))
        imech(sq_departure,rb);
    }

  if (nbpiece[dn] || nbpiece[tn])
    for (k= vec_rook_end; k>=vec_rook_start; k--) {
      finligne(rb,vec[k],p,sq_departure);
      if ((p==tn || p==dn)
          && evaluate(sq_departure,rb,rb))
        ridimech(sq_departure,rb,vec[k]);
    }

  if (nbpiece[dn] || nbpiece[fn])
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      finligne(rb,vec[k],p,sq_departure);
      if ((p==fn || p==dn)
          && evaluate(sq_departure,rb,rb))
        ridimech(sq_departure,rb,vec[k]);
    }

  if (flagfee)
    return feebechec(evaluate);
  else
    return false;
}

boolean annan_rbechec(evalfunction_t *evaluate) 
{
  square annan_sq[64];
  piece annan_p[64];
  int annan_cnt= 0;
  boolean ret;

  square i,j,z,z1;
  z= bas;
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
  ret= orig_rbechec(evaluate);

  while (annan_cnt--)
    e[annan_sq[annan_cnt]]= annan_p[annan_cnt];

  return ret;
}

boolean singleboxtype3_rbechec(evalfunction_t *evaluate)
{
  unsigned int promotionstried = 0;
  square sq;

  for (sq = next_latent_pawn(initsquare,noir);
       sq!=vide;
       sq = next_latent_pawn(sq,noir))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,noir);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,noir))
    {
      boolean result;
      ++promotionstried;
      e[sq] = -pprom;
      ++nbpiece[-pprom];
      result = orig_rbechec(evaluate);
      --nbpiece[-pprom];
      e[sq] = pn;
      if (result) {
        return true;
      }
    }
  }

  return promotionstried==0 && orig_rbechec(evaluate);
}

boolean (*rbechec)(evalfunction_t *evaluate);


boolean rncircech(square sq_departure, square sq_arrival, square sq_capture) {
  if (sq_departure == (*circerenai)(e[rn], spec[rn], sq_capture, sq_departure, sq_arrival, blanc)) {
    return false;
  }
  else
    return eval_2(sq_departure,sq_arrival,sq_capture);
}

boolean rbcircech(square sq_departure, square sq_arrival, square sq_capture) {
  if (sq_departure == (*circerenai)(e[rb], spec[rb], sq_capture, sq_departure, sq_arrival, noir)) {
    return false;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

boolean rnimmunech(square sq_departure, square sq_arrival, square sq_capture) {
  immrenroin= (*immunrenai)(e[rn], spec[rn], sq_capture, sq_departure, sq_arrival, blanc);

  if ((e[immrenroin] != vide && sq_departure != immrenroin)) {
    return false;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

boolean rbimmunech(square sq_departure, square sq_arrival, square sq_capture) {
  immrenroib= (*immunrenai)(e[rb], spec[rb], sq_capture, sq_departure, sq_arrival, noir);

  if ((e[immrenroib] != vide && sq_departure != immrenroib)) {
    return false;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

boolean echecc(couleur camp)
{
  if ((camp==blanc) != CondFlag[vogt]) {
    
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
        if (rbechec(eval_white)) {
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
                 (e[rb], spec[rb], rb, initsquare, initsquare, noir)] == vide))
    {
      return false;
    }
    if (TSTFLAG(PieSpExFlags,Neutral)) {
      initneutre(noir);
    }
    if (flagAssassin) {
      boolean flag;
      piece p;
      square *bnp;
      if (rbechec(eval_white)) {
        return true;
      }
      for (bnp= boardnum; *bnp; bnp++) {
        p = e[*bnp];
        if (p!=vide
            && p>roib
            && (*circerenai)(p,spec[*bnp],*bnp,initsquare,initsquare,noir)==rb)
        {
          square rb_sic = rb;
          rb = *bnp;
          flagAssassin=false;
          flag=rbechec(eval_white);
          flagAssassin=true;
          rb = rb_sic;
          if (flag) {
            return true;
          }
        }
      }
    }
    if (CondFlag[bicolores]) {
      boolean flag = rbechec(eval_white);
      if (!flag) {
        square  rn_sic = rn;
        rn = rb;
        CondFlag[bicolores] = false;
        flag = rnechec(eval_black);
        CondFlag[bicolores] = true;
        rn = rn_sic;
      }
      return flag;
    }
    else {
      return  CondFlag[antikings] != rbechec(eval_white);
    }
  }
  else {      /* camp == noir */
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
        if (rnechec(eval_black)) {
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
                 (e[rn], spec[rn], rn, initsquare, initsquare, blanc)] == vide))
    {
      return false;
    }

    if (TSTFLAG(PieSpExFlags,Neutral)) {
      initneutre(blanc);
    }
    if (flagAssassin) {
      boolean flag;
      piece p;
      square *bnp;
      if (rnechec(eval_black)) {
        return true;
      }
      for (bnp= boardnum; *bnp; bnp++) {
        p= e[*bnp];
        if (p!=vide
            && p<roin
            && ((*circerenai)(p,
                              spec[*bnp],
                              *bnp,
                              initsquare,
                              initsquare,
                              blanc)
                ==rn))
        {
          square rn_sic = rn;
          rn = *bnp;
          flagAssassin=false;
          flag=rnechec(eval_black);
          flagAssassin=true;
          rn = rn_sic;
          if (flag) {
            return true;
          }
        }
      }
    }
    if (CondFlag[bicolores]) {
      boolean flag = rnechec(eval_black);
      if (!flag) {
        square  rb_sic = rb;
        rb = rn;
        CondFlag[bicolores] = false;
        flag = rbechec(eval_white);
        CondFlag[bicolores] = true;
        rb = rb_sic;
      }
      return flag;
    }
    else {
      return  CondFlag[antikings] != rnechec(eval_black);
    }
  }
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

boolean paralysiert(square i) {
  square  roi;
  boolean flag;

  if (e[i] > obs) {
    roi = rb;
    rb = i;
    flag = rbechec(testparalyse);
    rb = roi;
  }
  else {
    roi = rn;
    rn = i;
    flag = rnechec(testparalyse);
    rn = roi;
  }
  return flag;
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

boolean rhuntcheck(
  square    i,
  piece p,
  evalfunction_t *evaluate)
{
  /* detect check of a rook/bishop-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (rook), down (bishop) !! */
  return ridcheck(i, 4, 4, p, evaluate)
    || ridcheck(i, 5, 6, p, evaluate);
}

boolean bhuntcheck(
  square    i,
  piece p,
  evalfunction_t *evaluate)
{
  /* detect check of a bishop/rook-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (bishop), down (rook) !! */
  return ridcheck(i, 2, 2, p, evaluate)
    || ridcheck(i, 7, 8, p, evaluate);
}

boolean AntiCirceEch(square sq_departure,
                     square sq_arrival,
                     square sq_capture,
                     couleur    camp)
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
        cren= (*antirenai)(pprom, spec[sq_departure], sq_capture, sq_departure, sq_arrival, camp);
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
      cren= (*antirenai)( TSTFLAG(spec[sq_departure], Chameleon)
                          ? champiece(e[sq_departure])
                          : e[sq_departure],
                          spec[sq_departure], sq_capture, sq_departure, sq_arrival, camp);
      if (!LegalAntiCirceMove(cren, sq_capture, sq_departure)) {
        return false;
      }
    }

  return eval_2(sq_departure,sq_arrival,sq_capture);
} /* AntiCirceEch */

boolean rnanticircech(square sq_departure, square sq_arrival, square sq_capture) {
  return AntiCirceEch(sq_departure, sq_arrival, sq_capture, noir);
}

boolean rbanticircech(square sq_departure, square sq_arrival, square sq_capture) {
  return AntiCirceEch(sq_departure, sq_arrival, sq_capture, blanc);
}

boolean rnsingleboxtype1ech(square sq_departure, square sq_arrival, square sq_capture) {
  if (is_forwardpawn(e[sq_departure]) && PromSq(blanc, sq_capture)) {
    /* Pawn checking on last rank */
    return next_singlebox_prom(vide,blanc)!=vide;
  }
  else {
    return eval_2(sq_departure,sq_arrival,sq_capture);
  }
}

boolean rbsingleboxtype1ech(square sq_departure, square sq_arrival, square sq_capture) {
  if (is_forwardpawn(e[sq_departure]) && PromSq(noir, sq_capture)) {
    /* Pawn checking on last rank */
    return next_singlebox_prom(vide,noir)!=vide;
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
  nextply();
  current_killer_state.move.departure = sq_departure;
  current_killer_state.move.arrival = sq_arrival;
  current_killer_state.found = false;
  move_generation_mode = move_generation_optimized_by_killer_move;
  trait[nbply]= noir;
  we_generate_exact = true;
  gen_bl_ply();
  finply();
  check = current_killer_state.found;
  we_generate_exact = false;
  move_generation_mode = save_move_generation_mode;
  current_killer_state = save_killer_state;

  return  check ? eval_2(sq_departure,sq_arrival,sq_capture) : false;
}

boolean rnultraech(square sq_departure, square sq_arrival, square sq_capture) {
  killer_state const save_killer_state = current_killer_state;
  move_generation_mode_type const save_move_generation_mode
    = move_generation_mode;
  boolean check;

  nextply();
  current_killer_state.move.departure = sq_departure;
  current_killer_state.move.arrival = sq_arrival;
  current_killer_state.found = false;
  move_generation_mode = move_generation_optimized_by_killer_move;
  trait[nbply]= blanc;
  we_generate_exact = true;
  gen_wh_ply();
  finply();
  check = current_killer_state.found;
  we_generate_exact = false;
  move_generation_mode = save_move_generation_mode;
  current_killer_state = save_killer_state;

  return check ? eval_2(sq_departure,sq_arrival,sq_capture) : false;
}

boolean skycharcheck(piece  p,
                     square sq_king,
                     square chp,
                     square sq_arrival1,
                     square sq_arrival2,
                     evalfunction_t *evaluate)
{
  if (e[chp] == p) {
    if (e[sq_arrival1]==vide
        && evaluate(chp,sq_arrival1,sq_king)) {
      return  True;
    }

    if (e[sq_arrival2]==vide
        && evaluate(chp,sq_arrival2,sq_king)) {
      return  True;
    }
  }
  
  return  False;
}

boolean skyllacheck(
  square    i,
  piece p,
  evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_right, i+dir_up+dir_left, i+dir_down+dir_left, evaluate)
    || skycharcheck(p, i, i+dir_left, i+dir_up+dir_right, i+dir_down+dir_right, evaluate)
    || skycharcheck(p, i, i+dir_up, i+dir_down+dir_right, i+dir_down+dir_left, evaluate)
    || skycharcheck(p, i, i+dir_down, i+dir_up+dir_left, i+dir_up+dir_right, evaluate);
}

boolean charybdischeck(
  square    i,
  piece p,
  evalfunction_t *evaluate)
{
  return  skycharcheck(p, i, i+dir_up+dir_right, i+dir_left, i - 24, evaluate)
    || skycharcheck(p, i, i+dir_down+dir_left, i+dir_right, i + 24, evaluate)
    || skycharcheck(p, i, i+dir_up+dir_left, i+dir_right, i - 24, evaluate)
    || skycharcheck(p, i, i+dir_down+dir_right, i+dir_left, i + 24, evaluate);
}

boolean echecc_normal(couleur camp)
{
  /* for strict SAT - need to compute whether the K square is normally checked */
  boolean flag;
  SATCheck= false;
  flag= echecc(camp);
  SATCheck= true;
  return flag;
}

