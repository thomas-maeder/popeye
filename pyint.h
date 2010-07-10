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

extern boolean isIntelligentModeActive;


boolean isGoalReachable(void);

boolean IntelligentHelp(slice_index si, stip_length_type n);

boolean IntelligentSeries(slice_index si, stip_length_type n);

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @return false iff the user asks for intelligent mode, but the
 * stipulation doesn't support it
 */
boolean init_intelligent_mode(void);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_help_solve_in_n(slice_index si,
                                                     stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type
goalreachable_guard_help_has_solution_in_n(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_series_solve_in_n(slice_index si,
                                                       stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type
goalreachable_guard_series_has_solution_in_n(slice_index si,
                                             stip_length_type n);

#endif
