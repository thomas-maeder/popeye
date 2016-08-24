#include "optimisations/intelligent/mate/goalreachable_guard.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/castling.h"
#include "solving/move_effect_journal.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

static unsigned int OpeningsRequired[maxply+1];

static boolean mate_isGoalReachable(void)
{
  boolean result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal
      && target_position[GetPieceId(move_effect_journal[capture].u.piece_removal.flags)].diagram_square!=initsquare)
    /* a piece has been captured that participates in the mate */
    result = false;

  else
  {
    move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
    TraceValue("%u",MovesLeft[White]);
    TraceValue("%u",MovesLeft[Black]);
    TraceEOL();

    if (parent_ply[nbply]==ply_retro_move
        || move_effect_journal[top-1].type==move_effect_disable_castling_right)
    {
      square const *bnp;
      MovesRequired[White][nbply] = 0;
      MovesRequired[Black][nbply] = 0;
      OpeningsRequired[nbply] = 0;
      for (bnp = boardnum; *bnp!=initsquare; bnp++)
      {
        square const from_square = *bnp;
        piece_walk_type const from_piece = get_walk_of_piece_on_square(from_square);
        if (from_piece!=Empty && from_piece!=Invalid)
        {
          PieceIdType const id = GetPieceId(being_solved.spec[from_square]);
          if (target_position[id].diagram_square!=initsquare)
          {
            Side const from_side = TSTFLAG(being_solved.spec[from_square],White) ? White : Black;
            if (from_side==White
                && white[PieceId2index[id]].usage==piece_gives_check
                && MovesLeft[White]>0)
            {
              square const save_king_square = being_solved.king_square[Black];
              PieceIdType const id_king = GetPieceId(being_solved.spec[being_solved.king_square[Black]]);
              being_solved.king_square[Black] = target_position[id_king].diagram_square;
              MovesRequired[from_side][nbply] += intelligent_count_nr_of_moves_from_to_checking(from_side,
                                                                                                from_piece,
                                                                                                from_square,
                                                                                                target_position[id].type,
                                                                                                target_position[id].diagram_square);
              being_solved.king_square[Black] = save_king_square;
            }
            else
              MovesRequired[from_side][nbply] += intelligent_count_nr_of_moves_from_to_no_check(from_side,
                                                                                                from_piece,
                                                                                                from_square,
                                                                                                target_position[id].type,
                                                                                                target_position[id].diagram_square);
          }

          if (nr_reasons_for_staying_empty[from_square]>0)
            ++OpeningsRequired[nbply];
        }
      }
    }
    else
    {
      move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
      square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
      square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
      square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
      PieceIdType const id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
      MovesRequired[White][nbply] = MovesRequired[White][parent_ply[nbply]];
      MovesRequired[Black][nbply] = MovesRequired[Black][parent_ply[nbply]];
      OpeningsRequired[nbply] = OpeningsRequired[parent_ply[nbply]];

      if (nr_reasons_for_staying_empty[sq_departure]>0)
      {
        assert(OpeningsRequired[nbply]>0);
        --OpeningsRequired[nbply];
      }
      if (move_effect_journal[capture].type==move_effect_piece_removal
          && nr_reasons_for_staying_empty[sq_capture]>0)
      {
        assert(OpeningsRequired[nbply]>0);
        --OpeningsRequired[nbply];
      }
      if (nr_reasons_for_staying_empty[sq_arrival]>0)
        ++OpeningsRequired[nbply];

      if (target_position[id].diagram_square!=initsquare)
      {
        unsigned int time_before;
        unsigned int time_now;
        piece_walk_type const pi_departing = move_effect_journal[movement].u.piece_movement.moving;
        piece_walk_type const pi_arrived = get_walk_of_piece_on_square(sq_arrival);
        Side const side_arrived = TSTFLAG(being_solved.spec[sq_arrival],White) ? White : Black;
        if (trait[nbply]==White
            && white[PieceId2index[id]].usage==piece_gives_check)
        {
          square const save_king_square = being_solved.king_square[Black];
          PieceIdType const id_king = GetPieceId(being_solved.spec[being_solved.king_square[Black]]);
          being_solved.king_square[Black] = target_position[id_king].diagram_square;
          time_before = intelligent_count_nr_of_moves_from_to_checking(side_arrived,
                                                                       pi_departing,
                                                                       sq_departure,
                                                                       target_position[id].type,
                                                                       target_position[id].diagram_square);
          being_solved.king_square[Black] = save_king_square;
        }
        else
          time_before = intelligent_count_nr_of_moves_from_to_no_check(side_arrived,
                                                                       pi_departing,
                                                                       sq_departure,
                                                                       target_position[id].type,
                                                                       target_position[id].diagram_square);

        if (trait[nbply]==White
            && white[PieceId2index[id]].usage==piece_gives_check
            && MovesLeft[White]>0)
        {
          square const save_king_square = being_solved.king_square[Black];
          PieceIdType const id_king = GetPieceId(being_solved.spec[being_solved.king_square[Black]]);
          being_solved.king_square[Black] = target_position[id_king].diagram_square;
          time_now = intelligent_count_nr_of_moves_from_to_checking(side_arrived,
                                                                    pi_arrived,
                                                                    sq_arrival,
                                                                    target_position[id].type,
                                                                    target_position[id].diagram_square);
          being_solved.king_square[Black] = save_king_square;
        }
        else
          time_now = intelligent_count_nr_of_moves_from_to_no_check(side_arrived,
                                                                    pi_arrived,
                                                                    sq_arrival,
                                                                    target_position[id].type,
                                                                    target_position[id].diagram_square);

        assert(MovesRequired[trait[nbply]][nbply]+time_now>=time_before);
        MovesRequired[trait[nbply]][nbply] += time_now-time_before;
      }
    }

    result = (MovesRequired[White][nbply]<=MovesLeft[White]
              && MovesRequired[Black][nbply]<=MovesLeft[Black]
              && OpeningsRequired[nbply]<=MovesLeft[White]+MovesLeft[Black]+(en_passant_was_multistep_played(nbply)));
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
void goalreachable_guard_mate_solve(slice_index si)
{
  Side const just_moved = advers(SLICE_STARTER(si));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  --MovesLeft[just_moved];
  TraceEnumerator(Side,SLICE_STARTER(si));
  TraceEnumerator(Side,just_moved);
  TraceValue("%u",MovesLeft[SLICE_STARTER(si)]);
  TraceValue("%u",MovesLeft[just_moved]);
  TraceEOL();

  pipe_this_move_doesnt_solve_if(si,!mate_isGoalReachable());

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[SLICE_STARTER(si)]);
  TraceValue("%u",MovesLeft[just_moved]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
