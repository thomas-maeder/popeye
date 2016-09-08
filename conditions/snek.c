#include "conditions/snek.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/walks.h"
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
  piece_walk_type const to_be_snekked = standard_walks[snekked_walk[orthodoxise_walk(walk_captured)]];

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

  if (!post_move_iteration_is_locked(&prev_post_move_iteration_id[nbply]))
  {
    ++current_snekked_pos[nbply];
    find_next_snekked(walk_captured);
    if (*current_snekked_pos[nbply])
      post_move_iteration_lock(&prev_post_move_iteration_id[nbply]);
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
  Side const side_captured = advers(SLICE_STARTER(si));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    piece_walk_type const walk_captured = move_effect_journal[capture].u.piece_removal.walk;

    if (walk_captured==standard_walks[Pawn])
    {
      square const sq_royal = being_solved.king_square[side_captured];
      piece_walk_type const walk_royal = get_walk_of_piece_on_square(sq_royal);

      if (walk_royal!=standard_walks[King])
        move_effect_journal_do_walk_change(move_effect_reason_snek,
                                           sq_royal,
                                           standard_walks[King]);

      pipe_solve_delegate(si);
    }
    else if (!post_move_am_i_iterating(prev_post_move_iteration_id[nbply]))
    {
      if (find_first_snekked(walk_captured))
        do_change(si,walk_captured);
      else
        pipe_solve_delegate(si);
    }
    else
    {
      assert(*current_snekked_pos[nbply]);
      do_change(si,walk_captured);
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

static void find_next_snekked_circle(piece_walk_type walk_snekked)
{
  while (*current_snekked_pos[nbply])
    if (piece_belongs_to_opponent(*current_snekked_pos[nbply])
        && get_walk_of_piece_on_square(*current_snekked_pos[nbply])==walk_snekked)
      break;
    else
      ++current_snekked_pos[nbply];
}

static boolean find_first_snekked_circle(piece_walk_type walk_snekked)
{
  boolean result;

  current_snekked_pos[nbply] = boardnum;
  find_next_snekked_circle(walk_snekked);
  result = *current_snekked_pos[nbply];

  return result;
}

static void do_change_circle(slice_index si,
                             piece_walk_type walk_captured,
                             piece_walk_type walk_snekked)
{
  move_effect_journal_do_walk_change(move_effect_reason_snek,
                                     *current_snekked_pos[nbply],
                                     walk_captured);

  pipe_solve_delegate(si);

  if (!post_move_iteration_is_locked(&prev_post_move_iteration_id[nbply]))
  {
    ++current_snekked_pos[nbply];
    find_next_snekked_circle(walk_snekked);
    if (*current_snekked_pos[nbply])
      post_move_iteration_lock(&prev_post_move_iteration_id[nbply]);
  }
}

static piece_walk_type get_walk_snekked(piece_walk_type walk_captured)
{
  piece_walk_type result;

  switch (walk_captured)
  {
    case Queen:
      result = Rook;
      break;
    case Rook:
      result = Bishop;
      break;
    case Bishop:
      result = Knight;
      break;
    case Knight:
      result = Queen;
      break;

    default:
      result = Invalid;
      break;
  }

  return result;
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
void snek_circle_substitutor_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    piece_walk_type const walk_captured = move_effect_journal[capture].u.piece_removal.walk;
    piece_walk_type const walk_snekked = get_walk_snekked(walk_captured);

    if (walk_snekked==Invalid)
      pipe_solve_delegate(si);
    else if (!post_move_am_i_iterating(prev_post_move_iteration_id[nbply]))
    {
      if (find_first_snekked_circle(walk_snekked))
        do_change_circle(si,walk_captured,walk_snekked);
      else
        pipe_solve_delegate(si);
    }
    else
    {
      assert(*current_snekked_pos[nbply]);
      do_change_circle(si,walk_captured,walk_snekked);
    }
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Snek Circle Chess
 * @param si identifies root slice of stipulation
 */
void solving_insert_snekcircle_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSnekCircleSubstitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
