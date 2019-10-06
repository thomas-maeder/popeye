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
#include "debugging/assert.h"
#include "debugging/trace.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/capture_by_invisible.h"

unsigned int total_invisible_number;

ply top_ply_of_regular_play;
static slice_index tester_slice;

static ply ply_replayed;

static stip_length_type combined_result;

play_phase_type play_phase = play_regular;

static mate_validation_type mate_validation_result;
static mate_validation_type combined_validation_result;

static square sq_mating_piece_to_be_attacked = initsquare;

// TODO what is a good size for this?
knowledge_type knowledge[MaxPieceId];
knowledge_index_type size_knowledge;

static motivation_type const motivation_null = {
    { purpose_none },
    { purpose_none },
    { 0 }
};

motivation_type motivation[MaxPieceId+1];

ply flesh_out_move_highwater = ply_retro_move;

PieceIdType top_visible_piece_id;
PieceIdType top_invisible_piece_id;

static ply uninterceptable_check_delivered_in_ply = ply_nil;
static square uninterceptable_check_delivered_from = initsquare;

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

boolean is_random_move_by_invisible(ply ply)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",ply);
  TraceFunctionParamListEnd();

  TraceSquare(move_effect_journal[movement].u.piece_movement.from);TraceEOL();

  result = move_effect_journal[movement].u.piece_movement.from==move_by_invisible;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void replay_fleshed_out_move_sequence(play_phase_type phase_replay)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",phase_replay);
  TraceFunctionParamListEnd();

  if (nbply>ply_retro_move+1)
  {
    --nbply;
    undo_move_effects();
    replay_fleshed_out_move_sequence(phase_replay);
    redo_move_effects();
    ++nbply;
  }
  else
  {
    ply_replayed = nbply;
    nbply = top_ply_of_regular_play;

    TracePosition(being_solved.board,being_solved.spec);

    play_phase = phase_replay;

    mate_validation_result = mate_unvalidated;

    pipe_solve_delegate(tester_slice);

    if (solve_result>combined_result)
      combined_result = solve_result;
    TracePosition(being_solved.board,being_solved.spec);

    play_phase = play_finalising_replay;

    nbply = ply_replayed;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_to(square sq_arrival);

static square const *find_next_backward_mover(square const *start_square)
{
  Side const side_playing = trait[nbply];
  square const *result;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  for (result = start_square; *result; ++result)
    if (TSTFLAG(being_solved.spec[*result],Chameleon)
        && TSTFLAG(being_solved.spec[*result],side_playing))
    {
      PieceIdType const id = GetPieceId(being_solved.spec[*result]);
      if (motivation[id].first.acts_when>nbply)
        break;
    }

  TraceFunctionExit(__func__);
  TraceSquare(*result);
  TraceFunctionResultEnd();
  return result;
}

static void retract_random_move_by_invisible(square const *start_square)
{
  square const *s;

  REPORT_DECISION_DECLARE(unsigned int const save_counter = report_decision_counter);

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  // TODO retract pawn captures?

  s = find_next_backward_mover(start_square);

  if (*s)
  {
    PieceIdType const id = GetPieceId(being_solved.spec[*s]);
    ply const save_when = motivation[id].first.acts_when;

    motivation[id].first.acts_when = nbply;

    flesh_out_random_move_by_specific_invisible_to(*s);

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      retract_random_move_by_invisible(s+1);
    }

    motivation[id].first.acts_when = save_when;
  }

#if defined(REPORT_DECISIONS)
  if (report_decision_counter==save_counter)
  {
    REPORT_DECISION_OUTCOME("%s","no retractable random move found - TODO we don't retract pawn captures");
    REPORT_DEADEND;
  }
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void fake_capture_by_invisible(void)
{
  ply const save_ply = uninterceptable_check_delivered_in_ply;

  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const precapture = effects_base;
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_entry_type const save_movement_entry = move_effect_journal[movement];

  Side const side = trait[nbply];
  Flags spec = BIT(side)|BIT(Chameleon);

  REPORT_DECISION_DECLARE(unsigned int const save_counter = report_decision_counter);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(!is_square_empty(uninterceptable_check_delivered_from));

  ++top_invisible_piece_id;
  SetPieceId(spec,top_invisible_piece_id);
  TraceValue("%u",top_invisible_piece_id);TraceEOL();

  assert(motivation[top_invisible_piece_id].last.purpose==purpose_none);
  motivation[top_invisible_piece_id].first.purpose = purpose_capturer;
  motivation[top_invisible_piece_id].first.acts_when = nbply;
  motivation[top_invisible_piece_id].first.on = capture_by_invisible;
  motivation[top_invisible_piece_id].last.purpose = purpose_capturer;
  motivation[top_invisible_piece_id].last.acts_when = nbply;
  motivation[top_invisible_piece_id].last.on = capture_by_invisible;

  assert(move_effect_journal[precapture].type==move_effect_none);
  move_effect_journal[precapture].type = move_effect_piece_readdition;
  move_effect_journal[precapture].u.piece_addition.added.on = capture_by_invisible;
  move_effect_journal[precapture].u.piece_addition.added.walk = Dummy;
  move_effect_journal[precapture].u.piece_addition.added.flags = spec;
  move_effect_journal[precapture].u.piece_addition.for_side = side;

  assert(move_effect_journal[capture].type==move_effect_no_piece_removal);
  move_effect_journal[capture].type = move_effect_piece_removal;
  move_effect_journal[capture].u.piece_removal.on = uninterceptable_check_delivered_from;
  move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(uninterceptable_check_delivered_from);
  move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[uninterceptable_check_delivered_from];

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  move_effect_journal[movement].type = move_effect_piece_movement;
  move_effect_journal[movement].u.piece_movement.from = capture_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = uninterceptable_check_delivered_from;
  move_effect_journal[movement].u.piece_movement.moving = Dummy;
  move_effect_journal[movement].u.piece_movement.movingspec = spec;

  ++being_solved.number_of_pieces[trait[nbply]][Dummy];
  occupy_square(capture_by_invisible,Dummy,spec);

  uninterceptable_check_delivered_from = initsquare;
  uninterceptable_check_delivered_in_ply = ply_nil;

  flesh_out_capture_by_invisible(nullsquare);

  uninterceptable_check_delivered_in_ply = save_ply;
  uninterceptable_check_delivered_from = move_effect_journal[capture].u.piece_removal.on;

  empty_square(capture_by_invisible);
  --being_solved.number_of_pieces[trait[nbply]][Dummy];

  move_effect_journal[movement] = save_movement_entry;
  move_effect_journal[capture].type = move_effect_no_piece_removal;
  move_effect_journal[precapture].type = move_effect_none;

  motivation[top_invisible_piece_id] = motivation_null;
  --top_invisible_piece_id;

#if defined(REPORT_DECISIONS)
  if (report_decision_counter==save_counter)
  {
    REPORT_DECISION_OUTCOME("%s","no invisible piece found that could capture the uninterceptable check deliverer");
    REPORT_DEADEND;
  }
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void backward_fleshout_random_move_by_invisible(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();


  if (uninterceptable_check_delivered_from!=initsquare
      && trait[uninterceptable_check_delivered_in_ply]!=trait[nbply])
  {
    // TODO what about king flights? they can even occur before uninterceptable_check_delivered_in_ply
    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      fake_capture_by_invisible();
    }
  }
  else
  {
    dynamic_consumption_type const save_consumption = current_consumption;

    current_consumption.claimed[trait[nbply]] = true;

    if (nr_total_invisbles_consumed()<=total_invisible_number)
    {
      dynamic_consumption_type const save_consumption = current_consumption;

      TraceText("stick to random move by unplaced invisible\n");
      current_consumption.claimed[trait[nbply]] = true;
      TraceConsumption();TraceEOL();
      if (nr_total_invisbles_consumed()<=total_invisible_number)
      {
        REPORT_DECISION_MOVE('<','-');
        ++curr_decision_level;
        restart_from_scratch();
        --curr_decision_level;
      }
      current_consumption = save_consumption;
      TraceConsumption();TraceEOL();
    }

    current_consumption = save_consumption;

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      retract_random_move_by_invisible(boardnum);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

static void place_mating_piece_attacker(Side side_attacking,
                                        square s,
                                        piece_walk_type walk)
{
  dynamic_consumption_type const save_consumption = current_consumption;
  Flags spec = BIT(side_attacking)|BIT(Chameleon);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  if (!has_been_taboo_since_random_move(s))
  {
    if (allocate_flesh_out_unplaced(side_attacking))
    {
      ++being_solved.number_of_pieces[side_attacking][walk];
      SetPieceId(spec,++top_invisible_piece_id);
      assert(motivation[top_invisible_piece_id].last.purpose==purpose_none);
      motivation[top_invisible_piece_id].first.purpose = purpose_attacker;
      motivation[top_invisible_piece_id].first.acts_when = top_ply_of_regular_play;
      motivation[top_invisible_piece_id].first.on = s;
      motivation[top_invisible_piece_id].last.purpose = purpose_attacker;
      motivation[top_invisible_piece_id].last.acts_when = top_ply_of_regular_play;
      motivation[top_invisible_piece_id].last.on = s;
      TraceValue("%u",top_invisible_piece_id);
      TraceValue("%u",motivation[top_invisible_piece_id].last.purpose);
      TraceEOL();
      occupy_square(s,walk,spec);
      restart_from_scratch();
      empty_square(s);
      motivation[top_invisible_piece_id] = motivation_null;
      --top_invisible_piece_id;
      --being_solved.number_of_pieces[side_attacking][walk];
    }
  }

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_placing_mating_piece_attacker(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  play_phase = play_initialising_replay;
  replay_fleshed_out_move_sequence(play_replay_testing);
  play_phase = play_attacking_mating_piece;

  if (solve_result==previous_move_has_not_solved)
    max_decision_level = decision_level_forever;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_rider(Side side_attacking,
                                               square sq_mating_piece,
                                               piece_walk_type walk_rider,
                                               vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && curr_decision_level<=max_decision_level; ++kcurr)
  {
    square s;
    for (s = sq_mating_piece+vec[kcurr]; curr_decision_level<=max_decision_level; s += vec[kcurr])
    {
      max_decision_level = decision_level_latest;
      if (is_square_empty(s))
      {
        TraceSquare(s);
        TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
        TraceEOL();

        if (nr_taboos_accumulated_until_ply[side_attacking][s]==0)
          place_mating_piece_attacker(side_attacking,s,walk_rider);
      }
      else
      {
        if ((get_walk_of_piece_on_square(s)==walk_rider
            || get_walk_of_piece_on_square(s)==Queen)
            && TSTFLAG(being_solved.spec[s],side_attacking))
          done_placing_mating_piece_attacker();

        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_leaper(Side side_attacking,
                                                square sq_mating_piece,
                                                piece_walk_type walk_leaper,
                                                vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && curr_decision_level<=max_decision_level; ++kcurr)
  {
    square const s = sq_mating_piece+vec[kcurr];

    TraceSquare(s);
    TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
    TraceEOL();

    max_decision_level = decision_level_latest;

    if (get_walk_of_piece_on_square(s)==walk_leaper
        && TSTFLAG(being_solved.spec[s],side_attacking))
      done_placing_mating_piece_attacker();
    else if (is_square_empty(s)
             && nr_taboos_accumulated_until_ply[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,walk_leaper);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_pawn(Side side_attacking,
                                              square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceFunctionParamListEnd();

  if (curr_decision_level<=max_decision_level)
  {
    square s = sq_mating_piece+dir_up+dir_left;

    TraceSquare(s);
    TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
    TraceEOL();

    max_decision_level = decision_level_latest;

    if (get_walk_of_piece_on_square(s)==Pawn
        && TSTFLAG(being_solved.spec[s],side_attacking))
      done_placing_mating_piece_attacker();
    else if (is_square_empty(s)
        && nr_taboos_accumulated_until_ply[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  if (curr_decision_level<=max_decision_level)
  {
    square s = sq_mating_piece+dir_up+dir_right;

    TraceSquare(s);
    TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
    TraceEOL();

    max_decision_level = decision_level_latest;

    if (get_walk_of_piece_on_square(s)==Pawn
        && TSTFLAG(being_solved.spec[s],side_attacking))
      done_placing_mating_piece_attacker();
    else if (is_square_empty(s)
             && nr_taboos_accumulated_until_ply[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void attack_mating_piece(Side side_attacking,
                                square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(sq_mating_piece);
  TraceFunctionParamListEnd();

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Bishop,
                                     vec_bishop_start,vec_bishop_end);

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Rook,
                                     vec_rook_start,vec_rook_end);

  place_mating_piece_attacking_leaper(side_attacking,
                                      sq_mating_piece,
                                      Knight,
                                      vec_knight_start,vec_knight_end);

  place_mating_piece_attacking_pawn(side_attacking,sq_mating_piece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_validating_king_placements(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TracePosition(being_solved.board,being_solved.spec);

  REPORT_EXIT;

  switch (play_phase)
  {
    case play_detecting_revelations:
      do_revelation_bookkeeping();
      break;

    case play_validating_mate:
      REPORT_DECISION_OUTCOME("%s","Replaying moves for validation");
      play_phase = play_initialising_replay;
      replay_fleshed_out_move_sequence(play_replay_validating);
      play_phase = play_validating_mate;

      if (mate_validation_result<combined_validation_result)
        combined_validation_result = mate_validation_result;
      if (mate_validation_result<=mate_attackable)
        max_decision_level = decision_level_forever;

      break;

    case play_testing_mate:
      if (combined_validation_result==mate_attackable)
      {
        play_phase = play_attacking_mating_piece;
        attack_mating_piece(advers(trait[top_ply_of_regular_play]),sq_mating_piece_to_be_attacked);
        play_phase = play_testing_mate;

        if (combined_result==previous_move_is_illegal)
          /* no legal placement found for a mating piece attacker */
          combined_result = previous_move_has_solved;
      }
      else
      {
        REPORT_DECISION_OUTCOME("%s","replaying moves for testing");
        play_phase = play_initialising_replay;
        replay_fleshed_out_move_sequence(play_replay_testing);
        play_phase = play_testing_mate;

        /* This:
         * assert(solve_result>=previous_move_has_solved);
         * held surprisingly long, especially since it's wrong.
         * E.g. mate by castling: if we attack the rook, the castling is not
         * even playable */
        if (solve_result==previous_move_has_not_solved)
          max_decision_level = decision_level_forever;
      }

      break;

    case play_attacking_mating_piece:
      REPORT_DECISION_OUTCOME("%s","placed mating piece attacker");
      done_placing_mating_piece_attacker();
      break;


    default:
      assert(0);
      break;
  }

//  printf("%u:",play_phase);
//  total_invisible_write_flesh_out_history();
//  printf(" - ");
//  {
//    square const *bnp;
//    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
//      if (!is_square_empty(*bnp) && !is_square_blocked(*bnp))
//      {
//        WriteSpec(&output_plaintext_engine,
//                  stdout,being_solved.spec[*bnp],
//                  being_solved.board[*bnp],true);
//        WriteWalk(&output_plaintext_engine,
//                  stdout,
//                  get_walk_of_piece_on_square(*bnp));
//        WriteSquare(&output_plaintext_engine,stdout,*bnp);
//        printf("(%lu) ",GetPieceId(being_solved.spec[*bnp]));
//      }
//  }
//  switch (play_phase)
//  {
//    case play_detecting_revelations:
//      printf("-");
//      {
//        unsigned int i;
//        for (i = 0; i!=nr_potential_revelations; ++i)
//        {
//          printf(TSTFLAG(revelation_status[i].spec,White) ? " w" : " s");
//          WriteWalk(&output_plaintext_engine,
//                    stdout,
//                    revelation_status[i].walk);
//          WriteSquare(&output_plaintext_engine,stdout,revelation_status[i].pos);
//          printf("(%lu)",GetPieceId(revelation_status[i].spec));
//        }
//      }
//      break;
//
//    default:
//      break;
//  }
//  printf(":%u\n",end_of_iteration);

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void nominate_king_invisible_by_invisible(void)
{
  Side const side_to_be_mated = Black;
  Side const side_mating = White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const *s;
    TraceText("Try to make one of the placed invisibles the king to be mated\n");
    for (s = boardnum; *s && curr_decision_level<=max_decision_level; ++s)
      if (get_walk_of_piece_on_square(*s)==Dummy
          && TSTFLAG(being_solved.spec[*s],side_to_be_mated))
      {
        Flags const save_flags = being_solved.spec[*s];
        CLRFLAG(being_solved.spec[*s],side_mating);
        SETFLAG(being_solved.spec[*s],Royal);
        ++being_solved.number_of_pieces[side_to_be_mated][King];
        being_solved.board[*s] = King;
        being_solved.king_square[side_to_be_mated] = *s;
        TraceSquare(*s);TraceEOL();
        max_decision_level = decision_level_latest;
        REPORT_DECISION_KING_NOMINATION(*s);
        ++curr_decision_level;
        restart_from_scratch();
        --curr_decision_level;
        being_solved.board[*s] = Dummy;
        --being_solved.number_of_pieces[side_to_be_mated][King];
        being_solved.spec[*s] = save_flags;
      }

    being_solved.king_square[side_to_be_mated] = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void indistinct_king_placement_validation(void)
{
  Side const side_to_be_mated = Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved.king_square[side_to_be_mated] = initsquare;
  TraceText("The king to be mated can be anywhere\n");
  REPORT_DECISION_OUTCOME("%s","The king to be mated can be anywhere");
  REPORT_DEADEND;

  switch (play_phase)
  {
    case play_detecting_revelations:
      do_revelation_bookkeeping();
      break;

    case play_validating_mate:
      combined_validation_result = no_mate;
      combined_result = previous_move_has_not_solved;
      max_decision_level = decision_level_forever;
      break;

    default:
      combined_result = previous_move_has_not_solved;
      max_decision_level = decision_level_forever;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void impossible_king_placement_validation(void)
{
  Side const side_to_be_mated = Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved.king_square[side_to_be_mated] = initsquare;
  TraceText("The king to be mated can't be placed\n");
  REPORT_DECISION_OUTCOME("%s","The king to be mated can't be placed");
  REPORT_DEADEND;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_king_placements(void)
{
  Side const side_to_be_mated = Black;
  Side const side_mating = White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,side_to_be_mated);
  TraceSquare(being_solved.king_square[side_to_be_mated]);
  TraceConsumption();
  TraceEnumerator(Side,side_mating);
  TraceSquare(being_solved.king_square[side_mating]);
  TraceEOL();
  TracePosition(being_solved.board,being_solved.spec);

  assert(being_solved.king_square[side_to_be_mated]!=nullsquare);

  if (being_solved.king_square[side_to_be_mated]==initsquare)
  {
    being_solved.king_square[side_to_be_mated] = nullsquare;
    if (nr_placeable_invisibles_for_side(side_to_be_mated)==0)
    {
      being_solved.king_square[side_to_be_mated] = initsquare;
      if (current_consumption.placed[side_to_be_mated]>0)
      {
        dynamic_consumption_type const save_consumption = current_consumption;

        allocate_flesh_out_placed(side_to_be_mated);
        nominate_king_invisible_by_invisible();
        current_consumption = save_consumption;
      }
      else
        impossible_king_placement_validation();
    }
    else
      indistinct_king_placement_validation();
  }
  else if (being_solved.king_square[side_mating]==initsquare
           && nr_total_invisbles_consumed()==total_invisible_number)
  {
    combined_result = previous_move_is_illegal;
    REPORT_DECISION_OUTCOME("%s","The king of the mating side can't be placed");
    REPORT_DEADEND;
  }
  else
    done_validating_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean is_capture_by_invisible_possible(ply ply)
{
  boolean result;
  dynamic_consumption_type const save_consumption = current_consumption;

  TraceFunctionEntry(__func__);
  TraceValue("%u",ply);
  TraceFunctionParamListEnd();

  if (allocate_flesh_out_unplaced(trait[ply]))
  {
    /* no problem - we can simply insert a capturer */
    result = true;
  }
  else
  {
    square const save_king_square = being_solved.king_square[trait[ply]];

    /* pretend that the king is placed; necessary if only aptures by the invisble king
     * are possisble */
    being_solved.king_square[trait[ply]] = square_a1;

    current_consumption = save_consumption;

    if (allocate_flesh_out_unplaced(trait[ply]))
    {
      /* no problem - we can simply insert a capturing king */
      result = true;
    }
    else
    {
      move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];

      move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
      square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
      PieceIdType id;

      /* only captures by existing invisibles are viable - can one of them reach the arrival square at all? */
      result = false; /* not until we have proved it */

      for (id = top_visible_piece_id+1; id<=top_invisible_piece_id; ++id)
      {
        TraceValue("%u",id);

        TraceValue("%u",motivation[id].first.purpose);
        TraceValue("%u",motivation[id].first.acts_when);
        TraceSquare(motivation[id].first.on);

        TraceValue("%u",motivation[id].last.purpose);
        TraceValue("%u",motivation[id].last.acts_when);
        TraceSquare(motivation[id].last.on);

        TraceWalk(get_walk_of_piece_on_square(motivation[id].last.on));
        TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
        TraceEOL();
      }

      if (ply==nbply)
      {
        for (id = top_visible_piece_id+1; id<=top_invisible_piece_id; ++id)
        {
          TraceValue("%u",id);TraceEOL();
          assert((motivation[id].first.acts_when>ply) // active in the future
                 || (motivation[id].first.acts_when==ply && motivation[id].first.purpose!=purpose_interceptor) // to become active later in this ply
                 || (motivation[id].first.acts_when==ply && motivation[id].first.purpose==purpose_interceptor) // revealed interceptor - not necessarly present
                 || (motivation[id].first.acts_when<ply && motivation[id].last.acts_when>ply) // in action
                 || (motivation[id].last.purpose==purpose_none && motivation[id].last.acts_when<ply) // put on hold by a revelation or capture
                 || (GetPieceId(being_solved.spec[motivation[id].last.on])==id));
        }
      }

      for (id = top_visible_piece_id+1;
           !result && id<=top_invisible_piece_id;
           ++id)
      {
        square const on = motivation[id].last.on;
        Flags const spec = being_solved.spec[motivation[id].last.on];

        if (GetPieceId(spec)==id)
        {
          if (TSTFLAG(spec,trait[ply]))
          {
            piece_walk_type const walk = get_walk_of_piece_on_square(on);
            int const diff = sq_arrival-on;

            switch (walk)
            {
              case King:
                if (CheckDir[Queen][diff]==diff)
                  result = true;
                break;

              case Queen:
                if (CheckDir[Queen][diff]!=0)
                  result = true;
                break;

              case Rook:
                if (CheckDir[Rook][diff]!=0)
                  result = true;
                break;

              case Bishop:
                if (CheckDir[Bishop][diff]!=0)
                  result = true;
                break;

              case Knight:
                if (CheckDir[Knight][diff]==diff)
                  result = true;
                break;

              case Pawn:
                if (CheckDir[Bishop][diff]==diff)
                  result = true;
                break;

              case Dummy:
                if (CheckDir[Queen][diff]!=0 || CheckDir[Knight][diff]==diff)
                  result = true;
                break;

              default:
                // TODO assert(0);?
                break;
            }
          }
        }
        else if (motivation[id].first.acts_when==ply && motivation[id].first.purpose==purpose_interceptor)
        {
          REPORT_DECISION_OUTCOME("%s","revelation of interceptor is violated");
          REPORT_DEADEND;
          break;
        }
      }
    }

    being_solved.king_square[trait[ply]] = save_king_square;
  }

  current_consumption = save_consumption;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

static void done_fleshing_out_random_move_by_specific_invisible_to(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  Side const side_moving = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (!was_taboo(move_effect_journal[movement].u.piece_movement.from))
  {
    Side const side_attacked = advers(side_moving);
    square const king_pos = being_solved.king_square[side_attacked];

    /* we can't do undo_move_effects() because we might inadvertently undo a piece
     * revelation!
     */
    empty_square(move_effect_journal[movement].u.piece_movement.to);
    occupy_square(move_effect_journal[movement].u.piece_movement.from,
                  move_effect_journal[movement].u.piece_movement.moving,
                  move_effect_journal[movement].u.piece_movement.movingspec);

    if (!is_square_uninterceptably_attacked(side_attacked,king_pos))
    {
      PieceIdType const id = GetPieceId(being_solved.spec[move_effect_journal[movement].u.piece_movement.from]);
      motivation_type const save_motivation = motivation[id];

      motivation[id].first.purpose = purpose_random_mover;
      motivation[id].first.on = move_effect_journal[movement].u.piece_movement.from;
      motivation[id].first.acts_when = nbply;

      update_nr_taboos_for_current_move_in_ply(+1);
      REPORT_DECISION_MOVE('<','-');
      ++curr_decision_level;
      restart_from_scratch();
      --curr_decision_level;

      /* it is irrevelant where the random move in the 1st ply is played from */
      if (nbply==ply_retro_move+1
          && (play_phase==play_detecting_revelations
              || play_phase==play_validating_mate
              || play_phase==play_testing_mate)
          && max_decision_level>curr_decision_level-1)
        max_decision_level = curr_decision_level-1;

      update_nr_taboos_for_current_move_in_ply(-1);

      motivation[id] = save_motivation;
    }

    empty_square(move_effect_journal[movement].u.piece_movement.from);
    occupy_square(move_effect_journal[movement].u.piece_movement.to,
                  move_effect_journal[movement].u.piece_movement.moving,
                  move_effect_journal[movement].u.piece_movement.movingspec);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_rider_to(vec_index_type kstart,
                                                                 vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kstart);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    while (curr_decision_level<=max_decision_level
           && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      max_decision_level = decision_level_latest;
      done_fleshing_out_random_move_by_specific_invisible_to();
      move_effect_journal[movement].u.piece_movement.from -= vec[k];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_king_to(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(being_solved.king_square[trait[nbply]]==move_effect_journal[movement].u.piece_movement.to);

  assert(move_effect_journal[king_square_movement].type==move_effect_none);
  move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
  move_effect_journal[king_square_movement].u.king_square_movement.to = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

  for (k = vec_queen_start; k<=vec_queen_end && curr_decision_level<=max_decision_level; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      max_decision_level = decision_level_latest;
      being_solved.king_square[trait[nbply]] = move_effect_journal[movement].u.piece_movement.from;
      move_effect_journal[king_square_movement].u.king_square_movement.from = move_effect_journal[movement].u.piece_movement.from;
      done_fleshing_out_random_move_by_specific_invisible_to();
    }
  }

  being_solved.king_square[trait[nbply]] = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal[king_square_movement].type = move_effect_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_leaper_to(vec_index_type kstart,
                                                                  vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kstart);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      max_decision_level = decision_level_latest;
      done_fleshing_out_random_move_by_specific_invisible_to();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_pawn_to(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  Side const side_moving = trait[nbply];
  int const dir = side_moving==White ? dir_up : dir_down;
  SquareFlags const promsq = side_moving==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side_moving==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-dir;
  TraceSquare(move_effect_journal[movement].u.piece_movement.from);
  TraceEOL();

  if (is_square_empty(move_effect_journal[movement].u.piece_movement.from)
      && !TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],basesq)
      && !TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],promsq))
  {
    done_fleshing_out_random_move_by_specific_invisible_to();

    if (curr_decision_level<=max_decision_level)
    {
      SquareFlags const doublestepsq = side_moving==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

      move_effect_journal[movement].u.piece_movement.from -= dir;
      if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublestepsq)
          && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
      {
        max_decision_level = decision_level_latest;
        done_fleshing_out_random_move_by_specific_invisible_to();
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_to_according_to_walk(square sq_arrival)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceSquare(motivation[id].first.on);TraceEOL();
  TraceWalk(get_walk_of_piece_on_square(sq_arrival));TraceEOL();

  assert(motivation[id].first.on==sq_arrival);
  assert(get_walk_of_piece_on_square(sq_arrival)!=Dummy);
  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
  assert(move_effect_journal[movement].u.piece_movement.to==move_by_invisible);
  assert(move_effect_journal[movement].u.piece_movement.moving==Empty);
  assert(move_effect_journal[movement].u.piece_movement.movingspec==0);

  move_effect_journal[movement].u.piece_movement.to = sq_arrival;
  move_effect_journal[movement].u.piece_movement.moving = get_walk_of_piece_on_square(sq_arrival);
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_arrival];

  switch (move_effect_journal[movement].u.piece_movement.moving)
  {
    case King:
      flesh_out_random_move_by_specific_invisible_king_to();
      break;

    case Queen:
      flesh_out_random_move_by_specific_invisible_rider_to(vec_queen_start,
                                                           vec_queen_end);
      break;

    case Rook:
      flesh_out_random_move_by_specific_invisible_rider_to(vec_rook_start,
                                                           vec_rook_end);
      break;

    case Bishop:
      flesh_out_random_move_by_specific_invisible_rider_to(vec_bishop_start,
                                                           vec_bishop_end);
      break;

    case Knight:
      flesh_out_random_move_by_specific_invisible_leaper_to(vec_knight_start,
                                                            vec_knight_end);
      break;

    case Pawn:
      flesh_out_random_move_by_specific_invisible_pawn_to();
      break;

    default:
      break;
  }

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.moving = Empty;
  move_effect_journal[movement].u.piece_movement.movingspec = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_to(square sq_arrival)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceValue("%u",id);
  TraceValue("%u",motivation[id].first.purpose);
  TraceValue("%u",motivation[id].first.acts_when);
  TraceSquare(motivation[id].first.on);
  TraceValue("%u",motivation[id].last.purpose);
  TraceValue("%u",motivation[id].last.acts_when);
  TraceSquare(motivation[id].last.on);
  TraceEOL();

  if (motivation[id].first.on==sq_arrival)
  {
    TraceWalk(get_walk_of_piece_on_square(sq_arrival));TraceEOL();
    if (get_walk_of_piece_on_square(sq_arrival)==Dummy)
    {
      Side const side_playing = trait[nbply];
      piece_walk_type walk;
      decision_levels_type const save_levels = motivation[id].levels;

      TraceValue("%u",id);
      TraceValue("%u",motivation[id].levels.walk);
      TraceEOL();
      assert(motivation[id].levels.walk==decision_level_latest);
      assert(motivation[id].levels.from==decision_level_latest);
      motivation[id].levels.walk = curr_decision_level;
      motivation[id].levels.from = curr_decision_level+1;

      for (walk = Pawn; walk<=Bishop && curr_decision_level<=max_decision_level; ++walk)
      {
        max_decision_level = decision_level_latest;

        REPORT_DECISION_WALK('<',walk);
        ++curr_decision_level;

        ++being_solved.number_of_pieces[side_playing][walk];
        replace_walk(sq_arrival,walk);

        flesh_out_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);

        replace_walk(sq_arrival,Dummy);
        --being_solved.number_of_pieces[side_playing][walk];

        --curr_decision_level;
      }

      motivation[id].levels = save_levels;
    }
    else
      flesh_out_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);
  }
  else
  {
    TraceText("the piece has already moved\n");
    REPORT_DECISION_OUTCOME("%s","the piece has already moved");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void adapt_pre_capture_effect(void)
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
        if (nbply<=flesh_out_move_highwater)
        {
          TraceText("victim was placed in previous iteration\n");
          adapt_capture_effect();
        }
        else if (!is_taboo(to,advers(trait[nbply]))
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
      else if (was_taboo(sq_addition))
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

static void done_fleshing_out_random_move_by_invisible_from(boolean is_dummy_moving)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  PieceIdType const id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
  motivation_type const save_motivation = motivation[id];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  REPORT_DECISION_MOVE('>','-');
  ++curr_decision_level;

  motivation[id].last.on = sq_arrival;

  update_nr_taboos_for_current_move_in_ply(+1);
  if (is_dummy_moving)
    restart_from_scratch();
  else
    recurse_into_child_ply();
  update_nr_taboos_for_current_move_in_ply(-1);

  motivation[id] = save_motivation;

  --curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_accidental_capture_by_invisible(boolean is_dummy_moving)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  if (is_on_board(sq_arrival)
      && TSTFLAG(being_solved.spec[sq_arrival],advers(trait[nbply]))
      && TSTFLAG(being_solved.spec[sq_arrival],Chameleon))
  {
    PieceIdType const id_victim = GetPieceId(being_solved.spec[sq_arrival]);

    TraceValue("%u",id_victim);
    TraceValue("%u",motivation[id_victim].first.purpose);
    TraceValue("%u",motivation[id_victim].first.acts_when);
    TraceSquare(motivation[id_victim].first.on);
    TraceValue("%u",motivation[id_victim].last.purpose);
    TraceValue("%u",motivation[id_victim].last.acts_when);
    TraceSquare(motivation[id_victim].last.on);
    TraceEOL();

    assert(motivation[id_victim].first.purpose!=purpose_none);
    assert(motivation[id_victim].last.purpose!=purpose_none);
    if (motivation[id_victim].last.acts_when>nbply)
    {
      TraceText("the planned victim was added to later act from its current square\n");
      REPORT_DECISION_OUTCOME("%s","the planned victim was added to later act from its current square");
      REPORT_DEADEND;
    }
    else
    {
      PieceIdType const id_captured = GetPieceId(being_solved.spec[sq_arrival]);
      action_type const save_last = motivation[id_captured].last;

      motivation[id_captured].last.purpose = purpose_none;

      assert(move_effect_journal[capture].type==move_effect_no_piece_removal);
      move_effect_journal[capture].type = move_effect_piece_removal;
      move_effect_journal[capture].u.piece_removal.on = sq_arrival;
      move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(sq_arrival);
      move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[sq_arrival];

      done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);

      move_effect_journal[capture].type = move_effect_no_piece_removal;

      motivation[id_captured].last = save_last;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_pawn_from(boolean is_dummy_moving)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  Side const side = trait[nbply];
  int const dir = side==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  {
    square const sq_singlestep = move_effect_journal[movement].u.piece_movement.from+dir;
    TraceSquare(sq_singlestep);TraceEOL();
    if (is_square_empty(sq_singlestep))
    {
      if (!is_taboo(sq_singlestep,side))
      {
        move_effect_journal[movement].u.piece_movement.to = sq_singlestep;
        // TODO promotion
        done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
      }

      if (curr_decision_level<=max_decision_level)
      {
        SquareFlags const doublstepsq = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
        if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublstepsq))
        {
          square const sq_doublestep = sq_singlestep+dir;
          TraceSquare(sq_doublestep);TraceEOL();
          if (is_square_empty(sq_doublestep))
          {
            if (!is_taboo(sq_doublestep,side))
            {
              max_decision_level = decision_level_latest;
              move_effect_journal[movement].u.piece_movement.to = sq_doublestep;
              done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
            }
          }
        }
      }
    }

    // TODO add capture victim if arrival square empty and nr_total...>0

    if (curr_decision_level<=max_decision_level)
    {
      square const sq_arrival = sq_singlestep+dir_right;
      max_decision_level = decision_level_latest;
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      flesh_out_accidental_capture_by_invisible(is_dummy_moving);
    }

    if (curr_decision_level<=max_decision_level)
    {
      square const sq_arrival = sq_singlestep+dir_left;
      max_decision_level = decision_level_latest;
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      flesh_out_accidental_capture_by_invisible(is_dummy_moving);
    }
  }


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_rider_from(vec_index_type kstart,
                                                          vec_index_type kend,
                                                          boolean is_dummy_moving)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceValue("%u",kstart);
  TraceValue("%u",kend);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    square sq_arrival;
    for (sq_arrival = move_effect_journal[movement].u.piece_movement.from+vec[k];
         curr_decision_level<=max_decision_level;
         sq_arrival += vec[k])
    {
      TraceSquare(sq_arrival);TraceEOL();
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;

      max_decision_level = decision_level_latest;

      /* "factoring out" the invokations of is_taboo() is tempting, but we
       * want to break the loop if sq_arrival is not empty whether or not
       * that square is nr_taboos_accumulated_until_ply!
       */
      if (is_square_empty(sq_arrival))
      {
        if (!is_taboo(sq_arrival,trait[nbply]))
          done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
      }
      else
      {
        if (!is_taboo(sq_arrival,trait[nbply]))
          flesh_out_accidental_capture_by_invisible(is_dummy_moving);
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_leaper_from(vec_index_type kstart,
                                                           vec_index_type kend,
                                                           boolean is_dummy_moving)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kstart);
  TraceFunctionParam("%u",kend);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  TraceWalk(get_walk_of_piece_on_square(sq_departure));TraceEOL();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    square const sq_arrival = sq_departure+vec[k];
    if (!is_taboo(sq_arrival,trait[nbply]))
    {
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      /* just in case: */
      move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;

      max_decision_level = decision_level_latest;

      if (is_square_empty(sq_arrival))
        done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
      else
        flesh_out_accidental_capture_by_invisible(is_dummy_moving);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_existing_invisible_from(square sq_departure,
                                                             boolean is_dummy_moving)
{
  piece_walk_type const walk_on_square = get_walk_of_piece_on_square(sq_departure);
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  piece_walk_type const save_walk_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_flags_moving = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  TraceWalk(walk_on_square);TraceEOL();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  move_effect_journal[movement].u.piece_movement.moving = walk_on_square;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

  switch (walk_on_square)
  {
    case King:
      assert(move_effect_journal[king_square_movement].type==move_effect_none);
      move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
      move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
      move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];
      flesh_out_random_move_by_invisible_leaper_from(vec_queen_start,vec_queen_end,
                                                     is_dummy_moving);
      move_effect_journal[king_square_movement].type = move_effect_none;
      break;

    case Queen:
      flesh_out_random_move_by_invisible_rider_from(vec_queen_start,vec_queen_end,
                                                    is_dummy_moving);
      break;

    case Rook:
      flesh_out_random_move_by_invisible_rider_from(vec_rook_start,vec_rook_end,
                                                    is_dummy_moving);
      break;

    case Bishop:
      flesh_out_random_move_by_invisible_rider_from(vec_bishop_start,vec_bishop_end,
                                                    is_dummy_moving);
      break;

    case Knight:
      flesh_out_random_move_by_invisible_leaper_from(vec_knight_start,vec_knight_end,
                                                     is_dummy_moving);
      break;

    case Pawn:
      flesh_out_random_move_by_invisible_pawn_from(is_dummy_moving);
      break;

    default:
      break;
  }

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.moving = save_walk_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_flags_moving;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_existing_invisible_as_non_king_from(square sq_departure)
{
  Side const side_playing = trait[nbply];
  Side const side_under_attack = advers(side_playing);
  square const king_pos = being_solved.king_square[side_under_attack];

  PieceIdType const id_moving = GetPieceId(being_solved.spec[sq_departure]);
  decision_level_type const save_level = motivation[id_moving].levels.walk;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  {
    SquareFlags const promsq = side_playing==White ? WhPromSq : BlPromSq;
    SquareFlags const basesq = side_playing==White ? WhBaseSq : BlBaseSq;
    if (!(TSTFLAG(sq_spec[sq_departure],basesq) || TSTFLAG(sq_spec[sq_departure],promsq)))
    {
      motivation[id_moving].levels.walk = curr_decision_level;
      REPORT_DECISION_WALK('>',Pawn);
      ++curr_decision_level;

      ++being_solved.number_of_pieces[side_playing][Pawn];
      replace_walk(sq_departure,Pawn);
      if (!(king_pos!=initsquare && pawn_check_ortho(side_playing,king_pos)))
        flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
      --being_solved.number_of_pieces[side_playing][Pawn];

      --curr_decision_level;
    }
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Knight);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Knight];
    replace_walk(sq_departure,Knight);
    if (!(king_pos!=initsquare && knight_check_ortho(side_playing,king_pos)))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Knight];

    --curr_decision_level;
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Bishop);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Bishop];
    replace_walk(sq_departure,Bishop);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_bishop_start,vec_bishop_end,
                                        Bishop))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Bishop];

    --curr_decision_level;
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Rook);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Rook];
    replace_walk(sq_departure,Rook);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_rook_start,vec_rook_end,
                                        Rook))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Rook];

    --curr_decision_level;
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Queen);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Queen];
    replace_walk(sq_departure,Queen);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_queen_start,vec_queen_end,
                                        Queen))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Queen];

    --curr_decision_level;
  }

  motivation[id_moving].levels.walk = save_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
static void flesh_out_random_move_by_specific_invisible_from(square sq_departure)
{
  Side const side_playing = trait[nbply];
  piece_walk_type const walk_on_square = get_walk_of_piece_on_square(sq_departure);
  Flags const save_flags = being_solved.spec[sq_departure];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  CLRFLAG(being_solved.spec[sq_departure],advers(side_playing));

  TraceWalk(walk_on_square);
  TraceSquare(sq_departure);
  TraceEOL();

  // TODO use a sibling ply and the regular move generation machinery?

  if (walk_on_square==Dummy)
  {
    Side const side_playing = trait[nbply];
    Side const side_under_attack = advers(side_playing);
    square const king_pos = being_solved.king_square[side_under_attack];
    dynamic_consumption_type const save_consumption = current_consumption;
    PieceIdType const id_moving = GetPieceId(save_flags);

    assert(play_phase==play_validating_mate);

    assert(TSTFLAG(being_solved.spec[sq_departure],side_playing));
    assert(!TSTFLAG(being_solved.spec[sq_departure],side_under_attack));

    allocate_flesh_out_placed(side_playing);

    if (being_solved.king_square[side_playing]==initsquare)
    {
      boolean are_allocations_exhausted;

      being_solved.king_square[side_playing] = sq_departure;

      are_allocations_exhausted  = nr_total_invisbles_consumed()==total_invisible_number;

      ++being_solved.number_of_pieces[side_playing][King];
      replace_walk(sq_departure,King);
      SETFLAG(being_solved.spec[sq_departure],Royal);
      if (!(king_pos!=initsquare && king_check_ortho(side_playing,king_pos)))
      {
        decision_level_type const save_level = motivation[id_moving].levels.walk;

        motivation[id_moving].levels.walk = curr_decision_level;
        REPORT_DECISION_WALK('>',King);
        ++curr_decision_level;

        flesh_out_random_move_by_existing_invisible_from(sq_departure,true);

        --curr_decision_level;

        motivation[id_moving].levels.walk = save_level;
      }
      CLRFLAG(being_solved.spec[sq_departure],Royal);
      --being_solved.number_of_pieces[side_playing][King];
      being_solved.king_square[side_playing] = initsquare;

      if (curr_decision_level<=max_decision_level
          && !are_allocations_exhausted)
      {
        max_decision_level = decision_level_latest;
        flesh_out_random_move_by_existing_invisible_as_non_king_from(sq_departure);
      }
    }
    else
      flesh_out_random_move_by_existing_invisible_as_non_king_from(sq_departure);

    current_consumption = save_consumption;

    replace_walk(sq_departure,walk_on_square);
    being_solved.spec[sq_departure] = save_flags;
  }
  else
    flesh_out_random_move_by_existing_invisible_from(sq_departure,false);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square const *find_next_forward_mover(square const *start_square)
{
  Side const side_playing = trait[nbply];
  square const *result;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  for (result = boardnum; *result; ++result)
    if (TSTFLAG(being_solved.spec[*result],Chameleon)
        && TSTFLAG(being_solved.spec[*result],side_playing))
    {
      PieceIdType const id = GetPieceId(being_solved.spec[*result]);

      if (motivation[id].last.acts_when<nbply
          || (motivation[id].last.purpose==purpose_interceptor
              && motivation[id].last.acts_when<=nbply))
        break;
    }

  TraceFunctionExit(__func__);
  TraceSquare(*result);
  TraceFunctionResultEnd();
  return result;
}

/* This function is recursive so that we remember which invisibles have already been
 * visited
 */
static void forward_random_move_by_invisible(square const *start_square)
{
  square const *s;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  s = find_next_forward_mover(start_square);

  if (*s)
  {
    PieceIdType const id = GetPieceId(being_solved.spec[*s]);
    action_type const save_last = motivation[id].last;

    motivation[id].last.acts_when = nbply;
    motivation[id].last.purpose = purpose_random_mover;

    REPORT_DECISION_SQUARE('>',*s);
    ++curr_decision_level;
    flesh_out_random_move_by_specific_invisible_from(*s);
    --curr_decision_level;

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      forward_random_move_by_invisible(s+1);
    }

    motivation[id].last = save_last;
  }
  else
  {
    // TODO Strictly speaking, there is no guarantee that such a move exists
    // but we probably save a lot of time by not fleshing it out. As long as we
    // restrict ourselves to h#n, the risk is printing some wrong cooks.
    // Options:
    // * find out how hight the cost would be
    // * fleshing it out
    // * option for activating fleshing out

    dynamic_consumption_type const save_consumption = current_consumption;

    TraceText("random move by unplaced invisible\n");

    current_consumption.claimed[trait[nbply]] = true;
    TraceConsumption();TraceEOL();

    if (nr_total_invisbles_consumed()<=total_invisible_number)
    {
      REPORT_DECISION_MOVE('>','-');
      ++curr_decision_level;
      recurse_into_child_ply();
      --curr_decision_level;
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  forward_random_move_by_invisible(boardnum);

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
    square const first_taboo_violation = find_taboo_violation();

    TraceValue("%u",nbply);
    TraceValue("%u",top_ply_of_regular_play);
    TraceSquare(sq_departure);
    TraceValue("%u",sq_departure);
    TraceValue("%u",capture_by_invisible);
    TraceValue("%u",flesh_out_move_highwater);
    TraceSquare(sq_arrival);
    TraceValue("%u",sq_arrival);
    TraceValue("%u",move_by_invisible);
    TraceEOL();

    {
      PieceIdType id;
      for (id = top_visible_piece_id+1; id<=top_invisible_piece_id; ++id)
      {
        TraceValue("%u",id);

        TraceValue("%u",motivation[id].first.purpose);
        TraceValue("%u",motivation[id].first.acts_when);
        TraceSquare(motivation[id].first.on);

        TraceValue("%u",motivation[id].last.purpose);
        TraceValue("%u",motivation[id].last.acts_when);
        TraceSquare(motivation[id].last.on);

        TraceWalk(get_walk_of_piece_on_square(motivation[id].last.on));
        TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
        TraceEOL();
      }
    }

    {
      PieceIdType id;
      for (id = top_visible_piece_id+1; id<=top_invisible_piece_id; ++id)
      {
        TraceValue("%u",id);TraceEOL();
        assert((motivation[id].first.acts_when>nbply) // active in the future
               || (motivation[id].first.acts_when==nbply && motivation[id].first.purpose!=purpose_interceptor) // to become active later in this ply
               || (motivation[id].first.acts_when==nbply && motivation[id].first.purpose==purpose_interceptor) // revealed interceptor - not necessarly present
               || (motivation[id].first.acts_when<nbply && motivation[id].last.acts_when>=nbply) // in action
               || (motivation[id].last.purpose==purpose_none && motivation[id].last.acts_when<nbply) // put on hold by a revelation or capture
               || (GetPieceId(being_solved.spec[motivation[id].last.on])==id));
      }
    }

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
    {
      assert(first_taboo_violation==nullsquare);
      flesh_out_random_move_by_invisible();
    }
    else if (nbply<=flesh_out_move_highwater)
    {
      if (first_taboo_violation==nullsquare)
        adapt_pre_capture_effect();
      else
      {
        TraceText("can't resolve taboo violation\n");
        REPORT_DECISION_OUTCOME("%s","can't resolve taboo violation");
        REPORT_DEADEND;
      }
    }
    else if (sq_departure>=capture_by_invisible
             && is_on_board(sq_arrival))
      flesh_out_capture_by_invisible(first_taboo_violation);
    else if (first_taboo_violation==nullsquare)
      adapt_pre_capture_effect();
    // TODO review
//    else
//      assert(is_taboo_violation_acceptable(first_taboo_violation));
  }
  else
    validate_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                      unsigned int nr_check_vectors);

static void walk_interceptor_any_walk(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                      unsigned int nr_check_vectors,
                                      Side side,
                                      square pos,
                                      piece_walk_type walk,
                                      Flags spec)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceWalk(walk);
  TraceValue("%x",spec);
  TraceFunctionParamListEnd();

  ++being_solved.number_of_pieces[side][walk];

  TraceValue("%u",top_invisible_piece_id);TraceEOL();

  SetPieceId(spec,top_invisible_piece_id);
  occupy_square(pos,walk,spec);
  REPORT_DECISION_WALK('>',walk);
  ++curr_decision_level;

  {
    Side const side_attacked = advers(side);
    square const king_pos = being_solved.king_square[side_attacked];
    vec_index_type const k = is_square_uninterceptably_attacked(side_attacked,
                                                                king_pos);
    if (k==UINT_MAX)
    {
      // TODO accept uninterceptable check if not illegal
      REPORT_DECISION_OUTCOME("%s","interceptor delivers uninterceptable check - TODO: not necessarily a deadend");
      REPORT_DEADEND;
    }
    else if (k==0 || king_pos+vec[k]!=pos)
    {
      if (nr_check_vectors==1)
        restart_from_scratch();
      else
        place_interceptor_on_line(check_vectors,nr_check_vectors-1);
    }
    else
    {
      // TODO accept uninterceptable check if not illegal
      REPORT_DECISION_OUTCOME("%s","interceptor delivers uninterceptable check - TODO: not necessarily a deadend");
      REPORT_DEADEND;
    }
  }

  --curr_decision_level;

  TraceWalk(get_walk_of_piece_on_square(pos));
  TraceWalk(walk);
  TraceEOL();
  assert(get_walk_of_piece_on_square(pos)==walk);
  empty_square(pos);
  --being_solved.number_of_pieces[side][walk];

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_pawn(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                  unsigned int nr_check_vectors,
                                  Side side,
                                  square pos,
                                  Flags spec)
{
  SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceValue("%x",spec);
  TraceFunctionParamListEnd();

  if ((TSTFLAG(sq_spec[pos],basesq) || TSTFLAG(sq_spec[pos],promsq)))
  {
    REPORT_DECISION_OUTCOME("%s","pawn is placed on impossible square");
    REPORT_DEADEND;
  }
  else
    walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Pawn,spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_king(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                  unsigned int nr_check_vectors,
                                  Side side,
                                  square pos)
{
  dynamic_consumption_type const save_consumption = current_consumption;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  /* this removes the implicit allocation for the invisible king ...*/
  being_solved.king_square[side] = pos;

  /* ... and thus allows this to succeed: */
  if (allocate_flesh_out_unplaced(side))
  {
    Flags const spec = BIT(side)|BIT(Royal)|BIT(Chameleon);
    walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,King,spec);
  }

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

  being_solved.king_square[side] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                             unsigned int nr_check_vectors,
                             Side side,
                             square pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  TraceSquare(being_solved.king_square[side]);
  TraceEOL();
  assert(is_square_empty(pos));

  motivation[top_invisible_piece_id].levels.walk = curr_decision_level;

  if (being_solved.king_square[side]==initsquare)
    walk_interceptor_king(check_vectors,nr_check_vectors,side,pos);

  {
    dynamic_consumption_type const save_consumption = current_consumption;

    if (allocate_flesh_out_unplaced(side))
    {
      Flags const spec = BIT(side)|BIT(Chameleon);

      if (curr_decision_level<=max_decision_level)
      {
        max_decision_level = decision_level_latest;
        walk_interceptor_pawn(check_vectors,nr_check_vectors,side,pos,spec);
      }

      if (side==trait[nbply])
      {
        if (curr_decision_level<=max_decision_level)
        {
          max_decision_level = decision_level_latest;
          walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Knight,spec);
        }

        if (curr_decision_level<=max_decision_level)
        {
          vec_index_type const k = check_vectors[nr_check_vectors-1];
          boolean const is_check_orthogonal = k<=vec_rook_end;

          max_decision_level = decision_level_latest;

          if (is_check_orthogonal)
            walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Bishop,spec);
          else
            walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Rook,spec);
        }
      }
      else
      {
        piece_walk_type walk;
        for (walk = Queen;
             walk<=Bishop && curr_decision_level<=max_decision_level;
             ++walk)
        {
          max_decision_level = decision_level_latest;
          walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,walk,spec);
        }
      }
    }
    else
    {
      REPORT_DECISION_OUTCOME("%s","not enough available invisibles for intercepting all illegal checks");
      REPORT_DEADEND;
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_interceptor(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                               unsigned int nr_check_vectors,
                               square pos)
{
  Side const preferred_side = trait[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  if (curr_decision_level<=max_decision_level)
  {
    if (!is_taboo(pos,preferred_side))
    {
      max_decision_level = decision_level_latest;
      REPORT_DECISION_COLOUR('>',BIT(preferred_side));
      ++curr_decision_level;
      walk_interceptor(check_vectors,nr_check_vectors,preferred_side,pos);
      --curr_decision_level;
    }
  }

  if (curr_decision_level<=max_decision_level)
  {
    if (!is_taboo(pos,advers(preferred_side)))
    {
      max_decision_level = decision_level_latest;
      REPORT_DECISION_COLOUR('>',BIT(advers(preferred_side)));
      ++curr_decision_level;
      walk_interceptor(check_vectors,nr_check_vectors,advers(preferred_side),pos);
      --curr_decision_level;
    }
  }

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                                unsigned int nr_check_vectors,
                                                square s,
                                                Side side)
{
  dynamic_consumption_type const save_consumption = current_consumption;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  REPORT_DECISION_COLOUR('>',BIT(side));
  ++curr_decision_level;

  if (allocate_placed(side))
  {
    if (is_taboo(s,side))
    {
      REPORT_DECISION_OUTCOME("%s","taboo violation");
      REPORT_DEADEND;
    }
    else
    {
      TraceSquare(s);TraceEnumerator(Side,trait[nbply-1]);TraceEOL();

      CLRFLAG(being_solved.spec[s],advers(side));

      if (nr_check_vectors==1)
        done_intercepting_illegal_checks();
      else
        place_interceptor_on_line(check_vectors,nr_check_vectors-1);

      SETFLAG(being_solved.spec[s],advers(side));
    }
  }
  else
  {
    REPORT_DECISION_OUTCOME("%s","not enough available invisibles for intercepting all illegal checks");
    REPORT_DEADEND;
  }

  --curr_decision_level;

  current_consumption = save_consumption;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                        unsigned int nr_check_vectors,
                                        square s)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  motivation[top_invisible_piece_id].first.on = s;
  motivation[top_invisible_piece_id].last.on = s;

  motivation[top_invisible_piece_id].levels.from = decision_level_latest;
  motivation[top_invisible_piece_id].levels.side = curr_decision_level;

  if (play_phase==play_validating_mate)
  {
    Flags spec = BIT(White)|BIT(Black)|BIT(Chameleon);

    SetPieceId(spec,top_invisible_piece_id);
    occupy_square(s,Dummy,spec);

    motivation[top_invisible_piece_id].levels.walk = decision_level_latest;

    place_interceptor_of_side_on_square(check_vectors,nr_check_vectors,s,White);

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      place_interceptor_of_side_on_square(check_vectors,nr_check_vectors,s,Black);
    }

    TraceConsumption();TraceEOL();

    empty_square(s);
  }
  else
    colour_interceptor(check_vectors,nr_check_vectors,s);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                      unsigned int nr_check_vectors)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];
  vec_index_type const kcurr = check_vectors[nr_check_vectors-1];
  int const dir = vec[kcurr];
  REPORT_DECISION_DECLARE(unsigned int const save_counter = report_decision_counter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  ++top_invisible_piece_id;

  TraceValue("%u",top_invisible_piece_id);
  TraceValue("%u",motivation[top_invisible_piece_id].last.purpose);
  TraceEOL();
  assert(motivation[top_invisible_piece_id].last.purpose==purpose_none);
  motivation[top_invisible_piece_id].first.purpose = purpose_interceptor;
  motivation[top_invisible_piece_id].first.acts_when = nbply;
  motivation[top_invisible_piece_id].last.purpose = purpose_interceptor;
  motivation[top_invisible_piece_id].last.acts_when = nbply;
  motivation[top_invisible_piece_id].levels.to = curr_decision_level;

  {
    square s;
    for (s = king_pos+dir;
         is_square_empty(s) && curr_decision_level<=max_decision_level;
         s += dir)
    {
      max_decision_level = decision_level_latest;

      /* use the taboo machinery to avoid attempting to intercept on the same
       * square in different iterations.
       * nbply minus 1 is correct - this taboo is equivalent to those deduced from
       * the previous move.
       */
      if (nr_taboos_for_current_move_in_ply[nbply-1][White][s]==0
          && nr_taboos_for_current_move_in_ply[nbply-1][Black][s]==0)
      {
        TraceSquare(s);
        TraceValue("%u",nr_taboos_accumulated_until_ply[White][s]);
        TraceValue("%u",nr_taboos_accumulated_until_ply[Black][s]);
        TraceEOL();
        if (!was_taboo(s))
        {
          // TODO if REPORT_DECISION_SQUARE() faked level 1 less, we could adjust
          // curr_decision_level outside of the loop
          REPORT_DECISION_SQUARE('>',s);
          ++curr_decision_level;
          place_interceptor_on_square(check_vectors,nr_check_vectors,s);
          --curr_decision_level;
        }
      }
      ++nr_taboos_for_current_move_in_ply[nbply-1][White][s];
      ++nr_taboos_for_current_move_in_ply[nbply-1][Black][s];
    }
    {
      square s2;
      for (s2 = king_pos+vec[kcurr]; s2!=s; s2 += vec[kcurr])
      {
        --nr_taboos_for_current_move_in_ply[nbply-1][White][s2];
        --nr_taboos_for_current_move_in_ply[nbply-1][Black][s2];
      }
    }

    TraceSquare(s);TraceEOL();

    motivation[top_invisible_piece_id] = motivation_null;
    --top_invisible_piece_id;

#if defined(REPORT_DECISIONS)
    if (report_decision_counter==save_counter)
    {
      REPORT_DECISION_DECLARE(PieceIdType const id_checker = GetPieceId(being_solved.spec[s]));
      REPORT_DECISION_DECLARE(ply const ply_check = motivation[id_checker].last.acts_when);
      REPORT_DECISION_OUTCOME("no available square found where to intercept check"
                              " from dir:%d"
                              " by id:%u"
                              " in ply:%u",
                              dir,
                              id_checker,
                              ply_check);
      REPORT_DEADEND;
    }
#endif
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_illegal_checks(void)
{
  vec_index_type check_vectors[vec_queen_end-vec_queen_start+1];
  unsigned int nr_check_vectors = 0;
  vec_index_type kcurr;
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];
  unsigned int const nr_available = nr_placeable_invisibles_for_both_sides();

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (king_pos==initsquare)
    done_intercepting_illegal_checks();
  else
  {
    vec_index_type const k = is_square_attacked_by_uninterceptable(side_in_check,king_pos);
    if (k!=0)
    {
      int const dir_check = vec[k];
      Flags checkerSpec;

      uninterceptable_check_delivered_from = king_pos+dir_check;
      checkerSpec = being_solved.spec[uninterceptable_check_delivered_from];
      if (TSTFLAG(checkerSpec,Chameleon))
      {
        PieceIdType const id_checker = GetPieceId(checkerSpec);
        assert(uninterceptable_check_delivered_in_ply==ply_nil);
        uninterceptable_check_delivered_in_ply = motivation[id_checker].last.acts_when;

        REPORT_DECISION_OUTCOME("uninterceptable illegal check"
                                " from dir:%d"
                                " by id:%u"
                                " delivered in ply:%u",
                                dir_check,
                                id_checker,
                                uninterceptable_check_delivered_in_ply);

        TraceValue("%u",nbply);TraceEOL();
        if (nbply==ply_retro_move+1)
        {
          REPORT_DEADEND;
        }
        else
          restart_from_scratch();

        uninterceptable_check_delivered_in_ply = ply_nil;
      }
      else
      {
        REPORT_DECISION_OUTCOME("%s","uninterceptable check by visible piece");
        REPORT_DEADEND;
      }

      uninterceptable_check_delivered_from = initsquare;
    }
    else
    {
      /* there are only interceptable checks - deal with them */
      for (kcurr = vec_rook_start;
           kcurr<=vec_rook_end && nr_available>=nr_check_vectors;
           ++kcurr)
      {
        int const dir = vec[kcurr];
        square const sq_end = find_end_of_line(king_pos,dir);
        piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

        TraceValue("%d",dir);
        TraceSquare(sq_end);
        TraceWalk(walk_at_end);
        TraceEOL();

        if (TSTFLAG(being_solved.spec[sq_end],side_checking)
            && (walk_at_end==Queen || walk_at_end==Rook))
          check_vectors[nr_check_vectors++] = kcurr;
      }

      for (kcurr = vec_bishop_start;
           kcurr<=vec_bishop_end && nr_available>=nr_check_vectors;
           ++kcurr)
      {
        int const dir = vec[kcurr];
        square const sq_end = find_end_of_line(king_pos,dir);
        piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

        TraceValue("%d",dir);
        TraceSquare(sq_end);
        TraceWalk(walk_at_end);
        TraceEOL();

        if (TSTFLAG(being_solved.spec[sq_end],side_checking)
            && (walk_at_end==Queen || walk_at_end==Bishop))
          check_vectors[nr_check_vectors++] = kcurr;
      }

      TraceValue("%u",nr_available);
      TraceValue("%u",nr_check_vectors);
      TraceEOL();

      if (nr_check_vectors==0)
        done_intercepting_illegal_checks();
      else if (nr_available>=nr_check_vectors)
        place_interceptor_on_line(check_vectors,nr_check_vectors);
      else
      {
        TraceText("not enough available invisibles for intercepting all illegal checks\n");
        REPORT_DECISION_OUTCOME("%s","not enough available invisibles for intercepting all illegal checks");
        REPORT_DEADEND;
      }
    }
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

static void rewind_effects(void)
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

static void unrewind_effects(void)
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

static void apply_knowledge(knowledge_index_type idx_knowledge,
                            void (*next_step)(void));

static void apply_royal_knowledge(knowledge_index_type idx_knowledge,
                                  void (*next_step)(void))
{
  square const s = knowledge[idx_knowledge].first_on;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx_knowledge);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[s],Royal))
  {
    Side const side = TSTFLAG(knowledge[idx_knowledge].spec,White) ? White : Black;
    TraceSquare(being_solved.king_square[side]);
    TraceWalk(get_walk_of_piece_on_square(being_solved.king_square[side]));
    TraceEOL();
    assert(being_solved.king_square[side]==initsquare);
    being_solved.king_square[side] = s;
    apply_knowledge(idx_knowledge+1,next_step);
    being_solved.king_square[side] = initsquare;
  }
  else
    apply_knowledge(idx_knowledge+1,next_step);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void apply_knowledge(knowledge_index_type idx_knowledge,
                            void (*next_step)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx_knowledge);
  TraceFunctionParamListEnd();

  TraceValue("%u",size_knowledge);TraceEOL();

  if (idx_knowledge==size_knowledge)
    (*next_step)();
  else
  {
    dynamic_consumption_type const save_consumption = current_consumption;
    square const sq_first_on = knowledge[idx_knowledge].first_on;
    Side const side = TSTFLAG(knowledge[idx_knowledge].spec,White) ? White : Black;

    TraceSquare(sq_first_on);
    TraceSquare(knowledge[idx_knowledge].revealed_on);
    TraceSquare(knowledge[idx_knowledge].first_on);
    TraceWalk(knowledge[idx_knowledge].walk);
    TraceValue("%x",knowledge[idx_knowledge].spec);
    TraceEnumerator(Side,side);
    TraceValue("%u",knowledge[idx_knowledge].last.acts_when);
    TraceValue("%u",knowledge[idx_knowledge].last.purpose);
    TraceSquare(knowledge[idx_knowledge].last.on);
    TraceEOL();

    if ((knowledge[idx_knowledge].is_allocated
         || allocate_placed(side))
        && !is_taboo(sq_first_on,side))
    {
      ++being_solved.number_of_pieces[side][knowledge[idx_knowledge].walk];
      occupy_square(knowledge[idx_knowledge].first_on,
                    knowledge[idx_knowledge].walk,
                    knowledge[idx_knowledge].spec);

      TraceValue("%u",knowledge[idx_knowledge].last.acts_when);TraceEOL();
      if (knowledge[idx_knowledge].last.acts_when!=ply_nil)
      {
        PieceIdType const id_on_board = GetPieceId(being_solved.spec[knowledge[idx_knowledge].last.on]);
        motivation_type const save_motivation = motivation[id_on_board];
        move_effect_journal_index_type const effects_base = move_effect_journal_base[knowledge[idx_knowledge].last.acts_when];
        move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

        TraceWalk(move_effect_journal[movement].u.piece_movement.moving);
        TraceValue("%x",move_effect_journal[movement].u.piece_movement.movingspec);
        TraceSquare(move_effect_journal[movement].u.piece_movement.from);
        TraceSquare(move_effect_journal[movement].u.piece_movement.to);
        TraceEOL();

        assert(move_effect_journal[movement].type==move_effect_piece_movement);

        if (knowledge[idx_knowledge].last.purpose==purpose_capturer)
        {
          /* applying this knowledge doesn't work if the revealed piece has
           * moved before this capturing move */
          PieceIdType const id_from_knowledge = GetPieceId(knowledge[idx_knowledge].spec);
          if (id_on_board!=id_from_knowledge)
            apply_knowledge(idx_knowledge+1,next_step);
          else
          {
            ply const save_nbply = nbply;
            move_effect_journal_index_type const precapture = effects_base;
            move_effect_journal_entry_type const save_movement = move_effect_journal[movement];

            motivation[id_on_board].first = knowledge[idx_knowledge].last;
            motivation[id_on_board].first.on = sq_first_on;
            motivation[id_on_board].first.acts_when = 0;
            motivation[id_on_board].last = knowledge[idx_knowledge].last;

            TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

            assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
            assert(move_effect_journal[movement].u.piece_movement.to==knowledge[idx_knowledge].revealed_on);
            assert(move_effect_journal[movement].u.piece_movement.from==capture_by_invisible);

            TraceText("prevent searching for capturer - we know who did it\n");

            move_effect_journal[precapture].type = move_effect_none;
            move_effect_journal[movement].u.piece_movement.from = knowledge[idx_knowledge].last.on;
            move_effect_journal[movement].u.piece_movement.moving = get_walk_of_piece_on_square(knowledge[idx_knowledge].last.on);
            move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_first_on];

            nbply = knowledge[idx_knowledge].last.acts_when;
            update_nr_taboos_for_current_move_in_ply(+1);
            nbply = save_nbply;

            apply_royal_knowledge(idx_knowledge,next_step);

            nbply = knowledge[idx_knowledge].last.acts_when;
            update_nr_taboos_for_current_move_in_ply(-1);
            nbply = save_nbply;

            move_effect_journal[movement] = save_movement;
            move_effect_journal[precapture].type = move_effect_piece_readdition;
          }
        }
        else if (knowledge[idx_knowledge].last.purpose==purpose_castling_partner)
        {
          motivation[id_on_board].first = knowledge[idx_knowledge].last;
          motivation[id_on_board].first.on = sq_first_on;
          motivation[id_on_board].first.acts_when = 0;
          motivation[id_on_board].last = knowledge[idx_knowledge].last;

          TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

          assert(move_effect_journal[movement].u.piece_movement.moving==King);
          assert(is_on_board(move_effect_journal[movement].u.piece_movement.from));
          assert(is_on_board(move_effect_journal[movement].u.piece_movement.to));
          apply_royal_knowledge(idx_knowledge,next_step);
        }
        else if (knowledge[idx_knowledge].last.purpose==purpose_random_mover)
        {
          motivation[id_on_board].first = knowledge[idx_knowledge].last;
          motivation[id_on_board].first.on = sq_first_on;
          motivation[id_on_board].first.acts_when = 0;
          motivation[id_on_board].last = knowledge[idx_knowledge].last;

          TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

          // TODO can we restrict generation of random move to the revealed piece?
          assert(move_effect_journal[movement].u.piece_movement.moving==Empty);
          assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
          assert(move_effect_journal[movement].u.piece_movement.to==move_by_invisible);
          apply_royal_knowledge(idx_knowledge,next_step);
        }
        else if (knowledge[idx_knowledge].last.purpose==purpose_interceptor)
        {
          motivation[id_on_board].first = knowledge[idx_knowledge].last;
          motivation[id_on_board].first.on = sq_first_on;
          motivation[id_on_board].first.acts_when = 0;
          motivation[id_on_board].last = knowledge[idx_knowledge].last;

          TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

          // We no longer look for different insertion squares for pieces
          // revealed after having been inserted to move to intercept an
          // illegal check.
          // This seems to have been useless knowledge anyway...
//          assert(sq_first_on==knowledge[idx_knowledge].revealed_on);
          apply_royal_knowledge(idx_knowledge,next_step);
        }
        else
          assert(0);

        motivation[id_on_board] = save_motivation;
      }
      else
        apply_royal_knowledge(idx_knowledge,next_step);

      assert(sq_first_on==knowledge[idx_knowledge].first_on);
      empty_square(knowledge[idx_knowledge].first_on);
      --being_solved.number_of_pieces[side][knowledge[idx_knowledge].walk];
    }
    else
    {
      combined_result = previous_move_has_not_solved;
      TraceText("allocation for application of knowledge not possible\n");
      REPORT_DECISION_OUTCOME("%s","allocation for application of knowledge not possible");
      REPORT_DEADEND;
    }

    current_consumption = save_consumption;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void make_revelations(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  top_ply_of_regular_play = nbply;
  setup_revelations();
  play_phase = play_rewinding;
  rewind_effects();
  play_phase = play_detecting_revelations;
  max_decision_level = decision_level_latest;
  REPORT_DECISION_CONTEXT(__func__);

  static_consumption.king[White] = being_solved.king_square[White]==initsquare;
  static_consumption.king[Black] = being_solved.king_square[Black]==initsquare;

  apply_knowledge(0,&start_iteration);

  static_consumption.king[White] = false;
  static_consumption.king[Black] = false;

  play_phase = play_unwinding;
  unrewind_effects();
  play_phase = play_regular;

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
void total_invisible_reveal_after_mating_move(slice_index si)
{
  dynamic_consumption_type const save_consumption = current_consumption;
  PieceIdType const save_next_invisible_piece_id = top_invisible_piece_id;
  knowledge_index_type const save_size_knowledge = size_knowledge;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  update_taboo(+1);
  update_nr_taboos_for_current_move_in_ply(+1);
  make_revelations();
  update_nr_taboos_for_current_move_in_ply(-1);
  update_taboo(-1);

  TraceValue("%u",top_invisible_piece_id);TraceEOL();

  if (!revelation_status_is_uninitialised)
    evaluate_revelations();

  pipe_solve_delegate(si);

  size_knowledge = save_size_knowledge;
  current_consumption = save_consumption;

  TraceValue("%u",top_invisible_piece_id);TraceEOL();

  assert(top_invisible_piece_id>=save_next_invisible_piece_id);
  while (top_invisible_piece_id>save_next_invisible_piece_id)
  {
    motivation[top_invisible_piece_id] = motivation_null;
    --top_invisible_piece_id;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_move_still_playable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side = SLICE_STARTER(si);
    move_effect_journal_index_type const base = move_effect_journal_base[ply_replayed];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture;

    TraceValue("%u",ply_replayed);
    TraceSquare(sq_departure);
    TraceSquare(sq_arrival);
    TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture);
    TraceEOL();

    // TODO redo (and undo) the pre-capture effect?

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
    {
      TraceText("we assume that un-fleshed-out random moves by total invisibles are always playable\n");
      result = true;
    }
    else
    {
      assert(TSTFLAG(being_solved.spec[sq_departure],side));

      if (sq_capture==queenside_castling)
      {
        SETCASTLINGFLAGMASK(side,ra_cancastle);
        generate_moves_for_piece(sq_departure);
        CLRCASTLINGFLAGMASK(side,ra_cancastle);
      }
      else if (sq_capture==kingside_castling)
      {
        SETCASTLINGFLAGMASK(side,rh_cancastle);
        generate_moves_for_piece(sq_departure);
        CLRCASTLINGFLAGMASK(side,rh_cancastle);
      }
      else
        generate_moves_for_piece(sq_departure);

      {
        numecoup start = MOVEBASE_OF_PLY(nbply);
        numecoup i;
        numecoup new_top = start;
        for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
        {
          assert(move_generation_stack[i].departure==sq_departure);
          if (move_generation_stack[i].arrival==sq_arrival)
          {
            ++new_top;
            move_generation_stack[new_top] = move_generation_stack[i];
            break;
          }
        }

        SET_CURRMOVE(nbply,new_top);
      }

      result = CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void copy_move_effects(void)
{
  move_effect_journal_index_type const replayed_base = move_effect_journal_base[ply_replayed];
  move_effect_journal_index_type const replayed_top = move_effect_journal_base[ply_replayed+1];
  move_effect_journal_index_type replayed_curr;
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_replayed);
  TraceValue("%u",move_effect_journal_base[ply_replayed]);
  TraceValue("%u",move_effect_journal_base[ply_replayed+1]);
  TraceValue("%u",nbply);
  TraceEOL();

  move_effect_journal_base[nbply+1] += (replayed_top-replayed_base);
  curr = move_effect_journal_base[nbply+1];
  replayed_curr = replayed_top;

  while (replayed_curr>replayed_base)
    move_effect_journal[--curr] = move_effect_journal[--replayed_curr];

  assert(curr==move_effect_journal_base[nbply]);

  TraceValue("%u",move_effect_journal_base[nbply]);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

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
void total_invisible_move_repeater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));

  if (!(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture==kingside_castling
        || move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture==queenside_castling)
      || is_move_still_playable(si))
  {
    copy_move_effects();
    redo_move_effects();
    ++ply_replayed;
    pipe_solve_delegate(si);
    --ply_replayed;
    undo_move_effects();
  }
  else
    solve_result = previous_move_is_illegal;

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
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO separate slice type for update nr_taboos_accumulated_until_ply?
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else if (nbply>ply_retro_move)
  {
    update_taboo(+1);
    update_nr_taboos_for_current_move_in_ply(+1);

    top_before_relevations[nbply] = move_effect_journal_base[nbply+1];

    make_revelations();

    if (revelation_status_is_uninitialised)
      pipe_solve_delegate(si);
    else
    {
      unsigned int nr_revealed_unplaced_invisibles[nr_sides] = { 0, 0 };
      {
        unsigned int i;
        for (i = 0; i!=nr_potential_revelations; ++i)
        {
          square const s = revelation_status[i].first_on;
          TraceValue("%u",i);
          TraceSquare(s);
          TraceWalk(revelation_status[i].walk);
          TraceValue("%x",revelation_status[i].spec);
          TraceEOL();
          if (revelation_status[i].walk!=Empty && is_square_empty(s))
          {
            Side const side = TSTFLAG(revelation_status[i].spec,White) ? White : Black;
            ++nr_revealed_unplaced_invisibles[side];
          }
        }
      }
      // TODO there must be a simpler, understandable way to do this
      if (current_consumption.claimed[White] && nr_revealed_unplaced_invisibles[White]>0)
        --nr_revealed_unplaced_invisibles[White];
      if (current_consumption.claimed[Black] && nr_revealed_unplaced_invisibles[Black]>0)
        --nr_revealed_unplaced_invisibles[Black];
      if ((current_consumption.fleshed_out[White]
           +current_consumption.fleshed_out[Black]
           +current_consumption.placed[White]
           +current_consumption.placed[Black]
           +nr_revealed_unplaced_invisibles[White]
           +nr_revealed_unplaced_invisibles[Black])
          <=total_invisible_number)
      {
        knowledge_index_type const save_size_knowledge = size_knowledge;
        PieceIdType const save_next_invisible_piece_id = top_invisible_piece_id;

        evaluate_revelations();
        pipe_solve_delegate(si);
        TraceConsumption();TraceEOL();

        assert(top_invisible_piece_id>=save_next_invisible_piece_id);
        while (top_invisible_piece_id>save_next_invisible_piece_id)
        {
          motivation[top_invisible_piece_id] = motivation_null;
          --top_invisible_piece_id;
        }

        size_knowledge = save_size_knowledge;
      }
    }

    update_nr_taboos_for_current_move_in_ply(-1);
    update_taboo(-1);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int find_nr_interceptors_needed(Side side_checking,
                                                square potential_flight,
                                                unsigned int nr_interceptors_available,
                                                vec_index_type start, vec_index_type end,
                                                piece_walk_type walk_rider)
{
  unsigned int result = 0;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(potential_flight);
  TraceValue("%u",nr_placeable_invisibles_for_side);
  TraceValue("%u",start);
  TraceValue("%u",end);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  for (k = start; k<=end && result<=nr_interceptors_available; ++k)
  {
    square const end = find_end_of_line(potential_flight,vec[k]);
    piece_walk_type const walk = get_walk_of_piece_on_square(end);
    Flags const flags = being_solved.spec[end];
    if ((walk==Queen || walk==walk_rider) && TSTFLAG(flags,side_checking))
    {
      square s;
      for (s = potential_flight+vec[k]; s!=end; s += vec[k])
        if (nr_taboos_accumulated_until_ply[White][s]==0 || nr_taboos_accumulated_until_ply[Black][s]==0)
        {
          ++result;
          break;
        }

      if (s==end)
      {
        /* line can't be intercepted - all guard are not interceptable */
        result = nr_interceptors_available+1;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_all_guards_interceptable(Side side_in_check, square potential_flight)
{
  Side const side_checking = advers(side_in_check);
  unsigned int nr_available;
  unsigned int nr_interceptors_needed;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(potential_flight);
  TraceFunctionParamListEnd();

  nr_available = nr_placeable_invisibles_for_side(side_in_check);
  TraceConsumption();
  TraceValue("%u",nr_available);
  TraceEOL();

  nr_interceptors_needed = find_nr_interceptors_needed(side_checking,
                                                       potential_flight,
                                                       nr_available,
                                                       vec_rook_start,vec_rook_end,
                                                       Rook);

  if (nr_interceptors_needed<=nr_available)
    nr_interceptors_needed += find_nr_interceptors_needed(side_checking,
                                                          potential_flight,
                                                          nr_available-nr_interceptors_needed,
                                                          vec_bishop_start,vec_bishop_end,
                                                          Bishop);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",(nr_interceptors_needed<=nr_available));
  TraceFunctionResultEnd();
  return nr_interceptors_needed<=nr_available;
}

static boolean make_flight(Side side_in_check, square s)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if ((is_square_empty(s)
      || TSTFLAG(being_solved.spec[s],advers(side_in_check)))
      && !is_square_uninterceptably_attacked(side_in_check,s)
      && are_all_guards_interceptable(side_in_check,s))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_a_flight(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_in_check = advers(trait[nbply]);
    square const king_pos = being_solved.king_square[side_in_check];
    int dir_vert;
    int dir_horiz;

    piece_walk_type const walk = get_walk_of_piece_on_square(king_pos);
    Flags const flags = being_solved.spec[king_pos];

    TraceWalk(walk);
    TraceValue("%x",flags);
    TraceEOL();

    empty_square(king_pos);

    for (dir_vert = dir_down; dir_vert<=dir_up && !result; dir_vert += dir_up)
      for (dir_horiz = dir_left; dir_horiz<=dir_right; dir_horiz += dir_right)
      {
        square const flight = king_pos+dir_vert+dir_horiz;
        if (flight!=king_pos && make_flight(side_in_check,flight))
        {
          result = true;
          break;
        }
      }

    occupy_square(king_pos,walk,flags);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void attack_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_delivering_check = trait[nbply];
    Side const side_in_check = advers(side_delivering_check);
    square const king_pos = being_solved.king_square[side_in_check];
    vec_index_type const k = is_square_uninterceptably_attacked(side_in_check,king_pos);
    if (k==0)
    {
      if (are_all_guards_interceptable(side_in_check,king_pos))
      {
        TraceText("interceptable check\n");
        /* the king square can be made a "flight" */
        mate_validation_result = no_mate;
        solve_result = previous_move_has_not_solved;
      }
      else
      {
        TraceText("mate can be refuted by interceptors or free TIs (if any)\n");
        mate_validation_result = mate_defendable_by_interceptors;
      }
    }
    else if (k==UINT_MAX)
    {
      TraceText("mate can be refuted by interceptors - no free TI avaliable\n");
      mate_validation_result = mate_defendable_by_interceptors;
    }
    else if (nr_placeable_invisibles_for_side(side_in_check)>0)
    {
      square const sq_attacker = find_end_of_line(king_pos,vec[k]);
      TraceSquare(king_pos);TraceValue("%u",k);TraceValue("%d",vec[k]);TraceSquare(sq_attacker);TraceEOL();
      assert(TSTFLAG(being_solved.spec[sq_attacker],side_delivering_check));
      CLRFLAG(being_solved.spec[sq_attacker],side_delivering_check);
      if (is_square_uninterceptably_attacked(side_in_check,king_pos))
      {
        TraceText("mate can not be defended\n");
        mate_validation_result = mate_with_2_uninterceptable_doublechecks;
      }
      else
      {
        TraceText("mate can be refuted by free TIs\n");
        mate_validation_result = mate_attackable;
        sq_mating_piece_to_be_attacked = sq_attacker;
      }
      SETFLAG(being_solved.spec[sq_attacker],side_delivering_check);
    }
    else
    {
      TraceText("mate can be refuted by interceptors - no free TI avaliable\n");
      mate_validation_result = mate_defendable_by_interceptors;
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
void total_invisible_goal_guard_solve(slice_index si)
{
  play_phase_type const save_play_phase = play_phase;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* make sure that we don't generate pawn captures total invisible */
  assert(play_phase==play_replay_validating || play_phase==play_replay_testing);

  play_phase = play_testing_goal;
  pipe_solve_delegate(si);
  play_phase = save_play_phase;

  if (play_phase==play_replay_validating)
  {
    if (solve_result==previous_move_has_not_solved)
      mate_validation_result = no_mate;
    else if (make_a_flight())
    {
      solve_result = previous_move_has_not_solved;
      mate_validation_result = no_mate;
    }
    else
      attack_checks();
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
void total_invisible_generate_moves_by_invisible(slice_index si)
{
  Side const side_moving = trait[nbply];
  Side const side_capturee = advers(side_moving);
  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  curr_generation->departure = capture_by_invisible;

  // TODO refuse capturing a rook that has just castled
  for (s = boardnum; *s; ++s)
  {
    if (!is_square_empty(*s))
    {
      TraceWalk(get_walk_of_piece_on_square(*s));
      TraceValue("%x",being_solved.spec[*s]);
      TraceEOL();
    }
    if (TSTFLAG(being_solved.spec[*s],side_capturee)
        && get_walk_of_piece_on_square(*s)!=Dummy
        && !TSTFLAG(being_solved.spec[*s],Chameleon)
        && !TSTFLAG(being_solved.spec[*s],Royal))
    {
      curr_generation->arrival = *s;
      push_move_regular_capture();
    }
  }

  curr_generation->departure = move_by_invisible;
  curr_generation->arrival = move_by_invisible;
  push_move_capture_extra(move_by_invisible);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_right(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO what kind of nr_taboos_accumulated_until_ply can we apply here?
  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    pipe_move_generation_delegate(si);
    empty_square(s);
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_left(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_left;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO what kind of nr_taboos_accumulated_until_ply can we apply here?
  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    generate_pawn_capture_right(si,dir_vertical);
    empty_square(s);
  }
  else
    generate_pawn_capture_right(si,dir_vertical);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_king_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;
  square const square_h = square_a+file_h;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if ((nr_total_invisbles_consumed()<total_invisible_number)
      && is_square_empty(square_h)
      && nr_taboos_accumulated_until_ply[side][square_h]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_h,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,rh_cancastle);
    pipe_move_generation_delegate(si);
    CLRCASTLINGFLAGMASK(side,rh_cancastle);
    empty_square(square_h);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_queen_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if ((nr_total_invisbles_consumed()<total_invisible_number)
      && is_square_empty(square_a)
      && nr_taboos_accumulated_until_ply[side][square_a]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_a,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,ra_cancastle);
    prepare_king_side_castling_generation(si);
    CLRCASTLINGFLAGMASK(side,ra_cancastle);
    empty_square(square_a);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    prepare_king_side_castling_generation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void total_invisible_generate_special_moves(slice_index si)
{
  square const sq_departure = curr_generation->departure ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (play_phase==play_regular)
    switch (being_solved.board[sq_departure])
    {
      case Pawn:
      {
        Side const side_playing = trait[nbply];
        Side const side_victim = advers(side_playing);
        if (nr_placeable_invisibles_for_side(side_victim)>0)
          generate_pawn_capture_left(si,side_playing==White ? dir_up : dir_down);
        else
          pipe_move_generation_delegate(si);
        break;
      }

      case King:
        if (TSTCASTLINGFLAGMASK(trait[nbply],castlings)>=k_cancastle)
          prepare_queen_side_castling_generation(si);
        else
          pipe_move_generation_delegate(si);
        break;

      default:
        pipe_move_generation_delegate(si);
        break;
    }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void play_castling_with_invisible_partner(slice_index si,
                                                 square sq_departure_partner,
                                                 square sq_arrival_partner)
{
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure_partner);
  TraceSquare(sq_arrival_partner);
  TraceFunctionParamListEnd();

  if (is_square_empty(sq_departure_partner))
  {
    dynamic_consumption_type const save_consumption = current_consumption;
    if (allocate_flesh_out_unclaimed(trait[nbply]))
    {
      Flags spec = BIT(side)|BIT(Chameleon);
      SetPieceId(spec,++top_invisible_piece_id);
      move_effect_journal_do_piece_readdition(move_effect_reason_castling_partner,
                                              sq_departure_partner,Rook,spec,side);
      assert(motivation[top_invisible_piece_id].last.purpose==purpose_none);
      motivation[top_invisible_piece_id].first.purpose = purpose_castling_partner;
      motivation[top_invisible_piece_id].first.acts_when = nbply;
      motivation[top_invisible_piece_id].first.on = sq_departure_partner;
      motivation[top_invisible_piece_id].last.purpose = purpose_castling_partner;
      motivation[top_invisible_piece_id].last.acts_when = nbply;
      motivation[top_invisible_piece_id].last.on = sq_arrival_partner;
      pipe_solve_delegate(si);
      motivation[top_invisible_piece_id] = motivation_null;
      --top_invisible_piece_id;
    }
    else
    {
      /* we have checked acts_when we generated the castling */
      assert(0);
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }
  else
  {
    move_effect_journal_do_null_effect();
    pipe_solve_delegate(si);
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
void total_invisible_special_moves_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;
    square const sq_capture = move_gen_top->capture;

    TraceSquare(sq_departure);
    TraceSquare(move_gen_top->arrival);
    TraceSquare(sq_capture);
    TraceEOL();

    if (sq_departure==move_by_invisible)
    {
      Side const side = trait[nbply];
      boolean const save_move_after_victim = static_consumption.move_after_victing[side];

      static_consumption.move_after_victing[side] = true;
      if (nr_total_invisbles_consumed()<=total_invisible_number)
        pipe_solve_delegate(si);
      else
        solve_result = this_move_is_illegal;
      static_consumption.move_after_victing[side] = save_move_after_victim;
    }
    else if (sq_departure>=capture_by_invisible)
    {
      Side const side = trait[nbply];
      Flags spec = BIT(side)|BIT(Chameleon);
      boolean const save_move_after_victim = static_consumption.move_after_victing[side];

      ++top_invisible_piece_id;
      SetPieceId(spec,top_invisible_piece_id);
      TraceValue("%u",top_invisible_piece_id);TraceEOL();
      assert(motivation[top_invisible_piece_id].last.purpose==purpose_none);
      motivation[top_invisible_piece_id].first.purpose = purpose_capturer;
      motivation[top_invisible_piece_id].first.acts_when = nbply;
      motivation[top_invisible_piece_id].first.on = sq_departure;
      motivation[top_invisible_piece_id].last.purpose = purpose_capturer;
      motivation[top_invisible_piece_id].last.acts_when = nbply;
      motivation[top_invisible_piece_id].last.on = sq_departure;
      motivation[top_invisible_piece_id].levels.walk = decision_level_latest;
      motivation[top_invisible_piece_id].levels.from = decision_level_latest;
      move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                              sq_departure,Dummy,spec,side);

      static_consumption.move_after_victing[side] = true;
      if (nr_total_invisbles_consumed()<=total_invisible_number)
      {
        /* No adjustment of current_consumption.placed here!
         * The capture may be done by an existing invisible. We can only do the
         * adjustment when we flesh out this capture by inserting a new invisible.
         */
        pipe_solve_delegate(si);
      }
      else
        solve_result = this_move_is_illegal;

      static_consumption.move_after_victing[side] = save_move_after_victim;

      motivation[top_invisible_piece_id] = motivation_null;
      --top_invisible_piece_id;
    }
    else
      switch (sq_capture)
      {
        case pawn_multistep:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case messigny_exchange:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case kingside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_h = square_a+file_h;
          square const square_f = square_a+file_f;

          TraceText("kingside_castling\n");
          play_castling_with_invisible_partner(si,square_h,square_f);
          break;
        }

        case queenside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_d = square_a+file_d;

          TraceText("queenside_castling\n");
          play_castling_with_invisible_partner(si,square_a,square_d);
          break;
        }

        case no_capture:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        default:
          /* pawn captures total invisible? */
          if (is_square_empty(sq_capture))
          {
            Side const side_victim = advers(SLICE_STARTER(si));
            Flags spec = BIT(side_victim)|BIT(Chameleon);
            boolean const save_move_after_victim = static_consumption.move_after_victing[side_victim];

            ++top_invisible_piece_id;
            SetPieceId(spec,top_invisible_piece_id);
            assert(motivation[top_invisible_piece_id].last.purpose==purpose_none);
            motivation[top_invisible_piece_id].first.purpose = purpose_victim;
            motivation[top_invisible_piece_id].first.acts_when = nbply;
            motivation[top_invisible_piece_id].first.on = sq_capture;
            motivation[top_invisible_piece_id].last.purpose = purpose_victim;
            motivation[top_invisible_piece_id].last.acts_when = nbply;
            motivation[top_invisible_piece_id].last.on = sq_capture;
            motivation[top_invisible_piece_id].levels.walk = decision_level_latest;
            motivation[top_invisible_piece_id].levels.from = decision_level_latest;
            move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                                    sq_capture,Dummy,spec,side_victim);

            /* No adjustment of current_consumption.placed here! Another invisible may
             * have moved to sq_capture and serve as a victim.
             */

            ++static_consumption.pawn_victims[side_victim];
            static_consumption.move_after_victing[side_victim] = false;

            if (nr_total_invisbles_consumed()<=total_invisible_number)
              pipe_solve_delegate(si);
            else
              solve_result = this_move_is_illegal;

            static_consumption.move_after_victing[side_victim] = save_move_after_victim;
            --static_consumption.pawn_victims[side_victim];

            motivation[top_invisible_piece_id] = motivation_null;
            --top_invisible_piece_id;
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
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

  top_visible_piece_id = being_solved.currPieceId;
  top_invisible_piece_id = top_visible_piece_id;

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
