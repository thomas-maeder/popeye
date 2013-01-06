#if !defined(PIECES_NEUTRAL_INITIALISER_H)
#define PIECES_NEUTRAL_INITIALISER_H

#include "solving/solve.h"

/* This module provides slice type STPiecesNeutralInitialiser - initialises
 * neutral pieces to the appropriate side so that the subsequent move
 * generation can generate moves capturing neutral pieces
 */

/* Side that the neutral pieces currently belong to
 */
extern Side neutral_side;

/* Change the side of the piece so that the piece belongs to the neutral side
 * @param p address of piece whose side to change
 */
void setneutre(piece *p);

/* Initialise the neutral pieces to belong to the side to be captured in the
 * subsequent move
 * @param captured_side side of pieces to be captured
 */
void initialise_neutrals(Side captured_side);

/* make sure that the retracting neutrals belong to the right side if our
 * posteriority has changed the neutral side*/
void neutral_initialiser_recolor_retracting(void);
void neutral_initialiser_recolor_replaying(void);

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index alloc_neutral_initialiser_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type neutral_initialiser_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type neutral_retracting_recolorer_solve(slice_index si,
                                                     stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type neutral_replaying_recolorer_solve(slice_index si,
                                                    stip_length_type n);

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_neutral_initialisers(slice_index si);

#endif
