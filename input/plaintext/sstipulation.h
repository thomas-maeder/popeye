#if !defined(INPUT_PLAINTEXT_SSTIPULATION_H)
#define INPUT_PLAINTEXT_SSTIPULATION_H

#include "stipulation/stipulation.h"

char *ParseStructuredStip(char *tok, slice_index start);

/* Remember the original stipulation for restoration after the stipulation has
 * been modified by a twinning
 * @param start input position at start of parsing the stipulation
 * @param start_pos position in input file where the stipulation starts
 * @param stipulation identifies the entry slice into the stipulation
 */
void move_effect_journal_do_insert_sstipulation(slice_index start_index,
                                                slice_index stipulation);

#endif
