#if !defined(CONDITIONS_POINTREFLECTION_H)
#define CONDITIONS_POINTREFLECTION_H

#include "conditions/conditions.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module implements the condition Point Reflection */

/* Make sure that the observer has the expected walk - point_reflectionised or originally
 * @return true iff the observation is valid
 */
boolean point_reflection_enforce_observer_walk(slice_index si);

/* Temporarily change walks of oppenent's pieces before move generation
 * @param identifies generator slice
 */
void point_reflection_temporarily_change_walks(slice_index si);

/* Resotre walks of oppenent's pieces
 * @param identifies generator slice
 */
void point_reflection_restore_walks(slice_index si);

/* Inialise the solving machinery with Annan Chess
 * @param si identifies root slice of solving machinery
 */
void point_reflection_initialise_solving(slice_index si);

#endif
