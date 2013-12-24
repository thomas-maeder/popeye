#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

PieNam transmpieces[nr_sides][PieceCount];

boolean testing_observation_by_transmuting_king[maxply+1];

/* Initialise the sequence of king transmuters
 * @param side for which side to initialise?
 */
void init_transmuters_sequence(Side side)
{
  unsigned int tp = 0;
  PieNam p;

  for (p = King; p<PieceCount; ++p) {
    if (may_exist[p] && p!=Dummy && p!=Hamster)
    {
      transmpieces[side][tp] = p;
      tp++;
    }
  }

  transmpieces[side][tp] = Empty;
}

/* Determine whether the moving side's king is transmuting as a specific piece
 * @param p the piece
 */
boolean is_king_transmuting_as(PieNam p, validator_id evaluate)
{
  boolean result;
  Side const side_attacking = trait[nbply];

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  siblingply(advers(side_attacking));
  push_observation_target(king_square[side_attacking]);
  observing_walk[nbply] = p;
  result = (*checkfunctions[p])(evaluate);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_square_observed_by_opponent(PieNam p)
{
  boolean result;

  siblingply(advers(trait[nbply]));
  push_observation_target(curr_generation->departure);
  observing_walk[nbply] = p;
  result = (*checkfunctions[p])(EVALUATE(observation));
  finply();

  return result;
}

/* Generate moves of a potentially transmuting king
 * @param si identifies move generator slice
 * @return true iff the king is transmuting (which doesn't necessarily mean that
 *              any moves were generated!)
 */
boolean generate_moves_of_transmuting_king(slice_index si)
{
  boolean result = false;
  Side const side_moving = trait[nbply];
  Side const side_transmuting = advers(side_moving);

  PieNam const *ptrans;
  for (ptrans = transmpieces[side_moving]; *ptrans!=Empty; ++ptrans)
    if (number_of_pieces[side_transmuting][*ptrans]>0
        && is_square_observed_by_opponent(*ptrans))
    {
      generate_moves_for_piece(slices[si].next1,*ptrans);
      result = true;
    }

  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void transmuting_kings_generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (!(p==King && generate_moves_of_transmuting_king(si)))
    generate_moves_for_piece(slices[si].next1,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void transmuting_kings_initialise_solving(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,side,STTransmutingKingsMovesForPieceGenerator);
  instrument_alternative_is_square_observed_king_testing(si,side,STTransmutingKingIsSquareObserved);

  stip_instrument_observation_validation(si,side,STTransmutingKingsEnforceObserverWalk);
  stip_instrument_check_validation(si,side,STTransmutingKingsEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef enum
{
  king_not_transmuting,
  king_transmuting_not_observing,
  king_transmuting_observing
} transmuting_king_observation_type;

static transmuting_king_observation_type
does_transmuting_king_observe(slice_index si, validator_id evaluate)
{
  transmuting_king_observation_type result = king_not_transmuting;
  PieNam *ptrans;

  testing_observation_by_transmuting_king[nbply] = true;

  for (ptrans = transmpieces[trait[nbply]]; *ptrans; ptrans++)
    if (is_king_transmuting_as(*ptrans,evaluate))
    {
      observing_walk[nbply] = *ptrans;
      if (is_square_observed_recursive(slices[si].next2,evaluate))
      {
        result = king_transmuting_observing;
        break;
      }
      else
        result = king_transmuting_not_observing;
    }

  testing_observation_by_transmuting_king[nbply] = false;

  return result;
}

/* Determine whether a square is observed be the side at the move according to
 * Transmuting Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean transmuting_king_is_square_observed(slice_index si, validator_id evaluate)
{
  if (king_square[trait[nbply]]==initsquare)
    return is_square_observed_recursive(slices[si].next1,evaluate);
  else
    switch (does_transmuting_king_observe(si,evaluate))
    {
      case king_transmuting_observing:
        return true;

      case king_transmuting_not_observing:
        return false;

      case king_not_transmuting:
        return is_square_observed_recursive(slices[si].next1,evaluate);

      default:
        assert(0);
        return false;
    }
}

/* Make sure to behave correctly while detecting observations by vaulting kings
 */
boolean transmuting_kings_enforce_observer_walk(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (testing_observation_by_transmuting_king[nbply])
  {
    square const sq_king = king_square[trait[nbply]];
    if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure==sq_king)
    {
      PieNam const save_walk = observing_walk[nbply];
      observing_walk[nbply] = get_walk_of_piece_on_square(sq_king);
      result = validate_observation_recursive(slices[si].next1);
      observing_walk[nbply] = save_walk;
    }
    else
      return false;
  }
  else
    result = validate_observation_recursive(slices[si].next1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
    Side side;
    slice_type type;
    slice_index determined;
} instrumentation_type;

static void instrument_testing(slice_index si, stip_structure_traversal *st)
{
  instrumentation_type * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (it->side==nr_sides || it->side==slices[si].starter)
    is_square_observed_insert_slice(si,it->type);

  stip_traverse_structure_children_pipe(si,st);

  it->determined = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_determined(slice_index si, stip_structure_traversal *st)
{
  instrumentation_type * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  it->determined = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void connect_to_determined(slice_index si, stip_structure_traversal *st)
{
  instrumentation_type * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (slices[si].next2==no_slice)
  {
    slices[si].next2 = alloc_proxy_slice();
    link_to_branch(slices[si].next2,it->determined);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the square observation machinery for a side with an alternative
 * slice dealing with observations by kings.
 * @param si identifies the root slice of the solving machinery
 * @param side side for which to instrument the square observation machinery
 * @param type type of slice to insert
 */
void instrument_alternative_is_square_observed_king_testing(slice_index si,
                                                            Side side,
                                                            slice_type type)
{
  instrumentation_type it = { side, type, no_slice };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STDeterminedObserverWalk,
                                           &remember_determined);
  stip_structure_traversal_override_single(&st,type,&connect_to_determined);
  stip_structure_traversal_override_single(&st,
                                           STFindingSquareObserverTrackingBackKing,
                                           &instrument_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
