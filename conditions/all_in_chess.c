#include "conditions/all_in_chess.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "stipulation/fork.h"
#include "solving/temporary_hacks.h"
#include "solving/check.h"
#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "solving/conditional_pipe.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "solving/machinery/slack_length.h"
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
void half_in_chess_king_move_generator_solve(slice_index si)
{
  Side const side_at_move = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(side_at_move);

  trait[nbply] = advers(trait[nbply]);

  if (TSTFLAG(being_solved.spec[being_solved.king_square[trait[nbply]]],Royal))
    generate_moves_for_piece(being_solved.king_square[trait[nbply]]);
  else
  {
    /* - there is no being_solved.king_square, or
     * - being_solved.king_square is a royal square */
  }

  trait[nbply] = advers(trait[nbply]);

  pipe_solve_delegate(si);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean half_in_chess_advance_departure_square(square const **next_square_to_try)
{
  trait[nbply] = advers(trait[nbply]);

  while (true)
  {
    square const sq_departure = **next_square_to_try;
    if (sq_departure==0)
      break;
    else
    {
      ++*next_square_to_try;

      if (TSTFLAG(being_solved.spec[sq_departure],trait[nbply])
        /* don't use king_square[side] - it may be a royal square occupied
         * by a non-royal piece! */
          && !TSTFLAG(being_solved.spec[sq_departure],Royal))
      {
        generate_moves_for_piece(sq_departure);
        trait[nbply] = advers(trait[nbply]);
        return true;
      }
    }
  }

  trait[nbply] = advers(trait[nbply]);
  return false;
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
void half_in_chess_non_king_move_generator_solve(slice_index si)
{
  square const *next_square_to_try = boardnum;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_result = immobility_on_next_move;

  nextply(SLICE_STARTER(si));

  while (solve_result<slack_length
         && half_in_chess_advance_departure_square(&next_square_to_try))
    pipe_solve_delegate(si);

  finply();

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
void half_in_chess_move_generator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));
  trait[nbply] = advers(trait[nbply]);
  generate_all_moves_for_moving_side();
  trait[nbply] = advers(trait[nbply]);
  pipe_solve_delegate(si);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void half_in_chess_do_substitute(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_substitute(si,alloc_pipe(STHalfInChessMoveGenerator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void half_in_chess_do_substitute_king(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_substitute(si,alloc_pipe(STHalfInChessKingMoveGenerator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void half_in_chess_do_substitute_non_king(slice_index si,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_substitute(si,alloc_pipe(STHalfInChessNonKingMoveGenerator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void half_in_chess_replace_move_generators(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMoveGenerator,&half_in_chess_do_substitute);
  stip_structure_traversal_override_single(&st,STKingMoveGenerator,&half_in_chess_do_substitute_king);
  stip_structure_traversal_override_single(&st,STNonKingMoveGenerator,&half_in_chess_do_substitute_non_king);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Half In Chess
 * @param si identifies entry slice into solving machinery
 */
void solving_instrument_half_in_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  half_in_chess_replace_move_generators(si);

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
void all_in_chess_king_move_generator_solve(slice_index si)
{
  Side const side_at_move = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(side_at_move);

  trait[nbply] = advers(trait[nbply]);

  if (TSTFLAG(being_solved.spec[being_solved.king_square[trait[nbply]]],Royal))
    generate_moves_for_piece(being_solved.king_square[trait[nbply]]);
  else
  {
    /* - there is no being_solved.king_square, or
     * - being_solved.king_square is a royal square */
  }

  trait[nbply] = advers(trait[nbply]);

  if (TSTFLAG(being_solved.spec[being_solved.king_square[trait[nbply]]],Royal))
    generate_moves_for_piece(being_solved.king_square[trait[nbply]]);
  else
  {
    /* - there is no being_solved.king_square, or
     * - being_solved.king_square is a royal square */
  }

  pipe_solve_delegate(si);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean all_in_chess_advance_departure_square(square const **next_square_to_try)
{
  while (true)
  {
    square const sq_departure = **next_square_to_try;
    if (sq_departure==0)
      break;
    else
    {
      ++*next_square_to_try;

      if (TSTFLAG(being_solved.spec[sq_departure],trait[nbply])
        /* don't use king_square[side] - it may be a royal square occupied
         * by a non-royal piece! */
          && !TSTFLAG(being_solved.spec[sq_departure],Royal))
      {
        generate_moves_for_piece(sq_departure);
        return true;
      }
    }
  }

  return false;
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
void all_in_chess_non_king_move_generator_solve(slice_index si)
{
  square const *next_square_to_try = boardnum;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solve_result = immobility_on_next_move;

  nextply(SLICE_STARTER(si));

  while (solve_result<slack_length
         && all_in_chess_advance_departure_square(&next_square_to_try))
    pipe_solve_delegate(si);

  next_square_to_try = boardnum;

  while (solve_result<slack_length
         && half_in_chess_advance_departure_square(&next_square_to_try))
    pipe_solve_delegate(si);

  finply();

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
void all_in_chess_move_generator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));
  trait[nbply] = advers(trait[nbply]);
  generate_all_moves_for_moving_side();
  trait[nbply] = advers(trait[nbply]);
  generate_all_moves_for_moving_side();
  pipe_solve_delegate(si);
  finply();

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
void all_in_chess_undo_move_avoider_solve(slice_index si)
{
  move_effect_journal_index_type const parent_base = move_effect_journal_base[parent_ply[nbply]];
  move_effect_journal_index_type const parent_capture = parent_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[parent_capture].type==move_effect_piece_removal)
    pipe_solve_delegate(si);
  else if (move_effect_journal[capture].type==move_effect_piece_removal)
    pipe_solve_delegate(si);
  else
  {
    move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

    assert(move_effect_journal[parent_capture].type==move_effect_no_piece_removal);
    assert(move_effect_journal[capture].type==move_effect_no_piece_removal);

    if (move_effect_journal[parent_movement].u.piece_movement.from==move_effect_journal[movement].u.piece_movement.to
        && move_effect_journal[parent_movement].u.piece_movement.to==move_effect_journal[movement].u.piece_movement.from
        && move_effect_journal[parent_movement].u.piece_movement.moving==move_effect_journal[movement].u.piece_movement.moving
        && move_effect_journal[parent_movement].u.piece_movement.movingspec==move_effect_journal[movement].u.piece_movement.movingspec)
      solve_result = this_move_is_illegal;
    else
      pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void all_in_chess_do_substitute(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_substitute(si,alloc_pipe(STAllInChessMoveGenerator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void all_in_chess_do_substitute_king(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_substitute(si,alloc_pipe(STAllInChessKingMoveGenerator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void all_in_chess_do_substitute_non_king(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_substitute(si,alloc_pipe(STAllInChessNonKingMoveGenerator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void all_in_chess_replace_move_generators(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMoveGenerator,&all_in_chess_do_substitute);
  stip_structure_traversal_override_single(&st,STKingMoveGenerator,&all_in_chess_do_substitute_king);
  stip_structure_traversal_override_single(&st,STNonKingMoveGenerator,&all_in_chess_do_substitute_non_king);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with All In Chess
 * @param si identifies entry slice into solving machinery
 */
void solving_instrument_all_in_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  all_in_chess_replace_move_generators(si);
  stip_instrument_moves(si,STAllInChessUndoMoveAvoider);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
