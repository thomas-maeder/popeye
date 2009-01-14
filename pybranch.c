#include "pybranch.h"
#include "pystip.h"
#include "pyproc.h"
#include "trace.h"

#include <assert.h>

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_must_starter_resign(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_must_starter_resign(slices[si].u.branch.next);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_write_unsolvability(slice_index si)
{
  slice_write_unsolvability(slices[si].u.branch.next);
}

/* Determine and write continuations at end of branch slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void branch_end_solve_continuations(int table, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u\n",slices[si].u.branch.next);
  slice_solve_continuations(table,slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Find and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean branch_root_end_solve_setplay(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_root_solve_setplay(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write set play provided every set move leads to end
 * @param si slice index
 * @return true iff every defender's move leads to end
 */
boolean branch_root_end_solve_complete_set(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_root_end_solve_complete_set(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}


/* Write the key just played, then continue solving at end of branch
 * slice to find and write the post key play (threats, variations)
 * @param si slice index
 * @param type type of attack
 */
void branch_root_end_write_key_solve_postkey(slice_index si,
                                                 attack_type type)
{
  slice_root_write_key_solve_postkey(slices[si].u.branch.next,type);
}

/* Solve at root level at the end of a branch slice
 * @param si slice index
 */
void branch_root_end_solve(slice_index si)
{
  slice_root_solve(slices[si].u.branch.next);
}

/* Continue solving at the end of a branch slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_end_solve(slice_index si)
{
  return slice_solve(slices[si].u.branch.next);
}

/* Determine whether there is a solution at the end of a quodlibet
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_end_has_solution(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_solution(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Find and write variations starting at end of branch slice
 * @param si slice index
 */
void branch_end_solve_variations(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  slice_solve_variations(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_end_has_non_starter_solved(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  result = slice_has_non_starter_solved(slices[si].u.branch.next);

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
boolean branch_has_starter_apriori_lost(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(0); /* just to be sure; this can't possibly be right: */
  result = slice_has_starter_apriori_lost(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_has_starter_won(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  assert(0); /* just to be sure; this can't possibly be right: */
  result = slice_has_starter_won(slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void branch_detect_starter(slice_index si, boolean is_duplex)
{
  slice_index const next = slices[si].u.branch.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u\n",is_duplex);

  slice_detect_starter(next,is_duplex);

  switch (slices[next].type)
  {
    case STBranchDirect:
    case STBranchHelp:
    case STBranchSeries:
      if (slice_get_starter(next)==no_side)
        slices[si].u.branch.starter = no_side;
      else
        slices[si].u.branch.starter = advers(slice_get_starter(next));
      break;

    case STLeafDirect:
    case STLeafSelf:
    case STLeafHelp:
      if (slice_get_starter(next)==no_side)
      {
        /* next can't tell - let's tell him */
        slices[si].u.branch.starter = is_duplex ? Black : White;
        slice_impose_starter(next,slices[si].u.branch.starter);
      }
      else
        slices[si].u.branch.starter = slice_get_starter(next);
      break;

    default:
      slices[si].u.branch.starter = slice_get_starter(next);
      break;
  }

  TraceValue("%u\n",slices[si].u.branch.starter);

  TraceFunctionExit(__func__);
  TraceText("\n");
}
