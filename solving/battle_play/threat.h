#if !defined(SOLVING_BATTLE_PLAY_THREAT_H)
#define SOLVING_BATTLE_PLAY_THREAT_H

#include "stipulation/structure_traversal.h"
#include "solving/machinery/solve.h"
#include "solving/ply.h"
#include "utilities/table.h"

/* This module provides functionality dealing with threats
 */

/* Table where threats of the various move levels are collected
 */
extern table threats[maxply+1];

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
void threat_enforcer_solve(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
void threat_collector_solve(slice_index si);

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
void threat_defeated_tester_solve(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 no solution found
 */
void threat_solver_solve(slice_index si);

/* Instrument the stipulation representation so that it can solve and enforce
 * threats
 * @param si identifies slice where to start
 */
void solving_insert_threat_handlers(slice_index si);

/* Instrument the stipulation representation with proxy slices marking the
 * beginning and end of the threat
 * @param si identifies slice where to start
 */
void solving_insert_threat_boundaries(slice_index si);

/* Instrument the solving machinery so that it can cope with conditions that
 * allow the defending side's dummy move that starts a threat to have effects.
 * @param si identifies the slice where to start
 */
void solving_threat_instrument_for_dummy_move_effects(slice_index si);

#endif
