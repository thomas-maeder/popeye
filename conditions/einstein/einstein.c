#include "conditions/einstein/einstein.h"
#include "stipulation/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Decrease the rank of a piece
 * @param p piece whose rank to decrease
 */
piece einstein_decrease_piece(piece p)
{
  switch (p)
  {
  case db:
    return tb;
    case dn:
      return tn;
    case tb:
      return fb;
    case tn:
      return fn;
    case fb:
      return cb;
    case fn:
      return cn;
    case cb:
      return pb;
    case cn:
      return pn;
    default:
      break;
  }

  return p;
}

/* Increase the rank of a piece
 * @param p piece whose rank to increase
 */
piece einstein_increase_piece(piece p)
{
  switch (p) {
    case pb:
      return cb;
    case pn:
      return cn;
    case cb:
      return fb;
    case cn:
      return fn;
    case fb:
      return tb;
    case fn:
      return tn;
    case tb:
      return db;
    case tn:
      return dn;
    default:
      break;
  }

  return p;
}

static void decrease_passive_piece(square sq_arrival)
{
  --nbpiece[e[sq_arrival]];
  e[sq_arrival] = einstein_decrease_piece(e[sq_arrival]);
  ++nbpiece[e[sq_arrival]];
}

static void increase_passive_piece(square sq_arrival)
{
  --nbpiece[e[sq_arrival]];
  e[sq_arrival] = einstein_increase_piece(e[sq_arrival]);
  ++nbpiece[e[sq_arrival]];
}

/* Adjust the castling partner if the current move is a castling
 * @param trait_ply side executing the current move
 */
void einstein_decrease_castling_partner(Side trait_ply)
{
  switch (move_generation_stack[current_move[nbply]].capture)
  {
    case kingside_castling:
    {
      decrease_passive_piece(trait_ply==White ? square_f1 : square_f8);
      break;
    }
    case queenside_castling:
    {
      decrease_passive_piece(trait_ply==White ? square_d1 : square_d8);
      break;
    }
  }
}

/* Adjust the castling partner if the current move is a castling
 * @param trait_ply side executing the current move
 */
void einstein_increase_castling_partner(Side trait_ply)
{
  switch (move_generation_stack[current_move[nbply]].capture)
  {
    case kingside_castling:
    {
      increase_passive_piece(trait_ply==White ? square_f1 : square_f8);
      break;
    }
    case queenside_castling:
    {
      increase_passive_piece(trait_ply==White ? square_d1 : square_d8);
      break;
    }
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
stip_length_type einstein_moving_adjuster_attack(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  piece const pi_arriving = e[move_generation_stack[current_move[nbply]].arrival];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  replace_arriving_piece(pprise[nbply]==vide
                         ? einstein_decrease_piece(pi_arriving)
                         : einstein_increase_piece(pi_arriving));

  einstein_decrease_castling_partner(slices[si].starter);

  result = attack(slices[si].next1,n);

  einstein_increase_castling_partner(slices[si].starter);

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
stip_length_type einstein_moving_adjuster_defend(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  piece const pi_arriving = e[move_generation_stack[current_move[nbply]].arrival];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  replace_arriving_piece(pprise[nbply]==vide
                         ? einstein_decrease_piece(pi_arriving)
                         : einstein_increase_piece(pi_arriving));

  einstein_decrease_castling_partner(slices[si].starter);

  result = defend(slices[si].next1,n);

  einstein_increase_castling_partner(slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_einstein_moving_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STEinsteinArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
