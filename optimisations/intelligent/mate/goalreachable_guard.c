#include "optimisations/intelligent/mate/goalreachable_guard.h"
#include "stipulation/stipulation.h"
#include "solving/castling.h"
#include "solving/en_passant.h"
#include "solving/move_effect_journal.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "debugging/trace.h"

#include <assert.h>

static unsigned int OpeningsRequired[maxply+1];

static boolean mate_isGoalReachable(void)
{
  boolean result;
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal
      && target_position[GetPieceId(move_effect_journal[capture].u.piece_removal.removedspec)].diagram_square!=initsquare)
    /* a piece has been captured that participates in the mate */
    result = false;

  else
  {
    TraceValue("%u",MovesLeft[White]);
    TraceValue("%u\n",MovesLeft[Black]);

    if (nbply==2
        || (testcastling && castling_flag[nbply]!=castling_flag[parent_ply[nbply]]))
    {
      square const *bnp;
      MovesRequired[White][nbply] = 0;
      MovesRequired[Black][nbply] = 0;
      OpeningsRequired[nbply] = 0;
      for (bnp = boardnum; *bnp!=initsquare; bnp++)
      {
        square const from_square = *bnp;
        piece const from_piece = e[from_square];
        if (from_piece!=vide && from_piece!=obs)
        {
          PieceIdType const id = GetPieceId(spec[from_square]);
          if (target_position[id].diagram_square!=initsquare)
          {
            Side const from_side = from_piece>vide ? White : Black;
            if (from_side==White
                && white[PieceId2index[id]].usage==piece_gives_check
                && MovesLeft[White]>0)
            {
              square const save_king_square = king_square[Black];
              PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
              king_square[Black] = target_position[id_king].diagram_square;
              MovesRequired[from_side][nbply] += intelligent_count_nr_of_moves_from_to_checking(from_piece,
                                                                                                from_square,
                                                                                                target_position[id].type,
                                                                                                target_position[id].diagram_square);
              king_square[Black] = save_king_square;
            }
            else
              MovesRequired[from_side][nbply] += intelligent_count_nr_of_moves_from_to_no_check(from_piece,
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
      PieceIdType const id = GetPieceId(spec[move_generation_stack[current_move[nbply]].arrival]);
      MovesRequired[White][nbply] = MovesRequired[White][parent_ply[nbply]];
      MovesRequired[Black][nbply] = MovesRequired[Black][parent_ply[nbply]];
      OpeningsRequired[nbply] = OpeningsRequired[parent_ply[nbply]];

      if (nr_reasons_for_staying_empty[move_generation_stack[current_move[nbply]].departure]>0)
      {
        assert(OpeningsRequired[nbply]>0);
        --OpeningsRequired[nbply];
      }
      if (move_effect_journal[capture].type==move_effect_piece_removal
          && nr_reasons_for_staying_empty[move_generation_stack[current_move[nbply]].capture]>0)
      {
        assert(OpeningsRequired[nbply]>0);
        --OpeningsRequired[nbply];
      }
      if (nr_reasons_for_staying_empty[move_generation_stack[current_move[nbply]].arrival]>0)
        ++OpeningsRequired[nbply];

      if (target_position[id].diagram_square!=initsquare)
      {
        unsigned int time_before;
        unsigned int time_now;
        move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
        move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
        piece const pi_departing = move_effect_journal[movement].u.piece_movement.moving;
        if (trait[nbply]==White
            && white[PieceId2index[id]].usage==piece_gives_check)
        {
          square const save_king_square = king_square[Black];
          PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
          king_square[Black] = target_position[id_king].diagram_square;
          time_before = intelligent_count_nr_of_moves_from_to_checking(pi_departing,
                                                                       move_generation_stack[current_move[nbply]].departure,
                                                                       target_position[id].type,
                                                                       target_position[id].diagram_square);
          king_square[Black] = save_king_square;
        }
        else
          time_before = intelligent_count_nr_of_moves_from_to_no_check(pi_departing,
                                                                       move_generation_stack[current_move[nbply]].departure,
                                                                       target_position[id].type,
                                                                       target_position[id].diagram_square);

        if (trait[nbply]==White
            && white[PieceId2index[id]].usage==piece_gives_check
            && MovesLeft[White]>0)
        {
          square const save_king_square = king_square[Black];
          PieceIdType const id_king = GetPieceId(spec[king_square[Black]]);
          king_square[Black] = target_position[id_king].diagram_square;
          time_now = intelligent_count_nr_of_moves_from_to_checking(e[move_generation_stack[current_move[nbply]].arrival],
                                                                    move_generation_stack[current_move[nbply]].arrival,
                                                                    target_position[id].type,
                                                                    target_position[id].diagram_square);
          king_square[Black] = save_king_square;
        }
        else
          time_now = intelligent_count_nr_of_moves_from_to_no_check(e[move_generation_stack[current_move[nbply]].arrival],
                                                                    move_generation_stack[current_move[nbply]].arrival,
                                                                    target_position[id].type,
                                                                    target_position[id].diagram_square);

        assert(MovesRequired[trait[nbply]][nbply]+time_now>=time_before);
        MovesRequired[trait[nbply]][nbply] += time_now-time_before;
      }
    }

    result = (MovesRequired[White][nbply]<=MovesLeft[White]
              && MovesRequired[Black][nbply]<=MovesLeft[Black]
              && OpeningsRequired[nbply]<=MovesLeft[White]+MovesLeft[Black]+(ep[nbply]!=initsquare));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goalreachable_guard_mate_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,slices[si].starter,"");
  TraceEnumerator(Side,just_moved,"");
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  if (mate_isGoalReachable())
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[slices[si].starter]);
  TraceValue("%u\n",MovesLeft[just_moved]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
