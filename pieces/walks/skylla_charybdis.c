#include "pieces/walks/skylla_charybdis.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

static void generate_one_move(square sq_arrival, square sq_capture)
{
  if (is_square_empty(sq_arrival))
  {
    if (is_square_empty(sq_capture))
      push_move_generation(sq_arrival);
    else if (piece_belongs_to_opponent(sq_capture))
      push_move_generation_capture_extra(sq_arrival,sq_capture);
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
