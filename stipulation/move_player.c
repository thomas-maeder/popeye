#include "stipulation/move_player.h"
#include "pyproc.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    slice_index const prototype = alloc_pipe(*type);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void stip_instrument_moves(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STMovePlayer slice.
 * @return index of allocated slice
 */
slice_index alloc_move_player_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMovePlayer);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void play_move(void)
{
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  assert(sq_arrival!=nullsquare);

  pjoue[nbply] = e[sq_departure];
  pprise[nbply] = e[sq_capture];
  pprispec[nbply] = spec[sq_capture];

  if (e[sq_capture]==vide)
    move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                          sq_departure,sq_arrival);
  else
  {
    move_effect_journal_index_type const
      removal = move_effect_journal_do_piece_removal(move_effect_reason_regular_capture,
                                                     sq_capture);
    move_effect_journal_index_type const
      movement = move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                                       sq_departure,sq_arrival);
    move_effect_journal_link_capture_to_movement(removal,movement);
  }
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_player_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  play_move();
  result = attack(slices[si].next1,n);

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
stip_length_type move_player_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  play_move();
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
