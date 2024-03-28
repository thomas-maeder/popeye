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

      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
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

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
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
  if (!(TSTFLAG(being_solved.spec[curr_generation->departure],ColourChange)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Bul)
        || TSTFLAG(being_solved.spec[curr_generation->departure],Dob)))
    remove_duplicate_moves_of_single_piece(save_current_move);
}

boolean marguerite_check(validator_id evaluate)
{
  return (sparrow_check(evaluate)
          || eagle_check(evaluate)
          || moose_check(evaluate)
          || grasshopper_check(evaluate));
}

static void eagle_equihopper_generate_moves_queen_lines_2nd_leg(square sq_departure,
                                                                square sq_hurdle,
                                                                vec_index_type k,
                                                                vec_index_type k1)
{
  square const end_of_line = find_end_of_line(sq_hurdle,angle_vectors[angle_90][k1]);
  int const dist_hurdle_end = (end_of_line-sq_hurdle)/angle_vectors[angle_90][k1];
  int const dist_hurdle_dep = (sq_hurdle-sq_departure)/vec[k];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_hurdle);
  TraceValue("%u",k);
  TraceValue("%u",k1);
  TraceFunctionParamListEnd();

  assert(dist_hurdle_dep>0);
  assert(dist_hurdle_end>0);

  curr_generation->arrival = sq_hurdle+dist_hurdle_dep*angle_vectors[angle_90][k1];
  TraceSquare(end_of_line);
  TraceValue("%d",dist_hurdle_end);
  TraceValue("%d",dist_hurdle_dep);
  TraceSquare(curr_generation->arrival);
  TraceEOL();

  if (!is_square_blocked(curr_generation->arrival))
  {
    if (dist_hurdle_end>dist_hurdle_dep)
      hoppers_push_move(k,sq_hurdle);
    else if (dist_hurdle_end==dist_hurdle_dep
             && piece_belongs_to_opponent(curr_generation->arrival))
      hoppers_push_capture(k,sq_hurdle);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void eagle_equihopper_generate_moves_other_lines_2nd_leg(square sq_hurdle,
                                                                vec_index_type k)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_hurdle);
  TraceFunctionParam("%u",k);
  TraceFunctionParamListEnd();

  TraceSquare(curr_generation->arrival);TraceEOL();

  if (!is_square_blocked(curr_generation->arrival))
  {
    if (is_square_empty(curr_generation->arrival))
      hoppers_push_move(k,sq_hurdle);
    else if (piece_belongs_to_opponent(curr_generation->arrival))
      hoppers_push_capture(k,sq_hurdle);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void eagle_equihopper_generate_moves(void)
{
  numecoup const save_base = CURRMOVE_OF_PLY(nbply);
  square const sq_departure = curr_generation->departure;
  vec_index_type  k;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);TraceEOL();

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_hurdle = find_end_of_line(sq_departure,vec[k]);
    TraceValue("%u",k);TraceSquare(sq_hurdle);TraceEOL();

    if (!is_square_blocked(sq_hurdle))
    {
      vec_index_type const k1 = 2*k;

      eagle_equihopper_generate_moves_queen_lines_2nd_leg(sq_departure,
                                                          sq_hurdle,
                                                          k,
                                                          k1);
      eagle_equihopper_generate_moves_queen_lines_2nd_leg(sq_departure,
                                                          sq_hurdle,
                                                          k,
                                                          k1-1);
    }
  }

  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)
  {
    square const sq_hurdle = sq_departure+vec[k];

    if (get_walk_of_piece_on_square(sq_hurdle)>=King)
    {
      numvec const x = sq_hurdle%onerow - sq_departure%onerow;
      numvec const y = sq_hurdle/onerow - sq_departure/onerow;

      curr_generation->arrival = sq_hurdle + x*onerow - y;
      eagle_equihopper_generate_moves_other_lines_2nd_leg(sq_hurdle,k);

      curr_generation->arrival = sq_hurdle - x*onerow + y;
      eagle_equihopper_generate_moves_other_lines_2nd_leg(sq_hurdle,k);
    }
  }

  remove_duplicate_moves_of_single_piece(save_base);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean eagle_equihopper_check_queen_lines_2nd_leg(validator_id evaluate,
                                                          square sq_target,
                                                          square sq_hurdle)
{
  boolean result = false;
  square const sq_departure = find_end_of_line(sq_hurdle,angle_vectors[angle_90][interceptable_observation[observation_context].vector_index2]);
  int const dist_hurdle_target = (sq_hurdle-sq_target)/vec[interceptable_observation[observation_context].vector_index1];
  int const dist_hurdle_dep = (sq_departure-sq_hurdle)/angle_vectors[angle_90][interceptable_observation[observation_context].vector_index2];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceSquare(sq_hurdle);
  TraceFunctionParamListEnd();

  assert(dist_hurdle_dep>0);
  assert(dist_hurdle_target>0);
  if (dist_hurdle_dep==dist_hurdle_target
      && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean eagle_equihopper_check_queen_lines(validator_id evaluate,
                                                  square sq_target)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  for (interceptable_observation[observation_context].vector_index1 = vec_queen_end;
       interceptable_observation[observation_context].vector_index1>=vec_queen_start;
       interceptable_observation[observation_context].vector_index1--)
  {
    square const sq_hurdle = find_end_of_line(sq_target,vec[interceptable_observation[observation_context].vector_index1]);

    if (!is_square_blocked(sq_hurdle))
    {
      interceptable_observation[observation_context].vector_index2 = 2*interceptable_observation[observation_context].vector_index1;

      hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;

      if (eagle_equihopper_check_queen_lines_2nd_leg(evaluate,sq_target,sq_hurdle))
      {
        result = true;
        break;
      }
      else
      {
        --interceptable_observation[observation_context].vector_index2;
        if (eagle_equihopper_check_queen_lines_2nd_leg(evaluate,sq_target,sq_hurdle))
        {
          result = true;
          break;
        }
      }
    }
  }

  interceptable_observation[observation_context+1].vector_index1 = 0;
  interceptable_observation[observation_context+1].vector_index2 = 0;

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean eagle_equihopper_check_other_lines_2nd_leg(validator_id evaluate,
                                                          square sq_target,
                                                          square sq_hurdle,
                                                          square sq_departure)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceSquare(sq_hurdle);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
  if (!is_square_empty(sq_hurdle)
      && !is_square_blocked(sq_hurdle)
      && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean eagle_equihopper_check_other_lines(validator_id evaluate,
                                                  square sq_target)
{
  boolean result = false;
  vec_index_type  k;

  for (k = vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++)      /* 2,4; 2,6; 4,6; */
  {
    square const sq_hurdle = sq_target+vec[k];
    numvec const x = sq_hurdle%onerow - sq_target%onerow;
    numvec const y = sq_hurdle/onerow - sq_target/onerow;

    if (eagle_equihopper_check_other_lines_2nd_leg(evaluate,
                                                   sq_target,
                                                   sq_hurdle,
                                                   sq_hurdle + x*onerow - y)
        || eagle_equihopper_check_other_lines_2nd_leg(evaluate,
                                                      sq_target,
                                                      sq_hurdle,
                                                      sq_hurdle - x*onerow + y))
    {
      result = true;
      break;
    }
  }

  return result;
}

boolean eagle_equihopper_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++observation_context;

  result = (eagle_equihopper_check_queen_lines(evaluate,sq_target)
            || eagle_equihopper_check_other_lines(evaluate,sq_target));

  --observation_context;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void eagle_nonstop_equihopper_generate_moves_for_vector_range(vec_index_type start,
                                                                     vec_index_type end)
{
  square const sq_departure = curr_generation->departure;
  vec_index_type  k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",end);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);TraceEOL();

  for (k = start; k<=end; k++)
  {
    square sq_hurdle = sq_departure;

    for (sq_hurdle += vec[k];
        !is_square_blocked(sq_hurdle);
        sq_hurdle += vec[k])
    {
      if (sq_hurdle!=sq_departure /* prevent nNE from capturing itself */
          && !is_square_empty(sq_hurdle))
      {
        numvec const x = sq_hurdle%onerow - sq_departure%onerow;
        numvec const y = sq_hurdle/onerow - sq_departure/onerow;

        {
          curr_generation->arrival = sq_hurdle + x*onerow - y;

          if (is_square_empty(curr_generation->arrival))
            hoppers_push_move(0,sq_hurdle);
          else if (piece_belongs_to_opponent(curr_generation->arrival))
            hoppers_push_capture(0,sq_hurdle);
        }

        {
          curr_generation->arrival = sq_hurdle - x*onerow + y;

          if (is_square_empty(curr_generation->arrival))
            hoppers_push_move(0,sq_hurdle);
          else if (piece_belongs_to_opponent(curr_generation->arrival))
            hoppers_push_capture(0,sq_hurdle);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void eagle_nonstop_equihopper_generate_moves(void)
{
  numecoup const save_base = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  eagle_nonstop_equihopper_generate_moves_for_vector_range(vec_queen_start,vec_queen_end);
  eagle_nonstop_equihopper_generate_moves_for_vector_range(vec_equi_nonintercept_start,vec_equi_nonintercept_end);

  remove_duplicate_moves_of_single_piece(save_base);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean eagle_nonstop_equihopper_check_for_vector_range(vec_index_type start,
                                                               vec_index_type end,
                                                               validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",end);
  TraceFunctionParamListEnd();

  TraceSquare(sq_target);
  TraceEOL();

  ++observation_context;

  for (k = start; k<=end && !result; k++)
  {
    square sq_hurdle = sq_target;

    for (sq_hurdle += vec[k];
        !is_square_blocked(sq_hurdle);
        sq_hurdle += vec[k])
    {
      if (!is_square_empty(sq_hurdle))
      {
        numvec const x = sq_hurdle%onerow - sq_target%onerow;
        numvec const y = sq_hurdle/onerow - sq_target/onerow;

        TraceSquare(sq_hurdle);
        TraceValue("%d",x);
        TraceValue("%d",y);
        TraceEOL();

        {
          square const sq_departure = sq_hurdle + x*onerow - y;

          TraceSquare(sq_departure);
          TraceEOL();

          hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
          if (sq_target!=sq_departure
              && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
          {
            result = true;
            break;
          }
        }

        {
          square const sq_departure = sq_hurdle - x*onerow + y;

          TraceSquare(sq_departure);
          TraceEOL();

          hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = sq_hurdle;
          if (sq_target!=sq_departure
              && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
          {
            result = true;
            break;
          }
        }
      }
    }
  }

  hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle = initsquare;

  --observation_context;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean eagle_nonstop_equihopper_check(validator_id evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = (eagle_nonstop_equihopper_check_for_vector_range(vec_queen_start,vec_queen_end,evaluate)
            || eagle_nonstop_equihopper_check_for_vector_range(vec_equi_nonintercept_start,vec_equi_nonintercept_end,evaluate));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
