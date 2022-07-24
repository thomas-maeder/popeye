#include "pieces/walks/marine.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/locusts.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
    generate_locust_capture(sq_capture,vec[k]);
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
    numvec const dir = vec[k];
    curr_generation->arrival = curr_generation->departure+dir;
    if (is_square_empty(curr_generation->arrival))
      push_move_no_capture();
    else
      generate_locust_capture(curr_generation->arrival,dir);
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
      generate_locust_capture(sq_capture,dir);
    else
      pawns_generate_ep_capture_move(sq_capture);
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

static boolean marine_leaper_check(vec_index_type kanf, vec_index_type kend,
                                   validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type k;
  for (k = kanf; k<=kend; ++k)
  {
    square const sq_arrival = sq_target-vec[k];
    square const sq_departure = sq_target+vec[k];
    if (is_square_empty(sq_arrival)
        && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_arrival))
      return true;
  }

  return false;
}

boolean marine_knight_check(validator_id evaluate)
{
  return marine_leaper_check(vec_knight_start,vec_knight_end,evaluate);
}

boolean poseidon_check(validator_id evaluate)
{
  return marine_leaper_check(vec_queen_start,vec_queen_end,evaluate);
}

static boolean marine_pawn_test_check(square sq_departure,
                                      square sq_hurdle,
                                      validator_id evaluate)
{
  boolean result;
  numvec const dir_check = sq_hurdle-sq_departure;
  square const sq_arrival = sq_hurdle+dir_check;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_hurdle);
  TraceFunctionParamListEnd();

  result = (is_square_empty(sq_arrival)
            && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_arrival));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean marine_pawn_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  numvec const dir_forward_right = dir_forward+dir_right;
  numvec const dir_forward_left = dir_forward+dir_left;

  if (marine_pawn_test_check(sq_target-dir_forward_right,sq_target,evaluate))
    return true;
  else if (marine_pawn_test_check(sq_target-dir_forward_left,sq_target,evaluate))
    return true;
  else if (en_passant_test_check(dir_forward_right,&marine_pawn_test_check,evaluate))
    return true;
  else if (en_passant_test_check(dir_forward_left,&marine_pawn_test_check,evaluate))
    return true;

  return false;
}

boolean marine_ship_check(validator_id evaluate)
{
  return marine_pawn_check(evaluate) || rooklocust_check(evaluate);
}
