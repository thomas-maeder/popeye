#include "retro/retro.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"

#include "debugging/assert.h"

void retro_play_null_move(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_do_null_move();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void retro_start_retraction_ply(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_retro_move_takeback);

  pipe_solve_delegate(si);

  undo_move_effects();

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void retro_start_retro_move_ply(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* Make sure that trait is the opposite of the first move (or there
   * will be no e.p. capture). */
  assert(slices[si].starter!=no_side);
  nextply(advers(slices[si].starter));

  assert(nbply==ply_retro_move);

  {
    /* TODO let Mars Circe or SingleBox Type 3 add slices here that do this */
    unsigned int i;
    for (i = 0; i!=move_effect_journal_index_offset_capture; ++i)
      move_effect_journal_do_null_effect();
  }

  pipe_solve_delegate(si);

  undo_move_effects();

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
