#include "pieces/walks/roses.h"
#include "pieces/walks/locusts.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/fork.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <stdlib.h>

static square generate_moves_on_circle_segment(square sq_base,
                                               vec_index_type *idx_curr_dir,
                                               rose_rotation_sense sense)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_base);
  TraceFunctionParam("%u",*idx_curr_dir);
  TraceFunctionParam("%d",sense);
  TraceFunctionParamListEnd();

  curr_generation->arrival = sq_base;

  curr_generation->arrival += vec[*idx_curr_dir];
  // TODO does this overflow work on all implementations?
  assert(abs(sense)==1);
  assert(*idx_curr_dir>0 || sense>0);
  *idx_curr_dir += (unsigned int)sense;

  while (curr_generation->arrival!=sq_base
         && is_square_empty(curr_generation->arrival))
  {
    push_move_no_capture();
    curr_generation->arrival += vec[*idx_curr_dir];
    // TODO does this overflow work on all implementations?
    assert(abs(sense)==1);
    assert(*idx_curr_dir>0 || sense>0);
    *idx_curr_dir += (unsigned int)sense;
  }

  TraceFunctionExit(__func__);
  TraceSquare(curr_generation->arrival);
  TraceFunctionResultEnd();
  return curr_generation->arrival;
}

/* Find the next occupied square on a circular line
 * @param sq_departure indicates where to start the search
 * @param idx_curr_dir indicates the direction in which to start the line
 * @param indicates the rotation sense
 * @return first occupied square met
 * @note sets *idx_curr_dir to the direction at the result square, allowing
 *       the circle to be continued from that square
 */
static square find_end_of_circle_line(square sq_departure,
                                      vec_index_type *idx_curr_dir,
                                      rose_rotation_sense sense)
{
  square sq_result = sq_departure;
  do
  {
    sq_result += vec[*idx_curr_dir];
    // TODO does this overflow work on all implementations?
    assert(abs(sense)==1);
    assert(*idx_curr_dir>0 || sense>0);
    *idx_curr_dir += (unsigned int)sense;
  } while (is_square_empty(sq_result));

  return sq_result;
}

static void rose_generate_circle(vec_index_type idx_curr_dir,
                                 rose_rotation_sense sense)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx_curr_dir);
  TraceFunctionParam("%d",sense);
  TraceFunctionParamListEnd();

  curr_generation->arrival = generate_moves_on_circle_segment(curr_generation->departure,
                                                              &idx_curr_dir,
                                                              sense);
  if (curr_generation->arrival!=curr_generation->departure
      && piece_belongs_to_opponent(curr_generation->arrival))
    push_move_regular_capture();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a Rose
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void rose_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    rose_generate_circle(vec_index_start,
                         rose_rotation_clockwise);
    rose_generate_circle(vec_index_start + vec_range_end-vec_range_start+1,
                         rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

/* Detect observation on a rose line
 * @param idx_curr_dir indicates the direction in which to start the line
 * @param indicates the rotation sense
 */
static boolean detect_rose_check_on_line(vec_index_type idx_curr_dir,
                                         rose_rotation_sense sense,
                                         validator_id evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",idx_curr_dir);
  TraceValue("%d",sense);
  TraceFunctionParamListEnd();

  interceptable_observation[observation_context].vector_index1 = idx_curr_dir;
  interceptable_observation[observation_context].auxiliary = sense;

  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    square const sq_departure = find_end_of_circle_line(sq_target,&idx_curr_dir,sense);

    result = (sq_departure!=sq_target /* pieces don't observe themselves */
              && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean rose_check(validator_id evaluate)
{
  boolean result = false;
  vec_index_type idx_curr_dir;

  ++observation_context;

  for (idx_curr_dir = vec_knight_start;
       idx_curr_dir<=vec_knight_end;
       idx_curr_dir++)
    if (detect_rose_check_on_line(idx_curr_dir,
                                  rose_rotation_clockwise,
                                  evaluate)
        || detect_rose_check_on_line(idx_curr_dir + vec_knight_end-vec_knight_start + 1,
                                     rose_rotation_counterclockwise,
                                     evaluate))
    {
      result =true;
      break;
    }

  --observation_context;

  return result;
}

static void rao_generate_circle(vec_index_type idx_curr_dir,
                                rose_rotation_sense sense)
{
  square sq_hurdle = generate_moves_on_circle_segment(curr_generation->departure,
                                                      &idx_curr_dir,
                                                      sense);
  if (sq_hurdle!=curr_generation->departure && !is_square_blocked(sq_hurdle))
  {
    curr_generation->arrival = find_end_of_circle_line(sq_hurdle,
                                                       &idx_curr_dir,
                                                       sense);
    if (curr_generation->arrival!=curr_generation->departure
        && piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

/* Generate moves for a Rao
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void rao_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    rao_generate_circle(vec_index_start,
                        rose_rotation_clockwise);
    rao_generate_circle(vec_index_start + vec_range_end-vec_range_start + 1,
                        rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void roselion_generate_circle(vec_index_type idx_curr_dir,
                                     rose_rotation_sense sense)
{
  square sq_hurdle = find_end_of_circle_line(curr_generation->departure,
                                             &idx_curr_dir,
                                             sense);
  if (sq_hurdle!=curr_generation->departure && !is_square_blocked(sq_hurdle))
  {
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    /* cf. issue 1747928 */
    /* temporarily remove the moving piece to prevent it from blocking
     * itself */
    piece save_piece = being_solved.board[curr_generation->departure];
    being_solved.board[curr_generation->departure] = /* vide */ Empty /* TODO: Is Empty the correct value here? */;
    /* could be going for another 8 steps
    let's make sure we don't run out of S vectors */
    if (delta_k > 0)
    {
      if (k1+k2 > vec_knight_end)
        k2-=8;
    }
    else
    {
      if (k1+k2 <= vec_knight_end)
        k2+=8;
    }
#endif
    curr_generation->arrival = generate_moves_on_circle_segment(sq_hurdle,
                                                                &idx_curr_dir,
                                                                sense);
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    being_solved.board[curr_generation->departure] = save_piece;
#endif
    if (curr_generation->arrival!=curr_generation->departure
        && piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

/* Generate moves for a Rose Lion
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void roselion_generate_moves(vec_index_type vec_range_start,
                             vec_index_type vec_range_end)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type vec_index_start;
  for (vec_index_start= vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    roselion_generate_circle(vec_index_start,
                             rose_rotation_clockwise);
    roselion_generate_circle(vec_index_start + vec_range_end-vec_range_start + 1,
                             rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

/* Detect observation on a rose lion line
 * @param idx_curr_dir indicates the direction in which to start the line
 * @param indicates the rotation sense
 */
static boolean detect_roselion_check_on_line(vec_index_type idx_curr_dir,
                                             rose_rotation_sense sense,
                                             validator_id evaluate)
{
  interceptable_observation[observation_context].vector_index1 = idx_curr_dir;
  interceptable_observation[observation_context].auxiliary = sense;

  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    square const sq_hurdle = find_end_of_circle_line(sq_target,
                                                     &idx_curr_dir,
                                                     sense);
    if (sq_hurdle!=sq_target && !is_square_blocked(sq_hurdle))
    {
      square sq_departure= find_end_of_circle_line(sq_hurdle,
                                                   &idx_curr_dir,
                                                   sense);

#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
      /* cf. issue 1747928 */
      if (sq_departure==sq_target && being_solved.board[sq_hurdle]==observing_walk[nbply]) {
        /* special case: king and rose lion are the only pieces on the
         * line -> king is hurdle, and what we thought to be the hurdle
         * is in fact the rose lion! */
        if (EVALUATE_OBSERVATION(evaluate,sq_hurdle,sq_target))
          return true;
      }
#endif

      if (sq_departure!=sq_target /* pieces don't give check to themselves */
          && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

boolean roselion_check(validator_id evaluate)
{
  boolean result = false;
  vec_index_type idx_curr_dir;

  ++observation_context;

  for (idx_curr_dir = vec_knight_start;
       idx_curr_dir<=vec_knight_end;
       idx_curr_dir++)
    if (detect_roselion_check_on_line(idx_curr_dir,
                                      rose_rotation_clockwise,
                                      evaluate)
        || detect_roselion_check_on_line(idx_curr_dir + vec_knight_end-vec_knight_start + 1,
                                         rose_rotation_counterclockwise,
                                         evaluate))
    {
      result = true;
      break;
    }

  --observation_context;

  return result;
}

static void rosehopper_genrerate_circle(vec_index_type rotation,
                                        rose_rotation_sense sense)
{
  square sq_hurdle = find_end_of_circle_line(curr_generation->departure,
                                             &rotation,
                                             sense);
  if (sq_hurdle!=curr_generation->departure && !is_square_blocked(sq_hurdle))
  {
    curr_generation->arrival = sq_hurdle+vec[rotation];
    if (is_square_empty(curr_generation->arrival))
      push_move_no_capture();
    else if  (curr_generation->arrival!=curr_generation->departure
              && piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

/* Generate moves for a Rose Hopper
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void rosehopper_generate_moves(vec_index_type vec_range_start,
                               vec_index_type vec_range_end)
{
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; vec_index_start++)
  {
    rosehopper_genrerate_circle(vec_index_start,
                                rose_rotation_clockwise);
    rosehopper_genrerate_circle(vec_index_start + vec_range_end-vec_range_start + 1,
                                rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

/* Detect observation on a rose hopper line
 * @param sq_hurdle position of the hurdle
 * @param idx_curr_dir indicates the direction in which to continue the line
 *                     from sq_hurdle
 * @param indicates the rotation sense
 */
static boolean detect_rosehopper_check_on_line(square sq_hurdle,
                                               vec_index_type idx_curr_dir,
                                               rose_rotation_sense sense,
                                               validator_id evaluate)
{
  interceptable_observation[observation_context].vector_index1 = idx_curr_dir;
  interceptable_observation[observation_context].auxiliary = sense;

  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    square const sq_departure = find_end_of_circle_line(sq_hurdle,
                                                        &idx_curr_dir,
                                                        sense);
    return (sq_departure!=sq_target
            && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target));
  }
}

boolean rosehopper_check(validator_id evaluate)
{
  boolean result = false;
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type idx_curr_dir;

  ++observation_context;

  for (idx_curr_dir = vec_knight_start;
       idx_curr_dir<=vec_knight_end;
       idx_curr_dir++)
  {
    square const sq_hurdle = sq_target+vec[idx_curr_dir];
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
      /* idx_curr_dir==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_rosehopper_check_on_line(sq_hurdle,
                                          idx_curr_dir+1,
                                          rose_rotation_clockwise,
                                          evaluate)
          || detect_rosehopper_check_on_line(sq_hurdle,
                                             idx_curr_dir+vec_knight_end-vec_knight_start,
                                             rose_rotation_counterclockwise,
                                             evaluate))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}

static void roselocust_generate_circle(vec_index_type idx_curr_dir,
                                       rose_rotation_sense sense)
{
  square sq_capture= find_end_of_circle_line(curr_generation->departure,
                                             &idx_curr_dir,
                                             sense);
  if (sq_capture!=curr_generation->departure
      && !is_square_blocked(sq_capture))
    generate_locust_capture(sq_capture,vec[idx_curr_dir]);
}

/* Generate moves for a Rose Locust
 * @param vec_range_start start and ...
 * @param vec_range_end ... end of range of single step vectors
 */
void roselocust_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; vec_index_start++)
  {
    roselocust_generate_circle(vec_index_start,
                               rose_rotation_clockwise);
    roselocust_generate_circle(vec_index_start + vec_range_end-vec_range_start + 1,
                               rose_rotation_counterclockwise);
  }
}

/* Detect observation on a rose hopper line
 * @param sq_arrival arrival square of the imgaginary capture
 * @param idx_curr_dir indicates the direction in which to continue the line
 *                     from the observee
 * @param indicates the rotation sense
 */
static boolean detect_roselocust_check_on_line(square sq_arrival,
                                               vec_index_type idx_curr_dir,
                                               rose_rotation_sense sense,
                                               validator_id evaluate)
{
  interceptable_observation[observation_context].vector_index1 = idx_curr_dir;
  interceptable_observation[observation_context].auxiliary = sense;

  {
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    square const sq_departure = find_end_of_circle_line(sq_target,
                                                        &idx_curr_dir,
                                                        sense);
    return (sq_departure!=sq_target
            && EVALUATE_OBSERVATION(evaluate,sq_departure,sq_arrival));
  }
}

boolean roselocust_check(validator_id evaluate)
{
  boolean result = false;
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type idx_curr_dir;

  ++observation_context;

  for (idx_curr_dir = vec_knight_start;
       idx_curr_dir<=vec_knight_end;
       idx_curr_dir++)
  {
    square const sq_arrival = sq_target-vec[idx_curr_dir];
    if (is_square_empty(sq_arrival))
    {
      /* idx_curr_dir==0 (and the equivalent
       * vec_knight_end-vec_knight_start+1) were already used for
       * sq_hurdle! */
      if (detect_roselocust_check_on_line(sq_arrival,
                                          idx_curr_dir+1,
                                          rose_rotation_clockwise,
                                          evaluate)
          || detect_roselocust_check_on_line(sq_arrival,
                                             idx_curr_dir + vec_knight_end-vec_knight_start,
                                             rose_rotation_counterclockwise,
                                             evaluate))
      {
        result = true;
        break;
      }
    }
  }

  --observation_context;

  return result;
}
