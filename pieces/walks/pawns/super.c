#include "pieces/walks/pawns/super.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

static void superpawn_generate_noncaptures(numvec dir)
{
  for (curr_generation->arrival = curr_generation->departure+dir;
       is_square_empty(curr_generation->arrival);
       curr_generation->arrival += dir)
    push_move();
}

static void superpawn_generate_captures(numvec dir)
{
  curr_generation->arrival = find_end_of_line(curr_generation->departure,dir);

  /* it can move from first rank */
  if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move();
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

boolean superpawn_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec[sq_target],base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_left);
      if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
        return true;
    }

    {
      square const sq_departure = find_end_of_line(sq_target,dir_backward+dir_right);
      if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
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

boolean superberolinapawn_check(evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  SquareFlags const base = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  if (!TSTFLAG(sq_spec[sq_target],base))
  {
    numvec const dir_backward = trait[nbply]==White ? dir_down : dir_up;
    square const sq_departure = find_end_of_line(sq_target,dir_backward);
    if (INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}
