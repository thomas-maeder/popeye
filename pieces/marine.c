#include "pieces/marine.h"
#include "pieces/pawns/pawns.h"
#include "pieces/pawns/pawn.h"
#include "pydata.h"
#include "py4.h"
#include "debugging/trace.h"

#include <assert.h>

/* Generate moves for a marine rider
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_rider_generate_moves(square sq_departure,
                                 vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k = kbeg; k<=kend; k++)
  {
    square const sq_capture = generate_moves_on_line_segment(sq_departure,
                                                             sq_departure,
                                                             k);
    generate_locust_capture(sq_departure,sq_capture,k);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void marine_leaper_generate_moves(square sq_departure,
                                         vec_index_type kanf,
                                         vec_index_type kend)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  for (k = kanf; k<=kend; ++k)
  {
    square sq_arrival = sq_departure+vec[k];
    if (is_square_empty(sq_arrival))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
    else if (piece_belongs_to_opponent(sq_arrival))
    {
      square const sq_capture = sq_arrival;
      sq_arrival += vec[k];
      if (is_square_empty(sq_arrival))
        add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine knight
 * @param sq_departure departure square of the marine rider
 */
void marine_knight_generate_moves(square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  marine_leaper_generate_moves(sq_departure,vec_knight_start,vec_knight_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a poseidon
 * @param sq_departure departure square of the marine rider
 */
void poseidon_generate_moves(square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  marine_leaper_generate_moves(sq_departure,vec_queen_start,vec_queen_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void marine_pawn_generate_capture(square sq_departure, int dir)
{
  square const sq_capture = sq_departure+dir;
  square const sq_arrival = sq_capture+dir;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  if (is_square_empty(sq_arrival))
  {
    if (piece_belongs_to_opponent(sq_capture))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture);
    else
      pawns_generate_ep_capture_move(sq_departure,sq_arrival,sq_capture);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine pawn
 * @param sq_departure departure square of the marine rider
 */
void marine_pawn_generate_moves(square sq_departure)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],sq_departure);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (no_capture_length>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    pawns_generate_nocapture_moves(sq_departure,dir_forward,no_capture_length);
    marine_pawn_generate_capture(sq_departure,dir_forward+dir_right);
    marine_pawn_generate_capture(sq_departure,dir_forward+dir_left);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine ship (Triton + marine pawn)
 * @param sq_departure departure square of the marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_ship_generate_moves(square  sq_departure,
                                vec_index_type kbeg, vec_index_type kend)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  marine_rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
  marine_pawn_generate_capture(sq_departure,dir_forward+dir_right);
  marine_pawn_generate_capture(sq_departure,dir_forward+dir_left);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
