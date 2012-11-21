#include "conditions/circe/takemake.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/circe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];
static square rebirth_square[toppile+1];
numecoup take_make_circe_current_rebirth_square_index[maxply+1];

static boolean init_rebirth_squares(Side side_reborn)
{
  boolean result = false;
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  piece const save_p = e[sq_capture];
  Flags const save_f = spec[sq_capture];

  e[sq_capture] = pprise[nbply];
  spec[sq_capture] = pprispec[nbply];

  take_make_circe_current_rebirth_square_index[nbply] = take_make_circe_current_rebirth_square_index[nbply-1];
  TraceValue("%u",take_make_circe_current_rebirth_square_index[nbply]);
  TraceValue("%u\n",take_make_circe_current_rebirth_square_index[nbply-1]);

  nextply();
  if (side_reborn==White)
    gen_wh_piece(sq_capture,pprise[nbply-1]);
  else
    gen_bl_piece(sq_capture,pprise[nbply-1]);
  finply();

  numecoup i;
  for (i = current_move[nbply+1]; i>current_move[nbply]; --i)
  {
    TraceSquare(move_generation_stack[i].departure);
    TraceSquare(move_generation_stack[i].arrival);
    TraceSquare(move_generation_stack[i].capture);
    TracePiece(e[move_generation_stack[i].capture]);
    if (e[move_generation_stack[i].capture]==vide)
    {
      TraceText("accepting non-capture");
      ++take_make_circe_current_rebirth_square_index[nbply];
      TraceValue("%u\n",take_make_circe_current_rebirth_square_index[nbply]);
      rebirth_square[take_make_circe_current_rebirth_square_index[nbply]] = move_generation_stack[i].arrival;
      result = true;
    }
    else
      TraceText("discarding capture\n");
  }

  spec[sq_capture] = save_f;
  e[sq_capture] = save_p;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type take_make_circe_determine_rebirth_squares_solve(slice_index si,
                                                                 stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply]
      && !init_rebirth_squares(advers(slices[si].starter)))
    result = slack_length-2;
  else
  {
    current_circe_rebirth_square[nbply] = rebirth_square[take_make_circe_current_rebirth_square_index[nbply]];
    TraceSquare(current_circe_rebirth_square[nbply]);
    TraceValue("%u\n",take_make_circe_current_rebirth_square_index[nbply]);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      --take_make_circe_current_rebirth_square_index[nbply];

      TraceValue("%u",take_make_circe_current_rebirth_square_index[nbply]);
      TraceValue("%u\n",take_make_circe_current_rebirth_square_index[nbply-1]);
      if (take_make_circe_current_rebirth_square_index[nbply]>take_make_circe_current_rebirth_square_index[nbply-1])
        lock_post_move_iterations();
    }

    prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_take_make_circe(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STCirceDetermineRelevantPiece);
  stip_instrument_moves(si,STTakeMakeCirceDetermineRebirthSquares);
  stip_instrument_moves(si,STCircePlaceReborn);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
