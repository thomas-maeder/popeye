#include "conditions/haan.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Complete blocking of a square
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 */
static void move_effect_journal_do_square_block(move_effect_reason_type reason,
                                                square square)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_square_block,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(square);
  TraceFunctionParamListEnd();

  entry->u.square_block.square = square;

  block_square(square);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_square_block(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.square_block.square;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  empty_square(on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_square_block(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.square_block.square;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  block_square(on);

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
void haan_chess_hole_inserter_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  boolean piece_visited[MaxPieceId+1] = { false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
    {
      PieceIdType const id = GetPieceId(move_effect_journal[curr].u.piece_movement.movingspec);
      if (!piece_visited[id])
      {
        square const from = move_effect_journal[curr].u.piece_movement.from;
        if (is_square_empty(from))
          move_effect_journal_do_square_block(move_effect_journal[curr].reason,from);
        piece_visited[id] = true;
      }
    }
    else if (move_effect_journal[curr].type==move_effect_piece_removal
             && move_effect_journal[curr].reason==move_effect_reason_ep_capture)
    {
      PieceIdType const id = GetPieceId(move_effect_journal[curr].u.piece_removal.flags);
      if (!piece_visited[id])
      {
        square const on = move_effect_journal[curr].u.piece_removal.on;
        assert(is_square_empty(on));
        move_effect_journal_do_square_block(move_effect_journal[curr].reason,on);
        piece_visited[id] = true;
      }
    }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_haan_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_set_effect_doers(move_effect_square_block,
                                       &undo_square_block,
                                       &redo_square_block);
  stip_instrument_moves(si,STHaanChessHoleInserter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
