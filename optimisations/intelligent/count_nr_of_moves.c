#include "optimisations/intelligent/count_nr_of_moves.h"
#include "pydata.h"
#include "pyproof.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/moves_left.h"
#include "solving/castling.h"
#include "solving/en_passant.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    unsigned int nr_remaining_moves[nr_sides];
    unsigned int nr_unused_masses[nr_sides];
} reserve_elmt_type;

static reserve_elmt_type reserve[nr_squares_on_board];

static unsigned int curr_reserve;

static unsigned int king_no_castling(square from, square to)
{
  unsigned int const diffcol = abs(from%onerow - to%onerow);
  unsigned int const diffrow = abs(from/onerow - to/onerow);

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
    if (TSTCASTLINGFLAGMASK(nbply,White,ra_cancastle&castling_flag[castlings_flags_no_castling]))
    {
      unsigned int const withcast = king_no_castling(square_c1,to_square);
      if (withcast<result)
        result = withcast;
    }
    if (TSTCASTLINGFLAGMASK(nbply,White,rh_cancastle&castling_flag[castlings_flags_no_castling]))
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
    if (TSTCASTLINGFLAGMASK(nbply,Black,ra_cancastle&castling_flag[castlings_flags_no_castling]))
    {
      unsigned int const withcast = king_no_castling(square_c8,to_square);
      if (withcast<result)
        result = withcast;
    }
    if (TSTCASTLINGFLAGMASK(nbply,Black,rh_cancastle&castling_flag[castlings_flags_no_castling]))
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

  else if (from_square>=square_a7 && diffrow-2 >= diffcol)
    /* double step */
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

  else  if (from_square<=square_h2 && -diffrow-2 >= diffcol)
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

  result = CheckDir[Queen][from_square-to_square]==0 ? 2 : 1;

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

  result = CheckDir[Rook][from_square-to_square]==0 ? 2 : 1;

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
    result = CheckDir[Bishop][from_square-to_square]==0 ? 2 : 1;
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

  result = ProofKnightMoves[abs(from_square-to_square)];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int officer(piece piece, square from_square, square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(piece);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  assert(from_square!=to_square);
  switch (abs(piece))
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
                                         piece to_piece,
                                         square to_square)
{
  unsigned int result = maxply+1;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TracePiece(to_piece);
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
                                         piece to_piece,
                                         square to_square)
{
  unsigned int result = maxply+1;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TracePiece(to_piece);
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

static unsigned int from_to_different(piece from_piece,
                                      square from_square,
                                      piece to_piece,
                                      square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(from_piece);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  switch (from_piece)
  {
    case roib:
      result = white_king(from_square,to_square);
      break;

    case roin:
      result = black_king(from_square,to_square);
      break;

    case db:
    case dn:
      result = queen(from_square,to_square);
      break;

    case tb:
    case tn:
      result = rook(from_square,to_square);
      break;

    case fb:
    case fn:
      result = bishop(from_square,to_square);
      break;

    case cb:
    case cn:
      result = knight(from_square,to_square);
      break;

    case pb:
      if (from_piece==to_piece)
        result = white_pawn_no_promotion(from_square,to_square);
      else
        result = white_pawn_promotion(from_square,to_piece,to_square);
      break;

    case pn:
      if (from_piece==to_piece)
        result = black_pawn_no_promotion(from_square,to_square);
      else
        result = black_pawn_promotion(from_square,to_piece,to_square);
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

unsigned int intelligent_count_nr_of_moves_from_to_no_check(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(from_piece);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square && from_piece==to_piece)
    result = 0;
  else
    result = from_to_different(from_piece,from_square,to_piece,to_square);

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
    unsigned int moves = (pawn_comes_from>=square_a7
                          ? 5
                          : pawn_comes_from/onerow - nr_of_slack_rows_below_board);
    assert(moves<=5);

    if (to_be_blocked>=square_a2)
      /* square is not on 8th rank -- 1 move necessary to get there */
      ++moves;

    if (reserve[curr_reserve].nr_remaining_moves[Black]>=moves)
    {
      PieNam pp;
      for (pp = getprompiece[Empty]; pp!=Empty; pp = getprompiece[pp])
      {
        unsigned int const time = black_pawn_promotion(pawn_comes_from,
                                                       -pp,
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
        piece const type = black[i].type;
        if (type==pn)
        {
          if (to_square>=square_a2)
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

static unsigned int count_nr_of_moves_same_piece_same_square_checking(piece piece,
                                                                      square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  switch (piece)
   {
     case pb:
       result = maxply+1;
       break;

     case cb:
       result = 2;
       break;

     default:
       /* it's a rider */
       if (move_diff_code[abs(king_square[Black]-to_square)]<3)
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

unsigned int intelligent_count_nr_of_moves_from_to_checking(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(from_piece);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square && from_piece==to_piece)
    result = count_nr_of_moves_same_piece_same_square_checking(from_piece,
                                                               to_square);
  else
    result = from_to_different(from_piece,from_square,to_piece,to_square);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int intelligent_count_moves_to_white_promotion(square from_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceFunctionParamListEnd();

  if (MovesLeft[White]==5
      && from_square<=square_h2
      && (e[from_square+dir_up]>vide || e[from_square+2*dir_up]>vide))
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
        if (e[from_square+dir_up]==pn
            && (e[from_square+dir_left]<=roib
                && e[from_square+dir_right]<=roib))
          /* Black can't immediately get rid of block on 3th row
           * -> no immediate double step possible */
          ++result;

        else if (e[from_square+2*dir_up]==pn
                 && (e[from_square+dir_up+dir_left]<=roib
                     && e[from_square+dir_up+dir_right]<=roib
                     && ep[1]!=from_square+dir_up+dir_left
                     && ep[1]!=from_square+dir_up+dir_right))
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
  unsigned int const min_nr_moves_by_p = (to_square<=square_h7
                                          ? moves_to_white_prom[index]+1
                                          : moves_to_white_prom[index]);
  boolean const result = reserve[curr_reserve].nr_remaining_moves[White]>=min_nr_moves_by_p;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

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
  unsigned int min_nr_moves_by_p = (placed_from>=square_a7
                                    ? 5
                                   : placed_from/onerow - nr_of_slack_rows_below_board);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

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
      TraceValue("%u\n",reserve[curr_reserve].nr_unused_masses[Black]);
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
  TraceEnumerator(Side,side,"");
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
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Test whether there are available masses for a side
 * @param side whose masses to reserve
 * @param nr_of_masses number of masses
 * @return true iff nr_of_masses are available
 */
boolean intelligent_reserve_masses(Side side, unsigned int nr_of_masses)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",nr_of_masses);
  TraceFunctionParamListEnd();

  if (reserve[curr_reserve].nr_unused_masses[side]>=nr_of_masses)
  {
    push_reserve();
    reserve[curr_reserve].nr_unused_masses[side] -= nr_of_masses;
    TraceValue("%u\n",reserve[curr_reserve].nr_unused_masses[side]);
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
    TraceValue("%u\n",reserve[curr_reserve].nr_unused_masses[White]);
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
  unsigned int const diffcol = abs(from_square%onerow - to_square%onerow);
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
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[Black]);
    TraceValue("%u\n",reserve[curr_reserve].nr_unused_masses[White]);
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
  unsigned int const diffcol = abs(from_square%onerow - to_square%onerow);

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
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
    TraceValue("%u\n",reserve[curr_reserve].nr_unused_masses[Black]);
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
                                                                 piece checker_type,
                                                                 square to_square)
{
  boolean result;
  unsigned int nr_of_moves;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TracePiece(checker_type);
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
    TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[White]);
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
    unsigned int const diffcol = abs(from_square%onerow - to_square%onerow);
    if (diffcol<=reserve[curr_reserve].nr_unused_masses[Black])
    {
      unsigned int const nr_of_moves = white_pawn_no_promotion(from_square,
                                                               to_square);
      if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[White])
      {
        push_reserve();
        reserve[curr_reserve].nr_unused_masses[Black] -= diffcol;
        reserve[curr_reserve].nr_remaining_moves[White] -= nr_of_moves;
        TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
        TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[White]);
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
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[Black]);
    TraceValue("%u\n",reserve[curr_reserve].nr_unused_masses[White]);
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
    TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
    TraceValue("%u\n",reserve[curr_reserve].nr_unused_masses[Black]);
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
                                                               piece promotee_type,
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
      unsigned int const diffcol = abs(from_square%onerow - prom_square%onerow);
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
                                                               piece promotee_type,
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
      unsigned int const diffcol = abs(from_square%onerow - prom_square%onerow);
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
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",nr_of_moves);
  TraceFunctionParamListEnd();

  if (nr_of_moves<=reserve[curr_reserve].nr_remaining_moves[side])
  {
    push_reserve();
    reserve[curr_reserve].nr_remaining_moves[side] -= nr_of_moves;
    TraceEnumerator(Side,side,"");
    TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[side]);
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
                                                   piece checker_type,
                                                   square to_square)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(via);
  TracePiece(checker_type);
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
      TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[White]);
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if an officer can reach some square in a sequence of moves
 * @param from_square from
 * @param officer_type type of officer
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_officer_moves_from_to(square from_square,
                                                  piece officer_type,
                                                  square to_square)
{
  Side const side = officer_type>obs ? White : Black;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TracePiece(officer_type);
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
      TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[side]);
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
  unsigned int const diffcol = abs(from_square%onerow - via%onerow);

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
        TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
        TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[Black]);
        TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[White]);
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
  unsigned int const diffcol = abs(from_square%onerow - via%onerow);

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
      TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
      TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[White]);
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
                                                    piece promotee_type,
                                                    square via)
{
  boolean result;
  unsigned int min_nr_of_moves = maxply+1;
  unsigned int min_diffcol = nr_files_on_board;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TracePiece(promotee_type);
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
      unsigned int const diffcol = abs(from_square%onerow - prom_square%onerow);
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
      unsigned int const diffcol = abs(from_square%onerow - via%onerow);
      if (diffcol+1<=reserve[curr_reserve].nr_unused_masses[Black])
      {
        push_reserve();
        reserve[curr_reserve].nr_remaining_moves[Black] -= 1;
        reserve[curr_reserve].nr_unused_masses[Black] -= diffcol+1;
        reserve[curr_reserve].nr_remaining_moves[White] -= to_via+1;
        TraceValue("%u",reserve[curr_reserve].nr_unused_masses[Black]);
        TraceValue("%u",reserve[curr_reserve].nr_remaining_moves[White]);
        TraceValue("%u\n",reserve[curr_reserve].nr_remaining_moves[Black]);
        result = true;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
