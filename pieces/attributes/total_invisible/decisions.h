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

extern unsigned long report_decision_counter;
extern unsigned long prev_report_decision_counter;

void report_decision_context_impl(char const *file, unsigned int line, char const *context);
void report_decision_move_impl(char const *file, unsigned int line, char direction, char action);
void report_decision_square_impl(char const *file, unsigned int line, char direction, square pos);
void report_decision_colour_impl(char const *file, unsigned int line, char direction, Flags colourspec);
void report_decision_walk_impl(char const *file, unsigned int line, char direction, piece_walk_type walk);
void report_decision_king_nomination_impl(char const *file, unsigned int line, square pos);
void report_decision_outcome_impl(char const *file, unsigned int line, char const *format, ...);

#define report_decision_context(context) \
    report_decision_context_impl(__FILE__,__LINE__,context)

#define report_decision_move(direction,action) \
    report_decision_move_impl(__FILE__,__LINE__,direction,action)

#define report_decision_square(direction,pos) \
    report_decision_square_impl(__FILE__,__LINE__,direction,pos)

#define report_decision_colour(direction,colourspec) \
    report_decision_colour_impl(__FILE__,__LINE__,direction,colourspec)

#define report_decision_walk(direction,walk) \
    report_decision_walk_impl(__FILE__,__LINE__,direction,walk)

#define report_decision_king_nomination(pos) \
    report_decision_king_nomination_impl(__FILE__,__LINE__,pos)

#define report_decision_outcome(format, ...) \
    report_decision_outcome_impl(__FILE__,__LINE__,format,__VA_ARGS__)

#endif
