#if !defined(PIECES_PAWNS_PROMOTION_H)
#define PIECES_PAWNS_PROMOTION_H

#include "solving/machinery/solve.h"
#include "solving/move_effect_journal.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/structure_traversal.h"
#include "position/board.h"

/* This module provides implements the promotion of the moving pawn
 */

enum
{
  /* we calculate with:*/
  max_nr_promotions_per_ply = 4
};

/* effects up to this index have been consumed for pawn promotions */
extern move_effect_journal_index_type promotion_horizon[maxply+1];

/* Find the last square occupied by a piece since we last checked.
 * @param base index of move effects that have already been dealt with
 * @param candidate to hold the square; initsquare if there isn't any
 * @param as_side for whom did the pawn reach *candidate?
 */
void find_potential_promotion_square(move_effect_journal_index_type base,
                                     square *candidate,
                                     Side *as_side);

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
void pawn_promoter_solve(slice_index si);

/* Initialise a structure traversal for inserting slices
 * into the promotion execution sequence
 * @param st address of structure representing the traversal
 */
void promotion_init_slice_insertion_traversal(stip_structure_traversal *st);

/* Insert slices into a promotion execution slices sequence.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by promotion_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param context initial context of the insertion traversal; typically the
 *                current context of a surrounding traversal that has arrived
 *                at slice si
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void promotion_insert_slices(slice_index si,
                             stip_traversal_context_type context,
                             slice_index const prototypes[],
                             unsigned int nr_prototypes);

/* Instrument all promotion slice sequences of the solving machinery with the
 * default promotion behavior
 * @param si identifies root slice of the solving machinery
 */
void promotion_instrument_solving_default(slice_index si);

/* Insert (the boundaries of) a promotion slice sequence into the solving
 * machinery
 * @param si identifies the root slice of the solving machinery
 * @param insertion_point type of insertion point slices
 * @param inserter slice insertion function for inserting from insertion_point
 *                 slices
 */
void promotion_insert_slice_sequence(slice_index si,
                                     slice_type insertion_point,
                                     slice_inserter_contextual_type inserter);

#endif
