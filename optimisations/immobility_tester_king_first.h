#if !defined(STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_H)
#define STIPULATION_GOAL_IMMOBILE_REACHED_TESTER_H

#include "pystip.h"

/* This module provides an optimisation for detecting whether a side is
 * immobile: king moves are attempted first
 */

/* Substitute king first immobility testers
 * @param si where to start (entry slice into stipulation)
 */
void immobility_testers_substitute_king_first(slice_index si);

#endif
