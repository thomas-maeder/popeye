#include "position/effects/null_move.h"
#include "position/effects/piece_removal.h"

/* Add a null effect to the current move of the current ply
 */
void move_effect_journal_do_null_effect(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_allocate_entry(move_effect_none,move_effect_no_reason);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the effects of a null move to the current move of the current ply
 */
void move_effect_journal_do_null_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_do_no_piece_removal();
  move_effect_journal_do_null_effect(); /* and no piece movement */

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
