#if !defined(OUTPUT_PLAINTEXT_TREE_TRIVIAL_VARIATION_FILTER_H)
#define OUTPUT_PLAINTEXT_TREE_TRIVIAL_VARIATION_FILTER_H

#include "stipulation/battle_play/attack_play.h"

/* Used to inform STTrivialEndFilter about when to filter out trivial
 * variations (e.g. short mates in self stipulations if there are defenses that
 * don't deliver mate).
 */
extern boolean do_write_trivial_ends[maxply+1];

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
trivial_end_filter_attack(slice_index si, stip_length_type n);

/* Instrument a stipulation with trivial variation filters
 * @param si identifies the entry slice of the stipulation to be instrumented
 */
void stip_insert_trivial_varation_filters(slice_index si);

#endif
