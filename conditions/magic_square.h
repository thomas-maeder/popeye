#if !defined(CONDITIONS_MAGIC_SQUARE_H)
#define CONDITIONS_MAGIC_SQUARE_H

#include "solving/solve.h"

/* This module implements the condition Magic squares */


typedef enum
{
  magic_square_type1,
  magic_square_type2
} magic_square_type_type;

extern magic_square_type_type magic_square_type;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type magic_square_side_changer_solve(slice_index si,
                                                  stip_length_type n);

/* Instrument slices with magic square side changers
 */
void stip_insert_magic_square(slice_index si);

#endif
