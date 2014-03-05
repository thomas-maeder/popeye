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

typedef enum
{
  circe_relevant_capture_thismove,
  circe_relevant_capture_lastmove
} circe_relevant_capture;

typedef enum
{
  circe_relevant_piece_capturee,
  circe_relevant_piece_capturer
} circe_relevant_piece;

typedef enum
{
  circe_determine_rebirth_square_from_pas,
  circe_determine_rebirth_square_symmetry,
  circe_determine_rebirth_square_diagram,
  circe_determine_rebirth_square_pwc,
  circe_determine_rebirth_square_rank,
  circe_determine_rebirth_square_file,
  circe_determine_rebirth_square_equipollents,
  circe_determine_rebirth_square_cage,
  circe_determine_rebirth_square_antipodes,
  circe_determine_rebirth_square_super,
  circe_determine_rebirth_square_take_and_make,
  circe_determine_rebirth_square_april
} circe_determine_rebirth_square_type;

typedef enum
{
  circe_on_occupied_rebirth_square_default_no_rebirth,
  circe_on_occupied_rebirth_square_default_illegal
} circe_default_behaviour_on_occupied_rebirth_square_type;

typedef enum
{
  circe_on_occupied_rebirth_square_default,
  circe_on_occupied_rebirth_square_assassinate
} circe_behaviour_on_occupied_rebirth_square_type;

typedef enum
{
  circe_reborn_walk_adapter_none,
  circe_reborn_walk_adapter_clone,
  circe_reborn_walk_adapter_chameleon
} circe_reborn_walk_adapter_type;

typedef enum
{
  anticirce_type_cheylan,
  anticirce_type_calvet,

  anticirce_type_count
} anticirce_type_type;

typedef struct
{
    boolean is_rex_inclusive;
    boolean is_mirror;
    boolean is_diametral;
    circe_default_behaviour_on_occupied_rebirth_square_type on_occupied_rebirth_square_default;
    circe_behaviour_on_occupied_rebirth_square_type on_occupied_rebirth_square;
    circe_reborn_walk_adapter_type reborn_walk_adapter;
    boolean is_turncoat;
    boolean is_promotion_possible;
    circe_relevant_piece relevant_piece;
    circe_relevant_capture relevant_capture;
    circe_determine_rebirth_square_type determine_rebirth_square;
    boolean is_frischauf;
    move_effect_reason_type rebirth_reason;
    anticirce_type_type anticirce_type;
} circe_variant_type;

extern circe_variant_type circe_variant;

typedef struct
{
    PieNam reborn_walk;
    Flags reborn_spec;
    square rebirth_square;
    PieNam relevant_walk;
    Flags relevant_spec;
    square relevant_square;
    Side relevant_side;
} circe_rebirth_context_elmt_type;

extern circe_rebirth_context_elmt_type circe_rebirth_context_stack[maxply+1];

typedef unsigned int circe_rebirth_context_index;

extern circe_rebirth_context_index circe_rebirth_context_stack_pointer;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void circe_reset_variant(circe_variant_type *variant);

/* Override the reborn walk adapter of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_reborn_walk_adapter(circe_variant_type *variant,
                                           circe_reborn_walk_adapter_type adapter);

/* Override the method for determining the rebirth square of a Circe variant object
 * @param adapter the overrider
 * @return true if it hasn't been overridden yet
 */
boolean circe_override_determine_rebirth_square(circe_variant_type *variant,
                                                circe_reborn_walk_adapter_type determine);

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
stip_length_type circe_initialise_from_current_capture_solve(slice_index si,
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

void circe_initialise_solving(slice_index si);

/* Instrument the Circe solving machinery with some slice
 * @param si identifies root slice of stipulation
 * @param type slice type of which to add instances
 */
void circe_instrument_solving(slice_index si, slice_type type);

/* Instrument Circe rebirths with pawn promotion
 * @param si root slice
 */
void circe_allow_pawn_promotion(slice_index si, slice_type hook_type);

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
