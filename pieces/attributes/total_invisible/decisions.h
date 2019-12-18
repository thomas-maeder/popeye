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
  decision_purpose_invisible_capturer,
  decision_purpose_mating_piece_attacker,
  decision_purpose_illegal_check_interceptor,
  decision_purpose_random_mover,
  nr_decision_purposes
} decision_purpose_type;

extern decision_levels_type decision_levels[MaxPieceId+1];

extern decision_level_type curr_decision_level;
extern decision_level_type max_decision_level;

enum
{
  decision_level_dir_capacity = 100
};

extern unsigned long record_decision_counter;
extern unsigned long prev_record_decision_counter;

void record_decision_for_inserted_capturer(PieceIdType id);

void record_decision_context_impl(char const *file, unsigned int line, char const *context);
void record_decision_outcome_impl(char const *file, unsigned int line, char const *format, ...);

void push_decision_random_move_impl(char const *file, unsigned int line, char direction);
void push_decision_departure_impl(char const *file, unsigned int line, char direction, PieceIdType id, square pos, decision_purpose_type purpose);
void push_decision_move_vector_impl(char const *file, unsigned int line, PieceIdType id, int dir, decision_purpose_type purpose);
void push_decision_arrival_impl(char const *file, unsigned int line, char direction, PieceIdType id, square pos, decision_purpose_type purpose);
void push_decision_side_impl(char const *file, unsigned int line, char direction, PieceIdType id, Side side, decision_purpose_type purpose);
void push_decision_walk_impl(char const *file, unsigned int line, char direction, PieceIdType id, piece_walk_type walk, decision_purpose_type purpose);
void push_decision_king_nomination_impl(char const *file, unsigned int line, square pos);

#define record_decision_context() \
    record_decision_context_impl(__FILE__,__LINE__,__func__)

#define record_decision_outcome(format, ...) \
    record_decision_outcome_impl(__FILE__,__LINE__,format,__VA_ARGS__)

#define push_decision_random_move(direction) \
    push_decision_random_move_impl(__FILE__,__LINE__,direction);

#define push_decision_departure(direction,id,sq_departure,purpose) \
    push_decision_departure_impl(__FILE__,__LINE__,direction,id,sq_departure,purpose);

#define push_decision_move_vector(id,dir,purpose) \
    push_decision_move_vector_impl(__FILE__,__LINE__,id,dir,purpose);

#define push_decision_arrival(direction,id,sq_arrival,purpose) \
    push_decision_arrival_impl(__FILE__,__LINE__,direction,id,sq_arrival,purpose);

#define push_decision_side(direction,id,side,purpose) \
    push_decision_side_impl(__FILE__,__LINE__,direction,id,side,purpose);

#define push_decision_walk(direction,id,walk,purpose) \
    push_decision_walk_impl(__FILE__,__LINE__,direction,id,walk,purpose);

#define push_decision_king_nomination(pos) \
    push_decision_king_nomination_impl(__FILE__,__LINE__,pos);

void pop_decision(void);

void backtrack_from_failure_to_intercept_illegal_checks(void);
void backtrack_from_failed_capture_by_invisible(void);
void backtrack_from_failed_capture_of_invisible_by_pawn(void);

boolean can_decision_level_be_continued(void);

#endif
