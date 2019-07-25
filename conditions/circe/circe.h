#if !defined(CONDITIONS_CIRCE_CIRCE_H)
#define CONDITIONS_CIRCE_CIRCE_H

/* This module implements Circe
 */

#include "pieces/pieces.h"
#include "position/side.h"
#include "position/board.h"
#include "solving/machinery/solve.h"
#include "stipulation/slice_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/slice_insertion.h"
#include "solving/move_effect_journal.h"
#include "solving/ply.h"
#include "solving/machinery/twin.h"
#include "conditions/circe/rebirth_square_occupied.h"
#include "conditions/circe/reborn_piece.h"
#include "conditions/circe/rebirth_square.h"
#include "conditions/circe/relevant_capture.h"
#include "conditions/circe/relevant_piece.h"
#include "conditions/circe/relevant_side.h"

typedef enum
{
  anticirce_type_calvet,
  anticirce_type_cheylan,

  anticirce_type_count
} anticirce_type_type;

typedef struct circe_variant_type
{
    boolean is_rex_inclusive;
    circe_relevant_side_overrider_type relevant_side_overrider;
    circe_rebirth_square_adapter_type rebirth_square_adapter;
    circe_behaviour_on_occupied_rebirth_square_type on_occupied_rebirth_square_default;
    circe_behaviour_on_occupied_rebirth_square_type on_occupied_rebirth_square;
    circe_reborn_walk_adapter_type reborn_walk_adapter;
    boolean is_turncoat;
    boolean do_place_reborn;
    boolean is_promotion_possible;
    circe_relevant_piece default_relevant_piece;
    circe_relevant_piece actual_relevant_piece;
    circe_relevant_capture relevant_capture;
    circe_determine_rebirth_square_type determine_rebirth_square;
    move_effect_reason_type rebirth_reason;
    anticirce_type_type anticirce_type;
    boolean is_restricted_to_walks;
    boolean is_walk_affected[nr_piece_walks];
    twin_id_type explicit_chameleon_squence_set_in_twin;
    piece_walk_type chameleon_walk_sequence[nr_piece_walks];
} circe_variant_type;

extern circe_variant_type circe_variant;

typedef struct
{
    ply relevant_ply;
    piece_walk_type reborn_walk;
    Flags reborn_spec;
    square rebirth_square;
    piece_walk_type relevant_walk;
    Flags relevant_spec;
    square relevant_square;
    Side relevant_side;
    square rebirth_from;
    Side rebirth_as;
} circe_rebirth_context_elmt_type;

extern circe_rebirth_context_elmt_type circe_rebirth_context_stack[maxply+1];

typedef unsigned int circe_rebirth_context_index;

extern circe_rebirth_context_index circe_rebirth_context_stack_pointer;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void circe_reset_variant(circe_variant_type *variant);

/* Find the Circe rebirth effect in the current move
 * @return the index of the rebirth effect
 *         move_effect_journal_base[nbply+1] if there is none
 */
move_effect_journal_index_type circe_find_current_rebirth(void);

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
void circe_determine_rebirth_square_solve(slice_index si);

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
void circe_place_reborn_solve(slice_index si);

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
void circe_done_with_rebirth(slice_index si);

/* Instrument the solving machinery with Circe
 * @param si identifies the root slice of the solving machinery
 * @param variant address of the structure holding the details of the Circe variant
 * @param what what exactly is being instrumented?
 * @param inserter slice insertion function for inserting from what slices
 * @param interval_start start of the slices interval where to instrument
 */
void circe_initialise_solving(slice_index si,
                              circe_variant_type *variant,
                              slice_type what,
                              slice_inserter_contextual_type inserter,
                              slice_type interval_start);

/* Instrument the Circe solving machinery with some slice
 * @param si identifies root slice of stipulation
 * @param interval_start start of the slices interval where to instrument
 * @param outside_mark end of the slices interval where to instrument
 * @param hook_type insertion is tried at each slice of this type
 * @param prototype prototype of type of which to add instances
 * @note circe_instrument_solving() assumes ownership of prototype
 */
void circe_instrument_solving(slice_index si,
                              slice_type interval_start,
                              slice_type hook_type,
                              slice_index prototype);

/* Allocate a Circe handler slice
 * @param type type of Circe handler slice
 * @param variant address of Circe variant structure to be used for parametrising
 * @return the allocated pipe
 */
slice_index alloc_circe_handler_slice(slice_type type,
                                      circe_variant_type const *variant);

/* Initialise a structure traversal for inserting slices
 * into the Circe execution sequence
 * @param st address of structure representing the traversal
 */
void circe_init_slice_insertion_traversal(stip_structure_traversal *st);

/* Insert slices into a Circe execution slices sequence.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by circe_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param context initial context of the insertion traversal; typically the
 *                current context of a surrounding traversal that has arrived
 *                at slice si
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void circe_insert_slices(slice_index si,
                         stip_traversal_context_type context,
                         slice_index const prototypes[],
                         unsigned int nr_prototypes);

#endif
