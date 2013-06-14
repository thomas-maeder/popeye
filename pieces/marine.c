#include "pieces/marine.h"
#include "pieces/pawns/pawns.h"
#include "pieces/pawns/pawn.h"
#include "solving/castling.h"
#include "pydata.h"
#include "py4.h"
#include "debugging/trace.h"

#include <assert.h>

/* Generate moves for a marine rider
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_rider_generate_moves(Side side,
                                 square sq_departure,
                                 vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k = kbeg; k<=kend; k++)
  {
    square const sq_capture = generate_moves_on_line_segment(sq_departure,
                                                             sq_departure,
                                                             k);
    generate_locust_capture(sq_departure,sq_capture,k,side);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine knight
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void marine_knight_generate_moves(Side side, square sq_departure)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    square sq_arrival = sq_departure+vec[k];
    if (is_square_empty(sq_arrival))
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (piece_belongs_to_opponent(sq_arrival,side))
    {
      square const sq_capture = sq_arrival;
      sq_arrival += vec[k];
      if (is_square_empty(sq_arrival))
        empile(sq_departure,sq_arrival,sq_capture);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a poseidon
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void poseidon_generate_moves(Side side, square sq_departure)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k = vec_queen_start; k<=vec_queen_end; ++k)
  {
    square sq_arrival = sq_departure+vec[k];
    if (is_square_empty(sq_arrival))
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (piece_belongs_to_opponent(sq_arrival,side))
    {
      square const sq_capture = sq_arrival;
      sq_arrival += vec[k];
      if (is_square_empty(sq_arrival))
        empile(sq_departure,sq_arrival,sq_capture);
    }
  }

  if (castling_supported)
    generate_castling(side);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void marine_pawn_generate_capture(Side side, square sq_departure, int dir)
{
  square const sq_capture = sq_departure+dir;
  square const sq_arrival = sq_capture+dir;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  if (is_square_empty(sq_arrival))
  {
    if (piece_belongs_to_opponent(sq_capture,side))
      empile(sq_departure,sq_arrival,sq_capture);
    else
      pawns_generate_ep_capture_move(side,sq_departure,sq_arrival,sq_capture);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine pawn
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void marine_pawn_generate_moves(Side side, square sq_departure)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(side,sq_departure);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (no_capture_length>0)
  {
    int const dir_forward = side==White ? dir_up : dir_down;
    pawns_generate_nocapture_moves(sq_departure,dir_forward,no_capture_length);
    marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_right);
    marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_left);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine ship (Triton + marine pawn)
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_ship_generate_moves(Side side,
                                square  sq_departure,
                                vec_index_type kbeg, vec_index_type kend)
{
  int const dir_forward = side==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  marine_rider_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
  marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_right);
  marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_left);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
