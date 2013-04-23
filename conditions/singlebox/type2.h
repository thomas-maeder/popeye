#if !defined(CONDITIONS_SINGLEBOX_TYPE2_H)
#define CONDITIONS_SINGLEBOX_TYPE2_H

/* Implementation of condition Singlebox Type 2
 */

#include "solving/solve.h"
#include "pieces/pawns/promotion.h"

/* TODO this assumes that <=1 latent pawn promotion is possible per ply */
typedef struct
{
    square where;
    Side side;
    pieces_pawns_promotion_sequence_type promotion;
} singlebox_type2_latent_pawn_promotion_type;

extern singlebox_type2_latent_pawn_promotion_type singlebox_type2_latent_pawn_promotions[maxply+1];

/* Initialise the sequence of promotions of a latent pawn
 * @param sq_prom potential promotion square
 * @param side address of side; *side will be assigned the side of which sq_prom
 *             is a promotion square
 * @param sequence address of structure holding the promotion sequence
 * @note the sequence only contains the promotees legal according to type 2
 */
void singlebox_type2_initialise_singlebox_promotion_sequence(square sq_prom,
                                                             Side *side,
                                                             pieces_pawns_promotion_sequence_type *sequence);

/* Continue a promotion sequence intialised by
 * singlebox_type2_initialise_singlebox_promotion_sequence()
 * @param side side of the promotion square assigned during the initialisation
 * @param sequence address of structure holding the promotion sequence
 */
void singlebox_type2_continue_singlebox_promotion_sequence(Side side,
                                                           pieces_pawns_promotion_sequence_type *sequence);

/* Find the next latent pawn
 * @param s position of previous latent pawn (initsquare if we search the first
 *          latent pawn)
 * @param c side of latent pawn to be found
 * @return position of next latent pawn; initsquare if there is none
 */
square next_latent_pawn(square s, Side c);

/* Does the current position contain an illegal latent white pawn?
 * @return true iff it does
 */
boolean singlebox_illegal_latent_white_pawn(void);

/* Does the current position contain an illegal latent black pawn?
 * @return true iff it does
 */
boolean singlebox_illegal_latent_black_pawn(void);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type2(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type singlebox_type2_latent_pawn_selector_solve(slice_index si,
                                                             stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type singlebox_type2_latent_pawn_promoter_solve(slice_index si,
                                                             stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type singlebox_type2_legality_tester_solve(slice_index si,
                                                        stip_length_type n);

#endif
