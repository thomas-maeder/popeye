#if !defined(OPTIMISATION_INTELLIGENT_DUPLICATE_AVOIDER_H)
#define OPTIMISATION_INTELLIGENT_DUPLICATE_AVOIDER_H

#include "solving/battle_play/attack_play.h"

/* This module provides the slice type STIntelligentDuplicateAvoider
 * which avoids producing certain solutions twice in intelligent mode
 */

/* Initialise the duplication avoidance machinery
 */
void intelligent_duplicate_avoider_init(void);

/* Cleanup the duplication avoidance machinery
 */
void intelligent_duplicate_avoider_cleanup(void);

/* Allocate a STIntelligentDuplicateAvoider slice.
 * @return index of allocated slice
 */
slice_index alloc_intelligent_duplicate_avoider_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type intelligent_duplicate_avoider_attack(slice_index si,
                                                      stip_length_type n);

#endif
