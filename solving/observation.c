#include "solving/observation.h"
#include "conditions/annan.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/plus.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/singlebox/type3.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

boolean (*next_observation_validator)(square sq_observer, square sq_landing, square sq_observee) = &validate_observation;

vec_index_type interceptable_observation_vector_index[maxply+1];
unsigned int observation_context = 0;

enum
{
  observation_validators_capacity = 10
};

static evalfunction_t *observation_geometry_validators[observation_validators_capacity];
static unsigned int nr_observation_geometry_validators;

/* Forget about the observation validators registered in a previous round of
 * solving.
 */
void reset_observation_geometry_validators(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_observation_geometry_validators = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Register an observation validator for the next round of solving
 * @param validator validator to be registered
 */
void register_observation_geometry_validator(evalfunction_t *validator)
{
  assert(nr_observation_geometry_validators<observation_validators_capacity);
  observation_geometry_validators[nr_observation_geometry_validators] = validator;
  ++nr_observation_geometry_validators;
}

/* Validate an observation
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean validate_observation_geometry(square sq_observer,
                                      square sq_landing,
                                      square sq_observee)
{
  boolean result = true;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_observation_geometry_validators);

  for (i = 0; i!=nr_observation_geometry_validators; ++i)
    if (!(*observation_geometry_validators[i])(sq_observer,sq_landing,sq_observee))
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static evalfunction_t *observer_validators[observation_validators_capacity];
static unsigned int nr_observer_validators;

/* Forget about the observer validators registered in a previous round of
 * solving.
 */
void reset_observer_validators(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_observer_validators = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Register an observer validator for the next round of solving
 * @param validator validator to be registered
 */
void register_observer_validator(evalfunction_t *validator)
{
  assert(nr_observer_validators<observation_validators_capacity);
  observer_validators[nr_observer_validators] = validator;
  ++nr_observer_validators;
}

/* Validate an observation
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean validate_observer(square sq_observer,
                          square sq_landing,
                          square sq_observee)
{
  boolean result = true;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_observer_validators);

  next_observation_validator = &validate_observation_geometry;

  for (i = 0; i!=nr_observer_validators; ++i)
    if (!(*observer_validators[i])(sq_observer,sq_landing,sq_observee))
    {
      result = false;
      break;
    }

  next_observation_validator = &validate_observer;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static evalfunction_t *observation_validators[observation_validators_capacity];
static unsigned int nr_observation_validators;

/* Forget about the observation validators registered in a previous round of
 * solving.
 */
void reset_observation_validators(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_observation_validators = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Register an observation validator for the next round of solving
 * @param validator validator to be registered
 */
void register_observation_validator(evalfunction_t *validator)
{
  assert(nr_observation_validators<observation_validators_capacity);
  observation_validators[nr_observation_validators] = validator;
  ++nr_observation_validators;
}

/* Retrieve the number of observation validators registered since program start
 * or the last invokation of reset_observation_validators()
 * @return number of registered observation validators
 */
unsigned int get_nr_observation_validators(void)
{
  return nr_observation_validators;
}

/* Validate an observation
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean validate_observation(square sq_observer,
                             square sq_landing,
                             square sq_observee)
{
  boolean result = true;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_observation_validators);

  next_observation_validator = &validate_observer;

  for (i = 0; i!=nr_observation_validators; ++i)
    if (!(*observation_validators[i])(sq_observer,sq_landing,sq_observee))
    {
      result = false;
      break;
    }

  next_observation_validator = &validate_observation;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_square_observed(slice_index si,
                           square sq_target,
                           evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");

  switch (slices[si].type)
  {
    case STSingleBoxType3IsSquareObserved:
      result = singleboxtype3_is_square_observed(si,sq_target,evaluate);
      break;

    case STAnnanIsSquareObserved:
      result = annan_is_square_observed(si,sq_target,evaluate);
      break;

    case STPhantomIsSquareObserved:
      result = phantom_is_square_observed(si,sq_target,evaluate);
      break;

    case STPlusIsSquareObserved:
      result = plus_is_square_observed(si,sq_target,evaluate);
      break;

    case STMarsIsSquareObserved:
      result = marscirce_is_square_observed(si,sq_target,evaluate);
      break;

    case STFindSquareObserverTrackingBackKing:
      result = find_square_observer_tracking_back_from_target_king(si,sq_target,evaluate);
      break;

    case STFindSquareObserverTrackingBack:
      result = find_square_observer_tracking_back_from_target(si,sq_target,evaluate);
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

static slice_index const slice_rank_order[] =
{
    STTestingIfSquareIsObserved,
    STSingleBoxType3IsSquareObserved,
    STAnnanIsSquareObserved,
    STPhantomIsSquareObserved,
    STPlusIsSquareObserved,
    STMarsIsSquareObserved,
    STFindSquareObserverTrackingBackKing,
    STFindSquareObserverTrackingBack,
    STTrue
};

enum
{
  nr_slice_rank_order_elmts = sizeof slice_rank_order / sizeof slice_rank_order[0]
};

static void insert_slice(slice_index testing, slice_type type)
{
  slice_index const prototype = alloc_pipe(type);
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    &prototype,1,
    slice_rank_order, nr_slice_rank_order_elmts,
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

static void instrument_testing(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    insert_slice(si,*type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument square observation testing with a slice type
 * @param identifies where to start instrumentation
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_is_square_observed_testing(slice_index si,
                                                slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,
                                           STTestingIfSquareIsObserved,
                                           &instrument_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
