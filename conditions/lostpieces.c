#include "conditions/lostpieces.h"
#include "position/position.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/mate/reached_tester.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/proxy.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "solving/conditional_pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/temporary_hacks.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void lostpieces_remover_solve(slice_index si)
{
  Side const side_removing = SLICE_STARTER(si);
  Side const side_losing = advers(side_removing);
  square const save_king_square_removing = being_solved.king_square[side_removing];
  square const save_king_square_losing = being_solved.king_square[side_losing];
  Flags const save_king_flags_removing = being_solved.spec[save_king_square_removing];
  Flags const save_king_flags_losing = being_solved.spec[save_king_square_losing];
  square squares_with_lost[nr_squares_on_board] = { 0 };
  unsigned int nr_lost = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  CLRFLAG(being_solved.spec[save_king_square_removing],Royal);
  being_solved.king_square[side_removing] = nullsquare;

  CLRFLAG(being_solved.spec[save_king_square_losing],Royal);

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (piece_belongs_to_opponent(*bnp) && *bnp!=save_king_square_losing)
      {
        TraceSquare(*bnp);TraceEOL();
        being_solved.king_square[side_losing] = *bnp;
        SETFLAG(being_solved.spec[*bnp],Royal);
        if (conditional_pipe_solve_delegate(temporary_hack_lost_piece_tester[side_losing])
            ==previous_move_has_solved)
          squares_with_lost[nr_lost++] = *bnp;
        CLRFLAG(being_solved.spec[*bnp],Royal);
      }
  }

  being_solved.spec[save_king_square_losing] = save_king_flags_losing;
  being_solved.king_square[side_losing] = save_king_square_losing;

  being_solved.spec[save_king_square_removing] = save_king_flags_removing;
  being_solved.king_square[side_removing] = save_king_square_removing;

  {
    unsigned int i;
    for (i = 0; i!=nr_lost; ++i)
      move_effect_journal_do_piece_removal(move_effect_reason_zeroed_in,
                                           squares_with_lost[i]);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STLostPiecesRemover);
    move_insert_slices(si,st->context,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_lostpieces(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STGoalCheckReachedTester,&stip_traverse_structure_children_pipe);
    stip_structure_traversal_override_single(&st,STLostPiecesTester,&stip_traverse_structure_children_pipe);
    stip_structure_traversal_override_single(&st,STMove,&instrument_move);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
