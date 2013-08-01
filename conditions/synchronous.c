#include "conditions/synchronous.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

/* Determine the length of a move in Synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int synchronous_measure_length(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;

  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
  {
    square const sq_parent_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
    numvec const diff = sq_departure-sq_arrival;
    return diff==parent_diff;
  }
  else
    return true;
}

/* Determine the length of a move in Anti-synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int antisynchronous_measure_length(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;

  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
  {
    square const sq_parent_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
    numvec const diff = sq_departure-sq_arrival;
    return diff==-parent_diff;
  }
  else
    return true;
}
