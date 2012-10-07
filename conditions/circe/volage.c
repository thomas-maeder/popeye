#include "conditions/circe/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_volage_recolorer_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  square const sq_rebirth = current_circe_rebirth_square[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();


  if (TSTFLAG(spec[sq_rebirth],Volage)
      && (SquareCol(sq_rebirth)
          !=SquareCol(move_generation_stack[current_move[nbply]].capture)))
  {
    move_effect_journal_do_side_change(move_effect_reason_volage_side_change,
                                       sq_rebirth,
                                       e[sq_rebirth]<vide ? White : Black);
    if (!CondFlag[hypervolage])
    {
      Flags changed = spec[sq_rebirth];
      CLRFLAG(changed,Volage);
      move_effect_journal_do_flags_change(move_effect_reason_volage_side_change,
                                          sq_rebirth,changed);
    }
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STCirceVolageRecolorer);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_volage_recolorers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCirceRebirthHandler,
                                           &instrument_rebirth);
  stip_structure_traversal_override_single(&st,
                                           STSuperCirceRebirthHandler,
                                           &instrument_rebirth);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
