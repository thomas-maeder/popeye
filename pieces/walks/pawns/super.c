#include "pieces/walks/pawns/super.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

static void superpawn_generate_noncaptures(numvec dir)
{
  for (curr_generation->arrival = curr_generation->departure+dir;
       is_square_empty(curr_generation->arrival);
       curr_generation->arrival += dir)
    push_move_no_capture();
}

static void superpawn_generate_captures(numvec dir)
{
  curr_generation->arrival = find_end_of_line(curr_generation->departure,dir);

  /* it can move from first rank */
  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move_regular_capture();
}

/* Generate moves for a Super Pawn
 */
void super_pawn_generate_moves(void)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  superpawn_generate_noncaptures(dir_forward);
  superpawn_generate_captures(dir_forward+dir_left);
  superpawn_generate_captures(dir_forward+dir_right);
}

boolean superpawn_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec(sq_target),base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_left);
      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_right);
      if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

/* Generate moves for a Super-Berolina Pawn
 */
void super_berolina_pawn_generate_moves(void)
{
  int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  superpawn_generate_noncaptures(dir_forward+dir_left);
  superpawn_generate_noncaptures(dir_forward+dir_right);
  superpawn_generate_captures(dir_forward);
}

boolean superberolinapawn_check(validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec(sq_target),base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;
    square const sq_departure = find_end_of_line(sq_target,dir_backward);
    if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}
