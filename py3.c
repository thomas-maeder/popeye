/******************** MODIFICATIONS to py3.c **************************
**
** Date       Who  What
**
** 2001/09/29 ThM  new conditions singleboxtype[123]
**
** 2001/10/26 NG   assertions only with -DASSERT due to performance penalty
**
**************************** End of List ******************************/

#ifdef macintosh	/* is always defined on macintosh's  SB */
#	define SEGM1
#	include "pymac.h"
#endif

#ifdef ASSERT
#include <assert.h> /* V3.71 TM */
#else
    /* When ASSERT is not defined, eliminate assert calls.
     * This way, "#ifdef ASSERT" is not clobbering the source.
     *						ElB, 2001-12-17.
     */
#define assert(x)
#endif	/* ASSERT */
#include <stdio.h>
#include "py.h"
#include "pyproc.h"
#include "pydata.h"

boolean rubiech(
    square	i,
    square	itest,
    piece	p,
    smallint	*e_ub,
    boolean	(*evaluate)(square,square,square))	/* V1.7c  NG */
{
    numvec k;
    square j;

    e_ub[i]= obs;
    for (k= 9; k <= 16; k++) {
	j= i + vec[k];
	if (e_ub[j] == vide) {
	    if (rubiech(j, itest, p, e_ub, evaluate)) {
		return true;
	    }
	}
	else {
	    if ( e_ub[j] == p
	      && (*evaluate)(j, itest, itest))		/* V3.02  TLi */
	    {
		return true;
	    }
	}
    }
    return false;
}

boolean rrfouech(
    square	i,
    square	itest,
    numvec	k,
    piece	p,
    smallint	x,
    boolean	(*evaluate)(square,square,square))	 /* V2.1c  NG */
{
    numvec k1;
    piece p1;
    square j;

    if (e[i + k] == obs) {
	return false;
    }
    finligne(i, k, p1, j);
    if (p1 == p) {
	if ((* evaluate)(j, itest, itest)) {		/* V3.02  TLi */
	    return true;
	}
    }
    else if (x && (p1 == obs)) {
	j-= k;
	k1= 5;
	while (vec[k1] != k)
	    k1++;
	k1= k1<<1;
	if (rrfouech(j, itest, mixhopdata[1][k1], p, x - 1, evaluate)) {
	    return true;
	}
	k1--;
	if (rrfouech(j, itest, mixhopdata[1][k1], p, x - 1, evaluate)) {
	    return true;
	}
    }
    return false;
}

boolean rcardech(
    square	i,
    square	itest,
    numvec	k,
    piece	p,
    smallint	x,
    boolean	(*evaluate)(square,square,square))	/* V2.1c  NG */
{
    numvec k1;
    piece p1;
    square j;

    finligne(i, k, p1, j);
    if (p1 == p) {
	if ((* evaluate)(j, itest, itest)) {		/* V3.02  TLi */
		return true;
	}
    }
    else if (x && (p1 == obs)) {
	for (k1= 1; k1 <= 4; k1++) {
	    if (e[j + vec[k1]] != obs) {
		break;
	    }
	}
	if (k1 <= 4) {
	    j += vec[k1];
	    if (e[j] == p) {
		if ((* evaluate)(j, itest, itest)) {	/* V3.02  TLi */
		    return true;
		}
	    }
	    else if (e[j] == vide) {
		k1= 5;
		while (vec[k1] != k)
		    k1++;
		k1= k1<<1;
		if (e[j + mixhopdata[1][k1]] == obs)
		    k1--;
		if (rcardech(j, itest, mixhopdata[1][k1], p, x-1, evaluate))
		{
		    return true;
		}
	    }
	}
    }
    return false;
}


boolean feebechec(
    boolean	(* evaluate)(square,square,square))	/* V1.5c  NG */
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

boolean feenechec(boolean (* evaluate)(square,square,square)) {
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

boolean marsechecc(
    couleur	camp,
    boolean	(*evaluate)(square, square, square))
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
	    if ( (!is_phantomchess
		  || (e[z] != e[rb] && e[z] != e[rn])
		  || rex_phan)			  /* V3.51, V3.62  NG */
		&& rightcolor(e[z],camp))
	    {
		more_ren=0;
		do {
		    i= (*marsrenai)(p=e[z],
				      psp=spec[z], z, initsquare, camp);
		    if ((e[i]==vide) || (i==z)) {
			e[z]=vide;
			e[i]=p;
			spec[i]=psp;
			ch=(*checkfunctions[(p > 0)?p:-p])
				    (i, camp ? e[rn] : e[rb], evaluate);
			e[i]=vide;
			e[z]=p;
			spec[z]=psp;
			if (ch) {
			    return true;
			}
		    }	/* if */
		} while (more_ren);
	    }
	}
    }
    return false;
} /* marsechecc */

static boolean orig_rnechec(boolean (* evaluate)(square,square,square))
	/* V3.71 TM */
{
    /* detect, if black king is checked     */
    /* I didn't change this function, because it would be much (20% !)
	slower. V2.60  NG
     */
    numvec k;
    piece p;
    square sq;

    if (anymars) {				       /* SE/TLi 3.46 */
	boolean anymarscheck= marsechecc(noir, evaluate);
	if ( !is_phantomchess || anymarscheck) {	/* V3.47  NG */
	    return anymarscheck;
	}
    }
    if (rb) {						/* V3.1  TLi */
	if (CondFlag[whrefl_king]) {			/* V3.00  TLi,
							   V3.47  NG */
	    piece   *ptrans;
	    boolean flag = true;
	    for (ptrans= transmpieces; *ptrans; ptrans++) {
		if (nbpiece[-*ptrans]
		  && (*checkfunctions[*ptrans])(rb, -*ptrans, evaluate))
		{
		    flag= false;
		    if ((*checkfunctions[*ptrans])(rn, roib, evaluate)) {
			return true;
		    }
		}
	    }
	    if (!CondFlag[whtrans_king] || flag) {	/* V3.05  TLi,
							   V3.47  NG */
		for (k= 8; k > 0; k--) {		/* V2.4c  NG */
		    if (e[rn + vec[k]] == roib) {
			if ((*evaluate)(rb, rn, rn)) {
					     /* V1.3c  NG, V3.02  TLi */
			    imech(rb, rn);		 /* V2.4d  TM */
			}
		    }
		}
	    }
	}
	else {
	    if ( CondFlag[sting]
	      && (*checkfunctions[sb])(rn, roib, evaluate))
							/* V3.63  NG */
	    {
		return true;
	    }
	    for (k= 8; k > 0; k--) {			/* V2.4c  NG */
		if (e[rn + vec[k]] == roib) {
		    if ((*evaluate)(rb, rn, rn)) {	/* V1.3c  NG,
							   V3.02  TLi */
			imech(rb, rn);			/* V2.4d  TM */
		    }
		}
	    }
	}
    }
    if (nbpiece[pb]) {
	if ( rn >= bas + 48
	  || CondFlag[parrain]
	  || CondFlag[einstein])
	{
			       /* V1.6c  NG ,  V3.02  TLi , V3.1  TLi */
	    if (e[rn - 23] == pb) {
		if ((*evaluate)(rn - 23, rn, rn)) {	/* V1.3c  NG,
							   V3.02  TLi */
		    imech(rn - 23, rn);			/* V2.4d  TM */
		}
	    }
	    if (e[rn - 25] == pb) {
		if ((*evaluate)(rn - 25, rn, rn)) {	/* V1.3c  NG,
							   V3.02  TLi */
			imech(rn - 25, rn);		/* V2.4d  TM */
		}
	    }
	    if (ep[nbply] && RN_[nbply] != rn) {	    /* V3.45  TLi */
		/* ep captures of royal pawns */
		/* ep[nbply] != initsquare --> a pawn has made a
		   double/triple step.
		   RN_[nbply] != rn --> the black king has moved
		 */
		square sq= ep[nbply];	   /* square passed */
		if (e[sq-23] == pb && (*evaluate)(sq-23, sq, rn)) {
		    imech(sq-23, sq);
		}
		if (e[sq-25] == pb && (*evaluate)(sq-25, sq, rn)) {
		    imech(sq-25, sq);
		}
		if (ep2[nbply]) {	      /* Einstein triple step */
		    sq= ep2[nbply];
		    if (e[sq-23] == pb && (*evaluate)(sq-23, sq, rn)) {
			imech(sq-23, sq);
		    }
		    if (e[sq-25] == pb && (*evaluate)(sq-25, sq, rn)) {
			imech(sq-25, sq);
		    }
		}
	    }
	}
    }
    if (nbpiece[cb]) {
	for (k= 9; k <= 16; k++) {
	    if (e[rn + vec[k]] == cb) {
		if ((*evaluate)(rn + vec[k], rn, rn)) { /* V1.3c  NG,
							   V3.02  TLi */
		    imech(rn+vec[k], rn);		/* V2.4d  TM */
		}
	    }
	}
    }
    if (nbpiece[db] || nbpiece[tb]) {
	for (k= 4; k > 0; k--) {			/* V2.4c  NG */
	    finligne(rn, vec[k], p, sq);
	    if ((p == tb) || (p == db)) {
		if ((*evaluate)(sq, rn, rn)) {		/* V1.3c  NG,
							   V3.02  TLi */
		    ridimech(sq, rn, vec[k]);/* V2.4d TM */
		}
	    }
	}
    }
    if (nbpiece[db] || nbpiece[fb]) {
	for (k= 5; k <= 8; k++) {
	    finligne(rn, vec[k], p, sq);
	    if ((p == fb) || (p == db)) {
		if ((*evaluate)(sq, rn, rn)) {		/* V1.3c  NG,
							   V3.02  TLi */
		    ridimech(sq, rn, vec[k]);		/* V2.4d TM */
		}
	    }
	}
    }
    if (flagfee) {					/* V1.4c  NG */
	return feenechec(evaluate);
    }
    else {
	return false;
    }
}

boolean singleboxtype3_rnechec(
    boolean	(* evaluate)(square,square,square))	 /* V3.71 TM */
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

boolean (*rnechec)(boolean (* evaluate)(square,square,square))
					= &orig_rnechec; /* V3.71 TM */

static boolean orig_rbechec(boolean (* evaluate)(square,square,square)) /* V3.71 TM */
{
    /* detect, if white king is checked  */
    /* I didn't change this function, because it would be much (20% !)
	slower. V2.60  NG
     */

    numvec k;
    piece p;
    square sq;

    if (anymars) {		    /* SE/TLi 3.46 */
	boolean anymarscheck= marsechecc(blanc, evaluate);
	if ( !is_phantomchess || anymarscheck) {	/* V3.47  NG */
	    return anymarscheck;
	}
    }

    if (rn) {						/* V3.1  TLi */
	if (CondFlag[blrefl_king]) {			/* V3.00  TLi,
							   V3.47  NG */
	    piece   *ptrans;
	    boolean flag= true;
	    for (ptrans= transmpieces; *ptrans; ptrans++) {
		if (nbpiece[*ptrans]
		  && (*checkfunctions[*ptrans])(rn, *ptrans, evaluate))
		{
		    flag= false;
		    if ((*checkfunctions[*ptrans])(rb, roin, evaluate))
		    {
			return true;
		    }
		}
	    }
	    if (!CondFlag[bltrans_king] || flag) {	/* V3.05  TLi,
							   V3.47  NG */
		for (k= 8; k > 0; k--) {				/* V2.4c  NG */
		    if (e[rb + vec[k]] == roin) {
			if ((*evaluate)(rn, rb, rb)) {	/* V1.3c NG,
							   V3.02  TLi */
				imech(rn, rb);		/* V2.4d  TM */
			}
		    }
		}
	    }
	}
	else {
	    if ( CondFlag[sting]
	      && (*checkfunctions[sb])(rb, roin, evaluate))
							/* v3.63  NG */
	    {

		    return true;
	    }
	    for (k= 8; k > 0; k--) {			/* V2.4c  NG */
		if (e[rb + vec[k]] == roin) {
		    if ((*evaluate)(rn, rb, rb)) {	/* V1.3c  NG,
							   V3.02  TLi */
			imech(rn, rb);			/* V2.4d  TM */
		    }
		}
	    }
	}
    }
    if (nbpiece[pn]) {
	if ((rb <= haut - 48) || CondFlag[parrain]
	  || CondFlag[einstein])
	{
				/* V1.6c  NG , V3.02  TLi , V3.1  TLi */
	    if (e[rb + 23] == pn) {
		if ((*evaluate)(rb + 23, rb, rb)) {	/* V1.3c  NG,
							   V3.02  TLi */
		    imech(rb + 23, rb);			 /* V2.4d  TM */
		}
	    }
	    if (e[rb + 25] == pn) {
		if ((*evaluate)(rb + 25, rb, rb)) {	/* V1.3c  NG,
							   V3.02  TLi */
		    imech(rb + 25, rb);			/* V2.4d  TM */
		}
	    }
	    if (ep[nbply] && RB_[nbply] != rb) {	/* V3.45  TLi */
		/* ep captures of royal pawns.
		   ep[nbply] != initsquare
		      --> a pawn has made a double/triple step.
		   RB_[nbply] != rb
		      --> the white king has moved
		 */
		square sq= ep[nbply];		     /* square passed */
		if (e[sq+23] == pn && (*evaluate)(sq+23, sq, rb)) {
		    imech(sq+23, sq);
		}
		if (e[sq+25] == pn && (*evaluate)(sq+25, sq, rb)) {
		    imech(sq+25, sq);
		}
		if (ep2[nbply]) {	      /* Einstein triple step */
		    sq= ep2[nbply];
		    if (e[sq+23] == pn && (*evaluate)(sq+23, sq, rb)) {
			imech(sq+23, sq);
		    }
		    if (e[sq+25] == pn && (*evaluate)(sq+25, sq, rb)) {
			imech(sq+25, sq);
		    }
		}
	    }
	}
    }
    if (nbpiece[cn]) {
	for (k= 9; k <= 16; k++) {
	    if (e[rb + vec[k]] == cn) {
		if ((*evaluate)(rb + vec[k], rb, rb)) { /* V1.3c  NG,
							   V3.02  TLi */
		    imech(rb+vec[k], rb);		/* V2.4d  TM */
		}
	    }
	}
    }
    if (nbpiece[dn] || nbpiece[tn]) {

	for (k= 4; k > 0; k--) {			/* V2.4c  NG */
	    finligne(rb, vec[k], p, sq);
	    if ((p == tn) || (p == dn)) {
		if ((*evaluate)(sq, rb, rb)) {		/* V1.3c  NG,
							   V3.02  TLi */
		    ridimech(sq, rb, vec[k]);/* V2.4d TM */
		}
	    }
	}
    }
    if (nbpiece[dn] || nbpiece[fn]) {
	for (k= 5; k <= 8; k++) {
	    finligne(rb, vec[k], p, sq);
	    if ((p == fn) || (p == dn)) {
		if ((*evaluate)(sq, rb, rb)) {		/* V1.3c  NG,
							   V3.02  TLi */
		    ridimech(sq, rb, vec[k]);/* V2.4d TM */
		}
	    }
	}
    }

    if (flagfee)					/* V1.4c  NG */
	return feebechec(evaluate);
    else
	return false;
}

boolean singleboxtype3_rbechec(boolean (* evaluate)(square,square,square)) /* V3.71 TM */
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

boolean (*rbechec)(boolean (* evaluate)(square,square,square))
					= &orig_rbechec; /* V3.71 TM */


boolean rncircech(square id, square ia, square ip) {
    if (id == (*circerenai)(e[rn], spec[rn], ip, id, blanc)) {
							 /* V3.55 SE */
	return false;
    }
    else
	return (*eval_2)(id, ia, ip);			/* V2.90  NG */
}

boolean rbcircech(square id, square ia, square ip) {
    if (id == (*circerenai)(e[rb], spec[rb], ip, id, noir)) {
							/* V3.55 SE */
	return false;
    }
    else {
	return (*eval_2)(id, ia, ip);			/* V2.90  NG */
    }
}

boolean rnimmunech(square id, square ia, square ip) {
    immrenroin= (*immunrenai)(e[rn], spec[rn], ip, id, blanc);
							/* V3.55 SE */
    if ((e[immrenroin] != vide && id != immrenroin)) {
	return false;
    }
    else {
	return (*eval_2)(id, ia, ip);			/* V2.90  NG */
    }
}

boolean rbimmunech(square id, square ia, square ip) {
    immrenroib= (*immunrenai)(e[rb], spec[rb], ip, id, noir);
							/* V3.55 SE */
    if ((e[immrenroib] != vide && id != immrenroib)) {
	return false;
    }
    else {
	return (*eval_2)(id, ia, ip);			/* V2.90  NG */
    }
}

boolean echecc(couleur camp)
{
    if ((camp == blanc) ^ CondFlag[vogt]) {
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
		      (e[rb], spec[rb], rb, initsquare, noir)] == vide))
							 /* V3.55 SE */
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
		if ((p= e[*bnp])
		  && p>roib
		  && (*circerenai)(p,spec[*bnp],*bnp,initsquare,noir)==rb)
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
	if (CondFlag[bicolores]) {	/* V3.1  TLi */
	    boolean flag = rbechec(eval_white);
	    if (!flag) {
		square	rn_sic = rn;
		rn = rb;
		CondFlag[bicolores] = false;
		flag = rnechec(eval_black);
		CondFlag[bicolores] = true;
		rn = rn_sic;
	    }
	    return flag;
	}
	else {
	    return rbechec(eval_white);
	}
    }
    else {	  /* camp == noir */
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
		     (e[rn], spec[rn], rn, initsquare, blanc)] == vide))
							/* V3.55 SE */
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
		if ((p= e[*bnp])
		  && p<roin
		  && (*circerenai)
			(p,spec[*bnp],*bnp,initsquare,blanc)==rn)
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
	if (CondFlag[bicolores]) {	/* V3.1  TLi */
	    boolean flag = rnechec(eval_black);
	    if (!flag) {
		square	rb_sic = rb;
		rb = rn;
		CondFlag[bicolores] = false;
		flag = rbechec(eval_white);
		CondFlag[bicolores] = true;
		rb = rb_sic;
	    }
	    return flag;
	}
	else {
	    return rnechec(eval_black);
	}
    }
} /* end of echecc */

boolean testparalyse(square id, square ia, square ip) {
    if (flaglegalsquare && !legalsquare(id, ia, ip))
	return false;
    else
	return TSTFLAG(spec[id], Paralyse);
}

boolean paraechecc(square id, square ia, square ip) {
    if (  TSTFLAG(spec[id], Paralyse)
      || (flaglegalsquare && !legalsquare(id, ia, ip)))
    {
	return false;
    }
    else {
	return (!paralysiert(id));
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

/* V3.03  TLi  -  Jaeger */

boolean rhuntcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    /* detect check of a rook/bishop-hunter */
    /* it's not dependent of the piece-color !! */
    /* always moves up (rook), down (bishop) !!  V3.04	NG */

    return ridcheck(i, 4, 4, p, evaluate)
	|| ridcheck(i, 5, 6, p, evaluate);
}

boolean bhuntcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    /* detect check of a bishop/rook-hunter */
    /* it's not dependent of the piece-color !! */
    /* always moves up (bishop), down (rook) !!  V3.04	NG */

    return ridcheck(i, 2, 2, p, evaluate)
	|| ridcheck(i, 7, 8, p, evaluate);
}

/* new anticirce implementation by TLi	V3.39 */

boolean AntiCirceEch(
    square	id,
    square	ia,
    square	ip,
    couleur	camp)					/* V3.39  TLi */
{
    if (is_pawn(e[id]) && PromSq(advers(camp), ip)) {
	/* Pawn checking on last rank */
	piece	pprom= getprompiece[vide];
	square	cren;
	do {
	    cren= (*antirenai)(pprom, spec[id], ip, id, camp);
	    pprom= getprompiece[pprom];
	} while (!LegalAntiCirceMove(cren, ip, id) && pprom != vide);
	if (	!LegalAntiCirceMove(cren, ip, id)  /* V3.62  TLi */
	     && pprom == vide)
	{
	    return false;
	}
    }
    else {
	square	cren;
	cren= (*antirenai)( TSTFLAG(spec[id], Chameleon)
			      ? champiece(e[id])
			      : e[id],
			    spec[id], ip, id, camp);
	if (!LegalAntiCirceMove(cren, ip, id)) {
	    return false;
	}
    }
    return (*eval_2)(id, ia, ip);
} /* AntiCirceEch */

boolean rnanticircech(square id, square ia, square ip) /* V3.39  TLi */
{
    return AntiCirceEch(id, ia, ip, noir);
} /* rnanticirech */

boolean rbanticircech(square id, square ia, square ip) {/* V3.39  TLi */
    return AntiCirceEch(id, ia, ip, blanc);
} /* rbanticircech */


/*****	V3.71  TM  *****  begin  *****/
boolean rnsingleboxtype1ech(square id, square ia, square ip) {
    if (is_pawn(e[id]) && PromSq(blanc, ip)) {
	/* Pawn checking on last rank */
	return next_singlebox_prom(vide,blanc)!=vide;
    }
    else {
	return (*eval_2)(id, ia, ip);
    }
}

boolean rbsingleboxtype1ech(square id, square ia, square ip) {
    if (is_pawn(e[id]) && PromSq(noir, ip)) {
	/* Pawn checking on last rank */
	return next_singlebox_prom(vide,noir)!=vide;
    }
    else {
	return (*eval_2)(id, ia, ip);
    }
}
/*****	V3.71  TM  *****  end  *****/


/*****	V3.20  TLi  *****  begin  *****/
/* boolean rbexactech(square id, square ia, square ip)	V3.22  TLi */
boolean rbultraech(square id, square ia, square ip) {
    boolean how_is_flagkil = flagkil;
    square  how_is_cakil = cakil,
	    how_is_cdkil = cdkil,
	    how_is_cpkil = cpkil;
    boolean check;

    /* if we_generate_consmoves is set this function is never called.
      Let's check this for a while.
     */
    if (we_generate_exact) {
	StdString(
	  "rbultra is called while we_generate_exact is set!\n");
    }
    nextply();
    cdkil = id;
    cakil = ia;
    flagkil = false;
    trait[nbply]= noir;
    /* flagminmax= false;				   V3.44  TLi */
    /* flag_minmax[nbply]= false;			   V3.44  TLi */
    we_generate_exact = true;
    gen_bl_ply();
    finply();
    check = flagkil;
    we_generate_exact = false;
    flagkil = how_is_flagkil;
    /* flagminmax = how_is_flagminmax;			   V3.44  TLi */
    cakil = how_is_cakil;
    cdkil = how_is_cdkil;
    cpkil = how_is_cpkil;

    return  check ? (*eval_2)(id, ia, ip) : false;
}

boolean rnultraech(square id, square ia, square ip) {
    boolean how_is_flagkil = flagkil;
    /* how_is_flagminmax = flagminmax;			   V3.44  TLi */
    square  how_is_cakil = cakil,
	    how_is_cdkil = cdkil,
	    how_is_cpkil = cpkil;
    boolean check;

    nextply();
    cdkil = id;
    cakil = ia;
    flagkil = false;
    trait[nbply]= blanc;
    /* flagminmax= false;	  V3.44  TLi */
    /* flag_minmax[nbply]= false;  V3.44  TLi */
    we_generate_exact = true;
    gen_wh_ply();
    finply();
    check = flagkil;
    we_generate_exact = false;
    flagkil = how_is_flagkil;
    /* flagminmax = how_is_flagminmax;	 V3.44	TLi */
    cakil = how_is_cakil;
    cdkil = how_is_cdkil;
    cpkil = how_is_cpkil;

    return (check ? (*eval_2)(id, ia, ip) : false);
}
/*****	V3.20  TLi  *****   end   *****/

/**********  V3.12  TLi  begin	**********/
boolean skycharcheck(
    piece	p,
    square	king,
    square	chp,
    square	arr1,
    square	arr2,
    boolean	(*evaluate)(square, square, square))
{
    if (e[chp] == p) {
	if ((e[arr1] == vide) && evaluate(chp, arr1, king)) {
	    return  True;
	}
	if ((e[arr2] == vide) && evaluate(chp, arr2, king)) {
	    return  True;
	}
    }
    return  False;
}

boolean skyllacheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    return  skycharcheck(p, i, i +  1, i + 23, i - 25, evaluate)
	 || skycharcheck(p, i, i -  1, i + 25, i - 23, evaluate)
	 || skycharcheck(p, i, i + 24, i - 23, i - 25, evaluate)
	 || skycharcheck(p, i, i - 24, i + 23, i + 25, evaluate);
}

boolean charybdischeck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    return  skycharcheck(p, i, i + 25, i -  1, i - 24, evaluate)
	 || skycharcheck(p, i, i - 25, i +  1, i + 24, evaluate)
	 || skycharcheck(p, i, i + 23, i +  1, i - 24, evaluate)
	 || skycharcheck(p, i, i - 23, i -  1, i + 24, evaluate);
}

/**********  V3.12  TLi  end  **********/
