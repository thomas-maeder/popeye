#include "output/plaintext/line/line_writer.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyoutput.h"
#include "trace.h"
#include "pymsg.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/line/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/plaintext.h"
#include "platform/beep.h"
#ifdef _SE_
#include "se.h"
#endif

#include <assert.h>

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write lines in line mode.
 */

static void write_line(Side starting_side, goal_type goal)
{
  int next_movenumber = 1;
  ply current_ply;
  ply history[maxply];
  unsigned int history_pos = 0;

  ply const start_ply = 2;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starting_side,"");
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  if (OptFlag[beep])
    produce_beep();

  Message(NewLine);

  ResetPosition();

  history[history_pos] = nbply;
  current_ply = nbply;
  ++history_pos;
  while (current_ply!=start_ply)
  {
    current_ply = parent_ply[current_ply];
    if (repere[current_ply+1]>repere[current_ply])
    {
      history[history_pos] = current_ply;
      ++history_pos;
    }
  }

  TraceValue("%u\n",
             output_plaintext_line_nr_move_inversions_in_ply[start_ply]);

  switch (output_plaintext_line_nr_move_inversions_in_ply[start_ply])
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

  TraceValue("%u",start_ply);
  TraceValue("%u\n",nbply);
  while (history_pos>0)
  {
    --history_pos;
    current_ply = history[history_pos];
    if (current_ply>start_ply && is_end_of_intro_series[current_ply-1])
    {
      next_movenumber = 1;
      starting_side = trait[current_ply];
    }

    TraceEnumerator(Side,starting_side," ");
    TraceValue("%u",current_ply);
    TraceEnumerator(Side,trait[current_ply],"\n");
    if (trait[current_ply]==starting_side)
    {
      sprintf(GlobalStr,"%3d.",next_movenumber);
      ++next_movenumber;
      StdString(GlobalStr);
    }

    initneutre(advers(trait[current_ply]));
    jouecoup_no_test(current_ply);
    output_plaintext_write_move(current_ply);
    if (current_ply==nbply)
    {
      if (!output_plaintext_goal_writer_replaces_check_writer(goal)
          && echecc(current_ply,advers(trait[current_ply])))
        StdString(" +");
      StdString(goal_end_marker[goal]);
    }
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
 * @param root_slice slice at the start of the solution
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_line_writer_slice(slice_index root_slice, Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlaintextLineLineWriter);
  slices[result].u.line_writer.goal = goal;
  slices[result].u.line_writer.root_slice = root_slice;

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

  result = slice_has_solution(slices[si].u.line_writer.next);

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
  slice_index const next = slices[si].u.line_writer.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_solve(next);

  if (result==has_solution)
  {
    Goal const goal = slices[si].u.line_writer.goal;
    slice_index const root_slice = slices[si].u.line_writer.root_slice;
    Side initial_starter = slices[root_slice].starter;
    if (areColorsSwapped)
      initial_starter = advers(initial_starter);
    write_line(initial_starter,goal.type);
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
