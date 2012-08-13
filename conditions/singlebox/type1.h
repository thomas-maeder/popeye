#if !defined(CONDITIONS_SINGLEBOX_TYPE1_H)
#define CONDITIONS_SINGLEBOX_TYPE1_H

/* Implementation of condition Singlebox Type 1
 */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* Determine the next legal single box promotee type
 * @param p type of previous promotee (vide if the first promotee type is to be
 *          found)
 * @param c side of promotee type to be found
 * @return next promotee type; vide if there is none
 */
PieNam next_singlebox_prom(PieNam p, Side c);

/* Determine whether the move just played is legal according to Singlebox Type 1
 * @return true iff the move is legal
 */
boolean singlebox_type1_illegal(void);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type1(slice_index si);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type1_legality_tester_attack(slice_index si,
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
stip_length_type singlebox_type1_legality_tester_defend(slice_index si,
                                                        stip_length_type n);

#endif
