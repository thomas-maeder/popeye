#include "conditions/zeroinchess.h"
#include "position/position.h"
#include "stipulation/move.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "solving/conditional_pipe.h"
#include "solving/temporary_hacks.h"
#include "solving/move_effect_journal.h"
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
void zeroin_remover_solve(slice_index si)
{
  Side const side_zeroing_in = SLICE_STARTER(si);
  Side const side_zeroed_in = advers(side_zeroing_in);
  square const save_king_square = being_solved.king_square[side_zeroed_in];
  Flags const save_king_flags = being_solved.spec[save_king_square];
  square zeroed_in_squares[nr_squares_on_board] = { 0 };
  unsigned int nr_zeroed_in = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_zeroing_in);
  TraceEnumerator(Side,side_zeroed_in);
  TraceSquare(save_king_square);
  TraceEOL();

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (piece_belongs_to_opponent(*bnp)
          && !TSTFLAG(being_solved.spec[*bnp],Royal))
      {
        TraceSquare(*bnp);
        TraceEOL();
        CLRFLAG(being_solved.spec[save_king_square],Royal);
        being_solved.king_square[side_zeroed_in] = *bnp;
        SETFLAG(being_solved.spec[*bnp],Royal);
        if (conditional_pipe_solve_delegate(temporary_hack_mate_tester[side_zeroed_in])
            ==previous_move_has_solved)
          zeroed_in_squares[nr_zeroed_in++] = *bnp;
        TraceEOL();
        CLRFLAG(being_solved.spec[*bnp],Royal);
        being_solved.king_square[side_zeroed_in] = save_king_square;
        being_solved.spec[save_king_square] = save_king_flags;
      }
  }

  {
    unsigned int i;
    for (i = 0; i!=nr_zeroed_in && zeroed_in_squares[i]!=0; ++i)
      move_effect_journal_do_piece_removal(move_effect_reason_zeroed_in,
                                           zeroed_in_squares[i]);
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

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    slice_index const prototype = alloc_pipe(*type);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_zeroin(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    slice_type type = STZeroedInRemover;
    stip_structure_traversal_init(&st,&type);
    stip_structure_traversal_override_single(&st,STGoalCheckReachedTester,&stip_structure_visitor_noop);
    stip_structure_traversal_override_single(&st,STMove,&instrument_move);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
