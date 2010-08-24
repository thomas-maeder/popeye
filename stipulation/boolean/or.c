#include "pyquodli.h"
#include "pyslice.h"
#include "pypipe.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "pyintslv.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a quodlibet slice.
 * @param proxy1 1st operand
 * @param proxy2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index proxy1, slice_index proxy2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy1);
  TraceFunctionParam("%u",proxy2);
  TraceFunctionParamListEnd();

  assert(proxy1!=no_slice);
  assert(proxy2!=no_slice);

  result = alloc_slice(STQuodlibet);

  slices[result].u.binary.op1 = proxy1;
  assert(slices[proxy1].type==STProxy);
  slices[result].u.binary.op2 = proxy2;
  assert(slices[proxy2].type==STProxy);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void quodlibet_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  slice_index copy;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  copy = copy_slice(si);

  stip_traverse_structure(slices[si].u.binary.op1,st);
  slices[copy].u.binary.op1 = *root;

  *root = no_slice;
  
  stip_traverse_structure(slices[si].u.binary.op2,st);
  slices[copy].u.binary.op2 = *root;
  
  *root = copy;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_has_solution(slice_index si)
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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
quodlibet_has_solution_in_n(slice_index si,
                            stip_length_type n,
                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = attack_has_solution_in_n(op1,n,n_max_unsolvable);
  if (result>n)
    result = attack_has_solution_in_n(op2,n,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_solve(slice_index si)
{
  has_solution_type result;
  has_solution_type found_solution_op1;
  has_solution_type found_solution_op2;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* avoid short-cut boolean evaluation */
  found_solution_op1 = slice_solve(op1);
  found_solution_op2 = slice_solve(op2);

  result = (found_solution_op1>found_solution_op2
            ? found_solution_op1
            : found_solution_op2);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type quodlibet_solve_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  stip_length_type result1;
  stip_length_type result2;
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result1 = attack_solve_in_n(op1,n,n_max_unsolvable);
  result2 = attack_solve_in_n(op2,n,n_max_unsolvable);
  result = result1<result2 ? result1 : result2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void quodlibet_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const op1 = slices[si].u.binary.op1;
  slice_index const op2 = slices[si].u.binary.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    stip_traverse_structure(op1,st);
    stip_traverse_structure(op2,st);

    TraceStipulation(si);

    if (slices[op1].starter==no_side)
      slices[si].starter = slices[op2].starter;
    else
    {
      assert(slices[op2].starter==no_side
             || slices[op1].starter==slices[op2].starter);
      slices[si].starter = slices[op1].starter;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
