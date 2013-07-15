#if !defined(SOLVING_OBSERVATION_H)
#define SOLVING_OBSERVATION_H

#include "pieces/walks/vectors.h"
#include "pyproc.h"

/* This module provides supports observation as used by many conditions and
 * piece attributes. This includes
 * - 1st degree:
 *   * check (observation of the enemy king)
 *   * Madrasi paralysis
 *   * Patrol Chess observation
 *   * ...
 * - 2nd degree
 *   * Madrasi paralysis of a Patrol chess observer
 *   * ...
 * - 3rd degree
 *   * observation in Monochrome Chess and the like
 *   * ...
 */

extern boolean (*next_observation_validator)(square sq_observer, square sq_landing, square sq_observee);

extern vec_index_type interceptable_observation_vector_index[maxply+1];
extern unsigned int observation_context;

/* Forget about the observation validators registered in a previous round of
 * solving.
 */
void reset_observation_geometry_validators(void);

/* Register an observation validator for the next round of solving
 * @param validator validator to be registered
 */
void register_observation_geometry_validator(evalfunction_t *validator);

/* Validate an observation
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean validate_observation_geometry(square sq_observer,
                                      square sq_landing,
                                      square sq_observee);


/* Forget about the observation validators registered in a previous round of
 * solving.
 */
void reset_observer_validators(void);

/* Register an observer validator for the next round of solving
 * @param validator validator to be registered
 */
void register_observer_validator(evalfunction_t *validator);

/* Validate an observation
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean validate_observer(square sq_observer,
                          square sq_landing,
                          square sq_observee);


/* Forget about the observation validators registered in a previous round of
 * solving.
 */
void reset_observation_validators(void);

/* Register an observation validator for the next round of solving
 * @param validator validator to be registered
 */
void register_observation_validator(evalfunction_t *validator);

/* Retrieve the number of observation validators registered since program start
 * or the last invokation of reset_observation_validators()
 * @return number of registered observation validators
 */
unsigned int get_nr_observation_validators(void);

/* Validate an observation
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean validate_observation(square sq_observer,
                             square sq_landing,
                             square sq_observee);

#endif
