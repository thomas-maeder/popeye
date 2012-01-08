#include "stipulation/boolean/or.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STOr slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_or_slice(slice_index op1, slice_index op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STOr,op1,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type or_has_solution(slice_index si)
{
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slice_has_solution(op1))
  {
    case opponent_self_check:
      if (slice_has_solution(op2)==has_solution)
        result = has_solution;
      else
        result = opponent_self_check;
      break;

    case has_no_solution:
      result = slice_has_solution(op2);
      break;

    case has_solution:
      switch (slice_has_solution(op2))
      {
        case opponent_self_check:
          result = opponent_self_check;
          break;

        default:
          result = has_solution;
          break;
      }
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionParamListEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type or_can_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_attack(op1,n);
  if (result<=slack_length_battle-2 || result>n)
    result = can_attack(op2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type or_solve(slice_index si)
{
  has_solution_type result;
  has_solution_type result1;
  has_solution_type result2;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result1 = slice_solve(op1);
  result2 = slice_solve(op2);

  switch (result1)
  {
    case opponent_self_check:
      if (result2==has_solution)
        result = has_solution;
      else
        result = opponent_self_check;
      break;

    case has_no_solution:
      result = result2;
      break;

    case has_solution:
      result = has_solution;
      break;

    default:
      assert(0);
      result = opponent_self_check;
      break;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type or_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  stip_length_type result1;
  stip_length_type result2;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result1 = attack(op1,n);
  result2 = attack(op2,n);
  result = result1<result2 ? result1 : result2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
