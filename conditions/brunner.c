#include "conditions/brunner.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/single_move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

static Side guess_side_at_move(square sq_departure, square sq_capture)
{
  Side result;

  /* the following does not suffice if we have neutral kings,
     but we have no chance to recover the information who is to
     move from sq_departure, sq_arrival and sq_capture.
     TLi
  */
  if (TSTFLAG(some_pieces_flags,Neutral)
      && king_square[White]!=initsquare
      && TSTFLAG(spec[king_square[White]],Neutral))
    /* will this do for neutral Ks? */
    result = neutral_side;
  else if (sq_capture==king_square[Black])
    result = White;
  else if (sq_capture==king_square[White])
    result = Black;
  else
    result = e[sq_departure]<0 ? Black : White;

  return result;
}

static boolean avoid_undoable_observation(square sq_observer,
                                          square sq_landing,
                                          square sq_observee)
{
  boolean result;
  Side side;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  side = guess_side_at_move(sq_observer,sq_observee);

  init_single_move_generator(sq_observer,sq_landing,sq_observee);
  result = solve(slices[temporary_hack_brunner_check_defense_finder[side]].next2,length_unspecified)==next_move_has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Brunner Chess
 */
void brunner_initialise_solving(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  register_observation_validator(&avoid_undoable_observation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
