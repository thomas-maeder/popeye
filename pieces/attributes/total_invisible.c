#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "position/effects/piece_readdition.h"
#include "position/effects/piece_movement.h"
#include "position/effects/null_move.h"
#include "position/effects/walk_change.h"
#include "position/effects/flags_change.h"
#include "position/effects/king_square.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/slice_insertion.h"
#include "solving/check.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/castling.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "options/movenumbers.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible/intercept_illegal_checks.h"
#include "pieces/attributes/total_invisible/attack_mating_piece.h"
#include "pieces/attributes/total_invisible/replay_fleshed_out.h"
#include "pieces/attributes/total_invisible/king_placement.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

unsigned int total_invisible_number;

ply top_ply_of_regular_play;
slice_index tester_slice;

play_phase_type play_phase = play_regular;

void report_deadend(char const *s, unsigned int lineno)
{
  printf("%s;%u;%u\n",s,lineno,(unsigned int)play_phase);
}

void write_history_recursive(ply ply)
{
  if (parent_ply[ply]>ply_retro_move)
    write_history_recursive(parent_ply[ply]);

  {
    move_effect_journal_index_type const base = move_effect_journal_base[ply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

    printf(" %u:",ply);
    WriteWalk(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.from);
    putchar('-');
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.to);
  }
}

void total_invisible_write_flesh_out_history(void)
{
  if (total_invisible_number>0 && nbply>ply_retro_move)
  {
    fputs(" -", stdout);
    write_history_recursive(top_ply_of_regular_play);
  }
}

void restart_from_scratch(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(check_by_uninterceptable_delivered_from);
  TraceValue("%u",check_by_uninterceptable_delivered_in_ply);
  TraceEOL();

  if (check_by_uninterceptable_delivered_in_ply>=nbply)
  {
    record_decision_outcome("%s","uninterceptable piece delivering check can't be captured by random move");
    REPORT_DEADEND;
    backtrack_from_failure_to_capture_uninterceptable_checker(advers(trait[check_by_uninterceptable_delivered_in_ply]),1);
  }
  else if (nbply==ply_retro_move+1)
  {
    TraceValue("%u",nbply);TraceEOL();
    adapt_pre_capture_effect();
  }
  else
  {
    --nbply;
    undo_revelation_effects(move_effect_journal_base[nbply+1]);
    ++nbply;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void recurse_into_child_ply(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  move_effect_journal_index_type const save_top = move_effect_journal_base[nbply+1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%x",move_effect_journal[movement].u.piece_movement.movingspec);
  TraceValue("%u",GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec));
  TraceEOL();

  assert(sq_departure==move_by_invisible
         || GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec)!=NullPieceId);

  move_effect_journal_base[nbply+1] = top_before_revelations[nbply];
  assert(top_before_revelations[nbply]>move_effect_journal_base[nbply]);
  redo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

  ++nbply;
  TraceValue("%u",nbply);TraceEOL();

  test_and_execute_revelations(top_before_revelations[nbply-1]);

  --nbply;

  move_effect_journal_base[nbply+1] = top_before_revelations[nbply];
  assert(top_before_revelations[nbply]>move_effect_journal_base[nbply]);
  undo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void protect_castling_king_on_intermediate_square(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  Side const side_in_check = trait[nbply];
  square const intermediate_square = (move_effect_journal[movement].u.piece_movement.to+move_effect_journal[movement].u.piece_movement.from)/2;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].reason==move_effect_reason_castling_king_movement);
  deal_with_illegal_checks(side_in_check,intermediate_square,&recurse_into_child_ply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void protect_castling_king_at_home(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  Side const side_in_check = trait[nbply];
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[movement].reason==move_effect_reason_castling_king_movement)
    deal_with_illegal_checks(side_in_check,king_pos,&protect_castling_king_on_intermediate_square);
  else
    recurse_into_child_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void adapt_capture_effect(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(to);
  TraceWalk(get_walk_of_piece_on_square(to));
  TraceValue("%x",being_solved.spec[to]);
  TraceEOL();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    if (is_square_empty(to))
    {
      TraceText("no capture planned and destination square empty - no need for adaptation\n");
      protect_castling_king_at_home();
    }
    else if (TSTFLAG(being_solved.spec[to],Royal))
    {
      TraceText("we don't capture the king\n");
    }
    else
    {
      PieceIdType const id_captured = GetPieceId(being_solved.spec[to]);
      purpose_type const save_purpose = motivation[id_captured].last.purpose;

      TraceText("capture of a total invisible that happened to land on the arrival square\n");

      assert(TSTFLAG(being_solved.spec[to],advers(trait[nbply])));
      assert(move_effect_journal[movement].u.piece_movement.moving!=Pawn);

      move_effect_journal[capture].type = move_effect_piece_removal;
      move_effect_journal[capture].reason = move_effect_reason_regular_capture;
      move_effect_journal[capture].u.piece_removal.on = to;
      move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
      move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];

      protect_castling_king_at_home();

      move_effect_journal[capture].type = move_effect_no_piece_removal;

      motivation[id_captured].last.purpose = save_purpose;
    }
  }
  else if (move_effect_journal[base].type==move_effect_piece_readdition)
  {
    assert(move_effect_journal[base].u.piece_addition.added.on==to);

    TraceText("capture of invisible victim added for the purpose\n");

    if (is_square_empty(to))
      protect_castling_king_at_home();
    else
    {
      assert(move_effect_journal[movement].u.piece_movement.moving==Pawn);
      TraceText("another total invisible has appeared on the arrival square\n");

      if (TSTFLAG(being_solved.spec[to],advers(trait[nbply])))
      {
        piece_walk_type const walk_victim_orig = move_effect_journal[capture].u.piece_removal.walk;
        /* if the piece to be captured is royal, then our tests for self check have failed */
        assert(!TSTFLAG(being_solved.spec[to],Royal));
        move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
        protect_castling_king_at_home();
        move_effect_journal[capture].u.piece_removal.walk = walk_victim_orig;
      }
      else
      {
        TraceText("move is now blocked\n");
        record_decision_outcome("%s","move is now blocked");
        REPORT_DEADEND;
      }
    }
  }
  else if (is_square_empty(to))
  {
    TraceText("original capture victim was captured by a TI that has since left\n");
    if (is_pawn(move_effect_journal[movement].u.piece_movement.moving))
    {
      TraceText("bad idea if the capturer is a pawn!\n");
      record_decision_outcome("%s","bad idea if the capturer is a pawn!");
      REPORT_DEADEND;
    }
    else
    {
      move_effect_journal[capture].type = move_effect_no_piece_removal;
      protect_castling_king_at_home();
      move_effect_journal[capture].type = move_effect_piece_removal;
    }
  }
  else
  {
    piece_walk_type const orig_walk_removed = move_effect_journal[capture].u.piece_removal.walk;
    Flags const orig_flags_removed = move_effect_journal[capture].u.piece_removal.flags;

    TraceText("adjusting removal to actual victim, which may be different from planned victim\n");

    move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
    move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];

    if (TSTFLAG(orig_flags_removed,Chameleon))
    {
      PieceIdType const id_removed = GetPieceId(orig_flags_removed);
      purpose_type const orig_purpose_removed = motivation[id_removed].last.purpose;

      TraceValue("%x",orig_flags_removed);
      TraceValue("%u",id_removed);
      TraceEOL();

      motivation[id_removed].last.purpose = purpose_none;
      protect_castling_king_at_home();
      motivation[id_removed].last.purpose = orig_purpose_removed;
    }
    else
      protect_castling_king_at_home();

    move_effect_journal[capture].u.piece_removal.walk = orig_walk_removed;
    move_effect_journal[capture].u.piece_removal.flags = orig_flags_removed;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_invisible_capturer(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const pre_capture = effects_base;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (move_effect_journal[pre_capture].type)
  {
    case move_effect_piece_readdition:
    {
      square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
      piece_walk_type const walk_added = move_effect_journal[pre_capture].u.piece_addition.added.walk;
      Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;
      Side const side_added = TSTFLAG(flags_added,White) ? White : Black;

      PieceIdType const id = GetPieceId(flags_added);

      move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
      square const to = move_effect_journal[movement].u.piece_movement.to;

      assert(sq_addition!=to);
      assert(is_square_empty(sq_addition));
      assert(!was_taboo(sq_addition,side_added));

      move_effect_journal[pre_capture].type = move_effect_none;
      record_decision_for_inserted_invisible(id);
      ++being_solved.number_of_pieces[side_added][walk_added];
      occupy_square(sq_addition,walk_added,flags_added);
      restart_from_scratch();
      empty_square(sq_addition);
      --being_solved.number_of_pieces[side_added][walk_added];
      move_effect_journal[pre_capture].type = move_effect_piece_readdition;

      break;
    }

    case move_effect_none:
    {
      ply const ply_capture_by_pawn = nbply+1;

      TraceText("no capturer to be inserted\n");
      if (need_existing_invisible_as_victim_for_capture_by_pawn(ply_capture_by_pawn)==initsquare)
        adapt_capture_effect();
      else
      {
        record_decision_outcome("capture in ply %u will not be possible",ply_capture_by_pawn);
        REPORT_DEADEND;
        backtrack_from_failed_capture_of_invisible_by_pawn(trait[ply_capture_by_pawn]);
      }
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_intercepting_illegal_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply<=top_ply_of_regular_play)
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

    TraceValue("%u",nbply);
    TraceValue("%u",top_ply_of_regular_play);
    TraceSquare(sq_departure);
    TraceValue("%u",sq_departure);
    TraceValue("%u",capture_by_invisible);
    TraceSquare(sq_arrival);
    TraceValue("%u",sq_arrival);
    TraceValue("%u",move_by_invisible);
    TraceEOL();

    {
      PieceIdType id;
      for (id = get_top_visible_piece_id()+1; id<=get_top_invisible_piece_id(); ++id)
      {
        TraceValue("%u",id);TraceEOL();
        TraceAction(&motivation[id].first);TraceEOL();
        TraceAction(&motivation[id].last);TraceEOL();
        TraceWalk(get_walk_of_piece_on_square(motivation[id].last.on));
        TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
        TraceEOL();
      }
    }

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
      flesh_out_random_move_by_invisible();
    else if (sq_departure==capture_by_invisible
             && is_on_board(sq_arrival))
      flesh_out_capture_by_invisible();
    else
    {
      square const first_taboo_violation = find_taboo_violation();
      if (first_taboo_violation==nullsquare)
        insert_invisible_capturer();
      else
      {
        // TODO review
//        assert(is_taboo_violation_acceptable(first_taboo_violation));
      }
    }
  }
  else
    validate_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void adapt_pre_capture_effect(void)
{
  Side const side_just_moved = trait[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply>top_ply_of_regular_play)
  {
    square const king_pos = being_solved.king_square[side_just_moved];
    TraceText("there are no post-play pre-capture effects\n");
    deal_with_illegal_checks(side_just_moved,king_pos,&done_intercepting_illegal_checks);
  }
  else
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const pre_capture = effects_base;

    TraceValue("%u",nbply);
    TraceValue("%u",move_effect_journal[pre_capture].type);
    TraceEOL();

    if (move_effect_journal[pre_capture].type==move_effect_piece_readdition)
    {
      move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
      square const to = move_effect_journal[movement].u.piece_movement.to;

      assert(is_on_board(to));

      if (move_effect_journal[pre_capture].u.piece_addition.added.on==to)
      {
        Side const side_capturing_pawn = trait[nbply];
        Side const side_pawn_victim = advers(side_capturing_pawn);

        if (is_square_empty(to))
        {
          if (!is_taboo(to,side_pawn_victim)
              && !will_be_taboo(to,side_pawn_victim)
              && !was_taboo(to,side_pawn_victim))
          {
            dynamic_consumption_type const save_consumption = current_consumption;

            if (allocate_placed(side_pawn_victim))
            {
              square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
              piece_walk_type const walk_added = move_effect_journal[pre_capture].u.piece_addition.added.walk;
              Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;

              record_decision_outcome("%s","adding victim of capture by pawn");
              assert(move_effect_journal[pre_capture].type==move_effect_piece_readdition);
              move_effect_journal[pre_capture].type = move_effect_none;
              occupy_square(sq_addition,walk_added,flags_added);
              restart_from_scratch();
              empty_square(sq_addition);
              move_effect_journal[pre_capture].type = move_effect_piece_readdition;
              current_consumption = save_consumption;
              TraceConsumption();TraceEOL();
            }
            else
            {
              TraceText("no invisible left for placing it as victim of capture by pawn\n");
              record_decision_outcome("%s","no invisible left for placing it as victim");
              REPORT_DEADEND;
              current_consumption = save_consumption;
              TraceConsumption();TraceEOL();
              /* if we correctly flesh out random moves, we don't arrive here any more */
//              assert(0);
            }
          }
          else
          {
            record_decision_outcome("%s","can't add victim of capture by pawn because of taboos");
            REPORT_DEADEND;
          }
        }
        else if (TSTFLAG(being_solved.spec[to],side_pawn_victim))
        {
          PieceIdType const id = GetPieceId(being_solved.spec[to]);
          purpose_type const save_purpose = motivation[id].last.purpose;
          square const king_pos = being_solved.king_square[side_just_moved];

          record_decision_outcome("%s","no need to add victim of capture by pawn any more");
          move_effect_journal[pre_capture].type = move_effect_none;
          motivation[id].last.purpose = purpose_none;
          deal_with_illegal_checks(side_just_moved,king_pos,&done_intercepting_illegal_checks);
          motivation[id].last.purpose = save_purpose;
          move_effect_journal[pre_capture].type = move_effect_piece_readdition;
        }
        else
        {
          record_decision_outcome("%s","arrival square occupied by piece of the wrong side");
          REPORT_DEADEND;
        }
      }
      else
      {
        square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
        Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;
        Side const side_added = TSTFLAG(flags_added,White) ? White : Black;

        TraceSquare(sq_addition);
        TraceValue("%u",is_square_empty(sq_addition));
        TraceValue("%u",move_effect_journal[pre_capture].reason);
        TraceEOL();

        if (was_taboo_forever(sq_addition,side_added))
        {
          TraceText("Hmm - some invisible piece must have traveled through the castling partner's square\n");
          TraceText("This should have been prevented by the taboo machinery\n");
          assert(0);
        }
        else
        {
          square const king_pos = being_solved.king_square[side_just_moved];
          TraceText("possible addition of an invisible capturer - details to be clarified later\n");
          deal_with_illegal_checks(side_just_moved,king_pos,&done_intercepting_illegal_checks);
        }
      }
    }
    else
    {
      square const king_pos = being_solved.king_square[side_just_moved];
      TraceText("no piece addition to be adapted\n");
      deal_with_illegal_checks(side_just_moved,king_pos,&done_intercepting_illegal_checks);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_mate(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[top_ply_of_regular_play];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",top_ply_of_regular_play);
  TraceSquare(sq_departure);
  TraceValue("%u",sq_departure);
  TraceSquare(sq_arrival);
  TraceValue("%u",sq_arrival);
  TraceValue("%u",move_by_invisible);
  TraceEOL();

  if (sq_departure==move_by_invisible
      && sq_arrival==move_by_invisible)
    combined_validation_result = mate_defendable_by_interceptors;
  else
  {
    combined_validation_result = mate_unvalidated;
    initialise_decision_context();
    adapt_pre_capture_effect();
    record_decision_outcome("validate_mate(): combined_validation_result:%u",
                            combined_validation_result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_mate(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (combined_validation_result)
  {
    case mate_unvalidated:
      assert(get_decision_result()==previous_move_is_illegal);
      break;

    case no_mate:
      assert(get_decision_result()==previous_move_has_not_solved);
      break;

    case mate_attackable:
    case mate_defendable_by_interceptors:
      initialise_decision_context();
      adapt_pre_capture_effect();
      record_decision_outcome("test_mate(): get_decision_result():%u",
                              get_decision_result());
      break;

    case mate_with_2_uninterceptable_doublechecks:
      /* we only replay moves for TI revelation */
      initialise_decision_context();
      adapt_pre_capture_effect();
      record_decision_outcome("test_mate(): get_decision_result():%u",
                              get_decision_result());
      assert(get_decision_result()==previous_move_has_solved);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void rewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (nbply!=ply_retro_move)
  {
    TraceConsumption();
    assert(nr_total_invisbles_consumed()<=total_invisible_number);
    undo_move_effects();
    --nbply;
  }

  ++nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void unrewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --nbply;

  while (nbply!=top_ply_of_regular_play)
  {
    ++nbply;
    redo_move_effects();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_and_test(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  play_phase = play_validating_mate;
  validate_mate();
  play_phase = play_testing_mate;
  test_mate();

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
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply-ply_retro_move);TraceEOL();

  remember_taboos_for_current_move();

  /* necessary for detecting checks by pawns and leapers */
  if (being_solved.king_square[trait[nbply]]!=initsquare
      && is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else
  {
    /* make sure that our length corresponds to the length of the tested move sequence
     * (which may vary if STFindShortest is used)
     */
    assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
    slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

    TraceValue("%u",SLICE_NEXT2(si));
    TraceValue("%u",slices[SLICE_NEXT2(si)].u.branch.length);
    TraceEOL();

    top_ply_of_regular_play = nbply;
    tester_slice = si;

    top_before_revelations[nbply] = move_effect_journal_base[nbply+1];

    play_phase = play_rewinding;
    rewind_effects();

    static_consumption.king[White] = being_solved.king_square[White]==initsquare;
    static_consumption.king[Black] = being_solved.king_square[Black]==initsquare;

    validate_and_test();

    static_consumption.king[White] = false;
    static_consumption.king[Black] = false;

    play_phase = play_unwinding;
    unrewind_effects();
    play_phase = play_regular;

    solve_result = get_decision_result();
  }

  forget_taboos_for_current_move();

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
void total_invisible_reserve_king_movement(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",move_effect_journal[capture].type);
  TraceEOL();
  if (move_effect_journal[capture].type==move_effect_piece_removal
      && TSTFLAG(move_effect_journal[capture].u.piece_removal.flags,Royal))
  {
    /* out of here */
  }
  else
  {
    /* reserve a spot in the move effect journal for the case that a move by an invisible
     * turns out to move a side's king square
     */
    move_effect_journal_do_null_effect(move_effect_no_reason);
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",(void *)st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleMoveSequenceMoveRepeater);
    SLICE_STARTER(prototype) = SLICE_STARTER(si);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_the_pipe(slice_index si,
                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_castling_player(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  dealloc_slice(SLICE_NEXT2(si));
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_self_check_guard(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const remembered = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *remembered = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_goal_guard(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index remembered = no_slice;

    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,&remembered);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remember_self_check_guard);
    stip_traverse_structure(SLICE_NEXT2(si),&st_nested);

    if (remembered!=no_slice)
    {
      slice_index prototype = alloc_pipe(STTotalInvisibleGoalGuard);
      SLICE_STARTER(prototype) = SLICE_STARTER(remembered);
      goal_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
      pipe_remove(remembered);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_replay_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;

    stip_structure_traversal_init_nested(&st_nested,st,0);
    // TODO prevent instrumentation in the first place?
    stip_structure_traversal_override_single(&st_nested,
                                             STFindShortest,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STFindAttack,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveEffectJournalUndoer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPostMoveIterationInitialiser,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STCastlingPlayer,
                                             &remove_castling_player);
    stip_structure_traversal_override_single(&st_nested,
                                             STMovePlayer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPawnPromoter,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveGenerator,
                                             &subsitute_generator);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachedTester,
                                             &subsitute_goal_guard);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remove_the_pipe);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void replace_self_check_guard(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* This iteration ends at STTotalInvisibleMoveSequenceTester. We can therefore
   * blindly tamper with all STSelfCheckGuard slices that we meet.
   */
  stip_traverse_structure_children_pipe(si,st);

  SLICE_TYPE(si) = STTotalInvisibleUninterceptableSelfCheckGuard;

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
void total_invisible_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STTotalInvisibleMoveSequenceTester,
                                             &instrument_replay_branch);
    stip_structure_traversal_override_single(&st,
                                             STSelfCheckGuard,
                                             &replace_self_check_guard);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  output_plaintext_check_indication_disabled = true;

  solving_instrument_moves_for_piece_generation(si,nr_sides,STTotalInvisibleSpecialMoveGenerator);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInvisiblesAllocator);
    slice_insertion_insert(si,&prototype,1);
  }

  current_consumption.is_king_unplaced[White] = being_solved.king_square[White]==initsquare;
  current_consumption.is_king_unplaced[Black] = being_solved.king_square[Black]==initsquare;

  pipe_solve_delegate(si);

  current_consumption.is_king_unplaced[White] = false;
  current_consumption.is_king_unplaced[Black] = false;

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
void total_invisible_invisibles_allocator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  initialise_invisible_piece_ids(being_solved.currPieceId);

  being_solved.currPieceId += total_invisible_number;

  pipe_solve_delegate(si);

  being_solved.currPieceId -= total_invisible_number;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean instrumenting;
    slice_index the_copy;
    stip_length_type length;
} insertion_state_type;

static void insert_copy(slice_index si,
                        stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->the_copy==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const substitute = alloc_pipe(STTotalInvisibleMoveSequenceTester);
    pipe_link(proxy,substitute);
    link_to_branch(substitute,state->the_copy);
    SLICE_NEXT2(substitute) = state->the_copy;
    state->the_copy = no_slice;
    dealloc_slices(SLICE_NEXT2(si));
    SLICE_NEXT2(si) = proxy;

    assert(state->length!=no_stip_length);
    if (state->length%2!=0)
      pipe_append(proxy,alloc_pipe(STMoveInverter));

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleRevealAfterFinalMove)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      help_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_help_branch(slice_index si,
                             stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->instrumenting);
  TraceEOL();

  state->length = slices[si].u.branch.length;

  if (state->instrumenting)
    stip_traverse_structure_children(si,st);
  else
  {
    state->instrumenting = true;
    state->the_copy = stip_deep_copy(si);
    stip_traverse_structure_children(si,st);
    assert(state->the_copy==no_slice);

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement),
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement)
      };
      enum { nr_protypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_protypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // later:
  // - in original
  //   - insert revelation logic
  // - in copy
  //   - logic for iteration over all possibilities of invisibles
  //     - special case of invisible king
  //     - special case: position has no legal placement of all invisibles may have to be dealt with:
  //       - not enough empty squares :-)
  //   - substitute for STFindShortest

  // bail out at STAttackAdapter

  // input for total_invisible_number, initialize to 0

  // what about:
  // - structured stipulations?
  // - goals that don't involve immobility
  // ?

  // we shouldn't need to set the starter of
  // - STTotalInvisibleMoveSequenceMoveRepeater
  // - STTotalInvisibleGoalGuard

  // check indication should also be deactivated in tree output

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInstrumenter);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    insertion_state_type state = { false, no_slice, no_stip_length };

    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_single(&st,
                                             STHelpAdapter,
                                             &copy_help_branch);
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &insert_copy);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  move_effect_journal_register_pre_capture_effect();

  move_effect_journal_set_effect_doers(move_effect_revelation_of_new_invisible,
                                       &undo_revelation_of_new_invisible,
                                       &redo_revelation_of_new_invisible);

  move_effect_journal_set_effect_doers(move_effect_revelation_of_placed_invisible,
                                       &undo_revelation_of_placed_invisible,
                                       &redo_revelation_of_placed_invisible);

  solving_instrument_check_testing(si,STNoKingCheckTester);

  TraceFunctionResultEnd();
  TraceFunctionExit(__func__);
}
