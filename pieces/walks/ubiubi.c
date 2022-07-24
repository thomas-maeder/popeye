#include "pieces/walks/ubiubi.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/fork.h"
#include "debugging/assert.h"

typedef enum
{
  ubiubi_empty,
  ubiubi_opposite,
  ubiubi_taboo
} ubiubi_square_state_type;

typedef ubiubi_square_state_type ubiubi_traversal_state_type[maxsquare+4];

static void utiubi_generate_moves_recursive(square step_departure,
                                            ubiubi_traversal_state_type traversal_state)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    curr_generation->arrival = step_departure+vec[k];
    if (!is_square_blocked(curr_generation->arrival))
      switch (traversal_state[curr_generation->arrival])
      {
        case ubiubi_empty:
          push_move_no_capture();
          traversal_state[curr_generation->arrival] = ubiubi_taboo;
          utiubi_generate_moves_recursive(curr_generation->arrival,traversal_state);
          break;

        case ubiubi_opposite:
          push_move_regular_capture();
          traversal_state[curr_generation->arrival] = ubiubi_taboo;
          break;

        case ubiubi_taboo:
          break;

        default:
          assert(0);
          break;
      }
  }
}

void ubiubi_generate_moves(void)
{
  Side const opposite = advers(trait[nbply]);
  ubiubi_traversal_state_type board_state;

  square const *bnp;
  for (bnp = boardnum; *bnp; ++bnp)
    if (is_square_empty(*bnp))
      board_state[*bnp] = ubiubi_empty;
    else if (TSTFLAG(being_solved.spec[*bnp],opposite))
      board_state[*bnp] = ubiubi_opposite;
    else
      board_state[*bnp] = ubiubi_taboo;

  board_state[curr_generation->departure] = ubiubi_taboo; /* for neutral UbiUbis */

  utiubi_generate_moves_recursive(curr_generation->departure,board_state);
}

static boolean ubiubi_check_recursive(square intermediate_square,
                                      ubiubi_traversal_state_type traversal_state,
                                      validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type k;

  traversal_state[intermediate_square] = ubiubi_taboo;

  for (k = vec_knight_start; k<=vec_knight_end; k++)
  {
    square const sq_departure = intermediate_square+vec[k];
    if (!is_square_blocked(sq_departure))
    {
      if (traversal_state[sq_departure]==ubiubi_empty)
      {
        if (ubiubi_check_recursive(sq_departure,traversal_state,evaluate))
          return true;
      }
      else if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
        return true;
    }
  }

  return false;
}

boolean ubiubi_check(validator_id evaluate)
{
  ubiubi_traversal_state_type board_state;
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  square const *bnp;
  for (bnp = boardnum; *bnp; ++bnp)
    board_state[*bnp] = is_square_empty(*bnp) ? ubiubi_empty : ubiubi_taboo;

  return ubiubi_check_recursive(sq_target,board_state,evaluate);
}
