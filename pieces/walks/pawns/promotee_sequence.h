#if !defined(PIECES_PAWNS_PROMOTEE_SEQUENCE_H)
#define PIECES_PAWNS_PROMOTEE_SEQUENCE_H

#include "pieces/pieces.h"
#include "position/position.h"
#include "utilities/boolean.h"

typedef enum
{
  pieces_pawns_promotee_chain_orthodox,
  pieces_pawns_promotee_chain_marine,

  pieces_pawns_nr_promotee_chains
} pieces_pawns_promotee_sequence_selector_type;

extern PieNam pieces_pawns_promotee_sequence[pieces_pawns_nr_promotee_chains][PieceCount];

typedef struct
{
    pieces_pawns_promotee_sequence_selector_type selector;
    PieNam promotee;
} pieces_pawns_promotion_sequence_type;

#define ForwardPromSq(col,sq) (TSTFLAG(sq_spec[(sq)],(col)==White?WhPromSq:BlPromSq))
#define ReversePromSq(col,sq) (TSTFLAG(sq_spec[(sq)],(col)==Black?WhPromSq:BlPromSq))

extern boolean promonly[PieceCount];

/* Start a sequence of promotees
 * @param sq_arrival arrival square of the move
 * @param sequence address of structure to represent the sequence
 * @note If sq_arrival is a promotion square of a side
 *          and sq_arrival is still occupied by a pawn of that side
 *       then *state is initialised with a promotion sequence.
 *       Otherwise, state->promotee will be ==Empty.
 */
void pieces_pawns_start_promotee_sequence(square sq_arrival,
                                                pieces_pawns_promotion_sequence_type *sequence);

/* Continue an iteration over a sequence of promotions of a pawn started with an
 * invokation of pieces_pawns_start_promotee_sequence().
 * @param sequence address of structure representing the sequence
 * @note assigns state->promotee the value Empty if iteration has ended
 */
void pieces_pawns_continue_promotee_sequence(pieces_pawns_promotion_sequence_type *sequence);

/* Has a pawn reached a promotion square
 * @param square_reached square reached by the pawn
 * @return side for which the pawn has reached the promotion square
 *         no_side if the pawn hasn't
 */
Side is_square_occupied_by_promotable_pawn(square square_reached);

/* Initialise the set of promotion pieces for the current twin
 */
void pieces_pawns_init_promotees(void);

#endif
