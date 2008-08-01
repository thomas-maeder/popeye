#include "pyquodli.h"
#include "pycompos.h"
#include "pyleaf.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

static void solve_continuations_delegator(int table, slice_index operand)
{
  switch (slices[operand].type)
  {
    case STLeaf:
      d_leaf_solve_continuations(table,operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_solve_continuations(slices[operand].u.composite.length,
                                      table,
                                      operand);
      break;

    default:
      assert(0);
      break;
  }
}

/* Determine and write continuations at end of quodlibet slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of quodlibet slice
 */
void d_quodlibet_end_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  solve_continuations_delegator(table,slices[si].u.composite.op1);
  solve_continuations_delegator(table,slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

static void solve_setplay_delegator(slice_index operand)
{
  switch (slices[operand].type)
  {
    case STLeaf:
      d_leaf_solve_setplay(operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_solve_setplay(slices[operand].u.composite.length,operand);
      break;

    default:
      assert(0);
      break;
  }
}

/* Find and write defender's set play
 * @param si slice index
 */
void d_quodlibet_end_solve_setplay(slice_index si)
{
  solve_setplay_delegator(slices[si].u.composite.op1);
  solve_setplay_delegator(slices[si].u.composite.op2);
}

boolean d_quodlibet_end_solve_complete_set(Side defender, slice_index si)
{
  /* TODO */
  return false;
}

/* Determine and write solutions starting at the end of a quodlibet
 * direct/self/reflex stipulation. 
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_quodlibet_end_solve(boolean restartenabled, slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  if (d_leaf_is_unsolvable(op1))
    d_leaf_write_unsolvability(op1);
  else if (d_leaf_is_unsolvable(op2))
    d_leaf_write_unsolvability(op2);
  else
  {
    int solutions = alloctab();
    d_leaf_solve(restartenabled,op1,solutions);
    d_leaf_solve(restartenabled,op2,solutions);
    freetab();
  }
}

static void write_key_solve_postkey_delegator(int refutations,
                                              slice_index operand,
                                              boolean is_try)
{
  switch (slices[operand].type)
  {
    case STLeaf:
      d_leaf_write_key_solve_postkey(refutations,operand,is_try);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_write_key_solve_postkey(slices[operand].u.composite.length,
                                          refutations,
                                          operand,
                                          is_try);
      break;

    default:
      assert(0);
      break;
  }
}

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any), starting at the end
 * of a quodlibet slice.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param is_try true iff what we are solving is a try
 */
void d_quodlibet_end_write_key_solve_postkey(int refutations,
                                             slice_index si,
                                             boolean is_try)
{
  write_key_solve_postkey_delegator(refutations,
                                    slices[si].u.composite.op1,
                                    is_try);
  write_key_solve_postkey_delegator(refutations,
                                    slices[si].u.composite.op2,
                                    is_try);
}

extern boolean hashing_suspended; /* TODO */

static boolean does_attacker_win_delegator(slice_index operand)
{
  boolean result = false;

  switch (slices[operand].type)
  {
    case STLeaf:
      if (d_leaf_does_attacker_win(operand))
        result = true;
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      if (d_composite_does_attacker_win(slices[operand].u.composite.length,
                                        operand))
        result = true;
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Determine whether the attacker wins at the end of a quodlibet slice
 * @param si slice index
 * @param parent_is_exact true iff parent of slice si has exact length
 * @return true iff attacker wins
 */
boolean d_quodlibet_end_does_attacker_win(slice_index si)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;
  boolean result;
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  /* avoid conflict in hash table between op1 and op2 */
  /* TODO use disjoint "hash slots" to avoid this conflict while
   * hashing both in op1 and op2 */
  result = (does_attacker_win_delegator(op1)
            || does_attacker_win_delegator(op2));

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%d\n",result);
  return result;
}

static void solve_variations_delegator(int len_threat,
                                       int threats,
                                       int refutations,
                                       slice_index operand)
{
  switch (slices[operand].type)
  {
    case STLeaf:
      d_leaf_solve_variations(operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      d_composite_solve_variations(slices[operand].u.composite.length,
                                   len_threat,
                                   threats,
                                   refutations,
                                   operand);
      freetab();
      freetab();
      break;

    default:
      assert(0);
  }
}

/* Find and write variations from the end of a quodlibet slice.
 * @param len_threat length of threat (shorter variations are suppressed) 
 * @param threats table containing threats (variations not defending
 *                against all threats are suppressed)
 * @param refutations table containing refutations (written at end)
 * @param si slice index
 */
void d_quodlibet_end_solve_variations(int len_threat,
                                      int threats,
                                      int refutations,
                                      slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",len_threat);
  TraceFunctionParam("%d\n",si);

  solve_variations_delegator(len_threat,
                             threats,
                             refutations,
                             slices[si].u.composite.op1);
  solve_variations_delegator(len_threat,
                             threats,
                             refutations,
                             slices[si].u.composite.op2);

  TraceFunctionExit(__func__);
}

static d_composite_win_type does_defender_win_delegator(slice_index operand)
{
  d_composite_win_type result = win;

  switch (slices[operand].type)
  {
    case STLeaf:
      result = d_leaf_does_defender_win(operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      result = d_composite_does_defender_win(slices[operand].u.composite.length,
                                             operand);
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Determine whether the defending side wins at the end of quodlibet
 * in direct play. 
 * @param si slice identifier
 */
d_composite_win_type d_quodlibet_end_does_defender_win(slice_index si)
{
  d_composite_win_type result = win;
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  if (does_defender_win_delegator(slices[si].u.composite.op1)>=loss
      || does_defender_win_delegator(slices[si].u.composite.op2)>=loss)
    result = loss;

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean has_defender_lost_delegator(slice_index operand)
{
  boolean result = false;

  switch (slices[operand].type)
  {
    case STLeaf:
      result = d_leaf_has_defender_lost(operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      result = d_composite_has_defender_lost(operand);
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

/* Determine whether the defender has lost in direct play with his move
 * just played.
 * Assumes that there is no short win for the defending side.
 * @param si slice identifier
 * @return whether there is a short win or loss
 */
boolean d_quodlibet_end_has_defender_lost(slice_index si)
{
  boolean result = true;
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  result = (has_defender_lost_delegator(slices[si].u.composite.op1)
            || has_defender_lost_delegator(slices[si].u.composite.op2));

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean has_defender_won_delegator(slice_index operand)
{
  boolean result = false;

  switch (slices[operand].type)
  {
    case STLeaf:
      result = d_leaf_is_unsolvable(operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      result = d_composite_has_defender_won(operand);
      break;

    default:
      assert(0);
      break;
  }

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
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  result = (has_defender_won_delegator(slices[si].u.composite.op1)
            && has_defender_won_delegator(slices[si].u.composite.op2));

  hashing_suspended = save_hashing_suspended;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean has_attacker_won_delegator(slice_index operand)
{
  boolean result = false;

  switch (slices[operand].type)
  {
    case STLeaf:
      result = d_leaf_has_attacker_won(operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      result = d_composite_has_attacker_won(operand);
      break;

    default:
      assert(0);
      break;
  }

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
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;

  result = (has_attacker_won_delegator(slices[si].u.composite.op1)
            || has_attacker_won_delegator(slices[si].u.composite.op2));

  hashing_suspended = save_hashing_suspended;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static boolean has_attacker_lost_delegator(slice_index operand)
{
  boolean result = false;

  switch (slices[operand].type)
  {
    case STLeaf:
      result = d_leaf_has_attacker_lost(operand);
      break;

    case STQuodlibet:
    case STSequence:
    case STReciprocal:
      result = d_composite_has_attacker_lost(operand);
      break;

    default:
      assert(0);
      break;
  }

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
  boolean const save_hashing_suspended = hashing_suspended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",si);

  hashing_suspended = true;
  
  result = (has_attacker_lost_delegator(slices[si].u.composite.op1)
            || has_attacker_lost_delegator(slices[si].u.composite.op2));

  hashing_suspended = save_hashing_suspended;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

/* Intialize starter field with the starting side if possible, and
 * no_side otherwise. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void quodlibet_init_starter(slice_index si, boolean is_duplex)
{
  slice_index const op1 = slices[si].u.composite.op1;
  slice_index const op2 = slices[si].u.composite.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",si);
  TraceFunctionParam("%d\n",is_duplex);

  composite_init_starter(op1,is_duplex);
  composite_init_starter(op2,is_duplex);

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

  TraceValue("%d\n",slices[si].starter);
  TraceFunctionExit(__func__);
  TraceText("\n");
}
