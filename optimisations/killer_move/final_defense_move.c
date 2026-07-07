#include "optimisations/killer_move/final_defense_move.h"
#include "position/position.h"
#include "stipulation/pipe.h"
#include "stipulation/binary.h"
#include "solving/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "solving/single_piece_move_generator.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "optimisations/killer_move/prioriser.h"
#include "optimisations/killer_move/killer_move.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

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
  stip_traverse_structure(SLICE_NEXT1(si),&st_nested);

  init_deep_copy(&st_nested,st,&copies2);
  stip_structure_traversal_override_single(&st_nested,
                                           STDonePriorisingMoves,
                                           &stop_copying);
  stip_traverse_structure(SLICE_NEXT1(si),&st_nested);

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

    pipe_link(SLICE_PREV(si),fork);
    pipe_link(proxy1,si);
    pipe_link(proxy2,last_defense);
    pipe_link(proxy3,copies1[SLICE_NEXT1(si)]);
    pipe_link(proxy4,copies2[SLICE_NEXT1(si)]);
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
  Side const defender = SLICE_STARTER(si);
  square const killer_pos = killer_moves[nbply].departure;
  square const *bnp;
  stip_length_type result = immobility_on_next_move;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; result<=next_move_has_solution && *bnp!=initsquare; ++bnp)
    if (*bnp!=killer_pos && TSTFLAG(being_solved.spec[*bnp],defender))
    {
      init_single_piece_move_generator(*bnp);
      pipe_solve_delegate(si);
      if (solve_result>result)
        result = solve_result;
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
static void defend_with_killer_piece(slice_index si)
{
  Side const defender = SLICE_STARTER(si);
  square const killer_pos = killer_moves[nbply].departure;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(killer_pos);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[killer_pos],defender))
  {
    init_single_piece_move_generator(killer_pos);
    fork_solve_delegate(si);
  }
  else
    solve_result = immobility_on_next_move;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void killer_move_final_defense_move_solve(slice_index si)
{
  stip_length_type result_intermediate;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining==next_move_has_solution);

  defend_with_killer_piece(si);
  result_intermediate = solve_result;

  if (result_intermediate<=next_move_has_solution)
  {
    stip_length_type const result_non_killers = defend_with_non_killer_pieces(si);
    if (result_non_killers>result_intermediate)
      result_intermediate = result_non_killers;
  }

  solve_result = result_intermediate;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
