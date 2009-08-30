#include "pyquodli.h"
#include "pyslice.h"
#include "pyproc.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>

/* Construct a quodlibet slice over an already allocated slice object
 * @param si index of slice object where to construct quodlibet slice
 * @param op1 1st operand
 * @param op2 2nd operand
 */
void make_quodlibet_slice(slice_index si, slice_index op1, slice_index op2)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  slices[si].type = STQuodlibet; 
  slices[si].u.fork.op1 = op1;
  slices[si].u.fork.op2 = op2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a quodlibet slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index op1, slice_index op2)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",op1);
  TraceFunctionParam("%u",op2);
  TraceFunctionParamListEnd();

  make_quodlibet_slice(result,op1,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void quodlibet_write_unsolvability(slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",slices[si].u.fork.op1);
  TraceValue("%u\n",slices[si].u.fork.op2);

  slice_write_unsolvability(op1);
  slice_write_unsolvability(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write continuations of a quodlibet slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of quodlibet slice
 */
void quodlibet_solve_continuations(table continuations, slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_solve_continuations(continuations,op1);
  slice_solve_continuations(continuations,op2);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void quodlibet_root_solve_in_n(slice_index si, stip_length_type n)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_root_solve_in_n(op1,n);
  slice_root_solve_in_n(op2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a quodlibet slice at root level
 * @param si slice index
 */
boolean quodlibet_root_solve(slice_index si)
{
  boolean result = false;
  boolean result1;
  boolean result2;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result1 = slice_root_solve(op1);
  result2 = slice_root_solve(op2);
  result = result1 || result2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write the key just played, then solve the post key play (threats,
 * variations) of a quodlibet slice.
 * @param si slice index
 * @param type type of attack
 */
void quodlibet_root_write_key(slice_index si, attack_type type)
{
  if (slice_has_starter_won(slices[si].u.fork.op1))
    slice_root_write_key(slices[si].u.fork.op1,type);
  if (slice_has_starter_won(slices[si].u.fork.op2))
    slice_root_write_key(slices[si].u.fork.op2,type);
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean quodlibet_are_threats_refuted(table threats, slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* TODO this can't be correct */
  result = (slice_are_threats_refuted(threats,op1)
            && slice_are_threats_refuted(threats,op2));

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_has_solution(slice_index si)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(op1);
  if (result==has_no_solution)
    result = slice_has_solution(op2);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionParamListEnd();
  return result;
}

/* Find and write post key play
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_solve_postkey(slice_index si)
{
  boolean result;
  boolean result1;
  boolean result2;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result1 = slice_solve_postkey(op1);
  result2 = slice_solve_postkey(op2);

  result = result1 || result2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a quodlibet slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean quodlibet_has_non_starter_solved(slice_index si)
{
  boolean result = true;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (slice_has_non_starter_solved(op1)
            || slice_has_non_starter_solved(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return whether the starter has won
 */
has_starter_won_result_type quodlibet_has_starter_won(slice_index si)
{
  has_starter_won_result_type result;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_starter_won(op1);
  if (result==starter_has_not_won)
    result = slice_has_starter_won(op2);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_starter_won_result_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean quodlibet_has_starter_reached_goal(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (slice_has_starter_reached_goal(op1)
            || slice_has_starter_reached_goal(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @return true iff defender wins
 */
boolean quodlibet_does_defender_win(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (slice_does_defender_win(op1)
            && slice_does_defender_win(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a quodlibet slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_solve(slice_index si)
{
  boolean found_solution_op1 = false;
  boolean found_solution_op2 = false;
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  /* avoid short-cut boolean evaluation */
  found_solution_op1 = slice_solve(op1);
  found_solution_op2 = slice_solve(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution_op1 || found_solution_op2);
  return found_solution_op1 || found_solution_op2;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter quodlibet_detect_starter(slice_index si,
                                                boolean same_side_as_root)
{
  slice_index const op1 = slices[si].u.fork.op1;
  slice_index const op2 = slices[si].u.fork.op2;
  who_decides_on_starter result;
  who_decides_on_starter result1;
  who_decides_on_starter result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].type==STQuodlibet);

  TraceValue("%u",slices[si].u.fork.op1);
  TraceValue("%u\n",slices[si].u.fork.op2);

  result1 = slice_detect_starter(op1,same_side_as_root);
  result2 = slice_detect_starter(op2,same_side_as_root);

  if (slices[op1].starter==no_side)
    slices[si].starter = slices[op2].starter;
  else
    slices[si].starter = slices[op1].starter;

  if (result1==dont_know_who_decides_on_starter)
    result = result2;
  else
  {
    assert(result2==dont_know_who_decides_on_starter
           || slices[op1].starter==slices[op2].starter);
    result = result1;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean quodlibet_impose_starter(slice_index si, slice_traversal *st)
{
  boolean result;
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
