#include "pysequen.h"
#include "pystip.h"
#include "pyleaf.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

/* Determine and write continuations at end of sequence slice
 * @param attacker attacking side
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of sequence slice
 */
void d_sequence_end_solve_continuations(couleur attacker,
                                        int table,
                                        slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  switch (slices[op1].type)
  {
    case STLeaf:
      d_leaf_solve_continuations(attacker,table,op1);
      break;
    
    default:
      assert(0);
      break;
  }
}

/* Find and write defender's set play
 * @param defender defending side
 * @param leaf slice index
 */
void d_sequence_end_solve_setplay(couleur defender, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  switch (slices[op1].type)
  {
    case STLeaf:
      d_leaf_solve_setplay(defender,op1);
      break;

    default:
      assert(0);
      break;
  }
}

/* Find and write defender's set play in self/reflex play if every
 * set move leads to end
 * @param defender defending side
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean d_sequence_end_solve_complete_set(couleur defender, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  switch (slices[op1].type)
  {
    case STLeaf:
      return d_leaf_solve_complete_set(defender,op1);

    default:
      assert(0);
      return false;
  }
}


/* Continue solving at the end of a sequence slice
 * Unsolvability (e.g. because of a forced reflex move) has already
 * been dealt with.
 * @param attacker attacking side
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param leaf slice index 
 */
void d_sequence_end_solve(couleur attacker,
                          boolean restartenabled,
                          slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  switch (slices[op1].type)
  {
    case STLeaf:
    {
      d_leaf_solve(attacker,restartenabled,op1,alloctab());
      freetab();
      break;
    }

    default:
      assert(0);
      break;
  }
}

/* Write the key just played, then continue solving at end of sequence
 * slice to find and write the post key play (threats, variations) and
 * write the refutations (if any)
 * @param attacker attacking side (has just played key)
 * @param refutations table containing the refutations (if any)
 * @param leaf slice index
 * @param is_try true iff what we are solving is a try
 */
void d_sequence_end_write_key_solve_postkey(couleur attacker,
                                            int refutations,
                                            slice_index si,
                                            boolean is_try)
{
  slice_index const op1 = slices[si].u.composite.op1;
  switch (slices[op1].type)
  {
    case STLeaf:
      d_leaf_write_key_solve_postkey(attacker,refutations,op1,is_try);
      break;

    default:
      assert(0);
      break;
  }
}

/* Continue solving at the end of a sequence slice
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean h_sequence_end_solve(couleur side_at_move,
                             hashwhat no_succ_hash_category,
                             boolean restartenabled,
                             slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  switch (slices[op1].type)
  {
    case STLeaf:
      return h_leaf_solve(side_at_move,
                          no_succ_hash_category,
                          restartenabled,
                          op1);

    default:
      assert(0);
      return false;
  }
}

/* Continue solving series play at the end of a sequence slice
 * @param side_at_move side at the move
 * @param no_succ_hash_category hash category for storing failures
 * @param restartenabled true iff option movenum is activated
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean ser_sequence_end_solve(couleur series_side,
                               boolean restartenabled,
                               slice_index si)
{
  boolean solution_found = false;
  slice_index const op1 = slices[si].u.composite.op1;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[op1].type)
  {
    case STLeaf:
      solution_found = ser_leaf_solve(series_side,
                                      SerNoSucc,
                                      restartenabled,
                                      op1);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",solution_found);
  return solution_found;
}

/* Determine whether the attacking side wins at the end of a sequence slice
 * @param attacker attacking side
 * @param si slice identifier
 * @return true iff attacker wins
 */
boolean d_sequence_end_does_attacker_win(couleur attacker, slice_index si)
{
  boolean result = false;
  slice_index const op1 = slices[si].u.composite.op1;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  switch (slices[op1].type)
  {
    case STLeaf:
    {
      boolean const should_hash = true;
      result = d_leaf_does_attacker_win(attacker,op1,should_hash);
    }

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
  TraceValue("%d",si);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Find and write variations starting at end of sequence slice
 * @param defender attacking side
 * @param leaf slice index
 */
void d_sequence_end_solve_variations(couleur attacker, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);
  switch (slices[op1].type)
  {
    case STLeaf:
      d_leaf_solve_variations(attacker,op1);

    default:
      assert(0);
  }

  TraceFunctionExit(__func__);
}

/* Determine whether the defending side wins in 0 (its final half
 * move) in direct play.
 * @param defender defending side
 * @param si slice identifier
 */
boolean d_sequence_end_does_defender_win(couleur defender, slice_index si)
{
  boolean result = false;
  slice_index const op1 = slices[si].u.composite.op1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  switch (slices[op1].type)
  {
    case STLeaf:
      result = d_leaf_does_defender_win(defender,op1);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}
