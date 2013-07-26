#include "conditions/isardam.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "stipulation/temporary_hacks.h"
#include "conditions/madrasi.h"
#include "solving/single_move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_isardam_legality_testers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STIsardamLegalityTester);

  if (!IsardamB)
    stip_instrument_observation_testing(si,
                                        nr_sides,
                                        STTestObservationGeometryByPlayingMove);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_piece_illegally_observed(Side side, square z)
{
  boolean result;

  if (TSTFLAG(spec[z],side))
  {
    trait[nbply] = side; /* from Madrasi's perspective! */
    result = madrasi_is_moving_piece_observed(z);
  }
  else
    result = false;

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
stip_length_type isardam_legality_tester_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (find_illegal_observation())
    result = previous_move_is_illegal;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
