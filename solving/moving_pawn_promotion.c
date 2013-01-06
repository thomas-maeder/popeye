#include "solving/moving_pawn_promotion.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/moving_pawn_promotion.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

PieNam getprompiece[PieceCount];
PieNam current_promotion_of_moving[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

/* Initialise the set of promotion pieces for the current twin
 */
void init_promotion_pieces(void)
{
  PieNam p;
  PieNam prev_prom_piece = Empty;
  PieNam firstprompiece;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
    firstprompiece = King;
  else if ((CondFlag[singlebox] && SingleBoxType!=singlebox_type1) || CondFlag[football])
    firstprompiece = Pawn;
  else
    firstprompiece = Queen;

  for (p = firstprompiece; p<PieceCount; ++p)
  {
    getprompiece[p] = Empty;

    if (exist[p])
    {
      if ((p!=Pawn || (CondFlag[singlebox] && SingleBoxType!=singlebox_type1))
          && (p!=King
              || CondFlag[losingchess]
              || CondFlag[dynasty]
              || CondFlag[extinction])
          && p!=Dummy
          && p!=BerolinaPawn
          && p!=SuperBerolinaPawn
          && p!=SuperPawn
          && p!=ReversePawn
          && (!CondFlag[promotiononly] || promonly[p]))
      {
        TracePiece(prev_prom_piece);
        TracePiece(p);
        TraceText("\n");
        getprompiece[prev_prom_piece] = p;
        prev_prom_piece = p;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Has a pawn reached a promotion square
 * @param side the pawn's side
 * @param square_reached square reached by the pawn
 * @return true iff square_reached is a promotion square
 */
boolean has_pawn_reached_promotion_square(Side side, square square_reached)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(square_reached);
  TraceFunctionParamListEnd();

  result = (is_pawn(abs(e[square_reached]))
           && PromSq(is_reversepawn(abs(e[square_reached]))^side,square_reached)
           && ((!CondFlag[protean] && !TSTFLAG(spec[square_reached],Protean))
               || pprise[nbply]==vide));

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
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type moving_pawn_promoter_solve(slice_index si, stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    boolean const is_prom_square = has_pawn_reached_promotion_square(slices[si].starter,
                                                                     sq_arrival);
    current_promotion_of_moving[nbply] = is_prom_square ? getprompiece[Empty] : Empty;
  }

  if (current_promotion_of_moving[nbply]==Empty)
    result = solve(slices[si].next1,n);
  else
  {
    piece const promotee = (e[sq_arrival]<vide
                            ? -current_promotion_of_moving[nbply]
                            : current_promotion_of_moving[nbply]);

    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        sq_arrival,promotee);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_moving[nbply] = getprompiece[current_promotion_of_moving[nbply]];
      if (current_promotion_of_moving[nbply]!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_moving_pawn_promoters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMovingPawnPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
