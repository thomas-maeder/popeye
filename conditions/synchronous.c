#include "conditions/synchronous.h"
#include "position/piece_movement.h"
#include "solving/move_generator.h"
#include "pieces/walks/vectors.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Determine the length of a move in Synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type synchronous_measure_length(void)
{
  int result;
  ply const parent = parent_ply[nbply];
  square const sq_parent_departure = move_effect_journal_get_departure_square(parent);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (sq_parent_departure==initsquare)
    result = true;
  else
  {
    move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
    move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
    square const sq_parent_arrival = move_effect_journal[parent_movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;

    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
    numvec const diff = sq_departure-sq_arrival;

    result = diff==parent_diff;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the length of a move in Anti-synchronous Chess; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type antisynchronous_measure_length(void)
{
  int result;
  ply const parent = parent_ply[nbply];
  square const sq_parent_departure = move_effect_journal_get_departure_square(parent);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (sq_parent_departure==initsquare)
    result = true;
  else
  {
    move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
    move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
    square const sq_parent_arrival = move_effect_journal[parent_movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;

    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
    numvec const diff = sq_departure-sq_arrival;
    result = diff==-parent_diff;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
