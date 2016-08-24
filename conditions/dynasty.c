#include "conditions/dynasty.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static void update_king_square(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  TraceValue("%u",being_solved.number_of_pieces[side][King]);
  TraceEOL();
  if (being_solved.number_of_pieces[side][King]==1)
  {
    if (being_solved.king_square[side]==initsquare)
    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
        if (TSTFLAG(being_solved.spec[*bnp],side) && get_walk_of_piece_on_square(*bnp)==King)
        {
          Flags flags = being_solved.spec[*bnp];
          SETFLAG(flags,Royal);
          move_effect_journal_do_flags_change(move_effect_reason_royal_dynasty,
                                              *bnp,flags);
          enable_castling_rights(move_effect_reason_royal_dynasty,*bnp);
          break;
        }
    }
  }
  else
  {
    if (being_solved.king_square[side]!=initsquare)
    {
      Flags flags = being_solved.spec[being_solved.king_square[side]];

      /* before move_effect_journal_do_flags_change() or being_solved.king_square may have
       * been modified
       */
      disable_castling_rights(move_effect_reason_royal_dynasty,being_solved.king_square[side]);

      CLRFLAG(flags,Royal);
      move_effect_journal_do_flags_change(move_effect_reason_royal_dynasty,
                                          being_solved.king_square[side],flags);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
void dynasty_king_square_updater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_king_square(White);
  update_king_square(Black);
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the solving machinery with Royal Dynasty
 * @param si identifies root slice of stipulation
 */
void dynasty_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDynastyKingSquareUpdater);
  check_no_king_is_possible();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
