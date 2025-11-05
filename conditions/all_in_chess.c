#include "conditions/all_in_chess.h"
#include "position/position.h"
#include "pieces/walks/pawns/en_passant.h"
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
void half_in_chess_move_marker_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,trait[nbply]))
    pipe_solve_delegate(si);
  else
  {
    assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);
    move_effect_journal[movement].reason = move_effect_reason_moving_piece_movement_all_in_chess;
    pipe_solve_delegate(si);
    move_effect_journal[movement].reason = move_effect_reason_moving_piece_movement;
  }

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
  stip_instrument_moves(si,STHalfInChessMoveMarker);

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

static ply find_ply_with_potentially_repeated_position(void)
{
  ply result;
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (parent==ply_retro_move || trait[parent]==trait[nbply])
    result = ply_nil;
  else
    result = parent_ply[parent];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean en_passant_modified_since_last_move_of_same_side(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    ply const ply_position_potentially_repeated = find_ply_with_potentially_repeated_position();

    if (ply_position_potentially_repeated!=ply_nil)
    {
      assert(trait[nbply]==trait[ply_position_potentially_repeated]);

      if (en_passant_was_multistep_played(ply_position_potentially_repeated))
      {
        TraceSquare(en_passant_multistep_over[en_passant_top[ply_position_potentially_repeated]]);TraceEOL();

        if (en_passant_was_multistep_played(nbply))
        {
          /* en passant rights potentially modified */
          TraceSquare(en_passant_multistep_over[en_passant_top[nbply]]);TraceEOL();
          result = en_passant_multistep_over[en_passant_top[ply_position_potentially_repeated]]!=en_passant_multistep_over[en_passant_top[nbply]];
        }
        else
        {
          /* en passant rights lost */
          result = true;
        }
      }
      else
      {
        if (en_passant_was_multistep_played(nbply))
        {
          /* en passant rights added */
          TraceSquare(en_passant_multistep_over[en_passant_top[nbply]]);TraceEOL();
          result = true;
        }
        else
        {
          /* no en passant activity */
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
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
        && move_effect_journal[parent_movement].u.piece_movement.movingspec==move_effect_journal[movement].u.piece_movement.movingspec
        && !en_passant_modified_since_last_move_of_same_side())
    {
      TraceValue("%u",nbply);
      TraceText("AllInChess deems this move illegal because it undoes the preceding move\n");
      solve_result = this_move_is_illegal;
    }
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
  stip_instrument_moves(si,STHalfInChessMoveMarker);

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
void mainly_in_chess_same_mover_avoider_solve(slice_index si)
{
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (parent==ply_retro_move)
    pipe_solve_delegate(si);
  else
  {
    move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
    move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
    PieceIdType parent_id = GetPieceId(move_effect_journal[parent_movement].u.piece_movement.movingspec);

    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    PieceIdType id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);

    if (parent_id==id)
      solve_result = this_move_is_illegal;
    else
      pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with Mainly In Chess
 * @param si identifies entry slice into solving machinery
 */
void solving_instrument_mainly_in_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  all_in_chess_replace_move_generators(si);
  stip_instrument_moves(si,STMainlyInChessSameMoverAvoider);
  stip_instrument_moves(si,STHalfInChessMoveMarker);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
