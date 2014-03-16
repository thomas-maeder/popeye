#if !defined(PIECES_PAWNS_PROMOTION_H)
#define PIECES_PAWNS_PROMOTION_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"
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
extern move_effect_journal_index_type promotion_horizon;

/* Find the last square occupied by a piece since we last checked.
 * @param base index of move effects that have already been dealt with
 * @return the square; initsquare if there isn't any
 */
square find_potential_promotion_square(move_effect_journal_index_type base);

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
stip_length_type pawn_promoter_solve(slice_index si, stip_length_type n);

/* Start inserting according to the slice type order for promotion execution
 * @param si identifies starting point of insertion
 * @param st insertion traversal where we come from and will return to
 * @param end_of_factored_order slice type where to return to insertion defined
 *                              by st
 */
void start_insertion_according_to_promotion_order(slice_index si,
                                                  stip_structure_traversal *st,
                                                  slice_type end_of_factored_order);

/* Determine whether a slice type contributes to the execution of moves
 * @param type slice type
 * @return true iff type is a slice type that contributes to the execution of moves
 */
boolean is_promotion_slice_type(slice_type type);

/* Instrument the solving machinery with the promotion of something other than
 * the moving piece
 * @param si identifies the root slice of the solving machinery
 * @param hook_type type of slice marking a position where pawn promotion is
 *                  required
 */
void pieces_pawns_promotion_insert_solvers(slice_index si, slice_type hook_type);

/* Try to start slice insertion within the sequence of slices that deal with
 * pawn promotion.
 * @param base_type type relevant for determining the position of the slices to
 *                  be inserted
 * @param si identifies the slice where to actually start the insertion traversal
 * @param st address of the structure representing the insertion traversal
 * @return true iff base_type effectively is a type from the move slices sequence
 */
boolean promotion_start_insertion(slice_type base_type,
                                  slice_index si,
                                  stip_structure_traversal *st);

/* Initialise a structure traversal for inserting slices
 * into the promotion execution sequence
 * @param st address of structure representing the traversal
 */
void promotion_init_slice_insertion_traversal(stip_structure_traversal *st);

#endif
