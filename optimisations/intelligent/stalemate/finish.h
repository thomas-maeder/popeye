#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_FINISH_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_FINISH_H

#include "stipulation/stipulation.h"
#include "stipulation/structure_traversal.h"

/* Test the position created by the taken operations; if the position is a
 * solvable target position: solve it; otherwise: improve it
 */
void intelligent_stalemate_test_target_position(slice_index si);

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
void impose_starter_intelligent_stalemate_target_position_tester(slice_index si,
                                                                 stip_structure_traversal *st);

/* Allocate a STIntelligentStalemateTargetPositionTester slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_stalemate_target_position_tester(void);

#endif
