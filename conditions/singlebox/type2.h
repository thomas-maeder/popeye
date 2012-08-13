#if !defined(CONDITIONS_SINGLEBOX_TYPE2_H)
#define CONDITIONS_SINGLEBOX_TYPE2_H

/* Implementation of condition Singlebox Type 2
 */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* TODO this assumes that <=1 latent pawn promotion is possible per ply */
typedef struct
{
    square where;
    PieNam what;
} singlebox_type2_latent_pawn_promotion_type;

extern singlebox_type2_latent_pawn_promotion_type singlebox_type2_latent_pawn_promotions[maxply+1];

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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type2_latent_pawn_selector_attack(slice_index si,
                                                             stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type singlebox_type2_latent_pawn_selector_defend(slice_index si,
                                                             stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type2_latent_pawn_promoter_attack(slice_index si,
                                                             stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type singlebox_type2_latent_pawn_promoter_defend(slice_index si,
                                                             stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type2_legality_tester_attack(slice_index si,
                                                        stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type singlebox_type2_legality_tester_defend(slice_index si,
                                                        stip_length_type n);

#endif
