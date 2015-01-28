#include "conditions/snek.h"
#include "pieces/walks/classification.h"
#include "position/board.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/post_move_iteration.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static square const *current_snekked_pos[maxply+1];

static piece_walk_type const snekked_walk[Bishop+1] =
{
    Empty,
    Empty,
    Empty,
    Empty,
    Rook,
    King,
    Bishop,
    Knight
};

static void find_next_snekked(piece_walk_type walk_captured)
{
  piece_walk_type const to_be_snekked = snekked_walk[walk_captured];

  while (*current_snekked_pos[nbply])
    if (piece_belongs_to_opponent(*current_snekked_pos[nbply])
        && get_walk_of_piece_on_square(*current_snekked_pos[nbply])==to_be_snekked)
      break;
    else
      ++current_snekked_pos[nbply];
}

static boolean find_first_snekked(piece_walk_type walk_captured)
{
  boolean result;

  current_snekked_pos[nbply] = boardnum;
  find_next_snekked(walk_captured);
  result = *current_snekked_pos[nbply];

  return result;
}

static void do_change(slice_index si, piece_walk_type walk_captured)
{
  move_effect_journal_do_walk_change(move_effect_reason_snek,
                                     *current_snekked_pos[nbply],
                                     walk_captured);

  pipe_solve_delegate(si);

  if (!post_move_iteration_locked[nbply])
  {
    ++current_snekked_pos[nbply];
    find_next_snekked(walk_captured);
    if (*current_snekked_pos[nbply])
      lock_post_move_iterations();
  }
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
void snek_substitutor_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  Side const side_captured = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    piece_walk_type const walk_captured = move_effect_journal[capture].u.piece_removal.walk;

    if (walk_captured==Pawn)
    {
      square const sq_royal = being_solved.king_square[side_captured];
      piece_walk_type const walk_royal = get_walk_of_piece_on_square(sq_royal);

      if (!is_king(walk_royal))
        move_effect_journal_do_walk_change(move_effect_reason_snek,
                                           sq_royal,
                                           King);

      pipe_solve_delegate(si);
    }
    else
    {
      if (post_move_iteration_id[nbply]==prev_post_move_iteration_id[nbply])
      {
        assert(*current_snekked_pos[nbply]);
        do_change(si,walk_captured);
      }
      else if (find_first_snekked(walk_captured))
        do_change(si,walk_captured);
      else
        pipe_solve_delegate(si);

      prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
    }
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Snek Chess
 * @param si identifies root slice of stipulation
 */
void solving_insert_snek_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSnekSubstitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
