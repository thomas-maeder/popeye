#include "output/plaintext/line/line_writer.h"
#include "pydata.h"
#include "pymsg.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/plaintext.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "solving/move_effect_journal.h"
#include "platform/beep.h"
#include "debugging/trace.h"

#ifdef _SE_
#include "se.h"
#endif

#include <assert.h>

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write lines in line mode.
 */

static void write_line_intro(unsigned int *next_move_number,
                             Side *numbered_side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (OptFlag[beep])
    produce_beep();

  Message(NewLine);

  TraceValue("%u\n",output_plaintext_nr_move_inversions);
  switch (output_plaintext_nr_move_inversions)
  {
    case 2:
      StdString("  1...  ...");
      *next_move_number = 2;
      *numbered_side = trait[nbply];
      break;

    case 1:
      StdString("  1...");
      *next_move_number = 2;
      *numbered_side = advers(trait[nbply]);
      break;

    case 0:
      *next_move_number = 1;
      *numbered_side = trait[nbply];
      break;

    default:
      assert(0);
      break;
  }

  TraceValue("%u",*next_move_number);
  TraceValue("%u",nbply);
  TraceEnumerator(Side,trait[nbply],"");
  TraceEnumerator(Side,*numbered_side,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_move_number_if_necessary(unsigned int *next_move_number,
                                           Side const *numbered_side)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",*next_move_number);
  TraceEnumerator(Side,*numbered_side,"");
  TraceFunctionParamListEnd();

  if (trait[nbply]==*numbered_side)
  {
    sprintf(GlobalStr,"%3u.",*next_move_number);
    StdString(GlobalStr);

    ++*next_move_number;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_potential_check(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,trait[nbply],"\n");

  if (echecc(advers(trait[nbply])))
    StdString(" +");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_ply_history(unsigned int *next_move_number,
                              Side *numbered_side)
{
  ply const start_ply = 2;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply==start_ply)
    write_line_intro(next_move_number,numbered_side);
  else
  {
    ply const child_nbply = nbply;

    undo_move_effects();

    TraceValue("%u",nbply);
    TraceValue("%u\n",parent_ply[nbply]);
    nbply = parent_ply[nbply];

    write_ply_history(next_move_number,numbered_side);

    if (encore())
    {
      /* not a dummy move ply */
      write_move_number_if_necessary(next_move_number,numbered_side);
      output_plaintext_write_move();
      write_potential_check();
      StdChar(blank);
    }

    if (is_end_of_intro_series[nbply])
    {
      *next_move_number = 1;
      *numbered_side = trait[child_nbply];
    }

    nbply = child_nbply;

    redo_move_effects();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write a move
 * @param goal goal reached by that line
 */
void output_plaintext_line_write_line(goal_type goal)
{
  unsigned int next_movenumber = 0;
  Side numbered_side = no_side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  write_ply_history(&next_movenumber,&numbered_side);

  write_move_number_if_necessary(&next_movenumber,&numbered_side);
  output_plaintext_write_move();
  if (!output_plaintext_goal_writer_replaces_check_writer(goal))
    write_potential_check();

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
  slices[result].u.goal_handler.goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type output_plaintext_line_line_writer_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

#ifdef _SE_DECORATE_SOLUTION_
  se_start_pos();
#endif

  output_plaintext_line_write_line(slices[si].u.goal_handler.goal.type);
  result = solve(slices[si].next1,n);

#ifdef _SE_DECORATE_SOLUTION_
  se_end_pos();
#endif
#ifdef _SE_FORSYTH_
  se_forsyth();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
