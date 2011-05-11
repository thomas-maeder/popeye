/******************** MODIFICATIONS to pyint.h **************************
**
** Date       Who  What
**
** 1997/04/04 TLi  Original
**
**************************** End of List ******************************/

#if !defined(PYINT_H)
#define PYINT_H

#include "pyposit.h"

extern int MovesLeft[nr_sides];


boolean isGoalReachable(void);

boolean IntelligentHelp(slice_index si, stip_length_type n);

boolean IntelligentSeries(slice_index si, stip_length_type n);

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @param si identifies slice where to start
 * @return false iff the user asks for intelligent mode, but the
 * stipulation doesn't support it
 */
boolean init_intelligent_mode(slice_index si);

/* Determine whether intelligent mode overrides option movenum
 * @return true iff intelligent mode overrides option movenum
 */
boolean intelligent_mode_overrides_movenbr(void);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_help(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_can_help(slice_index si,
                                              stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_series(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_has_series(slice_index si,
                                                stip_length_type n);

#endif
