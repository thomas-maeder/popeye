#include "pieces/walks/angle/hoppers.h"
#include "pieces/walks/hoppers.h"
#include "pieces/walks/hamster.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Generated moves for an angle hopper
 * @param kanf first vectors index
 * @param kend last vectors index
 * @param angle angle to take from hurdle to arrival squares
 */
static void angle_hoppers_generate_moves(vec_index_type kanf, vec_index_type kend,
                                         angle_t angle)
{
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam ("%u",kanf);
  TraceFunctionParam ("%u",kend);
  TraceFunctionParam ("%u",angle);
  TraceFunctionParamListEnd();

  for (k = kend; k>=kanf; k--)
  {
    square const sq_hurdle = find_end_of_line(curr_generation->departure,vec[k]);
    if (!is_square_blocked(sq_hurdle))
    {
      vec_index_type const k1 = 2*k;

      {
        curr_generation->arrival = sq_hurdle+angle_vectors[angle][k1];
        if (is_square_empty(curr_generation->arrival))
          hoppers_push_move(k,sq_hurdle);
        else if (piece_belongs_to_opponent(curr_generation->arrival))
          hoppers_push_capture(k,sq_hurdle);
      }

      {
        curr_generation->arrival = sq_hurdle+angle_vectors[angle][k1-1];
        if (is_square_empty(curr_generation->arrival))
          hoppers_push_move(k,sq_hurdle);
        else if (piece_belongs_to_opponent(curr_generation->arrival))
          hoppers_push_capture(k,sq_hurdle);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean angle_hoppers_is_square_observed_one_dir(square sq_hurdle,
                                                        vec_index_type vec_index_departure_hurdle,
                                                        angle_t angle,
                                                        validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  numvec const vec_departure_hurdle = angle_vectors[angle][vec_index_departure_hurdle];
  square const sq_departure = find_end_of_line(sq_hurdle,vec_departure_hurdle);

  return EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target);
}

/* Is a particular square observed by a particular type of angle hopper?
 * @param kanf first vectors index
 * @param kend last vectors index
 * @param angle angle to take from hurdle to arrival squares
 */
static boolean angle_hoppers_is_square_observed(vec_index_type kanf, vec_index_type kend,
                                                angle_t angle,
                                                validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParam ("%u",kanf);
  TraceFunctionParam ("%u",kend);
  TraceFunctionParam ("%u",angle);
  TraceFunctionParamListEnd();

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kend;
       interceptable_observation[observation_context].vector_index1>=kanf;
       --interceptable_observation[observation_context].vector_index1)
  {
    numvec const vec_hurdle_target = vec[interceptable_observation[observation_context].vector_index1];
    square const sq_hurdle = sq_target+vec_hurdle_target;

    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      vec_index_type const vec_index_departure_hurdle = 2*interceptable_observation[observation_context].vector_index1;

      if (angle_hoppers_is_square_observed_one_dir(sq_hurdle,
                                                   vec_index_departure_hurdle,
                                                   angle,
                                                   evaluate)
          || angle_hoppers_is_square_observed_one_dir(sq_hurdle,
                                                      vec_index_departure_hurdle-1,
                                                      angle,
                                                      evaluate))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generated moves for an Elk
 */
void elk_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_queen_start,vec_queen_end, angle_45);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean moose_check(validator_id evaluate)
{
  return angle_hoppers_is_square_observed(vec_queen_start,vec_queen_end, angle_45, evaluate);
}

/* Generated moves for an Rook Moose
 */
void rook_moose_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_rook_start,vec_rook_end, angle_45);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean rookmoose_check(validator_id evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_bishop_start,vec_bishop_end, angle_45, evaluate);
}

/* Generated moves for an Bishop Moose
 */
void bishop_moose_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_bishop_start,vec_bishop_end, angle_45);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean bishopmoose_check(validator_id evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_rook_start,vec_rook_end, angle_45, evaluate);
}

/* Generated moves for an Eagle
 */
void eagle_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_queen_start,vec_queen_end, angle_90);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean eagle_check(validator_id evaluate)
{
  return angle_hoppers_is_square_observed(vec_queen_start,vec_queen_end, angle_90, evaluate);
}

/* Generated moves for a Rook Eagle
 */
void rook_eagle_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_rook_start,vec_rook_end, angle_90);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean rookeagle_check(validator_id evaluate)
{
  return angle_hoppers_is_square_observed(vec_rook_start,vec_rook_end, angle_90, evaluate);
}

/* Generated moves for a Bishop Eagle
 */
void bishop_eagle_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_bishop_start,vec_bishop_end, angle_90);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean bishopeagle_check(validator_id evaluate)
{
  return angle_hoppers_is_square_observed(vec_bishop_start,vec_bishop_end, angle_90, evaluate);
}

/* Generated moves for a Sparrow
 */
void sparrow_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_queen_start,vec_queen_end, angle_135);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean sparrow_check(validator_id evaluate)
{
  return angle_hoppers_is_square_observed(vec_queen_start,vec_queen_end, angle_135, evaluate);
}

/* Generated moves for a Rook Sparrow
 */
void rook_sparrow_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_rook_start,vec_rook_end, angle_135);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean rooksparrow_check(validator_id evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_bishop_start,vec_bishop_end, angle_135, evaluate);
}

/* Generated moves for a Bishop Sparrow
 */
void bishop_sparrow_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_bishop_start,vec_bishop_end, angle_135);
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean bishopsparrow_check(validator_id evaluate)
{
  /* these vector indices are correct - we are retracting along these vectors! */
  return angle_hoppers_is_square_observed(vec_rook_start,vec_rook_end, angle_135, evaluate);
}

/* Generated moves for a Marguerite
 */
void marguerite_generate_moves(void)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);
  angle_hoppers_generate_moves(vec_queen_start,vec_queen_end, angle_45);
  angle_hoppers_generate_moves(vec_queen_start,vec_queen_end, angle_90);
  angle_hoppers_generate_moves(vec_queen_start,vec_queen_end, angle_135);
  rider_hoppers_generate_moves(vec_queen_start,vec_queen_end);
  hamster_generate_moves();
  if (!TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean marguerite_check(validator_id evaluate)
{
  return (sparrow_check(evaluate)
          || eagle_check(evaluate)
          || moose_check(evaluate)
          || grasshopper_check(evaluate));
}
