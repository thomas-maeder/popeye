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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#if defined(__TURBOC__)
# include <mem.h>
#endif
#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pyhash.h"
#include "DHT/dhtbcmem.h"
#include "pyproof.h"
#include "pymsg.h"
#include "pyslice.h"
#include "pyint.h"
#include "platform/maxtime.h"
#include "trace.h"
#include "pyposit.h"


static position start;
static position target;

/* an array to store the position */
static piece ProofPieces[32];
static square ProofSquares[32];
static unsigned int ProofNbrAllPieces;

static int ProofNbrWhitePieces, ProofNbrBlackPieces;

static boolean BlockedBishopc1, BlockedBishopf1, BlockedQueend1,
  BlockedBishopc8, BlockedBishopf8, BlockedQueend8,
  CapturedBishopc1, CapturedBishopf1, CapturedQueend1,
  CapturedBishopc8, CapturedBishopf8, CapturedQueend8;

ProofImpossible_fct_t alternateImpossible;
static ProofImpossible_fct_t seriesImpossible;

static goal_type goal_to_be_reached;

stip_length_type current_length;

static boolean ProofFairy;

void ProofEncode(stip_length_type validity_value)
{
  HashBuffer *hb = &hashBuffers[nbply];
  byte    *position= hb->cmv.Data;
  byte    *bp= position+nr_rows_on_board;
  byte    pieces= 0;
  int       row, col;
  square a_square= square_a1;
  boolean even= false;

  /* detect cases where we encode the same position twice */
  assert(hashBufferValidity[nbply]!=validity_value);

  /* clear the bits for storing the position of pieces */
  memset(position, 0, nr_rows_on_board);

  for (row=0; row<nr_rows_on_board; row++, a_square+= onerow)
  {
    square curr_square = a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square+=dir_right)
    {
      piece p= e[curr_square];
      if (p!=vide) {
        if (even)
          *bp++ = pieces+(((byte)(p<vide ? 7-p : p))<<(CHAR_BIT/2));
        else
          pieces= (byte)(p<vide ? 7-p : p);
        even= !even;
        position[row] |= BIT(col);
      }
    }
  }

  if (even)
    *bp++ = pieces+(15<<(CHAR_BIT/2));

  assert(validity_value<=(1<<CHAR_BIT));
  *bp++ = (byte)(validity_value);

  if (CondFlag[duellist]) {
    *bp++ = (byte)(whduell[nbply] - square_a1);
    *bp++ = (byte)(blduell[nbply] - square_a1);
  }

  if (CondFlag[blfollow] || CondFlag[whfollow])
    *bp++ = (byte)(move_generation_stack[nbcou].departure - square_a1);

  if (ep[nbply])
    *bp++ = (byte)(ep[nbply] - square_a1);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);

  validateHashBuffer(validity_value);
}

int proofwkm[square_h8+25-(square_a1-25)+1];
int proofbkm[square_h8+25-(square_a1-25)+1];

#define WhKingMoves  (proofwkm-(square_a1-25))
#define BlKingMoves  (proofbkm-(square_a1-25))

static void ProofInitialiseKingMoves(square ProofRB, square ProofRN)
{
  square const *bnp;
  square sq;
  numvec    k;
  int   MoveNbr;
  boolean   GoOn;

  /* set all squares to a maximum */
  for (bnp= boardnum; *bnp; bnp++)
  {
    WhKingMoves[*bnp] = current_length;
    BlKingMoves[*bnp] = current_length;
  }

  /* mark squares occupied or garded by immobile pawns
     white pawns
  */
  for (sq= square_a2; sq <= square_h2; sq++)
    if (target.board[sq] == pb)
    {
      WhKingMoves[sq]= -1;
      BlKingMoves[sq]= -1;    /* blocked */
      if (eval_white == eval_ortho)
      {
        BlKingMoves[sq+dir_up+dir_left]= -2;
        BlKingMoves[sq+dir_up+dir_right]= -2; /* guarded */
      }
    }

  /* black pawns */
  for (sq= square_a7; sq <= square_h7; sq++)
    if (target.board[sq] == pn)
    {
      BlKingMoves[sq]= -1;
      WhKingMoves[sq]= -1;    /* blocked */
      if (eval_black == eval_ortho)
      {
        WhKingMoves[sq+dir_down+dir_right]= -2;
        WhKingMoves[sq+dir_down+dir_left]= -2;    /* guarded */
      }
    }

  /* cornered bishops */
  if (BlockedBishopc1)
  {
    WhKingMoves[square_c1]= -1;
    BlKingMoves[square_c1]= -1; /* blocked */
  }
  if (BlockedBishopf1)
  {
    WhKingMoves[square_f1]= -1;
    BlKingMoves[square_f1]= -1; /* blocked */
  }
  if (BlockedBishopc8)
  {
    WhKingMoves[square_c8]= -1;
    BlKingMoves[square_c8]= -1; /* blocked */
  }
  if (BlockedBishopf8)
  {
    WhKingMoves[square_f8]= -1;
    BlKingMoves[square_f8]= -1; /* blocked */
  }

  /* initialise wh king */
  WhKingMoves[ProofRB]= 0;
  MoveNbr= 0;
  do
  {
    GoOn= false;
    for (bnp= boardnum; *bnp; bnp++)
    {
      if (WhKingMoves[*bnp] == MoveNbr)
      {
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq= *bnp+vec[k];
          if (WhKingMoves[sq] > MoveNbr)
          {
            WhKingMoves[sq]= MoveNbr+1;
            GoOn= true;
          }
          if (calc_trans_king[White]) {
            sq= *bnp+vec[k];
            while (e[sq]!=obs && WhKingMoves[sq]!=-1)
            {
              if (WhKingMoves[sq] > MoveNbr)
              {
                WhKingMoves[sq]= MoveNbr+1;
                GoOn= true;
              }
              sq += vec[k];
            }
          }
        }
        if (calc_trans_king[White])
          for (k= vec_knight_end; k>=vec_knight_start; k--)
          {
            sq= *bnp+vec[k];
            if (e[sq]!=obs && WhKingMoves[sq]>MoveNbr)
            {
              WhKingMoves[sq]= MoveNbr+1;
              GoOn= true;
            }
          }
      }
    }
    MoveNbr++;
  } while(GoOn);

  /* initialise blank king */
  BlKingMoves[ProofRN]= 0;
  MoveNbr= 0;
  do
  {
    GoOn= false;
    for (bnp= boardnum; *bnp; bnp++)
    {
      if (BlKingMoves[*bnp] == MoveNbr)
      {
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq= *bnp+vec[k];
          if (BlKingMoves[sq] > MoveNbr)
          {
            BlKingMoves[sq]= MoveNbr+1;
            GoOn= true;
          }
          if (calc_trans_king[Black])
          {
            sq= *bnp+vec[k];
            while (e[sq]!=obs && BlKingMoves[sq]!=-1)
            {
              if (BlKingMoves[sq] > MoveNbr)
              {
                BlKingMoves[sq]= MoveNbr+1;
                GoOn= true;
              }
              sq += vec[k];
            }
          }
        }
        if (calc_trans_king[Black])
          for (k= vec_knight_end; k>=vec_knight_start; k--)
          {
            sq= *bnp+vec[k];
            if (e[sq]!=obs && BlKingMoves[sq]>MoveNbr)
            {
              BlKingMoves[sq]= MoveNbr+1;
              GoOn= true;
            }
          }
      }
    }

    MoveNbr++;
  } while(GoOn);
}

void ProofInitialiseIntelligent(stip_length_type length)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParamListEnd();

  current_length = length;

  ProofNbrWhitePieces = 0;
  ProofNbrBlackPieces = 0;

  for (i = roib; i <= fb; ++i)
  {
    ProofNbrWhitePieces += nr_piece(target)[i];
    ProofNbrBlackPieces += nr_piece(target)[-i];
  }

  if (!ProofFairy)
  {
    /* determine pieces blocked */
    BlockedBishopc1 = target.board[square_c1] == fb
        && target.board[square_b2] == pb
        && target.board[square_d2] == pb;

    BlockedBishopf1 = target.board[square_f1] == fb
        && target.board[square_e2] == pb
        && target.board[square_g2] == pb;

    BlockedBishopc8 = target.board[square_c8] == fn
        && target.board[square_b7] == pn
        && target.board[square_d7] == pn;

    BlockedBishopf8 = target.board[square_f8] == fn
        && target.board[square_e7] == pn
        && target.board[square_g7] == pn;

    BlockedQueend1 = BlockedBishopc1
        && BlockedBishopf1
        && target.board[square_d1] == db
        && target.board[square_c2] == pb
        && target.board[square_f2] == pb;

    BlockedQueend8 = BlockedBishopc8
        && BlockedBishopf8
        && target.board[square_d8] == dn
        && target.board[square_c7] == pn
        && target.board[square_f7] == pn;

    /* determine pieces captured */
    CapturedBishopc1 = target.board[square_c1] != fb
        && target.board[square_b2] == pb
        && target.board[square_d2] == pb;

    CapturedBishopf1 = target.board[square_f1] != fb
        && target.board[square_e2] == pb
        && target.board[square_g2] == pb;

    CapturedBishopc8 = target.board[square_c8] != fn
        && target.board[square_b7] == pn
        && target.board[square_d7] == pn;

    CapturedBishopf8 = target.board[square_f8] != fn
        && target.board[square_e7] == pn
        && target.board[square_g7] == pn;

    CapturedQueend1 = BlockedBishopc1
        && BlockedBishopf1
        && target.board[square_d1] != db
        && target.board[square_c2] == pb
        && target.board[square_f2] == pb;

    CapturedQueend8 = BlockedBishopc8
        && BlockedBishopf8
        && target.board[square_d8] != dn
        && target.board[square_c7] == pn
        && target.board[square_f7] == pn;

    /* update castling possibilities */
    if (BlockedBishopc1)
      /* wh long castling impossible */
      CLRCASTLINGFLAGMASK(0,White,ra_cancastle);

    if (BlockedBishopf1)
      /* wh short castling impossible */
      CLRCASTLINGFLAGMASK(0,White,rh_cancastle);

    if (BlockedBishopc8)
      /* blank long castling impossible */
      CLRCASTLINGFLAGMASK(0,Black,ra_cancastle);

    if (BlockedBishopf8)
      /* blank short castling impossible */
      CLRCASTLINGFLAGMASK(0,Black,rh_cancastle);

    if (!TSTCASTLINGFLAGMASK(0,White,ra_cancastle|rh_cancastle))
      /* no wh rook can castle, so the wh king cannot either */
      CLRCASTLINGFLAGMASK(0,White,k_cancastle);

    if (!TSTCASTLINGFLAGMASK(0,Black,ra_cancastle|rh_cancastle))
      /* no blank rook can castle, so the blank king cannot either */
      CLRCASTLINGFLAGMASK(0,Black,k_cancastle);

    castling_flag[2] = castling_flag[1] = castling_flag[0];

    /* initialise king diff_move arrays */
    ProofInitialiseKingMoves(target.rb, target.rn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofInitialiseStartPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  start = game_array;

  if (CondFlag[cavaliermajeur])
  {
    start.board[square_b1] = nb;
    start.board[square_g1] = nb;

    start.board[square_b8] = nn;
    start.board[square_g8] = nn;

    nr_piece(start)[nb] = 2;
    nr_piece(start)[nn] = 2;

    nr_piece(start)[cb] = 0;
    nr_piece(start)[cn] = 0;
  }
  else if (CondFlag[leofamily])
  {
    start.board[square_d1] = leob;
    start.board[square_a1] = paob;
    start.board[square_h1] = paob;
    start.board[square_c1] = vaob;
    start.board[square_f1] = vaob;
    start.board[square_b1] = maob;
    start.board[square_g1] = maob;

    start.board[square_d8] = leon;
    start.board[square_a8] = paon;
    start.board[square_h8] = paon;
    start.board[square_c8] = vaon;
    start.board[square_f8] = vaon;
    start.board[square_b8] = maon;
    start.board[square_g8] = maon;

    nr_piece(start)[leob] = 1;
    nr_piece(start)[paob] = 2;
    nr_piece(start)[vaob] = 2;
    nr_piece(start)[maob] = 2;
    nr_piece(start)[leon] = 1;
    nr_piece(start)[paon] = 2;
    nr_piece(start)[vaon] = 2;
    nr_piece(start)[maon] = 2;

    nr_piece(start)[db] = 0;
    nr_piece(start)[tb] = 0;
    nr_piece(start)[fb] = 0;
    nr_piece(start)[cb] = 0;
    nr_piece(start)[dn] = 0;
    nr_piece(start)[tn] = 0;
    nr_piece(start)[fn] = 0;
    nr_piece(start)[cn] = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofSaveStartPosition(void)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  start.rn = king_square[Black];
  start.rb = king_square[White];

  for (i = roib; i <= derbla; ++i)
  {
    nr_piece(start)[i] = nbpiece[i];
    nr_piece(start)[-i] = nbpiece[-i];
  }

  for (i = 0; i<maxsquare; ++i)
    start.board[i] = e[i];

  for (i = 0; i<nr_squares_on_board; ++i)
    start.spec[boardnum[i]] = spec[boardnum[i]];

  start.inum = inum[1];
  for (i = 0; i<maxinum; ++i)
    start.isquare[i] = isquare[i];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreStartPosition(void)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king_square[Black] = start.rn;
  king_square[White] = start.rb;

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    square const square_i = boardnum[i];
    e[square_i] = start.board[square_i];
    spec[square_i] = start.spec[square_i];
  }

  inum[1] = start.inum;
  for (i = 0; i<maxinum; ++i)
    isquare[i] = start.isquare[i];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Swap the color of the piece on a square in a position
 * @param pos address of position
 * @param s
 */
static void swap_color(position *pos, square s)
{
  pos->board[s] = -pos->board[s];
  pos->spec[s]^= BIT(White)+BIT(Black);
}

/* a=>b: swap pieces' colors in the starting position
 */
void ProofStartSwapColors(void)
{
  square const *bnp;
  for (bnp = boardnum; *bnp; bnp++)
    if (!TSTFLAG(start.spec[*bnp],Neutral) && start.board[*bnp]!=vide)
    {
      swap_color(&start,*bnp);
      swap_color(&target,*bnp);
    }

  {
    unsigned int i;
    for (i = 0; i<ProofNbrAllPieces; ++i)
      ProofPieces[i] = -ProofPieces[i];
  }
}

/* Swap the content of two squares in a position
 * @param pos address of position
 * @param s1
 * @param s2
 */
static void swap_squares(position *pos, square s1, square s2)
{
  piece const p = pos->board[s1];
  Flags const sp = pos->spec[s2];

  pos->board[s1] = pos->board[s2];
  pos->spec[s1] = pos->spec[s2];

  pos->board[s2] = p;
  pos->spec[s2] = sp;
}

/* a=>b: reflect starting position at the horizontal center line
 */
void ProofStartReflectboard(void)
{
  {
    square const *bnp;
    for (bnp = boardnum; *bnp<(square_a1+square_h8)/2; ++bnp)
    {
      square const sq_reflected = transformSquare(*bnp,mirra1a8);
      swap_squares(&start,*bnp,sq_reflected);
      swap_squares(&target,*bnp,sq_reflected);
    }
  }

  {
    unsigned int i;
    for (i = 0; i<ProofNbrAllPieces; ++i)
      ProofSquares[i] = transformSquare(ProofSquares[i],mirra1a8);
  }
}

void ProofSaveTargetPosition(void)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  target.rb = king_square[White];
  target.rn = king_square[Black];

  for (i = roib; i <= derbla; ++i)
  {
    nr_piece(target)[i] = nbpiece[i];
    nr_piece(target)[-i] = nbpiece[-i];
  }

  for (i = 0; i<maxsquare; ++i)
    target.board[i] = e[i];

  for (i = 0; i<nr_squares_on_board; ++i)
    target.spec[boardnum[i]] = spec[boardnum[i]];

  target.inum = inum[1];
  for (i = 0; i<maxinum; ++i)
    target.isquare[i] = isquare[i];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreTargetPosition(void)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king_square[Black] = target.rn;
  king_square[White] = target.rb;

  for (i = 0; i<maxsquare; ++i)
    e[i] = target.board[i];

  for (i= 0; i<nr_squares_on_board; ++i)
  {
    square const square_i = boardnum[i];
    spec[square_i] = target.spec[square_i];
  }

  inum[1] = target.inum;
  for (i = 0; i<maxinum; ++i)
    isquare[i] = target.isquare[i];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofWriteStartPosition(slice_index start)
{
  if (goal_to_be_reached==goal_atob)
  {
    char InitialLine[40];
    sprintf(InitialLine,
            "\nInitial (%s ->):\n",
            PieSpString[UserLanguage][slices[start].starter]);
    StdString(InitialLine);
    WritePosition();
  }
  else
  {
    /* nothing - we don't write the game array */
  }
}

static boolean compareProofPieces(void)
{
  boolean result = true;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ProofNbrAllPieces);

  for (i = 0; i<ProofNbrAllPieces; ++i)
  {
    TracePiece(ProofPieces[i]);
    TraceSquare(ProofSquares[i]);
    TracePiece(e[ProofSquares[i]]);
    TraceText("\n");
    if (ProofPieces[i] != e[ProofSquares[i]])
    {
      result = false;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean compareProofNbrPiece(void)
{
  boolean result = true;
  piece const last_piece = flagfee ? derbla : fb;
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = roib; p<=last_piece; ++p)
    if (nr_piece(target)[p]!=nbpiece[p]
        || nr_piece(target)[-p]!=nbpiece[-p])
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean compareImitators(void)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[imitators])
  {
    unsigned int imi_idx;
    for (imi_idx = 0; imi_idx<inum[nbply]; imi_idx++)
      if (target.isquare[imi_idx]!=isquare[imi_idx])
      {
        result = false;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* function that compares the current position with the desired one
 * and returns true if they are identical. Otherwise it returns false.
 */
boolean ProofIdentical(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = compareProofPieces() && compareProofNbrPiece() && compareImitators();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

int const ProofKnightMoves[square_h8-square_a1+1]=
{
  /*   1-  7 */     0,  3,  2,  3,  2,  3,  4,  5,
  /* dummies  8- 16 */ -1, -1, -1, -1, -1, -1, -1, -1, -1,
  /*  17- 31*/      4,  3,  4,  3,  2,  1,  2,  3,  2, 1, 2, 3, 4, 3, 4,
  /* dummies 32- 40 */ -1, -1, -1, -1, -1, -1, -1, -1, -1,
  /*  41- 55 */     5,  4,  3,  2,  3,  4,  1,  2,  1, 4, 3, 2, 3, 4, 5,
  /* dummies 56- 64 */ -1, -1, -1, -1, -1, -1, -1, -1, -1,
  /*  65- 79*/      4,  3,  4,  3,  2,  3,  2,  3,  2, 3, 2, 3, 4, 3, 4,
  /* dummies 80- 88 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /*  89-103 */     5,  4,  3,  4,  3,  2,  3,  2,  3, 2, 3, 4, 3, 4, 5,
  /* dummies104-112 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /* 113-127 */     4,  5,  4,  3,  4,  3,  4,  3,  4, 3, 4, 3, 4, 5, 4,
  /* dummies128-136 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /* 137-151 */     5,  4,  5,  4,  3,  4,  3,  4,  3, 4, 3, 4, 5, 4, 5,
  /* dummies152-160 */ -1, -1, -1, -1, -1, -1, -1, -1,-1,
  /* 161-175 */     6,  5,  4,  5,  4,  5,  4,  5,  4, 5, 4, 5, 4, 5, 6
};

static int ProofBlKingMovesNeeded(void)
{
  int   cast;
  int   needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (king_square[Black]==initsquare)
    /* no king in play, or king can be created by promotion
     * -> no optimisation possible */
    needed = 0;

  else
  {
    needed= BlKingMoves[king_square[Black]];

    if (TSTCASTLINGFLAGMASK(nbply,Black,k_cancastle))
    {
      if (TSTCASTLINGFLAGMASK(nbply,Black,ra_cancastle))
      {
        /* blank long castling */
        /* BlKingMoves is the number of moves the blank king
           still needs after castling. It takes 1 move to castle,
           but we might save a rook move
        */
        cast= BlKingMoves[square_c8];
        if (cast < needed)
          needed= cast;
      }
      if (TSTCASTLINGFLAGMASK(nbply,Black,rh_cancastle))
      {
        /* blank short castling */
        /* BlKingMoves is the number of moves the blank king still
           needs after castling. It takes 1 move to castle, but we
           might save a rook move
        */
        cast= BlKingMoves[square_g8];
        if (cast < needed)
          needed= cast;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",needed);
  TraceFunctionResultEnd();
  return needed;
}

static int ProofWhKingMovesNeeded(void)
{
  int   needed;
  int   cast;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(king_square[White]);
  TraceText("\n");

  if (king_square[White]==initsquare)
    /* no king in play, or king can be created by promotion
     * -> no optimisation possible */
    needed = 0;
  else
  {
    needed = WhKingMoves[king_square[White]];

    if (TSTCASTLINGFLAGMASK(nbply,White,k_cancastle))
    {
      if (TSTCASTLINGFLAGMASK(nbply,White,ra_cancastle))
      {
        /* wh long castling */
        /* WhKingMoves is the number of moves the wh king still
           needs after castling. It takes 1 move to castle, but we
           might save a rook move.
        */
        cast = WhKingMoves[square_c1];
        if (cast<needed)
          needed= cast;
      }
      if (TSTCASTLINGFLAGMASK(nbply,White,rh_cancastle))
      {
        /* wh short castling */
        /* WhKingMoves is the number of moves the wh king still
           needs after castling. It takes 1 move to castle, but we
           might save a rook move
        */
        cast = WhKingMoves[square_g1];
        if (cast<needed)
          needed= cast;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",needed);
  TraceFunctionResultEnd();
  return needed;
}

static void WhPawnMovesFromTo(
  square    from,
  square    to,
  stip_length_type *moves,
  stip_length_type *captures,
  stip_length_type captallowed)
{
  int rank_to= to/onerow;
  int rank_from= from/onerow;

  /* calculate number of captures */
  *captures= abs(to%onerow-from%onerow);

  /* calculate number of moves */
  if (rank_to<rank_from)
    *moves= current_length;
  else
  {
    *moves= rank_to-rank_from;
    if (*moves<*captures || *captures>captallowed)
      *moves= current_length;
    else if (from<=square_h2 && *captures<*moves-1)
      /* double step possible */
      --*moves;
  }
}

static void BlPawnMovesFromTo(
  square    from,
  square    to,
  stip_length_type *moves,
  stip_length_type *captures,
  stip_length_type captallowed)
{
  int rank_to= to/onerow;
  int rank_from= from/onerow;

  /* calculate number of captures */
  *captures= abs(to%onerow-from%onerow);

  /* calculate number of moves */
  if (rank_from<rank_to)
    *moves= current_length;
  else
  {
    *moves= rank_from-rank_to;
    if (*moves<*captures || *captures>captallowed)
      *moves= current_length;
    else if (from>=square_a7 && *captures < *moves-1)
      /* double step possible */
      --*moves;
  }
}

static stip_length_type WhPawnMovesNeeded(square sq)
{
  stip_length_type MovesNeeded;
  stip_length_type MovesNeeded1;

  /* The first time ProofWhPawnMovesNeeded is called the following
     test is always false. It has already been checked in
     ProofImpossible. But we need it here for the recursion.
  */
  if (e[sq]==pb && target.board[sq]!=pb)
    return 0;

  if (sq<=square_h2)
    /* there is no pawn at all that can enter this square */
    return current_length;

  /* double step */
  if (square_a4<=sq && square_h4>=sq
      && e[sq+2*dir_down] == pb
      && target.board[sq+2*dir_down] != pb)
    return 1;

  if (e[sq+dir_down+dir_right] != obs)
  {
    MovesNeeded= WhPawnMovesNeeded(sq+dir_down+dir_right);
    if (!MovesNeeded)
      /* There is a free pawn on sq+dir_down+dir_right
      ** so it takes just 1 move */
      return 1;
  }
  else
    MovesNeeded= current_length;

  if (e[sq+dir_down+dir_left] != obs)
  {
    MovesNeeded1= WhPawnMovesNeeded(sq+dir_down+dir_left);
    if (!MovesNeeded1)
      /* There is a free pawn on sq+dir_down+dir_left
      ** so it takes just 1 move */
      return 1;
    if (MovesNeeded1 < MovesNeeded)
      MovesNeeded= MovesNeeded1;
  }

  MovesNeeded1= WhPawnMovesNeeded(sq+dir_down);
  if (MovesNeeded1<MovesNeeded)
    MovesNeeded= MovesNeeded1;

  return MovesNeeded+1;
}

static stip_length_type BlPawnMovesNeeded(square sq)
{
  stip_length_type MovesNeeded;
  stip_length_type MovesNeeded1;

  /* The first time ProofBlPawnMovesNeeded is called the following
     test is always false. It has already been checked in
     ProofImpossible. But we need it here for the recursion.
  */

  if (e[sq] == pn && target.board[sq] != pn)
    return 0;

  if (sq>=square_a7)
    /* there is no pawn at all that can enter this square */
    return current_length;

  /* double step */
  if (square_a5<=sq && square_h5>=sq
      && e[sq+2*dir_up] == pn
      && target.board[sq+2*dir_up] != pn)
    return 1;

  if (e[sq+dir_up+dir_left] != obs)
  {
    MovesNeeded= BlPawnMovesNeeded(sq+dir_up+dir_left);
    if (!MovesNeeded)
      /* There is a free pawn on sq+dir_up+dir_left
      ** so it takes just 1 move */
      return 1;
  }
  else
    MovesNeeded= current_length;

  if (e[sq+dir_up+dir_right] != obs)
  {
    MovesNeeded1= BlPawnMovesNeeded(sq+dir_up+dir_right);
    if (!MovesNeeded1)
      /* There is a free pawn on sq+dir_up+dir_right
      ** so it takes just 1 move */
      return 1;
    if (MovesNeeded1 < MovesNeeded)
      MovesNeeded= MovesNeeded1;
  }

  MovesNeeded1= BlPawnMovesNeeded(sq+dir_up);
  if (MovesNeeded1 < MovesNeeded)
    MovesNeeded= MovesNeeded1;

  return MovesNeeded+1;
}

#define BLOCKED(sq)                             \
  (  (e[sq] == pb                               \
      && target.board[sq] == pb                   \
      && WhPawnMovesNeeded(sq)>=current_length)       \
     || (e[sq] == pn                            \
         && target.board[sq] == pn                \
         && BlPawnMovesNeeded(sq)>=current_length))

static void PieceMovesFromTo(piece p,
                             square from, square to,
                             stip_length_type *moves)
{
  numvec dir;
  int    sqdiff= from-to;

  if (sqdiff==0)
  {
    *moves= 0;
    return;
  }
  switch (abs(p))
  {
  case Knight:
    *moves= ProofKnightMoves[abs(sqdiff)];
    if (*moves > 1)
    {
      square    sqi, sqj;
      int   i, j;
      stip_length_type testmov;
      stip_length_type testmin = current_length;
      for (i= vec_knight_start; i<=vec_knight_end; ++i)
      {
        sqi= from+vec[i];
        if (!BLOCKED(sqi) && e[sqi] != obs)
          for (j= vec_knight_start; j<=vec_knight_end; j++)
          {
            sqj= to+vec[j];
            if (!BLOCKED(sqj) && e[sqj] != obs)
            {
              testmov= ProofKnightMoves[abs(sqi-sqj)]+2;
              if (testmov == *moves)
                return;
              if (testmov < testmin)
                testmin= testmov;
            }
          }
      }
      *moves= testmin;
    }
    break;

  case Bishop:
    if (SquareCol(from) != SquareCol(to))
      *moves= current_length;
    else
    {
      dir= CheckDirBishop[sqdiff];
      if (dir)
      {
        do {
          from-= dir;
        } while (to != from && !BLOCKED(from));
        *moves= to == from ? 1 : 3;
      }
      else
        *moves= 2;
    }
    break;

  case Rook:
    dir= CheckDirRook[sqdiff];
    if (dir)
    {
      do {
        from-= dir;
      } while (to != from && !BLOCKED(from));
      *moves= to == from ? 1 : 3;
    }
    else
      *moves= 2;
    break;

  case Queen:
    dir= CheckDirQueen[sqdiff];
    if (dir)
    {
      do {
        from-= dir;
      } while (to != from && !BLOCKED(from));
      *moves= to == from ? 1 : 2;
    }
    else
      *moves= 2;
    break;

  default:
    StdString("error in PieceMovesFromTo - piece:");WritePiece(p);
    StdString("\n");
  }
}

static void WhPromPieceMovesFromTo(
    square    from,
    square    to,
    stip_length_type *moves,
    stip_length_type *captures,
    stip_length_type captallowed)
{
  stip_length_type i;
  stip_length_type mov1, mov2, cap1;
  square    cenpromsq;

  cenpromsq= (from%onerow
              + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow);
  *moves= current_length;

  WhPawnMovesFromTo(from, cenpromsq, &mov1, &cap1, captallowed);
  PieceMovesFromTo(target.board[to], cenpromsq, to, &mov2);
  if (mov1+mov2 < *moves)
    *moves= mov1+mov2;

  for (i= 1; i<=captallowed; ++i)
  {
    if (cenpromsq+i <= square_h8) {
      /* got out of range sometimes ! */
      WhPawnMovesFromTo(from, cenpromsq+i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(target.board[to], cenpromsq+i, to, &mov2);
      if (mov1+mov2 < *moves)
        *moves= mov1+mov2;
    }
    if (cenpromsq-i>=square_a8)
    {
      /* got out of range sometimes ! */
      WhPawnMovesFromTo(from, cenpromsq-i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(target.board[to], cenpromsq-i, to, &mov2);
      if (mov1+mov2 < *moves) {
        *moves= mov1+mov2;
      }
    }
  }

  /* We cannot say for sure how many captures we really need.
  ** We may need 3 moves and 1 capture or 2 moves and 2 captures.
  ** Therefore zero is returned. */
  *captures= 0;
}

static void BlPromPieceMovesFromTo(
    square    from,
    square    to,
    stip_length_type *moves,
    stip_length_type *captures,
    stip_length_type captallowed)
{
  square    cenpromsq;
  stip_length_type i, mov1, mov2, cap1;

  cenpromsq= from%onerow + nr_of_slack_rows_below_board*onerow;
  *moves= current_length;

  BlPawnMovesFromTo(from, cenpromsq, &mov1, &cap1, captallowed);
  PieceMovesFromTo(target.board[to], cenpromsq, to, &mov2);
  if (mov1+mov2 < *moves)
    *moves= mov1+mov2;

  for (i= 1; i <= captallowed; ++i)
  {
    if (cenpromsq+i<=square_h1)
    {
      /* got out of range sometimes !*/
      BlPawnMovesFromTo(from, cenpromsq+i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(target.board[to], cenpromsq+i, to, &mov2);
      if (mov1+mov2 < *moves)
        *moves= mov1+mov2;
    }
    if (cenpromsq-i >= square_a1)
    {
      /* got out of range sometimes ! */
      BlPawnMovesFromTo(from, cenpromsq-i, &mov1, &cap1, captallowed);
      PieceMovesFromTo(target.board[to], cenpromsq-i, to, &mov2);
      if (mov1+mov2 < *moves)
        *moves= mov1+mov2;
    }
  }

  /* We cannot say for sure how many captures we really need.
  ** We may need 3 moves and 1 capture or 2 moves and 2 captures.
  ** Therefore zero is returned. */
  *captures= 0;
}

static void WhPieceMovesFromTo(
    square    from,
    square    to,
    stip_length_type *moves,
    stip_length_type *captures,
    stip_length_type captallowed,
    int       captrequ)
{
  piece pfrom= e[from];
  piece pto= target.board[to];

  *moves= current_length;

  switch (pto)
  {
  case pb:
    if (pfrom == pb)
      WhPawnMovesFromTo(from, to, moves, captures, captallowed);
    break;

  default:
    if (pfrom == pto)
    {
      PieceMovesFromTo(pfrom, from, to, moves);
      *captures= 0;
    }
    else if (pfrom == pb)
      WhPromPieceMovesFromTo(from,
                             to, moves, captures, captallowed-captrequ);
  }
}

static void BlPieceMovesFromTo(
    square    from,
    square    to,
    stip_length_type *moves,
    stip_length_type *captures,
    stip_length_type captallowed,
    int       captrequ)
{
  piece pfrom, pto;

  pfrom= e[from];
  pto= target.board[to];
  *moves= current_length;

  switch (pto)
  {
    case pn:
      if (pfrom == pn)
        BlPawnMovesFromTo(from, to, moves, captures, captallowed);
      break;

    default:
      if (pfrom == pto)
      {
        PieceMovesFromTo(pfrom, from, to, moves);
        *captures= 0;
      }
      else if (pfrom == pn)
        BlPromPieceMovesFromTo(from,
                               to, moves, captures, captallowed-captrequ);
  }
}

typedef struct
{
    int     Nbr;
    square  sq[16];
} PieceList;

typedef struct
{
    int     Nbr;
    stip_length_type moves[16];
    stip_length_type captures[16];
    int     id[16];
} PieceList2;

PieceList ProofWhPawns, CurrentWhPawns,
  ProofWhPieces, CurrentWhPieces,
  ProofBlPawns, CurrentBlPawns,
  ProofBlPieces, CurrentBlPieces;

static stip_length_type ArrangeListedPieces(
  PieceList2    *pl,
  int       nto,
  int       nfrom,
  boolean   *taken,
  stip_length_type CapturesAllowed)
{
  stip_length_type Diff, Diff2;
  int i, id;

  Diff= current_length;

  if (nto == 0)
    return 0;

  for (i= 0; i < pl[0].Nbr; ++i)
  {
    id= pl[0].id[i];
    if (taken[id] || pl[0].captures[i]>CapturesAllowed)
      continue;

    taken[id]= true;
    Diff2= pl[0].moves[i]
      + ArrangeListedPieces(pl+1, nto-1, nfrom,
                            taken, CapturesAllowed-pl[0].captures[i]);

    if (Diff2 < Diff)
      Diff= Diff2;

    taken[id]= false;
  }

  return Diff;
}

static stip_length_type ArrangePieces(
  stip_length_type CapturesAllowed,
  Side   camp,
  stip_length_type CapturesRequired)
{
  int       ifrom, ito, Diff;
  stip_length_type moves, captures;
  PieceList2    pl[16];
  boolean   taken[16];
  PieceList *from, *to;

  from= camp == White
    ? &CurrentWhPieces
    : &CurrentBlPieces;

  to= camp == White
    ? &ProofWhPieces
    : &ProofBlPieces;

  if (to->Nbr == 0)
    return 0;

  for (ito= 0; ito < to->Nbr; ito++)
  {
    pl[ito].Nbr= 0;
    for (ifrom= 0; ifrom < from->Nbr; ifrom++)
    {
      if (camp == White)
        WhPieceMovesFromTo(from->sq[ifrom],
                           to->sq[ito], &moves, &captures,
                           CapturesAllowed, CapturesRequired);
      else
        BlPieceMovesFromTo(from->sq[ifrom],
                           to->sq[ito], &moves, &captures,
                           CapturesAllowed, CapturesRequired);
      if (moves < current_length)
      {
        pl[ito].moves[pl[ito].Nbr]= moves;
        pl[ito].captures[pl[ito].Nbr]= captures;
        pl[ito].id[pl[ito].Nbr]= ifrom;
        pl[ito].Nbr++;
      }
    }
  }

  for (ifrom= 0; ifrom < from->Nbr; ifrom++)
    taken[ifrom]= false;

  /* determine minimal number of moves required */
  Diff= ArrangeListedPieces(pl, to->Nbr, from->Nbr, taken, CapturesAllowed);

  return Diff;
}

static stip_length_type ArrangePawns(stip_length_type CapturesAllowed,
                                     Side   camp,
                                     stip_length_type *CapturesRequired)
{
  int       ifrom, ito;
  stip_length_type moves, captures, Diff;
  PieceList2    pl[8];
  boolean   taken[8];
  PieceList *from, *to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",CapturesAllowed);
  TraceFunctionParam("%u",camp);
  TraceFunctionParamListEnd();

  from= camp == White
    ? &CurrentWhPawns
    : &CurrentBlPawns;
  to= camp == White
    ? &ProofWhPawns
    : &ProofBlPawns;

  if (to->Nbr == 0)
  {
    *CapturesRequired= 0;
    Diff = 0;
  }
  else
  {
    for (ito= 0; ito < to->Nbr; ito++)
    {
      pl[ito].Nbr= 0;
      for (ifrom= 0; ifrom<from->Nbr; ifrom++)
      {
        if (camp == White)
          WhPawnMovesFromTo(from->sq[ifrom],
                            to->sq[ito], &moves, &captures, CapturesAllowed);
        else
          BlPawnMovesFromTo(from->sq[ifrom],
                            to->sq[ito], &moves, &captures, CapturesAllowed);
        if (moves < current_length)
        {
          pl[ito].moves[pl[ito].Nbr]= moves;
          pl[ito].captures[pl[ito].Nbr]= captures;
          pl[ito].id[pl[ito].Nbr]= ifrom;
          pl[ito].Nbr++;
        }
      }
    }

    for (ifrom= 0; ifrom < from->Nbr; ifrom++)
      taken[ifrom]= false;

    /* determine minimal number of moves required */
    Diff= ArrangeListedPieces(pl,
                              to->Nbr, from->Nbr, taken, CapturesAllowed);

    if (Diff != current_length)
    {
      /* determine minimal number of captures required */
      captures= 0;
      while (ArrangeListedPieces(pl, to->Nbr, from->Nbr, taken, captures)
             == current_length)
        captures++;

      *CapturesRequired= captures;
      TraceValue("%u\n",*CapturesRequired);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",Diff);
  TraceFunctionResultEnd();
  return Diff;
}

static boolean NeverImpossible(void)
{
  return false;
}

static boolean ProofFairyImpossible(void)
{
  square const *bnp;
  square sq;
  piece pparr;
  int   NbrWh, NbrBl;
  int MovesAvailable = MovesLeft[Black]+MovesLeft[White];

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
  if (change_moving_piece)
  {
    if (NbrWh + NbrBl - ProofNbrWhitePieces - ProofNbrBlackPieces
        > MovesAvailable)
      return true;

    if (CondFlag[andernach]
        && !anycirce) {
      int count= 0;
      /* in AndernachChess we need at least 1 capture if a pawn
         residing at his initial square has moved and has to be
         reestablished via a capture of the opposite side.
         has a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq <= square_h2; sq++)
        if (e[sq]!=pb && target.board[sq]==pb)
          count++;

      if ((16 - count) < ProofNbrBlackPieces)
        return true;

      count= 0;

      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++)
        if (e[sq]!=pn && target.board[sq]==pn)
          count++;

      if ((16 - count) < ProofNbrWhitePieces)
        return true;
    }
  }
  else
  {
    if (!CondFlag[masand])
    {
      /* not enough time to capture the remaining pieces */
      if (NbrWh-ProofNbrWhitePieces > MovesLeft[Black]
          || NbrBl-ProofNbrBlackPieces > MovesLeft[White])
        return true;
    }

    pparr = anyparrain ? pprise[nbply] : vide;
    if (!CondFlag[sentinelles])
    {
      /* note, that we are in the !change_moving_piece section
         too many pawns captured or promoted
      */
      if (nr_piece(target)[pb] > nbpiece[pb]+(pparr==pb)
          || nr_piece(target)[pn] > nbpiece[pn]+(pparr==pn))
        return true;
    }

    if (CondFlag[anti])
    {
      /* note, that we are in the !change_moving_piece section */
      int count= 0;
      /* in AntiCirce we need at least 2 captures if a pawn
         residing at his initial square has moved and has to be
         reborn via capture because we need a second pawn to do
         the same to the other rank NOT ALWAYS TRUE ! Only if
         there's no pawn of the same colour on the same rank has
         a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq<=square_h2; sq++)
        if (e[sq] != pb)
        {
          if (target.board[sq]==pb)
          {
            if (target.board[sq+dir_up]!=pb
                && target.board[sq+2*dir_up]!=pb
                && target.board[sq+3*dir_up]!=pb
                && target.board[sq+4*dir_up]!=pb
                && target.board[sq+5*dir_up]!=pb)
              count++;
          }
          else if (target.board[sq+dir_up] == pb
                   && e[sq+dir_up] != pb)
          {
            if (target.board[sq+2*dir_up]!=pb
                && target.board[sq+3*dir_up]!=pb
                && target.board[sq+4*dir_up]!=pb
                && target.board[sq+5*dir_up]!=pb)
              count++;
          }
        }

      if (count%2 == 1)
        count++;

      if ((16 - count) < ProofNbrBlackPieces)
        return true;

      count= 0;
      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++)
        if (e[sq]!=pn)
        {
          if (target.board[sq] == pn)
          {
            if (target.board[sq+dir_down]!=pn
                && target.board[sq+2*dir_down]!=pn
                && target.board[sq+3*dir_down]!=pn
                && target.board[sq+4*dir_down]!=pn
                && target.board[sq+5*dir_down]!=pn)
              count++;
          }
          else if (target.board[sq+dir_down]==pn
                   && e[sq+dir_down]!=pn)
          {
            if (target.board[sq+2*dir_down]!=pn
                && target.board[sq+3*dir_down]!=pn
                && target.board[sq+4*dir_down]!=pn
                && target.board[sq+5*dir_down]!=pn)
              count++;
          }
        }

      if (count%2 == 1)
        count++;
      if ((16 - count) < ProofNbrWhitePieces)
        return true;
    }
  }

  /* find a solution ... */
  MovesAvailable *= 2;

  for (bnp = boardnum; *bnp; bnp++)
  {
    piece const p = target.board[*bnp];
    if (p!=vide && p!=e[*bnp])
      MovesAvailable--;
  }

  return MovesAvailable < 0;
}

static boolean ProofImpossible(void)
{
  square const *bnp;
  stip_length_type black_moves_left = MovesLeft[Black];
  stip_length_type white_moves_left = MovesLeft[White];
  stip_length_type WhPieToBeCapt, BlPieToBeCapt;
  stip_length_type WhCapturesRequired, BlCapturesRequired;
  stip_length_type white_king_moves_needed, black_king_moves_needed;
  piece p1, p2;
  square    sq;
  int       NbrWh, NbrBl;

  /* too many pawns captured or promoted */
  if (nr_piece(target)[pb] > nbpiece[pb])
  {
    TraceValue("%d ",nr_piece(target)[pb]);
    TraceValue("%d\n",nbpiece[pb]);
    return true;
  }

  if (nr_piece(target)[pn] > nbpiece[pn])
  {
    TraceValue("%d ",nr_piece(target)[pn]);
    TraceValue("%d\n",nbpiece[pn]);
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

  /* too many pieces captured */
  if (NbrWh < ProofNbrWhitePieces)
  {
    TraceValue("%d ",NbrWh);
    TraceValue("%d\n",ProofNbrWhitePieces);
    return true;
  }
  if (NbrBl < ProofNbrBlackPieces)
  {
    TraceValue("%d ",NbrBl);
    TraceValue("%d\n",ProofNbrBlackPieces);
    return true;
  }

  /* check if there is enough time left to capture the
     superfluos pieces
  */

  /* not enough time to capture the remaining pieces */
  WhPieToBeCapt = NbrWh-ProofNbrWhitePieces;
  TraceValue("%d ",WhPieToBeCapt);
  TraceValue("%d ",NbrWh);
  TraceValue("%d ",ProofNbrWhitePieces);
  TraceValue("%d\n",black_moves_left);
  if (WhPieToBeCapt>black_moves_left)
    return true;

  BlPieToBeCapt = NbrBl - ProofNbrBlackPieces;
  TraceValue("%d ",BlPieToBeCapt);
  TraceValue("%d ",NbrBl);
  TraceValue("%d ",ProofNbrBlackPieces);
  TraceValue("%d\n",white_moves_left);
  if (BlPieToBeCapt>white_moves_left)
    return true;

  /* has one of the blocked pieces been captured ? */
  if ((BlockedBishopc1 && target.board[square_c1]!=fb)
      || (BlockedBishopf1 && target.board[square_f1]!=fb)
      || (BlockedBishopc8 && target.board[square_c8]!=fn)
      || (BlockedBishopf8 && target.board[square_f8]!=fn)
      || (BlockedQueend1  && target.board[square_d1]!=db)
      || (BlockedQueend8  && target.board[square_d8]!=dn))
  {
    TraceText("blocked piece was captured\n");
    return true;
  }

  /* has a white pawn on the second rank moved or has it
     been captured?
  */
  for (sq= square_a2; sq<=square_h2; sq+=dir_right)
    if (target.board[sq]==pb && e[sq]!=pb)
    {
      TraceValue("%d ",sq);
      TraceText("target.board[sq]==pb && e[sq]!=pb\n");
      return true;
    }

  /* has a black pawn on the seventh rank moved or has it
     been captured?
  */
  for (sq= square_a7; sq<=square_h7; sq+=dir_right)
    if (target.board[sq]==pn && e[sq]!=pn)
    {
      TraceValue("%d ",sq);
      TraceText("target.board[sq]==pn && e[sq]!=pn\n");
      return true;
    }

  white_king_moves_needed = ProofWhKingMovesNeeded();
  if (white_moves_left<white_king_moves_needed)
  {
    TraceValue("%u",white_king_moves_needed);
    TraceValue("%u",white_moves_left);
    TraceText(" white_moves_left<white_king_moves_needed\n");
    return true;
  }
  else
    white_moves_left -= ProofWhKingMovesNeeded();

  black_king_moves_needed = ProofBlKingMovesNeeded();
  if (black_moves_left<black_king_moves_needed)
  {
    TraceText("black_moves_left<black_king_moves_needed\n");
    return true;
  }
  else
  {
    black_moves_left -= black_king_moves_needed;
    TraceValue("%u",black_king_moves_needed);
    TraceValue("->%u\n",black_moves_left);
  }

  if (CondFlag[haanerchess])
  {
    TraceText("impossible hole created\n");
    return target.board[move_generation_stack[nbcou].departure] != vide;
  }

  /* collect the pieces for further investigations */
  ProofWhPawns.Nbr = 0;
  ProofWhPieces.Nbr = 0;
  ProofBlPawns.Nbr = 0;
  ProofBlPieces.Nbr = 0;
  CurrentWhPawns.Nbr = 0;
  CurrentWhPieces.Nbr = 0;
  CurrentBlPawns.Nbr = 0;
  CurrentBlPieces.Nbr= 0;

  for (bnp= boardnum; *bnp; bnp++)
  {
    p1= target.board[*bnp];
    p2= e[*bnp];

    if (p1 == p2)
      continue;

    if (p1 != vide)
    {
      if (p1 > vide)
      {  /* it's a white piece */
        switch (p1)
        {
          case roib:
            break;

          case pb:
            ProofWhPawns.sq[ProofWhPawns.Nbr]= *bnp;
            ProofWhPawns.Nbr++;
            ProofWhPieces.sq[ProofWhPieces.Nbr]= *bnp;
            ProofWhPieces.Nbr++;
            break;

          default:
            ProofWhPieces.sq[ProofWhPieces.Nbr]= *bnp;
            ProofWhPieces.Nbr++;
            break;
        }
      }
      else
      {  /* it's a black piece */
        switch (p1)
        {
          case roin:
            break;

          case pn:
            ProofBlPawns.sq[ProofBlPawns.Nbr]= *bnp;
            ProofBlPawns.Nbr++;
            ProofBlPieces.sq[ProofBlPieces.Nbr]= *bnp;
            ProofBlPieces.Nbr++;
            break;

          default:
            ProofBlPieces.sq[ProofBlPieces.Nbr]= *bnp;
            ProofBlPieces.Nbr++;
            break;
        }
      }
    } /* p1 != vide */

    if (p2 != vide)
    {
      if (p2 > vide)  /* it's a white piece */
      {
        switch (p2)
        {
          case roib:
            break;

          case pb:
            CurrentWhPawns.sq[CurrentWhPawns.Nbr]= *bnp;
            CurrentWhPawns.Nbr++;
            CurrentWhPieces.sq[CurrentWhPieces.Nbr]= *bnp;
            CurrentWhPieces.Nbr++;
            break;

          default:
            if (!(CapturedBishopc1 && *bnp == square_c1 && p2 == fb)
                &&!(CapturedBishopf1 && *bnp == square_f1 && p2 == fb)
                &&!(CapturedQueend1 && *bnp == square_d1 && p2 == db))
              CurrentWhPieces.sq[CurrentWhPieces.Nbr++]= *bnp;
            break;
        }
      }
      else  /* it's a black piece */
      {
        switch (p2)
        {
          case roin:
            break;
          case pn:
            CurrentBlPawns.sq[CurrentBlPawns.Nbr]= *bnp;
            CurrentBlPawns.Nbr++;
            CurrentBlPieces.sq[CurrentBlPieces.Nbr]= *bnp;
            CurrentBlPieces.Nbr++;
            break;
          default:
            if (!(CapturedBishopc1 && *bnp == square_c1 && p2 == fn)
                &&!(CapturedBishopf1 && *bnp == square_f1 && p2 == fn)
                &&!(CapturedQueend1 && *bnp == square_d1 && p2 == dn))
              CurrentBlPieces.sq[CurrentBlPieces.Nbr++]= *bnp;
            break;
        }
      }
    } /* p2 != vide */
  } /* for (bnp... */

  if (ArrangePawns(BlPieToBeCapt,White,&BlCapturesRequired)>white_moves_left)
  {
    TraceText("ArrangePawns(BlPieToBeCapt,White,&BlCapturesRequired)"
              ">white_moves_left\n");
    return true;
  }

  if (ArrangePawns(WhPieToBeCapt,Black,&WhCapturesRequired)>black_moves_left)
  {
    TraceValue("%u\n",black_moves_left);
    TraceText("ArrangePawns(WhPieToBeCapt,Black,&WhCapturesRequired)"
              ">black_moves_left\n");
    return true;
  }

  if (ArrangePieces(BlPieToBeCapt,White,BlCapturesRequired)>white_moves_left)
  {
    TraceText("(ArrangePieces(BlPieToBeCapt,White,BlCapturesRequired)"
              ">white_moves_left\n");
    return true;
  }

  if (ArrangePieces(WhPieToBeCapt,Black,WhCapturesRequired)>black_moves_left)
  {
    TraceText("ArrangePieces(WhPieToBeCapt,Black,WhCapturesRequired)"
              ">black_moves_left\n");
    return true;
  }

  TraceText("not ProofImpossible\n");
  return false;
}

static boolean ProofSeriesImpossible(void)
{
  square const *bnp;
  square sq;
  stip_length_type BlPieToBeCapt, BlCapturesRequired;
  int       NbrBl;
  stip_length_type white_moves_left= MovesLeft[Black]+MovesLeft[White];
  stip_length_type white_king_moves_needed;

  TraceValue("%d\n",MovesLeft[Black]+MovesLeft[White]);
  /* too many pawns captured or promoted */
  if (nr_piece(target)[pb]>nbpiece[pb]
      || nr_piece(target)[pn]>nbpiece[pn])
    return true;

  NbrBl= nbpiece[pn]
    + nbpiece[cn]
    + nbpiece[tn]
    + nbpiece[fn]
    + nbpiece[dn]
    + nbpiece[roin];

  /* to many pieces captured    or */
  /* not enough time to capture the remaining pieces */
  if (NbrBl<ProofNbrBlackPieces)
    return true;
  else
  {
    BlPieToBeCapt= NbrBl - ProofNbrBlackPieces;
    if (BlPieToBeCapt>white_moves_left)
      return true;
  }

  /* has a white pawn on the second rank moved ? */
  for (sq = square_a2; sq<=square_h2; sq += dir_right)
    if (target.board[sq]==pb && e[sq]!=pb)
      return true;

  /* has a black pawn on the seventh rank been captured ? */
  for (sq = square_a7; sq<=square_h7; sq += dir_right)
    if (target.board[sq]==pn && e[sq]!=pn)
      return true;

  /* has a black piece on the eigth rank been captured ? */
  for (sq = square_a8; sq<=square_h8; sq += dir_right)
    if (target.board[sq]<roin && target.board[sq]!=e[sq])
      return true;

  white_king_moves_needed = ProofWhKingMovesNeeded();
  if (white_moves_left<white_king_moves_needed)
    return true;
  else
    white_moves_left -= white_king_moves_needed;

  /* collect the pieces for further investigations */
  ProofWhPawns.Nbr=
    ProofWhPieces.Nbr=
    CurrentWhPawns.Nbr=
    CurrentWhPieces.Nbr= 0;

  for (bnp= boardnum; *bnp; bnp++) {
    piece const p1= target.board[*bnp];
    piece const p2= e[*bnp];

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

  if (ArrangePawns(BlPieToBeCapt,White,&BlCapturesRequired)
      > white_moves_left)
    return true;

  if (ArrangePieces(BlPieToBeCapt,White,BlCapturesRequired)
      > white_moves_left)
    return true;

  return false;
}

static void saveTargetPiecesAndSquares(void)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ProofNbrAllPieces = 0;

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    square const square_i = boardnum[i];
    piece const p = target.board[square_i];
    if (p!=vide)
    {
      ProofPieces[ProofNbrAllPieces] = p;
      ProofSquares[ProofNbrAllPieces] = square_i;
      ++ProofNbrAllPieces;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofInitialise(slice_index si)
{
  Goal unique_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  unique_goal = find_unique_goal(si);
  if (unique_goal.type==no_goal)
    VerifieMsg(MultipleGoalsWithProogGameNotAcceptable);
  else
  {
    saveTargetPiecesAndSquares();

    goal_to_be_reached = unique_goal.type;
    assert(goal_to_be_reached==goal_proofgame || goal_to_be_reached==goal_atob);

    ProofFairy = (change_moving_piece
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
                  || CondFlag[mars]
                  || CondFlag[castlingchess]
                  || CondFlag[football]);

    /* TODO Masand can't possibly be the only condition that doesn't
     * allow any optimisation at all.
     */
    TraceValue("%u\n",flagfee);
    if (flagfee
        || PieSpExFlags&(~(BIT(White)+BIT(Black)))
        || CondFlag[masand])
    {
      TraceText("no optimisation\n");
      alternateImpossible = &NeverImpossible;
      seriesImpossible = &NeverImpossible;
    }
    else if (ProofFairy)
    {
      TraceText("fairy optimisation\n");
      alternateImpossible = &ProofFairyImpossible;
      seriesImpossible = &ProofFairyImpossible;
    }
    else
    {
      TraceText("full optimisation\n");
      alternateImpossible = &ProofImpossible;
      seriesImpossible = &ProofSeriesImpossible;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
