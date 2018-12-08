#if !defined(SOLVING_OBSERVATION_H)
#define SOLVING_OBSERVATION_H

#include "stipulation/stipulation.h"
#include "solving/machinery/dispatch.h"
#include "pieces/walks/vectors.h"
#include "solving/ply.h"
#include "debugging/trace.h"

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

typedef struct
{
    vec_index_type vector_index1;
    vec_index_type vector_index2;
    int auxiliary;
} interceptable_observation_type;
extern interceptable_observation_type interceptable_observation[maxply+1];
extern unsigned int observation_context;

/* Continue validating an observation (or observer or observation geometry)
 * @param si identifies the slice with which to continue
 * @return rue iff the observation is valid
 */
boolean validate_observation_recursive(slice_index si);

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

/* Cause observations to be validated by playing the move representing the
 * observation.
 * @param si root slice of solving machinery
 * @param side for which side (pass nr_sides to indicate both sides)
 */
void observation_play_move_to_validate(slice_index si, Side side);

/* version with function pointers
typedef boolean (*validator_id)(void);
#define EVALUATE(key) (&validate_##key)
#define INVOKE_EVALUATE(validator) ((*validator)())
*/

/* version with slice indices */
#include "solving/temporary_hacks.h"
typedef slice_index validator_id;
#include "solving/fork.h"
#define EVALUATE(key) (temporary_hack_##key##_validator[trait[nbply]])
#define INVOKE_EVALUATE(validator) (fork_validate_observation_recursive_delegate(validator))

#define EVALUATE_OBSERVATION(evaluate,sq_departure,sq_arrival) \
  ( TSTFLAG(being_solved.spec[sq_departure],trait[nbply]) \
    && ( move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure = (sq_departure), \
         move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival = (sq_arrival), \
         INVOKE_EVALUATE(evaluate)) \
  )

extern validator_id observation_validator;
extern boolean observation_result;

/* Determine whether a square is observed be the side at the move
 * @return true iff the target square is observed
 */
boolean is_square_observed(validator_id evaluate);

/* Determine whether a square is observed by a side
 * @param side observing side
 * @param s the square
 * @param evaluate identifies the set of restrictions imposed on the observation
 * @return true iff the square is observed
 */
boolean is_square_observed_general(Side side, square s, validator_id evaluate);

/* Perform a nested observation validation run from within an observation
 * validation run
 * Restores observation_validator and observation_result to their previous
 * values before returning.
 * @return true iff the target square is observed
 */
boolean is_square_observed_nested(slice_index si, validator_id evaluate);

#if defined(DOTRACE)
#define is_square_observed_recursive(si) \
  dispatch(si), \
  TraceValue("%u",observation_result), TraceEOL()
#else
#define is_square_observed_recursive(si) dispatch(si)
#endif

/* Determine whether a side observes a specific square
 * @param si identifies the tester slice
 * @note sets observation_result
 */
void is_square_observed_two_paths(slice_index si);

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
                                      unsigned int nr_prototypes);

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


/* Instrument the square observation testing machinery so that there are two
 * paths which can be adapted separately.
 * @param si root slice of solving machinery
 * @param side side for which to instrument; pass nr_sides for both sides
 * @note inserts proxy slices STIsSquareObservedStandardPath and
 *       STIsSquareObservedAlternativePath that can be used for adjusting the move
 *       generation
 */
void is_square_observed_instrument_for_alternative_paths(slice_index si,
                                                         Side side);

#endif
