#include "pyquodli.h"
#include "pystip.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean quodlibet_end_is_unsolvable(slice_index si)
{
  boolean result;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u",op1);
  TraceValue("%u\n",op2);

  result = slice_is_unsolvable(op1) && slice_is_unsolvable(op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_unsolvable(si)
 * @param si slice index
 */
void quodlibet_write_unsolvability(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u",slices[si].u.composite.op1);
  TraceValue("%u\n",slices[si].u.composite.op2);

  slice_write_unsolvability(slices[si].u.composite.op1);
  slice_write_unsolvability(slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine and write continuations at end of quodlibet slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of quodlibet slice
 */
void quodlibet_end_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  d_slice_solve_continuations(table,slices[si].u.composite.op1);
  d_slice_solve_continuations(table,slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 */
void quodlibet_root_end_solve_setplay(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_root_solve_setplay(slices[si].u.composite.op1);
  slice_root_solve_setplay(slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean quodlibet_root_end_solve_complete_set(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (slice_root_solve_complete_set(slices[si].u.composite.op1)
            || slice_root_solve_complete_set(slices[si].u.composite.op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write solutions at root level starting at the end of
 * a quodlibet stipulation.
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
boolean quodlibet_root_end_solve(boolean restartenabled, slice_index si)
{
  boolean result = false;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",restartenabled);
  TraceFunctionParam("%u\n",si);

  if (slice_is_unsolvable(op1))
    slice_write_unsolvability(op1);
  else if (slice_is_unsolvable(op2))
    slice_write_unsolvability(op2);
  else
  {
    result = true;
    slice_root_solve(restartenabled,op1);
    slice_root_solve(restartenabled,op2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write the key just played, then solve the post key play (threats,
 * variations), starting at the end of a quodlibet slice.
 * @param si slice index
 * @param type type of attack
 */
void d_quodlibet_root_end_write_key_solve_postkey(slice_index si,
                                                  attack_type type)
{
  d_slice_root_write_key_solve_postkey(slices[si].u.composite.op1,type);
  d_slice_root_write_key_solve_postkey(slices[si].u.composite.op2,type);
}

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean quodlibet_end_has_solution(slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  /* use shortcut evaluation */
  result = slice_has_solution(op1) || slice_has_solution(op2);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u\n",result);
  return result;
}

/* Find and write variations from the end of a quodlibet slice.
 * @param si slice index
 */
void d_quodlibet_end_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  d_slice_solve_variations(slices[si].u.composite.op1);
  d_slice_solve_variations(slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
}

/* Determine whether a quodlibet slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean quodlibet_end_has_non_starter_solved(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (slice_has_non_starter_solved(slices[si].u.composite.op1)
            || slice_has_non_starter_solved(slices[si].u.composite.op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the defender has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the defending side has directly won
 */
boolean d_quodlibet_end_has_defender_won(slice_index si)
{
  boolean result = true;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (d_slice_has_defender_won(slices[si].u.composite.op1)
            && d_slice_has_defender_won(slices[si].u.composite.op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has immediately won in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly won
 */
boolean d_quodlibet_end_has_attacker_won(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (d_slice_has_attacker_won(slices[si].u.composite.op1)
            || d_slice_has_attacker_won(slices[si].u.composite.op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has immediately lost in direct play
 * with his move just played.
 * @param si slice identifier
 * @return true iff the attacking side has directly lost
 */
boolean d_quodlibet_end_has_attacker_lost(slice_index si)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = (d_slice_has_attacker_lost(slices[si].u.composite.op1)
            || d_slice_has_attacker_lost(slices[si].u.composite.op2));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Has the threat just played been refuted by the preceding defense?
 * @param si identifies stipulation slice
 * @return true iff the threat is refuted
 */
boolean d_quodlibet_end_is_threat_refuted(slice_index si)
{
  return (d_slice_is_threat_refuted(slices[si].u.composite.op1)
          && d_slice_is_threat_refuted(slices[si].u.composite.op2));
}

/* Continue solving at the end of a quodlibet slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_end_solve(slice_index si)
{
  boolean found_solution_op1 = false;
  boolean found_solution_op2 = false;
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

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
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",is_duplex);

  slice_detect_starter(op1,is_duplex);
  slice_detect_starter(op2,is_duplex);

  slices[si].starter = no_side;

  if (slices[op1].starter==no_side && slices[op1].type==STLeaf)
  {
    /* op1 can't tell - let's tell him */
    slices[si].starter = slices[op2].starter;
    slices[op1].starter = slices[op2].starter;
  }
  else if (slices[op2].starter==no_side && slices[op2].type==STLeaf)
  {
    /* op2 can't tell - let's tell him */
    slices[si].starter = slices[op1].starter;
    slices[op2].starter = slices[op1].starter;
  }
  else if (slices[op1].starter==slices[op2].starter)
    slices[si].starter = slices[op1].starter;

  TraceValue("%u\n",slices[si].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Impose the starting side on a slice.
 * @param si identifies sequence
 * @param s starting side of leaf
 */
void quodlibet_impose_starter(slice_index si, Side s)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  Side const next_starter = (slices[si].u.composite.play==PHelp
                             && slices[si].u.composite.length%2==1
                             ? advers(s)
                             : s);

  slices[si].starter = s;
  slice_impose_starter(op1,next_starter);
  slice_impose_starter(op2,next_starter);
}
