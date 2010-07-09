#include "output/plaintext/line/line_writer.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"
#include "optimisations/maxsolutions/maxsolutions.h"
#include "pyint.h"
#include "pymsg.h"
#include "output/output.h"
#ifdef _SE_
#include "se.h"
#endif

#include <assert.h>

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write lines in line mode.
 */

void write_line(goal_type goal)
{
  int next_movenumber = 1;
  Side starting_side;
  slice_index slice;
  ply current_ply;

  ply const start_ply = 2;

  if (isIntelligentModeActive)
  {
    if (SolAlreadyFound())
      return;
    else
    {
      increase_nr_found_solutions();
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
    }
    StoreSol();
  }
  else
  {
    increase_nr_found_solutions();
    if (OptFlag[beep])
      BeepOnSolution(maxbeep);
  }
      
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  Message(NewLine);

  slice = active_slice[start_ply];
  starting_side = slices[root_slice].starter;

  ResetPosition();

  TraceValue("%u\n",nr_color_inversions_in_ply[start_ply]);

  switch (nr_color_inversions_in_ply[start_ply])
  {
    case 2:
      StdString("  1...  ...");
      next_movenumber = 2;
      break;

    case 1:
      StdString("  1...");
      next_movenumber = 2;
      break;

    case 0:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_start_pos();
#endif

  TraceValue("%u\n",nbply);
  for (current_ply = start_ply; current_ply<=nbply; ++current_ply)
  {
    TraceValue("%u",current_ply);
    TraceValue("%u",slice);
    TraceValue("%u\n",active_slice[current_ply]);
    if (slice!=active_slice[current_ply])
    {
      if (slices[slice].type==STSeriesMove
          && slices[active_slice[current_ply]].type==STSeriesMove
          && trait[current_ply-1]!=trait[current_ply])
      {
        next_movenumber = 1;
        starting_side = trait[current_ply];
      }

      slice = active_slice[current_ply];
    }

    TraceEnumerator(Side,starting_side," ");
    TraceEnumerator(Side,trait[current_ply],"\n");
    if (trait[current_ply]==starting_side)
    {
      sprintf(GlobalStr,"%3d.",next_movenumber);
      ++next_movenumber;
      StdString(GlobalStr);
    }

    initneutre(advers(trait[current_ply]));
    jouecoup_no_test(current_ply);
    ecritcoup(current_ply);
    if (nbply==current_ply)
      StdString(goal_end_marker[goal]);
    else if (echecc(current_ply,advers(trait[current_ply])))
      StdString(" +");
    StdChar(blank);
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_end_pos();
#endif
#ifdef _SE_FORSYTH_
  se_forsyth();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STOutputPlaintextLineLineWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_line_writer_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlaintextLineLineWriter);
  slices[result].u.goal_reached_tester.goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter 
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type line_writer_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_has_solution(slices[si].u.goal_reached_tester.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type line_writer_solve(slice_index si)
{
  has_solution_type result;
  Goal const goal = slices[si].u.goal_reached_tester.goal;
  slice_index const next = slices[si].u.goal_reached_tester.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_solve(next);
  if (result==has_solution)
    write_line(goal.type);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
