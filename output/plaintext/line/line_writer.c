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

typedef struct
{
    Side side;
    int next_movenumber;
} write_line_status_type;

static void write_line_intro(write_line_status_type *status)
{
  if (OptFlag[beep])
    produce_beep();

  Message(NewLine);

  TraceValue("%u\n",output_plaintext_nr_move_inversions);
  switch (output_plaintext_nr_move_inversions)
  {
    case 2:
      StdString("  1...  ...");
      status->next_movenumber = 2;
      break;

    case 1:
      StdString("  1...");
      status->next_movenumber = 2;
      break;

    case 0:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }
}

static void write_next_move(ply ply, write_line_status_type *status)
{
  TraceEnumerator(Side,status->side," ");
  TraceValue("%u",ply);
  TraceEnumerator(Side,trait[ply],"\n");

  initneutre(advers(trait[ply]));
  jouecoup(ply,replay);

  if (trait[ply]==status->side)
  {
    sprintf(GlobalStr,"%3d.",status->next_movenumber);
    ++status->next_movenumber;
    StdString(GlobalStr);
  }
  output_plaintext_write_move(ply);

  if (echecc(ply,advers(trait[ply])))
    StdString(" +");
  StdChar(blank);
}

static void write_last_move(goal_type goal, write_line_status_type const *status)
{
  initneutre(advers(trait[nbply]));
  jouecoup(nbply,replay);

  if (trait[nbply]==status->side)
  {
    sprintf(GlobalStr,"%3d.",status->next_movenumber);
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

static unsigned int init_ply_history(ply ply_history[maxply])
{
  ply const start_ply = 2;
  unsigned int result = 0;
  int current_ply = nbply;
  while (current_ply!=start_ply)
  {
    current_ply = parent_ply[current_ply];
    if (repere[current_ply+1]>repere[current_ply])
    {
      ply_history[result] = current_ply;
      ++result;
    }
  }

  return result;
}

static void write_ply_history(ply const ply_history[maxply], unsigned int length,
                              write_line_status_type *status)
{
  ply const start_ply = 2;
  unsigned int history_pos = length;

  while (history_pos>0)
  {
    int const current_ply = ply_history[--history_pos];
    if (current_ply>start_ply && is_end_of_intro_series[current_ply-1])
    {
      status->next_movenumber = 1;
      status->side = trait[current_ply];
    }

    write_next_move(current_ply,status);
  }
}

/* identifies a slice whose starter is the nominal starter of the stipulation
 * before any move inversions are applied
 * (e.g. in a h#N.5, this slice's starter is Black)
 */
slice_index output_plaintext_slice_determining_starter = no_slice;

static void write_line(Side starting_side, goal_type goal)
{
  write_line_status_type status = { starting_side, 1 };

  ply ply_history[maxply];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starting_side,"");
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  ResetPosition();

#ifdef _SE_DECORATE_SOLUTION_
  se_start_pos();
#endif

  write_line_intro(&status);
  write_ply_history(ply_history,init_ply_history(ply_history),&status);
  write_last_move(goal,&status);

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
