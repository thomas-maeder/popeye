#include "optimisations/intelligent/duplicate_avoider.h"
#include "pydata.h"
#include "pypipe.h"
#include "pyint.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* This module provides the slice type STIntelligentDuplicateAvoider
 * which avoids producing certain solutions twice in intelligent mode
 */

/* we can use a simplified move type because intelligent mode is only
 * supported in orthodox play
 */
typedef struct
{
    square  from;
    square  to;
    piece   prom;
} simplified_move_type;

static simplified_move_type **stored_solutions;

static unsigned int nr_stored_solutions;

/* Initialise the duplication avoidance machinery
 */
void intelligent_duplicate_avoider_init(void)
{
  nr_stored_solutions = 0;
}

/* Cleanup the duplication avoidance machinery
 */
void intelligent_duplicate_avoider_cleanup(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nr_stored_solutions>0)
  {
    simplified_move_type **sol;
    for (sol = stored_solutions;
         sol!=stored_solutions+nr_stored_solutions;
         ++sol)
      free(*sol);

    free(stored_solutions);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate room for storing the current solution
 * @return true iff room could be allocated
 */
static boolean alloc_room_for_solution(void)
{
  boolean result;

  if (stored_solutions==NULL)
    result = false;
  else
  {
    simplified_move_type ** const sol = stored_solutions+nr_stored_solutions;
    *sol = calloc(nbply+1, sizeof **sol);
    result = *sol!=NULL;
  }

  return result;
}

/* Store the current solution in order to avoid writing it again later
 */
static void store_solution(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nr_stored_solutions>0)
  {
    size_t const size = (nr_stored_solutions+1) * sizeof *stored_solutions;
    stored_solutions = realloc(stored_solutions,size);
  }
  else
    stored_solutions = malloc(sizeof *stored_solutions);

  if (!alloc_room_for_solution())
  {
    fprintf(stderr, "Cannot (re)allocate enough memory\n");
    exit(EXIT_FAILURE);
  }

  {
    simplified_move_type ** const sol = stored_solutions+nr_stored_solutions;
    ply cp;
    for (cp = 2; cp <= nbply; ++cp)
    {
      simplified_move_type * const elmt = *sol + cp;
      elmt->from = move_generation_stack[repere[cp+1]].departure;
      elmt->to = move_generation_stack[repere[cp+1]].arrival;
      elmt->prom = jouearr[cp];
    }
  }

  ++nr_stored_solutions;

  ++sol_per_matingpos;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the current solution was already found earlier
 * @return true iff the current solution was already found earlier
 */
static boolean is_duplicate_solution(void)
{
  boolean found = false;
  simplified_move_type ** sol;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  repere[nbply+1] = nbcou;

  for (sol = stored_solutions;
       sol!=stored_solutions+nr_stored_solutions && !found;
       ++sol)
  {
    found = true;

    {
      ply cp;
      for (cp = 2; cp <= nbply && found; ++cp)
      {
        simplified_move_type * const elmt = (*sol)+cp;
        found = (elmt->from==move_generation_stack[repere[cp+1]].departure
                 && elmt->to==move_generation_stack[repere[cp+1]].arrival
                 && elmt->prom==jouearr[cp]);
      }
    }
  }


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",found);
  TraceFunctionResultEnd();
  return found;
}

/* Allocate a STIntelligentDuplicateAvoider slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_duplicate_avoider_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentDuplicateAvoider);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type intelligent_duplicate_avoider_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type intelligent_duplicate_avoider_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_duplicate_solution())
    result = has_solution;
  else
  {
    result = slice_solve(slices[si].u.pipe.next);
    if (result==has_solution)
      store_solution();
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
