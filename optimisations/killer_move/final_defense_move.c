#include "optimisations/killer_move/final_defense_move.h"
#include "stipulation/pipe.h"
#include "stipulation/binary.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "solving/single_piece_move_generator.h"
#include "optimisations/killer_move/prioriser.h"
#include "optimisations/killer_move/killer_move.h"
#include "debugging/trace.h"

#include <assert.h>

static void stop_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*copies)[si] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Optimise a STMoveGenerator slice for defending against a goal
 * @param si identifies slice to be optimised
 * @param st structure holding traversal that led to the generator to be
 *        optimised
 */
void killer_move_optimise_final_defense_move(slice_index si,
                                             stip_structure_traversal *st)
{
  stip_structure_traversal st_nested;
  stip_deep_copies_type copies1;
  stip_deep_copies_type copies2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_deep_copy(&st_nested,st,&copies1);
  stip_structure_traversal_override_single(&st_nested,
                                           STDonePriorisingMoves,
                                           &stop_copying);
  stip_traverse_structure(slices[si].next1,&st_nested);

  init_deep_copy(&st_nested,st,&copies2);
  stip_structure_traversal_override_single(&st_nested,
                                           STDonePriorisingMoves,
                                           &stop_copying);
  stip_traverse_structure(slices[si].next1,&st_nested);

  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
    slice_index const proxy3 = alloc_proxy_slice();
    slice_index const proxy4 = alloc_proxy_slice();
    slice_index const last_defense = alloc_binary_slice(STKillerMoveFinalDefenseMove,proxy3,proxy4);
    slice_index const prototype1 = alloc_single_piece_move_generator_slice();
    slice_index const prototypes2[] =
    {
        alloc_single_piece_move_generator_slice(),
        alloc_killer_move_prioriser_slice()
    };

    pipe_link(slices[si].prev,fork);
    pipe_link(proxy1,si);
    pipe_link(proxy2,last_defense);
    pipe_link(proxy3,copies1[slices[si].next1]);
    pipe_link(proxy4,copies2[slices[si].next1]);
    defense_branch_insert_slices_behind_proxy(proxy3,&prototype1,1,last_defense);
    defense_branch_insert_slices_behind_proxy(proxy4,prototypes2,2,last_defense);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try defenses by pieces other than the killer piece
 * @param si slice index
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
static stip_length_type defend_with_non_killer_pieces(slice_index si)
{
  Side const defender = slices[si].starter;
  square const killer_pos = killer_moves[nbply+1].departure;
  square const *bnp;
  stip_length_type result = immobility_on_next_move;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; result<=next_move_has_solution && *bnp!=initsquare; ++bnp)
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
 *            previous_move_is_illegal the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
static stip_length_type defend_with_killer_piece(slice_index si)
{
  Side const defender = slices[si].starter;
  square const killer_pos = killer_moves[nbply+1].departure;
  stip_length_type result = immobility_on_next_move;

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
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type killer_move_final_defense_move_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result = immobility_on_next_move;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length+1);

  result = defend_with_killer_piece(si);

  if (result<=next_move_has_solution)
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
