#if !defined(PIECES_WALKS_HUNTERS_H)
#define PIECES_WALKS_HUNTERS_H

#include "pieces/pieces.h"
#include "position/board.h"
#include "position/position.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"

typedef struct
{
    piece_walk_type home;
    piece_walk_type away;
} HunterType;

extern HunterType huntertypes[max_nr_hunter_walks];

piece_walk_type hunter_find_type(piece_walk_type away, piece_walk_type home);
piece_walk_type hunter_make_type(piece_walk_type away, piece_walk_type home);

void move_effect_journal_undo_hunter_type_definition(move_effect_journal_entry_type const *entry);

void hunter_generate_moves(void);
boolean hunter_check(validator_id evaluate);

void rook_hunter_generate_moves(void);
boolean rookhunter_check(validator_id evaluate);

void bishop_hunter_generate_moves(void);
boolean bishophunter_check(validator_id evaluate);

/* Make sure that the observer has the expected direction
 * @return true iff the observation is valid
 */
boolean hunter_enforce_observer_direction(slice_index si);

/* Initialise the solving machinery with hunter direction enforcement
 * @param root identifies the root slice of the solving machinery
 */
void solving_initialise_hunters(slice_index root);

#endif
