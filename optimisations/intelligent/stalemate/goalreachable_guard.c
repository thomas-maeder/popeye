#include "optimisations/intelligent/stalemate/goalreachable_guard.h"
#include "stipulation/stipulation.h"
#include "solving/castling.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "debugging/trace.h"

#include <assert.h>


static boolean stalemate_are_there_sufficient_moves_left_for_required_captures(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  CapturesLeft[nbply] = CapturesLeft[parent_ply[nbply]];
  if (pprise[nbply]<vide)
    --CapturesLeft[nbply];

  TraceValue("%u\n",CapturesLeft[nbply]);
  result = MovesLeft[White]>=CapturesLeft[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean stalemate_isGoalReachable(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (pprise[nbply]
      && target_position[GetPieceId(pprispec[nbply])].diagram_square!=initsquare)
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
            MovesRequired[from_side][nbply] += intelligent_count_nr_of_moves_from_to_no_check(from_piece,
                                                                                              from_square,
                                                                                              target_position[id].type,
                                                                                              target_position[id].diagram_square);
          }
        }
      }
    }
    else
    {
      PieceIdType const id = GetPieceId(spec[move_generation_stack[current_move[nbply]].arrival]);
      MovesRequired[White][nbply] = MovesRequired[White][parent_ply[nbply]];
      MovesRequired[Black][nbply] = MovesRequired[Black][parent_ply[nbply]];

      if (target_position[id].diagram_square!=initsquare)
      {
        unsigned int const time_before = intelligent_count_nr_of_moves_from_to_no_check(pjoue[nbply],
                                                                                        move_generation_stack[current_move[nbply]].departure,
                                                                                        target_position[id].type,
                                                                                        target_position[id].diagram_square);

        unsigned int const time_now = intelligent_count_nr_of_moves_from_to_no_check(e[move_generation_stack[current_move[nbply]].arrival],
                                                                                     move_generation_stack[current_move[nbply]].arrival,
                                                                                     target_position[id].type,
                                                                                     target_position[id].diagram_square);

        TracePiece(pjoue[nbply]);
        TraceSquare(move_generation_stack[current_move[nbply]].departure);
        TracePiece(e[move_generation_stack[current_move[nbply]].arrival]);
        TraceSquare(move_generation_stack[current_move[nbply]].arrival);
        TracePiece(target_position[id].type);
        TraceSquare(target_position[id].diagram_square);
        TraceValue("%u",time_before);
        TraceValue("%u\n",time_now);

        assert(MovesRequired[trait[nbply]][nbply]+time_now>=time_before);
        MovesRequired[trait[nbply]][nbply] += time_now-time_before;
      }
    }

    TraceValue("%u",MovesRequired[White][nbply]);
    TraceValue("%u\n",MovesRequired[Black][nbply]);

    result = (MovesRequired[White][nbply]<=MovesLeft[White]
              && MovesRequired[Black][nbply]<=MovesLeft[Black]
              && stalemate_are_there_sufficient_moves_left_for_required_captures());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type goalreachable_guard_stalemate_attack(slice_index si,
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

  if (stalemate_isGoalReachable())
    result = attack(slices[si].next1,n);
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
