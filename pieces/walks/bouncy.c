#include "pieces/walks/bouncy.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "position/position.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static void bouncy_knight_generate_moves_recursive(square step_departure, int x)
{
  vec_index_type k;

  /* ATTENTION:   first call of grefcn: x must be 2 !!   */

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    curr_generation->arrival = step_departure+vec[k];
    if (is_square_empty(curr_generation->arrival))
    {
      push_move_no_capture();
      if (x>0 && !NoEdge(curr_generation->arrival))
        bouncy_knight_generate_moves_recursive(curr_generation->arrival,x-1);
    }
    else if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

/* Generate moves for a bouncy night
 */
void bouncy_knight_generate_moves(void)
{
  bouncy_knight_generate_moves_recursive(curr_generation->departure,2);
}

enum
{
  nr_edge_squares_row = nr_files_on_board,
  nr_edge_squares_file = nr_rows_on_board-2, /* count corners once only */

  nr_edge_squares = 2*nr_edge_squares_row + 2*nr_edge_squares_file
};

typedef square edge_square_index;

/* remember edge traversals by reflecting pieces
 */
static boolean edgestraversed[nr_edge_squares];

/* clear edge traversal memory
 */
void clearedgestraversed(void)
{
  int i;
  for (i=0; i<nr_edge_squares; i++)
    edgestraversed[i] = false;
}

/* map edge squares to indices into edgestraversed
 * the mapping occurs in the order
 * - bottom row
 * - top row
 * - left file
 * - right file
 * @param edge_square square on board edge
 * @return index into edgestraversed where to remember traversal of
 *         edge_square
 */
static edge_square_index square_2_edge_square_index(square edge_square)
{
  int const row =  edge_square/onerow;
  int const file = edge_square%onerow;
  edge_square_index result;

  assert(row==bottom_row || row==top_row
         || file==left_file || file==right_file);
  if (row==bottom_row)
    result = file-left_file;
  else if (row==top_row)
    result = file-left_file + nr_edge_squares_row;
  else if (file==left_file)
    result = row-bottom_row-1 + 2*nr_edge_squares_row;
  else
    result = row-bottom_row-1 + 2*nr_edge_squares_row + nr_edge_squares_file;

  assert(result<nr_edge_squares);
  return result;
}

/* query traversal of an edge square
 * @param edge_square square on board edge
 * @return has edge_square been traversed?
 */
boolean traversed(square edge_square)
{
  return edgestraversed[square_2_edge_square_index(edge_square)];
}

/* remember traversal of an edge square
 * @param edge_square square on board edge
 */
void settraversed(square edge_square)
{
  edgestraversed[square_2_edge_square_index(edge_square)] = true;
}

static void bouncy_nightrider_generate_moves_recursive(square step_departure)
{
  vec_index_type k;

  if (!NoEdge(step_departure))
    settraversed(step_departure);

  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    curr_generation->arrival = step_departure+vec[k];

    while (is_square_empty(curr_generation->arrival))
    {
      push_move_no_capture();
      if (!NoEdge(curr_generation->arrival) && !traversed(curr_generation->arrival))
      {
        bouncy_nightrider_generate_moves_recursive(curr_generation->arrival);
        break;
      }
      else
        curr_generation->arrival += vec[k];
    }

    if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

/* Generate moves for a bouncy nightrider
 */
void bouncy_nightrider_generate_moves(void)
{
  clearedgestraversed();
  bouncy_nightrider_generate_moves_recursive(curr_generation->departure);
}

static boolean rrefcech(square i1, int x, validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type k;

  /* ATTENTION:   first call of rrefech: x must be 2 !!   */

  if (x)
  {
    for (k= vec_knight_start; k <= vec_knight_end; k++)
    {
      square const sq_departure= i1+vec[k];
      if (is_square_empty(sq_departure))
      {
        if (!NoEdge(sq_departure)) {
              if (rrefcech(sq_departure,x-1,evaluate))
                return true;
        }
      }
      else if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }
  }
  else
  {
    for (k= vec_knight_start; k <= vec_knight_end; k++)
    {
      square const sq_departure= i1+vec[k];
      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

boolean bouncy_knight_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  return rrefcech(sq_target, 2, evaluate);
}

static boolean rrefnech(square i1, validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type k;

  if (!NoEdge(i1))
    settraversed(i1);

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    square sq_departure = i1+vec[k];

    while (is_square_empty(sq_departure))
    {
      if (!NoEdge(sq_departure) &&
          !traversed(sq_departure)) {
        if (rrefnech(sq_departure,evaluate))
          return true;
        break;
      }
      sq_departure += vec[k];
    }

    if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      return true;
  }
  return false;
}

boolean bouncy_nightrider_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  clearedgestraversed();
  return rrefnech(sq_target, evaluate);
}
