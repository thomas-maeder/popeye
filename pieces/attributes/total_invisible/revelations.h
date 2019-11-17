#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_REVELATIONS_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_REVELATIONS_H

#include "position/position.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible.h"
#include "solving/move_effect_journal.h"

#include <limits.h>

typedef struct
{
    square revealed_on;
    square first_on;
    action_type last;
    piece_walk_type walk;
    Flags spec;
    boolean is_allocated;
} knowledge_type;

// TODO what is a good size for this?
extern knowledge_type knowledge[MaxPieceId];

typedef unsigned int knowledge_index_type;
extern knowledge_index_type size_knowledge;

typedef struct
{
    square first_on;
    piece_walk_type walk;
    Flags spec;
    action_type first;
    action_type last;
} revelation_status_type;

// TODO this doesn't belong here
typedef struct
{
    action_type first;
    action_type last;
    decision_levels_type levels;
} motivation_type;

extern motivation_type motivation[MaxPieceId+1];
extern motivation_type const motivation_null;

extern boolean revelation_status_is_uninitialised;
extern unsigned int nr_potential_revelations;
extern revelation_status_type revelation_status[nr_squares_on_board];
extern decision_level_type curr_decision_level;
extern decision_level_type max_decision_level;
extern move_effect_journal_index_type top_before_revelations[maxply+1];

void initialise_invisible_piece_ids(PieceIdType last_visible_piece_id);

PieceIdType get_top_visible_piece_id(void);
PieceIdType get_top_invisible_piece_id(void);

PieceIdType initialise_motivation(purpose_type purpose, square sq_first,
                                  purpose_type purpose_last, square sq_last);
PieceIdType initialise_motivation_from_revelation(revelation_status_type const *revelation);
void uninitialise_motivation(PieceIdType id_uninitialised);

void reveal_new(move_effect_journal_entry_type const *entry);
void unreveal_new(move_effect_journal_entry_type const *entry);

void reveal_placed(move_effect_journal_entry_type const *entry);
void unreveal_placed(move_effect_journal_entry_type const *entry);

void undo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry);
void redo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry);

void undo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry);
void redo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry);

void adapt_id_of_existing_to_revealed(move_effect_journal_entry_type const *entry);
void unadapt_id_of_existing_to_revealed(move_effect_journal_entry_type const *entry);

void undo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry);
void redo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry);

void replace_moving_piece_ids_in_past_moves(PieceIdType from, PieceIdType to, ply up_to_ply);


void apply_knowledge(knowledge_index_type idx_knowledge,
                     void (*next_step)(void));

void setup_revelations(void);
void initialise_revelations(void);
void update_revelations(void);
void evaluate_revelations(slice_index si, unsigned int nr_potential_revelations);
void make_revelations(void);

void do_revelation_bookkeeping(void);

void undo_revelation_effects(move_effect_journal_index_type curr);

void test_and_execute_revelations(move_effect_journal_index_type curr);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_reveal_after_mating_move(slice_index si);

#endif
