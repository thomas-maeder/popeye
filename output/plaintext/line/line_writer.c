#include "output/plaintext/line/line_writer.h"
#include "stipulation/stipulation.h"
#include "stipulation/boolean/true.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/move_player.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/discriminate_by_right_to_move.h"
#include "stipulation/help_play/adapter.h"
#include "solving/fork_on_remaining.h"
#include "pydata.h"
#include "debugging/trace.h"
#include "pymsg.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/plaintext.h"
#include "pieces/attributes/neutral/initialiser.h"
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
    ply ply_history[maxply];
    unsigned int length;
} write_line_status_type;

static write_line_status_type write_line_status;

static void write_line_intro(void)
{
  if (OptFlag[beep])
    produce_beep();

  Message(NewLine);

  TraceValue("%u\n",output_plaintext_nr_move_inversions);
  switch (output_plaintext_nr_move_inversions)
  {
    case 2:
      StdString("  1...  ...");
      write_line_status.next_movenumber = 2;
      break;

    case 1:
      StdString("  1...");
      write_line_status.next_movenumber = 2;
      break;

    case 0:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }
}

static void init_ply_history(void)
{
  ply const start_ply = 2;
  int current_ply = nbply;
  write_line_status.length = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (current_ply!=start_ply)
  {
    current_ply = parent_ply[current_ply];
    if (current_move[current_ply]>current_move[current_ply-1])
    {
      write_line_status.ply_history[write_line_status.length] = current_ply;
      ++write_line_status.length;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_ply_history(slice_index si)
{
  ply const start_ply = 2;
  unsigned int history_pos = write_line_status.length;
  ply const save_nbply = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while (history_pos>0)
  {
    nbply = write_line_status.ply_history[--history_pos];
    if (nbply>start_ply && is_end_of_intro_series[nbply-1])
    {
      write_line_status.next_movenumber = 1;
      write_line_status.side = trait[nbply];
    }

    attack(slices[si].next2,length_unspecified+1);
  }

  nbply = save_nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* identifies a slice whose starter is the nominal starter of the stipulation
 * before any move inversions are applied
 * (e.g. in a h#N.5, this slice's starter is Black)
 */
slice_index output_plaintext_slice_determining_starter = no_slice;

static void write_line(slice_index si, Side starting_side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,starting_side,"");
  TraceFunctionParamListEnd();

  ResetPosition();

#ifdef _SE_DECORATE_SOLUTION_
  se_start_pos();
#endif

  write_line_status.next_movenumber = 1;
  write_line_status.side = starting_side;
  write_line_intro();
  init_ply_history();
  write_ply_history(si);
  attack(slices[si].next2,length_unspecified);

#ifdef _SE_DECORATE_SOLUTION_
  se_end_pos();
#endif
#ifdef _SE_FORSYTH_
  se_forsyth();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index alloc_writers_for_one_side(Goal goal)
{
  slice_index const result = alloc_proxy_slice();
  slice_index const replaying = alloc_pipe(STReplayingMoves);
  slice_index const replayer = alloc_move_replayer_slice();
  slice_index const landing = alloc_pipe(STLandingAfterMovePlay);
  slice_index const proxyIntermediate = alloc_proxy_slice();
  slice_index const proxyLast = alloc_proxy_slice();
  slice_index const fork = alloc_fork_on_remaining_slice(proxyIntermediate,proxyLast,0);
  slice_index const writerIntermediate = alloc_pipe(STOutputPlaintextLineIntermediateMoveWriter);
  slice_index const trueIntermediate = alloc_true_slice();
  slice_index const writerLast = alloc_pipe(STOutputPlaintextLineLastMoveWriter);
  slice_index const trueLast = alloc_true_slice();

  pipe_link(result,replaying);
  pipe_link(replaying,replayer);
  pipe_link(replayer,landing);
  pipe_link(landing,fork);
  pipe_link(proxyIntermediate,writerIntermediate);
  pipe_link(writerIntermediate,trueIntermediate);
  pipe_link(proxyLast,writerLast);
  slices[writerLast].u.goal_handler.goal = goal;
  pipe_link(writerLast,trueLast);

  return result;
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

  {
    slice_index const adapter = alloc_help_adapter_slice(slack_length,slack_length);

    slice_index const writersWhite = alloc_writers_for_one_side(goal);
    slice_index const writersBlack = alloc_writers_for_one_side(goal);
    slice_index const discriminate = alloc_discriminate_by_right_to_move_slice(writersWhite,writersBlack);

    result = alloc_pipe(STOutputPlaintextLineLineWriter);
    slices[result].next2 = adapter;
    pipe_link(adapter,discriminate);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void write_move_number_if_necessary(slice_index si)
{
  Side const just_moved = slices[si].starter;
  if (just_moved==write_line_status.side)
  {
    sprintf(GlobalStr,"%3d.",write_line_status.next_movenumber);
    ++write_line_status.next_movenumber;
    StdString(GlobalStr);
  }
}

static void write_potential_check(slice_index si)
{
  Side const just_moved = slices[si].starter;
  Side const potentially_in_check = advers(just_moved);
  if (echecc(potentially_in_check))
    StdString(" +");
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type output_plaintext_line_intermediate_move_writer_attack(slice_index si,
                                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  write_move_number_if_necessary(si);
  output_plaintext_write_move(nbply);
  write_potential_check(si);
  StdChar(blank);

  result = attack(slices[si].next1,n);

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
stip_length_type output_plaintext_line_last_move_writer_attack(slice_index si,
                                                               stip_length_type n)
{
  stip_length_type result;
  goal_type const goal_type = slices[si].u.goal_handler.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  write_move_number_if_necessary(si);
  output_plaintext_write_move(nbply);
  if (!output_plaintext_goal_writer_replaces_check_writer(goal_type))
    write_potential_check(si);
  if (goal_type!=no_goal)
    StdString(goal_end_marker[goal_type]);
  StdChar(blank);

  result = attack(slices[si].next1,n);

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
