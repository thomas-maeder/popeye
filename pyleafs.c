#include "pyleafs.h"
#include "trace.h"
#include "pydata.h"
#include "platform/maxtime.h"
#include "pyint.h"
#include "pyoutput.h"
#include "pyleaf.h"
#include "pyleaff.h"
#include "pyhash.h"

#include <assert.h>
#include <stdlib.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf leaf's slice index
 * @return true iff starter must resign
 */
boolean leaf_s_must_starter_resign(slice_index leaf)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = slice_must_starter_resign(slices[leaf].u.leafself.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether there is a solution in a self leaf.
 * @param leaf slice index of leaf slice
 * @return true iff attacker wins
 */
boolean leaf_s_has_solution(slice_index leaf)
{
  boolean solution_found = false;
  Side const attacker = slices[leaf].u.leafself.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  /* It is more likely that a position has no solution. */
  /*    Therefore let's check for "no solution" first. TLi */
  if (inhash(leaf,DirNoSucc,1))
    assert(!inhash(leaf,DirSucc,0));
  else if (inhash(leaf,DirSucc,0))
    solution_found = true;
  else
  {
    genmove(attacker);

    while (!solution_found && encore())
    {
      if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
          && !echecc(nbply,attacker)
          && !leaf_forced_does_defender_win(slices[leaf].u.leafself.next))
      {
        solution_found = true;
        coupfort();
      }

      repcoup();

      if (maxtime_status==MAXTIME_TIMEOUT)
        break;
    }

    finply();

    if (solution_found)
      addtohash(leaf,DirSucc,0);
    else
      addtohash(leaf,DirNoSucc,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",solution_found);
  return solution_found;
}

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_s_has_non_starter_solved(slice_index leaf)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = leaf_forced_has_non_starter_solved(slices[leaf].u.leafself.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine and write solutions in a self stipulation in 1 move
 * @param leaf slice index of the leaf slice
 * @return true iff >=1 key was found and written
 */
boolean leaf_s_solve(slice_index leaf)
{
  boolean found_solution = false;
  Side const attacker = slices[leaf].u.leafself.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker)
        && !leaf_forced_does_defender_win(slices[leaf].u.leafself.next))
    {
      found_solution = true;

      write_attack(attack_key);
      leaf_forced_solve_variations(slices[leaf].u.leafself.next);
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",found_solution);
  return found_solution;
}

/* Determine and write the solution of a leaf slice at root level.
 * @param leaf identifies leaf slice
 */
void leaf_s_root_solve(slice_index leaf)
{
  Side const attacker = slices[leaf].u.leafself.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  output_start_continuation_level();

  active_slice[nbply+1] = leaf;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker)
        && !leaf_forced_does_defender_win(slices[leaf].u.leafself.next))
    {
      write_attack(attack_key);
      leaf_forced_solve_variations(slices[leaf].u.leafself.next);
    }

    repcoup();
  }

  finply();

  output_end_continuation_level();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Write the key and solve the remainder of a leaf in direct play
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_s_root_write_key(slice_index leaf, attack_type type)
{
  write_attack(type);
}

/* Solve the post key play
 * @param refutations table containing the refutations (if any)
 * @param leaf slice index
 */
void leaf_s_root_solve_postkey(table refutations, slice_index leaf)
{
  if (OptFlag[solvariantes])
    leaf_forced_solve_variations(slices[leaf].u.leafself.next);
}

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_s_has_starter_apriori_lost(slice_index leaf)
{
  boolean result = false;
  slice_index const next = slices[leaf].u.leafself.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = leaf_forced_has_starter_apriori_lost(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_s_has_starter_won(slice_index leaf)
{
  boolean result;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = leaf_forced_has_starter_won(slices[leaf].u.leafself.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param leaf slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_s_has_starter_reached_goal(slice_index leaf)
{
  boolean result;
  slice_index const next = slices[leaf].u.leafself.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = leaf_forced_has_starter_reached_goal(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Spin off a set play slice
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index leaf_s_root_make_setplay_slice(slice_index leaf)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  if (slices[leaf].u.leafself.goal==goal_target)
    result = alloc_target_leaf_slice(STLeafForced,
                                     slices[leaf].u.leafself.target);
  else
    result = alloc_leaf_slice(STLeafForced,slices[leaf].u.leafself.goal);

  slices[result].u.leafself.starter =
      advers(slices[leaf].u.leafself.starter);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write post key play
 * @param leaf slice index
 */
void leaf_s_solve_postkey(slice_index leaf)
{
  leaf_forced_solve_variations(slices[leaf].u.leafself.next);
}

/* Find and write continuations and append them to the top table
 * @param leaf slice index
 */
void leaf_s_solve_continuations(slice_index leaf)
{
  Side const attacker = slices[leaf].u.leafself.starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  active_slice[nbply+1] = leaf;
  genmove(attacker);

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,attacker)
        && !leaf_forced_does_defender_win(slices[leaf].u.leafself.next))
    {
      write_attack(attack_regular);
      leaf_forced_solve_variations(slices[leaf].u.leafself.next);
      append_to_top_table();
      coupfort();
    }

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_s_detect_starter(slice_index leaf,
                                             boolean same_side_as_root)
{
  slice_index const next = slices[leaf].u.leafself.next;
  who_decides_on_starter result = dont_know_who_decides_on_starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",leaf);

  result = slice_detect_starter(next,same_side_as_root);
  slices[leaf].u.leafself.starter = advers(slice_get_starter(next));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Impose the starting side on a leaf. 
 * @param leaf identifies leaf
 * @param s starting side of leaf
 */
void leaf_s_impose_starter(slice_index leaf, Side s)
{
  slices[leaf].u.leafself.starter = s;
  slice_impose_starter(slices[leaf].u.leafself.next,advers(s));
}

/* Write a move by the non-starter that has reached a leaf's goal
 * @param leaf slice index of leaf
 */
void leaf_s_write_non_starter_has_solved(slice_index leaf)
{
  leaf_forced_write_non_starter_has_solved(slices[leaf].u.leafself.next);
}
