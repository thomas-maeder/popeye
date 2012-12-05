#include "conditions/circe/takemake.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/circe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "stipulation/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/single_piece_move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];
static square rebirth_square[toppile+1];
numecoup take_make_circe_current_rebirth_square_index[maxply+1];

static boolean init_rebirth_squares(Side side_reborn)
{
  boolean result = false;
  square const sq_capture = move_generation_stack[current_move[nbply]].capture;
  piece const pi_capturing = e[sq_capture];
  Flags const flags_capturing = spec[sq_capture];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  e[sq_capture] = pprise[nbply];
  spec[sq_capture] = pprispec[nbply];

  init_single_piece_move_generator(sq_capture);

  result = solve(slices[temporary_hack_circe_take_make_rebirth_squares_finder[side_reborn]].next2,length_unspecified)==has_solution;

  e[sq_capture] = pi_capturing;
  spec[sq_capture] = flags_capturing;

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
stip_length_type take_make_circe_collect_rebirth_squares_solve(slice_index si,
                                                               stip_length_type n)
{
  stip_length_type result = slack_length-2;
  numecoup i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  take_make_circe_current_rebirth_square_index[nbply-1] = take_make_circe_current_rebirth_square_index[nbply-2];

  for (i = current_move[nbply]; i>current_move[nbply-1]; --i)
    if (e[move_generation_stack[i].capture]==vide)
    {
      ++take_make_circe_current_rebirth_square_index[nbply-1];
      rebirth_square[take_make_circe_current_rebirth_square_index[nbply-1]] = move_generation_stack[i].arrival;
      result = n;
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

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      --take_make_circe_current_rebirth_square_index[nbply];

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
