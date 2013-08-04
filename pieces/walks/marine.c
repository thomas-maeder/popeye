#include "pieces/walks/marine.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/locusts.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

/* Generate moves for a marine rider
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_rider_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kbeg);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (k = kbeg; k<=kend; k++)
  {
    square const sq_capture = generate_moves_on_line_segment(curr_generation->departure,k);
    generate_locust_capture(sq_capture,k);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void marine_leaper_generate_moves(vec_index_type kanf, vec_index_type kend)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (k = kanf; k<=kend; ++k)
  {
    curr_generation->arrival = curr_generation->departure+vec[k];
    if (is_square_empty(curr_generation->arrival))
      push_move();
    else if (piece_belongs_to_opponent(curr_generation->arrival))
    {
      square const sq_capture = curr_generation->arrival;
      curr_generation->arrival += vec[k];
      if (is_square_empty(curr_generation->arrival))
        push_move_capture_extra(sq_capture);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine knight
 * @param sq_departure departure square of the marine rider
 */
void marine_knight_generate_moves(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  marine_leaper_generate_moves(vec_knight_start,vec_knight_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a poseidon
 */
void poseidon_generate_moves(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  marine_leaper_generate_moves(vec_queen_start,vec_queen_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void marine_pawn_generate_capture(int dir)
{
  square const sq_capture = curr_generation->departure+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  curr_generation->arrival = sq_capture+dir;

  if (is_square_empty(curr_generation->arrival))
  {
    if (piece_belongs_to_opponent(sq_capture))
      push_move_capture_extra(sq_capture);
    else
      pawns_generate_ep_capture_move(curr_generation->arrival,sq_capture);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine pawn
 */
void marine_pawn_generate_moves(void)
{
  unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],
                                                                    curr_generation->departure);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (no_capture_length>0)
  {
    int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    pawns_generate_nocapture_moves(dir_forward,no_capture_length);
    marine_pawn_generate_capture(dir_forward+dir_right);
    marine_pawn_generate_capture(dir_forward+dir_left);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a marine ship (Triton + marine pawn)
 * @param kbeg,kend delimiters of interval in vector table
 */
void marine_ship_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  marine_rider_generate_moves(vec_rook_start,vec_rook_end);
  marine_pawn_generate_capture(dir_forward+dir_right);
  marine_pawn_generate_capture(dir_forward+dir_left);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
