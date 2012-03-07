#if !defined(STIPULATION_CONDITION_OHNESCHACH_IMMOBILE_TESTER_H)
#define STIPULATION_CONDITION_OHNESCHACH_IMMOBILE_TESTER_H

#include "pyslice.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

/* Replace immobility tester slices to cope with condition Ohneschach
 * @param si where to start (entry slice into stipulation)
 */
void ohneschach_replace_immobility_testers(slice_index si);

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_suspender_help(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_check_guard_help(slice_index si,
                                             stip_length_type n);

/* Determine whether the move just played has led to a legal position according
 * to condition Ohneschach
 * @param just_moved identifies the side that has just moved
 * @return true iff the position reached is legal according to Ohneschach
 */
boolean ohneschach_pos_legal(Side just_moved);

#endif
