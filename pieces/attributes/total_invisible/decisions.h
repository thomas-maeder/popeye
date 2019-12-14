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

extern decision_level_type curr_decision_level;
extern decision_level_type max_decision_level;

enum
{
  decision_level_dir_capacity = 100
};

extern char decision_level_dir[decision_level_dir_capacity];

extern unsigned long record_decision_counter;
extern unsigned long prev_record_decision_counter;

void record_decision_context_impl(char const *file, unsigned int line, char const *context);
void record_decision_random_move_impl(char const *file, unsigned int line, char direction);
void record_decision_square_impl(char const *file, unsigned int line, char direction, square pos);
void record_decision_side_impl(char const *file, unsigned int line, char direction, Side side);
void record_decision_walk_impl(char const *file, unsigned int line, char direction, piece_walk_type walk);
void record_decision_king_nomination_impl(char const *file, unsigned int line, square pos);
void record_decision_outcome_impl(char const *file, unsigned int line, char const *format, ...);

#define record_decision_context() \
    record_decision_context_impl(__FILE__,__LINE__,__func__)

#define record_decision_random_move(direction) \
    record_decision_random_move_impl(__FILE__,__LINE__,direction)

#define record_decision_side(direction,side) \
    record_decision_side_impl(__FILE__,__LINE__,direction,side)

#define record_decision_walk(direction,walk) \
    record_decision_walk_impl(__FILE__,__LINE__,direction,walk)

#define record_decision_king_nomination(pos) \
    record_decision_king_nomination_impl(__FILE__,__LINE__,pos)

#define record_decision_outcome(format, ...) \
    record_decision_outcome_impl(__FILE__,__LINE__,format,__VA_ARGS__)

#define push_decision_departure(direction,id,sq_departure) \
    record_decision_square_impl(__FILE__,__LINE__,direction,sq_departure); \
    motivation[id].levels.from = curr_decision_level; \
    ++curr_decision_level;

#define push_decision_arrival(direction,id,sq_arrival) \
    record_decision_square_impl(__FILE__,__LINE__,direction,sq_arrival); \
    motivation[id].levels.to = curr_decision_level; \
    ++curr_decision_level;

#endif
