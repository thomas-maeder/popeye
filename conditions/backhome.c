#include "conditions/backhome.h"
#include "conditions/mummer.h"
#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "solving/observation.h"
#include "pydata.h"
#include "debugging/trace.h"

static square pieceid2pos[MaxPieceId+1];

int len_backhome(square sq_departure, square sq_arrival, square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = sq_arrival==pieceid2pos[GetPieceId(spec[sq_departure])];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean avoid_non_backhome_observations(square sq_observer,
                                               square sq_landing,
                                               square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  solve(slices[temporary_hack_ultra_mummer_length_measurer[trait[nbply]]].next2,length_unspecified);
  result = (*mummer_measure_length[trait[nbply]])(sq_observer,sq_landing,sq_observee)==mum_length[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise solving in Back-Home
 * @param si identifies root slice of stipulation
 */
void backhome_initialise_solving(void)
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

  register_observation_validator(&avoid_non_backhome_observations);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
