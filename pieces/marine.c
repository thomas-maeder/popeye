#include "pieces/marine.h"
#include "pieces/pawns/pawns.h"
#include "pieces/pawns/pawn.h"
#include "solving/castling.h"
#include "pydata.h"
#include "py4.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Generate moves for a marine rider
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_rider_generate_moves(Side side,
                                 square sq_departure,
                                 numvec kbeg, numvec kend)
{
  numvec k;

  for (k = kbeg; k<=kend; k++)
  {
    square const sq_capture = generate_moves_on_line_segment(sq_departure,
                                                             sq_departure,
                                                             k);
    generate_locust_capture(sq_departure,sq_capture,k,side);
  }
}

/* Generate moves for a marine knight
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void marine_knight_generate_moves(Side side, square sq_departure)
{
  numvec  k;
  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    square sq_arrival = sq_departure+vec[k];
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (piece_belongs_to_opponent(sq_arrival,side))
    {
      square const sq_capture = sq_arrival;
      sq_arrival += vec[k];
      if (e[sq_arrival]==vide)
        empile(sq_departure,sq_arrival,sq_capture);
    }
  }
}

/* Generate moves for a poseidon
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void poseidon_generate_moves(Side side, square sq_departure)
{
  numvec  k;
  for (k = vec_queen_start; k<=vec_queen_end; ++k)
  {
    square sq_arrival = sq_departure+vec[k];
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (piece_belongs_to_opponent(sq_arrival,side))
    {
      square const sq_capture = sq_arrival;
      sq_arrival += vec[k];
      if (e[sq_arrival]==vide)
        empile(sq_departure,sq_arrival,sq_capture);
    }
  }

  if (castling_supported)
    generate_castling(side);
}

static void marine_pawn_generate_capture(Side side, square sq_departure, int dir)
{
  square const sq_capture = sq_departure+dir;
  if (piece_belongs_to_opponent(sq_capture,side))
  {
    square const sq_arrival = sq_capture+dir;
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

/* Generate moves for a marine pawn
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 */
void marine_pawn_generate_moves(Side side, square sq_departure)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(side,sq_departure);

  if (no_capture_length>0)
  {
    int const dir_forward = side==White ? dir_up : dir_down;
    pawns_generate_nocapture_moves(sq_departure,dir_forward,no_capture_length);
    marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_right);
    marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_left);
  }
}

/* Generate moves for a marine ship (Triton + marine pawn)
 * @param side side for which to generate moves
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_ship_generate_moves(Side side,
                                square  sq_departure,
                                numvec  kbeg, numvec  kend)
{
  int const dir_forward = side==White ? dir_up : dir_down;
  marine_rider_generate_moves(side,sq_departure, vec_rook_start,vec_rook_end);
  marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_right);
  marine_pawn_generate_capture(side,sq_departure,dir_forward+dir_left);
}
