#include "conditions/transmuting_kings/super.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

#define MAX_OTHER_LEN 1000 /* needs to be at least the max of any value that can be returned in the len functions */

/* the mummer logic is (ab)used to priorise transmuting king moves */
int len_supertransmuting_kings(square sq_departure,
                               square sq_arrival,
                               square sq_capture)
{
  return MAX_OTHER_LEN * (move_generation_stack[current_move[nbply]].current_transmutation!=vide ? 1 : 0);
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type supertransmuting_kings_transmuter_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_generation_stack[coup_id].current_transmutation!=vide)
  {
    square const sq_arrival = move_generation_stack[coup_id].arrival;
    Flags flags = spec[sq_arrival];

    CLRFLAG(flags,Royal);
    move_effect_journal_do_flags_change(move_effect_reason_king_transmutation,
                                        sq_arrival,flags);

    move_effect_journal_do_piece_change(move_effect_reason_king_transmutation,
                                        sq_arrival,
                                        move_generation_stack[coup_id].current_transmutation);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  Cond const cond = slices[si].starter==White ? whsupertrans_king : blsupertrans_king;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (CondFlag[cond])
  {
    slice_index const prototype = alloc_pipe(STSuperTransmutingKingTransmuter);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void stip_insert_supertransmuting_kings(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_impose_starter(si,slices[si].starter);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
