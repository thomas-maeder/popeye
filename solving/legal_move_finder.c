#include "solving/legal_move_finder.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

square legal_move_finder_departure;
square legal_move_finder_arrival;

/* Allocate a STLegalMoveFinder slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_move_finder_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STLegalMoveFinder);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the search for a legal move */
void init_legal_move_finder(void)
{
  assert(legal_move_finder_departure==initsquare);
  assert(legal_move_finder_arrival==initsquare);
}

/* Finalise the search for a legal move */
void fini_legal_move_finder(void)
{
  legal_move_finder_departure = initsquare;
  legal_move_finder_arrival = initsquare;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type legal_move_finder_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.pipe.next);

  if (result==has_solution)
  {
    legal_move_finder_departure = move_generation_stack[nbcou].departure;
    legal_move_finder_arrival = move_generation_stack[nbcou].arrival;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
