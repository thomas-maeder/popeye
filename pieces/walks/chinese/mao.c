#include "pieces/walks/chinese/mao.h"
#include "pieces/walks/angle/angles.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/trace.h"

numvec mao_intermediate_vector(numvec to_arrival)
{
  numvec const y = to_arrival/(onerow-2);
  numvec const x = to_arrival - y*onerow;
  return (y/2)*onerow + (x/2);
}

numvec moa_intermediate_vector(numvec to_arrival)
{
  numvec const y = to_arrival/(onerow-2);
  numvec const x = to_arrival - y*onerow;
  return (y>0 ? dir_up : dir_down) + (x>0 ? dir_right : dir_left);
}

static void maooa_generate_move(numvec to_passed, numvec to_arrival)
{
  square const sq_departure = curr_generation->departure;
  square const sq_passed = sq_departure+to_passed;

  if (is_square_empty(sq_passed))
  {
    curr_generation->arrival = sq_departure+to_arrival;

    if (is_square_empty(curr_generation->arrival)
        || piece_belongs_to_opponent(curr_generation->arrival))
      hoppers_push_move(0,sq_passed);
  }
}

/* Generate moves for a Mao
 */
void mao_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    numvec const to_arrival = vec[k];
    maooa_generate_move(mao_intermediate_vector(to_arrival),to_arrival);
  }
}

/* Generate moves for a Moa
 */
void moa_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    numvec const to_arrival = vec[k];
    maooa_generate_move(moa_intermediate_vector(to_arrival),to_arrival);
  }
}

boolean maooacheck_onedir(square sq_pass,
                          vec_index_type vec_index_angle_departure_pass,
                          validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  numvec const vec_departure_pass = angle_vectors[angle_45][vec_index_angle_departure_pass];
  square const sq_departure = sq_pass+vec_departure_pass;

  return (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target));
}

static boolean maooacheck(vec_index_type vec_index_pass_target_begin,
                          vec_index_type vec_index_pass_target_end,
                          validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_index_pass_target_end;
       interceptable_observation[observation_context].vector_index1>=vec_index_pass_target_begin;
       --interceptable_observation[observation_context].vector_index1)
  {
    numvec const vec_pass_target = vec[interceptable_observation[observation_context].vector_index1];
    square const sq_pass = sq_target+vec_pass_target;

    if (is_square_empty(sq_pass)
        && (maooacheck_onedir(sq_pass,2*interceptable_observation[observation_context].vector_index1,evaluate)
            || maooacheck_onedir(sq_pass,2*interceptable_observation[observation_context].vector_index1-1,evaluate)))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

boolean mao_check(validator_id evaluate)
{
  return maooacheck(vec_bishop_start,vec_bishop_end,evaluate);
}

boolean moa_check(validator_id evaluate)
{
  return maooacheck(vec_rook_start,vec_rook_end,evaluate);
}

static void maooa_rider_generate_moves(numvec to_passed, numvec to_arrival)
{
  square const sq_departure = curr_generation->departure;

  square sq_passed = sq_departure+to_passed;
  curr_generation->arrival = sq_departure+to_arrival;

  while (is_square_empty(sq_passed) && is_square_empty(curr_generation->arrival))
  {
    push_move();
    sq_passed += to_arrival;
    curr_generation->arrival += to_arrival;
  }

  if (is_square_empty(sq_passed)
      && piece_belongs_to_opponent(curr_generation->arrival))
    push_move();
}

/* Generate moves for a Mao Rider
 */
void maorider_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    numvec const to_arrival = vec[k];
    maooa_rider_generate_moves(mao_intermediate_vector(to_arrival),to_arrival);
  }
}

/* Generate moves for a Moa Rider
 */
void moarider_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    numvec const to_arrival = vec[k];
    maooa_rider_generate_moves(moa_intermediate_vector(to_arrival),to_arrival);
  }
}

static boolean maooarider_check(numvec to_passed,
                                numvec to_departure,
                                validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  square sq_passed = sq_target+to_passed;
  square sq_departure= sq_target+to_departure;

  while (is_square_empty(sq_passed) && is_square_empty(sq_departure))
  {
    sq_passed += to_departure;
    sq_departure += to_departure;
  }

  return (is_square_empty(sq_passed)
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target));
}

boolean maorider_check(validator_id evaluate)
{
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_knight_start;
      interceptable_observation[observation_context].vector_index1<=vec_knight_end;
      ++interceptable_observation[observation_context].vector_index1)
  {
    numvec const to_departure = vec[interceptable_observation[observation_context].vector_index1];
    if (maooarider_check(moa_intermediate_vector(to_departure), /* we are going backward! */
                        to_departure,
                        evaluate))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

boolean moarider_check(validator_id evaluate)
{
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_knight_start;
      interceptable_observation[observation_context].vector_index1<=vec_knight_end;
      ++interceptable_observation[observation_context].vector_index1)
  {
    numvec const to_departure = vec[interceptable_observation[observation_context].vector_index1];
    if (maooarider_check(mao_intermediate_vector(to_departure), /* we are going backward! */
                        to_departure,
                        evaluate))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

static void maooa_rider_lion_generate_moves(numvec to_passed, numvec to_arrival)
{
  square const sq_departure = curr_generation->departure;
  square sq_passed = sq_departure+to_passed;
  curr_generation->arrival = sq_departure+to_arrival;

  while (is_square_empty(sq_passed) && is_square_empty(curr_generation->arrival))
  {
    sq_passed += to_arrival;
    curr_generation->arrival += to_arrival;
  }

  if (!is_square_blocked(sq_passed) && !is_square_blocked(curr_generation->arrival))
  {
    if (!is_square_empty(sq_passed)
        && (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival)))
      push_move();
    if (is_square_empty(sq_passed) || is_square_empty(curr_generation->arrival))
    {
      sq_passed += to_arrival;
      curr_generation->arrival += to_arrival;
      while (is_square_empty(sq_passed) && is_square_empty(curr_generation->arrival))
      {
        push_move();
        sq_passed += to_arrival;
        curr_generation->arrival += to_arrival;
      }
    }

    if (is_square_empty(sq_passed)
        && piece_belongs_to_opponent(curr_generation->arrival))
      push_move();
  }
}

/* Generate moves for a Mao Rider Lion
 */
void maoriderlion_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    numvec const to_arrival = vec[k];
    maooa_rider_lion_generate_moves(mao_intermediate_vector(to_arrival),to_arrival);
  }
}

/* Generate moves for a Moa Rider Lion
 */
void moariderlion_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    numvec const to_arrival = vec[k];
    maooa_rider_lion_generate_moves(moa_intermediate_vector(to_arrival),to_arrival);
  }
}

static boolean maooariderlion_check(numvec to_passed,
                                    numvec to_departure,
                                    validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  square sq_passed = sq_target+to_passed;
  square sq_departure = sq_target+to_departure;

  while (is_square_empty(sq_passed) && is_square_empty(sq_departure))
  {
    sq_passed += to_departure;
    sq_departure += to_departure;
  }

  if (!is_square_empty(sq_passed)
      && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
    return true;

  if (!is_square_blocked(sq_passed) && !is_square_blocked(sq_departure)
      && (is_square_empty(sq_passed) || is_square_empty(sq_departure)))
  {
    sq_passed += to_departure;
    sq_departure += to_departure;

    while (is_square_empty(sq_passed) && is_square_empty(sq_departure))
    {
      sq_passed += to_departure;
      sq_departure += to_departure;
    }

    if (is_square_empty(sq_passed)
        && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean maoriderlion_check(validator_id evaluate)
{
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_knight_start;
      interceptable_observation[observation_context].vector_index1<=vec_knight_end;
      ++interceptable_observation[observation_context].vector_index1)
  {
    numvec const to_departure = vec[interceptable_observation[observation_context].vector_index1];
    if (maooariderlion_check(moa_intermediate_vector(to_departure), /* we are going backward! */
                             to_departure,
                             evaluate))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

boolean moariderlion_check(validator_id evaluate)
{
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_knight_start;
      interceptable_observation[observation_context].vector_index1<=vec_knight_end;
      ++interceptable_observation[observation_context].vector_index1)
  {
    numvec const to_departure = vec[interceptable_observation[observation_context].vector_index1];
    if (maooariderlion_check(mao_intermediate_vector(to_departure), /* we are going backward! */
                             to_departure,
                             evaluate))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

static void maooahopper_generate_moves(vec_index_type k,
                                       numvec to_intermediate)
{
  numvec const to_arrival = vec[k];
  square const sq_departure = curr_generation->departure;
  square const sq_hurdle = sq_departure+to_intermediate;

  if (!is_square_empty(sq_hurdle))
  {
    curr_generation->arrival = sq_departure+to_arrival;
    if (is_square_empty(curr_generation->arrival)
        || piece_belongs_to_opponent(curr_generation->arrival))
      hoppers_push_move(k,sq_hurdle);
  }
}

void moahopper_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
    maooahopper_generate_moves(k,moa_intermediate_vector(vec[k]));
}

void maohopper_generate_moves(void)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
    maooahopper_generate_moves(k,mao_intermediate_vector(vec[k]));
}

static boolean maooahopper_check(vec_index_type vec_index_pass_target_begin,
                                 vec_index_type vec_index_pass_target_end,
                                 validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = vec_index_pass_target_end;
       interceptable_observation[observation_context].vector_index1>=vec_index_pass_target_begin;
       --interceptable_observation[observation_context].vector_index1)
  {
    numvec const vec_pass_target = vec[interceptable_observation[observation_context].vector_index1];
    square const sq_pass = sq_target+vec_pass_target;

    if (!is_square_empty(sq_pass)
        && (maooacheck_onedir(sq_pass,2*interceptable_observation[observation_context].vector_index1,evaluate)
            || maooacheck_onedir(sq_pass,2*interceptable_observation[observation_context].vector_index1-1,evaluate)))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  return result;
}

boolean maohopper_check(validator_id evaluate)
{
  return maooahopper_check(vec_bishop_start,vec_bishop_end,evaluate);
}

boolean moahopper_check(validator_id evaluate)
{
  return maooahopper_check(vec_rook_start,vec_rook_end,evaluate);
}
