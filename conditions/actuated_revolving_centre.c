#include "conditions/actuated_revolving_centre.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STActuatedRevolvingCentre);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_actuated_revolving_centre(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void revolve(void)
{
  piece const piece_temp = e[square_d4];
  Flags const spec_temp = spec[square_d4];

  e[square_d4] = e[square_e4];
  spec[square_d4] = spec[square_e4];

  e[square_e4] = e[square_e5];
  spec[square_e4] = spec[square_e5];

  e[square_e5] = e[square_d5];
  spec[square_e5] = spec[square_d5];

  e[square_d5] = piece_temp;
  spec[square_d5] = spec_temp;

  if (king_square[White]==square_d4)
    king_square[White] = square_d5;
  else if (king_square[White]==square_d5)
    king_square[White] = square_e5;
  else if (king_square[White]==square_e5)
    king_square[White] = square_e4;
  else if (king_square[White]==square_e4)
    king_square[White] = square_d4;

  if (king_square[Black]==square_d4)
    king_square[Black] = square_d5;
  else if (king_square[Black]==square_d5)
    king_square[Black] = square_e5;
  else if (king_square[Black]==square_e5)
    king_square[Black] = square_e4;
  else if (king_square[Black]==square_e4)
    king_square[Black] = square_d4;
}

static void unrevolve(void)
{
  piece const piece_temp = e[square_d5];
  Flags const spec_temp = spec[square_d5];

  e[square_d5] = e[square_e5];
  spec[square_d5] = spec[square_e5];

  e[square_e5] = e[square_e4];
  spec[square_e5] = spec[square_e4];

  e[square_e4] = e[square_d4];
  spec[square_e4] = spec[square_d4];

  e[square_d4] = piece_temp;
  spec[square_d4] = spec_temp;

  if (king_square[White]==square_d4)
    king_square[White] = square_e4;
  else if (king_square[White]==square_d5)
    king_square[White] = square_d4;
  else if (king_square[White]==square_e5)
    king_square[White] = square_d5;
  else if (king_square[White]==square_e4)
    king_square[White] = square_e5;

  if (king_square[Black]==square_d4)
    king_square[Black] = square_e4;
  else if (king_square[Black]==square_d5)
    king_square[Black] = square_d4;
  else if (king_square[Black]==square_e5)
    king_square[Black] = square_d5;
  else if (king_square[Black]==square_e4)
    king_square[Black] = square_e5;
}

static boolean does_move_trigger_revolution(void)
{
  boolean result;
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = (sq_departure==square_d4    || sq_departure==square_e4
            || sq_departure==square_d5 || sq_departure==square_e5
            || sq_arrival==square_d4   || sq_arrival==square_e4
            || sq_arrival==square_d5   || sq_arrival==square_e5);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type actuated_revolving_centre_attack(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (does_move_trigger_revolution())
  {
    revolve();
    result = attack(next,n);
    unrevolve();
  }
  else
    result = attack(next,n);

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
stip_length_type actuated_revolving_centre_defend(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (does_move_trigger_revolution())
  {
    revolve();
    result = defend(next,n);
    unrevolve();
  }
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
