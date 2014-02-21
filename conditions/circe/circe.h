#if !defined(CONDITIONS_CIRCE_CIRCE_H)
#define CONDITIONS_CIRCE_CIRCE_H

/* This module implements Circe
 */

#include "pieces/pieces.h"
#include "solving/solve.h"
#include "stipulation/slice_type.h"
#include "solving/move_effect_journal.h"
#include "solving/ply.h"
#include "position/position.h"

typedef struct
{
    PieNam reborn_walk;
    Flags reborn_spec;
    square rebirth_square;
    PieNam relevant_walk;
    Flags relevant_spec;
    square relevant_square;
    Side relevant_side;
    move_effect_reason_type rebirth_reason;
} circe_rebirth_context_elmt_type;

extern circe_rebirth_context_elmt_type circe_rebirth_context_stack[maxply+1];

typedef unsigned int circe_rebirth_context_index;

extern circe_rebirth_context_index circe_rebirth_context_stack_pointer;

/* Find the Circe rebirth effect in the current move
 * @return the index of the rebirth effect
 *         move_effect_journal_base[nbply+1] if there is none
 */
move_effect_journal_index_type circe_find_current_rebirth(void);

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
stip_length_type circe_determine_reborn_piece_solve(slice_index si,
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
stip_length_type circe_determine_rebirth_square_solve(slice_index si,
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
stip_length_type circe_test_reborn_existance_solve(slice_index si, stip_length_type n);

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
stip_length_type circe_test_rebirth_square_empty_solve(slice_index si, stip_length_type n);

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
stip_length_type circe_place_reborn_solve(slice_index si, stip_length_type n);

/* Instrument the solving machinery with Circe
 * @param si identifies root slice of stipulation
 */
void circe_initialise_solving(slice_index si);

/* Instrument the Circe solving machinery with some slice
 * @param si identifies root slice of stipulation
 * @param type slice type of which to add instances
 */
void circe_instrument_solving(slice_index si, slice_type type);

/* Instrument Circe rebirths with pawn promotion
 * @param si root slice
 */
void circe_allow_pawn_promotion(slice_index si);

square rennormal_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegel_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rendiagramm_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renantipoden_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rensymmetrie_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renfile_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegelfile_polymorphic(PieNam p, Flags pspec, square j, square i, square ip, Side camp);

square rennormal(PieNam pnam_captured, Flags p_captured_spec,
                 square sq_capture,
                 Side capturer);
square renfile(PieNam p_captured, square sq_capture, Side capturer);


#endif
