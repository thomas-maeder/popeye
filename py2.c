/********************* MODIFICATIONS to py2.c **************************
**
** Date       Who  What
**
** 2001/01/14 SE   new specification : ColourChanging
**
** 2001/02/05 TLi  new piece: Moariderlion
**
** 2001/10/02 TLi  new piece: Dolphin (Grashopper + Kangaroo)
**
** 2002/05/18 NG   new pieces: rabbit, bob
**
**************************** End of List ******************************/

#ifdef macintosh	/* is always defined on macintosh's  SB */
#	define SEGM1
#	include "pymac.h"
#endif


#include <stdio.h>
#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"

boolean eval_ortho(square id, square ia, square ip) {
    return true;
}

boolean legalsquare(square id, square ia, square ip) {
    /* id - case dep., ia - case arr., ip - case pri. */
    if (CondFlag[koeko]) {				/* V1.3c  NG */
	if (nocontact(id, ia, ip))
	    return(false);
    }
    if (CondFlag[gridchess]) {
	    if (GridNum(id) == GridNum(ia))		/* V3.22  TLi */
		    return(false);			/* V1.3c  NG */
    }
    if (CondFlag[blackedge]) {				 /* V2.90  NG */
	if (e[id] <= roin)
	    if (NoEdge(ia))				/* V3.22  TLi */
		return(false);
    }
    if (CondFlag[whiteedge]) {				/* V2.90  NG */
	if (e[id] >= roib)
	    if (NoEdge(ia))				/* V3.22  TLi */
		return(false);
    }
    if (CondFlag[bichro]) {
	if (SquareCol(id) == SquareCol(ia))		/* V3.22  TLi */
	    return(false);				/* V1.3c  NG */
    }
    if (CondFlag[monochro]) {
	if (SquareCol(id) != SquareCol(ia))		/* V3.22  TLi */
	    return(false);				/* V1.3c  NG */
    }
    if (TSTFLAG(spec[id], Jigger)) {			/* V3.1  TLi */
	if (nocontact(id, ia, ip))
	    return(false);
    }
    if (CondFlag[newkoeko]) {				/* V3.1  TLi */
	if (nocontact(id, ia, ip)
	  != nocontact(initsquare, id, initsquare))
	{
	    return false;
	}
    }
    return(true);
} /* end of legalsquare */

boolean imok(square i, square j) {			/* V2.4d  TM */
    /* move i->j ok? */
    smallint	count;
    smallint	diff = j - i;
    square	j2;					/* V2.90  NG */

    for (count= inum[nbply]-1; count >= 0; count--) {
	j2= isquare[count] + diff;
	if ((j2 != i) && (e[j2] != vide)) {
	    return false;
	}
    }
    return(true);
}


boolean ridimok(square i, square j, smallint diff) {	/* V2.4d  TM */

    /* move i->j in steps of diff ok? */
    square  i2= i;
    boolean ret;
    piece   p= e[i];

    e[i]= vide;/* an imitator might be disturbed by the moving rider! */
    do {
	i2-= diff;
    } while (imok(i, i2) && (i2 != j));

    ret= (i2 == j) && imok (i, j);
    e[i]= p;			 /* only after the last call of imok! */
    return ret;
}

boolean hopimok(square i, square j, square k, smallint diff) {
							/* V3.12  TM */
    /* hop i->j hopping over k in steps of diff ok? */
    square	i2= i;
    piece	p=e[i];
    smallint	l;
    boolean	ret= true;

    if (TSTFLAG(spec[i], ColourChange)) {		 /* V3.64 SE */
	chop[nbcou+1]= k;
	ret= true;
    }

    if (!CondFlag[imitators]) {
	return ret;
    }

    /* an imitator might be disturbed by the moving hopper! */
    e[i]= vide;

    /* Are the lines from the imitators to the square to hop over free?
    */
    do {
	i2+= diff;
    } while (imok(i, i2) && (i2 != k));

    ret = i2 == k;
    if (ret) {
	/* Are the squares the imitators have to hop over occupied? */
	l= inum[nbply];
	while (l>0) {
	    if (e[isquare[--l]+k-i] == vide) {
		ret= false;
		break;
	    }
	}
    }

    if (ret) {
	do {
	    i2+= diff;
	} while (imok(i,i2) && (i2 != j));
    }

    ret = ret && i2==j && imok(i,j);

    e[i]= p;
    return ret;
}


void joueim(smallint diff) {				/* V2.4d  TM */
    smallint i;

    for (i=inum[nbply]-1; i >= 0; i--)			/* V2.90  NG */
	isquare[i]+= diff;

}

boolean rmhopech(
    square	i,
    numvec	kend,
    numvec	kanf,
    smallint	m,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.1c, V3.62  NG */
{
    square j, j1;
    numvec k, k1;
    piece p1;

    /* ATTENTION:
     *	m == 0: moose	 45 degree hopper
     *	m == 1: eagle	 90 degree hopper
     *	m == 2: sparrow 135 degree hopper
     *
     *	kend == 8, kanf == 1: all types        (moose, eagle, sparrow)
     *	kend == 8, kanf == 5: orthogonal types (rookmoose, rooksparrow,
     *						bishopeagle)
     *	kend == 4, kanf == 1: diagonal types   (bishopmoose,
     *						bishopsparrow,
     *						rookeagle)
     *
     *	YES, this is definitely different to generating their moves ...
     *								     NG
     */

    for (k= kend; k >= kanf; k--) {
	j = i + vec[k];
	if (abs(e[j]) >= roib) {
	    k1= k<<1;
	    finligne(j, mixhopdata[m][k1], p1, j1);
	    if (p1 == p) {
		if ((*evaluate)(j1, i, i))		/* V3.02  TLi */
		    return true;
	    }
	    finligne(j, mixhopdata[m][k1 - 1], p1, j1);
	    if (p1 == p) {
		if ((*evaluate)(j1, i, i))		/* V3.02  TLi */
		    return true;
	    }
	}
    }
    return false;
}

boolean rcsech(
    square  i,
    numvec  k,
    numvec  k1,
    piece   p,
    boolean (*evaluate)(square,square,square))		/* V2.1c  NG */
{
    square j= i;

    /* ATTENTION: There is a parameter dependency between the
     *		  indexes of k and of k1 !
     *			  p		  index of k (ik) I index of k1
     *		  ----------------------------------------I------------
     *		  Spiralspringer	   9 to 16	  I 25 - ik
     *							  I
     *		  DiagonalSpiralspringer   9 to 14	  I 23 - ik
     *		  DiagonalSpiralspringer  15 to 16	  I 27 - ik
     */

    while (e[j+= k] == vide) {
	if (e[j+= k1] != vide)
	    break;
    }

    if (e[j] == p) {
	if ((*evaluate)(j, i, i))			/* V3.02  TLi */
	    return true;
    }

    j = i;
    while (e[j+= k] == vide) {
	if (e[j-= k1] != vide)
	    break;
    }

    if (e[j] == p) {
	if ((*evaluate)(j, i, i))			/* V3.02  TLi */
	    return true;
    }

    return false;
}

boolean cscheck(
    square  i,
    piece   p,
    boolean (*evaluate)(square,square,square))		/* V2.1c  NG */
{
    numvec  k;

    for (k= 9; k <= 16; k++) {
	if (rcsech(i, vec[k], vec[25 - k], p, evaluate)) {
	    return true;
	}
    }
    return false;
}

boolean bscoutcheck(
    square  i,
    piece   p,
    boolean (* evaluate)(square,square,square))		/* V3.05  NG */
{
    numvec  k;

    for (k= 5; k <= 8; k++) {
	if (rcsech(i, vec[k], vec[13 - k], p, evaluate)) {
	    return true;
	}
    }
    return false;
}

boolean gscoutcheck(
    square  i,
    piece   p,
    boolean (* evaluate)(square,square,square))		/* V3.05  NG */
{
    numvec  k;

    for (k= 4; k > 0; k--) {
	if (rcsech(i, vec[k], vec[5 - k], p, evaluate)) {
	    return true;
	}
    }
    return false;
}

boolean rrefcech(
    square	i,
    square	i1,
    smallint	x,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.1c  NG */
{
    square j;
    numvec k;

    /* ATTENTION:   first call of rrefech: x must be 2 !!   */

    for (k= 9; k <= 16; k++) {
	if (x) {
	    if (e[j= i1 + vec[k]] == vide) {
		/* if (boolnoedge[j]) */
		if (NoEdge(j)) {			/* V3.22  TLi */
		    if (rrefcech(i, j, x - 1, p, evaluate))
			return true;
		}
	    }
	    else if (e[j] == p) {
		if ((*evaluate)(j, i, i))		/* V3.02  TLi */
		    return true;
	    }
	}
	else {
	    for (k= 9; k <= 16; k++) {
		if (e[j= i1 + vec[k]] == p) {
		    if ((*evaluate)(j, i, i))		/* V3.02  TLi */
			    return true;
		}
	    }
	}
    }
    return false;
}

boolean nequicheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))   /* V2.60  NG */
{
    numvec k, k1;
    square j, j1, j2;

    j1= coinequis(i);
    for (k= 3; k >= 0; k--) {
	for (k1= 72; k1 >= 0; k1-= 24) {		/* V2.90  NG */
	    if ( e[j2 = j1 + k + k1] != vide	 /* V1.5c, V2.90  NG */
	      && e[j= (j2<<1) - i] == p		 /* V1.5c, V2.90  NG */
	      && i != j					/* V3.54  NG */
	      && (*evaluate)(j, i, i)			/* V3.02  TLi */
	      && hopimcheck(j,i,j2,j2-j))		/* V3.12  TM */
	    {
		return true;
	    }
	}
    }
    return false;
}

boolean vizircheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 1, 4, p, evaluate);		/* V2.60  NG */
}

boolean dabcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 61, 64, p, evaluate);		/* V2.60  NG */
}

boolean ferscheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 5, 8, p, evaluate);		/* V2.60  NG */
}


boolean alfilcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 65, 68, p, evaluate);		/* V2.60  NG */
}

boolean rccinqcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 69, 80, p, evaluate);		/* V2.60  NG */
}


boolean bucheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 49, 60, p, evaluate);		/* V2.60  NG */
}


boolean gicheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 41, 48, p, evaluate);		/* V2.60  NG */
}

boolean chcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 33, 40, p, evaluate);		/* V2.60  NG */
}


boolean zcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 25, 32, p, evaluate);		/* V2.60  NG */
}

boolean leap16check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.38  NG */
{
    return leapcheck(i, 89, 96, p, evaluate);
}

boolean leap24check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))     /* V3.42  TLi */
{
    return leapcheck(i, 97, 104, p, evaluate);
}

boolean leap35check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.42  TLi */
{
    return leapcheck(i, 105, 112, p, evaluate);
}

boolean leap37check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.34  TLi */
{
    return leapcheck(i, 81, 88, p, evaluate);
}

boolean okapicheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.32  TLi */
{
    return leapcheck(i, 17, 32, p, evaluate);	  /* knight+zebra !!! */
}

boolean bisoncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.60  TLi */
{
    return leapcheck(i, 25, 40, p, evaluate);	   /* camel+zebra !!! */
}

boolean elephantcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  TLi */
{
    return ridcheck(i, 1, 16, p, evaluate);	/* queen+nightrider  */
}

boolean ncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return ridcheck(i, 9, 16, p, evaluate);		/* V2.60  NG */
}

boolean scheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rhopcheck(i, 1, 8, p, evaluate);		/* V2.60  NG */
}

boolean grasshop2check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.34  TLi */
{
    return rhop2check(i, 1, 8, p, evaluate);
}

boolean grasshop3check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.34  TLi */
{
    return rhop3check(i, 1, 8, p, evaluate);
}

/***** V3.44  SE  begin *****/

boolean kinghopcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    return shopcheck(i, 1, 8, p, evaluate);		/* SE */
}

boolean doublegrascheck(
    square	i,
    piece	p,
    boolean	(* evaluate)(square,square,square))	/* V3.44  SE */
{
    /* SE */ /* W.B.Trumper feenschach 1968 - but here
		 null moves will not be allowed by Popeye
	     */
    piece	p1;
    square	j, j1, j2;
    numvec	k, k1;

    for (k=8; k>0; k--) {
	j=i+vec[k];
	if (abs(e[j]) >= roib) {
	    while ((e[j+=vec[k]])==vide) {
		for (k1=8; k1>0; k1--) {
		    j1=j+vec[k1];
		    if (abs(e[j1]) >= roib) {
			finligne(j1, vec[k1], p1, j2);
			if (p1==p) {
			      if ((*evaluate)(j2, i, i)) {
				  return true;
			    }
			}
		    }
		}
	    }
	}
    }
    return false;
}

/***** V3.44  SE  end *****/

/***** V3.22  TLi ***** begin *****/

boolean contragrascheck(
    square	i,
    piece	p,
    boolean	(* evaluate)(square,square,square))	/* V2.60  NG */
{
    return crhopcheck(i, 1, 8, p, evaluate);
}

/***** V3.22 TLi  *****  end  *****/

boolean nightlocustcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.53  TLi */
{
    return marincheck(i, 9, 16, p, evaluate);
} /* nightlocustcheck */

boolean loccheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return marincheck(i, 1, 8, p, evaluate);
}

boolean tritoncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return marincheck(i, 1, 4, p, evaluate);
}

boolean nereidecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return marincheck(i, 5, 8, p, evaluate);
}

boolean nightriderlioncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.64  TLi */
{
    return lrhopcheck(i, 9, 16, p, evaluate);
} /* nightriderlioncheck */

boolean lioncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return lrhopcheck(i, 1, 8, p, evaluate);		/* V2.60  NG */
}

boolean t_lioncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return lrhopcheck(i, 1, 4, p, evaluate);
}

boolean f_lioncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return lrhopcheck(i, 5, 8, p, evaluate);
}

boolean rosecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k, k1;
    square  j;

    for (k= 9; k <= 16; k++) {
	j= i;
	k1= 0;
	do {
	    j+= vec[k + k1];
	    k1++;
	} while (e[j] == vide);
	if ((e[j] == p) && (j != i)) {	      /* bug fixed. V1.3c  NG */
	    if ((*evaluate)(j, i, i)) {			/* V3.02  TLi */
		return true;
	    }
	}
	j= i;
	k1= 8;
	do {
	    j+= vec[k + k1];
	    k1--;
	} while (e[j] == vide);
	if ((e[j] == p) && (j != i)) {
	    if ((*evaluate)(j, i, i)) {			/* V3.02  TLi */
		return true;
	    }
	}
    }
    return false;
}

#define _rlion	0
#define _rhopper 1

boolean rlh_check(
    square	i,
    piece	p,
    int		type,
    numvec	k,
    numvec	k1,
    boolean	(*evaluate)(square,square,square))
{
    /* detects check by rose hopper or lion on a single rose line
       defined by k and direction k1  -  V3.23	TLi
       Bug fixed -- TLi, V3.39
    */
    square j = i;
    numvec kinc = k1 ? -1 : 1;

    /* find the hurdle */
    switch (type) {

      case _rlion:
	do {
	    j+= vec[k + k1];
	    k1+= kinc;
	} while (e[j] == vide);
	if (j == i) {
	    return false;
	}
	break;

      case _rhopper:
	j+= vec[k + k1];
	k1+= kinc;
	if (e[j] == vide) {
	    return false;
	}
	break;
    } /* end switch */

    if (e[j] == obs) {
	return false;
    }

    /* is it really a piece */
    do {
	j+= vec[k + k1];
	k1+= kinc;
    } while (e[j] == vide);

    return (((e[j] == p) && (j != i)) && ((*evaluate)(j, i, i)));
} /* rlh_check */


boolean roselioncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    /* detects check by a rose lion  -	V3.23  TLi */
    numvec  k;

    for (k= 9; k <= 16; k++) {
	if (rlh_check(i, p, _rlion, k, 0, evaluate) ||
	    rlh_check(i, p, _rlion, k, 8, evaluate))
	{
	    return true;
	}
    }
    return false;
} /* roselioncheck */

boolean rosehoppercheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    /* detects check by a rose hopper  -  V3.23  TLi */
    numvec  k;

    for (k= 9; k <= 16; k++) {
	if (rlh_check(i, p, _rhopper, k, 0, evaluate) ||
	    rlh_check(i, p, _rhopper, k, 8, evaluate))
	{
	    return true;
	}
    }
    return false;
} /* rosehoppercheck */

boolean maocheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    if (e[i + 25] == vide) {
	if (e[i + 26] == p) {
	    if ((*evaluate)(i + 26, i, i))  /* V3.02  TLi */
		return true;
	}
	if (e[i + 49] == p) {
	    if ((*evaluate)(i + 49, i, i))  /* V3.02  TLi */
		return true;
	}
    }
    if (e[i - 25] == vide) {
	if (e[i - 26] == p) {
	    if ((*evaluate)(i - 26, i, i))  /* V3.02  TLi */
		return true;
	}
	if (e[i - 49] == p) {
	    if ((*evaluate)(i - 49, i, i))  /* V3.02  TLi */
		return true;
	}
    }
    if (e[i + 23] == vide) {
	if (e[i + 22] == p) {
	    if ((*evaluate)(i + 22, i, i))  /* V3.02  TLi */
		return true;
	}
	if (e[i + 47] == p) {
	    if ((*evaluate)(i + 47, i, i))  /* V3.02  TLi */
		return true;
	}
    }
    if (e[i - 23] == vide) {
	if (e[i - 47] == p) {
	    if ((*evaluate)(i - 47, i, i))  /* V3.02  TLi */
		return true;
	}
	if (e[i - 22] == p) {
	    if ((*evaluate)(i - 22, i, i))  /* V3.02  TLi */
		return true;
	}
    }
    return false;
}

boolean moacheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    if (e[i + 24] == vide) {
	if (e[i + 47] == p) {
	    if ((*evaluate)(i + 47, i, i))		/* V3.02  TLi */
		return true;
	}
	if (e[i + 49] == p) {
	    if ((*evaluate)(i + 49, i, i))		/* V3.02  TLi */
		return true;
	}
    }
    if (e[i - 24] == vide) {
	if (e[i - 47] == p) {
	    if ((*evaluate)(i - 47, i, i))		/* V3.02  TLi */
		return true;
	}
	if (e[i - 49] == p) {
	    if ((*evaluate)(i - 49, i, i))		/* V3.02  TLi */
		return true;
	}
    }
    if (e[i + 1] == vide) {
	if (e[i + 26] == p) {
	    if ((*evaluate)(i + 26, i, i))		/* V3.02  TLi */
		return true;
	}
	if (e[i - 22] == p) {
	    if ((*evaluate)(i - 22, i, i))		/* V3.02  TLi */
		return true;
	}
    }
    if (e[i - 1] == vide) {
	if (e[i - 26] == p) {
	    if ((*evaluate)(i - 26, i, i))		/* V3.02  TLi */
		return true;
	}
	if (e[i + 22] == p) {
	    if ((*evaluate)(i + 22, i, i))		/* V3.02  TLi */
		return true;
	}
    }
    return false;
}

boolean paocheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return lrhopcheck(i, 1, 4, p, evaluate);		/* V2.60  NG */
}

boolean vaocheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return lrhopcheck(i, 5, 8, p, evaluate);		/* V2.60  NG */
}

boolean naocheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  TLi */
{
    return lrhopcheck(i, 9, 16, p, evaluate);		/* V3.62  TLi */
}

boolean leocheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return lrhopcheck(i, 1, 8, p, evaluate);		/* V2.60  NG */
}

boolean pbcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    if (anymars) {	    /* NG  3.47 */
	boolean anymarscheck=
		 ((p == e[rb]) && (e[i-24] == p))
	      || ((p == e[rn]) && (e[i+24] == p));
	if ( !is_phantomchess || anymarscheck)
	    return anymarscheck;
    }

    if (p <= roin) {					/* V3.47  NG */
	if ( i <= haut - 48				/* V3.02  TLi */
	  || CondFlag[parrain]
	  || CondFlag[einstein]				 /* V3.2  TLi */
	  || p == orphann)				 /* V3.2  TLi */
	{
	    if (e[i + 24] == p
	      && (*evaluate)(i + 24, i, i))		/* V3.02  TLi */
	    {
		return true;
	    }
	    if (ep[nbply] && RB_[nbply] != rb) {	/* V3.45  TLi */
		/* ep captures of royal pawns */
		square sq= ep[nbply];		     /* square passed */
		if (e[sq+24] == pbn && (*evaluate)(sq+24, sq, rn))
		    imech(sq+24, sq);
	    }
	}
    }
    else {	  /* hopefully (p >= roib) */		/* V3.47  NG */
	if ( i >= bas + 48				/* V3.02  TLi */
	  || CondFlag[parrain]
	  || CondFlag[einstein]				/* V3.2  TLi */
	  || p == orphanb)
	{						 /* V3.2  TLi */
	    if	 (e[i - 24] == p
	      && (*evaluate)(i - 24, i, i))		/* V3.02  TLi */
	    {
		return true;
	    }
	    if (ep[nbply] && RN_[nbply] != rn) {	    /* V3.45  TLi */
		/* ep captures of royal pawns */
		square sq= ep[nbply];	   /* square passed */
		if (e[sq-24] == pbb && (*evaluate)(sq-24, sq, rn)) {
		    imech(sq-24, sq);
		}
	    }
	}
    }
    return false;
}

boolean bspawncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    piece   p1;
    square  j;

    if ((p == bspawnn)
      || (CondFlag[blrefl_king] && p == roin))		/* V3.04  NG */
    {
	if (i <= haut - 24) {	       /* it can move from eigth rank */
	    finligne(i, 24, p1, j);
	    if (p1 == p) {
		if ((*evaluate)(j, i, i))		/* V3.02  TLi */
		    return true;
	    }
	}
    }
    else {/* hopefully ((p == bspawnb)
	   || (CondFlag[whrefl_king] && p == roib)) */	/* V3.04  NG */

	if (i >= bas + 24) {	       /* it can move from first rank */
	    finligne(i, -24, p1, j);
	    if (p1 == p) {
		if ((*evaluate)(j, i, i))		/* V3.02  TLi */
		    return true;
	    }
	}
    }
    return false;
}

boolean spawncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    piece   p1;
    square  j;

    if ((p == spawnn)
      || (CondFlag[blrefl_king] && p == roin))		/* V3.04  NG */
    {
	if (i <= haut - 24) {	       /* it can move from eigth rank */
	    finligne(i, 23, p1, j);
	    if (p1 == p) {
		if ((*evaluate)(j, i, i))		/* V3.02  TLi */
		    return true;
	    }
	    finligne(i, 25, p1, j);
	    if (p1 == p) {
		if ((*evaluate)(j, i, i))		/* V3.02  TLi */
		    return true;
	    }
	}
    }
    else {/* hopefully ((p == bspawnb)
	    || (CondFlag[whrefl_king] && p == roib)) */  /* V3.04  NG */
	if (i >= bas + 24) {	       /* it can move from first rank */
	    finligne(i, -23, p1, j);
	    if (p1 == p) {
		if ((*evaluate)(j, i, i))		/* V3.02  TLi */
		    return true;
	    }
	    finligne(i, -25, p1, j);
	    if (p1 == p) {
		if ((*evaluate)(j, i, i))		/* V3.02  TLi */
			return true;
	    }
	}
    }
    return false;
}

boolean amazcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return  leapcheck(i, 9, 16, p, evaluate)		 /* V2.60  NG */
	 || ridcheck(i, 1, 8, p, evaluate);
}

boolean impcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return  leapcheck(i, 9, 16, p, evaluate)		/* V2.60  NG */
	 || ridcheck(i, 1, 4, p, evaluate);
}

boolean princcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return  leapcheck(i, 9, 16, p, evaluate)		/* V2.60  NG */
	 || ridcheck(i, 5, 8, p, evaluate);
}

boolean gnoucheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return  leapcheck(i, 9, 16, p, evaluate)		/* V2.60  NG */
	 || leapcheck(i, 33, 40, p, evaluate);
}

boolean antilcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 49, 56, p, evaluate);		/* V2.60  NG */
}

boolean ecurcheck(
    square	i,
    piece	p,
    boolean	(* evaluate)(square,square,square))	/* V2.60  NG */
{
    return  leapcheck(i, 9, 16, p, evaluate)		/* V2.60  NG */
	 || leapcheck(i, 61, 68, p, evaluate);
}

boolean warancheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return  ridcheck(i, 9, 16, p, evaluate)		/* V2.60  NG */
	 || ridcheck(i, 1, 4, p, evaluate);
}

boolean dragoncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    if (leapcheck(i, 9, 16, p, evaluate))		/* V2.60  NG */
	return true;

    if (    p == dragonn
	|| (CondFlag[blrefl_king] && p == roin))	/* V3.04  NG */
    {

	if (  i <= haut - 48				/* V3.04  NG */
	   || CondFlag[parrain])
	{
	    if (e[i + 23] == p) {
		if ((*evaluate)(i + 23, i, i))		/* V3.02  TLi */
		    return true;
	    }
	    if (e[i + 25] == p) {
		if ((*evaluate)(i + 25, i, i))		/* V3.02  TLi */
		    return true;
	    }
	}
    }
    else {/* hopefully ((p == dragonb)
	   || (CondFlag[whrefl_king] && p == roib)) */	/* V3.04  NG */

	if (  i >= bas + 48				/* V3.04  NG */
	   || CondFlag[parrain])
	{
	    if (e[i - 23] == p) {
		if ((*evaluate)(i - 23, i, i))		/* V3.02  TLi */
		    return true;
	    }
	    if (e[i - 25] == p) {
		if ((*evaluate)(i - 25, i, i))		/* V3.02  TLi */
		    return true;
	    }
	}
    }
    return false;
}

boolean kangoucheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k;
    piece   p1;
    square j, j1;

    for (k= 8; k > 0; k--) {
	j = i + vec[k];					/* V2.51  NG */
	if (abs(e[j]) >= roib) {
	    finligne(j, vec[k], p1, j1);
	    if (p1 != obs) {
		finligne(j1, vec[k], p1, j);
		if (p1 == p) {
		    if ((*evaluate)(j, i, i))		/* V3.02  TLi */
			    return true;
		}
	    }
	}
    }
    return false;
}

boolean rabbitcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.76  NG */
{
/* 2 hurdle lion */
    numvec  k;
    piece   p1;
    square j, j1;

    for (k= 8; k > 0; k--) {
	finligne(i, vec[k], p1, j);
	if (abs(p1) >= roib) {
	    finligne(j, vec[k], p1, j1);
	    if (p1 != obs) {
		finligne(j1, vec[k], p1, j);
		if (p1 == p) {
		    if ((*evaluate)(j, i, i))
			    return true;
		}
	    }
	}
    }
    return false;
}

boolean bobcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.76  NG */
{
/* 4 hurdle lion */
    numvec  k;
    piece   p1;
    square j, j1;

    for (k= 8; k > 0; k--) {
	finligne(i, vec[k], p1, j);
	if (abs(p1) >= roib) {
	    finligne(j, vec[k], p1, j1);
	    if (p1 != obs) {
		finligne(j1, vec[k], p1, j);
		if (p1 != obs) {
		    finligne(j, vec[k], p1, j1);
		    if (p1 != obs) {
			finligne(j1, vec[k], p1, j);
			if (p1 == p) {
		    	    if ((*evaluate)(j, i, i))
				return true;
			}
		    }
		}
	    }
	}
    }
    return false;
}

boolean ubicheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    square  *bnp;

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

boolean moosecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rmhopech(i, 8, 1, 0, p, evaluate);
}

boolean eaglecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rmhopech(i, 8, 1, 1, p, evaluate);
}

boolean sparrcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rmhopech(i, 8, 1, 2, p, evaluate);
}

boolean margueritecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.64  TLi */
{
    return  sparrcheck(i, p, evaluate)
	 || eaglecheck(i, p, evaluate)
	 || moosecheck(i, p, evaluate)
	 || scheck(i, p, evaluate);
} /* margueritecheck */

boolean leap36check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.64  TLi */
{
    return leapcheck(i, 129, 136, p, evaluate);
} /* leap36check */

boolean rookmoosecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  NG */
{
    return rmhopech(i, 8, 5, 0, p, evaluate);
}

boolean rookeaglecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  NG */
{
    return rmhopech(i, 4, 1, 1, p, evaluate);
}

boolean rooksparrcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  NG */
{
    return rmhopech(i, 8, 5, 2, p, evaluate);
}

boolean bishopmoosecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  NG */
{
    return rmhopech(i, 4, 1, 0, p, evaluate);
}

boolean bishopeaglecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  NG */
{
    return rmhopech(i, 8, 5, 1, p, evaluate);
}

boolean bishopsparrcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.62  NG */
{
    return rmhopech(i, 4, 1, 2, p, evaluate);
}

boolean archcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k;

    for (k= 5; k <= 8; k++) {
	if (rrfouech(i, i, vec[k], p, 1, evaluate)) {	/* V2.4c  NG */
	    return true;
	}
    }
    return false;
}

boolean reffoucheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k;

    for (k= 5; k <= 8; k++) {
	if (rrfouech(i, i, vec[k], p, 4, evaluate)) {	/* V2.4c  NG */
	    return true;
	}
    }
    return false;
}

boolean cardcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k;

    for (k= 5; k <= 8; k++) {
	if (rcardech(i, i, vec[k], p, 1, evaluate)) {	/* V2.4c  NG */
	    return true;
	}
    }
    return false;
}

boolean nsautcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rhopcheck(i, 9, 16, p, evaluate);
}

boolean camridcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return ridcheck(i, 33, 40, p, evaluate);
}

boolean zebridcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return ridcheck(i, 25, 32, p, evaluate);
}

boolean gnuridcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return  ridcheck(i, 9, 16, p, evaluate)
	 || ridcheck(i, 33, 40, p, evaluate);
}

boolean camhopcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rhopcheck(i, 33, 40, p, evaluate);
}

boolean zebhopcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rhopcheck(i, 25, 32, p, evaluate);
}

boolean gnuhopcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return  rhopcheck(i, 9, 16, p, evaluate)
	 || rhopcheck(i, 33, 40, p, evaluate);
}

boolean dcscheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k;

    for (k= 9; k <= 14; k++) {
	if (rcsech(i, vec[k], vec[23 - k], p, evaluate)) {
	    return true;
	}
    }
    for (k= 15; k <= 16; k++) {
	if (rcsech(i, vec[k], vec[27 - k], p, evaluate)) {
	    return true;
	    }
    }
    return false;
}

boolean refccheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return rrefcech(i, i, 2, p, evaluate);
}

boolean equicheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k;
    piece   p1;
    square  j, j1;

    for (k= 8; k > 0; k--) {	    /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
	finligne(i, vec[k], p1, j);
	if (p1 != obs) {
	    finligne(j, vec[k], p1, j1);
	    if (p1 == p && j1 - j == j - i) {
		if ((*evaluate)(j1, i, i)		/* V3.02  TLi */
		  && hopimcheck(j1,i,j,-vec[k]))	/* V3.12  TM */
		{
			return true;
		}
	    }
	}
    }
    for (k= 17; k <= 40; k++) {      /* 2,4; 2,6; 4,6; */
	if ( abs(e[i + vec[k]]) >= roib
	  && e[j1= i + (vec[k]<<1)] == p)
	{
	    if ((*evaluate)(j1, i, i)			/* V3.02  TLi */
		&& hopimcheck(j1,i,j1+vec[k],vec[k]))	/* V3.12  TM */
	    {
		return true;
	    }
	}
    }
    return false;
}

boolean catcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    numvec  k;
    square  j, j1;

    if (leapcheck(i, 9, 16, p, evaluate)) {
	return true;
    }
    for (k= 61; k <= 64; k++) {
	j= i + vec[k];
	while (e[j] == vide) {
	    if (e[j1= j + mixhopdata[3][k - 60]] == p) {
		if ((*evaluate)(j1, i, i))		/* V3.02  TLi */
		    return true;
	    }
	    if (e[j1= j + mixhopdata[3][k - 56]] == p) {
		if ((*evaluate)(j1, i, i))		/* V3.02  TLi */
		    return true;
	    }
	    j+= vec[k];
	}
    }
    return false;
}

boolean roicheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 1, 8, p, evaluate);
}

boolean cavcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return leapcheck(i, 9, 16, p, evaluate);
}

boolean damecheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return ridcheck(i, 1, 8, p, evaluate);
}

boolean tourcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return ridcheck(i, 1, 4, p, evaluate);
}

boolean foucheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    return ridcheck(i, 5, 8, p, evaluate);
}

boolean pioncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.60  NG */
{
    if (anymars) {				      /* SE/TLi  3.46 */
	boolean anymarscheck=				/* V3.47  NG */
		((p==e[rb]) && ((e[i-23]==p) || (e[i-25]==p)))
	     || ((p==e[rn]) && ((e[i+23]==p) || (e[i+25]==p)));
	if (!is_phantomchess || anymarscheck) {		/* V3.47  NG */
	    return anymarscheck;
	}
    }

    if (p <= roin) {					/* V3.2  TLi */
	if ( i <= haut - 48				/* V3.02  TLi */
	    || CondFlag[parrain]
	    || CondFlag[einstein]			/* V3.2  TLi */
	    || p == orphann)				/* V3.2  TLi */
	{
	    if (e[i + 23] == p) {
		if ((*evaluate)(i + 23, i, i)) {	/* V3.02  TLi */
		    return true;
		}
	    }
	    if (e[i + 25] == p) {
		if ((*evaluate)(i + 25, i, i)) {	/* V3.02  TLi */
		    return true;
		}
	    }
	}
    }
    else {	  /* hopefully (p >= roib) */		/* V3.21  NG */
	if ( i >= bas + 48				/* V3.02  TLi */
	  || CondFlag[parrain]
	  || CondFlag[einstein]				/* V3.2   TLi */
	  || p == orphanb)				/* V3.2   TLi */
	{
	    if (e[i - 23] == p) {
		if ((*evaluate)(i - 23, i, i)) {	/* V3.02  TLi */
		    return true;
		}
	    }
	    if (e[i - 25] == p) {
		if ((*evaluate)(i - 25, i, i)) {	/* V3.02  TLi */
		    return true;
		}
	    }
	}
    }
    return false;
}

boolean ep_not_libre(
    piece	p,
    square	sq,
    boolean	generating,   /* V3.44	TLi */
    boolean	(*checkfunc)(square,piece,
			     boolean (*)(square,square,square)))
{
    /* Returns true if a pawn who has just crossed the square sq is
       paralysed by a piece p due to the ugly Madrasi-ep-rule by a
       pawn p.
       ---------------------------------------------------------
       Dear inventors of fairys:
	  Make it as sophisticated and inconsistent as possible!
       ---------------------------------------------------------

       Checkfunc must be the corresponding checking function.

       pawn just moved	    p	    checkfunc
       --------------------------------------
       white pawn	    pn	    pioncheck
       black pawn	    pb	    pioncheck
       white berolina pawn  pbn     pbcheck
       black berolina pawn  pbb     pbcheck
    */

    ply ply_dblstp= generating ? nbply-1 : nbply;

      return ((ep[ply_dblstp] == sq) || (ep2[ply_dblstp] == sq))
	  && nbpiece[p]
	  && (*checkfunc)(sq, p, flaglegalsquare
				 ? legalsquare
				 : eval_ortho);
} /* end eplibre */

boolean libre(square sq, boolean generating) {
    piece   p= e[sq];		/* V3.51  NG */
    boolean flag= true, neutcoul_sic= neutcoul;

    if (TSTFLAG(PieSpExFlags, Neutral)) {		/* V3.60  TLi */
	if (generating)
	    p= -p;
	else
	    initneutre(advers(neutcoul));
    }

    if (CondFlag[madras] || CondFlag[isardam]) {	/* V3.60  TLi */
	if (! rex_mad) {
	    if ((sq == rb) || (sq == rn)) {
		return true;
	    }
	}

	/* The ep capture needs special handling. */
	switch (p) {					/* V3.22  TLi */

	  case pb: /* white pawn */
	    if (ep_not_libre(pn, sq - 24, generating, pioncheck)) {
		flag= False;
	    }
	    break;

	  case pn: /* black pawn */
	    if (ep_not_libre(pb, sq + 24, generating, pioncheck)) {
		flag= False;
	    }
	    break;

	  case pbb: /* white berolina pawn */
	    if ( ep_not_libre(pbn, sq - 23, generating, pbcheck)
	      || ep_not_libre(pbn, sq - 25, generating, pbcheck))
	    {
		flag= False;
	    }
	    break;

	  case pbn: /* black berolina pawn */
	    if (ep_not_libre(pbb, sq + 23, generating, pbcheck)
	     || ep_not_libre(pbb, sq + 25, generating, pbcheck))
	    {
		flag= False;
	    }
	    /* NB: Super (Berolina) pawns cannot neither be captured
		   ep nor capture ep themselves.
	     */
	    break;
	}

	flag = flag
	       && (!nbpiece[-p]
		   || !(*checkfunctions[abs(p)])(sq, -p,
			  flaglegalsquare ? legalsquare : eval_ortho));
    } /* if (CondFlag[madrasi] ... */

    if (CondFlag[eiffel]) {				/* V3.60  TLi */
	boolean test= true;
	piece ep;

	switch (p) {					/* V3.22  TLi */
	  case pb: ep= dn; break;
	  case db: ep= tn; break;
	  case tb: ep= fn; break;
	  case fb: ep= cn; break;
	  case cb: ep= pn; break;
	  case pn: ep= db; break;
	  case dn: ep= tb; break;
	  case tn: ep= fb; break;
	  case fn: ep= cb; break;
	  case cn: ep= pb; break;
	  default:
	    test= false;
	    ep= 0;	 /* avoid compiler warning. ElB, 2001-12-16. */
	    break;
	}

	if (test) {
	    flag = flag
		   && (!nbpiece[ep]
		       || !(*checkfunctions[abs(ep)])(sq, ep,
			  flaglegalsquare ? legalsquare : eval_ortho));
	}
    } /* CondFlag[eiffel] */

    if (TSTFLAG(PieSpExFlags, Neutral) && !generating)	/* V3.60  TLi */
	initneutre(neutcoul_sic);

    return flag;
} /* libre */

boolean soutenu(square id, square ia, square ip) {

    /* id - case dep., ia - case arr., ip - case pri.	   V3.02  TLi */
    piece	p= 0;	    /* avoid compiler warning ElB, 2001-12-16 */
    boolean	Result;
    boolean	(*evaluate)(square, square, square);

    if (CondFlag[central]) {				 /* V3.50 SE */
	if ( id == rb || id == rn) {
	    return true;
	}
	nbpiece[p= e[id]]--;
	e[id]= (p > vide) ? dummyb : dummyn;
	evaluate= soutenu;
    }
    else if (flaglegalsquare) {
	if (!legalsquare(id, ia, ip)) {
	    return false;
	}
	evaluate= legalsquare;
    }
    else if (flag_madrasi) {			/* V3.32, V3.60  TLi */
	if (!eval_madrasi(id, ia, ip)) {
	    return false;
	}
	evaluate= eval_madrasi;
    }
    else if (TSTFLAG(PieSpExFlags,Paralyse)) {		/* V3.32  TLi */
	if (!paraechecc(id, ia, ip)) {
	    return false;
	}
	evaluate= paraechecc;
    }
    else {
	evaluate= eval_ortho;
    }

    if ((color(id) == blanc)			 /* V3.32, V3.53  TLi */
      ^ (CondFlag[beamten] || TSTFLAG(PieSpExFlags, Beamtet)))
    {
	if (  TSTFLAG(PieSpExFlags, Beamtet)
	  && !TSTFLAG(spec[id], Beamtet))		/* V3.53  TLi */
	{
	    Result= True;
	}
	else {
	    ia= rn;
	    rn= id;
	    Result= rnechec(evaluate);
	    rn= ia;
	}
    }
    else {
	if ( TSTFLAG(PieSpExFlags, Beamtet)
	  && !TSTFLAG(spec[id], Beamtet))		/* V3.53  TLi */
	{
	    Result= True;
	}
	else {
	    ia= rb;
	    rb= id;
	    Result= rbechec(evaluate);
	    rb= ia;
	}
    }

    if (CondFlag[central])				 /* V3.50 SE */
	nbpiece[e[id]= p]++;

    return(Result);
} /* soutenu */

boolean eval_madrasi(square id, square ia, square ip) {
    /* id - case dep., ia - case arr., ip - case pri.	V3.02  TLi */

    if (flaglegalsquare && !legalsquare(id, ia, ip)) {	/* V3.02  TLi */
	return false;
    }
    else {
	return libre(id, false);
    }
} /* eval_madrasi */

boolean eval_shielded(square id, square ia, square ip) {  /* V3.62 SE */
    /* id - case dep., ia - case arr., ip - case pri.	V3.02  TLi */
    if ((id == rn && ip == rb) || (id == rb && ip == rn)) {
	return !soutenu(ip, id, id);  /* won't work for locust Ks etc.*/
    }
    else {
	return true;
    }
} /* eval_madrasi */

boolean edgehcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    /* detect "check" of edgehog p */
    piece p1;
    square j1;
    numvec  k;

    for (k= 8; k > 0; k--) {				/* V3.00  NG */
	finligne(i, vec[k], p1, j1);
	if ( p1 == p					/* V3.22  TLi */
	  && NoEdge(j1) != NoEdge(i)
	  && (*evaluate)(j1, i, i))			/* V3.02  TLi */
	{
	    return true;
	}
    }
    return false;
}

/***************  V3.1	TLi  ***************/

boolean maooaridercheck(
    square	i,
    piece	p,
    numvec	fir,
    numvec	sec,
    boolean	(*evaluate)(square,square,square))
{
    square  j1, j2;

    j1 = i + fir;
    j2 = i + sec;
    while ((e[j1] == vide) && (e[j2] == vide)) {
	j1 += sec;
	j2 += sec;
    }
    if ((e[j1] == vide) && (e[j2] == p) && (*evaluate)(j2, i, i)) {
	return true;
    }
    else {
	return false;
    }
} /* end of maooaridercheck */

boolean moaridercheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    if (maooaridercheck(i, p,  24,  47, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,  24,  49, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p, -24, -47, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p, -24, -49, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,	1,  26, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,	1, -22, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,  -1, -26, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,  -1,  22, evaluate)) {
	return true;
    }
    return false;
}

boolean maoridercheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
    if (maooaridercheck(i, p,  25,  49, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,  25,  26, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p, -23, -22, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p, -23, -47, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p, -25, -49, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p, -25, -26, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,  23,  22, evaluate)) {
	return true;
    }
    if (maooaridercheck(i, p,  23,  47, evaluate)) {
	return true;
    }
    return false;
}

boolean maooariderlioncheck(
    square	i,
    piece	p,
    numvec	fir,
    numvec	sec,
    boolean	(*evaluate)(square,square,square))	/* V3.64  TLi */
{
    square	j1= i+fir;
    square	j2= i+sec;

    while ((e[j1] == vide) && (e[j2] == vide)) {
	j1 += sec;
	j2 += sec;
    }
    if (e[j1] != vide && e[j2] == p && (*evaluate)(j2, i, i)) {
	return true;
    }

    if ( (e[j1] != obs && e[j2] != obs)
      && (e[j1] == vide || e[j2] == vide))
    {
	j1 += sec;
	j2 += sec;
	while ((e[j1] == vide) && (e[j2] == vide)) {
	    j1 += sec;
	    j2 += sec;
	}
	if ((e[j1] == vide) && (e[j2] == p) && (*evaluate)(j2, i, i)) {
	    return true;
	}
    }
    return false;
} /* maooariderlioncheck */

boolean maoriderlioncheck(
    square	i,
    piece	p,
    boolean	(* evaluate)(square,square, square))
{
    if (maooariderlioncheck(i, p,  25,	49, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,  25,	26, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p, -23, -22, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p, -23, -47, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p, -25, -49, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p, -25, -26, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,  23,	22, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,  23,	47, evaluate)) {
	return true;
    }
    return false;
} /* maoriderlioncheck */

boolean moariderlioncheck(
    square	i,
    piece	p,
    boolean	(* evaluate)(square,square,square))	/* V3.65  TLi */
{
    if (maooariderlioncheck(i, p,  24,	47, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,  24,	49, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p, -24, -47, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p, -24, -49, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,   1,	26, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,   1, -22, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,  -1, -26, evaluate)) {
	return true;
    }
    if (maooariderlioncheck(i, p,  -1,	22, evaluate)) {
	return true;
    }
    return false;
} /* moariderlioncheck */

boolean r_hopcheck(
    square	i,
    piece	p,
    boolean	(* evaluate)(square,square,square))
{
    return rhopcheck(i, 1, 4, p, evaluate);
}

boolean b_hopcheck(
    square	i,
    piece	p,
    boolean	(* evaluate)(square,square,square))
{
    return rhopcheck(i, 5, 8, p, evaluate);
}

/***************  V3.1	TLi  ***************/

boolean pos_legal(void) {			     /* V3.44  SE/TLi */
    /* could be used for other genres e.g. Ohneschach */

    if (CondFlag[isardam]) {
	square z;
	int i,j;

	initneutre(trait[nbply]);			 /* V3.50 SE */
						/* for e.p. captures */
	z=haut;
	for (i=8; i>0; i--, z-=16)
	for (j=8; j>0; j--, z--) {
	    if (e[z]!=vide) {
		if (!libre(z, false)) {
		    return false;
		}
	    }
	}
    }

    /* To avoid messing up the ???[nbply] arrays during output of
       the solution */
    if (flag_writinglinesolution) {		       /* V3.55  TLi */
	return true;
    }

    if (CondFlag[ohneschach]) {
	couleur camp= trait[nbply];
	couleur ad= advers(camp);

	if (nbply > maxply-1)  {
	    FtlMsg(ChecklessUndecidable);
	}

	if (echecc(camp)) {
	    return false;
	}

	if (echecc(ad) && !patt(ad)) {
	    return false;
	}
    }

    if (CondFlag[exclusive]) {				/* V3.45  TLi */
	if (nbply > maxply-1) {
	    FtlMsg(ChecklessUndecidable);
	}

	if (!mateallowed[nbply] && (*stipulation)(trait[nbply])) {
	    return false;
	}
    }

    return true;
} /* pos_legal */

boolean eval_isardam(square id, square ia, square ip) {
						     /* V3.44  SE/TLi */
    boolean flag=false;
    couleur camp;

    VARIABLE_INIT(camp);

    /* the following does not suffice if we have neutral kings,
       but we have no chance to recover the information who is to
       move from id, ia and ip.
								TLi
     */
    if (ip == rn) {
	camp=blanc;
    }
    else if (ip == rb) {
	camp=noir;
    }

    if (flag_nk) {	    /* V3.50 SE will this do for neutral Ks? */
	camp= neutcoul;
    }

    nextply();
    trait[nbply]= camp;

    initkiller();
    k_cap=true;		  /* set to allow K capture in e.g. AntiCirce */
    empile(id,ia,ip);	  /* generate only the K capture */
    k_cap=false;
    finkiller();

    while (encore() && !flag) {
	/* may be several K capture moves e.g. PxK=S,B,R,Q */
	if (CondFlag[brunner]) {			 /* V3.50 SE */
	    /* For neutral Ks will need to return true always */
	    flag= jouecoup()
		&& (camp==blanc ? !echecc(blanc) : !echecc(noir));
	}
	else if (CondFlag[isardam]) {
		flag= jouecoup();
	}
	/* Isardam + Brunner may be possible! in which case this logic
	   is correct
	 */
	repcoup();
    }

    finply();
    return flag;
} /* eval_isardam */


boolean orixcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.44  NG */
{
    numvec  k;
    piece   p1;
    square  j, j1;

    for (k= 8; k > 0; k--) {	    /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
	finligne(i, vec[k], p1, j);
	if (p1 != obs) {
	    finligne(j, vec[k], p1, j1);
	    if (p1 == p && j1 - j == j - i) {
		if ((*evaluate)(j1, i, i)
		    && hopimcheck(j1,i,j,-vec[k]))
		{
		    return true;
		}
	    }
	}
    }
    return false;
}

boolean leap15check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.46  NG */
{
    return leapcheck(i, 113, 120, p, evaluate);
}

boolean leap25check(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.46  NG */
{
    return leapcheck(i, 121, 128, p, evaluate);
}

boolean gralcheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.46  NG */
{
    if (leapcheck(i, 65, 68, p, evaluate)) {		/* Alfil */
	return true;
    }
    return rhopcheck(i, 1, 4, p, evaluate);		/* RookHopper */
}

/*** woozles + heffalumps ***/

square	sq_woo_from;
square	sq_woo_to;
couleur col_woo;

boolean aux_whx(square id, square ia, square ip) {	/* V3.55 TLi */
    /* id - case dep., ia - case arr., ip - case pri. */

    if (id != sq_woo_from)
	return false;

    /* id == sq_woo_from */
    if (CondFlag[heffalumps]) {
	smallint cd1= id%24 - ia%24;
	smallint rd1= id/24 - ia/24;
	smallint cd2= sq_woo_to%24 - id%24;
	smallint rd2= sq_woo_to/24 - id/24;
	smallint t= 7;

	if (cd1 != 0)
	    t= abs(cd1);
	if (rd1 != 0 && t > abs(rd1))
	    t= abs(rd1);

	while (!(cd1%t == 0 && rd1%t == 0))
	    t--;
	cd1= cd1/t;
	rd1= rd1/t;

	t= 7;
	if (cd2 != 0)
	    t= abs(cd2);
	if (rd2 != 0 && t > abs(rd2))
	    t= abs(rd2);

	while (!(cd2%t == 0 && rd2%t == 0))
	    t--;

	cd2= cd2/t;
	rd2= rd2/t;

	if (!(	(cd1 == cd2 && rd1 == rd2)
	     || (cd1 == -cd2 && rd1 == -rd2)))
	{
	    return false;
	}
    }

    if (flaglegalsquare) {
	return legalsquare(id, ia, ip);
    }
    else {
	return eval_ortho(id, ia, ip);
    }
} /* aux_whx */

boolean aux_wh(square id, square ia, square ip) {	/* V3.55  TLi */
    /* id - case dep., ia - case arr., ip - case pri. */

    piece p;
    if (flaglegalsquare) {
	if (!legalsquare(id, ia, ip)) {
	    return false;
	}
    }
    else {
	if (!eval_ortho(id, ia, ip)) {
	    return false;
	}
    }
    p= e[sq_woo_from];
    if (nbpiece[p]
      && (*checkfunctions[abs(p)])(id, e[sq_woo_from], aux_whx))
    {
	return true;
    }

    return false;
} /* aux_wh */

boolean woohefflibre(square to, square from) {		/* V3.55  TLi */

    piece   *pcheck, p;

    if (rex_wooz_ex && (from == rb || from == rn)) {
	return true;
    }

    sq_woo_from= from;
    sq_woo_to= to;
    col_woo= e[from] > vide ? blanc : noir;

    pcheck = transmpieces;
    if (rex_wooz_ex)
	pcheck++;

    while (*pcheck) {
	if (CondFlag[biwoozles] ^ (col_woo == noir)) {
	    p= -*pcheck;
	}
	else {
	    p= *pcheck;
	}
	if (nbpiece[p] && (*checkfunctions[*pcheck])(from, p, aux_wh)) {
	    return false;
	}
	pcheck++;
    }

    return true;
} /* woohefflibre */

boolean eval_wooheff(square id, square ia, square ip) {
    /* id - case dep., ia - case arr., ip - case pri.   V3.02  TLi */
    if (flaglegalsquare && !legalsquare(id, ia, ip)) {	/* V3.02  TLi */
	return false;
    }
    else {
	return woohefflibre(ia, id);
    }
} /* eval_wooheff */


boolean scorpioncheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V2.63  NG */
{
    return  leapcheck(i, 1, 8, p, evaluate)
	 || rhopcheck(i, 1, 8, p, evaluate);
}

boolean dolphincheck(
    square	i,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.70  TLi */
{
    return  rhopcheck(i, 1, 8, p, evaluate)
	 || kangoucheck(i, p, evaluate);
} /* dolphincheck */
