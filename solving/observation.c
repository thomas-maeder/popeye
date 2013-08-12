#include "solving/observation.h"
#include "conditions/backhome.h"
#include "conditions/beamten.h"
#include "conditions/bgl.h"
#include "conditions/brunner.h"
#include "conditions/central.h"
#include "conditions/disparate.h"
#include "conditions/geneva.h"
#include "conditions/imitator.h"
#include "conditions/immune.h"
#include "conditions/lortap.h"
#include "conditions/mummer.h"
#include "conditions/patrol.h"
#include "conditions/provocateurs.h"
#include "conditions/shielded_kings.h"
#include "conditions/superguards.h"
#include "conditions/woozles.h"
#include "conditions/wormhole.h"
#include "conditions/madrasi.h"
#include "conditions/masand.h"
#include "conditions/eiffel.h"
#include "conditions/monochrome.h"
#include "conditions/bichrome.h"
#include "conditions/grid.h"
#include "conditions/edgemover.h"
#include "conditions/annan.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/singlebox/type3.h"
#include "conditions/sting.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/transmuting_kings/reflective_kings.h"
#include "conditions/vaulting_kings.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/magic.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/single_move_generator.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
#include "optimisations/orthodox_square_observation.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

interceptable_observation_type interceptable_observation[maxply+1];
unsigned int observation_context = 0;

/* Continue validating an observation (or observer or observation geometry)
 * @param si identifies the slice with which to continue
 * @return rue iff the observation is valid
 */
boolean validate_observation_recursive(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");

  switch (slices[si].type)
  {
    case STEnforceObserverWalk:
    {
      square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
      if (get_walk_of_piece_on_square(sq_departure)==observing_walk[nbply])
        result = validate_observation_recursive(slices[si].next1);
      else
        result = false;
      break;
    }

    case STEnforceObserverSide:
    {
      square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
      if (TSTFLAG(spec[sq_departure],trait[nbply]))
        result = validate_observation_recursive(slices[si].next1);
      else
        result = false;
      break;
    }

    case STAnnanEnforceObserverWalk:
      result = annan_enforce_observer_walk(si);
      break;

    case STMarsCirceMovesForPieceGenerator:
      result = mars_enforce_observer(si);
      break;

    case STMagicPiecesObserverEnforcer:
      result = magic_enforce_observer(si);
      break;

    case STMasandEnforceObserver:
      result = masand_enforce_observer(si);
      break;

    case STBackhomeExistanceTester:
      result = backhome_existance_tester_validate_observation(si);
      break;

    case STBackhomeRemoveIllegalMoves:
      result = back_home_validate_observation(si);
      break;

    case STBeamtenMovesForPieceGenerator:
      result = beamten_validate_observation(si);
      break;

    case STBGLEnforcer:
      result = bgl_validate_observation(si);
      break;

    case STBrunnerValidateCheck:
      result = brunner_validate_observation(si);
      break;

    case STCentralMovesForPieceGenerator:
      result = central_validate_observation(si);
      break;

    case STDisparateMovesForPieceGenerator:
      result = disparate_validate_observation(si);
      break;

    case STGenevaRemoveIllegalCaptures:
      result = geneva_validate_observation(si);
      break;

    case STImmuneRemoveCapturesOfImmune:
      result = immune_validate_observation(si);
      break;

    case STLortapRemoveSupportedCaptures:
      result = lortap_validate_observation(si);
      break;

    case STValidatingObservationUltraMummer:
      result = ultra_mummer_validate_observation(si);
      break;

    case STPatrolRemoveUnsupportedCaptures:
      result = patrol_validate_observation(si);
      break;

    case STUltraPatrolMovesForPieceGenerator:
      result = ultrapatrol_validate_observation(si);
      break;

    case STProvocateursRemoveUnobservedCaptures:
      result = provocateurs_validate_observation(si);
      break;

    case STShieldedKingsRemoveIllegalCaptures:
      result = shielded_kings_validate_observation(si);
      break;

    case STSuperguardsRemoveIllegalCaptures:
      result = superguards_validate_observation(si);
      break;

    case STWoozlesRemoveIllegalCaptures:
      result = woozles_validate_observation(si);
      break;

    case STBiWoozlesRemoveIllegalCaptures:
      result = biwoozles_validate_observation(si);
      break;

    case STHeffalumpsRemoveIllegalCaptures:
      result = heffalumps_validate_observation(si);
      break;

    case STBiHeffalumpsRemoveIllegalCaptures:
      result = biheffalumps_validate_observation(si);
      break;

    case STWormholeRemoveIllegalCaptures:
      result = wormhole_validate_observation(si);
      break;

    case STMadrasiMovesForPieceGenerator:
      result = madrasi_validate_observer(si);
      break;

    case STEiffelMovesForPieceGenerator:
      result = eiffel_validate_observer(si);
      break;

    case STParalysingMovesForPieceGenerator:
      result = paralysing_validate_observer(si);
      break;

    case STMonochromeRemoveBichromeMoves:
      result = monochrome_validate_observation_geometry(si);
      break;

    case STBichromeRemoveMonochromeMoves:
      result = bichrome_validate_observation_geometry(si);
      break;

    case STGridRemoveIllegalMoves:
      result = grid_validate_observation_geometry(si);
      break;

    case STEdgeMoverRemoveIllegalMoves:
      result = edgemover_validate_observation_geometry(si);
      break;

    case STImitatorRemoveIllegalMoves:
      result = imitator_validate_observation(si);
      break;

    case STValidatingObservationGeometryByPlayingMove:
    {
      square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
      square const sq_landing = move_generation_stack[current_move[nbply]-1].arrival;
      TraceValue("%u",nbply);
      TraceValue("%u\n",current_move[nbply]);
      move_generation_stack[current_move[nbply]-1].departure = sq_observer;
      move_generation_stack[current_move[nbply]-1].arrival = sq_landing;
      result = (solve(slices[temporary_hack_move_legality_tester[trait[nbply]]].next2,
                      length_unspecified)
                ==next_move_has_solution);
      current_move[nbply] = current_move[nbply-1]+1;
      break;
    }

    case STValidateCheckMoveByPlayingCapture:
    {
      square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
      square const sq_landing = move_generation_stack[current_move[nbply]-1].arrival;
      TraceValue("%u",nbply);
      TraceValue("%u\n",current_move[nbply]);
      move_generation_stack[current_move[nbply]-1].departure = sq_observer;
      move_generation_stack[current_move[nbply]-1].arrival = sq_landing;
      result = (solve(slices[temporary_hack_king_capture_legality_tester[trait[nbply]]].next2,
                      length_unspecified)
                ==next_move_has_solution);
      current_move[nbply] = current_move[nbply-1]+1;
      break;
    }

    case STTrue:
      result = true;
      break;

    case STFalse:
      result = false;
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
    Side side;
    slice_type type;
} instrumentation_type;

static void insert_slice(slice_index testing,
                         slice_type type,
                         slice_index const rank_order[],
                         unsigned int nr_rank_order)
{
  slice_index const prototype = alloc_pipe(type);
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    &prototype,1,
    rank_order, nr_rank_order,
    branch_slice_rank_order_nonrecursive,
    0,
    testing,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(slices[testing].type,&state);
  assert(state.base_rank!=no_slice_rank);
  init_slice_insertion_traversal(&st,&state,stip_traversal_context_intro);
  stip_traverse_structure_children_pipe(testing,&st);
  dealloc_slice(prototype);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index const observation_validation_slice_rank_order[] =
{
    STValidatingCheck,
    STValidatingObservation,
    STMarsCirceMovesForPieceGenerator,
    STMagicPiecesObserverEnforcer,
    STMasandEnforceObserver,
    STEnforceObserverWalk,
    STEnforceObserverSide,
    STAnnanEnforceObserverWalk,
    STBackhomeExistanceTester,
    STBackhomeRemoveIllegalMoves,
    STBeamtenMovesForPieceGenerator,
    STBGLEnforcer,
    STBrunnerValidateCheck,
    STCentralMovesForPieceGenerator,
    STDisparateMovesForPieceGenerator,
    STGenevaRemoveIllegalCaptures,
    STImmuneRemoveCapturesOfImmune,
    STLortapRemoveSupportedCaptures,
    STValidatingObservationUltraMummer,
    STPatrolRemoveUnsupportedCaptures,
    STUltraPatrolMovesForPieceGenerator,
    STProvocateursRemoveUnobservedCaptures,
    STShieldedKingsRemoveIllegalCaptures,
    STSuperguardsRemoveIllegalCaptures,
    STWoozlesRemoveIllegalCaptures,
    STBiWoozlesRemoveIllegalCaptures,
    STHeffalumpsRemoveIllegalCaptures,
    STBiHeffalumpsRemoveIllegalCaptures,
    STWormholeRemoveIllegalCaptures,

    STValidatingObserver,
    STMarsCirceMovesForPieceGenerator,
    STEnforceObserverWalk,
    STEnforceObserverSide,
    STMadrasiMovesForPieceGenerator,
    STEiffelMovesForPieceGenerator,
    STParalysingMovesForPieceGenerator,

    STValidatingObservationGeometry,
    STEnforceObserverWalk,
    STEnforceObserverSide,
    STMonochromeRemoveBichromeMoves,
    STBichromeRemoveMonochromeMoves,
    STGridRemoveIllegalMoves,
    STEdgeMoverRemoveIllegalMoves,
    STImitatorRemoveIllegalMoves,
    STValidatingObservationGeometryByPlayingMove,
    STValidateCheckMoveByPlayingCapture,
    STTrue
};

enum
{
  nr_observation_validation_slice_rank_order = sizeof observation_validation_slice_rank_order / sizeof observation_validation_slice_rank_order[0]
};

static void observation_validation_insert_slice(slice_index validating,
                                                slice_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",validating);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  insert_slice(validating,
               type,
               observation_validation_slice_rank_order,
               nr_observation_validation_slice_rank_order);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_observation_validation(slice_index si,
                                              stip_structure_traversal *st)
{
  instrumentation_type const * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (it->side==nr_sides || it->side==slices[si].starter)
    observation_validation_insert_slice(si,it->type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation
 * @return true iff the observation is valid
 */
boolean validate_observation_geometry(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(slices[temporary_hack_observation_geometry_validator[trait[nbply]]].next2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument square observation validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_observation_geometry_validation(slice_index si,
                                                     Side side,
                                                     slice_type type)
{
  instrumentation_type it = { side, type };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STValidatingObservationGeometry,
                                           &instrument_observation_validation);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation
 * @return true iff the observation is valid
 */
boolean validate_observer(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(slices[temporary_hack_observer_validator[trait[nbply]]].next2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument observer validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_observer_validation(slice_index si,
                                         Side side,
                                         slice_type type)
{
  instrumentation_type it = { side, type };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STValidatingObserver,
                                           &instrument_observation_validation);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate a check
 * @return true iff the observation is valid
 */
boolean validate_check(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(move_generation_stack[current_move[nbply]-1].departure);
  TraceSquare(move_generation_stack[current_move[nbply]-1].arrival);
  TraceSquare(move_generation_stack[current_move[nbply]-1].capture);
  TraceText("\n");

  result = validate_observation_recursive(slices[temporary_hack_check_validator[trait[nbply]]].next2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation
 * @return true iff the observation is valid
 */
boolean validate_observation(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(slices[temporary_hack_observation_validator[trait[nbply]]].next2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument observation validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_observation_validation(slice_index si,
                                            Side side,
                                            slice_type type)
{
  instrumentation_type it = { side, type };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STValidatingObservation,
                                           &instrument_observation_validation);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument observation validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_check_validation(slice_index si,
                                      Side side,
                                      slice_type type)
{
  instrumentation_type it = { side, type };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STValidatingCheck,
                                           &instrument_observation_validation);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Cause observations to be validated by playing the move representing the
 * observation.
 * @param si root slice of solving machinery
 * @param side for which side (pass nr_sides to indicate both sides)
 */
void observation_play_move_to_validate(slice_index si, Side side)
{
  stip_instrument_observation_validation(si,
                                         side,
                                         STValidatingObservationGeometryByPlayingMove);
  stip_instrument_check_validation(si,
                                   side,
                                   STValidateCheckMoveByPlayingCapture);
}

static slice_type const slice_order_is_square_observed_with_proxies[] = {
  STProxy,
  STTestingIfSquareIsObserved,
  STFindSquareObserverTrackingBackKing,
  STLandingAfterFindSquareObserverTrackingBackKing,
  STFindSquareObserverTrackingBack,
  STFalse
};

static slice_type const slice_order_check_with_proxies[] = {
  STProxy,
  STValidatingCheck,
  STEnforceObserverWalk,
  STEnforceObserverSide,
  STValidatingObserver,
  STValidatingObservationGeometry,
  STTrue
};

static slice_type const slice_order_observation_with_proxies[] = {
  STProxy,
  STValidatingObservation,
  STEnforceObserverWalk,
  STEnforceObserverSide,
  STValidatingObserver,
  STValidatingObservationGeometry,
  STTrue
};

static slice_type const slice_order_observation_geometry_with_proxies[] = {
  STProxy,
  STValidatingObservationGeometry,
  STEnforceObserverWalk,
  STEnforceObserverSide,
  STTrue
};

static slice_type const slice_order_observer_with_proxies[] = {
  STProxy,
  STValidatingObserver,
  STEnforceObserverWalk,
  STEnforceObserverSide,
  STValidatingObservationGeometry,
  STTrue
};

static slice_type const slice_order_observation_without_proxies[] = {
  STEnforceObserverWalk,
  STEnforceObserverSide,
  STTrue
};

enum
{
  nr_slice_order_is_square_observed_with_proxies = sizeof slice_order_is_square_observed_with_proxies / sizeof slice_order_is_square_observed_with_proxies[0],
  nr_slice_order_check_with_proxies = sizeof slice_order_check_with_proxies / sizeof slice_order_check_with_proxies[0],
  nr_slice_order_observation_with_proxies = sizeof slice_order_observation_with_proxies / sizeof slice_order_observation_with_proxies[0],
  nr_slice_order_observation_geometry_with_proxies = sizeof slice_order_observation_geometry_with_proxies / sizeof slice_order_observation_geometry_with_proxies[0],
  nr_slice_order_observer_with_proxies = sizeof slice_order_observer_with_proxies / sizeof slice_order_observer_with_proxies[0],
  nr_slice_order_observation_without_proxies = sizeof slice_order_observation_without_proxies / sizeof slice_order_observation_without_proxies[0]
};

static boolean has_slice_order(slice_index si, slice_type const order[], unsigned int number)
{
  boolean result = true;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",number);
  TraceFunctionParamListEnd();

  for (i = 0; i!=number; ++i)
    if (slices[si].type==order[i])
      si = slices[si].next1;
    else
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void optimise_side(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (has_slice_order(slices[temporary_hack_is_square_observed[side]].next2,
                      slice_order_is_square_observed_with_proxies,
                      nr_slice_order_is_square_observed_with_proxies))
    stip_instrument_is_square_observed_testing(si,side,STIsSquareObservedOrtho);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Optimise the square observation machinery if possible
 * @param si identifies the root slice of the solving machinery
 */
void optimise_is_square_observed(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  /* this is invoked when the proxy slices are still there ... */
  if (has_slice_order(slices[temporary_hack_check_validator[White]].next2,
                      slice_order_check_with_proxies,
                       nr_slice_order_check_with_proxies)
      && has_slice_order(slices[temporary_hack_observation_validator[White]].next2,
                         slice_order_observation_with_proxies,
                      nr_slice_order_observation_with_proxies)
      && has_slice_order(slices[temporary_hack_observation_geometry_validator[White]].next2,
                         slice_order_observation_geometry_with_proxies,
                         nr_slice_order_observation_geometry_with_proxies)
      && has_slice_order(slices[temporary_hack_observer_validator[White]].next2,
                         slice_order_observer_with_proxies,
                         nr_slice_order_observer_with_proxies))
    optimise_side(si,White);

  if (has_slice_order(slices[temporary_hack_check_validator[Black]].next2,
                      slice_order_check_with_proxies,
                       nr_slice_order_check_with_proxies)
      && has_slice_order(slices[temporary_hack_observation_validator[Black]].next2,
                         slice_order_observation_with_proxies,
                      nr_slice_order_observation_with_proxies)
      && has_slice_order(slices[temporary_hack_observation_geometry_validator[Black]].next2,
                         slice_order_observation_geometry_with_proxies,
                         nr_slice_order_observation_geometry_with_proxies)
      && has_slice_order(slices[temporary_hack_observer_validator[Black]].next2,
                         slice_order_observer_with_proxies,
                         nr_slice_order_observer_with_proxies))
    optimise_side(si,Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether observations are trivially validated (allowing for some
 * optimisations)
 * @param side for which side?
 */
boolean is_observation_trivially_validated(Side side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  /* ... and this when the proxy slices are removed */
  result = (has_slice_order(slices[temporary_hack_check_validator[side]].next2,
                            slice_order_observation_without_proxies,
                             nr_slice_order_observation_without_proxies)
            && has_slice_order(slices[temporary_hack_observation_validator[side]].next2,
                               slice_order_observation_without_proxies,
                               nr_slice_order_observation_without_proxies)
            && has_slice_order(slices[temporary_hack_observation_geometry_validator[side]].next2,
                               slice_order_observation_without_proxies,
                               nr_slice_order_observation_without_proxies)
            && has_slice_order(slices[temporary_hack_observer_validator[side]].next2,
                               slice_order_observation_without_proxies,
                               nr_slice_order_observation_without_proxies));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_square_observed_recursive(slice_index si, evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");

  switch (slices[si].type)
  {
    case STIsSquareObservedOrtho:
      if (evaluate==&validate_observation || evaluate==&validate_check)
        result = is_square_observed_ortho();
      else
        result = is_square_observed_recursive(slices[si].next1,evaluate);
      break;

    case STSingleBoxType3IsSquareObserved:
      result = singleboxtype3_is_square_observed(si,evaluate);
      break;

    case STPhantomIsSquareObserved:
      result = phantom_is_square_observed(si,evaluate);
      break;

    case STPlusIsSquareObserved:
      result = plus_is_square_observed(si,evaluate);
      break;

    case STMarsIsSquareObserved:
      result = marscirce_is_square_observed(si,evaluate);
      break;

    case STStingIsSquareObserved:
      result = sting_is_square_observed(si,evaluate);
      break;

    case STVaultingKingIsSquareObserved:
      result = vaulting_king_is_square_observed(si,evaluate);
      break;

    case STTransmutingKingIsSquareObserved:
      result = transmuting_king_is_square_observed(si,evaluate);
      break;

    case STReflectiveKingIsSquareObserved:
      result = reflective_king_is_square_observed(si,evaluate);
      break;

    case STFindSquareObserverTrackingBackKing:
      result = find_square_observer_tracking_back_from_target_king(si,evaluate);
      break;

    case STFindSquareObserverTrackingBack:
      result = find_square_observer_tracking_back_from_target_non_king(si,evaluate);
      break;

    case STFindSquareObserverTrackingBackFairy:
      result = find_square_observer_tracking_back_from_target_fairy(si,evaluate);
      break;

    case STTrue:
      result = true;
      break;

    case STFalse:
      result = false;
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_square_observed(evalfunction_t *evaluate)
{
  return is_square_observed_recursive(slices[temporary_hack_is_square_observed[trait[nbply]]].next2,
                                      evaluate);
}

static slice_index const is_square_observed_slice_rank_order[] =
{
    STTestingIfSquareIsObserved,
    STIsSquareObservedOrtho,
    STSingleBoxType3IsSquareObserved,
    STPhantomIsSquareObserved,
    STPlusIsSquareObserved,
    STMarsIsSquareObserved,
    STStingIsSquareObserved,
    STVaultingKingIsSquareObserved,
    STTransmutingKingIsSquareObserved,
    STReflectiveKingIsSquareObserved,
    STFindSquareObserverTrackingBackKing,
    STFindSquareObserverTrackingBack,
    STFindSquareObserverTrackingBackFairy,
    STFalse
};

enum
{
  nr_is_square_observed_slice_rank_order_elmts = sizeof is_square_observed_slice_rank_order / sizeof is_square_observed_slice_rank_order[0]
};

/* Instrument a particular square observation validation branch with a slice type
 * @param testing identifies STTestingIfSquareIsObserved at entrance of branch
 * @param type type of slice to insert
 */
void is_square_observed_insert_slice(slice_index testing, slice_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  insert_slice(testing,
               type,
               is_square_observed_slice_rank_order,
               nr_is_square_observed_slice_rank_order_elmts);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_square_observed_testing(slice_index si,
                                               stip_structure_traversal *st)
{
  instrumentation_type const * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (it->side==nr_sides || it->side==slices[si].starter)
    is_square_observed_insert_slice(si,it->type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument square observation validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_is_square_observed_testing(slice_index si,
                                                Side side,
                                                slice_type type)
{
  instrumentation_type it = { side, type };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STTestingIfSquareIsObserved,
                                           &instrument_square_observed_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
