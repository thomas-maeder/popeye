#include "pieces/walks/roses.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static square generate_moves_on_circle_segment(square sq_base,
                                               vec_index_type vec_index_start,
                                               vec_index_type *rotation,
                                               rose_rotation_direction direction)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_base);
  TraceFunctionParam("%u",vec_index_start);
  TraceFunctionParam("%d",*rotation);
  TraceFunctionParam("%d",direction);
  TraceFunctionParamListEnd();

  curr_generation->arrival = sq_base+vec[vec_index_start+*rotation];

  *rotation += direction;

  while (curr_generation->arrival!=sq_base
         && is_square_empty(curr_generation->arrival))
  {
    push_move();
    curr_generation->arrival += vec[vec_index_start+*rotation];
    *rotation += direction;
  }

  TraceFunctionExit(__func__);
  TraceSquare(curr_generation->arrival);
  TraceFunctionResultEnd();
  return curr_generation->arrival;
}

square find_end_of_circle_line(square sq_departure,
                               vec_index_type vec_index_start, vec_index_type *rotation,
                               rose_rotation_direction direction)
{
  square sq_result = sq_departure;
  do {
    sq_result += vec[vec_index_start+*rotation];
    *rotation += direction;
  } while (is_square_empty(sq_result));

  return sq_result;
}

static void rose_generate_circle(vec_index_type vec_index_start,
                                 vec_index_type vec_range_start, vec_index_type vec_range_end,
                                 rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",vec_index_start);
  TraceFunctionParam("%d",rotation);
  TraceFunctionParam("%u",vec_range_start);
  TraceFunctionParam("%u",vec_range_end);
  TraceFunctionParam("%d",direction);
  TraceFunctionParamListEnd();

  {
    curr_generation->arrival = generate_moves_on_circle_segment(curr_generation->departure,vec_index_start,&rotation,direction);
    if (curr_generation->arrival!=curr_generation->departure
        && piece_belongs_to_opponent(curr_generation->arrival))
      push_move();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void rose_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    rose_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    rose_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void rao_generate_circle(vec_index_type vec_index_start,
                                vec_index_type vec_range_start, vec_index_type vec_range_end,
                                rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_hurdle = generate_moves_on_circle_segment(curr_generation->departure,
                                                      vec_index_start,&rotation,direction);
  if (sq_hurdle!=curr_generation->departure && !is_square_blocked(sq_hurdle))
  {
    curr_generation->arrival = find_end_of_circle_line(sq_hurdle,vec_index_start,&rotation,direction);
    if (curr_generation->arrival!=curr_generation->departure
        && piece_belongs_to_opponent(curr_generation->arrival))
      push_move();
  }
}

void rao_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    rao_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    rao_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void roselion_generate_circle(vec_index_type vec_index_start,
                                     vec_index_type vec_range_start, vec_index_type vec_range_end,
                                     rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_hurdle = find_end_of_circle_line(curr_generation->departure,vec_index_start,&rotation,direction);
  if (sq_hurdle!=curr_generation->departure && !is_square_blocked(sq_hurdle))
  {
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    /* cf. issue 1747928 */
    /* temporarily remove the moving piece to prevent it from blocking
     * itself */
    piece save_piece = e[curr_generation->departure];
    e[curr_generation->departure] = vide;
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
                                                                vec_index_start,&rotation,direction);
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    e[curr_generation->departure] = save_piece;
#endif
    if (curr_generation->arrival!=curr_generation->departure
        && piece_belongs_to_opponent(curr_generation->arrival))
      push_move();
  }
}

void roselion_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type vec_index_start;
  for (vec_index_start= vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    roselion_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    roselion_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void rosehopper_genrerate_circle(vec_index_type vec_index_start,
                                        vec_index_type vec_range_start, vec_index_type vec_range_end,
                                        rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_hurdle= find_end_of_circle_line(curr_generation->departure,vec_index_start,&rotation,direction);
  if (sq_hurdle!=curr_generation->departure && !is_square_blocked(sq_hurdle))
  {
    curr_generation->arrival = sq_hurdle+vec[vec_index_start+rotation];
    if (is_square_empty(curr_generation->arrival)
        || (curr_generation->arrival!=curr_generation->departure
            && piece_belongs_to_opponent(curr_generation->arrival)))
      push_move();
  }
}

void rosehopper_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply]-1;

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; vec_index_start++)
  {
    rosehopper_genrerate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    rosehopper_genrerate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void roselocust_generate_circle(vec_index_type vec_index_start,
                                       vec_index_type vec_range_start, vec_index_type vec_range_end,
                                       rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_capture= find_end_of_circle_line(curr_generation->departure,vec_index_start,&rotation,direction);
  if (sq_capture!=curr_generation->departure
      && !is_square_blocked(sq_capture)
      && piece_belongs_to_opponent(sq_capture))
  {
    curr_generation->arrival = sq_capture+vec[vec_index_start+rotation];
    if (is_square_empty(curr_generation->arrival))
      push_move_capture_extra(sq_capture);
  }
}

void roselocust_generate_moves(vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; vec_index_start++)
  {
    roselocust_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    roselocust_generate_circle(vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }
}
