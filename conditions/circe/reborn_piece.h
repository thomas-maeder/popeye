#if !defined(CONDITIONS_CIRCE_REBORN_PIECE_H)
#define CONDITIONS_CIRCE_REBORN_PIECE_H

/* This module deals with pieces reborn by Circe variants
 */

#include "solving/machinery/solve.h"

typedef enum
{
  circe_reborn_walk_adapter_none,
  circe_reborn_walk_adapter_clone,
  circe_reborn_walk_adapter_chameleon,
  circe_reborn_walk_adapter_einstein,
  circe_reborn_walk_adapter_reversaleinstein
} circe_reborn_walk_adapter_type;

struct circe_variant_type;

/* Instrument the solving machinery with the logic for determining which piece
 * is reborn in a Circe variant
 * @param si root slice of the solving machinery
 * @param variant address of the structure representing the Circe variant
 * @param interval_start start of the slice sequence representing the variant
 */
void circe_solving_instrument_reborn_piece(slice_index si,
                                           struct circe_variant_type const *variant,
                                           slice_type interval_start);

/* Override the reborn walk adapter of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_reborn_walk_adapter(struct circe_variant_type *variant,
                                           circe_reborn_walk_adapter_type adapter);

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
void circe_initialise_reborn_from_capturee_solve(slice_index si);

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
void circe_initialise_reborn_from_capturer_solve(slice_index si);

#endif
