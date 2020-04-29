#include "solving/observation.h"
#include "conditions/amu/attack_counter.h"
#include "conditions/backhome.h"
#include "conditions/beamten.h"
#include "conditions/bgl.h"
#include "conditions/bolero.h"
#include "conditions/brunner.h"
#include "conditions/central.h"
#include "conditions/disparate.h"
#include "conditions/geneva.h"
#include "conditions/imitator.h"
#include "conditions/lortap.h"
#include "conditions/mummer.h"
#include "conditions/patrol.h"
#include "conditions/provocateurs.h"
#include "conditions/shielded_kings.h"
#include "conditions/superguards.h"
#include "conditions/woozles.h"
#include "conditions/wormhole.h"
#include "conditions/madrasi.h"
#include "conditions/partial_paralysis.h"
#include "conditions/masand.h"
#include "conditions/eiffel.h"
#include "conditions/monochrome.h"
#include "conditions/bichrome.h"
#include "conditions/grid.h"
#include "conditions/edgemover.h"
#include "conditions/annan.h"
#include "conditions/pointreflection.h"
#include "conditions/facetoface.h"
#include "conditions/circe/circe.h"
#include "conditions/marscirce/phantom.h"
#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/singlebox/type3.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/transmuting_kings/reflective_kings.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "optimisations/observation.h"
#include "pieces/walks/hunters.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/magic.h"
#include "solving/move_generator.h"
#include "solving/has_solution_type.h"
#include "solving/conditional_pipe.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "solving/fork.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/binary.h"
#include "debugging/assert.h"

interceptable_observation_type interceptable_observation[maxply+1];
unsigned int observation_context = 0;

static boolean enforce_observer_walk(slice_index si)
{
  boolean result = false;
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  piece_walk_type const walk = get_walk_of_piece_on_square(sq_departure);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);TraceWalk(walk);TraceEOL();
  if (walk==observing_walk[nbply])
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue validating an observation (or observer or observation geometry)
 * @param si identifies the slice with which to continue
 * @return true iff the observation is valid
 */
boolean validate_observation_recursive(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,SLICE_TYPE(si));
  TraceEOL();

  switch (SLICE_TYPE(si))
  {
    case STSingleBoxType3EnforceObserverWalk:
      result = singleboxtype3_enforce_observer_walk(si);
      break;

    case STTransmutingKingsEnforceObserverWalk:
      result = transmuting_kings_enforce_observer_walk(si);
      break;

    case STVaultingKingsEnforceObserverWalk:
      result = vaulting_kings_enforce_observer_walk(si);
      break;

    case STReflectiveKingsEnforceObserverWalk:
      result = reflective_kings_enforce_observer_walk(si);
      break;

    case STUndoOptimiseObservationsByQueen:
      result = undo_optimise_observation_by_queen(si);
      break;

    case STEnforceObserverWalk:
      result = enforce_observer_walk(si);
      break;

    case STEnforceHunterDirection:
      result = hunter_enforce_observer_direction(si);
      break;

    case STAnnanEnforceObserverWalk:
      result = annan_enforce_observer_walk(si);
      break;

    case STNannaEnforceObserverWalk:
      result = nanna_enforce_observer_walk(si);
      break;

    case STBoleroInverseEnforceObserverWalk:
      result = bolero_inverse_enforce_observer_walk(si);
      break;

    case STPointReflectionEnforceObserverWalk:
      result = point_reflection_enforce_observer_walk(si);
      break;

    case STFaceToFaceEnforceObserverWalk:
      result = facetoface_enforce_observer_walk(si);
      break;

    case STBackToBackEnforceObserverWalk:
      result = backtoback_enforce_observer_walk(si);
      break;

    case STCheekToCheekEnforceObserverWalk:
      result = cheektocheek_enforce_observer_walk(si);
      break;

    case STMarsCirceGenerateFromRebirthSquare:
      result = mars_enforce_observer(si);
      break;

    case STMagicPiecesObserverEnforcer:
      result = magic_enforce_observer(si);
      break;

    case STAMUObservationCounter:
      result = amu_count_observation(si);
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

    case STCentralObservationValidator:
      result = central_validate_observation(si);
      break;

    case STDisparateMovesForPieceGenerator:
      result = disparate_validate_observation(si);
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
      result = woozles_validate_observer(si);
      break;

    case STBiWoozlesRemoveIllegalCaptures:
      result = biwoozles_validate_observer(si);
      break;

    case STHeffalumpsRemoveIllegalCaptures:
      result = heffalumps_validate_observer(si);
      break;

    case STBiHeffalumpsRemoveIllegalCaptures:
      result = biheffalumps_validate_observer(si);
      break;

    case STWormholeRemoveIllegalCaptures:
      result = wormhole_validate_observation(si);
      break;

    case STMadrasiMovesForPieceGenerator:
      result = madrasi_validate_observer(si);
      break;

    case STPartialParalysisMovesForPieceGenerator:
      result = partial_paralysis_validate_observer(si);
      break;

    case STEiffelMovesForPieceGenerator:
      result = eiffel_validate_observer(si);
      break;

    case STParalysingObserverValidator:
      result = paralysing_validate_observer(si);
      break;

    case STParalysingObservationGeometryValidator:
      result = paralysing_validate_observation_geometry(si);
      break;

    case STMonochromeRemoveNonMonochromeMoves:
      result = monochrome_validate_observation_geometry(si);
      break;

    case STBichromeRemoveNonBichromeMoves:
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
      result = (conditional_pipe_solve_delegate(temporary_hack_move_legality_tester[trait[nbply]])
                ==previous_move_has_solved);
      PUSH_OBSERVATION_TARGET_AGAIN(nbply);
      break;

    case STValidateCheckMoveByPlayingCapture:
      result = (conditional_pipe_solve_delegate(temporary_hack_king_capture_legality_tester[trait[nbply]])
                ==previous_move_has_solved);
      PUSH_OBSERVATION_TARGET_AGAIN(nbply);
      break;

    case STTrue:
      result = true;
      break;

    case STFalse:
      result = false;
      break;

    default:
      assert(0);
      result = false;
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
                         slice_index prototype,
                         slice_index const rank_order[],
                         unsigned int nr_rank_order)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    &prototype,1,
    rank_order, nr_rank_order, 1,
    branch_slice_rank_order_nonrecursive,
    0,
    testing,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing);
  TraceFunctionParam("%u",prototype);
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(SLICE_TYPE(testing),&state);
  assert(state.base_rank!=no_slice_rank);
  slice_insertion_init_traversal(&st,&state,stip_traversal_context_intro);
  stip_traverse_structure_children_pipe(testing,&st);
  dealloc_slice(prototype);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index const observation_validation_slice_rank_order[] =
{
    STValidatingCheck,
    STValidatingObservation,
    STMarsCirceGenerateFromRebirthSquare,
    STAMUObservationCounter,
    STMasandEnforceObserver,
    STUndoOptimiseObservationsByQueen,
    STSingleBoxType3EnforceObserverWalk,
    STTransmutingKingsEnforceObserverWalk,
    STVaultingKingsEnforceObserverWalk,
    STReflectiveKingsEnforceObserverWalk,
    STEnforceObserverWalk,
    STAnnanEnforceObserverWalk,
    STNannaEnforceObserverWalk,
    STBoleroInverseEnforceObserverWalk,
    STPointReflectionEnforceObserverWalk,
    STMagicPiecesObserverEnforcer,
    STFaceToFaceEnforceObserverWalk,
    STBackToBackEnforceObserverWalk,
    STCheekToCheekEnforceObserverWalk,
    STEnforceHunterDirection,
    STBackhomeExistanceTester,
    STBackhomeRemoveIllegalMoves,
    STBeamtenMovesForPieceGenerator,
    STBGLEnforcer,
    STBrunnerValidateCheck,
    STCentralObservationValidator,
    STDisparateMovesForPieceGenerator,
    STLortapRemoveSupportedCaptures,
    STValidatingObservationUltraMummer,
    STPatrolRemoveUnsupportedCaptures,
    STUltraPatrolMovesForPieceGenerator,
    STProvocateursRemoveUnobservedCaptures,
    STShieldedKingsRemoveIllegalCaptures,
    STSuperguardsRemoveIllegalCaptures,
    STWormholeRemoveIllegalCaptures,

    STValidatingObserver,
    STUndoOptimiseObservationsByQueen,
    STEnforceObserverWalk,
    STAnnanEnforceObserverWalk,
    STNannaEnforceObserverWalk,
    STBoleroInverseEnforceObserverWalk,
    STPointReflectionEnforceObserverWalk,
    STEnforceHunterDirection,
    STMadrasiMovesForPieceGenerator,
    STPartialParalysisMovesForPieceGenerator,
    STEiffelMovesForPieceGenerator,
    STParalysingObserverValidator,
    STWoozlesRemoveIllegalCaptures,
    STBiWoozlesRemoveIllegalCaptures,
    STHeffalumpsRemoveIllegalCaptures,
    STBiHeffalumpsRemoveIllegalCaptures,

    STValidatingObservationGeometry,
    STUndoOptimiseObservationsByQueen,
    STParalysingObservationGeometryValidator,
    STEnforceObserverWalk,
    STAnnanEnforceObserverWalk,
    STNannaEnforceObserverWalk,
    STBoleroInverseEnforceObserverWalk,
    STPointReflectionEnforceObserverWalk,
    STEnforceHunterDirection,
    STMonochromeRemoveNonMonochromeMoves,
    STBichromeRemoveNonBichromeMoves,
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
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  insert_slice(validating,
               alloc_pipe(type),
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

  if (it->side==nr_sides || it->side==SLICE_STARTER(si))
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

  result = fork_validate_observation_recursive_delegate(temporary_hack_observation_geometry_validator[trait[nbply]]);

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
  TraceEnumerator(Side,side);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,type,&stip_structure_visitor_noop);
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

  result = fork_validate_observation_recursive_delegate(temporary_hack_observer_validator[trait[nbply]]);

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
  TraceEnumerator(Side,side);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,type,&stip_structure_visitor_noop);
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

  result = fork_validate_observation_recursive_delegate(temporary_hack_check_validator[trait[nbply]]);

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

  result = fork_validate_observation_recursive_delegate(temporary_hack_observation_validator[trait[nbply]]);

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
  TraceEnumerator(Side,side);
  TraceEnumerator(slice_type,type);
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
  TraceEnumerator(Side,side);
  TraceEnumerator(slice_type,type);
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

validator_id observation_validator;
boolean observation_result;

/* Determine whether the current target square is observed be the side at the move
 * @return true iff the target square is observed
 */
boolean is_square_observed(validator_id evaluate)
{
  return fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed[trait[nbply]],
                                                 evaluate);
}

/* Determine whether a square is observed by a side
 * @param side observing side
 * @param s the square
 * @param evaluate identifies the set of restrictions imposed on the observation
 * @return true iff the square is observed
 */
boolean is_square_observed_general(Side side, square s, validator_id evaluate)
{
  boolean result;


  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  siblingply(side);
  push_observation_target(s);
  result = is_square_observed(evaluate);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Perform a nested observation validation run from within an observation
 * validation run
 * Restores observation_validator and observation_result to their previous
 * values before returning.
 * @return true iff the target square is observed
 */
boolean is_square_observed_nested(slice_index si, validator_id evaluate)
{
  boolean result;
  boolean const save_observation_validation_result = observation_result;
  validator_id const save_observation_validator = observation_validator;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  observation_validator = evaluate;
  is_square_observed_recursive(si);
  result = observation_result;

  observation_validator = save_observation_validator;
  observation_result = save_observation_validation_result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side observes a specific square
 * @param si identifies the tester slice
 * @note sets observation_result
 */
void is_square_observed_two_paths(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  pipe_is_square_observed_delegate(si);

  if (!observation_result)
    fork_is_square_observed_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index const is_square_observed_slice_rank_order[] =
{
    STTestingIfSquareIsObserved,
    STIsSquareObservedOrtho,
    STTransmutingKingIsSquareObserved,
    STVaultingKingIsSquareObserved,
    STOptimiseObservationsByQueenInitialiser,
    STDetermineObserverWalk,
    STBicoloresTryBothSides,
    STTestingIfSquareIsObservedWithSpecificWalk,
    STOptimisingObserverWalk,
    STDontTryObservingWithNonExistingWalk,
    STDontTryObservingWithNonExistingWalkBothSides,
    STOptimiseObservationsByQueen,
    STTransmutingKingDetectNonTransmutation,
    STIsSquareObservedTwoPaths,
    STIsSquareObservedStandardPath,
    STIsSquareObservedAlternativePath,
    STMarsCirceConsideringObserverRebirth,
    STCirceDoneWithRebirth,
    STMarsIsSquareObserved,
    STPlusIsSquareObserved,
    STIsSquareObservedPathsJoint,
    STTrackBackFromTargetAccordingToObserverWalk,
    STTrue
};

enum
{
  nr_is_square_observed_slice_rank_order_elmts = sizeof is_square_observed_slice_rank_order / sizeof is_square_observed_slice_rank_order[0]
};

static void observation_branch_insert_slices_impl(slice_index si,
                                                  slice_index const prototypes[],
                                                  unsigned int nr_prototypes,
                                                  slice_index base)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    prototypes,nr_prototypes,
    is_square_observed_slice_rank_order, nr_is_square_observed_slice_rank_order_elmts, 1,
    branch_slice_rank_order_nonrecursive,
    0,
    si,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(SLICE_TYPE(base),&state);
  assert(state.base_rank!=no_slice_rank);

  slice_insertion_init_traversal(&st,&state,stip_traversal_context_intro);
  circe_init_slice_insertion_traversal(&st);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a observation branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by help_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void observation_branch_insert_slices(slice_index si,
                                      slice_index const prototypes[],
                                      unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  observation_branch_insert_slices_impl(si,prototypes,nr_prototypes,si);
  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a particular square observation validation branch with a slice type
 * @param testing identifies STTestingIfSquareIsObserved at entrance of branch
 * @param type type of slice to insert
 */
void is_square_observed_insert_slice(slice_index testing,
                                     slice_index prototype)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing);
  TraceFunctionParam("%u",prototype);
  TraceFunctionParamListEnd();

  insert_slice(testing,
               prototype,
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

  if (it->side==nr_sides || it->side==SLICE_STARTER(si))
    is_square_observed_insert_slice(si,alloc_pipe(it->type));

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
  TraceEnumerator(Side,side);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STTestingIfSquareIsObserved,
                                           &instrument_square_observed_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_separator(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy_standard = alloc_proxy_slice();
    slice_index const standard = alloc_pipe(STIsSquareObservedStandardPath);

    slice_index const proxy_alternative = alloc_proxy_slice();
    slice_index const alternative = alloc_pipe(STIsSquareObservedAlternativePath);

    slice_index const generator = alloc_binary_slice(STIsSquareObservedTwoPaths,
                                                     proxy_standard,
                                                     proxy_alternative);

    pipe_link(SLICE_PREV(si),generator);

    pipe_link(proxy_standard,standard);
    pipe_link(standard,si);

    pipe_link(proxy_alternative,alternative);
    pipe_link(alternative,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the square observation testing machinery so that there are two
 * paths which can be adapted separately.
 * @param si root slice of solving machinery
 * @param side side for which to instrument; pass nr_sides for both sides
 * @note inserts proxy slices STIsSquareObservedStandardPath and
 *       STIsSquareObservedAlternativePath that can be used for adjusting the move
 *       generation
 */
void is_square_observed_instrument_for_alternative_paths(slice_index si,
                                                         Side side)
{
  stip_structure_traversal st;

  stip_instrument_is_square_observed_testing(si,
                                             side,
                                             STIsSquareObservedPathsJoint);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STIsSquareObservedPathsJoint,
                                           &insert_separator);
  stip_traverse_structure(si,&st);
}
