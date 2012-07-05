#if !defined(CONDITIONS_REPUBLICAN_H)
#define CONDITIONS_REPUBLICAN_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* In Republican Chess Type 1, Republican Chess is suspended once a
 * side has inserted the opposite king.
 */
extern boolean is_republican_suspended;

/* Perform the necessary verification steps for solving a Republican
 * Chess problem
 * @param si identifies root slice of stipulation
 * @return true iff verification passed
 */
boolean republican_verifie_position(slice_index si);

/* Write the Republican Chess diagram caption
 * @param CondLine string containing the condition caption
 * @param lineLength size of array CondLine
 */
void republican_write_diagram_caption(char CondLine[], size_t lineLength);

/* Advance the square where to place the opposite king as part of
 * taking back a move
 */
boolean republican_advance_king_square(void);

/* Save the Republican Chess part of the current move in a play
 * @param ply_id identifies ply of move to be saved
 * @param mov address of structure where to save the move
 */
void republican_current(ply ply_id, coup *move);

/* Compare the Republican Chess parts of two saved moves
 * @param move1 address of 1st saved move
 * @param move2 address of 2nd saved move
 * @return true iff the Republican Chess parts are equal
 */
boolean republican_moves_equal(coup const *move1, coup const *move2);

/* Write how the opposite king is placed as part of a move
 * @param move address of move being written
 */
void write_republican_king_placement(coup const *move);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_republican_king_placers(slice_index si);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type republican_king_placer_attack(slice_index si,
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
stip_length_type republican_king_placer_defend(slice_index si,
                                               stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type republican_king_placer_replay_attack(slice_index si,
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
stip_length_type republican_king_placer_replay_defend(slice_index si,
                                                      stip_length_type n);

#endif
