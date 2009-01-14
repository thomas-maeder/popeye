#include "pyquodli.h"
#include "pystip.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

/* Allocate a quodlibet slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index op1, slice_index op2)
{
  slice_index const result = alloc_slice_index();

  TraceFunctionEntry(__func__);
  TraceText("\n");

  slices[result].type = STQuodlibet; 
  slices[result].u.quodlibet.op1 = op1;
  slices[result].u.quodlibet.op2 = op2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean quodlibet_must_starter_resign(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = (slice_must_starter_resign(op1)
            && slice_must_starter_resign(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void quodlibet_write_unsolvability(slice_index si)
{
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u",slices[si].u.quodlibet.op1);
  TraceValue("%u\n",slices[si].u.quodlibet.op2);

  slice_write_unsolvability(op1);
  slice_write_unsolvability(op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write continuations of a quodlibet slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of quodlibet slice
 */
void quodlibet_solve_continuations(int table, slice_index si)
{
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_solve_continuations(table,op1);
  slice_solve_continuations(table,op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 */
boolean quodlibet_root_solve_setplay(slice_index si)
{
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;
  boolean result1;
  boolean result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result1 = slice_root_solve_setplay(op1);
  result2 = slice_root_solve_setplay(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n", result1 || result2);
  return result1 || result2;
}

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean quodlibet_root_solve_complete_set(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (slice_root_end_solve_complete_set(op1)
            || slice_root_end_solve_complete_set(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Solve a quodlibet slice at root level
 * @param si slice index
 */
boolean quodlibet_root_solve(slice_index si)
{
  boolean result = false;
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  if (slice_must_starter_resign(op1))
    slice_write_unsolvability(op1);
  else if (slice_must_starter_resign(op2))
    slice_write_unsolvability(op2);
  else
  {
    result = true;
    slice_root_solve(op1);
    slice_root_solve(op2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write the key just played, then solve the post key play (threats,
 * variations) of a quodlibet slice.
 * @param si slice index
 * @param type type of attack
 */
void quodlibet_root_write_key_solve_postkey(slice_index si,
                                            attack_type type)
{
  slice_root_write_key_solve_postkey(slices[si].u.quodlibet.op1,type);
  slice_root_write_key_solve_postkey(slices[si].u.quodlibet.op2,type);
}

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean quodlibet_has_solution(slice_index si)
{
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* use shortcut evaluation */
  result = slice_has_solution(op1) || slice_has_solution(op2);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u\n",result);
  return result;
}

/* Find and write variations of a quodlibet slice.
 * @param si slice index
 */
void quodlibet_solve_variations(slice_index si)
{
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_solve_variations(op1);
  slice_solve_variations(op2);

  TraceFunctionExit(__func__);
}

/* Determine whether a quodlibet slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean quodlibet_has_non_starter_solved(slice_index si)
{
  boolean result = true;
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (slice_has_non_starter_solved(op1)
            || slice_has_non_starter_solved(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean quodlibet_has_starter_won(slice_index si)
{
  boolean result = true;
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_starter_won(op1) || slice_has_starter_won(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean quodlibet_has_starter_apriori_lost(slice_index si)
{
  boolean result = true;
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (slice_has_starter_apriori_lost(op1)
            || slice_has_starter_apriori_lost(op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

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
 * @param is_duplex is this for duplex?
 */
void quodlibet_detect_starter(slice_index si, boolean is_duplex)
{
  slice_index const op1 = slices[si].u.quodlibet.op1;
  slice_index const op2 = slices[si].u.quodlibet.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",is_duplex);

  assert(slices[si].type==STQuodlibet);

  TraceValue("%u",slices[si].u.quodlibet.op1);
  TraceValue("%u\n",slices[si].u.quodlibet.op2);

  slice_detect_starter(op1,is_duplex);
  slice_detect_starter(op2,is_duplex);

  if (slice_get_starter(op1)==no_side)
    /* op1 can't tell - let's tell him */
    slice_impose_starter(op1,slice_get_starter(op2));
  else if (slice_get_starter(op2)==no_side)
    /* op2 can't tell - let's tell him */
    slice_impose_starter(op2,slice_get_starter(op1));

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of leaf
 */
void quodlibet_impose_starter(slice_index si, Side s)
{
  slice_impose_starter(slices[si].u.quodlibet.op1,s);
  slice_impose_starter(slices[si].u.quodlibet.op2,s);
}
