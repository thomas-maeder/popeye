#if !defined(PIECES_ATTRIBUTES_MAGIC_H)
#define PIECES_ATTRIBUTES_MAGIC_H

/* This module implements magic pieces */

#include "solving/solve.h"

typedef struct
{
    int bottom;
    int top;
} magicstate_type;

extern magicstate_type magicstate[maxply + 1];

/* Can a specific type of (fairy) piece be magic?
 * @param p type of piece
 * @return true iff pieces of type p can be magic
 */
boolean magic_is_piece_supported(PieNam p);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type magic_views_initialiser_solve(slice_index si,
                                                stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type magic_pieces_recolorer_solve(slice_index si,
                                               stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_magic_pieces_recolorers(slice_index si);

#endif
