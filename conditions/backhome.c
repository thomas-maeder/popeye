#include "conditions/backhome.h"
#include "conditions/mummer.h"
#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "solving/observation.h"
#include "solving/legal_move_counter.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

static square pieceid2pos[MaxPieceId+1];

static boolean goes_back_home(square sq_departure, square sq_arrival)
{
  return sq_arrival==pieceid2pos[GetPieceId(spec[sq_departure])];
}

int len_backhome(square sq_departure, square sq_arrival, square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = goes_back_home(sq_departure,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
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
stip_length_type back_home_moves_only_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (goes_back_home(move_generation_stack[current_move[nbply]].departure,
                     move_generation_stack[current_move[nbply]].arrival))
    result = solve(slices[si].next1,n);
  else
    result = previous_move_is_illegal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Back Home
 * @return true iff the observation is valid
 */
boolean back_home_validate_observation(slice_index si)
{
  square const sq_observer = move_generation_stack[current_move[nbply]].departure;
  square const sq_landing = move_generation_stack[current_move[nbply]].arrival;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (goes_back_home(sq_observer,sq_landing))
    result = true;
  else
  {
    /* avoid concurrent counts */
    assert(legal_move_counter_count[nbply]==0);

    /* stop counting once we have 1 legal move back home */
    legal_move_counter_interesting[nbply] = 1;

    /* the observation is ok if there is no legal move back home */
    result = (solve(slices[temporary_hack_back_home_finder[trait[nbply]]].next2,
                     length_unspecified)
              != next_move_has_no_solution);

    assert(result
           ==(legal_move_counter_count[nbply]
              <=legal_move_counter_interesting[nbply]));

    /* clean up after ourselves */
    legal_move_counter_count[nbply] = 0;
  }

  if (result)
    result = validate_observation_recursive(slices[si].next1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise solving in Back-Home
 * @param si identifies root slice of stipulation
 */
void backhome_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      pieceid2pos[id] = initsquare;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp) && !is_square_blocked(*bnp))
        pieceid2pos[GetPieceId(spec[*bnp])] = *bnp;
  }

  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationBackHome);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
