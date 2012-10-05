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
#include "pieces/attributes/neutral/initialiser.h"
#include "platform/beep.h"
#include "debugging/trace.h"

#ifdef _SE_
#include "se.h"
#endif

#include <assert.h>

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write lines in line mode.
 */

/* identifies a slice whose starter is the nominal starter of the stipulation
 * before any move inversions are applied
 * (e.g. in a h#N.5, this slice's starter is Black)
 */
slice_index output_plaintext_slice_determining_starter = no_slice;

static void write_line_intro(unsigned int *next_move_number)
{
  if (OptFlag[beep])
    produce_beep();

  Message(NewLine);

  TraceValue("%u\n",output_plaintext_nr_move_inversions);
  switch (output_plaintext_nr_move_inversions)
  {
    case 2:
      StdString("  1...  ...");
      *next_move_number = 2;
      break;

    case 1:
      StdString("  1...");
      *next_move_number = 2;
      break;

    case 0:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }
}

static void write_move_number_if_necessary(unsigned int *next_move_number,
                                           Side const *side_to_move)
{
  if (trait[nbply]==*side_to_move)
  {
    sprintf(GlobalStr,"%3u.",*next_move_number);
    StdString(GlobalStr);

    ++*next_move_number;
  }
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
                              Side *side_to_move)
{
  ply const start_ply = 2;
  ply const child_nbply = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u\n",parent_ply[nbply]);
  nbply = parent_ply[nbply];

  if (nbply>=start_ply)
  {
    if (encore())
    {
      undo_move_effects();
      neutral_initialiser_recolor_retracting();

      write_ply_history(next_move_number,side_to_move);

      redo_move_effects();
      neutral_initialiser_recolor_replaying();

      write_move_number_if_necessary(next_move_number,side_to_move);
      output_plaintext_write_move();
      write_potential_check();
      StdChar(blank);
    }
    else
      /* dummy move ply */
      write_ply_history(next_move_number,side_to_move);
  }

  if (is_end_of_intro_series[nbply])
  {
    *next_move_number = 1;
    *side_to_move = trait[child_nbply];
  }

  nbply = child_nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_line(slice_index si, Side side_to_move)
{
  goal_type const type = slices[si].u.goal_handler.goal.type;
  unsigned int next_movenumber = 1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_to_move,"");
  TraceFunctionParamListEnd();

#ifdef _SE_DECORATE_SOLUTION_
  se_start_pos();
#endif

  write_line_intro(&next_movenumber);

  undo_move_effects();
  neutral_initialiser_recolor_retracting();

  write_ply_history(&next_movenumber,&side_to_move);

  redo_move_effects();
  neutral_initialiser_recolor_replaying();

  write_move_number_if_necessary(&next_movenumber,&side_to_move);
  output_plaintext_write_move();
  if (!output_plaintext_goal_writer_replaces_check_writer(type))
    write_potential_check();
  if (type!=no_goal)
    StdString(goal_end_marker[type]);
  StdChar(blank);

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
  slices[result].u.goal_handler.goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type output_plaintext_line_line_writer_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(slices[si].next1,n);

  if (slack_length<=result && result<=n)
  {
    Side initial_starter = slices[output_plaintext_slice_determining_starter].starter;
    if (areColorsSwapped)
      initial_starter = advers(initial_starter);
    TraceValue("%u\n",output_plaintext_slice_determining_starter);
    write_line(si,initial_starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type output_plaintext_line_line_writer_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(slices[si].next1,n);

  if (result<=n+2)
  {
    Side initial_starter = slices[output_plaintext_slice_determining_starter].starter;
    if (areColorsSwapped)
      initial_starter = advers(initial_starter);
    TraceValue("%u\n",output_plaintext_slice_determining_starter);
    write_line(si,initial_starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
