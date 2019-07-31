#include "pieces/walks/spiral_springers.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/trace.h"

static void generate_zigzag(vec_index_type idx_zig, vec_index_type idx_zag)
{
  curr_generation->arrival = curr_generation->departure+vec[idx_zig];

  while (is_square_empty(curr_generation->arrival))
  {
    push_move_no_capture();
    curr_generation->arrival += vec[idx_zag];
    if (is_square_empty(curr_generation->arrival))
    {
      push_move_no_capture();
      curr_generation->arrival += vec[idx_zig];
    }
    else
      break;
  }
  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move_regular_capture();
}

static boolean zigzag_check(vec_index_type idx_zig, vec_index_type idx_zag,
                            validator_id evaluate)
{
  boolean result;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  square sq_departure = sq_arrival+vec[idx_zig];

  ++observation_context;

  interceptable_observation[observation_context].vector_index1 = idx_zig;
  interceptable_observation[observation_context].auxiliary = idx_zag;

  while (is_square_empty(sq_departure))
  {
    sq_departure += vec[idx_zag];
    if (is_square_empty(sq_departure))
      sq_departure += vec[idx_zig];
    else
      break;
  }

  result = EVALUATE_OBSERVATION(evaluate,sq_departure,sq_arrival);

  --observation_context;

  return result;
}

/* calculate the vector index refering to the vector pointing into the
 * opposite direction
 * Note: this only works for sequences of vec elements where the second
 * half of the sequence contains the negative values of the first half;
 * i.e. this doesn't work for queen or king, but luckily it does for rook,
 * bishop and knight
 */
static vec_index_type vec_index_mirror(vec_index_type start,
                                       vec_index_type end,
                                       vec_index_type to_be_negated)
{
  vec_index_type const len_range = end-start+1;
  return start + (to_be_negated-start+len_range/2)%len_range;
}

void spiralspringer_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  vec_index_type const top = vec_knight_start+vec_knight_end;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    generate_zigzag(k,top-k);
    generate_zigzag(k,vec_index_mirror(vec_knight_start,vec_knight_end,top-k));
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean spiralspringer_check(validator_id evaluate)
{
  vec_index_type const top = vec_knight_start+vec_knight_end;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k++)
    if (zigzag_check(k,top-k,evaluate)
        || zigzag_check(k,vec_index_mirror(vec_knight_start,vec_knight_end,top-k),evaluate))
      return true;

  return false;
}

void diagonalspiralspringer_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_zigzag(k,k+1);
    generate_zigzag(k,vec_index_mirror(vec_knight_start,vec_knight_end,k+1));
    generate_zigzag(k+1,k);
    generate_zigzag(k+1,vec_index_mirror(vec_knight_start,vec_knight_end,k));
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean diagonalspiralspringer_check(validator_id evaluate)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
    if (zigzag_check(k,k+1,evaluate)
        || zigzag_check(k,vec_index_mirror(vec_knight_start,vec_knight_end,k+1),evaluate)
        || zigzag_check(k+1,k,evaluate)
        || zigzag_check(k+1,vec_index_mirror(vec_knight_start,vec_knight_end,k),evaluate))
      return true;

  return false;
}

void boyscout_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type const top = vec_bishop_start+vec_bishop_end;
  vec_index_type k;
  for (k = vec_bishop_start; k<=vec_bishop_end; ++k)
  {
    generate_zigzag(k,top-k);
    generate_zigzag(k,vec_index_mirror(vec_bishop_start,vec_bishop_end,top-k));
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean boyscout_check(validator_id evaluate)
{
  vec_index_type const top = vec_bishop_start+vec_bishop_end;
  vec_index_type k;

  for (k= vec_bishop_start; k<=vec_bishop_end; k++)
    if (zigzag_check(k,top-k,evaluate)
        || zigzag_check(k,vec_index_mirror(vec_bishop_start,vec_bishop_end,top-k),evaluate))
      return true;

  return false;
}

void girlscout_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type const top = vec_rook_start+vec_rook_end;
  vec_index_type k;
  for (k = vec_rook_end; k>=vec_rook_start; --k)
  {
    generate_zigzag(k,top-k);
    generate_zigzag(k,vec_index_mirror(vec_rook_start,vec_rook_end,top-k));
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean girlscout_check(validator_id evaluate)
{
  vec_index_type const top = vec_rook_start+vec_rook_end;
  vec_index_type k;

  for (k= vec_rook_end; k>=vec_rook_start; k--)
    if (zigzag_check(k,top-k,evaluate)
        || zigzag_check(k,vec_index_mirror(vec_rook_start,vec_rook_end,top-k),evaluate))
      return true;

  return false;
}

void spiralspringer40_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_zigzag(k, k+7);
    generate_zigzag(k+7, k);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean spiralspringer40_check(validator_id evaluate)
{
  boolean result = false;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
    if (zigzag_check(k,k+7,evaluate) || zigzag_check(k+7,k,evaluate))
    {
      result = true;
      break;
    }

  return result;
}

void spiralspringer20_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_zigzag(k, k+3);
    generate_zigzag(k+3, k);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean spiralspringer20_check(validator_id evaluate)
{
  boolean result = false;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
    if (zigzag_check(k,k+3,evaluate) || zigzag_check(k+3,k,evaluate))
    {
      result = true;
      break;
    }

  return result;
}

void spiralspringer33_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_zigzag(k, k+1);
    generate_zigzag(k+1, k);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean spiralspringer33_check(validator_id evaluate)
{
  boolean result = false;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
    if (zigzag_check(k,k+1,evaluate) || zigzag_check(k+1,k,evaluate))
    {
      result = true;
      break;
    }

  return result;
}

void spiralspringer11_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type k;
  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    generate_zigzag(k, k+5);
    generate_zigzag(k+5, k);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean spiralspringer11_check(validator_id evaluate)
{
  boolean result = false;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
    if (zigzag_check(k,k+5,evaluate) || zigzag_check(k+5,k,evaluate))
    {
      result = true;
      break;
    }

  return result;
}

void quintessence_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    generate_zigzag(k, k+2);
    generate_zigzag(k+2, k);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean quintessence_check(validator_id evaluate)
{
  boolean result = false;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
    if (zigzag_check(k,k+2,evaluate) || zigzag_check(k+2,k,evaluate))
    {
      result = true;
      break;
    }

  return result;
}
