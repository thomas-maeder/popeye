#include "conditions/alice.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/goals/slice_insertion.h"
#include "solving/pipe.h"
#include "solving/has_solution_type.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/non_king_move_generator.h"
#include "solving/machinery/slack_length.h"
#include "solving/check.h"
#include "position/board.h"
#include "position/effects/flags_change.h"
#include "position/position.h"
#include "pieces/walks/pawns/en_passant.h"

#include "debugging/assert.h"
#include "debugging/trace.h"

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
void alice_change_board_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (curr = base; curr<top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
    {
      square const pos = move_effect_journal[curr].u.piece_movement.to;
      Flags flags = being_solved.spec[pos];
      Flags const flag_new = TSTFLAG(flags,AliceBoardA) ? AliceBoardB : AliceBoardA;

      assert(TSTFLAG(flags,AliceBoardA)|TSTFLAG(flags,AliceBoardB));

      CLRFLAG(flags,AliceBoardA);
      CLRFLAG(flags,AliceBoardB);
      SETFLAG(flags,flag_new);
      move_effect_journal_do_flags_change(move_effect_reason_alice,pos,flags);
    }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square const *advance(Flags board, square const *curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%x",board);
  TraceFunctionParamListEnd();

  while (*curr)
  {
    if (is_square_empty(*curr))
      ++curr;
    else
    {
      Flags const flags = being_solved.spec[*curr];
      if (TSTFLAG(flags,board))
        ++curr;
      else
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
  return curr;
}

static void generate_all_moves_on_board_recursive(Flags boardMoving, square const *curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%x",boardMoving);
  TraceFunctionParamListEnd();

  curr = advance(boardMoving,curr);

  if (*curr)
  {
    Flags const flags = being_solved.spec[*curr];
    piece_walk_type const walk = being_solved.board[*curr];

    TraceSquare(*curr);
    TraceText("temporarily emptying square *curr\n");

    empty_square(*curr);
    generate_all_moves_on_board_recursive(boardMoving,curr+1);
    occupy_square(*curr,walk,flags);

    TraceSquare(*curr);
    TraceText("restored piece on square *curr\n");
  }
  else
    generate_all_moves_for_moving_side();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_all_moves_on_board(Flags boardMoving)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%x",boardMoving);
  TraceFunctionParamListEnd();

  generate_all_moves_on_board_recursive(boardMoving,boardnum);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean not_illegal_for_piece_moving_from_board_B(numecoup n)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceSquare(move_generation_stack[n].departure);
  TraceSquare(move_generation_stack[n].arrival);
  TraceValue("%u",move_generation_stack[n].capture==kingside_castling);
  TraceValue("%u",move_generation_stack[n].capture==queenside_castling);
  TraceEOL();

  if (TSTFLAG(being_solved.spec[move_generation_stack[n].arrival],AliceBoardA))
    result = false;
  else if (move_generation_stack[n].capture==kingside_castling
           || move_generation_stack[n].capture==queenside_castling)
  {
    square const intermediate = (move_generation_stack[n].departure+move_generation_stack[n].arrival)/2;
    result = !TSTFLAG(being_solved.spec[intermediate],AliceBoardA);
  }
  else if (en_passant_is_ep_capture(move_generation_stack[n].capture)
           && (TSTFLAG(being_solved.spec[move_generation_stack[n].arrival],AliceBoardB)
               || !TSTFLAG(being_solved.spec[move_generation_stack[n].capture-offset_en_passant_capture],AliceBoardB)))
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean not_illegal_for_piece_moving_from_board_A(numecoup n)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceSquare(move_generation_stack[n].departure);
  TraceSquare(move_generation_stack[n].arrival);
  TraceValue("%u",move_generation_stack[n].capture==kingside_castling);
  TraceValue("%u",move_generation_stack[n].capture==queenside_castling);
  TraceEOL();

  if (TSTFLAG(being_solved.spec[move_generation_stack[n].arrival],AliceBoardB))
    result = false;
  else if (move_generation_stack[n].capture==kingside_castling
           || move_generation_stack[n].capture==queenside_castling)
  {
    square const intermediate = (move_generation_stack[n].departure+move_generation_stack[n].arrival)/2;
    result = !TSTFLAG(being_solved.spec[intermediate],AliceBoardB);
  }
  else if (en_passant_is_ep_capture(move_generation_stack[n].capture)
           && (TSTFLAG(being_solved.spec[move_generation_stack[n].arrival],AliceBoardA)
               || !TSTFLAG(being_solved.spec[move_generation_stack[n].capture-offset_en_passant_capture],AliceBoardA)))
    result = false;

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
void alice_move_generator_solve(slice_index si)
{
  numecoup base = MOVEBASE_OF_PLY(nbply+1);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));
  generate_all_moves_on_board(AliceBoardA);
  move_generator_filter_moves(base,&not_illegal_for_piece_moving_from_board_A);
  base = MOVEBASE_OF_PLY(nbply+1);
  generate_all_moves_on_board(AliceBoardB);
  move_generator_filter_moves(base,&not_illegal_for_piece_moving_from_board_B);
  pipe_solve_delegate(si);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_substitute(slice_index si,
                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_substitute(si,alloc_pipe(STAliceMoveGenerator));

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
void alice_self_check_guard_solve(slice_index si)
{
  Side const side_in_check = advers(SLICE_STARTER(si));
  square const king_square = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_in_check);
  TraceSquare(king_square);
  TraceEOL();

  if (king_square==initsquare)
    pipe_solve_delegate(si);
  // TODO can we eliminate this check with better instrumentation (no alice_self_check_guard_solve at root level)?
  else if (nbply>ply_retro_move)
  {
    move_effect_journal_index_type const top = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;

    TraceValue("%u",nbply);
    TraceValue("%u",parent_ply[nbply]);
    TraceValue("%u",top);
    TraceValue("%u",movement);
    TraceValue("%x",move_effect_journal[movement].u.piece_movement.movingspec);
    TraceValue("%x",being_solved.spec[king_square]);
    TraceEOL();

    if (TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal))
    {
      boolean self_check_detected;
      Flags const board = TSTFLAG(being_solved.spec[king_square],AliceBoardA) ? AliceBoardA : AliceBoardB;
      Flags const other_board = board==AliceBoardA ? AliceBoardB : AliceBoardA;

      assert(!TSTFLAG(being_solved.spec[king_square],other_board));

      CLRFLAG(being_solved.spec[king_square],board);
      SETFLAG(being_solved.spec[king_square],other_board);

      self_check_detected = is_in_check(side_in_check);

      CLRFLAG(being_solved.spec[king_square],other_board);
      SETFLAG(being_solved.spec[king_square],board);

      if (self_check_detected)
        solve_result = previous_move_is_illegal;
      else
        pipe_solve_delegate(si);
    }
    else
      pipe_solve_delegate(si);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_alice_selfcheckguard(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STAliceSelfCheckGuard);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_alice_selfcheckguard_goal(slice_index si,
                                             stip_structure_traversal *st)
{
  slice_index const tester = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STAliceSelfCheckGuard);
    goal_branch_insert_slices(tester,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean check_by_piece_on_board_recursive(slice_index si,
                                                 Flags board,
                                                 Side side_in_check,
                                                 square const *curr)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%x",board);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  curr = advance(board,curr);

  if (*curr)
  {
    Flags const flags = being_solved.spec[*curr];
    piece_walk_type const walk = being_solved.board[*curr];

    TraceSquare(*curr);
    TraceText("temporarily emptying square *curr\n");

    empty_square(*curr);
    result = check_by_piece_on_board_recursive(si,board,side_in_check,curr+1);
    occupy_square(*curr,walk,flags);
  }
  else
    result = pipe_is_in_check_recursive_delegate(si,side_in_check);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean check_by_piece_on_board(slice_index si,
                                       Flags board,
                                       Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%x",board);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  result = check_by_piece_on_board_recursive(si,board,side_in_check,boardnum);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean alice_check_test_initialiser_is_in_check(slice_index si,
                                                 Side side_in_check)
{
  boolean result;
  square const king_square = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  assert(king_square!=initsquare);

  if (TSTFLAG(being_solved.spec[king_square],AliceBoardA))
    result = check_by_piece_on_board(si,AliceBoardA,side_in_check);
  else
  {
    assert(TSTFLAG(being_solved.spec[king_square],AliceBoardB));
    result = check_by_piece_on_board(si,AliceBoardB,side_in_check);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void solving_insert_alice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAliceBoardChanger);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STMoveGenerator,&do_substitute);
    stip_structure_traversal_override_single(&st,STSelfCheckGuard,&insert_alice_selfcheckguard);
    stip_structure_traversal_override_single(&st,STGoalReachedTester,&insert_alice_selfcheckguard_goal);
    stip_traverse_structure(si,&st);
  }

  solving_instrument_check_testing(si,STAliceCheckTestInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
