#if !defined(PIECESS_PAWNS_PROMOTION_H)
#define PIECESS_PAWNS_PROMOTION_H

#include "py.h"

typedef enum
{
  pieces_pawns_promotee_chain_orthodox,
  pieces_pawns_promotee_chain_marine,

  pieces_pawns_nr_promotee_chains
} pieces_pawns_promotee_chain_selector_type;

extern PieNam pieces_pawns_promotee_chain[pieces_pawns_nr_promotee_chains][PieceCount];

typedef struct
{
    pieces_pawns_promotee_chain_selector_type selector;
    PieNam promotee;
} pieces_pawns_promotion_sequence_type;

/* Initialise a sequence of promotions
 * @param sq_arrival arrival square of the move
 * @param sequence address of structure to represent the sequence
 * @note If sq_arrival is a promotion square of a side
 *          and sq_arrival is still occupied by a pawn of that side
 *       then *state is initialised with a promotion sequence.
 *       Otherwise, state->promotee will be ==Empty.
 */
void pieces_pawns_initialise_promotion_sequence(square sq_arrival,
                                                pieces_pawns_promotion_sequence_type *sequence);

/* Continue an iteration over a sequence of promotions of a pawn started with an
 * invokation of pieces_pawns_initialise_promotion_sequence().
 * @param sequence address of structure representing the sequence
 * @note assigns state->promotee the value Empty if iteration has ended
 */
void pieces_pawns_continue_promotion_sequence(pieces_pawns_promotion_sequence_type *sequence);

/* Has a pawn reached a promotion square
 * @param side the pawn's side
 * @param square_reached square reached by the pawn
 * @return true iff square_reached is a promotion square
 */
boolean has_pawn_reached_promotion_square(Side side, square square_reached);

/* Initialise the set of promotion pieces for the current twin
 */
void pieces_pawns_init_promotion_pieces(void);

#endif
