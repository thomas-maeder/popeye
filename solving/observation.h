#if !defined(SOLVING_OBSERVATION_H)
#define SOLVING_OBSERVATION_H

#include "stipulation/slice_type.h"
#include "pieces/walks/vectors.h"
#include "py.h"
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

/* Determine whether a square is observed be the side at the move; recursive
 * implementation over various slices
 * @param si identifies next slice
 * @param sq_target the square
 * @return true iff sq_target is observed by the side at the move
 */
boolean is_square_observed_recursive(slice_index si,
                                     square sq_target,
                                     evalfunction_t *evaluate);

/* Determine whether a square is observed be the side at the move
 * @param sq_target the square
 * @return true iff sq_target is observed by the side at the move
 */
boolean is_square_observed(square sq_target, evalfunction_t *evaluate);

/* Instrument a particular square observation testing branch with a slice type
 * @param testing identifies STTestingIfSquareIsObserved at entrance of branch
 * @param type type of slice to insert
 */
void stip_instrument_is_square_observed_insert_slice(slice_index testing,
                                                     slice_type type);

/* Instrument square observation testing with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_is_square_observed_testing(slice_index si,
                                                Side side,
                                                slice_type type);

#endif
