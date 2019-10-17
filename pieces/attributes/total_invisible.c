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
#include "debugging/assert.h"
#include "debugging/trace.h"

unsigned int total_invisible_number;

ply top_ply_of_regular_play;
slice_index tester_slice;

stip_length_type combined_result;

play_phase_type play_phase = play_regular;

ply flesh_out_move_highwater = ply_retro_move;

void report_deadend(char const *s, unsigned int lineno)
{
  printf("%s;%u;%u\n",s,lineno,play_phase);
}

static void write_history_recursive(ply ply)
{
  if (parent_ply[ply]>ply_retro_move)
    write_history_recursive(parent_ply[ply]);

  {
    move_effect_journal_index_type const base = move_effect_journal_base[ply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

    fprintf(stdout," %u:",ply);
    WriteWalk(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.from);
    fputs("-",stdout);
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.to);
  }
}

void total_invisible_write_flesh_out_history(void)
{
  if (total_invisible_number>0 && nbply!=ply_nil)
  {
    fprintf(stdout," -");
    write_history_recursive(top_ply_of_regular_play);
  }
}

void restart_from_scratch(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(uninterceptable_check_delivered_from);
  TraceValue("%u",uninterceptable_check_delivered_in_ply);
  TraceEOL();

  if (uninterceptable_check_delivered_from!=initsquare
      && (uninterceptable_check_delivered_in_ply>=nbply
          || nbply==ply_retro_move+1))
  {
    REPORT_DECISION_OUTCOME("%s","piece delivering uninterceptable check can't be captured by random move");
    REPORT_DEADEND;
  }
  else if (nbply==ply_retro_move+1)
  {
    TraceValue("%u",nbply);TraceEOL();
    start_iteration();
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

void recurse_into_child_ply(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  move_effect_journal_index_type const save_top = move_effect_journal_base[nbply+1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(sq_departure==move_by_invisible
         || GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec)!=NullPieceId);

  move_effect_journal_base[nbply+1] = top_before_relevations[nbply];
  assert(top_before_relevations[nbply]>move_effect_journal_base[nbply]);
  redo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

  test_and_execute_revelations(top_before_relevations[nbply]);

  move_effect_journal_base[nbply+1] = top_before_relevations[nbply];
  assert(top_before_relevations[nbply]>move_effect_journal_base[nbply]);
  undo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

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
      recurse_into_child_ply();
    }
    else
    {
      PieceIdType const id_captured = GetPieceId(being_solved.spec[to]);
      purpose_type const save_purpose = motivation[id_captured].last.purpose;

      TraceText("capture of a total invisible that happened to land on the arrival square\n");

      assert(TSTFLAG(being_solved.spec[to],advers(trait[nbply])));
      assert(move_effect_journal[movement].u.piece_movement.moving!=Pawn);
      assert(!TSTFLAG(being_solved.spec[to],Royal));

      motivation[id_captured].last.purpose = purpose_none;

      move_effect_journal[capture].type = move_effect_piece_removal;
      move_effect_journal[capture].reason = move_effect_reason_regular_capture;
      move_effect_journal[capture].u.piece_removal.on = to;
      move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
      move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];

      recurse_into_child_ply();

      move_effect_journal[capture].type = move_effect_no_piece_removal;

      motivation[id_captured].last.purpose = save_purpose;
    }
  }
  else if (move_effect_journal[base].type==move_effect_piece_readdition)
  {
    assert(move_effect_journal[base].u.piece_addition.added.on==to);

    TraceText("capture of invisible victim added for the purpose\n");

    if (is_square_empty(to))
      recurse_into_child_ply();
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
        recurse_into_child_ply();
        move_effect_journal[capture].u.piece_removal.walk = walk_victim_orig;
      }
      else
      {
        TraceText("move is now blocked\n");
        REPORT_DECISION_OUTCOME("%s","move is now blocked");
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
      REPORT_DECISION_OUTCOME("%s","bad idea if the capturer is a pawn!");
      REPORT_DEADEND;
    }
    else
    {
      move_effect_journal[capture].type = move_effect_no_piece_removal;
      recurse_into_child_ply();
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
      // TODO always make sure we deal with an invisible before deactivating through motivation?
      // or never?
      PieceIdType const id_removed = GetPieceId(orig_flags_removed);
      purpose_type const orig_purpose_removed = motivation[id_removed].last.purpose;

      TraceValue("%x",orig_flags_removed);
      TraceValue("%u",id_removed);
      TraceEOL();

      motivation[id_removed].last.purpose = purpose_none;
      recurse_into_child_ply();
      motivation[id_removed].last.purpose = orig_purpose_removed;
    }
    else
      recurse_into_child_ply();

    move_effect_journal[capture].u.piece_removal.walk = orig_walk_removed;
    move_effect_journal[capture].u.piece_removal.flags = orig_flags_removed;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void adapt_pre_capture_effect(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const pre_capture = effects_base;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[pre_capture].type==move_effect_piece_readdition)
  {
    if (move_effect_journal[pre_capture].u.piece_addition.added.on==to)
    {
      TraceText("capture of invisible victim added for the purpose\n");

      if (is_square_empty(to))
      {
        if (!is_taboo(to,advers(trait[nbply]))
            && !has_been_taboo_since_random_move(to))
        {
          dynamic_consumption_type const save_consumption = current_consumption;

          if (allocate_flesh_out_unplaced(advers(trait[nbply])))
          {
            square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
            piece_walk_type const walk_added = move_effect_journal[pre_capture].u.piece_addition.added.walk;
            Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;

            assert(move_effect_journal[pre_capture].type==move_effect_piece_readdition);
            move_effect_journal[pre_capture].type = move_effect_none;
            occupy_square(sq_addition,walk_added,flags_added);
            restart_from_scratch();
            empty_square(sq_addition);
            move_effect_journal[pre_capture].type = move_effect_piece_readdition;
          }
          else
          {
            TraceText("no invisible left for placing it as victim\n");
            REPORT_DECISION_OUTCOME("%s","no invisible left for placing it as victim");
            REPORT_DEADEND;
          }

          current_consumption = save_consumption;
          TraceConsumption();TraceEOL();
        }
        else
        {
          TraceText("we should add a victim, but we can't because of how we have fleshed out earlier moves\n");
          REPORT_DECISION_OUTCOME("%s","we should add a victim, but we can't because of how we have fleshed out earlier moves");
          REPORT_DEADEND;
        }
      }
      else
      {
        PieceIdType const id = GetPieceId(being_solved.spec[to]);
        purpose_type const save_purpose = motivation[id].last.purpose;

        TraceText("another total invisible has appeared on the arrival square - "
                  "no need for addition any more!\n");
        move_effect_journal[pre_capture].type = move_effect_none;
        motivation[id].last.purpose = purpose_none;
        adapt_capture_effect();
        motivation[id].last.purpose = save_purpose;
        move_effect_journal[pre_capture].type = move_effect_piece_readdition;
      }
    }
    else
    {
      square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
      piece_walk_type const walk_added = move_effect_journal[pre_capture].u.piece_addition.added.walk;
      TraceText("addition of a castling partner - must have happened at diagram setup time\n");
      if (!is_square_empty(sq_addition)
          && sq_addition==knowledge[0].first_on
          && walk_added==knowledge[0].walk
          && knowledge[0].is_allocated)
      {
        // TODO this will always be the case once we fully use our knowledge
        TraceText("castling partner was added as part of applying our knowledge\n");
        move_effect_journal[pre_capture].type = move_effect_none;
        adapt_capture_effect();
        move_effect_journal[pre_capture].type = move_effect_piece_readdition;
      }
      else if (was_taboo(sq_addition,White) || was_taboo(sq_addition,Black))
      {
        TraceText("Hmm - some invisible piece must have traveled through the castling partner's square\n");
        REPORT_DECISION_OUTCOME("%s","Hmm - some invisible piece must have traveled through the castling partner's square");
        REPORT_DEADEND;
      }
      else
      {
        Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;
        Side const side_added = TSTFLAG(flags_added,White) ? White : Black;
        PieceIdType const id = GetPieceId(flags_added);
        decision_levels_type const save_levels = motivation[id].levels;

        assert(move_effect_journal[pre_capture].type==move_effect_piece_readdition);
        move_effect_journal[pre_capture].type = move_effect_none;
        // TODO can we do better here? the rook has ben here FOREVER
        motivation[id].levels.side = curr_decision_level;
        motivation[id].levels.walk = curr_decision_level;
        ++being_solved.number_of_pieces[side_added][walk_added];
        occupy_square(sq_addition,walk_added,flags_added);
        restart_from_scratch();
        empty_square(sq_addition);
        --being_solved.number_of_pieces[side_added][walk_added];
        move_effect_journal[pre_capture].type = move_effect_piece_readdition;

        motivation[id].levels = save_levels;
      }
    }
  }
  else
  {
    TraceText("no piece addition to be adapted\n");
    assert(move_effect_journal[pre_capture].type==move_effect_none);
    adapt_capture_effect();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void start_iteration(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  deal_with_illegal_checks();

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
    combined_result = previous_move_is_illegal;
    max_decision_level = decision_level_latest;
    REPORT_DECISION_CONTEXT(__func__);
    start_iteration();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_mate(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",combined_result);
  TraceValue("%u",combined_validation_result);
  TraceEOL();

  switch (combined_validation_result)
  {
    case mate_unvalidated:
      assert(combined_result==previous_move_is_illegal);
      break;

    case no_mate:
      assert(combined_result==previous_move_has_not_solved);
      break;

    case mate_attackable:
    case mate_defendable_by_interceptors:
      max_decision_level = decision_level_latest;
      combined_result = previous_move_is_illegal;
      REPORT_DECISION_CONTEXT(__func__);
      start_iteration();
      break;

    case mate_with_2_uninterceptable_doublechecks:
      /* we only replay moves for TI revelation */
      max_decision_level = decision_level_latest;
      combined_result = previous_move_is_illegal;
      REPORT_DECISION_CONTEXT(__func__);
      start_iteration();
      assert(combined_result==previous_move_has_solved);
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

  update_taboo(+1);
  update_nr_taboos_for_current_move_in_ply(+1);

  /* necessary for detecting checks by pawns and leapers */
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
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

    top_before_relevations[nbply] = move_effect_journal_base[nbply+1];

    play_phase = play_rewinding;
    rewind_effects();

    static_consumption.king[White] = being_solved.king_square[White]==initsquare;
    static_consumption.king[Black] = being_solved.king_square[Black]==initsquare;

    apply_knowledge(0,&validate_and_test);

    static_consumption.king[White] = false;
    static_consumption.king[Black] = false;

    play_phase = play_unwinding;
    unrewind_effects();
    play_phase = play_regular;

    solve_result = combined_result==immobility_on_next_move ? previous_move_has_not_solved : combined_result;
  }

  update_nr_taboos_for_current_move_in_ply(-1);
  update_taboo(-1);

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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* reserve a spot in the move effect journal for the case that a move by an invisible
   * turns out to move a side's king square
   */
  move_effect_journal_do_null_effect();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
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

  solving_instrument_move_generation(si,nr_sides,STTotalInvisibleSpecialMoveGenerator);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInvisiblesAllocator);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        ++nr_taboos_accumulated_until_ply[White][*s];
        ++nr_taboos_accumulated_until_ply[Black][*s];
      }
  }

  pipe_solve_delegate(si);

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        --nr_taboos_accumulated_until_ply[White][*s];
        --nr_taboos_accumulated_until_ply[Black][*s];
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

  move_effect_journal_set_effect_doers(move_effect_revelation_of_castling_partner,
                                       &undo_revelation_of_castling_partner,
                                       &redo_revelation_of_castling_partner);

  move_effect_journal_set_effect_doers(move_effect_revelation_of_placed_invisible,
                                       &undo_revelation_of_placed_invisible,
                                       &redo_revelation_of_placed_invisible);

  TraceFunctionResultEnd();
  TraceFunctionExit(__func__);
}
