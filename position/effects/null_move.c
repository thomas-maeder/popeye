#include "position/effects/null_move.h"
#include "position/effects/piece_removal.h"

/* Add a null effect to the current move of the current ply
 * @param reason for null effect
 */
void move_effect_journal_do_null_effect(move_effect_reason_type reason)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  move_effect_journal_allocate_entry(move_effect_none,reason);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the effects of a null move to the current move of the current ply
 * @param reason for null move
 */
void move_effect_journal_do_null_move(move_effect_reason_type reason)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  move_effect_journal_do_no_piece_removal();
  move_effect_journal_do_null_effect(reason); /* and no piece movement */

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
