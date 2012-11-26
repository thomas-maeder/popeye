#include "optimisations/killer_move/final_defense_move.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/binary.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "solving/single_piece_move_generator.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Optimise a STMoveGenerator slice for defending against a goal
 * @param si identifies slice to be optimised
 */
void killer_move_optimise_final_defense_move(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
    slice_index const proxy3 = alloc_proxy_slice();
    slice_index const proxy4 = alloc_proxy_slice();
    slice_index const proxy5 = alloc_proxy_slice();
    slice_index const prioriser = alloc_pipe(STKillerMovePrioriser);
    slice_index const generator1 = alloc_single_piece_move_generator_slice();
    slice_index const generator2 = alloc_single_piece_move_generator_slice();
    slice_index const last_defense = alloc_binary_slice(STKillerMoveFinalDefenseMove,proxy3,proxy4);
    pipe_link(slices[si].prev,fork);
    pipe_link(proxy3,generator1);
    pipe_link(generator1,proxy5);
    pipe_link(proxy5,slices[si].next1);
    pipe_link(proxy1,si);
    pipe_link(proxy2,last_defense);
    pipe_link(proxy4,generator2);
    pipe_link(generator2,prioriser);
    pipe_link(prioriser,proxy5);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try defenses by pieces other than the killer piece
 * @param si slice index
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
static stip_length_type defend_with_non_killer_pieces(slice_index si)
{
  Side const defender = slices[si].starter;
  square const killer_pos = kpilcd[nbply+1];
  square const *bnp;
  stip_length_type result = slack_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; result<=slack_length+1 && *bnp!=initsquare; ++bnp)
    if (*bnp!=killer_pos && TSTFLAG(spec[*bnp],defender))
    {
      stip_length_type result_next_non_killer;
      init_single_piece_move_generator(*bnp);
      result_next_non_killer = solve(slices[si].next1,slack_length+1);
      if (result_next_non_killer>result)
        result = result_next_non_killer;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try defenses first by the killer piece, then by the other pieces
 * @param si slice index
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
static stip_length_type defend_with_killer_piece(slice_index si)
{
  Side const defender = slices[si].starter;
  square const killer_pos = kpilcd[nbply+1];
  stip_length_type result = slack_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[killer_pos],defender))
  {
    init_single_piece_move_generator(killer_pos);
    result = solve(slices[si].next2,slack_length+1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type killer_move_final_defense_move_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result = slack_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length+1);

  result = defend_with_killer_piece(si);

  if (result<=slack_length+1)
  {
    stip_length_type const result_non_killers = defend_with_non_killer_pieces(si);
    if (result_non_killers>result)
      result = result_non_killers;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
