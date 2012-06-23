#include "output/plaintext/line/line_writer.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "pydata.h"
#include "stipulation/fork.h"
#include "debugging/trace.h"
#include "pymsg.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/plaintext.h"
#include "stipulation/has_solution_type.h"
#include "platform/beep.h"
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

static void write_line(Side starting_side, goal_type goal)
{
  int next_movenumber = 1;
  ply current_ply;
  ply history[maxply];
  unsigned int history_pos = 0;

  ply const start_ply = 2;

  boolean const save_jouetestgenre = jouetestgenre;
  jouetestgenre = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starting_side,"");
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  if (OptFlag[beep])
    produce_beep();

  Message(NewLine);

  ResetPosition();

  current_ply = nbply;
  while (current_ply!=start_ply)
  {
    current_ply = parent_ply[current_ply];
    if (repere[current_ply+1]>repere[current_ply])
    {
      history[history_pos] = current_ply;
      ++history_pos;
    }
  }

  TraceValue("%u\n",output_plaintext_nr_move_inversions);
  switch (output_plaintext_nr_move_inversions)
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
    initneutre(advers(trait[current_ply]));
    jouecoup(current_ply,replay);

    if (trait[current_ply]==starting_side)
    {
      sprintf(GlobalStr,"%3d.",next_movenumber);
      ++next_movenumber;
      StdString(GlobalStr);
    }
    output_plaintext_write_move(current_ply);

    if (echecc(current_ply,advers(trait[current_ply])))
      StdString(" +");
    StdChar(blank);
  }

  {
    initneutre(advers(trait[nbply]));
    jouecoup(nbply,replay);

    if (trait[nbply]==starting_side)
    {
      sprintf(GlobalStr,"%3d.",next_movenumber);
      ++next_movenumber;
      StdString(GlobalStr);
    }
    output_plaintext_write_move(nbply);

    if (!output_plaintext_goal_writer_replaces_check_writer(goal)
        && echecc(nbply,advers(trait[nbply])))
      StdString(" +");
    if (goal!=no_goal)
      StdString(goal_end_marker[goal]);
    StdChar(blank);
  }

  jouetestgenre = save_jouetestgenre;

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
stip_length_type line_writer_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = attack(slices[si].next1,n);

  if (slack_length<=result && result<=n)
  {
    Goal const goal = slices[si].u.goal_handler.goal;
    Side initial_starter = slices[output_plaintext_slice_determining_starter].starter;
    if (areColorsSwapped)
      initial_starter = advers(initial_starter);
    TraceValue("%u\n",output_plaintext_slice_determining_starter);
    write_line(initial_starter,goal.type);
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
stip_length_type line_writer_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = defend(slices[si].next1,n);

  if (result<=n+2)
  {
    Goal const goal = slices[si].u.goal_handler.goal;
    Side initial_starter = slices[output_plaintext_slice_determining_starter].starter;
    if (areColorsSwapped)
      initial_starter = advers(initial_starter);
    TraceValue("%u\n",output_plaintext_slice_determining_starter);
    write_line(initial_starter,goal.type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
