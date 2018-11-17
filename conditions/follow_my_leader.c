#include "conditions/follow_my_leader.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Determine the length of a move for the Follow my leader condition; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type follow_my_leader_measure_length(void)
{
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;

  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_base[parent+1];
  if (movement<parent_top)
    return sq_arrival==move_effect_journal[movement].u.piece_movement.from;
  else
    return true;
}
