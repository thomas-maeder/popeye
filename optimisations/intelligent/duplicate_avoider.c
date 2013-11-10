#include "optimisations/intelligent/duplicate_avoider.h"
#include "optimisations/intelligent/intelligent.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>
#include <stdio.h>
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
    PieNam   prom;
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
    for (cp = 2; cp<=nbply; ++cp)
    {
      simplified_move_type * const elmt = *sol + cp;
      elmt->from = move_generation_stack[CURRMOVE_OF_PLY(cp)].departure;
      elmt->to = move_generation_stack[CURRMOVE_OF_PLY(cp)].arrival;
      elmt->prom = get_walk_of_piece_on_square(elmt->to);
    }
  }

  ++nr_stored_solutions;

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
        found = (elmt->from==move_generation_stack[CURRMOVE_OF_PLY(cp)].departure
                 && elmt->to==move_generation_stack[CURRMOVE_OF_PLY(cp)].arrival
                 && elmt->prom==get_walk_of_piece_on_square(move_generation_stack[CURRMOVE_OF_PLY(cp)].arrival));
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
stip_length_type intelligent_duplicate_avoider_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_duplicate_solution())
    result = n;
  else
  {
    result = solve(slices[si].next1,n);
    if (slack_length<=result && result<=n)
      store_solution();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
