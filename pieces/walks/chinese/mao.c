#include "pieces/walks/chinese/mao.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static void gmaooa(square  pass,
                   square  arrival1,
                   square  arrival2)
{
  if (is_square_empty(pass))
  {
    if (is_square_empty(arrival1)
        || piece_belongs_to_opponent(arrival1))
    {
      curr_generation->auxiliary.hopper.sq_hurdle = pass;
      curr_generation->arrival = arrival1;
      push_move();
    }

    if (is_square_empty(arrival2)
        || piece_belongs_to_opponent(arrival2))
    {
      curr_generation->auxiliary.hopper.sq_hurdle = pass;
      curr_generation->arrival = arrival2;
      push_move();
    }

    curr_generation->auxiliary.hopper.sq_hurdle = initsquare;
  }
}

/* Generate moves for a Mao
 */
void mao_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;

  gmaooa(sq_departure+dir_up, sq_departure+2*dir_up+dir_left, sq_departure+2*dir_up+dir_right);
  gmaooa(sq_departure+dir_down, sq_departure+2*dir_down+dir_right, sq_departure+2*dir_down+dir_left);
  gmaooa(sq_departure+dir_right, sq_departure+dir_up+2*dir_right, sq_departure+dir_down+2*dir_right);
  gmaooa(sq_departure+dir_left, sq_departure+dir_down+2*dir_left, sq_departure+dir_up+2*dir_left);
}

/* Generate moves for a Moa
 */
void moa_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;

  gmaooa(sq_departure+dir_up+dir_left, sq_departure+2*dir_up+dir_left, sq_departure+dir_up+2*dir_left);
  gmaooa(sq_departure+dir_down+dir_right, sq_departure+2*dir_down+dir_right, sq_departure+dir_down+2*dir_right);
  gmaooa(sq_departure+dir_up+dir_right, sq_departure+dir_up+2*dir_right, sq_departure+2*dir_up+dir_right);
  gmaooa(sq_departure+dir_down+dir_left, sq_departure+dir_down+2*dir_left, sq_departure+2*dir_down+dir_left);
}

static void gemaooarider(numvec tomiddle, numvec todest)
{
  square const sq_departure = curr_generation->departure;

  square middle = sq_departure+tomiddle;
  curr_generation->arrival = sq_departure+todest;

  while (is_square_empty(middle) && is_square_empty(curr_generation->arrival))
  {
    push_move();
    middle += todest;
    curr_generation->arrival += todest;
  }

  if (is_square_empty(middle)
      && piece_belongs_to_opponent(curr_generation->arrival))
    push_move();
}

/* Generate moves for a Moa Rider
 */
void moarider_generate_moves(void)
{
  gemaooarider(+dir_up+dir_left,+2*dir_up+dir_left);
  gemaooarider(+dir_up+dir_left,+dir_up+2*dir_left);
  gemaooarider(+dir_down+dir_right,+2*dir_down+dir_right);
  gemaooarider(+dir_down+dir_right,+dir_down+2*dir_right);
  gemaooarider(+dir_up+dir_right,+dir_up+2*dir_right);
  gemaooarider(+dir_up+dir_right,+2*dir_up+dir_right);
  gemaooarider(+dir_down+dir_left,+dir_down+2*dir_left);
  gemaooarider(+dir_down+dir_left,+2*dir_down+dir_left);
}

/* Generate moves for a Mao Rider
 */
void maorider_generate_moves(void)
{
  gemaooarider(+dir_right,+dir_up+2*dir_right);
  gemaooarider(+dir_right,+dir_down+2*dir_right);
  gemaooarider(+dir_down,+2*dir_down+dir_right);
  gemaooarider(+dir_down,+2*dir_down+dir_left);
  gemaooarider(+dir_left,+dir_down+2*dir_left);
  gemaooarider(+dir_left,+dir_up+2*dir_left);
  gemaooarider(+dir_up,+2*dir_up+dir_left);
  gemaooarider(+dir_up,+2*dir_up+dir_right);
}

static void gemaooariderlion(numvec tomiddle, numvec todest)
{
  square const sq_departure = curr_generation->departure;
  square middle = sq_departure + tomiddle;
  curr_generation->arrival = sq_departure+todest;

  while (is_square_empty(middle) && is_square_empty(curr_generation->arrival))
  {
    middle += todest;
    curr_generation->arrival += todest;
  }

  if (!is_square_blocked(middle) && !is_square_blocked(curr_generation->arrival))
  {
    if (!is_square_empty(middle)
        && (is_square_empty(curr_generation->arrival)
            || piece_belongs_to_opponent(curr_generation->arrival)))
      push_move();
    if (is_square_empty(middle) || is_square_empty(curr_generation->arrival))
    {
      middle += todest;
      curr_generation->arrival += todest;
      while (is_square_empty(middle) && is_square_empty(curr_generation->arrival))
      {
        push_move();
        middle += todest;
        curr_generation->arrival += todest;
      }
    }

    if (is_square_empty(middle)
        && piece_belongs_to_opponent(curr_generation->arrival))
      push_move();
  }
}

/* Generate moves for a Mao Rider Lion
 */
void maoriderlion_generate_moves(void)
{
  gemaooariderlion(+dir_right,+dir_up+2*dir_right);
  gemaooariderlion(+dir_right,+dir_down+2*dir_right);
  gemaooariderlion(+dir_down,+2*dir_down+dir_right);
  gemaooariderlion(+dir_down,+2*dir_down+dir_left);
  gemaooariderlion(+dir_left,+dir_down+2*dir_left);
  gemaooariderlion(+dir_left,+dir_up+2*dir_left);
  gemaooariderlion(+dir_up,+2*dir_up+dir_left);
  gemaooariderlion(+dir_up,+2*dir_up+dir_right);
}

/* Generate moves for a Moa Rider Lion
 */
void moariderlion_generate_moves(void)
{
  gemaooariderlion(+dir_up+dir_left,+2*dir_up+dir_left);
  gemaooariderlion(+dir_up+dir_left,+dir_up+2*dir_left);
  gemaooariderlion(+dir_down+dir_right,+2*dir_down+dir_right);
  gemaooariderlion(+dir_down+dir_right,+dir_down+2*dir_right);
  gemaooariderlion(+dir_up+dir_right,+dir_up+2*dir_right);
  gemaooariderlion(+dir_up+dir_right,+2*dir_up+dir_right);
  gemaooariderlion(+dir_down+dir_left,+dir_down+2*dir_left);
  gemaooariderlion(+dir_down+dir_left,+2*dir_down+dir_left);
}
