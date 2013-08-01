#include "pieces/walks/chinese/mao.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static void gmaooa(square  sq_departure,
                   square  pass,
                   square  arrival1,
                   square  arrival2)
{
  if (is_square_empty(pass))
  {
    if (is_square_empty(arrival1)
        || piece_belongs_to_opponent(arrival1))
    {
      move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = pass;
      add_to_move_generation_stack(sq_departure,arrival1,arrival1);
    }

    if (is_square_empty(arrival2)
        || piece_belongs_to_opponent(arrival2))
    {
      move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = pass;
      add_to_move_generation_stack(sq_departure,arrival2,arrival2);
    }
  }
}

/* Generate moves for a Mao
 * @param sq_departure common departure square of the generated moves
 */
void mao_generate_moves(square sq_departure)
{
  gmaooa(sq_departure, sq_departure+dir_up, sq_departure+2*dir_up+dir_left, sq_departure+2*dir_up+dir_right);
  gmaooa(sq_departure, sq_departure+dir_down, sq_departure+2*dir_down+dir_right, sq_departure+2*dir_down+dir_left);
  gmaooa(sq_departure, sq_departure+dir_right, sq_departure+dir_up+2*dir_right, sq_departure+dir_down+2*dir_right);
  gmaooa(sq_departure, sq_departure+dir_left, sq_departure+dir_down+2*dir_left, sq_departure+dir_up+2*dir_left);
}

/* Generate moves for a Moa
 * @param sq_departure common departure square of the generated moves
 */
void moa_generate_moves(square sq_departure)
{
  gmaooa(sq_departure, sq_departure+dir_up+dir_left, sq_departure+2*dir_up+dir_left, sq_departure+dir_up+2*dir_left);
  gmaooa(sq_departure, sq_departure+dir_down+dir_right, sq_departure+2*dir_down+dir_right, sq_departure+dir_down+2*dir_right);
  gmaooa(sq_departure, sq_departure+dir_up+dir_right, sq_departure+dir_up+2*dir_right, sq_departure+2*dir_up+dir_right);
  gmaooa(sq_departure, sq_departure+dir_down+dir_left, sq_departure+dir_down+2*dir_left, sq_departure+2*dir_down+dir_left);
}

static void gemaooarider(square sq_departure, numvec tomiddle, numvec todest)
{
  square middle= sq_departure+tomiddle;
  square sq_arrival= sq_departure+todest;

  while (is_square_empty(middle) && is_square_empty(sq_arrival))
  {
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
    middle += todest;
    sq_arrival += todest;
  }

  if (is_square_empty(middle)
      && piece_belongs_to_opponent(sq_arrival))
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
}

/* Generate moves for a Moa Rider
 * @param sq_departure common departure square of the generated moves
 */
void moarider_generate_moves(square i)
{
  gemaooarider(i,+dir_up+dir_left,+2*dir_up+dir_left);
  gemaooarider(i,+dir_up+dir_left,+dir_up+2*dir_left);
  gemaooarider(i,+dir_down+dir_right,+2*dir_down+dir_right);
  gemaooarider(i,+dir_down+dir_right,+dir_down+2*dir_right);
  gemaooarider(i,+dir_up+dir_right,+dir_up+2*dir_right);
  gemaooarider(i,+dir_up+dir_right,+2*dir_up+dir_right);
  gemaooarider(i,+dir_down+dir_left,+dir_down+2*dir_left);
  gemaooarider(i,+dir_down+dir_left,+2*dir_down+dir_left);
}

/* Generate moves for a Mao Rider
 * @param sq_departure common departure square of the generated moves
 */
void maorider_generate_moves(square i)
{
  gemaooarider(i,+dir_right,+dir_up+2*dir_right);
  gemaooarider(i,+dir_right,+dir_down+2*dir_right);
  gemaooarider(i,+dir_down,+2*dir_down+dir_right);
  gemaooarider(i,+dir_down,+2*dir_down+dir_left);
  gemaooarider(i,+dir_left,+dir_down+2*dir_left);
  gemaooarider(i,+dir_left,+dir_up+2*dir_left);
  gemaooarider(i,+dir_up,+2*dir_up+dir_left);
  gemaooarider(i,+dir_up,+2*dir_up+dir_right);
}

static void gemaooariderlion(square sq_departure, numvec tomiddle, numvec todest)
{
  square middle= sq_departure + tomiddle;
  square sq_arrival= sq_departure+todest;

  while (is_square_empty(middle) && is_square_empty(sq_arrival))
  {
    middle += todest;
    sq_arrival += todest;
  }

  if (!is_square_blocked(middle) && !is_square_blocked(sq_arrival))
  {
    if (!is_square_empty(middle)
        && (is_square_empty(sq_arrival)
            || piece_belongs_to_opponent(sq_arrival)))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
    if (is_square_empty(middle) || is_square_empty(sq_arrival))
    {
      middle += todest;
      sq_arrival += todest;
      while (is_square_empty(middle) && is_square_empty(sq_arrival))
      {
        add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
        middle += todest;
        sq_arrival += todest;
      }
    }

    if (is_square_empty(middle)
        && piece_belongs_to_opponent(sq_arrival))
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_arrival);
  }
}

/* Generate moves for a Mao Rider Lion
 * @param sq_departure common departure square of the generated moves
 */
void maoriderlion_generate_moves(square i)
{
  gemaooariderlion(i,+dir_right,+dir_up+2*dir_right);
  gemaooariderlion(i,+dir_right,+dir_down+2*dir_right);
  gemaooariderlion(i,+dir_down,+2*dir_down+dir_right);
  gemaooariderlion(i,+dir_down,+2*dir_down+dir_left);
  gemaooariderlion(i,+dir_left,+dir_down+2*dir_left);
  gemaooariderlion(i,+dir_left,+dir_up+2*dir_left);
  gemaooariderlion(i,+dir_up,+2*dir_up+dir_left);
  gemaooariderlion(i,+dir_up,+2*dir_up+dir_right);
}

/* Generate moves for a Moa Rider Lion
 * @param sq_departure common departure square of the generated moves
 */
void moariderlion_generate_moves(square i)
{
  gemaooariderlion(i,+dir_up+dir_left,+2*dir_up+dir_left);
  gemaooariderlion(i,+dir_up+dir_left,+dir_up+2*dir_left);
  gemaooariderlion(i,+dir_down+dir_right,+2*dir_down+dir_right);
  gemaooariderlion(i,+dir_down+dir_right,+dir_down+2*dir_right);
  gemaooariderlion(i,+dir_up+dir_right,+dir_up+2*dir_right);
  gemaooariderlion(i,+dir_up+dir_right,+2*dir_up+dir_right);
  gemaooariderlion(i,+dir_down+dir_left,+dir_down+2*dir_left);
  gemaooariderlion(i,+dir_down+dir_left,+2*dir_down+dir_left);
}
