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
#include "stipulation/stipulation.h"
#include "pyproc.h"
#include "pydata.h"
#include "DHT/dhtbcmem.h"
#include "pyproof.h"
#include "pymsg.h"
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "solving/castling.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/observation.h"
#include "optimisations/hash.h"
#include "optimisations/intelligent/moves_left.h"
#include "platform/maxtime.h"
#include "conditions/duellists.h"
#include "conditions/haunted_chess.h"
#include "conditions/vaulting_kings.h"
#include "conditions/imitator.h"
#include "position/position.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/walks.h"
#include "debugging/trace.h"


static position start;
static position target;

/* an array to store the position */
static PieNam ProofPieces[32];
static Flags ProofSpecs[32];
static square ProofSquares[32];

static unsigned int ProofNbrAllPieces;
static unsigned int ProofNbrPieces[nr_sides];

static boolean BlockedQueenBishop[nr_sides];
static boolean BlockedKingBishop[nr_sides];
static boolean BlockedQueen[nr_sides];
static boolean CapturedQueenBishop[nr_sides];
static boolean CapturedKingBishop[nr_sides];
static boolean CapturedQueen[nr_sides];

static stip_length_type current_length;

static boolean ProofFairy;

static byte const black_bit = CHAR_BIT/2 - 1;

typedef struct
{
    int     Nbr;
    square  sq[16];
} PieceList;

static PieceList ProofPawns[nr_sides];
static PieceList PawnsToBeArranged[nr_sides];
static PieceList ProofOfficers[nr_sides];
static PieceList PiecesToBeArranged[nr_sides];

static int KingMoves[nr_sides][maxsquare];

typedef struct
{
    int     Nbr;
    stip_length_type moves[16];
    stip_length_type captures[16];
    int     id[16];
} PieceList2;

static void ProofSmallEncodePiece(byte **bp,
                                  int row, int col,
                                  PieNam p, Flags flags,
                                  boolean *even)
{
  Side const side =  TSTFLAG(flags,White) ? White : Black;
  byte encoded = p;
  assert(!is_piece_neutral(flags));
  if (side==Black)
    encoded |= 1 << black_bit;
  assert(p < 1 << black_bit);
  if (*even)
  {
    **bp += encoded<<(CHAR_BIT/2);
    ++*bp;
  }
  else
    **bp = encoded;
  *even = !*even;
}

static void ProofLargeEncodePiece(byte **bp,
                                  int row, int col,
                                  PieNam p, Flags flags)
{
  **bp = p;
  ++*bp;

  **bp = flags&COLORFLAGS;
  ++*bp;
}

void ProofEncode(stip_length_type min_length, stip_length_type validity_value)
{
  HashBuffer *hb = &hashBuffers[nbply];
  byte *position = hb->cmv.Data;
  byte *bp = position+nr_rows_on_board;

  /* clear the bits for storing the position of pieces */
  memset(position, 0, nr_rows_on_board);

  {
    boolean even = false;
    square a_square= square_a1;
    unsigned int row;
    for (row = 0; row<nr_rows_on_board; ++row, a_square += onerow)
    {
      square curr_square = a_square;
      unsigned int col;
      for (col = 0; col<nr_files_on_board; ++col, curr_square += dir_right)
      {
        PieNam const p = get_walk_of_piece_on_square(curr_square);
        if (p!=Empty)
        {
          Flags const flags = spec[curr_square];
          if (flagfee || is_piece_neutral(some_pieces_flags))
            ProofLargeEncodePiece(&bp,row,col,p,flags);
          else
            ProofSmallEncodePiece(&bp,row,col,p,flags,&even);
          position[row] |= BIT(col);
        }
      }
    }

    if (even)
      ++bp;
  }

  {
    ghost_index_type gi;
    for (gi = 0; gi<nr_ghosts; ++gi)
    {
      square s = (ghosts[gi].on
                  - nr_of_slack_rows_below_board*onerow
                  - nr_of_slack_files_left_of_board);
      unsigned int const row = s/onerow;
      unsigned int const col = s%onerow;
      bp = SmallEncodePiece(bp,
                            row,col,
                            ghosts[gi].ghost,ghosts[gi].flags);
    }
  }

  /* Now the rest of the party */
  bp = CommonEncode(bp,min_length,validity_value);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);
}

static void ProofInitialiseKingMoves(Side side)
{
  square const *bnp;
  square sq;
  int   MoveNbr;
  boolean   GoOn;
  square const square_base = side==White ? square_a1 : square_a8;
  square const square_opponent_base = side==White ? square_a8 : square_a1;
  square const square_pawn_base = side==White ? square_a2 : square_a7;
  square const square_opponent_pawn_base = side==White ? square_a7 : square_a2;
  numvec const dir_backward = side==White ? dir_down : dir_up;
  Cond const trans_king = side==White ? whtrans_king : bltrans_king;
  Cond const supertrans_king = side==White ? whsupertrans_king : blsupertrans_king;
  Cond const vault_king = side==White ? whvault_king : blvault_king;

  /* set all squares to a maximum */
  for (bnp = boardnum; *bnp; ++bnp)
    KingMoves[side][*bnp] = current_length;

  for (sq = square_pawn_base; sq<square_pawn_base+nr_files_on_board; ++sq)
    if (target.board[sq]==Pawn && TSTFLAG(target.spec[sq],side))
      KingMoves[side][sq] = -1; /* blocked */

  for (sq = square_opponent_pawn_base; sq<square_opponent_pawn_base+nr_files_on_board; ++sq)
    if (target.board[sq]==Pawn && TSTFLAG(target.spec[sq],advers(side)))
    {
      KingMoves[side][sq]= -1;    /* blocked */
      if (get_nr_observation_validators()==0)
      {
        KingMoves[side][sq+dir_backward+dir_left] = -2;
        KingMoves[side][sq+dir_backward+dir_right] = -2; /* guarded */
      }
    }

  /* cornered bishops */
  if (BlockedQueenBishop[side])
    KingMoves[side][square_base+file_c]= -1;
  if (BlockedKingBishop[side])
    KingMoves[side][square_base+file_f]= -1;
  if (BlockedQueenBishop[advers(side)])
    KingMoves[side][square_opponent_base+file_c]= -1;
  if (BlockedKingBishop[advers(side)])
    KingMoves[side][square_opponent_base+file_f]= -1;

  /* initialise wh king */
  KingMoves[side][target.king_square[side]]= 0;
  MoveNbr= 0;
  do
  {
    GoOn= false;
    for (bnp= boardnum; *bnp; bnp++)
    {
      if (KingMoves[side][*bnp] == MoveNbr)
      {
        vec_index_type k;
        for (k= vec_queen_end; k>=vec_queen_start; k--)
        {
          sq= *bnp+vec[k];
          if (KingMoves[side][sq] > MoveNbr)
          {
            KingMoves[side][sq]= MoveNbr+1;
            GoOn= true;
          }
          if (CondFlag[trans_king]
              || CondFlag[supertrans_king]
              || (CondFlag[vault_king] && vaulting_kings_transmuting[side]))
          {
            sq= *bnp+vec[k];
            while (!is_square_blocked(sq) && KingMoves[side][sq]!=-1)
            {
              if (KingMoves[side][sq] > MoveNbr)
              {
                KingMoves[side][sq]= MoveNbr+1;
                GoOn= true;
              }
              sq += vec[k];
            }
          }
        }
        if (CondFlag[trans_king]
            || CondFlag[supertrans_king]
            || (CondFlag[vault_king] && vaulting_kings_transmuting[side]))
        {
          vec_index_type k;
          for (k= vec_knight_end; k>=vec_knight_start; k--)
          {
            sq= *bnp+vec[k];
            if (!is_square_blocked(sq) && KingMoves[side][sq]>MoveNbr)
            {
              KingMoves[side][sq]= MoveNbr+1;
              GoOn= true;
            }
          }
        }
      }
    }
    MoveNbr++;
  } while(GoOn);
}

void ProofInitialiseIntelligentSide(Side side)
{
  square const square_base = side==White ? square_a1 : square_a8;

  ProofNbrPieces[side] = 0;

  {
    PieNam i;
    for (i = King; i <= Bishop; ++i)
      ProofNbrPieces[side] += target.number_of_pieces[side][i];
  }

  if (!ProofFairy)
  {
    numvec const dir_forward = side==White ? dir_up : dir_down;
    square const sq_queen_bishop = square_base+file_c;
    square const sq_king_bishop = square_base+file_f;
    square const sq_queen = square_base+file_d;
    square const sq_queen_bishop_block_left = sq_queen_bishop+dir_forward+dir_left;
    square const sq_queen_bishop_block_right = sq_queen_bishop+dir_forward+dir_right;
    square const sq_king_bishop_block_left = sq_king_bishop+dir_forward+dir_left;
    square const sq_king_bishop_block_right = sq_king_bishop+dir_forward+dir_right;
    square const sq_queen_block_left = sq_queen+dir_forward+dir_left;
    square const sq_queen_block_right = sq_queen+dir_forward+dir_right;

    /* determine pieces blocked */
    BlockedQueenBishop[side] = (target.board[sq_queen_bishop]==Bishop && TSTFLAG(target.spec[sq_queen_bishop],side))
        && (target.board[sq_queen_bishop_block_left]==Pawn && TSTFLAG(target.spec[sq_queen_bishop_block_left],side))
        && (target.board[sq_queen_bishop_block_right]==Pawn && TSTFLAG(target.spec[sq_queen_bishop_block_right],side));

    BlockedKingBishop[side] = (target.board[sq_king_bishop]==Bishop && TSTFLAG(target.spec[sq_king_bishop],side))
        && (target.board[sq_king_bishop_block_left]==Pawn && TSTFLAG(target.spec[sq_king_bishop_block_left],side))
        && (target.board[sq_king_bishop_block_right]==Pawn && TSTFLAG(target.spec[sq_king_bishop_block_right],side));

    BlockedQueen[side] = BlockedQueenBishop[side]
        && BlockedKingBishop[side]
        && (target.board[sq_queen]==Queen && TSTFLAG(target.spec[sq_queen],side))
        && (target.board[sq_queen_block_left]==Pawn && TSTFLAG(target.spec[sq_queen_block_left],side))
        && (target.board[sq_queen_block_right]==Pawn && TSTFLAG(target.spec[sq_queen_block_right],side));

    /* determine pieces captured */
    CapturedQueenBishop[side] = !(target.board[sq_queen_bishop]==Bishop && TSTFLAG(target.spec[sq_queen_bishop],side))
        && (target.board[sq_queen_bishop_block_left]==Pawn && TSTFLAG(target.spec[sq_queen_bishop_block_left],side))
        && (target.board[sq_queen_bishop_block_right]==Pawn && TSTFLAG(target.spec[sq_queen_bishop_block_right],side));

    CapturedKingBishop[side] = !(target.board[sq_king_bishop]==Bishop && TSTFLAG(target.spec[sq_king_bishop],side))
        && (target.board[sq_king_bishop_block_left]==Pawn && TSTFLAG(target.spec[sq_king_bishop_block_left],side))
        && (target.board[sq_king_bishop_block_right]==Pawn && TSTFLAG(target.spec[sq_king_bishop_block_right],side));

    CapturedQueen[side] = BlockedQueenBishop[side]
        && BlockedKingBishop[side]
        && !(target.board[sq_queen]==Queen && TSTFLAG(target.spec[sq_queen],side))
        && (target.board[sq_queen_block_left]==Pawn && TSTFLAG(target.spec[sq_queen_block_left],side))
        && (target.board[sq_queen_block_right]==Pawn && TSTFLAG(target.spec[sq_queen_block_right],side));

    /* update castling possibilities */
    if (BlockedQueenBishop[side])
      /* long castling impossible */
      CLRCASTLINGFLAGMASK(0,side,ra_cancastle);

    if (BlockedKingBishop[side])
      /* short castling impossible */
      CLRCASTLINGFLAGMASK(0,side,rh_cancastle);

    if (!TSTCASTLINGFLAGMASK(0,side,ra_cancastle|rh_cancastle))
      /* no wh rook can castle, so the wh king cannot either */
      CLRCASTLINGFLAGMASK(0,side,k_cancastle);

    /* initialise king diff_move arrays */
    ProofInitialiseKingMoves(side);
  }
}

void ProofInitialiseIntelligent(stip_length_type length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParamListEnd();

  current_length = length;

  ProofInitialiseIntelligentSide(White);
  ProofInitialiseIntelligentSide(Black);

  if (!ProofFairy)
    castling_flag[2] = castling_flag[1] = castling_flag[0];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void override_standard_walk(square s, Side side, PieNam orthodox_walk)
{
  PieNam const overriding_walk = standard_walks[orthodox_walk];

  --start.number_of_pieces[side][start.board[s]];
  start.board[s] = overriding_walk;
  ++start.number_of_pieces[side][overriding_walk];
}

void ProofInitialiseStartPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  start = game_array;

  override_standard_walk(square_e1,White,King);
  override_standard_walk(square_d1,White,Queen);
  override_standard_walk(square_a1,White,Rook);
  override_standard_walk(square_h1,White,Rook);
  override_standard_walk(square_c1,White,Bishop);
  override_standard_walk(square_f1,White,Bishop);
  override_standard_walk(square_b1,White,Knight);
  override_standard_walk(square_g1,White,Knight);
  override_standard_walk(square_a2,White,Pawn);
  override_standard_walk(square_b2,White,Pawn);
  override_standard_walk(square_c2,White,Pawn);
  override_standard_walk(square_d2,White,Pawn);
  override_standard_walk(square_e2,White,Pawn);
  override_standard_walk(square_f2,White,Pawn);
  override_standard_walk(square_g2,White,Pawn);
  override_standard_walk(square_h2,White,Pawn);

  override_standard_walk(square_e8,Black,King);
  override_standard_walk(square_d8,Black,Queen);
  override_standard_walk(square_a8,Black,Rook);
  override_standard_walk(square_h8,Black,Rook);
  override_standard_walk(square_c8,Black,Bishop);
  override_standard_walk(square_f8,Black,Bishop);
  override_standard_walk(square_b8,Black,Knight);
  override_standard_walk(square_g8,Black,Knight);
  override_standard_walk(square_a7,Black,Pawn);
  override_standard_walk(square_b7,Black,Pawn);
  override_standard_walk(square_c7,Black,Pawn);
  override_standard_walk(square_d7,Black,Pawn);
  override_standard_walk(square_e7,Black,Pawn);
  override_standard_walk(square_f7,Black,Pawn);
  override_standard_walk(square_g7,Black,Pawn);
  override_standard_walk(square_h7,Black,Pawn);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofSaveStartPosition(void)
{
  unsigned int i;
  PieNam p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  start.king_square[Black] = king_square[Black];
  start.king_square[White] = king_square[White];

  for (p = King; p<PieceCount; ++p)
  {
    start.number_of_pieces[White][p] = number_of_pieces[White][p];
    start.number_of_pieces[Black][p] = number_of_pieces[Black][p];
  }

  for (i = 0; i<maxsquare; ++i)
    start.board[i] = get_walk_of_piece_on_square(i);

  for (i = 0; i<nr_squares_on_board; ++i)
    start.spec[boardnum[i]] = spec[boardnum[i]];

  start.inum = number_of_imitators;
  for (i = 0; i<number_of_imitators; ++i)
    start.isquare[i] = isquare[i];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreStartPosition(void)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king_square[Black] = start.king_square[Black];
  king_square[White] = start.king_square[White];

  for (i = 0; i<nr_squares_on_board; ++i)
    switch (start.board[boardnum[i]])
    {
      case Empty:
        empty_square(boardnum[i]);
        break;

      case Invalid:
        block_square(boardnum[i]);
        break;

      default:
        occupy_square(boardnum[i],start.board[boardnum[i]],start.spec[boardnum[i]]);
        break;
    }

  number_of_imitators = start.inum;
  for (i = 0; i<number_of_imitators; ++i)
    isquare[i] = start.isquare[i];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofSaveTargetPosition(void)
{
  unsigned int i;
  PieNam p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  target.king_square[White] = king_square[White];
  target.king_square[Black] = king_square[Black];

  for (p = King; p<PieceCount; ++p)
  {
    target.number_of_pieces[White][p] = number_of_pieces[White][p];
    target.number_of_pieces[Black][p] = number_of_pieces[Black][p];
  }

  for (i = 0; i<maxsquare; ++i)
    target.board[i] = get_walk_of_piece_on_square(i);

  for (i = 0; i<nr_squares_on_board; ++i)
    target.spec[boardnum[i]] = spec[boardnum[i]];

  target.inum = number_of_imitators;
  for (i = 0; i<number_of_imitators; ++i)
    target.isquare[i] = isquare[i];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreTargetPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  king_square[Black] = target.king_square[Black];
  king_square[White] = target.king_square[White];

  {
    square i;
    for (i = 0; i<maxsquare; ++i)
      switch (target.board[i])
      {
        case Empty:
          empty_square(i);
          break;

        case Invalid:
          block_square(i);
          break;

        default:
          occupy_square(i,target.board[i],target.spec[i]);
          break;
      }
  }

  number_of_imitators = target.inum;

  {
    unsigned int i;
    for (i = 0; i<number_of_imitators; ++i)
      isquare[i] = target.isquare[i];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofWriteStartPosition(slice_index start)
{
  char InitialLine[40];
  sprintf(InitialLine,
          "\nInitial (%s ->):\n",
          ColorString[UserLanguage][slices[start].starter]);
  StdString(InitialLine);
  WritePosition();
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
    if (ProofPieces[i] != get_walk_of_piece_on_square(ProofSquares[i]) && ProofSpecs[i]!=spec[i])
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
  PieNam const last_piece = flagfee ? PieceCount-1 : Bishop;
  PieNam p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = King; p<=last_piece; ++p)
    if (target.number_of_pieces[White][p]!=number_of_pieces[White][p]
        || target.number_of_pieces[Black][p]!=number_of_pieces[Black][p])
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
    for (imi_idx = 0; imi_idx<number_of_imitators; ++imi_idx)
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

static int ProofKingMovesNeeded(Side side)
{
  int   needed;
  int   cast;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  TraceSquare(king_square[side]);
  TraceText("\n");

  if (king_square[side]==initsquare)
    /* no king in play, or king can be created by promotion
     * -> no optimisation possible */
    needed = 0;
  else
  {
    needed = KingMoves[side][king_square[side]];

    if (TSTCASTLINGFLAGMASK(nbply,side,k_cancastle))
    {
      square const square_base = side==White ? square_a1 : square_a8;

      if (TSTCASTLINGFLAGMASK(nbply,side,ra_cancastle))
      {
        /* wh long castling */
        /* KingMoves[White] is the number of moves the wh king still
           needs after castling. It takes 1 move to castle, but we
           might save a rook move.
        */
        cast = KingMoves[side][square_base+file_c];
        if (cast<needed)
          needed= cast;
      }
      if (TSTCASTLINGFLAGMASK(nbply,side,rh_cancastle))
      {
        /* wh short castling */
        /* KingMoves[White] is the number of moves the wh king still
           needs after castling. It takes 1 move to castle, but we
           might save a rook move
        */
        cast = KingMoves[side][square_base+file_g];
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

static void PawnMovesFromTo(Side side,
                            square from, square to,
                            stip_length_type *moves,
                            stip_length_type *captures,
                            stip_length_type captallowed)
{
  SquareFlags const pawn_doublestep_square = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
  int rank_from = from/onerow;
  int rank_to = to/onerow;

  if (side==Black)
  {
    rank_from = nr_rows_on_board-rank_from;
    rank_to = nr_rows_on_board-rank_to;
  }

  /* calculate number of captures */
  *captures= abs(to%onerow-from%onerow);

  /* calculate number of moves */
  if (rank_to<rank_from)
    *moves = current_length;
  else
  {
    *moves = rank_to-rank_from;
    if (*moves<*captures || *captures>captallowed)
      *moves = current_length;
    else if (TSTFLAG(sq_spec[from],pawn_doublestep_square) && *captures<*moves-1)
      /* double step possible */
      --*moves;
  }
}

static stip_length_type PawnMovesNeeded(Side side, square sq)
{
  SquareFlags const double_step = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

  /* The first time ProofWhPawnMovesNeeded is called the following
     test is always false. It has already been checked in
     ProofImpossible. But we need it here for the recursion.
  */
  if ((get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],side))
      && !(target.board[sq]==Pawn && TSTFLAG(target.spec[sq],side)))
    return 0;

  else if (TSTFLAG(sq_spec[sq],double_step))
    /* there is no pawn at all that can enter this square */
    return current_length;

  else
  {
    stip_length_type MovesNeeded;
    stip_length_type MovesNeeded1;
    numvec const dir_backward = side==White ? dir_down : dir_up;

    /* double step */
    square const sq_double_step_departure = sq+2*dir_backward;
    if (TSTFLAG(sq_spec[sq_double_step_departure],double_step)
        && (get_walk_of_piece_on_square(sq_double_step_departure)==Pawn && TSTFLAG(spec[sq_double_step_departure],side))
        && !(target.board[sq_double_step_departure]==Pawn && TSTFLAG(target.spec[sq_double_step_departure],side)))
      return 1;

    if (!is_square_blocked(sq+dir_backward+dir_right))
    {
      MovesNeeded = PawnMovesNeeded(side,sq+dir_backward+dir_right);
      if (MovesNeeded==0)
        /* There is a free pawn on sq+dir_down+dir_right
        ** so it takes just 1 move */
        return 1;
    }
    else
      MovesNeeded = current_length;

    if (!is_square_blocked(sq+dir_backward+dir_left))
    {
      MovesNeeded1 = PawnMovesNeeded(side,sq+dir_backward+dir_left);
      if (MovesNeeded1==0)
        /* There is a free pawn on sq+dir_down+dir_left
        ** so it takes just 1 move */
        return 1;
      else if (MovesNeeded1 < MovesNeeded)
        MovesNeeded = MovesNeeded1;
    }

    MovesNeeded1 = PawnMovesNeeded(side,sq+dir_backward);
    if (MovesNeeded1<MovesNeeded)
      MovesNeeded = MovesNeeded1;

    return MovesNeeded+1;
  }
}

static boolean blocked_by_pawn(square sq)
{
  return (((get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],White))
           && (target.board[sq]==Pawn && TSTFLAG(target.spec[sq],White))
           && PawnMovesNeeded(White,sq)>=current_length)
          || ((get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],Black))
              && (target.board[sq]==Pawn && TSTFLAG(target.spec[sq],Black))
              && PawnMovesNeeded(Black,sq)>=current_length));
}

static void OfficerMovesFromTo(PieNam p,
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
  switch (p)
  {
    case Knight:
      *moves= ProofKnightMoves[abs(sqdiff)];
      if (*moves > 1)
      {
        square    sqi, sqj;
        stip_length_type testmov;
        stip_length_type testmin = current_length;
        vec_index_type i;
        for (i= vec_knight_start; i<=vec_knight_end; ++i)
        {
          sqi= from+vec[i];
          if (!blocked_by_pawn(sqi) && !is_square_blocked(sqi))
          {
            vec_index_type j;
            for (j= vec_knight_start; j<=vec_knight_end; j++)
            {
              sqj= to+vec[j];
              if (!blocked_by_pawn(sqj) && !is_square_blocked(sqj))
              {
                testmov= ProofKnightMoves[abs(sqi-sqj)]+2;
                if (testmov == *moves)
                  return;
                if (testmov < testmin)
                  testmin= testmov;
              }
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
        dir= CheckDir[Bishop][sqdiff];
        if (dir)
        {
          do {
            from-= dir;
          } while (to != from && !blocked_by_pawn(from));
          *moves= to == from ? 1 : 3;
        }
        else
          *moves= 2;
      }
      break;

    case Rook:
      dir= CheckDir[Rook][sqdiff];
      if (dir)
      {
        do {
          from-= dir;
        } while (to != from && !blocked_by_pawn(from));
        *moves= to == from ? 1 : 3;
      }
      else
        *moves= 2;
      break;

    case Queen:
      dir= CheckDir[Queen][sqdiff];
      if (dir)
      {
        do {
          from-= dir;
        } while (to != from && !blocked_by_pawn(from));
        *moves= to == from ? 1 : 2;
      }
      else
        *moves= 2;
      break;

    default:
      StdString("error in PieceMovesFromTo - piece:");WritePiece(p);
      StdString("\n");
      break;
  }
}

static void PromPieceMovesFromTo(Side side,
                                 square from, square to,
                                 stip_length_type *moves,
                                 stip_length_type *captures,
                                 stip_length_type captallowed)
{
  stip_length_type i;
  stip_length_type mov1;
  stip_length_type mov2;
  stip_length_type cap1;
  unsigned int const from_file = from%nr_files_on_board;
  square const promsq_a = side==White ? square_a8 : square_a1;
  square const cenpromsq = promsq_a+from_file;

  *moves= current_length;

  PawnMovesFromTo(side,from, cenpromsq, &mov1, &cap1, captallowed);
  OfficerMovesFromTo(target.board[to], cenpromsq, to, &mov2);
  if (mov1+mov2 < *moves)
    *moves= mov1+mov2;

  for (i = 1; i<=captallowed; ++i)
  {
    if (from_file+i <= file_h)
    {
      /* got out of range sometimes ! */
      PawnMovesFromTo(side,from, cenpromsq+i, &mov1, &cap1, captallowed);
      OfficerMovesFromTo(target.board[to], cenpromsq+i, to, &mov2);
      if (mov1+mov2 < *moves)
        *moves= mov1+mov2;
    }
    if (from_file>=file_a+i)
    {
      /* got out of range sometimes ! */
      PawnMovesFromTo(side,from, cenpromsq-i, &mov1, &cap1, captallowed);
      OfficerMovesFromTo(target.board[to], cenpromsq-i, to, &mov2);
      if (mov1+mov2 < *moves)
        *moves= mov1+mov2;
    }
  }

  /* We cannot say for sure how many captures we really need.
  ** We may need 3 moves and 1 capture or 2 moves and 2 captures.
  ** Therefore zero is returned. */
  *captures= 0;
}

static void PieceMovesFromTo(Side side,
                             square from, square to,
                             stip_length_type *moves,
                             stip_length_type *captures,
                             stip_length_type captallowed,
                             int captrequ)
{
  PieNam const pfrom = get_walk_of_piece_on_square(from);
  PieNam const pto = target.board[to];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%u",captallowed);
  TraceFunctionParam("%d",captrequ);
  TraceFunctionParamListEnd();

  assert(TSTFLAG(spec[from],side));
  assert(TSTFLAG(target.spec[to],side));

  *moves= current_length;

  switch (pto)
  {
    case Pawn:
      if (pfrom==Pawn)
        PawnMovesFromTo(side,from, to, moves, captures, captallowed);
      break;

    default:
      if (pfrom==pto)
      {
        OfficerMovesFromTo(pfrom, from, to, moves);
        *captures= 0;
      }
      else if (pfrom==Pawn)
        PromPieceMovesFromTo(side,
                             from,to,
                             moves,captures,
                             captallowed-captrequ);
      break;
  }

  TraceValue("%u",*moves);
  TraceValue("%u\n",*captures);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

static stip_length_type ArrangePieces(stip_length_type CapturesAllowed,
                                      Side camp,
                                      stip_length_type CapturesRequired)
{
  stip_length_type result;
  PieceList * const to = &ProofOfficers[camp];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",CapturesAllowed);
  TraceEnumerator(Side,camp,"");
  TraceFunctionParam("%u",CapturesRequired);
  TraceFunctionParamListEnd();

  if (to->Nbr == 0)
    result = 0;
  else
  {
    PieceList * const from = &PiecesToBeArranged[camp];
    PieceList2 pl[16];
    boolean   taken[16];
    int ito;
    int ifrom;

    for (ito = 0; ito<to->Nbr; ito++)
    {
      pl[ito].Nbr = 0;
      for (ifrom = 0; ifrom<from->Nbr; ifrom++)
      {
        stip_length_type moves, captures;
        PieceMovesFromTo(camp,
                         from->sq[ifrom],
                         to->sq[ito], &moves, &captures,
                         CapturesAllowed, CapturesRequired);
        if (moves<current_length)
        {
          pl[ito].moves[pl[ito].Nbr]= moves;
          pl[ito].captures[pl[ito].Nbr]= captures;
          pl[ito].id[pl[ito].Nbr]= ifrom;
          pl[ito].Nbr++;
        }
      }
    }

    for (ifrom = 0; ifrom<from->Nbr; ifrom++)
      taken[ifrom]= false;

    /* determine minimal number of moves required */
    result = ArrangeListedPieces(pl,to->Nbr,from->Nbr,taken,CapturesAllowed);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static stip_length_type ArrangePawns(stip_length_type CapturesAllowed,
                                     Side   camp,
                                     stip_length_type *CapturesRequired)
{
  int       ifrom, ito;
  stip_length_type moves, captures, Diff;
  PieceList2    pl[8];
  boolean   taken[8];
  PieceList *from = &PawnsToBeArranged[camp];
  PieceList *to = &ProofPawns[camp];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",CapturesAllowed);
  TraceFunctionParam("%u",camp);
  TraceFunctionParamListEnd();

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
        PawnMovesFromTo(camp,from->sq[ifrom],
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

static boolean ProofFairyImpossible(void)
{
  square const *bnp;
  square sq;
  unsigned int   Nbr[nr_sides];
  unsigned int MovesAvailable = MovesLeft[Black]+MovesLeft[White];

  TraceText("ProofFairyImpossible\n");

  Nbr[White] = number_of_pieces[White][Pawn]
          + number_of_pieces[White][Knight]
          + number_of_pieces[White][Rook]
          + number_of_pieces[White][Bishop]
          + number_of_pieces[White][Queen]
          + number_of_pieces[White][King];

  Nbr[Black] = number_of_pieces[Black][Pawn]
          + number_of_pieces[Black][Knight]
          + number_of_pieces[Black][Rook]
          + number_of_pieces[Black][Bishop]
          + number_of_pieces[Black][Queen]
          + number_of_pieces[Black][King];

  /* not enough time to capture the remaining pieces */
  if (change_moving_piece)
  {
    if (Nbr[White] + Nbr[Black]
        > MovesAvailable + ProofNbrPieces[White] + ProofNbrPieces[Black])
      return true;


    if (CondFlag[andernach]
        && !anycirce && !CondFlag[sentinelles]) {
      unsigned int count= 0;
      /* in AndernachChess we need at least 1 capture if a pawn
         residing at his initial square has moved and has to be
         reestablished via a capture of the opposite side.
         has a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq <= square_h2; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],White))
            && (target.board[sq]==Pawn && TSTFLAG(target.spec[sq],White)))
          ++count;

      if (16-count < ProofNbrPieces[Black])
        return true;

      count = 0;

      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],Black))
            && (target.board[sq]==Pawn && TSTFLAG(target.spec[sq],Black)))
          ++count;

      if (16-count < ProofNbrPieces[White])
        return true;
    }
  }
  else
  {
    if (!CondFlag[masand])
    {
      /* not enough time to capture the remaining pieces */
      if (Nbr[White] > MovesLeft[Black]+ProofNbrPieces[White]
          || Nbr[Black] > MovesLeft[White]+ProofNbrPieces[Black])
        return true;
    }

    if (!CondFlag[sentinelles])
    {
      /* note, that we are in the !change_moving_piece section
         too many pawns captured or promoted
      */
      boolean parrain_pawn[nr_sides] = { false, false };
      if (anyparrain)
      {
        move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
        move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
        if (move_effect_journal[capture].u.piece_removal.removed==Pawn)
        {
          Flags const removed_spec = move_effect_journal[capture].u.piece_removal.removedspec;
          if (TSTFLAG(removed_spec,White))
            parrain_pawn[White] = true;
          if (TSTFLAG(removed_spec,Black))
            parrain_pawn[Black] = true;
        }
      }

      if (target.number_of_pieces[White][Pawn] > number_of_pieces[White][Pawn]+parrain_pawn[White]
          || target.number_of_pieces[Black][Pawn] > number_of_pieces[Black][Pawn]+parrain_pawn[Black])
        return true;
    }

    if (CondFlag[anti])
    {
      /* note, that we are in the !change_moving_piece section */
      unsigned int count= 0;
      /* in AntiCirce we need at least 2 captures if a pawn
         residing at his initial square has moved and has to be
         reborn via capture because we need a second pawn to do
         the same to the other rank NOT ALWAYS TRUE ! Only if
         there's no pawn of the same colour on the same rank has
         a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq<=square_h2; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],White)))
        {
          if ((target.board[sq]==Pawn && TSTFLAG(target.spec[sq],White)))
          {
            if (!(target.board[sq+dir_up]==Pawn && TSTFLAG(target.spec[sq+dir_up],White))
                && !(target.board[sq+2*dir_up]==Pawn && TSTFLAG(target.spec[sq+2*dir_up],White))
                && !(target.board[sq+3*dir_up]==Pawn && TSTFLAG(target.spec[sq+3*dir_up],White))
                && !(target.board[sq+4*dir_up]==Pawn && TSTFLAG(target.spec[sq+4*dir_up],White))
                && !(target.board[sq+5*dir_up]==Pawn && TSTFLAG(target.spec[sq+5*dir_up],White)))
              ++count;
          }
          else if ((target.board[sq+dir_up]==Pawn && TSTFLAG(target.spec[sq+dir_up],White))
                   && !(get_walk_of_piece_on_square(sq+dir_up)==Pawn && TSTFLAG(spec[sq+dir_up],White)))
          {
            if (!(target.board[sq+2*dir_up]==Pawn && TSTFLAG(target.spec[sq+2*dir_up],White))
                && !(target.board[sq+3*dir_up]==Pawn && TSTFLAG(target.spec[sq+3*dir_up],White))
                && !(target.board[sq+4*dir_up]==Pawn && TSTFLAG(target.spec[sq+4*dir_up],White))
                && !(target.board[sq+5*dir_up]==Pawn && TSTFLAG(target.spec[sq+5*dir_up],White)))
              ++count;
          }
        }

      if (count%2 == 1)
        ++count;

      if (16-count < ProofNbrPieces[Black])
        return true;

      count= 0;
      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],Black)))
        {
          if ((target.board[sq]==Pawn && TSTFLAG(target.spec[sq],Black)))
          {
            if (!(target.board[sq+dir_down]==Pawn && TSTFLAG(target.spec[sq+dir_down],Black))
                && !(target.board[sq+2*dir_down]==Pawn && TSTFLAG(target.spec[sq+2*dir_down],Black))
                && !(target.board[sq+3*dir_down]==Pawn && TSTFLAG(target.spec[sq+3*dir_down],Black))
                && !(target.board[sq+4*dir_down]==Pawn && TSTFLAG(target.spec[sq+4*dir_down],Black))
                && !(target.board[sq+5*dir_down]==Pawn && TSTFLAG(target.spec[sq+5*dir_down],Black)))
              ++count;
          }
          else if ((target.board[sq+dir_down]==Pawn && TSTFLAG(target.spec[sq+dir_down],Black))
                   && !(get_walk_of_piece_on_square(sq+dir_down)==Pawn && TSTFLAG(spec[sq+dir_down],Black)))
          {
            if (!(target.board[sq+2*dir_down]==Pawn && TSTFLAG(target.spec[sq+2*dir_down],Black))
                && !(target.board[sq+3*dir_down]==Pawn && TSTFLAG(target.spec[sq+3*dir_down],Black))
                && !(target.board[sq+4*dir_down]==Pawn && TSTFLAG(target.spec[sq+4*dir_down],Black))
                && !(target.board[sq+5*dir_down]==Pawn && TSTFLAG(target.spec[sq+5*dir_down],Black)))
              ++count;
          }
        }

      if (count%2 == 1)
        ++count;
      if (16-count < ProofNbrPieces[White])
        return true;
    }
  }

  /* find a solution ... */
  MovesAvailable *= 2;

  for (bnp = boardnum; *bnp; bnp++)
  {
    PieNam const p = target.board[*bnp];
    if (p!=Empty)
    {
      if (p!=get_walk_of_piece_on_square(*bnp)
          || (target.spec[*bnp]&COLORFLAGS)!=(spec[*bnp]&COLORFLAGS))
      {
        if (MovesAvailable==0)
          return true;
        else
          --MovesAvailable;
      }
    }
  }

  return false;
}

static boolean ProofImpossible(void)
{
  square const *bnp;
  stip_length_type moves_left[nr_sides] = { MovesLeft[White], MovesLeft[Black] };
  stip_length_type to_be_captured[nr_sides];
  stip_length_type captures_required[nr_sides];
  square    sq;
  unsigned int Nbr[nr_sides];

  TraceText("ProofImpossible\n");

  /* too many pawns captured or promoted */
  if (target.number_of_pieces[White][Pawn] > number_of_pieces[White][Pawn])
  {
    TraceValue("%d ",target.number_of_pieces[White][Pawn]);
    TraceValue("%d\n",number_of_pieces[White][Pawn]);
    return true;
  }

  if (target.number_of_pieces[Black][Pawn] > number_of_pieces[Black][Pawn])
  {
    TraceValue("%d ",number_of_pieces[Black][Pawn]);
    TraceValue("%d\n",number_of_pieces[Black][Pawn]);
    return true;
  }

  Nbr[White] = number_of_pieces[White][Pawn]
          + number_of_pieces[White][Knight]
          + number_of_pieces[White][Rook]
          + number_of_pieces[White][Bishop]
          + number_of_pieces[White][Queen]
          + number_of_pieces[White][King];

  Nbr[Black] = number_of_pieces[Black][Pawn]
          + number_of_pieces[Black][Knight]
          + number_of_pieces[Black][Rook]
          + number_of_pieces[Black][Bishop]
          + number_of_pieces[Black][Queen]
          + number_of_pieces[Black][King];

  /* too many pieces captured */
  if (Nbr[White] < ProofNbrPieces[White])
  {
    TraceValue("%d ",Nbr[White]);
    TraceValue("%d\n",ProofNbrPieces[White]);
    return true;
  }
  if (Nbr[Black] < ProofNbrPieces[Black])
  {
    TraceValue("%d ",Nbr[Black]);
    TraceValue("%d\n",ProofNbrPieces[Black]);
    return true;
  }

  /* check if there is enough time left to capture the
     superfluos pieces
  */

  /* not enough time to capture the remaining pieces */
  to_be_captured[White] = Nbr[White]-ProofNbrPieces[White];
  TraceValue("%d ",to_be_captured[White]);
  TraceValue("%d ",Nbr[White]);
  TraceValue("%d ",ProofNbrPieces[White]);
  TraceValue("%d\n",moves_left[Black]);
  if (to_be_captured[White]>moves_left[Black])
    return true;

  to_be_captured[Black] = Nbr[Black] - ProofNbrPieces[Black];
  TraceValue("%d ",to_be_captured[Black]);
  TraceValue("%d ",Nbr[Black]);
  TraceValue("%d ",ProofNbrPieces[Black]);
  TraceValue("%d\n",moves_left[White]);
  if (to_be_captured[Black]>moves_left[White])
    return true;

  /* has one of the blocked pieces been captured ? */
  if ((BlockedQueenBishop[White] && !(target.board[square_c1]==Bishop && TSTFLAG(target.spec[square_c1],White)))
      || (BlockedKingBishop[White] && !(target.board[square_f1]==Bishop && TSTFLAG(target.spec[square_f1],White)))
      || (BlockedQueenBishop[Black] && !(target.board[square_c8]==Bishop && TSTFLAG(target.spec[square_c8],Black)))
      || (BlockedKingBishop[Black] && !(target.board[square_f8]==Bishop && TSTFLAG(target.spec[square_f8],Black)))
      || (BlockedQueen[White]  && !(target.board[square_d1]==Queen && TSTFLAG(target.spec[square_d1],White)))
      || (BlockedQueen[Black]  && !(target.board[square_d8]==Queen && TSTFLAG(target.spec[square_d8],Black))))
  {
    TraceText("blocked piece was captured\n");
    return true;
  }

  /* has a white pawn on the second rank moved or has it
     been captured?
  */
  for (sq= square_a2; sq<=square_h2; sq+=dir_right)
    if ((target.board[sq]==Pawn && TSTFLAG(target.spec[sq],White)) && !(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],White)))
      return true;

  /* has a black pawn on the seventh rank moved or has it
     been captured?
  */
  for (sq= square_a7; sq<=square_h7; sq+=dir_right)
    if ((target.board[sq]==Pawn && TSTFLAG(target.spec[sq],Black)) && !(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(spec[sq],Black)))
      return true;

  {
    stip_length_type const white_king_moves_needed = ProofKingMovesNeeded(White);
    if (moves_left[White]<white_king_moves_needed)
    {
      TraceText(" white_moves_left<white_king_moves_needed\n");
      return true;
    }
    else
      moves_left[White] -= ProofKingMovesNeeded(White);
  }

  {
    stip_length_type const black_king_moves_needed = ProofKingMovesNeeded(Black);
    if (moves_left[Black]<black_king_moves_needed)
    {
      TraceText("black_moves_left<black_king_moves_needed\n");
      return true;
    }
    else
      moves_left[Black] -= black_king_moves_needed;
  }

  if (CondFlag[haanerchess])
  {
    move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
    move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
    TraceText("impossible hole created\n");
    return target.board[move_effect_journal[movement].u.piece_movement.from] != Empty;
  }

  /* collect the pieces for further investigations */
  ProofPawns[White].Nbr = 0;
  ProofOfficers[White].Nbr = 0;
  ProofPawns[Black].Nbr = 0;
  ProofOfficers[Black].Nbr = 0;
  PawnsToBeArranged[White].Nbr = 0;
  PiecesToBeArranged[White].Nbr = 0;
  PawnsToBeArranged[Black].Nbr = 0;
  PiecesToBeArranged[Black].Nbr= 0;

  for (bnp= boardnum; *bnp; bnp++)
  {
    PieNam const p1 = target.board[*bnp];
    PieNam const p2 = get_walk_of_piece_on_square(*bnp);
    Side const side_target = TSTFLAG(target.spec[*bnp],White) ? White : Black;
    Side const side_current = TSTFLAG(spec[*bnp],White) ? White : Black;

    if (p1!=p2 || side_target!=side_current)
    {
      switch (p1)
      {
        case Empty:
        case King:
          break;

        case Pawn:
          ProofPawns[side_target].sq[ProofPawns[side_target].Nbr]= *bnp;
          ProofPawns[side_target].Nbr++;
          ProofOfficers[side_target].sq[ProofOfficers[side_target].Nbr]= *bnp;
          ProofOfficers[side_target].Nbr++;
          break;

        default:
          ProofOfficers[side_target].sq[ProofOfficers[side_target].Nbr]= *bnp;
          ProofOfficers[side_target].Nbr++;
          break;
      }

      switch (p2)
      {
        case Empty:
        case King:
          break;

        case Pawn:
          PawnsToBeArranged[side_current].sq[PawnsToBeArranged[side_current].Nbr++] = *bnp;
          PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr++] = *bnp;
          break;

        default:
          if (side_current==White)
          {
            if (!(CapturedQueenBishop[White] && *bnp == square_c1 && p2 == Bishop)
                && !(CapturedKingBishop[White] && *bnp == square_f1 && p2 == Bishop)
                && !(CapturedQueen[White] && *bnp == square_d1 && p2 == Queen))
              PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr++] = *bnp;
          }
          else
          {
            if (!(CapturedQueenBishop[Black] && *bnp == square_c8 && p2 == Bishop)
                && !(CapturedKingBishop[Black] && *bnp == square_f8 && p2 == Bishop)
                && !(CapturedQueen[Black] && *bnp == square_d8 && p2 == Queen))
              PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr++] = *bnp;
          }
          break;
      }
    }
  }

  if (ArrangePawns(to_be_captured[Black],White,&captures_required[Black])>moves_left[White])
  {
    TraceText("ArrangePawns(BlPieToBeCapt,White,&BlCapturesRequired)"
              ">white_moves_left\n");
    return true;
  }

  if (ArrangePawns(to_be_captured[White],Black,&captures_required[White])>moves_left[Black])
  {
    TraceValue("%u\n",moves_left[Black]);
    TraceText("ArrangePawns(WhPieToBeCapt,Black,&WhCapturesRequired)"
              ">black_moves_left\n");
    return true;
  }

  if (ArrangePieces(to_be_captured[Black],White,captures_required[Black])>moves_left[White])
  {
    TraceText("(ArrangePieces(BlPieToBeCapt,White,BlCapturesRequired)"
              ">white_moves_left\n");
    return true;
  }

  if (ArrangePieces(to_be_captured[White],Black,captures_required[White])>moves_left[Black])
  {
    TraceText("ArrangePieces(WhPieToBeCapt,Black,WhCapturesRequired)"
              ">black_moves_left\n");
    return true;
  }

  TraceText("not ProofImpossible\n");
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
    PieNam const p = target.board[square_i];
    if (p!=Empty)
    {
      ProofPieces[ProofNbrAllPieces] = p;
      ProofSpecs[ProofNbrAllPieces] = target.spec[square_i];
      ProofSquares[ProofNbrAllPieces] = square_i;
      ++ProofNbrAllPieces;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofInitialise(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  saveTargetPiecesAndSquares();

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
                || TSTFLAG(some_pieces_flags, ColourChange)
                || CondFlag[actrevolving]
                || CondFlag[arc]
                || CondFlag[annan]
                || CondFlag[glasgow]
                || CondFlag[takemake]
                || CondFlag[circeassassin]
                || CondFlag[messigny]
                || CondFlag[mars]
                || CondFlag[castlingchess]
                || CondFlag[platzwechselrochade]
                || CondFlag[football]
                || CondFlag[kobulkings]
                || CondFlag[wormholes]
                || CondFlag[dynasty]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

slice_type proof_make_goal_reachable_type(void)
{
  slice_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* TODO Masand can't possibly be the only condition that doesn't
   * allow any optimisation at all.
   */
  if (flagfee
      || (some_pieces_flags&~PieceIdMask&~BIT(Royal))
      || CondFlag[masand])
    result  = no_slice_type;
  else if (ProofFairy)
    result = STGoalReachableGuardFilterProofFairy;
  else
    result = STGoalReachableGuardFilterProof;

  TraceValue("%u\n",result);

  TraceFunctionExit(__func__);
  TraceEnumerator(slice_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goalreachable_guard_proofgame_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,slices[si].starter,"");
  TraceEnumerator(Side,just_moved,"");
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  if (ProofImpossible())
    result = n+2;
  else
    result = solve(slices[si].next1,n);

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goalreachable_guard_proofgame_fairy_solve(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,slices[si].starter,"");
  TraceEnumerator(Side,just_moved,"");
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  if (ProofFairyImpossible())
    result = n+2;
  else
    result = solve(slices[si].next1,n);

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
