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

vec_index_type interceptable_observation_vector_index[maxply+1];
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
    case STValidatingObservationBackHome:
      result = back_home_validate_observation(si);
      break;

    case STValidatingObservationBeamten:
      result = beamten_validate_observation(si);
      break;

    case STValidatingObservationBGL:
      result = bgl_validate_observation(si);
      break;

    case STValidatingObservationBrunner:
      result = brunner_validate_observation(si);
      break;

    case STValidatingObservationCentral:
      result = central_validate_observation(si);
      break;

    case STValidatingObservationDisparate:
      result = disparate_validate_observation(si);
      break;

    case STValidatingObservationGeneva:
      result = geneva_validate_observation(si);
      break;

    case STValidatingObservationImmune:
      result = immune_validate_observation(si);
      break;

    case STValidatingObservationLortap:
      result = lortap_validate_observation(si);
      break;

    case STValidatingObservationUltraMummer:
      result = ultra_mummer_validate_observation(si);
      break;

    case STValidatingObservationPatrol:
      result = patrol_validate_observation(si);
      break;

    case STValidatingObservationUltraPatrol:
      result = ultrapatrol_validate_observation(si);
      break;

    case STValidatingObservationProvocateurs:
      result = provocateurs_validate_observation(si);
      break;

    case STValidatingObservationShielded:
      result = shielded_kings_validate_observation(si);
      break;

    case STValidatingObservationSuperGuards:
      result = superguards_validate_observation(si);
      break;

    case STValidateObservationWoozles:
      result = woozles_validate_observation(si);
      break;

    case STValidateObservationBiWoozles:
      result = biwoozles_validate_observation(si);
      break;

    case STValidateObservationHeffalumps:
      result = heffalumps_validate_observation(si);
      break;

    case STValidateObservationBiHeffalumps:
      result = biheffalumps_validate_observation(si);
      break;

    case STValidatingObservationWormholes:
      result = wormhole_validate_observation(si);
      break;

    case STMadrasiObserverTester:
      result = madrasi_validate_observer(si);
      break;

    case STEiffelObserverTester:
      result = eiffel_validate_observer(si);
      break;

    case STParalysingPiecesObserverTester:
      result = paralysing_validate_observer(si);
      break;

    case STValidateObservationGeometryMonochrome:
      result = monochrome_validate_observation_geometry(si);
      break;

    case STValidateObservationGeometryBichrome:
      result = bichrome_validate_observation_geometry(si);
      break;

    case STValidateObservationGeometryGridChess:
      result = grid_validate_observation_geometry(si);
      break;

    case STValidateObservationGeometryEdgeMover:
      result = edgemover_validate_observation_geometry(si);
      break;

    case STValidateObservationGeometryImitator:
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

static slice_index const observation_geometry_validation_slice_rank_order[] =
{
    STValidatingObservationGeometry,
    STValidateObservationGeometryMonochrome,
    STValidateObservationGeometryBichrome,
    STValidateObservationGeometryGridChess,
    STValidateObservationGeometryEdgeMover,
    STValidateObservationGeometryImitator,
    STValidatingObservationGeometryByPlayingMove,
    STValidateCheckMoveByPlayingCapture,
    STTrue
};

enum
{
  nr_observation_geometry_validation_slice_rank_order_elmts = sizeof observation_geometry_validation_slice_rank_order / sizeof observation_geometry_validation_slice_rank_order[0]
};

static void observation_geometry_validation_insert_slice(slice_index testing,
                                                         slice_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  insert_slice(testing,
               type,
               observation_geometry_validation_slice_rank_order,
               nr_observation_geometry_validation_slice_rank_order_elmts);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_observation_geometry_validation(slice_index si,
                                                       stip_structure_traversal *st)
{
  instrumentation_type const * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (it->side==nr_sides || it->side==slices[si].starter)
    observation_geometry_validation_insert_slice(si,it->type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
                                           &instrument_observation_geometry_validation);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index const observer_validation_slice_rank_order[] =
{
    STValidatingObserver,
    STMadrasiObserverTester,
    STEiffelObserverTester,
    STParalysingPiecesObserverTester,
    STTrue
};

enum
{
  nr_observer_validation_slice_rank_order = sizeof observer_validation_slice_rank_order / sizeof observer_validation_slice_rank_order[0]
};

/* Validate an observation
 * @return true iff the observation is valid
 */
boolean validate_observer(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(slices[temporary_hack_observer_validator[trait[nbply]]].next2);

  if (result)
    result = validate_observation_recursive(slices[temporary_hack_observation_geometry_validator[trait[nbply]]].next2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void observer_validation_insert_slice(slice_index validating,
                                             slice_type type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",validating);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  insert_slice(validating,
               type,
               observer_validation_slice_rank_order,
               nr_observer_validation_slice_rank_order);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_observer_validation(slice_index si,
                                           stip_structure_traversal *st)
{
  instrumentation_type const * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (it->side==nr_sides || it->side==slices[si].starter)
    observer_validation_insert_slice(si,it->type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
                                           &instrument_observer_validation);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index const observation_validation_slice_rank_order[] =
{
    STValidatingCheck,
    STValidatingObservation,
    STValidatingObservationBackHome,
    STValidatingObservationBeamten,
    STValidatingObservationBGL,
    STValidatingObservationBrunner,
    STValidatingObservationCentral,
    STValidatingObservationDisparate,
    STValidatingObservationGeneva,
    STValidatingObservationImmune,
    STValidatingObservationLortap,
    STValidatingObservationUltraMummer,
    STValidatingObservationPatrol,
    STValidatingObservationUltraPatrol,
    STValidatingObservationProvocateurs,
    STValidatingObservationShielded,
    STValidatingObservationSuperGuards,
    STValidateObservationWoozles,
    STValidateObservationBiWoozles,
    STValidateObservationHeffalumps,
    STValidateObservationBiHeffalumps,
    STValidatingObservationWormholes,
    STValidatingObservationGeometryByPlayingMove,
    STValidateCheckMoveByPlayingCapture,
    STTrue
};

enum
{
  nr_observation_validation_slice_rank_order = sizeof observation_validation_slice_rank_order / sizeof observation_validation_slice_rank_order[0]
};

/* Validate a check
 * @return true iff the observation is valid
 */
boolean validate_check(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = validate_observation_recursive(slices[temporary_hack_check_validator[trait[nbply]]].next2);

  if (result)
  {
    result = validate_observation_recursive(slices[temporary_hack_observer_validator[trait[nbply]]].next2);

    if (result)
      result = validate_observation_recursive(slices[temporary_hack_observation_geometry_validator[trait[nbply]]].next2);
  }

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

  if (result)
  {
    result = validate_observation_recursive(slices[temporary_hack_observer_validator[trait[nbply]]].next2);

    if (result)
      result = validate_observation_recursive(slices[temporary_hack_observation_geometry_validator[trait[nbply]]].next2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
  stip_structure_traversal_override_single(&st,
                                           STValidatingCheck,
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

static void optimise_side(slice_index si, Side side)
{
  slice_index const proxy = slices[temporary_hack_is_square_observed[side]].next2;
  slice_index const testing = slices[proxy].next1;
  slice_index const tracking_back_king = slices[testing].next1;
  slice_index const landing = slices[tracking_back_king].next1;
  slice_index const tracking_back = slices[landing].next1;
  slice_index const leaf = slices[tracking_back].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[proxy].type,"");
  TraceEnumerator(slice_type,slices[testing].type,"");
  TraceEnumerator(slice_type,slices[tracking_back_king].type,"");
  TraceEnumerator(slice_type,slices[tracking_back].type,"");
  TraceEnumerator(slice_type,slices[landing].type,"");
  TraceEnumerator(slice_type,slices[leaf].type,"\n");
  if (slices[testing].type==STTestingIfSquareIsObserved
      && slices[tracking_back_king].type==STFindSquareObserverTrackingBackKing
      && slices[landing].type==STLandingAfterFindSquareObserverTrackingBackKing
      && slices[tracking_back].type==STFindSquareObserverTrackingBack
      && slices[leaf].type==STFalse)
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
  if (slices[slices[slices[slices[temporary_hack_check_validator[White]].next2].next1].next1].type==STTrue
      && slices[slices[slices[slices[temporary_hack_observation_validator[White]].next2].next1].next1].type==STTrue
      && slices[slices[slices[slices[temporary_hack_observation_geometry_validator[White]].next2].next1].next1].type==STTrue
      && slices[slices[slices[slices[temporary_hack_observer_validator[White]].next2].next1].next1].type==STTrue)
    optimise_side(si,White);

  if (slices[slices[slices[slices[temporary_hack_check_validator[Black]].next2].next1].next1].type==STTrue
      && slices[slices[slices[slices[temporary_hack_observation_validator[Black]].next2].next1].next1].type==STTrue
      && slices[slices[slices[slices[temporary_hack_observation_geometry_validator[Black]].next2].next1].next1].type==STTrue
      && slices[slices[slices[slices[temporary_hack_observer_validator[Black]].next2].next1].next1].type==STTrue)
    optimise_side(si,Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean is_observation_trivially_validated(Side side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  /* ... and this when the proxy slices are removed */
  result = (slices[slices[temporary_hack_observation_validator[side]].next2].type==STTrue
            && slices[slices[temporary_hack_observer_validator[side]].next2].type==STTrue
            && slices[slices[temporary_hack_observation_geometry_validator[side]].next2].type==STTrue);

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

    case STAnnanIsSquareObserved:
      result = annan_is_square_observed(si,evaluate);
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
    STAnnanIsSquareObserved,
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
