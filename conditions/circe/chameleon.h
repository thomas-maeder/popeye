#if !defined(CONDITIONS_CIRCE_CHAMELEON_H)
#define CONDITIONS_CIRCE_CHAMELEON_H

/* This module implements Chameleon Circe */

#include "solving/solve.h"

/* Reset the mapping from captured to reborn pieces
 */
void chameleon_circe_reset_reborn_pieces(void);

/* Initialise one mapping captured->reborn from an explicit indication
 * @param captured captured piece
 * @param reborn type of reborn piece if a piece of type captured is captured
 */
void chameleon_circe_set_reborn_piece_explicit(PieNam captured, PieNam reborn);

/* Initialise the reborn pieces if they haven't been already initialised
 * from explicit indications
 */
void chameleon_circe_init_implicit(void);

/* What kind of piece is to be reborn if a certain piece is captured?
 * @param captured kind of captured piece
 * @return kind of piece to be reborn
 */
piece chameleon_circe_get_reborn_piece(piece captured);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type chameleon_circe_adapt_reborn_piece_solve(slice_index si,
                                                          stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_chameleon_circe(slice_index si);

#endif
