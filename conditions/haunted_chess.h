#if !defined(CONDITION_HAUNTED_CHESS_H)
#define CONDITION_HAUNTED_CHESS_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* This module implements Haunted Chess.
 */

typedef struct
{
    square ghost_square;
    piece ghost_piece;
    Flags ghost_flags;
    boolean hidden;
} ghost_record_type;

enum
{
  ghost_capacity = 32,
  ghost_not_found = ghost_capacity
};

typedef ghost_record_type ghosts_type[ghost_capacity];

extern ghosts_type ghosts;

typedef unsigned int ghost_index_type;

extern ghost_index_type nr_ghosts;

/* Remember a captured piece as a ghost
 */
void haunted_chess_remember_ghost(void);

/* Forget a ghost (usually while taking a back a capturing move)
 */
void haunted_chess_forget_ghost(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type haunted_chess_ghost_summoner_attack(slice_index si,
                                                     stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type haunted_chess_ghost_summoner_defend(slice_index si,
                                                     stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type haunted_chess_ghost_rememberer_attack(slice_index si,
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
stip_length_type haunted_chess_ghost_rememberer_defend(slice_index si,
                                                       stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_haunted_chess(slice_index si);

#endif
