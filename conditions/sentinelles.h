#if !defined(CONDITION_SENTINELLES_H)
#define CONDITION_SENTINELLES_H

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"

/* This module implements Echecs Sentinelles.
 */

extern unsigned int sentinelles_max_nr_pawns[nr_sides];
extern unsigned int sentinelles_max_nr_pawns_total;

extern piece_walk_type sentinelle_walk;

typedef enum
{
  sentinelles_pawn_propre,
  sentinelles_pawn_adverse,
  sentinelles_pawn_neutre,

  nr_sentinelles_pawn_modes
} sentinelles_pawn_mode_type;

extern sentinelles_pawn_mode_type sentinelles_pawn_mode;

extern boolean sentinelles_is_para;

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void sentinelles_inserter_solve(slice_index si);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_sentinelles_inserters(slice_index si);

#endif
