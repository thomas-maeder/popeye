/********************* MODIFICATIONS to pyproof.c ***********************
 **
 ** Date       Who  What
 **
 ** 2006/05/17 SE   Changes to allow half-move specification for helpmates using 0.5 notation
 **                 Change for take&make
 **
 ** 2007/05/14 SE   Change for annan
 **
 ** 2008/01/01 SE   Bug fix: Circe Assassin + proof game (reported P.Raican)
 **
 ** 2008/01/01 SE   Bug fix: Circe Parrain + proof game (reported P.Raican)
 **
 **************************** End of List ******************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __TURBOC__
# include <mem.h>
#endif
#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pyhash.h"
#include "DHT/dhtbcmem.h"
#include "pyproof.h"
#include "pymsg.h"

/* an array to store the position */
static piece ProofPieces[32];
static square ProofSquares[32];
static smallint ProofNbrAllPieces;
echiquier ProofBoard, PosA;
static	byte buffer[256];
square Proof_rb, Proof_rn, rbA, rnA;
Flags ProofSpec[64], SpecA[64];
static imarr Proof_isquare;
imarr isquareA;

static smallint xxxxx[fb+fb+1];
#define ProofNbrPiece (xxxxx+fb)

static smallint ProofNbrWhitePieces, ProofNbrBlackPieces;

static boolean BlockedBishopc1, BlockedBishopf1, BlockedQueend1,
  BlockedBishopc8, BlockedBishopf8, BlockedQueend8,
  CapturedBishopc1, CapturedBishopf1, CapturedQueend1,
  CapturedBishopc8, CapturedBishopf8, CapturedQueend8;

boolean ProofVerifie(void) {
  if (flagfee || PieSpExFlags&(~(BIT(White)+BIT(Black)))) {
	return VerifieMsg(ProofAndFairyPieces);
  }

  ProofFairy= change_moving_piece
    || CondFlag[black_oscillatingKs]
    || CondFlag[white_oscillatingKs]
    || CondFlag[republican]
    || anycirce
    || CondFlag[sentinelles]
    || anyanticirce
    || CondFlag[singlebox]
    || CondFlag[blroyalsq]
    || CondFlag[whroyalsq]
    || TSTFLAG(PieSpExFlags, ColourChange)
    || CondFlag[actrevolving]
    || CondFlag[arc]
    || CondFlag[annan]
    || CondFlag[glasgow]
    || CondFlag[takemake]
    || flagAssassin
    || CondFlag[messigny]
    || CondFlag[mars];

  return true;
} /* ProofVerifie */

BCMemValue *ProofEncode(void) {
  byte	*bp, *position, pieces;
  int		row, col;
  square	bnp;
  BCMemValue	*bcm;
  boolean even= False;

  VARIABLE_INIT(pieces);

  bcm= (BCMemValue *)buffer;
  position= bp= bcm->Data;
  /* clear the bits for storing the position of pieces */
  memset(position, 0, 8);
  bp= position+8;

  bnp= bas;
  for (row=0; row<8; row++, bnp+= 16) {
	for (col=0; col<8; col++, bnp++) {
      piece   p;
      if ((p= e[bnp]) != vide) {
		if (!even) {
          pieces= (byte)(p < vide ? 7-p : p);
		}
		else {
          *bp++ = pieces+(((byte)(p < vide ? 7-p : p))<<4);
		}
		even= !even;
		position[row] |= BIT(col);
      }
	}
  }
  if (even)
	*bp++ = pieces+(15<<4);
  *bp++ = castling_flag[nbply];

  if (CondFlag[duellist]) {
    *bp++ = (byte)(whduell[nbply] - bas);
    *bp++ = (byte)(blduell[nbply] - bas);
  }
  if (CondFlag[blfollow] || CondFlag[whfollow]) {
    *bp++ = (byte)(move_generation_stack[nbcou].departure - bas);
  }

  if (ep[nbply]) {
	*bp++ = (byte)(ep[nbply] - bas);
  }

  bcm->Leng= bp - bcm->Data;
  return bcm;
}

smallint proofwkm[haut+25-(bas-25)+1];
smallint proofbkm[haut+25-(bas-25)+1];

#if defined(BC31_SUCKS)
smallint *WhKingMoves=proofwkm-(bas-25);
smallint *BlKingMoves=proofbkm-(bas-25);
#else
#define WhKingMoves  (proofwkm-(bas-25))
#define BlKingMoves  (proofbkm-(bas-25))
#endif /* BC31_SUCKS */
/* above changed due to BorlandC V3.1 problems	*/

void ProofInitialiseKingMoves(square ProofRB, square ProofRN) {
  square	*bnp, sq;
  numvec	k;
  smallint	MoveNbr;
  boolean	GoOn;

  /* set all squares to a maximum */
  for (bnp= boardnum; *bnp; bnp++)
	WhKingMoves[*bnp]= BlKingMoves[*bnp]= enonce;

  /* mark squares occupied or garded by immobile pawns
     white pawns
  */
  for (sq= square_a2; sq <= square_h2; sq++) {
	if (ProofBoard[sq] == pb) {
      WhKingMoves[sq]=
	    BlKingMoves[sq]= -1;	/* blocked */
      if (eval_white == eval_ortho) {
        BlKingMoves[sq+dir_up+dir_left]=
          BlKingMoves[sq+dir_up+dir_right]= -2;	/* guarded */
      }
	}
  }

  /* black pawns */
  for (sq= square_a7; sq <= square_h7; sq++) {
	if (ProofBoard[sq] == pn) {
      BlKingMoves[sq]=
	    WhKingMoves[sq]= -1;	/* blocked */
      if (eval_black == eval_ortho) {
        WhKingMoves[sq+dir_down+dir_right]=
          WhKingMoves[sq+dir_down+dir_left]= -2;	/* guarded */
      }
	}
  }

  /* cornered bishops */
  if (BlockedBishopc1)
	WhKingMoves[square_c1]= BlKingMoves[square_c1]= -1;	/* blocked */
  if (BlockedBishopf1)
	WhKingMoves[square_f1]= BlKingMoves[square_f1]= -1;	/* blocked */
  if (BlockedBishopc8)
	WhKingMoves[square_c8]= BlKingMoves[square_c8]= -1;	/* blocked */
  if (BlockedBishopf8)
	WhKingMoves[square_f8]= BlKingMoves[square_f8]= -1;	/* blocked */

  /* initialise wh king */
  WhKingMoves[ProofRB]=
    MoveNbr= 0;
  do {
	GoOn= False;
	for (bnp= boardnum; *bnp; bnp++) {
      if (WhKingMoves[*bnp] == MoveNbr) {
		for (k= 8; k; k--) {
          if (WhKingMoves[*bnp+vec[k]] > MoveNbr) {
			WhKingMoves[*bnp+vec[k]]= MoveNbr+1;
			GoOn= True;
          }
          if (CondFlag[whtrans_king]
              || CondFlag[trans_king]) {
            sq= *bnp;
            while (e[sq+=vec[k]] != obs &&
                   WhKingMoves[sq] != -1) {
              if (WhKingMoves[sq] > MoveNbr) {
                WhKingMoves[sq]= MoveNbr+1;
                GoOn= True;
              }
            }
          } /* trans_king */
		}
        if (CondFlag[whtrans_king]
            || CondFlag[trans_king]) {
          /* Knight moves */
          for (k= 16; k>8; k--) {
            sq= *bnp + vec[k];
            if (e[sq] != obs &&
                WhKingMoves[sq] > MoveNbr) {
              WhKingMoves[sq]= MoveNbr+1;
              GoOn= True;
            }
          }
        } /* trans_king */
      }
	}
	MoveNbr++;
  } while(GoOn);

  /* initialise bl king */
  BlKingMoves[ProofRN]=
    MoveNbr= 0;
  do {
	GoOn= False;
	for (bnp= boardnum; *bnp; bnp++) {
      if (BlKingMoves[*bnp] == MoveNbr) {
		for (k= 8; k; k--) {
          if (BlKingMoves[*bnp+vec[k]] > MoveNbr) {
			BlKingMoves[*bnp+vec[k]]= MoveNbr+1;
			GoOn= True;
          }
          if (CondFlag[bltrans_king]
              || CondFlag[trans_king]) {
            sq= *bnp;
            while (e[sq+=vec[k]] != obs &&
                   BlKingMoves[sq] != -1) {
              if (BlKingMoves[sq] > MoveNbr) {
                BlKingMoves[sq]= MoveNbr+1;
                GoOn= True;
              }
            }
          } /* trans_king */
		}
        if (CondFlag[bltrans_king]
            || CondFlag[trans_king]) {
          /* Knight moves */
          for (k= 16; k>8; k--) {
            sq= *bnp + vec[k];
            if (e[sq] != obs &&
                BlKingMoves[sq] > MoveNbr) {
              BlKingMoves[sq]= MoveNbr+1;
              GoOn= True;
            }
          }
        } /* trans_king */
      }
	}
	MoveNbr++;
  } while(GoOn);
} /* ProofInitialiseKingMoves */

/* a function to store the position and set the PAS */
void ProofInitialise(void) {
  int		i;
  piece	p;

  Proof_rb= rb;
  Proof_rn= rn;

  ProofNbrAllPieces=
    ProofNbrWhitePieces=
    ProofNbrBlackPieces= 0;

  for (i= roib; i <= fb; i++) {
	ProofNbrPiece[i]= nbpiece[i];
	ProofNbrWhitePieces+= ProofNbrPiece[i];
	ProofNbrPiece[-i]= nbpiece[-i];
	ProofNbrBlackPieces+= ProofNbrPiece[-i];
  }

  for (i= maxsquare - 1; i >= 0; i--) {
	ProofBoard[i]= e[i];
  }

  for (i = 0; i < 64; i++) {
	ProofSpec[i]=spec[boardnum[i]];
    /* in case continued twinning
     * to other than proof game
     */
	p= e[boardnum[i]];
	if (p != vide) {
      ProofPieces[ProofNbrAllPieces]= p;
      ProofSquares[ProofNbrAllPieces++]= boardnum[i];
	}
	CLEARFL(spec[boardnum[i]]);
	p= e[boardnum[i]]= flag_atob ? PosA[boardnum[i]] : PAS[i];

	/* We must set spec[] for the PAS.
	   This is used in jouecoup for andernachchess!*/
	if (p >= roib) {
      SETFLAG(spec[boardnum[i]], White);
	}
	else if (p <= roin) {
      SETFLAG(spec[boardnum[i]], Black);
	}
	if (flag_atob)	{
      spec[boardnum[i]]= SpecA[i];
	}
  }

  if (CondFlag[imitators]) {
	for (i= 0; i < maxinum; i++) {
      Proof_isquare[i]= isquare[i];
	}
  }

  /* set the king squares */
  rb= square_e1;
  rn= square_e8;
  if (flag_atob) {
	rb= rbA;
	rn= rnA;
	if (CondFlag[imitators]) {
      for (i= 0; i < maxinum; i++) {
		isquare[i]= isquareA[i];
      }
	}
  }

  if (flag_atob) {
    char InitialLine[40];
    sprintf(InitialLine,
            "Initial (%s ->):\n",
            PieSpString[ActLang][flag_appseul ? White : Black]);
	StdString(InitialLine);
	WritePosition();
  }

  /* update nbpiece */
  verifieposition();

  /* store the PAS to be set when linesolution is called */
  StorePosition();

  if (ProofFairy) {
	return;
  }

  /* determine pieces blocked */

  BlockedBishopc1= ProofBoard[square_c1] == fb
    && ProofBoard[square_b2] == pb
    && ProofBoard[square_d2] == pb;

  BlockedBishopf1= ProofBoard[square_f1] == fb
    && ProofBoard[square_e2] == pb
    && ProofBoard[square_g2] == pb;

  BlockedBishopc8= ProofBoard[square_c8] == fn
    && ProofBoard[square_b7] == pn
    && ProofBoard[square_d7] == pn;

  BlockedBishopf8= ProofBoard[square_f8] == fn
    && ProofBoard[square_e7] == pn
    && ProofBoard[square_g7] == pn;

  BlockedQueend1= BlockedBishopc1
    && BlockedBishopf1
    && ProofBoard[square_d1] == db
    && ProofBoard[square_c2] == pb
    && ProofBoard[square_f2] == pb;

  BlockedQueend8= BlockedBishopc8
    && BlockedBishopf8
    && ProofBoard[square_d8] == dn
    && ProofBoard[square_c7] == pn
    && ProofBoard[square_f7] == pn;

  /* determine pieces captured */
  CapturedBishopc1= ProofBoard[square_c1] != fb
    && ProofBoard[square_b2] == pb
    && ProofBoard[square_d2] == pb;

  CapturedBishopf1= ProofBoard[square_f1] != fb
    && ProofBoard[square_e2] == pb
    && ProofBoard[square_g2] == pb;

  CapturedBishopc8= ProofBoard[square_c8] != fn
    && ProofBoard[square_b7] == pn
    && ProofBoard[square_d7] == pn;

  CapturedBishopf8= ProofBoard[square_f8] != fn
    && ProofBoard[square_e7] == pn
    && ProofBoard[square_g7] == pn;

  CapturedQueend1= BlockedBishopc1
    && BlockedBishopf1
    && ProofBoard[square_d1] != db
    && ProofBoard[square_c2] == pb
    && ProofBoard[square_f2] == pb;

  CapturedQueend8= BlockedBishopc8
    && BlockedBishopf8
    && ProofBoard[square_d8] != dn
    && ProofBoard[square_c7] == pn
    && ProofBoard[square_f7] == pn;

  /* update castling possibilities */
  if (BlockedBishopc1) {
    /* wh long castling impossible */
    CLRFLAGMASK(castling_flag[0],ra1_cancastle);
  }
  if (BlockedBishopf1) {
    /* wh short castling impossible */
    CLRFLAGMASK(castling_flag[0],rh1_cancastle);
  }
  if (BlockedBishopc8) {
    /* bl long castling impossible */
    CLRFLAGMASK(castling_flag[0],ra8_cancastle);
  }
  if (BlockedBishopf8) {
    /* bl short castling impossible */
    CLRFLAGMASK(castling_flag[0],rh8_cancastle);
  }
  if (!TSTFLAGMASK(castling_flag[0],ra1_cancastle|rh1_cancastle)) {
    /* no wh rook can castle, so the wh king cannot either */
    CLRFLAGMASK(castling_flag[0],ke1_cancastle);
  }
  if (!TSTFLAGMASK(castling_flag[0],ra8_cancastle|rh8_cancastle)) {
    /* no bl rook can castle, so the bl king cannot either */
    CLRFLAGMASK(castling_flag[0],ke8_cancastle);
  }

  castling_flag[2]= castling_flag[1]= castling_flag[0];

  /* initialise king diff_move arrays */
  ProofInitialiseKingMoves(Proof_rb, Proof_rn);

} /* ProofInitialise */

/* function that compares the current position with the desired one
 * and returns True if they are identical. Otherwise it returns False.
 */
boolean ProofIdentical(void) {
  int i;

  for (i = 0; i < ProofNbrAllPieces; i++) {
    if (ProofPieces[i] != e[ProofSquares[i]]) {
      return False;
	}
  }

  for (i = roib; i <= fb; i++) {
	if (   ProofNbrPiece[i] != nbpiece[i]
           || ProofNbrPiece[-i] != nbpiece[-i])
	{
      return False;
	}
  }

  if (CondFlag[imitators]) {
	for (i= 0; i < inum[nbply]; i++) {
      if (Proof_isquare[i] != isquare[i]) {
		return False;
      }
	}
  }

  return True;
}

short ProofKnightMoves[haut-bas+1]= {
  /*   1-  7 */		0,  3,	2,  3,	2,  3,	4,  5,
  /* dummies  8- 16 */ -1, -1, -1, -1, -1, -1, -1, -1, -1,
  /*  17- 31*/		4,  3,	4,  3,	2,  1,	2,  3,	2, 1, 2, 3, 4, 3, 4,
  /* dummies 32- 40 */ -1, -1, -1, -1, -1, -1, -1, -1, -1,
  /*  41- 55 */		5,  4,	3,  2,	3,  4,	1,  2,	1, 4, 3, 2, 3, 4, 5,
  /* dummies 56- 64 */ -1, -1, -1, -1, -1, -1, -1, -1, -1,
  /*  65- 79*/		4,  3,	4,  3,	2,  3,	2,  3,	2, 3, 2, 3, 4, 3, 4,
  /* dummies 80- 88 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /*  89-103 */		5,  4,	3,  4,	3,  2,	3,  2,	3, 2, 3, 4, 3, 4, 5,
  /* dummies104-112 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /* 113-127 */		4,  5,	4,  3,	4,  3,	4,  3,	4, 3, 4, 3, 4, 5, 4,
  /* dummies128-136 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /* 137-151 */		5,  4,	5,  4,	3,  4,	3,  4,	3, 4, 3, 4, 5, 4, 5,
  /* dummies152-160 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /* 161-175 */		6,  5,	4,  5,	4,  5,	4,  5,	4, 5, 4, 5, 4, 5, 6
};

smallint ProofBlKingMovesNeeded(void) {
  smallint	cast;
  smallint	needed= BlKingMoves[rn];

  if (TSTFLAGMASK(castling_flag[nbply],ke8_cancastle)) {
    if (TSTFLAGMASK(castling_flag[nbply],ra8_cancastle)) {
      /* bl long castling */
      /* BlKingMoves is the number of moves the bl king
         still needs after castling. It takes 1 move to castle,
         but we might save a rook move
      */
      cast= BlKingMoves[square_c8];
      if (cast < needed) {
        needed= cast;
      }
    }
    if (TSTFLAGMASK(castling_flag[nbply],rh8_cancastle)) {
      /* bl short castling */
      /* BlKingMoves is the number of moves the bl king still
         needs after castling. It takes 1 move to castle, but we
         might save a rook move
      */
      cast= BlKingMoves[square_g8];
      if (cast < needed) {
        needed= cast;
      }
    }
  }
  return needed;
}

smallint ProofWhKingMovesNeeded(void) {
  smallint	needed= WhKingMoves[rb];
  smallint	cast;

  if (TSTFLAGMASK(castling_flag[nbply],ke1_cancastle)) {
    if (TSTFLAGMASK(castling_flag[nbply],ra1_cancastle)) {
      /* wh long castling */
      /* WhKingMoves is the number of moves the wh king still
         needs after castling. It takes 1 move to castle, but we
         might save a rook moves.
      */
      cast= WhKingMoves[square_c1];
      if (cast < needed) {
        needed= cast;
      }
    }
    if (TSTFLAGMASK(castling_flag[nbply],rh1_cancastle)) {
      /* wh short castling */
      /* WhKingMoves is the number of moves the wh king still
         needs after castling. It takes 1 move to castle, but we
         might save a rook moves
      */
      cast= WhKingMoves[square_g1];
      if (cast < needed) {
        needed= cast;
      }
    }
  }
  return needed;
}

void WhPawnMovesFromTo(
  square	from,
  square	to,
  int		*moves,
  int		*captures,
  int		captallowed)
{
  int rank_to= to/onerow;
  int rank_from= from/onerow;

  /* calculate number of captures */
  *captures= to%onerow-from%onerow;
  if (*captures < 0) {
	*captures= -*captures;
  }

  /* calculate number of moves */
  *moves= rank_to-rank_from;

  if (*moves<0 || *moves<*captures || *captures>captallowed)
	*moves= enonce;
  else {
	if (from<=square_h2 && *captures<*moves-1)
      /* double step possible */
      (*moves)--;
  }
}

void BlPawnMovesFromTo(
  square	from,
  square	to,
  int		*moves,
  int		*captures,
  int		captallowed)
{
  int rank_to= to/onerow;
  int rank_from= from/onerow;

  /* calculate number of captures */
  *captures= to%onerow-from%onerow;
  if (*captures < 0) {
	*captures= -*captures;
  }

  /* calculate number of moves */
  *moves= rank_from-rank_to;

  if (*moves<0 || *moves<*captures || *captures>captallowed)
	*moves= enonce;
  else {
	if (from>=square_a7 && *captures < *moves-1)
      /* double step possible */
      (*moves)--;
  }
}

smallint WhPawnMovesNeeded(square sq) {
  smallint	MovesNeeded;
  smallint	MovesNeeded1;

  /* The first time ProofWhPawnMovesNeeded is called the following
     test is always false. It has already been checked in
     ProofImpossible. But we need it here for the recursion.
  */
  if (e[sq]==pb && ProofBoard[sq]!=pb)
	return 0;

  if (sq<=square_h2)
	/* there is no pawn at all that can enter this square */
	return enonce;

  /* double step */
  if (square_a4<=sq && square_h4>=sq
      && e[sq+2*dir_down] == pb
      && ProofBoard[sq+2*dir_down] != pb) {
	return 1;
  }

  if (e[sq+dir_down+dir_right] != obs) {
	MovesNeeded= WhPawnMovesNeeded(sq+dir_down+dir_right);
	if (!MovesNeeded) {
      /* There is a free pawn on sq+dir_down+dir_right
      ** so it takes just 1 move */
      return 1;
	}
  }
  else {
	MovesNeeded= enonce;
  }

  if (e[sq+dir_down+dir_left] != obs) {
	MovesNeeded1= WhPawnMovesNeeded(sq+dir_down+dir_left);
	if (!MovesNeeded1) {
      /* There is a free pawn on sq+dir_down+dir_left
      ** so it takes just 1 move */
      return 1;
	}
	if (MovesNeeded1 < MovesNeeded) {
      MovesNeeded= MovesNeeded1;
	}
  }

  MovesNeeded1= WhPawnMovesNeeded(sq+dir_down);
  if (MovesNeeded1 < MovesNeeded) {
	MovesNeeded= MovesNeeded1;
  }

  return MovesNeeded+1;
}

smallint BlPawnMovesNeeded(square sq) {
  smallint	MovesNeeded;
  smallint	MovesNeeded1;

  /* The first time ProofBlPawnMovesNeeded is called the following
     test is always false. It has already been checked in
     ProofImpossible. But we need it here for the recursion.
  */

  if (e[sq] == pn && ProofBoard[sq] != pn) {
	return 0;
  }

  if (sq>=square_a7)
	/* there is no pawn at all that can enter this square */
	return enonce;

  /* double step */
  if (square_a5<=sq && square_h5>=sq
      && e[sq+2*dir_up] == pn
      && ProofBoard[sq+2*dir_up] != pn) {
	return 1;
  }

  if (e[sq+dir_up+dir_left] != obs) {
	MovesNeeded= BlPawnMovesNeeded(sq+dir_up+dir_left);
	if (!MovesNeeded) {
      /* There is a free pawn on sq+dir_up+dir_left
      ** so it takes just 1 move */
      return 1;
	}
  }
  else {
	MovesNeeded= enonce;
  }

  if (e[sq+dir_up+dir_right] != obs) {
	MovesNeeded1= BlPawnMovesNeeded(sq+dir_up+dir_right);
	if (!MovesNeeded1) {
      /* There is a free pawn on sq+dir_up+dir_right
      ** so it takes just 1 move */
      return 1;
	}
	if (MovesNeeded1 < MovesNeeded) {
      MovesNeeded= MovesNeeded1;
	}
  }

  MovesNeeded1= BlPawnMovesNeeded(sq+dir_up);
  if (MovesNeeded1 < MovesNeeded) {
	MovesNeeded= MovesNeeded1;
  }

  return MovesNeeded+1;
} /* BlPawnMovesNeeded */

#define BLOCKED(sq)                             \
  (  (e[sq] == pb                               \
      && ProofBoard[sq] == pb                   \
      && WhPawnMovesNeeded(sq) >= enonce)       \
     || (e[sq] == pn                            \
         && ProofBoard[sq] == pn                \
         && BlPawnMovesNeeded(sq) >= enonce))

void PieceMovesFromTo(piece p, square from, square to, int *moves) {
  numvec	dir;
  int		sqdiff= from-to;

  if (!sqdiff) {
	*moves= 0;
	return;
  }
  switch (abs(p)) {
  case Knight:
	*moves= ProofKnightMoves[abs(sqdiff)];
	if (*moves > 1) {
      square	sqi, sqj;
      smallint	i, j, testmov;
      smallint	testmin= enonce;
      for (i= 9; i <= 16; i++) {
		sqi= from+vec[i];
		if (!BLOCKED(sqi) && e[sqi] != obs) {
          for (j= 9; j <= 16; j++) {
			sqj= to+vec[j];
			if (!BLOCKED(sqj) && e[sqj] != obs) {
              testmov= ProofKnightMoves[abs(sqi-sqj)]+2;
              if (testmov == *moves) {
				return;
              }
              if (testmov < testmin) {
				testmin= testmov;
              }
			}
          }
		}
      }
      *moves= testmin;
	}
	break;

  case Bishop:
	if (SquareCol(from) != SquareCol(to)) {
      *moves= enonce;
	}
	else {
      dir= CheckDirBishop[sqdiff];
      if (dir) {
		do {
          from-= dir;
		} while (to != from && !BLOCKED(from));
		*moves= to == from ? 1 : 3;
      }
      else {
		*moves= 2;
      }
	}
	break;

  case Rook:
	dir= CheckDirRook[sqdiff];
	if (dir) {
      do {
		from-= dir;
      } while (to != from && !BLOCKED(from));
      *moves= to == from ? 1 : 3;
	}
	else {
      *moves= 2;
	}
	break;

  case Queen:
	dir= CheckDirQueen[sqdiff];
	if (dir) {
      do {
		from-= dir;
      } while (to != from && !BLOCKED(from));
      *moves= to == from ? 1 : 2;
	}
	else {
      *moves= 2;
	}
	break;

  default:
	printf("error in PieceMovesFromTo\n");
	WritePiece(p);
	printf("\n");
  }
} /* PieceMovesFromTo */

void WhPromPieceMovesFromTo(
  square	from,
  square	to,
  int		*moves,
  int		*captures,
  int		captallowed)
{
  int		i, mov1, mov2, cap1;
  square	cenpromsq;

  cenpromsq= (from%onerow
              + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow);
  *moves= enonce;

  WhPawnMovesFromTo(from, cenpromsq, &mov1, &cap1, captallowed);
  PieceMovesFromTo(ProofBoard[to], cenpromsq, to, &mov2);
  if (mov1+mov2 < *moves) {
	*moves= mov1+mov2;
  }

  for (i= 1; i <= captallowed; i++) {
	if (cenpromsq+i <= square_h8) {
      /* got out of range sometimes ! */
      WhPawnMovesFromTo(from, cenpromsq+i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(ProofBoard[to], cenpromsq+i, to, &mov2);
      if (mov1+mov2 < *moves)
		*moves= mov1+mov2;
	}
	if (cenpromsq-i>=square_a8) {
      /* got out of range sometimes ! */
      WhPawnMovesFromTo(from,
                        cenpromsq-i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(ProofBoard[to], cenpromsq-i, to, &mov2);
      if (mov1+mov2 < *moves) {
		*moves= mov1+mov2;
      }
	}
  }

  /* We cannot say for sure how many captures we really need.
  ** We may need 3 moves and 1 capture or 2 moves and 2 captures.
  ** Therefore zero is returned. */
  *captures= 0;
} /* WhPromPieceMovesFromTo */

void BlPromPieceMovesFromTo(
  square	from,
  square	to,
  int		*moves,
  int		*captures,
  int		captallowed)
{
  square	cenpromsq;
  int		i, mov1, mov2, cap1;

  cenpromsq= from%onerow + nr_of_slack_rows_below_board*onerow;
  *moves= enonce;

  BlPawnMovesFromTo(from, cenpromsq, &mov1, &cap1, captallowed);
  PieceMovesFromTo(ProofBoard[to], cenpromsq, to, &mov2);
  if (mov1+mov2 < *moves) {
	*moves= mov1+mov2;
  }

  for (i= 1; i <= captallowed; i++) {
	if (cenpromsq+i<=square_h1) {
      /* got out of range sometimes !*/
      BlPawnMovesFromTo(from, cenpromsq+i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(ProofBoard[to], cenpromsq+i, to, &mov2);
      if (mov1+mov2 < *moves)
		*moves= mov1+mov2;
	}
	if (cenpromsq-i >= square_a1) {
      /* got out of range sometimes ! */
      BlPawnMovesFromTo(from,
                        cenpromsq-i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(ProofBoard[to], cenpromsq-i, to, &mov2);
      if (mov1+mov2 < *moves) {
		*moves= mov1+mov2;
      }
	}
  }

  /* We cannot say for sure how many captures we really need.
  ** We may need 3 moves and 1 capture or 2 moves and 2 captures.
  ** Therefore zero is returned. */
  *captures= 0;
} /* BlPromPieceMovesFromTo */

void WhPieceMovesFromTo(
  square	from,
  square	to,
  int		*moves,
  int		*captures,
  int		captallowed,
  int		captrequ)
{
  piece	pfrom= e[from];
  piece	pto= ProofBoard[to];

  *moves= enonce;

  switch (pto) {
  case pb:
	if (pfrom == pb) {
      WhPawnMovesFromTo(from, to, moves, captures, captallowed);
	}
	break;

  default:
	if (pfrom == pto) {
      PieceMovesFromTo(pfrom, from, to, moves);
      *captures= 0;
	}
	else if (pfrom == pb) {
      WhPromPieceMovesFromTo(from,
                             to, moves, captures, captallowed-captrequ);
	}
  }
}

void BlPieceMovesFromTo(
  square	from,
  square	to,
  int		*moves,
  int		*captures,
  int		captallowed,
  int		captrequ)
{
  piece	pfrom, pto;

  pfrom= e[from];
  pto= ProofBoard[to];
  *moves= enonce;

  switch (pto) {
  case pn:
	if (pfrom == pn) {
      BlPawnMovesFromTo(from, to, moves, captures, captallowed);
	}
	break;

  default:
	if (pfrom == pto) {
      PieceMovesFromTo(pfrom, from, to, moves);
      *captures= 0;
	}
	else if (pfrom == pn) {
      BlPromPieceMovesFromTo(from,
                             to, moves, captures, captallowed-captrequ);
	}
  }
}

typedef struct {
    int		Nbr;
    square	sq[16];
} PieceList;

typedef struct {
    int		Nbr;
    int		moves[16];
    int		captures[16];
    int		id[16];
} PieceList2;

PieceList ProofWhPawns, CurrentWhPawns,
  ProofWhPieces, CurrentWhPieces,
  ProofBlPawns, CurrentBlPawns,
  ProofBlPieces, CurrentBlPieces;

void PrintPieceList(PieceList *pl) {
  int i;

  for (i= 0; i < pl->Nbr; i++) {
	WriteSquare(pl->sq[i]);
	StdString(" ");
  }
  StdString("\n");
}

void PrintPieceList2(PieceList2 *pl, int nto) {
  int i, j;

  for (i= 0; i < nto; i++) {
	for (j= 0; j < pl[i].Nbr; j++) {
      printf("%2d %2d %2d   ",
             pl[i].id[j], pl[i].moves[j], pl[i].captures[j]);
	}
	printf("\n");
  }
}

int ArrangeListedPieces(
  PieceList2	*pl,
  int		nto,
  int		nfrom,
  boolean	*taken,
  int		CapturesAllowed)
{
  int		Diff, Diff2, i, id;

  Diff= enonce;

  if (nto == 0) {
	return 0;
  }
  for (i= 0; i < pl[0].Nbr; i++) {
	id= pl[0].id[i];
	if (taken[id] || pl[0].captures[i] > CapturesAllowed) {
      continue;
	}
	taken[id]= true;
	Diff2= pl[0].moves[i]
      + ArrangeListedPieces(pl+1, nto-1, nfrom,
                            taken, CapturesAllowed-pl[0].captures[i]);

	if (Diff2 < Diff) {
      Diff= Diff2;
	}
	taken[id]= false;
  }
  return Diff;
}

int ArrangePieces(
  int		CapturesAllowed,
  couleur	camp,
  int		CapturesRequired)
{
  int		ifrom, ito, moves, captures, Diff;
  PieceList2	pl[16];
  boolean	taken[16];
  PieceList	*from, *to;

  from= camp == blanc
    ? &CurrentWhPieces
    : &CurrentBlPieces;

  to= camp == blanc
	? &ProofWhPieces
	: &ProofBlPieces;

  if (to->Nbr == 0) {
	return 0;
  }

  for (ito= 0; ito < to->Nbr; ito++) {
	pl[ito].Nbr= 0;
	for (ifrom= 0; ifrom < from->Nbr; ifrom++) {
      if (camp == blanc) {
		WhPieceMovesFromTo(from->sq[ifrom],
                           to->sq[ito], &moves, &captures,
                           CapturesAllowed, CapturesRequired);
      }
      else {
		BlPieceMovesFromTo(from->sq[ifrom],
                           to->sq[ito], &moves, &captures,
                           CapturesAllowed, CapturesRequired);
      }
      if (moves < enonce) {
		pl[ito].moves[pl[ito].Nbr]= moves;
		pl[ito].captures[pl[ito].Nbr]= captures;
		pl[ito].id[pl[ito].Nbr]= ifrom;
		pl[ito].Nbr++;
      }
	}
  }

  for (ifrom= 0; ifrom < from->Nbr; ifrom++) {
	taken[ifrom]= false;
  }

  /* determine minimal number of moves required */
  Diff= ArrangeListedPieces(pl,
                            to->Nbr, from->Nbr, taken, CapturesAllowed);

  return Diff;
}

int ArrangePawns(
  int		CapturesAllowed,
  couleur	camp,
  int		*CapturesRequired)
{
  int		ifrom, ito, moves, captures, Diff;
  PieceList2	pl[8];
  boolean	taken[8];
  PieceList	*from, *to;

  from= camp == blanc
    ? &CurrentWhPawns
    : &CurrentBlPawns;
  to= camp == blanc
	? &ProofWhPawns
	: &ProofBlPawns;

  if (to->Nbr == 0) {
	*CapturesRequired= 0;
	return 0;
  }

  for (ito= 0; ito < to->Nbr; ito++) {
	pl[ito].Nbr= 0;
	for (ifrom= 0; ifrom < from->Nbr; ifrom++) {
      if (camp == blanc) {
		WhPawnMovesFromTo(from->sq[ifrom],
                          to->sq[ito], &moves, &captures, CapturesAllowed);
      }
      else {
		BlPawnMovesFromTo(from->sq[ifrom],
                          to->sq[ito], &moves, &captures, CapturesAllowed);
      }
      if (moves < enonce) {
		pl[ito].moves[pl[ito].Nbr]= moves;
		pl[ito].captures[pl[ito].Nbr]= captures;
		pl[ito].id[pl[ito].Nbr]= ifrom;
		pl[ito].Nbr++;
      }
	}
  }
  for (ifrom= 0; ifrom < from->Nbr; ifrom++) {
	taken[ifrom]= false;
  }

  /* determine minimal number of moves required */
  Diff= ArrangeListedPieces(pl,
                            to->Nbr, from->Nbr, taken, CapturesAllowed);

  if (Diff == enonce) {
	return enonce;
  }

  /* determine minimal number of captures required */
  captures= 0;
  while (ArrangeListedPieces(pl, to->Nbr, from->Nbr, taken, captures)
         == enonce)
  {
	captures++;
  }

  *CapturesRequired= captures;

  return Diff;
}

boolean ProofFairyImpossible(int MovesAvailable) {
  square	*bnp, sq;
  piece	p1, pparr;
  smallint	NbrWh, NbrBl;

  NbrWh = nbpiece[pb]
    + nbpiece[cb]
    + nbpiece[tb]
    + nbpiece[fb]
    + nbpiece[db]
    + nbpiece[roib];

  NbrBl = nbpiece[pn]
    + nbpiece[cn]
    + nbpiece[tn]
    + nbpiece[fn]
    + nbpiece[dn]
    + nbpiece[roin];

  /* not enough time to capture the remaining pieces */
  if (change_moving_piece) {
	if ( (NbrWh + NbrBl
          - ProofNbrWhitePieces
          - ProofNbrBlackPieces) > MovesAvailable)
	{
      return true;
	}
	if (CondFlag[andernach]
        && !anycirce) {
      smallint count= 0;
      /* in AndernachChess we need at least 1 capture if a pawn
         residing at his initial square has moved and has to be
         reestablished via a capture of the opposite side.
         has a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq <= square_h2; sq++) {
		if (e[sq] != pb) {
          if (ProofBoard[sq] == pb) {
			count++;
          }
		}
      }
      if ((16 - count) < ProofNbrBlackPieces) {
		return true;
      }
      count= 0;
      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++) {
		if (e[sq] != pn) {
          if (ProofBoard[sq] == pn) {
			count++;
          }
		}
      }
      if ((16 - count) < ProofNbrWhitePieces) {
		return true;
      }
	}
  }
  else {
	smallint	BlMovesLeft, WhMovesLeft;

	if (FlowFlag(Alternate)) {
      BlMovesLeft= WhMovesLeft= MovesAvailable/2;
      if (MovesAvailable&1) {
		if ((flag_atob&&!flag_appseul) != (enonce&1)) { /* TODO use % */
          WhMovesLeft++;
		}
		else {
          BlMovesLeft++;
		}
      }
	}
	else {				/* ser-dia */
      BlMovesLeft= 0;
      WhMovesLeft= MovesAvailable;
	}

	/* not enough time to capture the remaining pieces */
	if (NbrWh-ProofNbrWhitePieces > BlMovesLeft
	    || NbrBl-ProofNbrBlackPieces > WhMovesLeft)
	{
      return true;
	}

    pparr = CondFlag[parrain] ? pprise[nbply] : vide;
	if (!CondFlag[sentinelles]) {
      /* note, that we are in the !change_moving_piece section
         too many pawns captured or promoted
      */
      if (ProofNbrPiece[pb] > nbpiece[pb]+(pparr==pb)
          || ProofNbrPiece[pn] > nbpiece[pn]+(pparr==pn))
      {
		return true;
      }
	}

	if (CondFlag[anti]) {
      /* note, that we are in the !change_moving_piece section */
      smallint count= 0;
      /* in AntiCirce we need at least 2 captures if a pawn
         residing at his initial square has moved and has to be
         reborn via capture because we need a second pawn to do
         the same to the other rank NOT ALWAYS TRUE ! Only if
         there's no pawn of the same colour on the same rank has
         a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq <= square_h2; sq++) {
		if (e[sq] != pb) {
          if (ProofBoard[sq] == pb) {
			if (   ProofBoard[sq+dir_up] != pb
                   && ProofBoard[sq+2*dir_up] != pb
                   && ProofBoard[sq+3*dir_up] != pb
                   && ProofBoard[sq+4*dir_up] != pb
                   && ProofBoard[sq+5*dir_up] != pb)
			{
              count++;
			}
          }
          else if (ProofBoard[sq+dir_up] == pb
                   && e[sq+dir_up] != pb)
          {
			if (   ProofBoard[sq+2*dir_up] != pb
                   && ProofBoard[sq+3*dir_up] != pb
                   && ProofBoard[sq+4*dir_up] != pb
                   && ProofBoard[sq+5*dir_up] != pb)
			{
              count++;
			}
          }
		}
      }
      if (count & 1) {
		count++;
      }
      if ((16 - count) < ProofNbrBlackPieces) {
		return true;
      }
      count= 0;
      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++) {
		if (e[sq] != pn) {
          if (ProofBoard[sq] == pn) {
			if (   ProofBoard[sq+dir_down] != pn
                   && ProofBoard[sq+2*dir_down] != pn
                   && ProofBoard[sq+3*dir_down] != pn
                   && ProofBoard[sq+4*dir_down] != pn
                   && ProofBoard[sq+5*dir_down] != pn)
			{
              count++;
			}
          }
          else if (ProofBoard[sq+dir_down] == pn
                   && e[sq+dir_down] != pn)
          {
			if (   ProofBoard[sq+2*dir_down] != pn
                   && ProofBoard[sq+3*dir_down] != pn
                   && ProofBoard[sq+4*dir_down] != pn
                   && ProofBoard[sq+5*dir_down] != pn)
			{
              count++;
			}
          }
		}
      }
      if (count & 1) {
		count++;
      }
      if ((16 - count) < ProofNbrWhitePieces) {
		return true;
      }
	}
  }

  /* find a solution ... */
  MovesAvailable *= 2;

  for (bnp= boardnum; *bnp; bnp++) {
	if ((p1= ProofBoard[*bnp]) != vide) {
      if (p1 != e[*bnp]) {
		MovesAvailable--;
      }
	}
  }

  return MovesAvailable < 0;

} /* ProofFairyImpossible */

boolean ProofImpossible(int MovesAvailable) {
  square	*bnp;
  int		WhMovesLeft, BlMovesLeft;
  int		WhPieToBeCapt, BlPieToBeCapt,
    WhCapturesRequired, BlCapturesRequired;
  piece	p1, p2;
  square	sq;
  int		NbrWh, NbrBl;

  if (ProofFairy) {
	return ProofFairyImpossible(MovesAvailable);
  }

  /* too many pawns captured or promoted */
  if (ProofNbrPiece[pb] > nbpiece[pb]
      || ProofNbrPiece[pn] > nbpiece[pn])
  {
	return true;
  }

  NbrWh = nbpiece[pb]
    + nbpiece[cb]
    + nbpiece[tb]
    + nbpiece[fb]
    + nbpiece[db]
    + nbpiece[roib];

  NbrBl = nbpiece[pn]
    + nbpiece[cn]
    + nbpiece[tn]
    + nbpiece[fn]
    + nbpiece[dn]
    + nbpiece[roin];

  /* to many pieces captured */
  if (NbrWh < ProofNbrWhitePieces
      || NbrBl < ProofNbrBlackPieces)
  {
	return true;
  }

  /* check if there is enough time left to capture the
     superfluos pieces
  */
  if (FlowFlag(Alternate)) {
	BlMovesLeft= WhMovesLeft= MovesAvailable/2;
	if (MovesAvailable&1) {
      if ((flag_atob&&!flag_appseul) != (enonce&1)) { /* TODO use % */
		WhMovesLeft++;
      }
      else {
		BlMovesLeft++;
      }
	}
  }
  else {				/* ser-dia */
	BlMovesLeft= 0;
	WhMovesLeft= MovesAvailable;
  }

  /* not enough time to capture the remaining pieces */
  WhPieToBeCapt= NbrWh - ProofNbrWhitePieces;
  BlPieToBeCapt= NbrBl - ProofNbrBlackPieces;
  if (   WhPieToBeCapt > BlMovesLeft
         || BlPieToBeCapt > WhMovesLeft)
  {
	return true;
  }

  /* has one of the blocked pieces been captured ? */
  if (   (BlockedBishopc1 && ProofBoard[square_c1] != fb)
         || (BlockedBishopf1 && ProofBoard[square_f1] != fb)
         || (BlockedBishopc8 && ProofBoard[square_c8] != fn)
         || (BlockedBishopf8 && ProofBoard[square_f8] != fn)
         || (BlockedQueend1  && ProofBoard[square_d1] != db)
         || (BlockedQueend8  && ProofBoard[square_d8] != dn))
  {
	return true;
  }

  /* has a white pawn on the second rank moved or has it
     been captured?
  */
  for (sq= square_a2; sq <= square_h2; sq++) {
	if (ProofBoard[sq] == pb && e[sq] != pb) {
      return true;
	}
  }

  /* has a black pawn on the seventh rank moved or has it
     been captured?
  */
  for (sq= square_a7; sq <= square_h7; sq++) {
	if (ProofBoard[sq] == pn && e[sq] != pn) {
      return true;
	}
  }

  WhMovesLeft -= ProofWhKingMovesNeeded();
  if (WhMovesLeft < 0) {
	return True;
  }
  BlMovesLeft -= ProofBlKingMovesNeeded();
  if (BlMovesLeft < 0) {
	return True;
  }

  if (CondFlag[haanerchess]) {
	return (ProofBoard[move_generation_stack[nbcou].departure] != vide);
  }

  /* collect the pieces for further investigations */
  ProofWhPawns.Nbr=
    ProofWhPieces.Nbr=
    ProofBlPawns.Nbr=
    ProofBlPieces.Nbr=
    CurrentWhPawns.Nbr=
    CurrentWhPieces.Nbr=
    CurrentBlPawns.Nbr=
    CurrentBlPieces.Nbr= 0;

  for (bnp= boardnum; *bnp; bnp++) {
	p1= ProofBoard[*bnp];
	p2= e[*bnp];

	if (p1 == p2)
      continue;

	if (p1 != vide) {
      if (p1 > vide) {  /* it's a white piece */
		switch (p1) {
        case roib:
          break;
        case pb:
          ProofWhPawns.sq[ProofWhPawns.Nbr++]= *bnp;
          ProofWhPieces.sq[ProofWhPieces.Nbr++]= *bnp;
          break;
        default:
          ProofWhPieces.sq[ProofWhPieces.Nbr++]= *bnp;
          break;
		}
      }
      else {  /* it's a black piece */
		switch (p1) {
        case roin:
          break;
        case pn:
          ProofBlPawns.sq[ProofBlPawns.Nbr++]= *bnp;
          ProofBlPieces.sq[ProofBlPieces.Nbr++]= *bnp;
          break;
        default:
          ProofBlPieces.sq[ProofBlPieces.Nbr++]= *bnp;
          break;
		}
      }
	} /* p1 != vide */

	if (p2 != vide) {
      if (p2 > vide) {  /* it's a white piece */
		switch (p2) {
        case roib:
          break;
        case pb:
          CurrentWhPawns.sq[CurrentWhPawns.Nbr++]= *bnp;
          CurrentWhPieces.sq[CurrentWhPieces.Nbr++]= *bnp;
          break;
        default:
          if (  !(CapturedBishopc1 && *bnp == square_c1 && p2 == fb)
                &&!(CapturedBishopf1 && *bnp == square_f1 && p2 == fb)
                &&!(CapturedQueend1 && *bnp == square_d1 && p2 == db))
          {
            CurrentWhPieces.sq[CurrentWhPieces.Nbr++]= *bnp;
          }
          break;
		}
      }
      else {  /* it's a black piece */
		switch (p2) {
        case roin:
          break;
        case pn:
          CurrentBlPawns.sq[CurrentBlPawns.Nbr++]= *bnp;
          CurrentBlPieces.sq[CurrentBlPieces.Nbr++]= *bnp;
          break;
        default:
          if (  !(CapturedBishopc1 && *bnp == square_c1 && p2 == fn)
                &&!(CapturedBishopf1 && *bnp == square_f1 && p2 == fn)
                &&!(CapturedQueend1 && *bnp == square_d1 && p2 == dn))
          {
			CurrentBlPieces.sq[CurrentBlPieces.Nbr++]= *bnp;
          }
          break;
		}
      }
	} /* p2 != vide */
  } /* for (bnp... */

  if (ArrangePawns(BlPieToBeCapt, blanc, &BlCapturesRequired)
      > WhMovesLeft)
  {
	return True;
  }

  if (ArrangePawns(WhPieToBeCapt, noir, &WhCapturesRequired)
      > BlMovesLeft)
  {
	return True;
  }

  if (ArrangePieces(BlPieToBeCapt, blanc, BlCapturesRequired)
      > WhMovesLeft)
  {
	return True;
  }

  if (ArrangePieces(WhPieToBeCapt, noir, WhCapturesRequired)
      > BlMovesLeft)
  {
	return True;
  }

  return false;
} /* ProofImpossible */

boolean ProofSeriesImpossible(int MovesAvailable) {
  square	*bnp, sq;
  piece	p1, p2;
  int		BlPieToBeCapt, BlCapturesRequired;
  int		NbrBl;
  int		WhMovesLeft= MovesAvailable;

  if (ProofFairy) {
	return ProofFairyImpossible(MovesAvailable);
  }

  /* too many pawns captured or promoted */
  if (   ProofNbrPiece[pb] > nbpiece[pb]
         || ProofNbrPiece[pn] > nbpiece[pn])
  {
	return true;
  }

  NbrBl= nbpiece[pn]
    + nbpiece[cn]
    + nbpiece[tn]
    + nbpiece[fn]
    + nbpiece[dn]
    + 1;

  /* to many pieces captured	or */
  /* not enough time to capture the remaining pieces */
  BlPieToBeCapt= NbrBl - ProofNbrBlackPieces;
  if (BlPieToBeCapt < 0 || BlPieToBeCapt > WhMovesLeft) {
	return true;
  }

  /* has a white pawn on the second rank moved ? */
  for (sq= 208; sq <= 215; sq++) {
	if (ProofBoard[sq] == pb && e[sq] != pb) {
      return true;
	}
  }

  /* has a black pawn on the seventh rank been captured ? */
  for (sq= square_a7; sq <= square_h7; sq++) {
	if (ProofBoard[sq] == pn && e[sq] != pn) {
      return true;
	}
  }

  /* has a black piece on the eigth rank been captured ? */
  for (sq= square_a8; sq <= square_h8; sq++) {
	if (ProofBoard[sq] < roin && ProofBoard[sq] != e[sq]) {
      return true;
	}
  }

  WhMovesLeft -= ProofWhKingMovesNeeded();
  if (WhMovesLeft < 0) {
	return true;
  }

  /* collect the pieces for further investigations */
  ProofWhPawns.Nbr=
    ProofWhPieces.Nbr=
    CurrentWhPawns.Nbr=
    CurrentWhPieces.Nbr= 0;

  for (bnp= boardnum; *bnp; bnp++) {
	p1= ProofBoard[*bnp];
	p2= e[*bnp];

	if (p1 != p2) {
      if (p1 > vide) {  /* it's a white piece */
		switch (p1) {
        case roib:
          break;
        case pb:
          ProofWhPawns.sq[ProofWhPawns.Nbr++]= *bnp;
          ProofWhPieces.sq[ProofWhPieces.Nbr++]= *bnp;
          break;
        default:
          ProofWhPieces.sq[ProofWhPieces.Nbr++]= *bnp;
          break;
		}
      } /* p1 > vide */

      if (p2 > vide) {  /* it's a white piece */
		switch (p2) {
        case roib:
          break;
        case pb:
          CurrentWhPawns.sq[CurrentWhPawns.Nbr++]= *bnp;
          CurrentWhPieces.sq[CurrentWhPieces.Nbr++]= *bnp;
          break;
        default:
          CurrentWhPieces.sq[CurrentWhPieces.Nbr++]= *bnp;
          break;
		}
      } /* p2 > vide */
	} /* p1 != p2 */
  } /* for (bnp... */

  if (ArrangePawns(BlPieToBeCapt, blanc, &BlCapturesRequired)
      > WhMovesLeft)
  {
	return true;
  }

  if (ArrangePieces(BlPieToBeCapt, blanc, BlCapturesRequired)
      > WhMovesLeft)
  {
	return true;
  }

  return false;
} /* ProofSeriesImpossible */

boolean ProofSol(couleur camp, smallint n, boolean restartenabled) {
  boolean	flag= false;
  couleur	ad= advers(camp);

  if ((OptFlag[maxsols] && (solutions >= maxsolutions))
      || FlagTimeOut)
  {
	return false;
  }

  /* Let us check whether the position is already in the
     hash table and marked unsolvable.
  */
  if (inhash(camp == blanc ? WhHelpNoSucc : BlHelpNoSucc, n)) {
	return false;
  }

  n--;
  genmove(camp);
  while (encore()){
	if (jouecoup() && !(restartenabled && MoveNbr < RestartNbr)) {
      if (n ? (!ProofImpossible(n)
               && !echecc(camp)
               && ProofSol(ad, n, False))
		  : (ProofIdentical()
		     && !echecc(camp)))
      {
		flag= true;
		if (!n) {
          linesolution();
		}
      }
	}
	if (restartenabled) {
      IncrementMoveNbr();
	}
	repcoup();
  }
  finply();
  n++;

  /* Add the position to the hash table if it has no solutions */
  if (!flag) {
	addtohash(camp == blanc ? WhHelpNoSucc : BlHelpNoSucc, n);
  }

  return flag;
} /* ProofSol */

boolean SeriesProofSol(smallint n, boolean restartenabled) {
  /* no camp, because we play always with white ! */
  boolean flag= false;

  if ((OptFlag[maxsols] && (solutions >= maxsolutions))
      || FlagTimeOut)
  {
	return false;
  }

  /* Let us check whether the position is already in the
     hash table and marked unsolvable.
  */
  if (inhash(SerNoSucc, n)) {
	return false;
  }

  n--;
  genmove(blanc);
  while (encore()){
	if (jouecoup() && !(restartenabled && MoveNbr < RestartNbr)) {
      if (n ? (!ProofSeriesImpossible(n)
               && !echecc(noir)
               && !echecc(blanc)
               && SeriesProofSol(n, False))
		  : (ProofIdentical()
		     && !echecc(blanc)))
      {
		flag= true;
		if (!n) {
          linesolution();
		}
      }
	}
	if (restartenabled) {
      IncrementMoveNbr();
	}
	repcoup();
  }
  finply();

  /* Add the position to the hash table if it has no solutions */
  if (!flag) {
	addtohash(SerNoSucc, n+1);
  }

  return flag;
} /* SeriesProofSol */
