#include "conditions/follow_my_leader.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

/* Determine the length of a move for the Follow my leader condition; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int follow_my_leader_measure_length(void)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;

  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
    return sq_arrival==move_effect_journal[movement].u.piece_movement.from;
  else
    return true;
}
