#include "conditions/leffie.h"
#include "conditions/eiffel.h"
#include "stipulation/move.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_leffie_legality_testers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STLeffieLegalityTester);
  observation_play_move_to_validate(si,nr_sides);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_piece_illegally_observed(Side side, square z)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(z);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[z],side))
  {
    trait[nbply] = side; /* from Madrasi's perspective! */
    result = eiffel_is_piece_observed(z);
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_illegal_observation(void)
{
  Side const save_trait = trait[nbply];
  boolean result = false;
  square square_h = square_h8;
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = nr_rows_on_board; i>0; i--, square_h += dir_down)
  {
    int j;
    square z = square_h;
    for (j = nr_files_on_board; j>0; j--, z += dir_left)
      if (!is_square_empty(z)
          && (is_piece_illegally_observed(White,z)
              || is_piece_illegally_observed(Black,z)))
      {
        result = true;
        break;
      }
  }

  /* We have juggled with trait[npbly] to be able to reuse Madrasi
   * functionality. Now we have to clean up after ourselves: */
  trait[nbply] = save_trait;

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
void leffie_legality_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,find_illegal_observation());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
