#include "conditions/disparate.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <stdlib.h>

/* Can a piece on a particular square can move according to Disparate chess?
 * @param sq position of piece
 * @return true iff the piece can move according to Disparate chess
 */
boolean disparate_can_piece_move(square sq)
{
  boolean result = true;
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (nbply>2 && trait[nbply]!=trait[parent])
  {
    move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
    move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
    if (parent_movement>=move_effect_journal_top[parent])
    {
      /* we are solving a threat - no disparate effect there */
    }
    else
    {
      PieNam const pi_parent_moving = move_effect_journal[parent_movement].u.piece_movement.moving;
      if (get_walk_of_piece_on_square(sq)==pi_parent_moving)
        result = false;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean avoid_undisparate_observation(square sq_observer,
                                             square sq_landing,
                                             square sq_observee)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  result = disparate_can_piece_move(sq_observer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  return result;
}

/* Inialise solving in Disparate chess
 */
void disparate_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observation_validator(&avoid_undisparate_observation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
