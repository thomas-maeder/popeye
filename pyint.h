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

/* How well does the stipulation support intelligent mode?
 */
typedef enum
{
  intelligent_not_supported,
  intelligent_not_active_by_default,
  intelligent_active_by_default
} support_for_intelligent_mode;

support_for_intelligent_mode stip_supports_intelligent(void);

/* Instrument stipulation with STGoalreachableGuard slices
 */
void stip_insert_goalreachable_guards(void);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean goalreachable_guard_help_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean goalreachable_guard_help_has_solution_in_n(slice_index si,
                                              stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void goalreachable_guard_help_solve_continuations_in_n(table continuations,
                                                  slice_index si,
                                                  stip_length_type n);

#endif
