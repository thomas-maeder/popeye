#include "pieces/walks/skylla_charybdis.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"

static void generate_one_move(square sq_arrival, square sq_capture)
{
  if (is_square_empty(sq_arrival))
  {
    if (is_square_empty(sq_capture))
    {
      curr_generation->arrival = sq_arrival;
      push_move();
    }
    else if (piece_belongs_to_opponent(sq_capture))
    {
      curr_generation->arrival = sq_arrival;
      push_move_capture_extra(sq_capture);
    }
  }
}

/* Generate moves for a Skylla
 */
void skylla_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;

  generate_one_move(sq_departure+dir_up+2*dir_right, sq_departure+dir_right);
  generate_one_move(sq_departure+2*dir_up+dir_right, sq_departure+dir_up);
  generate_one_move(sq_departure+2*dir_up+dir_left, sq_departure+dir_up);
  generate_one_move(sq_departure+dir_up+2*dir_left, sq_departure+dir_left);
  generate_one_move(sq_departure+dir_down+2*dir_left, sq_departure+dir_left);
  generate_one_move(sq_departure+2*dir_down+dir_left, sq_departure+dir_down);
  generate_one_move(sq_departure+2*dir_down+dir_right, sq_departure+dir_down);
  generate_one_move(sq_departure+dir_down+2*dir_right, sq_departure+dir_right);
}

/* Generate moves for a Charybdis
 */
void charybdis_generate_moves(void)
{
  square const sq_departure = curr_generation->departure;

  generate_one_move(sq_departure+dir_up+2*dir_right, sq_departure+dir_up+dir_right);
  generate_one_move(sq_departure+2*dir_up+dir_right, sq_departure+dir_up+dir_right);
  generate_one_move(sq_departure+2*dir_up+dir_left, sq_departure+dir_up+dir_left);
  generate_one_move(sq_departure+dir_up+2*dir_left, sq_departure+dir_up+dir_left);
  generate_one_move(sq_departure+dir_down+2*dir_left, sq_departure+dir_down+dir_left);
  generate_one_move(sq_departure+2*dir_down+dir_left, sq_departure+dir_down+dir_left);
  generate_one_move(sq_departure+2*dir_down+dir_right, sq_departure+dir_down+dir_right);
  generate_one_move(sq_departure+dir_down+2*dir_right, sq_departure+dir_down+dir_right);
}

static boolean skycharcheck(square chp,
                            square sq_arrival1,
                            square sq_arrival2,
                            evalfunction_t *evaluate)
{
  if (is_square_empty(sq_arrival1) && INVOKE_EVAL(evaluate,chp,sq_arrival1))
    return  true;

  if (is_square_empty(sq_arrival2) && INVOKE_EVAL(evaluate,chp,sq_arrival2))
    return  true;

  return  false;
}

boolean skylla_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  return  skycharcheck(sq_target+dir_right, sq_target+dir_up+dir_left, sq_target+dir_down+dir_left, evaluate)
       || skycharcheck(sq_target+dir_left, sq_target+dir_up+dir_right, sq_target+dir_down+dir_right, evaluate)
       || skycharcheck(sq_target+dir_up, sq_target+dir_down+dir_right, sq_target+dir_down+dir_left, evaluate)
       || skycharcheck(sq_target+dir_down, sq_target+dir_up+dir_left, sq_target+dir_up+dir_right, evaluate);
}

boolean charybdis_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  return  skycharcheck(sq_target+dir_up+dir_right, sq_target+dir_left, sq_target - 24, evaluate)
       || skycharcheck(sq_target+dir_down+dir_left, sq_target+dir_right, sq_target + 24, evaluate)
       || skycharcheck(sq_target+dir_up+dir_left, sq_target+dir_right, sq_target - 24, evaluate)
       || skycharcheck(sq_target+dir_down+dir_right, sq_target+dir_left, sq_target + 24, evaluate);
}
