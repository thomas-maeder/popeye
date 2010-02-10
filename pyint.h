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
#include "pytable.h"

extern int MovesLeft[nr_sides];

extern boolean isIntelligentModeActive;


boolean isGoalReachable(void);
boolean SolAlreadyFound(void);
void StoreSol(void);
boolean Intelligent(slice_index si,
                    stip_length_type n,
                    stip_length_type full_length);

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @return false iff the user asks for intelligent mode, but the
 * stipulation doesn't support it
 */
boolean init_intelligent_mode(void);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean goalreachable_guard_help_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean goalreachable_guard_help_has_solution_in_n(slice_index si,
                                              stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void goalreachable_guard_help_solve_threats_in_n(table threats,
                                                 slice_index si,
                                                 stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean goalreachable_guard_series_solve_in_n(slice_index si,
                                              stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean goalreachable_guard_series_has_solution_in_n(slice_index si,
                                              stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void goalreachable_guard_series_solve_threats_in_n(table threats,
                                                   slice_index si,
                                                   stip_length_type n);

#endif
