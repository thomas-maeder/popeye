#include "conditions/circe/takemake.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/circe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "stipulation/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/single_piece_move_generator.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];
static square rebirth_square[toppile+1];
static numecoup take_make_circe_current_rebirth_square_index[maxply+1];
static unsigned int stack_pointer = 1;

static boolean init_rebirth_squares(Side side_reborn)
{
  boolean result = false;
  square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  PieNam const pi_capturing = get_walk_of_piece_on_square(sq_capture);
  Flags const flags_capturing = spec[sq_capture];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  take_make_circe_current_rebirth_square_index[stack_pointer] = take_make_circe_current_rebirth_square_index[stack_pointer-1];

  occupy_square(sq_capture,
                move_effect_journal[capture].u.piece_removal.removed,
                move_effect_journal[capture].u.piece_removal.removedspec);

  init_single_piece_move_generator(sq_capture);

  result = solve(slices[temporary_hack_circe_take_make_rebirth_squares_finder[side_reborn]].next2,length_unspecified)==next_move_has_solution;

  assert(pi_capturing!=Invalid);

  if (pi_capturing==Empty)
    empty_square(sq_capture);
  else
    occupy_square(sq_capture,pi_capturing,flags_capturing);

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
stip_length_type take_make_circe_collect_rebirth_squares_solve(slice_index si,
                                                               stip_length_type n)
{
  stip_length_type result = previous_move_is_illegal;
  numecoup i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = CURRMOVE_OF_PLY(nbply); i>CURRMOVE_OF_PLY(nbply-1); --i)
    if (is_square_empty(move_generation_stack[i].capture))
    {
      ++take_make_circe_current_rebirth_square_index[stack_pointer];
      rebirth_square[take_make_circe_current_rebirth_square_index[stack_pointer]] = move_generation_stack[i].arrival;
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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
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
    result = previous_move_is_illegal;
  else
  {
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = rebirth_square[take_make_circe_current_rebirth_square_index[stack_pointer]];

    ++stack_pointer;
    result = solve(slices[si].next1,n);
    --stack_pointer;

    if (!post_move_iteration_locked[nbply])
    {
      --take_make_circe_current_rebirth_square_index[stack_pointer];

      if (take_make_circe_current_rebirth_square_index[stack_pointer]>take_make_circe_current_rebirth_square_index[stack_pointer-1])
        lock_post_move_iterations();
    }

    prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument the solving machinery with Circe Take&Make
 * @param si identifies root slice of stipulation
 */
void circe_take_make_initialse_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STTakeMakeCirceDetermineRebirthSquares);
  stip_instrument_moves(si,STCircePlaceReborn);
  stip_insert_rebirth_avoider(si,STSuperCirceCaptureFork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
