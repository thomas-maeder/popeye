#include "conditions/haan.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type haan_chess_hole_inserter_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;


  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_base[parent_ply[nbply]+1]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
    {
      PieNam const from = move_effect_journal[curr].u.piece_movement.from;
      if (is_square_empty(from))
        move_effect_journal_do_square_block(move_effect_journal[curr].reason,from);
    }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_haan_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STHaanChessHoleInserter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
