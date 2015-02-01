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

#include "debugging/assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#if defined(__TURBOC__)
# include <mem.h>
#endif
#include "stipulation/stipulation.h"
#include "DHT/dhtbcmem.h"
#include "solving/proofgames.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/pipe.h"
#include "solving/observation.h"
#include "optimisations/hash.h"
#include "platform/maxtime.h"
#include "conditions/haunted_chess.h"
#include "position/position.h"
#include "pieces/pieces.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/pawns/en_passant.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "debugging/trace.h"

boolean change_moving_piece;

position proofgames_start_position;
position proofgames_target_position;

/* an array to store the position */
static piece_walk_type ProofPieces[nr_squares_on_board];
static Flags ProofSpecs[nr_squares_on_board];
static square ProofSquares[nr_squares_on_board];

static unsigned int ProofNbrAllPieces;

static byte const black_bit = CHAR_BIT/2 - 1;

static void ProofSmallEncodePiece(byte **bp,
                                  int row, int col,
                                  piece_walk_type p, Flags flags,
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
                                  piece_walk_type p, Flags flags)
{
  **bp = p;
  ++*bp;

  **bp = flags&COLOURFLAGS;
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
        piece_walk_type const p = get_walk_of_piece_on_square(curr_square);
        if (p!=Empty)
        {
          Flags const flags = being_solved.spec[curr_square];
          if (piece_walk_may_exist_fairy || is_piece_neutral(some_pieces_flags))
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
    underworld_index_type gi;
    for (gi = 0; gi<nr_ghosts; ++gi)
    {
      square s = (underworld[gi].on
                  - nr_of_slack_rows_below_board*onerow
                  - nr_of_slack_files_left_of_board);
      unsigned int const row = s/onerow;
      unsigned int const col = s%onerow;
      bp = SmallEncodePiece(bp,
                            row,col,
                            underworld[gi].walk,underworld[gi].flags);
    }
  }

  /* Now the rest of the party */
  bp = CommonEncode(bp,min_length,validity_value);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);
}

static void override_standard_walk(square s, Side side, piece_walk_type orthodox_walk)
{
  piece_walk_type const overriding_walk = standard_walks[orthodox_walk];

  --proofgames_start_position.number_of_pieces[side][proofgames_start_position.board[s]];
  proofgames_start_position.board[s] = overriding_walk;
  ++proofgames_start_position.number_of_pieces[side][overriding_walk];
}

void ProofInitialiseStartPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  proofgames_start_position = game_array;

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
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  proofgames_start_position = being_solved;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreStartPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved = proofgames_start_position;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofSaveTargetPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  proofgames_target_position = being_solved;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void ProofRestoreTargetPosition(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved = proofgames_target_position;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
    TraceSquare(ProofSquares[i]);
    TraceWalk(ProofPieces[i]);
    TraceValue("%x",ProofSpecs[i]);
    TraceWalk(being_solved.board[ProofSquares[i]]);
    TraceValue("%x",being_solved.spec[ProofSquares[i]]);
    TraceEOL();
    if (ProofPieces[i]!=get_walk_of_piece_on_square(ProofSquares[i])
        || ProofSpecs[i]!=(being_solved.spec[ProofSquares[i]]&PieSpMask))
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
  piece_walk_type const last_piece = piece_walk_may_exist_fairy ? nr_piece_walks-1 : Bishop;
  piece_walk_type p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = King; p<=last_piece; ++p)
    if (proofgames_target_position.number_of_pieces[White][p]!=being_solved.number_of_pieces[White][p]
        || proofgames_target_position.number_of_pieces[Black][p]!=being_solved.number_of_pieces[Black][p])
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
    for (imi_idx = 0; imi_idx<being_solved.number_of_imitators; ++imi_idx)
      if (proofgames_target_position.isquare[imi_idx]!=being_solved.isquare[imi_idx])
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

static void loadTargetPiecesAndSquares(void)
{
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ProofNbrAllPieces = 0;

  for (i = 0; i<nr_squares_on_board; ++i)
  {
    square const square_i = boardnum[i];
    piece_walk_type const p = proofgames_target_position.board[square_i];
    if (p!=Empty && p!=Invalid)
    {
      ProofPieces[ProofNbrAllPieces] = p;
      ProofSpecs[ProofNbrAllPieces] = proofgames_target_position.spec[square_i]&PieSpMask;
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

  loadTargetPiecesAndSquares();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
