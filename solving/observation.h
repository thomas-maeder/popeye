#if !defined(SOLVING_OBSERVATION_H)
#define SOLVING_OBSERVATION_H

#include "stipulation/slice_type.h"
#include "pieces/walks/vectors.h"
#include "py.h"
#include "pyproc.h"

/* This module provides supports observation as used by many conditions and
 * piece attributes. This includes
 * - 1st degree: the observation
 *   * check (observation of the enemy king)
 *   * Madrasi paralysis
 *   * Patrol Chess observation
 *   * ...
 * - 2nd degree: the observer's observer
 *   * Madrasi paralysis of a Patrol chess observer
 *   * ...
 * - 3rd degree: the observation geometry
 *   * observation in Monochrome Chess and the like
 *   * ...
 */

extern vec_index_type interceptable_observation_vector_index[maxply+1];
extern unsigned int observation_context;

boolean validate_observation_geometry_recursive(slice_index si);

/* Validate an observation
 * @return true iff the observation is valid
 */
boolean validate_observation_geometry(void);

/* Instrument observation geometry validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_observation_geometry_validation(slice_index si,
                                                  Side side,
                                                  slice_type type);

boolean validate_observer_recursive(slice_index si);

/* Validate an observer
 * @return true iff the observation is valid
 */
boolean validate_observer(void);


/* Instrument observer validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_observer_validation(slice_index si,
                                      Side side,
                                      slice_type type);

boolean validate_observation_recursive(slice_index si);

/* Validate an observation
 * @return true iff the observation is valid
 */
boolean validate_observation(void);

/* Instrumenvalidationvation validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_observation_validation(slice_index si,
                                         Side side,
                                         slice_type type);

/* Validate a check
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @return true iff the observation is valid
 */
boolean validate_check(void);

/* Instrument observation validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_check_validation(slice_index si,
                                      Side side,
                                      slice_type type);

boolean is_observation_trivially_validated(Side side);

/* Determine whether a square is observed be the side at the move; recursive
 * implementation over various slices
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean is_square_observed_recursive(slice_index si,
                                     evalfunction_t *evaluate);

/* Determine whether a square is observed be the side at the move
 * @return true iff sq_target is observed by the side at the move
 */
boolean is_square_observed(evalfunction_t *evaluate);

/* Instrument a particular square observation validation branch with a slice type
 * @param testing identifies STTestingIfSquareIsObserved at entrance of branch
 * @param type type of slice to insert
 */
void is_square_observed_insert_slice(slice_index testing,
                                     slice_type type);

/* Instrument square observation validation with a slice type
 * @param identifies where to start instrumentation
 * @param side for which side (pass nr_sides to indicate both sides)
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_is_square_observed_testing(slice_index si,
                                                Side side,
                                                slice_type type);

/* Optimise the square observation machinery if possible
 * @param si identifies the root slice of the solving machinery
 */
void optimise_is_square_observed(slice_index si);

#endif
