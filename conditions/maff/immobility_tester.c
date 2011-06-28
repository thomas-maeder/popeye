#include "conditions/maff/immobility_tester.h"
#include "pydata.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "trace.h"

#include <stdlib.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static boolean maff_is_king_immobile(Side camp)
{
  unsigned int nr_king_flights = 0;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(camp));
  generate_king_moves(camp);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,camp))
      ++nr_king_flights;
    repcoup();
  }

  return nr_king_flights==1;
}

static boolean maff_immobile(Side camp)
{
  boolean result = true;
  nextply(nbply);
  current_killer_state = null_killer_state;
  trait[nbply] = camp;
  if (!maff_is_king_immobile(camp))
    result = false;
  else if (find_any_legal_move_king_first(camp))
    result = false;
  finply();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maff_immobility_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (maff_immobile(slices[si].starter))
    result = slice_has_solution(slices[si].u.immobility_tester.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
