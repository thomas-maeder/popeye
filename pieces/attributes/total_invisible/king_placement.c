#include "pieces/attributes/total_invisible/king_placement.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/attack_mating_piece.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/replay_fleshed_out.h"
#include "pieces/attributes/total_invisible.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

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
      record_decision_outcome("%s","Replaying moves for validation");
      play_phase = play_initialising_replay;
      replay_fleshed_out_move_sequence(play_replay_validating);
      play_phase = play_validating_mate;

      if (mate_validation_result<combined_validation_result)
        combined_validation_result = mate_validation_result;
      if (mate_validation_result<=mate_attackable)
        backtrack_definitively();

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
        record_decision_outcome("%s","replaying moves for testing");
        play_phase = play_initialising_replay;
        replay_fleshed_out_move_sequence(play_replay_testing);
        play_phase = play_testing_mate;

        /* This:
         * assert(solve_result>=previous_move_has_solved);
         * held surprisingly long, especially since it's wrong.
         * E.g. mate by castling: if we attack the rook, the castling is not
         * even playable */
        if (solve_result==previous_move_has_not_solved)
          backtrack_definitively();
      }

      break;

    case play_attacking_mating_piece:
      record_decision_outcome("%s","placed mating piece attacker");
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
    for (s = boardnum; *s && can_decision_level_be_continued(); ++s)
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
        push_decision_king_nomination(*s);
        restart_from_scratch();
        pop_decision();
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
  record_decision_outcome("%s","The king to be mated can be anywhere");
  REPORT_DEADEND;

  switch (play_phase)
  {
    case play_detecting_revelations:
      do_revelation_bookkeeping();
      break;

    case play_validating_mate:
      combined_validation_result = no_mate;
      combined_result = previous_move_has_not_solved;
      backtrack_definitively();
      break;

    default:
      combined_result = previous_move_has_not_solved;
      backtrack_definitively();
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
  record_decision_outcome("%s","The king to be mated can't be placed");
  REPORT_DEADEND;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void validate_king_placements(void)
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
    record_decision_outcome("%s","The king of the mating side can't be placed");
    REPORT_DEADEND;
  }
  else
    done_validating_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
