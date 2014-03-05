#if !defined(CONDITIONS_ANTICIRCE_ANTICIRCE_H)
#define CONDITIONS_ANTICIRCE_ANTICIRCE_H

#include "pieces/pieces.h"
#include "solving/solve.h"
#include "solving/ply.h"
#include "solving/move_effect_journal.h"
#include "stipulation/slice_type.h"
#include "position/position.h"
#include "conditions/circe/circe.h"

typedef enum
{
  AntiCirceTypeCheylan,
  AntiCirceTypeCalvet,

  AntiCirceVariantTypeCount
} AntiCirceVariantType;

extern AntiCirceVariantType AntiCirceType;

extern circe_variant_type anticirce_variant;

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
stip_length_type anticirce_determine_reborn_piece_solve(slice_index si,
                                                        stip_length_type n);

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
stip_length_type anticirce_remove_capturer_solve(slice_index si,
                                                 stip_length_type n);

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
stip_length_type anticirce_place_reborn_solve(slice_index si,
                                              stip_length_type n);

/* Initialise solving in Anticirce
 * @param si identifies root slice of stipulation
 */
void anticirce_initialise_solving(slice_index si);

/* Instrument the Anticirce solving machinery with some slice
 * @param si identifies root slice of stipulation
 * @param type slice type of which to add instances
 */
void anticirce_instrument_solving(slice_index si, slice_type type);

#endif
