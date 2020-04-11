#include "optimisations/intelligent/count_nr_of_moves.h"
#include "solving/proofgames.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "solving/castling.h"
#include "position/move_diff_code.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    unsigned int nr_remaining_moves[nr_sides];
    unsigned int nr_unused_masses[nr_sides];
    piece_usage usage;
} reserve_elmt_type;

static reserve_elmt_type reserve[nr_squares_on_board];

static unsigned int curr_reserve;

unsigned int const minimum_number_knight_moves[square_h8-square_a1+1]=
{
  /*   1-  7 */     0,  3,  2,  3,  2,  3,  4,  5,
  /* dummies  8- 16 */ UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX,
  /*  17- 31*/      4,  3,  4,  3,  2,  1,  2,  3,  2, 1, 2, 3, 4, 3, 4,
  /* dummies 32- 40 */ UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX,
  /*  41- 55 */     5,  4,  3,  2,  3,  4,  1,  2,  1, 4, 3, 2, 3, 4, 5,
  /* dummies 56- 64 */ UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX,
  /*  65- 79*/      4,  3,  4,  3,  2,  3,  2,  3,  2, 3, 2, 3, 4, 3, 4,
  /* dummies 80- 88 */ UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX,UINT_MAX,
  /*  89-103 */     5,  4,  3,  4,  3,  2,  3,  2,  3, 2, 3, 4, 3, 4, 5,
  /* dummies104-112 */ UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX,UINT_MAX,
  /* 113-127 */     4,  5,  4,  3,  4,  3,  4,  3,  4, 3, 4, 3, 4, 5, 4,
  /* dummies128-136 */ UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX,UINT_MAX,
  /* 137-151 */     5,  4,  5,  4,  3,  4,  3,  4,  3, 4, 3, 4, 5, 4, 5,
  /* dummies152-160 */ UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX,UINT_MAX,
  /* 161-175 */     6,  5,  4,  5,  4,  5,  4,  5,  4, 5, 4, 5, 4, 5, 6
};

static unsigned int king_no_castling(square from, square to)
{
  unsigned int const diffcol = (unsigned int)abs(from%onerow - to%onerow);
  unsigned int const diffrow = (unsigned int)abs(from/onerow - to/onerow);

  return diffcol>diffrow ? diffcol : diffrow;
}

static unsigned int white_king(square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = king_no_castling(from_square,to_square);

  if (testcastling && from_square==square_e1)
  {
    if (TSTCASTLINGFLAGMASK(White,ra_cancastle&castling_flags_no_castling))
    {
      unsigned int const withcast = king_no_castling(square_c1,to_square);
      if (withcast<result)
        result = withcast;
    }
    if (TSTCASTLINGFLAGMASK(White,rh_cancastle&castling_flags_no_castling))
    {
      unsigned int const withcast = king_no_castling(square_g1,to_square);
      if (withcast<result)
        result = withcast;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int black_king(square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = king_no_castling(from_square,to_square);

  if (testcastling && from_square==square_e8)
  {
    if (TSTCASTLINGFLAGMASK(Black,ra_cancastle)==ra_cancastle)
    {
      unsigned int const withcast = king_no_castling(square_c8,to_square);
      if (withcast<result)
        result = withcast;
    }
    if (TSTCASTLINGFLAGMASK(Black,rh_cancastle)==rh_cancastle)
    {
      unsigned int const withcast = king_no_castling(square_g8,to_square);
      if (withcast<result)
        result = withcast;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int black_pawn_no_promotion(square from_square, square to_square)
{
  unsigned int result;
  int const diffcol = abs(from_square%onerow - to_square%onerow);
  int const diffrow = from_square/onerow - to_square/onerow;

  if (diffrow<diffcol)
    /* if diffrow<=0 then this test is true, since diffcol is always
     * non-negative
     */
    result = maxply+1;

  else if (TSTFLAG(sq_spec(from_square),BlPawnDoublestepSq) && diffrow-2 >= diffcol)
    result = diffrow-1;
  else
    result = diffrow;

  return result;
}

static unsigned int white_pawn_no_promotion(square from_square,
                                            square to_square)
{
  unsigned int result;
  int const diffcol = abs(from_square%onerow - to_square%onerow);
  int const diffrow = from_square/onerow - to_square/onerow;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  /* white pawn */
  if (-diffrow<diffcol)
    result = maxply+1;

  else  if (TSTFLAG(sq_spec(from_square),WhPawnDoublestepSq) && -diffrow-2 >= diffcol)
    /* double step */
    result = -diffrow-1;

  else
    result = -diffrow;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int queen(square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = CheckDir(Queen)[from_square-to_square]==0 ? 2 : 1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int rook(square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = CheckDir(Rook)[from_square-to_square]==0 ? 2 : 1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int bishop(square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (SquareCol(from_square)==SquareCol(to_square))
    result = CheckDir(Bishop)[from_square-to_square]==0 ? 2 : 1;
  else
    result = maxply+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int knight(square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = minimum_number_knight_moves[abs(from_square-to_square)];
  assert(result!=UINT_MAX);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int officer(piece_walk_type piece, square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceWalk(piece);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  assert(from_square!=to_square);
  switch (piece)
  {
    case Queen:
      result = queen(from_square,to_square);
      break;

    case Rook:
      result = rook(from_square,to_square);
      break;

    case Bishop:
      result = bishop(from_square,to_square);
      break;

    case Knight:
      result = knight(from_square,to_square);
      break;

    default:
      assert(0);
      result = UINT_MAX;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int white_pawn_promotion(square from_square,
                                         piece_walk_type to_piece,
                                         square to_square)
{
  unsigned int result = maxply+1;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceWalk(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  for (prom_square = square_a8; prom_square<=square_h8; prom_square += dir_right)
  {
    unsigned int const to_prom = white_pawn_no_promotion(from_square,
                                                         prom_square);
    unsigned int const from_prom = (prom_square==to_square
                                    ? 0
                                    : officer(to_piece,prom_square,to_square));
    unsigned int const total = to_prom+from_prom;
    if (total<result)
      result = total;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int black_pawn_promotion(square from_square,
                                         piece_walk_type to_piece,
                                         square to_square)
{
  unsigned int result = maxply+1;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceWalk(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  for (prom_square = square_a1; prom_square<=square_h1; prom_square += dir_right)
  {
    unsigned int const to_prom = black_pawn_no_promotion(from_square,
                                                         prom_square);
    unsigned int const from_prom = (prom_square==to_square
                                    ? 0
                                    : officer(to_piece,prom_square,to_square));
    unsigned int const total = to_prom+from_prom;
    if (total<result)
      result = total;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int from_to_different(Side side,
                                      piece_walk_type from_piece,
                                      square from_square,
                                      piece_walk_type to_piece,
                                      square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceWalk(from_piece);
  TraceSquare(from_square);
  TraceWalk(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  switch (from_piece)
  {
    case King:
      result = side==White ? white_king(from_square,to_square) : black_king(from_square,to_square);
      break;

    case Queen:
      result = queen(from_square,to_square);
      break;

    case Rook:
      result = rook(from_square,to_square);
      break;

    case Bishop:
      result = bishop(from_square,to_square);
      break;

    case Knight:
      result = knight(from_square,to_square);
      break;

    case Pawn:
      if (side==White)
      {
        if (from_piece==to_piece)
          result = white_pawn_no_promotion(from_square,to_square);
        else
          result = white_pawn_promotion(from_square,to_piece,to_square);
      }
      else
      {
        if (from_piece==to_piece)
          result = black_pawn_no_promotion(from_square,to_square);
        else
          result = black_pawn_promotion(from_square,to_piece,to_square);
      }
      break;

    default:
      assert(0);
      result = UINT_MAX;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int intelligent_count_nr_of_moves_from_to_no_check(Side side,
                                                            piece_walk_type from_piece,
                                                            square from_square,
                                                            piece_walk_type to_piece,
                                                            square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceWalk(from_piece);
  TraceSquare(from_square);
  TraceWalk(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square && from_piece==to_piece)
    result = 0;
  else
    result = from_to_different(side,from_piece,from_square,to_piece,to_square);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int black_promoted_pawn_to(square pawn_comes_from,
                                           square to_be_blocked)
{
  unsigned int result = maxply+1;

  TraceFunctionEntry(__func__);
  TraceSquare(pawn_comes_from);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int moves = pawn_comes_from/onerow - nr_of_slack_rows_below_board;

    if (TSTFLAG(sq_spec(pawn_comes_from),BlPawnDoublestepSq))
      --moves;

    assert(moves<=5);

    if (!TSTFLAG(sq_spec(to_be_blocked),BlPromSq))
      /* not promotion square -- 1 move necessary to get there */
      ++moves;

    if (reserve[curr_reserve].nr_remaining_moves[Black]>=moves)
    {
      piece_walk_type pp;
      for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      {
        unsigned int const time = black_pawn_promotion(pawn_comes_from,
                                                       pp,
                                                       to_be_blocked);
        if (time<result)
          result = time;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int estimate_min_nr_black_moves_to(square to_square)
{
  unsigned int result = maxply+1;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
    {
      square const from_square = black[i].diagram_square;
      if (from_square==to_square)
      {
        result = 0;
        break;
      }
      else
      {
        piece_walk_type const type = black[i].type;
        if (type==Pawn)
        {
          if (!TSTFLAG(sq_spec(to_square),BlPromSq))
          {
            unsigned int const time = black_pawn_no_promotion(from_square,
                                                              to_square);
            if (time<result)
              result = time;
          }

          {
            unsigned int const time_prom = black_promoted_pawn_to(from_square,
                                                                  to_square);
            if (time_prom<result)
              result = time_prom;
          }
        }
        else
        {
          unsigned int const time = officer(type,from_square,to_square);
          if (time<result)
            result = time;
        }
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_of_moves_same_piece_same_square_checking(piece_walk_type piece,
                                                                      square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceWalk(piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  switch (piece)
   {
     case Pawn:
       result = maxply+1;
       break;

     case Knight:
       result = 2;
       break;

     default:
       /* it's a rider */
       if (move_diff_code[abs(being_solved.king_square[Black]-to_square)]<3)
         result = 2;
       else
         result = 0;
       break;
   }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int intelligent_count_nr_of_moves_from_to_checking(Side side,
                                                            piece_walk_type from_piece,
                                                            square from_square,
                                                            piece_walk_type to_piece,
                                                            square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceWalk(from_piece);
  TraceSquare(from_square);
  TraceWalk(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square && from_piece==to_piece)
    result = count_nr_of_moves_same_piece_same_square_checking(from_piece,
                                                               to_square);
  else
    result = from_to_different(side,from_piece,from_square,to_piece,to_square);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Count the number of moves required for promotiong a white pawn
 * @param from_square start square of the pawn
 * @return number of moves required
 */
unsigned int intelligent_count_moves_to_white_promotion(square from_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceFunctionParamListEnd();

  if (MovesLeft[White]==5
      && from_square<=square_h2
      && (TSTFLAG(being_solved.spec[from_square+dir_up],White)
          || TSTFLAG(being_solved.spec[from_square+2*dir_up],White)))
    /* pawn can't reach the promotion square */
    result = maxply+1;
  else
  {
    unsigned int const rank = from_square/onerow - nr_of_slack_rows_below_board;
    result = 7-rank;

    if (result==6)
    {
      --result; /* double step! */

      if (MovesLeft[White]<=6)
      {
        /* immediate double step is required if this pawn is to promote */
        if (get_walk_of_piece_on_square(from_square+dir_up)==Pawn && TSTFLAG(being_solved.spec[from_square+dir_up],Black)
            && (get_walk_of_piece_on_square(from_square+dir_left)==King || !TSTFLAG(being_solved.spec[from_square+dir_left],White))
            && (get_walk_of_piece_on_square(from_square+dir_right)==King || !TSTFLAG(being_solved.spec[from_square+dir_right],White)))
          /* Black can't immediately get rid of block on 3rd row
           * -> no immediate double step possible */
          ++result;

        else if (get_walk_of_piece_on_square(from_square+2*dir_up)==Pawn && TSTFLAG(being_solved.spec[from_square+2*dir_up],Black)
                 && (get_walk_of_piece_on_square(from_square+dir_up+dir_left)==King || !TSTFLAG(being_solved.spec[from_square+dir_up+dir_left],White))
                 && (get_walk_of_piece_on_square(from_square+dir_up+dir_right)==King || !TSTFLAG(being_solved.spec[from_square+dir_up+dir_right],White))
                 && !en_passant_is_capture_possible_to(White,from_square+dir_up+dir_left)
                 && !en_passant_is_capture_possible_to(White,from_square+dir_up+dir_right))
          /* Black can't immediately get rid of block on 4th row
           * -> no immediate double step possible */
          ++result;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* A rough check whether it is worth thinking about promotions
 * @param index index of white pawn
 * @param to_square to be reached by the promotee
 * @return true iff to_square is theoretically reachable
 */
boolean intelligent_can_promoted_white_pawn_theoretically_move_to(unsigned int index,
                                                                  square to_square)
{
  unsigned int min_nr_moves_by_p = moves_to_white_prom[index];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (!TSTFLAG(sq_spec(to_square),WhPromSq))
    ++min_nr_moves_by_p;

  result = reserve[curr_reserve].nr_remaining_moves[White]>=min_nr_moves_by_p;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* A rough check whether it is worth thinking about promotions
 * @param index index of white pawn
 * @param to_square to be reached by the promotee
 * @return true iff to_square is theoretically reachable
 */
boolean intelligent_can_promoted_black_pawn_theoretically_move_to(unsigned int index,
                                                                  square to_square)
{
  boolean result;
  square const placed_from = black[index].diagram_square;
  unsigned int min_nr_moves_by_p = placed_from/onerow - nr_of_slack_rows_below_board;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec(placed_from),BlPawnDoublestepSq))
    --min_nr_moves_by_p;

  assert(min_nr_moves_by_p<=5);

  if (to_square>=square_a2)
    /* square is not on 1st rank -- 1 move necessary to get there */
    ++min_nr_moves_by_p;

  result = min_nr_moves_by_p<=reserve[curr_reserve].nr_remaining_moves[Black];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the moves and masses reservation system
 * @param nr_remaining_white_moves
 * @param nr_remaining_black_moves
 * @param nr_unused_white_masses
 * @param nr_unused_black_masses
 */
void intelligent_init_reservations(unsigned int nr_remaining_white_moves,
                                   unsigned int nr_remaining_black_moves,
                                   unsigned int nr_unused_white_masses,
                                   unsigned int nr_unused_black_masses)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_unused_white_masses);
  TraceFunctionParam("%u",nr_unused_black_masses);
  TraceFunctionParamListEnd();

  assert(curr_reserve==0);
  reserve[0].nr_remaining_moves[White] = nr_remaining_white_moves;
  reserve[0].nr_remaining_moves[Black] = nr_remaining_black_moves;
  reserve[0].nr_unused_masses[White] = nr_unused_white_masses;
  reserve[0].nr_unused_masses[Black] = nr_unused_black_masses;
  reserve[0].usage = piece_is_king;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo a reservation
 */
static void push_reserve(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++curr_reserve;
  assert(curr_reserve<nr_squares_on_board);
  reserve[curr_reserve] = reserve[curr_reserve-1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo a reservation
 */
void intelligent_unreserve(void)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(curr_reserve>0);
  --curr_reserve;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Roughly test whether Black can possibly block all flights; if yes, reserve
 * the necessary black masses
 * @param flights flights to be blocked
 * @param nr_flights length of flights
 * @return true if masses have been reserved
 */
boolean intelligent_reserve_black_masses_for_blocks(square const flights[],
                                                    unsigned int nr_flights)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_flights);
  TraceFunctionParamListEnd();

  if (nr_flights<=reserve[curr_reserve].nr_unused_masses[Black])
  {
    unsigned int nr_of_moves = 0;
    unsigned int i;
    for (i = 0; i<nr_flights && nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[Black]; ++i)
      nr_of_moves += estimate_min_nr_black_moves_to(flights[i]);

    if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[Black])
    {
      push_reserve();
      reserve[curr_reserve].nr_unused_masses[Black] -= nr_flights;
      reserve[curr_reserve].usage = piece_blocks;
      TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
      TraceEOL();
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Retrieve the number of reservable masses for one or both sides
 * @param side Whose masses? Pass no_side to get both sides' masses.
 * @return number of reservable masses
 */
unsigned int intelligent_get_nr_reservable_masses(Side side)
{
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if (side!=White)
    result += reserve[curr_reserve].nr_unused_masses[Black];
  if (side!=Black)
    result += reserve[curr_reserve].nr_unused_masses[White];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Retrieve the number of remaining moves for a side
 * @param side Whose moves
 * @return number of remaining moves
 */
unsigned int intelligent_get_nr_remaining_moves(Side side)
{
  unsigned int const result = reserve[curr_reserve].nr_remaining_moves[side];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Test whether there are available masses for a side
 * @param side whose masses to reserve
 * @param nr_of_masses number of masses
 * @param usage of the reserved masses
 * @return true iff nr_of_masses are available
 */
boolean intelligent_reserve_masses(Side side,
                                   unsigned int nr_of_masses,
                                   piece_usage usage)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParam("%u",nr_of_masses);
  TraceFunctionParamListEnd();

  if (reserve[curr_reserve].nr_unused_masses[side]>=nr_of_masses)
  {
    push_reserve();
    reserve[curr_reserve].nr_unused_masses[side] -= nr_of_masses;
    reserve[curr_reserve].usage = usage;
    TraceValue("%u",reserve[curr_reserve].nr_unused_masses[side]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Test whether a white pinner is available
 * @return true iff a white pinner is available
 */
boolean intelligent_reserve_pinner(void)
{
  boolean result;
  unsigned int nr_pinners = reserve[curr_reserve].nr_unused_masses[White];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
    --nr_pinners;

  if (nr_pinners>=1)
  {
    push_reserve();
    --reserve[curr_reserve].nr_unused_masses[White];
    reserve[curr_reserve].usage = piece_pins;
    TraceValue("%u",reserve[curr_reserve].nr_unused_masses[White]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking white sequence of moves by the same black pawn
 * without promotion is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_black_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square)
{
  boolean result;
  unsigned int nr_of_moves;
  unsigned int const diffcol = (unsigned int)abs(from_square%onerow - to_square%onerow);
  unsigned int nr_capturable_white_masses = reserve[curr_reserve].nr_unused_masses[White];

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  nr_of_moves = black_pawn_no_promotion(from_square,to_square);

  if (white[index_of_king].usage==piece_is_unused)
    --nr_capturable_white_masses;

  if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[Black]
      && diffcol<=nr_capturable_white_masses)
  {
    push_reserve();
    reserve[curr_reserve].nr_remaining_moves[Black] -= nr_of_moves;
    reserve[curr_reserve].nr_unused_masses[White] -= diffcol;
    reserve[curr_reserve].usage = piece_is_captured;
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[Black]);
    TraceValue("%u",reserve[curr_reserve].nr_unused_masses[White]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking white sequence of moves by the same white pawn
 * without promotion is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_white_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square)
{
  boolean result;
  unsigned int nr_of_moves;
  unsigned int const diffcol = (unsigned int)abs(from_square%onerow - to_square%onerow);

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  nr_of_moves = white_pawn_no_promotion(from_square,to_square);

  if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[White]
      && diffcol<=reserve[curr_reserve].nr_unused_masses[Black])
  {
    push_reserve();
    reserve[curr_reserve].nr_remaining_moves[White] -= nr_of_moves;
    reserve[curr_reserve].nr_unused_masses[Black] -= diffcol;
    reserve[curr_reserve].usage = piece_is_captured;
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
    TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking white sequence of moves by the same officer is
 * still possible.
 * @param from_square from
 * @param checker_type type of officer
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_white_officer_moves_from_to_checking(square from_square,
                                                                 piece_walk_type checker_type,
                                                                 square to_square)
{
  boolean result;
  unsigned int nr_of_moves;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceWalk(checker_type);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square)
    nr_of_moves = count_nr_of_moves_same_piece_same_square_checking(checker_type,
                                                                    to_square);
  else
    nr_of_moves = officer(checker_type,from_square,to_square);

  if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[White])
  {
    push_reserve();
    reserve[curr_reserve].nr_remaining_moves[White] -= nr_of_moves;
    reserve[curr_reserve].usage = piece_gives_check;
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking white sequence of moves by the same pawn is
 * still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_white_pawn_moves_from_to_checking(square from_square,
                                                              square to_square)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square!=to_square)
  {
    unsigned int const diffcol = (unsigned int)abs(from_square%onerow - to_square%onerow);
    if (diffcol<=reserve[curr_reserve].nr_unused_masses[Black])
    {
      unsigned int const nr_of_moves = white_pawn_no_promotion(from_square,
                                                               to_square);
      if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[White])
      {
        push_reserve();
        reserve[curr_reserve].nr_unused_masses[Black] -= diffcol;
        reserve[curr_reserve].nr_remaining_moves[White] -= nr_of_moves;
        reserve[curr_reserve].usage = piece_is_captured;
        TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
        TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
        TraceEOL();
        result = true;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean reserve_promoting_black_pawn_moves_from_to(unsigned int min_nr_of_moves,
                                                          unsigned int min_diffcol)
{
  boolean result;
  unsigned int nr_capturable_masses = reserve[curr_reserve].nr_unused_masses[White];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_nr_of_moves);
  TraceFunctionParam("%u",min_diffcol);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
    --nr_capturable_masses;

  if (min_nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[Black]
      && min_diffcol<=nr_capturable_masses)
  {
    push_reserve();
    reserve[curr_reserve].nr_remaining_moves[Black] -= min_nr_of_moves;
    reserve[curr_reserve].nr_unused_masses[White] -= min_diffcol;
    reserve[curr_reserve].usage = piece_is_captured;
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[Black]);
    TraceValue("%u",reserve[curr_reserve].nr_unused_masses[White]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean reserve_promoting_white_pawn_moves_from_to(unsigned int min_nr_of_moves,
                                                          unsigned int min_diffcol)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_nr_of_moves);
  TraceFunctionParam("%u",min_diffcol);
  TraceFunctionParamListEnd();

  if (min_nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[White]
      && min_diffcol<=reserve[curr_reserve].nr_unused_masses[Black])
  {
    push_reserve();
    reserve[curr_reserve].nr_remaining_moves[White] -= min_nr_of_moves;
    reserve[curr_reserve].nr_unused_masses[Black] -= min_diffcol;
    reserve[curr_reserve].usage = piece_is_captured;
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
    TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking sequence of moves by the same white pawn
 * including its promotion is still possible.
 * @param from_square from
 * @param promotee_type type of piece that the pawn promotes to
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_promoting_white_pawn_moves_from_to(square from_square,
                                                               piece_walk_type promotee_type,
                                                               square to_square)
{
  boolean result;
  unsigned int min_nr_of_moves = maxply+1;
  unsigned int min_diffcol = nr_files_on_board;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  for (prom_square = square_a8; prom_square<=square_h8; prom_square += dir_right)
  {
    unsigned int nr_of_moves = white_pawn_no_promotion(from_square,prom_square);
    if (prom_square!=to_square)
      nr_of_moves += officer(promotee_type,prom_square,to_square);

    if (nr_of_moves<min_nr_of_moves)
      min_nr_of_moves = nr_of_moves;

    if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[White])
    {
      unsigned int const diffcol = (unsigned int)abs(from_square%onerow - prom_square%onerow);
      if (diffcol<min_diffcol)
        min_diffcol = diffcol;
    }
  }

  result = reserve_promoting_white_pawn_moves_from_to(min_nr_of_moves,min_diffcol);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking sequence of moves by the same black pawn
 * including its promotion is still possible.
 * @param from_square from
 * @param promotee_type type of piece that the pawn promotes to
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_promoting_black_pawn_moves_from_to(square from_square,
                                                               piece_walk_type promotee_type,
                                                               square to_square)
{
  boolean result;
  unsigned int min_nr_of_moves = maxply+1;
  unsigned int min_diffcol = nr_files_on_board;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  for (prom_square = square_a1; prom_square<=square_h1; prom_square += dir_right)
  {
    unsigned int nr_of_moves = black_pawn_no_promotion(from_square,prom_square);
    if (prom_square!=to_square)
      nr_of_moves += officer(promotee_type,prom_square,to_square);

    if (nr_of_moves<min_nr_of_moves)
      min_nr_of_moves = nr_of_moves;

    if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[Black])
    {
      unsigned int const diffcol = (unsigned int)abs(from_square%onerow - prom_square%onerow);
      if (diffcol<min_diffcol)
        min_diffcol = diffcol;
    }
  }

  result = reserve_promoting_black_pawn_moves_from_to(min_nr_of_moves,min_diffcol);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean reserve_king_moves_from_to(Side side, unsigned int nr_of_moves)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParam("%u",nr_of_moves);
  TraceFunctionParamListEnd();

  if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[side])
  {
    push_reserve();
    reserve[curr_reserve].nr_remaining_moves[side] -= nr_of_moves;
    reserve[curr_reserve].usage = piece_is_king;
    TraceEnumerator(Side,side);
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[side]);
    TraceEOL();
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific white king move sequence is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_white_king_moves_from_to(square from_square,
                                                     square to_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = reserve_king_moves_from_to(White,white_king(from_square,to_square));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific black king move sequence is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_black_king_moves_from_to(square from_square,
                                                     square to_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = reserve_king_moves_from_to(Black,black_king(from_square,to_square));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a white officer can be the front piece of a battery double check
 * using a specific route
 * @param from_square from
 * @param via departure square of the double checking move
 * @param checker_type type of officer
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_officer(square from_square,
                                                   square via,
                                                   piece_walk_type checker_type,
                                                   square to_square)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(via);
  TraceWalk(checker_type);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  assert(via!=to_square);
  if (officer(checker_type,via,to_square)==1)
  {
    unsigned int const nr_of_moves = (from_square==via
                                      ? 0
                                     : officer(checker_type,from_square,via));
    if (nr_of_moves+1<=reserve[curr_reserve].nr_remaining_moves[White])
    {
      push_reserve();
      reserve[curr_reserve].nr_remaining_moves[White] -= nr_of_moves+1;
      reserve[curr_reserve].usage = piece_gives_check;
      TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
      TraceEOL();
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if an officer can reach some square in a sequence of moves
 * @param side officer's side
 * @param from_square from
 * @param officer_type type of officer
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_officer_moves_from_to(Side side,
                                                  square from_square,
                                                  piece_walk_type officer_type,
                                                  square to_square)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceWalk(officer_type);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  {
    unsigned int const nr_of_moves = (from_square==to_square
                                      ? 0
                                      : officer(officer_type,from_square,to_square));
    if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[side])
    {
      push_reserve();
      reserve[curr_reserve].nr_remaining_moves[side] -= nr_of_moves;
      reserve[curr_reserve].usage = reserve[curr_reserve-1].usage;
      TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[side]);
      TraceEOL();
      result = true;
    }
    else
      result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a white pawn can be the front piece of a battery double check
 * using a specific route, where the last move is a capture
 * @param from_square from
 * @param via departure square of the double checking move
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_pawn_with_capture(square from_square,
                                                             square via,
                                                             square to_square)
{
  boolean result = false;
  unsigned int const diffcol = (unsigned int)abs(from_square%onerow - via%onerow);

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(via);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (diffcol<=reserve[curr_reserve].nr_unused_masses[Black])
  {
    unsigned int const to_via = (from_square==via
                                 ? 0
                                 : white_pawn_no_promotion(from_square,via));
    if (to_via+1<=reserve[curr_reserve].nr_remaining_moves[White])
    {
      unsigned int const time_capturee = estimate_min_nr_black_moves_to(to_square);
      if (time_capturee<=reserve[curr_reserve].nr_remaining_moves[Black])
      {
        push_reserve();
        reserve[curr_reserve].nr_remaining_moves[White] -= to_via+1;
        reserve[curr_reserve].nr_unused_masses[Black] -= diffcol+1;
        reserve[curr_reserve].nr_remaining_moves[Black] -= time_capturee;
        reserve[curr_reserve].usage = piece_is_captured;
        TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
        TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[Black]);
        TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
        TraceEOL();
        result = true;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a white pawn can be the front piece of a battery double check
 * using a specific route, where the last move is not a capture
 * @param from_square from
 * @param via departure square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_pawn_without_capture(square from_square,
                                                                square via)
{
  boolean result = false;
  unsigned int const diffcol = (unsigned int)abs(from_square%onerow - via%onerow);

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  if (diffcol<=reserve[curr_reserve].nr_unused_masses[Black])
  {
    unsigned int const to_via = (from_square==via
                                 ? 0
                                 : white_pawn_no_promotion(from_square,via));
    if (to_via+1<=reserve[curr_reserve].nr_remaining_moves[White])
    {
      push_reserve();
      reserve[curr_reserve].nr_remaining_moves[White] -= to_via+1;
      reserve[curr_reserve].nr_unused_masses[Black] -= diffcol;
      reserve[curr_reserve].usage = piece_is_captured;
      TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
      TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
      TraceEOL();
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if the front check of a double check can be given by pomotee
 * @param from_square departure square of pawn
 * @param promotee_type type of piece that the pawn promotes to
 * @param via departure square of mating move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_promotee(square from_square,
                                                    piece_walk_type promotee_type,
                                                    square via)
{
  boolean result;
  unsigned int min_nr_of_moves = maxply+1;
  unsigned int min_diffcol = nr_files_on_board;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceWalk(promotee_type);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  for (prom_square = square_a8; prom_square<=square_h8; prom_square += dir_right)
  {
    unsigned int nr_of_moves = white_pawn_no_promotion(from_square,prom_square);
    if (prom_square!=via)
      nr_of_moves += officer(promotee_type,prom_square,via);

    if (nr_of_moves<min_nr_of_moves)
      min_nr_of_moves = nr_of_moves;

    if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[White])
    {
      unsigned int const diffcol = (unsigned int)abs(from_square%onerow - prom_square%onerow);
      if (diffcol<min_diffcol)
        min_diffcol = diffcol;
    }
  }

  result = reserve_promoting_white_pawn_moves_from_to(min_nr_of_moves+1,min_diffcol);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a white pawn can be the front piece of a battery double check
 * using a specific route, where the last move is a capture
 * @param from_square from
 * @param via departure square of the double checking move
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_double_check_by_enpassant_capture(square from_square,
                                                              square via)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  if (reserve[curr_reserve].nr_remaining_moves[Black]>=1)
  {
    unsigned int const to_via = (from_square==via
                                 ? 0
                                 : white_pawn_no_promotion(from_square,via));
    if (to_via+1<=reserve[curr_reserve].nr_remaining_moves[White])
    {
      unsigned int const diffcol = (unsigned int)abs(from_square%onerow - via%onerow);
      if (diffcol+1<=reserve[curr_reserve].nr_unused_masses[Black])
      {
        push_reserve();
        reserve[curr_reserve].nr_remaining_moves[Black] -= 1;
        reserve[curr_reserve].nr_unused_masses[Black] -= diffcol+1;
        reserve[curr_reserve].nr_remaining_moves[White] -= to_via+1;
        reserve[curr_reserve].usage = piece_is_captured;
        TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
        TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
        TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[Black]);
        TraceEOL();
        result = true;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Dump the move and mass allocations for debugging
 */
void intelligent_dump_reservations(void)
{
  unsigned int i = 0;
  fputs("reservations:\n",stdout);
  for (i = 0; i<=curr_reserve; ++i)
  {
    fprintf(stdout,"%u  remaining moves:w:%u/b:%u  unused masses:w:%u/b:%u  mass usage:%s\n",
           i,
           reserve[i].nr_remaining_moves[White],
           reserve[i].nr_remaining_moves[Black],
           reserve[i].nr_unused_masses[White],
           reserve[i].nr_unused_masses[Black],
           piece_usage_names[reserve[i].usage]);
  }
}
