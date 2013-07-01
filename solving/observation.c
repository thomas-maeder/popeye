#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

boolean (*next_observation_validator)(square sq_observer, square sq_landing, square sq_observee) = &validate_observation;

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
