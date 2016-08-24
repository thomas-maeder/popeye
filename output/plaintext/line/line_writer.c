#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/message.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/check.h"
#include "solving/move_generator.h"
#include "platform/beep.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "options/options.h"

#ifdef _SE_
#include "se.h"
#endif

#include "debugging/assert.h"

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write lines in line mode.
 */

static void write_line_intro(unsigned int *next_move_number,
                             Side *numbered_side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (OptFlag[beep])
    platform_produce_beep();

  output_plaintext_message(NewLine);

  TraceValue("%u",output_plaintext_nr_move_inversions);
  TraceEOL();
  switch (output_plaintext_nr_move_inversions)
  {
    case 2:
      protocol_fprintf(stdout,"%s","  1...  ...");
      *next_move_number = 2;
      *numbered_side = trait[nbply];
      break;

    case 1:
      protocol_fprintf(stdout,"%s","  1...");
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
    protocol_fprintf(stdout,"%3u.",*next_move_number);
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

  if (is_in_check(advers(trait[nbply])))
    protocol_fprintf(stdout,"%s"," +");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_ply_history(unsigned int *next_move_number,
                              Side *numbered_side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (parent_ply[nbply]==ply_retro_move)
    write_line_intro(next_move_number,numbered_side);
  else
  {
    ply const child_nbply = nbply;

    /* TODO undoing and redoing is currently necessary for:
     * - detecting whether a move has delivered check
     * - correctly outputting iterator movements
     */
    undo_move_effects();

    nbply = parent_ply[nbply];

    write_ply_history(next_move_number,numbered_side);

    if (encore())
    {
      /* not a dummy move ply */
      write_move_number_if_necessary(next_move_number,numbered_side);
      output_plaintext_write_move(&output_plaintext_engine,
                                  stdout,
                                  &output_plaintext_symbol_table);
      write_potential_check();
      protocol_fputc(' ',stdout);
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
  output_plaintext_write_move(&output_plaintext_engine,
                              stdout,
                              &output_plaintext_symbol_table);
  if (!output_goal_preempts_check(goal))
    write_potential_check();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STOutputPlaintextLineLineWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_line_writer_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputPlaintextLineLineWriter);
  SLICE_U(result).goal_handler.goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void output_plaintext_line_line_writer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

#ifdef _SE_DECORATE_SOLUTION_
  se_start_pos();
#endif

  output_plaintext_line_write_line(SLICE_U(si).goal_handler.goal.type);

  pipe_solve_delegate(si);

#ifdef _SE_DECORATE_SOLUTION_
  se_end_pos();
#endif
#ifdef _SE_FORSYTH_
  se_forsyth();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
