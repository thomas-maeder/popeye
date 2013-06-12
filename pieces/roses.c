#include "pieces/roses.h"
#include "pydata.h"
#include "pyproc.h"
#include "debugging/trace.h"

static square generate_moves_on_circle_segment(square sq_departure,
                                               square sq_base,
                                               vec_index_type vec_index_start,
                                               vec_index_type *rotation,
                                               rose_rotation_direction direction)
{
  square sq_arrival = sq_base;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_base);
  TraceFunctionParam("%u",vec_index_start);
  TraceFunctionParam("%d",*rotation);
  TraceFunctionParam("%d",direction);
  TraceFunctionParamListEnd();

  do {
    sq_arrival += vec[vec_index_start+*rotation];
    *rotation += direction;
  } while (sq_arrival!=sq_base
           && is_square_empty(sq_arrival)
           && empile(sq_departure,sq_arrival,sq_arrival));

  TraceFunctionExit(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionResultEnd();
  return sq_arrival;
}

square find_end_of_circle_line(square sq_departure,
                               vec_index_type vec_index_start, vec_index_type *rotation,
                               rose_rotation_direction direction)
{
  square sq_result= sq_departure;
  do {
    sq_result += vec[vec_index_start+*rotation];
    *rotation += direction;
  } while (is_square_empty(sq_result));

  return sq_result;
}

static void rose_generate_circle(Side side,
                                 square sq_departure,
                                 vec_index_type vec_index_start,
                                 vec_index_type vec_range_start, vec_index_type vec_range_end,
                                 rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%u",vec_index_start);
  TraceFunctionParam("%d",rotation);
  TraceFunctionParam("%u",vec_range_start);
  TraceFunctionParam("%u",vec_range_end);
  TraceFunctionParam("%d",direction);
  TraceFunctionParamListEnd();

  {
    square const sq_arrival = generate_moves_on_circle_segment(sq_departure,sq_departure,
                                                               vec_index_start,&rotation,direction);
    if (sq_arrival!=sq_departure && piece_belongs_to_opponent(sq_arrival,side))
      empile(sq_departure,sq_arrival,sq_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void rose_generate_moves(Side side, square sq_departure,
                         vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    rose_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    rose_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void rao_generate_circle(Side side,
                                square sq_departure,
                                vec_index_type vec_index_start,
                                vec_index_type vec_range_start, vec_index_type vec_range_end,
                                rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_hurdle = generate_moves_on_circle_segment(sq_departure,sq_departure,
                                                      vec_index_start,&rotation,direction);
  if (sq_hurdle!=sq_departure && !is_square_blocked(sq_hurdle))
  {
    square const sq_arrival = find_end_of_circle_line(sq_hurdle,vec_index_start,&rotation,direction);
    if (sq_arrival!=sq_departure && piece_belongs_to_opponent(sq_arrival,side))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void rao_generate_moves(Side side, square sq_departure,
                        vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    rao_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    rao_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void roselion_generate_circle(Side side,
                                     square sq_departure,
                                     vec_index_type vec_index_start,
                                     vec_index_type vec_range_start, vec_index_type vec_range_end,
                                     rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_hurdle = find_end_of_circle_line(sq_departure,vec_index_start,&rotation,direction);
  if (sq_hurdle!=sq_departure && !is_square_blocked(sq_hurdle))
  {
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    /* cf. issue 1747928 */
    /* temporarily remove the moving piece to prevent it from blocking
     * itself */
    piece save_piece = e[sq_departure];
    e[sq_departure] = vide;
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
    square const sq_arrival = generate_moves_on_circle_segment(sq_departure,sq_hurdle,
                                                               vec_index_start,&rotation,direction);
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    e[sq_departure] = save_piece;
#endif
    if (sq_arrival!=sq_departure && piece_belongs_to_opponent(sq_arrival,side))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void roselion_generate_moves(Side side, square sq_departure,
                             vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type vec_index_start;
  for (vec_index_start= vec_range_start; vec_index_start<=vec_range_end; ++vec_index_start)
  {
    roselion_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    roselion_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void rosehopper_genrerate_circle(Side side,
                                        square sq_departure,
                                        vec_index_type vec_index_start,
                                        vec_index_type vec_range_start, vec_index_type vec_range_end,
                                        rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_hurdle= find_end_of_circle_line(sq_departure,vec_index_start,&rotation,direction);
  if (sq_hurdle!=sq_departure && !is_square_blocked(sq_hurdle))
  {
    square sq_arrival= sq_hurdle+vec[vec_index_start+rotation];
    if (is_square_empty(sq_arrival)
        || (sq_arrival!=sq_departure && piece_belongs_to_opponent(sq_arrival,side)))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void rosehopper_generate_moves(Side side, square sq_departure,
                               vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  numecoup const save_current_move = current_move[nbply];

  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; vec_index_start++)
  {
    rosehopper_genrerate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    rosehopper_genrerate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }

  remove_duplicate_moves_of_single_piece(save_current_move);
}

static void roselocust_generate_circle(Side side,
                                       square sq_departure,
                                       vec_index_type vec_index_start,
                                       vec_index_type vec_range_start, vec_index_type vec_range_end,
                                       rose_rotation_direction direction)
{
  vec_index_type rotation = direction==rose_rotation_clockwise ? 0 : vec_range_end-vec_range_start+1;

  square sq_capture= find_end_of_circle_line(sq_departure,vec_index_start,&rotation,direction);
  if (sq_capture!=sq_departure && !is_square_blocked(sq_capture) && piece_belongs_to_opponent(sq_capture,side))
  {
    square sq_arrival = sq_capture+vec[vec_index_start+rotation];
    if (is_square_empty(sq_arrival))
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

void roselocust_generate_moves(Side side, square sq_departure,
                               vec_index_type vec_range_start, vec_index_type vec_range_end)
{
  vec_index_type vec_index_start;
  for (vec_index_start = vec_range_start; vec_index_start<=vec_range_end; vec_index_start++)
  {
    roselocust_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_clockwise);
    roselocust_generate_circle(side, sq_departure, vec_index_start, vec_range_start,vec_range_end, rose_rotation_counterclockwise);
  }
}
