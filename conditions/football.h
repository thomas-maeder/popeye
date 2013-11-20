#if !defined(SOLVING_CONDITIONS_FOOTBALL_H)
#define SOLVING_CONDITIONS_FOOTBALL_H

#include "pieces/pieces.h"
#include "solving/solve.h"
#include "solving/ply.h"
#include "position/position.h"
#include "utilities/boolean.h"

/* This module implements Football Chess */

extern PieNam current_football_substitution[maxply+1];
extern boolean is_football_substitute[PieceCount];
extern PieNam next_football_substitute[PieceCount];
extern boolean football_are_substitutes_limited;

/* Initialise the substitutes' bench for the current twin
 */
void init_football_substitutes(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type football_chess_substitutor_solve(slice_index si,
                                                   stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_football_chess(slice_index si);

#endif
