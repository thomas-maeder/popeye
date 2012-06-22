#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "pystip.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "debugging/trace.h"

#include <stdlib.h>

/* maximum number of solutions per target positions set by user
 */
static unsigned long max_nr_solutions_per_target_position = ULONG_MAX;

/* number of solutions in the current target positions
 */
static unsigned long nr_solutions_in_current_target_position;

/* remember whether solving was affected because of the maximum number
 */
static boolean was_solving_affected;

/* Reset the maximum number of solutions per target position
 */
void reset_max_nr_solutions_per_target_position(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  max_nr_solutions_per_target_position = ULONG_MAX;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the number of solutions per target position
 */
void reset_nr_solutions_per_target_position(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_solutions_in_current_target_position = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset status whether solving the current problem was affected because the limit
 * on the number of solutions per target position was reached.
 */
void reset_was_max_nr_solutions_per_target_position_reached(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  was_solving_affected = false;;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether solving the current problem was affected because the limit
 * on the number of solutions per target position was reached.
 * @return true iff solving was affected
 */
boolean was_max_nr_solutions_per_target_position_reached(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",was_solving_affected);
  TraceFunctionResultEnd();
  return was_solving_affected;
}

/* Determine whether the maximum number of solutions per target position is
 * limited
 * @return true iff the maximum number is limited
 */
boolean is_max_nr_solutions_per_target_position_limited(void)
{
  boolean const result = max_nr_solutions_per_target_position!=ULONG_MAX;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Attempt to read the maximum number of solutions per target position
 * @param tok next1 input token
 * @return true iff the maximum number could be read from tok
 */
boolean read_max_nr_solutions_per_target_position(char const *tok)
{
  boolean result;
  char *end;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  max_nr_solutions_per_target_position = strtoul(tok,&end,10);
  if (end==tok)
  {
    max_nr_solutions_per_target_position = ULONG_MAX;
    result = false;
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STIntelligentSolutionsPerTargetPosCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_nr_solutions_per_target_position_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentSolutionsPerTargetPosCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
intelligent_nr_solutions_per_target_position_counter_attack(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(slices[si].next1,n);
  if (slack_length<=result && result<=n)
    ++nr_solutions_in_current_target_position;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STIntelligentLimitNrSolutionsPerTargetPos slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_limit_nr_solutions_per_target_position_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentLimitNrSolutionsPerTargetPos);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
intelligent_limit_nr_solutions_per_target_position_attack(slice_index si,
                                                          stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (nr_solutions_in_current_target_position
      >=max_nr_solutions_per_target_position)
  {
    was_solving_affected = true;
    result = n+2;
  }
  else
    result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
