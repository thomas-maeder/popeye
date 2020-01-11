#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_DECISIONS_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_DECISIONS_H

#include "position/board.h"
#include "pieces/attributes/total_invisible.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/plaintext.h"
#include "options/movenumbers.h"

#include <string.h>
#include <limits.h>
#include <stdarg.h>

typedef unsigned int decision_level_type;

enum
{
  decision_level_uninitialised = 0,
  decision_level_forever = 1,
  decision_level_initial,
  decision_level_latest = UINT_MAX
};

typedef struct
{
    decision_level_type side;
    decision_level_type walk;
    decision_level_type from;
    decision_level_type to;
} decision_levels_type;

typedef enum
{
  decision_purpose_invisible_capturer_existing,
  decision_purpose_invisible_capturer_inserted,
  decision_purpose_mating_piece_attacker,
  decision_purpose_illegal_check_interceptor,
  decision_purpose_random_mover_forward,
  decision_purpose_random_mover_backward,
  decision_purpose_king_nomination
} decision_purpose_type;

extern decision_levels_type decision_levels[MaxPieceId+1];

extern decision_level_type curr_decision_level;

enum
{
  decision_level_dir_capacity = 100
};

extern unsigned long record_decision_counter;

void record_decision_for_inserted_invisible(PieceIdType id);

void initialise_decision_context_impl(char const *file, unsigned int line, char const *context);

void record_decision_outcome_impl(char const *file, unsigned int line, char const *format, ...);

void push_decision_random_move_impl(char const *file, unsigned int line, decision_purpose_type purpose);
decision_level_type push_decision_side_impl(char const *file, unsigned int line, PieceIdType id, Side side, decision_purpose_type purpose);
decision_level_type push_decision_insertion_impl(char const *file, unsigned int line, PieceIdType id, Side side, decision_purpose_type purpose);
decision_level_type push_decision_departure_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose);
decision_level_type push_decision_move_vector_impl(char const *file, unsigned int line, PieceIdType id, int dir, decision_purpose_type purpose);
decision_level_type push_decision_arrival_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose);
decision_level_type push_decision_placement_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose);
decision_level_type push_decision_walk_impl(char const *file, unsigned int line, PieceIdType id, piece_walk_type walk, decision_purpose_type purpose, Side side);
void push_decision_king_nomination_impl(char const *file, unsigned int line, square pos);

#define initialise_decision_context() \
    initialise_decision_context_impl(__FILE__,__LINE__,__func__)

#define record_decision_outcome(format, ...) \
    record_decision_outcome_impl(__FILE__,__LINE__,format,__VA_ARGS__)

#define push_decision_random_move(purpose) \
    push_decision_random_move_impl(__FILE__,__LINE__,purpose);

#define push_decision_side(id,side,purpose) \
    push_decision_side_impl(__FILE__,__LINE__,id,side,purpose);

#define push_decision_insertion(id,side,purpose) \
    push_decision_insertion_impl(__FILE__,__LINE__,id,side,purpose);

#define push_decision_departure(id,sq_departure,purpose) \
    push_decision_departure_impl(__FILE__,__LINE__,id,sq_departure,purpose);

#define push_decision_move_vector(id,dir,purpose) \
    push_decision_move_vector_impl(__FILE__,__LINE__,id,dir,purpose);

#define push_decision_arrival(id,sq_arrival,purpose) \
    push_decision_arrival_impl(__FILE__,__LINE__,id,sq_arrival,purpose);

#define push_decision_placement(id,sq_placement,purpose) \
    push_decision_placement_impl(__FILE__,__LINE__,id,sq_placement,purpose);

#define push_decision_walk(id,walk,purpose,side) \
    push_decision_walk_impl(__FILE__,__LINE__,id,walk,purpose,side);

#define push_decision_king_nomination(pos) \
    push_decision_king_nomination_impl(__FILE__,__LINE__,pos);

void pop_decision(void);

/* Reduce max_decision_level to a value as low as possible considering that we have
 * reached a position where we aren't able to intercept all illegal checks by inserting
 * invisibles.
 * @param side_in_check the side that is in too many illegal checks
 */
void backtrack_from_failure_to_intercept_illegal_checks(Side side_in_check);

/* Reduce max_decision_level to a value as low as possible considering that we have
 * reached a position where we won't able to execute the planned capture by an invisible
 * in the subsequent move because
 * - no existing invisible of the relevant side can reach the capture square
 * - no invisible of the relevant side can be inserted
 * @param side_capturing the side that is supposed to capture
 */
void backtrack_from_failed_capture_by_invisible(Side side_capturing);

/* Reduce max_decision_level to a value as low as possible considering that we have
 * reached a position where we won't able to execute the planned capture of an invisible
 * by a pawn in the subsequent move because
 * - no existing invisible of the relevant side can sacrifice itself on the capture square
 * - no invisible of the relevant side can be inserted
 * @param side_capturing the side that is supposed to capture
 */
void backtrack_from_failed_capture_of_invisible_by_pawn(Side side_capturing);

/* Reduce max_decision_level to a value as low as possible considering that we have
 * determined that the we are done testing the current move sequence
 */
void backtrack_definitively(void);

/* To be invoked after backtrack_definitively(), possibly multiple times, to make that "definititively"
 * a bit more relative.
 * @param level level to which to backtrack at most
 */
void backtrack_no_further_than(decision_level_type level);

boolean can_decision_level_be_continued(void);

boolean has_decision_failed_capture(void);

#endif
