#if !defined(INPUT_PLAINTEXT_STIPULATION_H)
#define INPUT_PLAINTEXT_STIPULATION_H

#include "stipulation/stipulation.h"
#include "solving/move_effect_journal.h"

void select_output_mode(slice_index si, output_mode mode);

char *ParseStip(char *tok, slice_index start);

/* Instrument a slice sub-tree with a stipulatoin
 * @param start where to insert
 * @param stipulation template sub-tree to copy into slice structure at start
 */
void slice_instrument_with_stipulation(slice_index start,
                                       slice_index stipulation);

/* Remember the original stipulation for restoration after the stipulation has
 * been modified by a twinning
 * @param start input position at start of parsing the stipulation
 * @param start_pos position in input file where the stipulation starts
 * @param stipulation identifies the entry slice into the stipulation
 */
void move_effect_journal_do_insert_stipulation(slice_index start,
                                               slice_index stipulation);

/* Reinstall the orginal stipulation while undoing a twnning */
void move_effect_journal_undo_insert_stipulation(move_effect_journal_entry_type const *entry);

/* Remove the current stipulation for restoration after the stipulation has
 * been modified by a twinning
 * @param start input position at start of parsing the stipulation
 */
void move_effect_journal_do_remove_stipulation(slice_index start);

#endif
