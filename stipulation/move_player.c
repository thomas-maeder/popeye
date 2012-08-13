#include "stipulation/move_player.h"
#include "pyproc.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

/* Replace the arriving piece of the currently played move
 * @param p substitute
 */
void replace_arriving_piece(piece p)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  --nbpiece[e[sq_arrival]];
  e[sq_arrival] = p;
  ++nbpiece[e[sq_arrival]];
  jouearr[nbply] = e[sq_arrival];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    slice_index const prototype = alloc_pipe(*type);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
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
  stip_structure_traversal_override_single(&st,STReplayingMoves,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void stip_instrument_moves_no_replay(slice_index si, slice_type type)
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

void stip_instrument_moves_only_replay(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,STReplayingMoves,&instrument_move);
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

  jouespec[nbply] = spec[sq_departure];
  jouearr[nbply] = e[sq_departure];

  assert(sq_arrival!=nullsquare);

  pjoue[nbply] = e[sq_departure];

  --nbpiece[e[sq_departure]];
  e[sq_departure] = vide;
  spec[sq_departure]= 0;

  pprise[nbply] = e[sq_capture];
  pprispec[nbply] = spec[sq_capture];
  if (pprise[nbply]!=vide)
    --nbpiece[pprise[nbply]];
  e[sq_capture] = vide;
  spec[sq_capture] = 0;

  e[sq_arrival] = jouearr[nbply];
  spec[sq_arrival] = jouespec[nbply];
  ++nbpiece[e[sq_arrival]];
}

static void retract_move(void)
{
  move_generation_elmt const * const move_gen_top = move_generation_stack+current_move[nbply];

  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;
  square const sq_capture = move_gen_top->capture;
  piece const pi_captured = pprise[nbply];

  assert(sq_arrival!=nullsquare);

  --nbpiece[e[sq_arrival]];
  e[sq_arrival] = vide;
  spec[sq_arrival] = 0;

  e[sq_capture] = pi_captured;
  spec[sq_capture] = pprispec[nbply];
  if (pi_captured!=vide)
    ++nbpiece[pi_captured];

  e[sq_departure] = pjoue[nbply];
  spec[sq_departure] = jouespec[nbply];
  ++nbpiece[e[sq_departure]];
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
  retract_move();

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
  retract_move();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
