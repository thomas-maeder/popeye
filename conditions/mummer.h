#if !defined(CONDITIONS_MUMMER_H)
#define CONDITIONS_MUMMER_H

/* This module implements "mummer" conditions (maximummer, minimiummer etc.) */

#include "solving/solve.h"

boolean eval_ultra_mummer_black_king_check(square departure,
                                           square arrival,
                                           square capture);
boolean eval_ultra_mummer_white_king_check(square departure,
                                           square arrival,
                                           square capture);

typedef enum
{
  mummer_strictness_regular,
  mummer_strictness_exact,
  mummer_strictness_ultra,

  nr_mummer_strictness
} mummer_strictness_type;

extern mummer_strictness_type mummer_strictness[nr_sides];

extern mummer_strictness_type mummer_strictness_default_side;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ultra_mummer_measurer_deadend_solve(slice_index si,
                                                     stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type mummer_orchestrator_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type mummer_bookkeeper_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_mummer(slice_index si);

#endif
