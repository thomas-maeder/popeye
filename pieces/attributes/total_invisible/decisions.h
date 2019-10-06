#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_DECISIONS_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_DECISIONS_H

#include <limits.h>

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

//#define REPORT_DECISIONS

#if defined(REPORT_DECISIONS)

static unsigned long report_decision_counter;
static unsigned long prev_report_decision_counter;

#define REPORT_DECISION_DECLARE(x) x

#define REPORT_DECISION_CONTEXT(context) \
  printf("\n!%s",context); \
  write_history_recursive(top_ply_of_regular_play); \
  printf(" - L:%d",__LINE__); \
  printf(" - D:%lu",report_decision_counter); \
  printf(" - %lu",report_decision_counter-prev_report_decision_counter); \
  prev_report_decision_counter = report_decision_counter; \
  ++report_decision_counter; \
  move_numbers_write_history(top_ply_of_regular_play+1); \
  fflush(stdout);

#define REPORT_END_LINE \
    printf(" (K:%u+%u x:%u+%u !:%u+%u ?:%u+%u F:%u+%u)" \
           , static_consumption.king[White] \
           , static_consumption.king[Black] \
           , static_consumption.pawn_victims[White] \
           , static_consumption.pawn_victims[Black] \
           , current_consumption.claimed[White] \
           , current_consumption.claimed[Black] \
           , current_consumption.placed[White] \
           , current_consumption.placed[Black] \
           , current_consumption.fleshed_out[White] \
           , current_consumption.fleshed_out[Black] \
           ); \
    printf(" - L:%d",__LINE__); \
    printf(" - D:%lu\n",report_decision_counter++); \
    fflush(stdout);

#define REPORT_DECISION_MOVE(direction,action) \
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level); \
  printf("%c%u ",direction,nbply); \
  WriteWalk(&output_plaintext_engine, \
            stdout, \
            move_effect_journal[move_effect_journal_base[nbply]+move_effect_journal_index_offset_movement].u.piece_movement.moving); \
  WriteSquare(&output_plaintext_engine, \
              stdout, \
              move_effect_journal[move_effect_journal_base[nbply]+move_effect_journal_index_offset_movement].u.piece_movement.from); \
  printf("%c",action); \
  WriteSquare(&output_plaintext_engine, \
              stdout, \
              move_effect_journal[move_effect_journal_base[nbply]+move_effect_journal_index_offset_movement].u.piece_movement.to); \
  REPORT_END_LINE;

#define REPORT_DECISION_SQUARE(direction,pos) \
    printf("!%*s%d ",curr_decision_level,"",curr_decision_level); \
    printf("%c%u ",direction,nbply); \
    WriteSquare(&output_plaintext_engine, \
                stdout, \
                pos); \
    REPORT_END_LINE;

#define REPORT_DECISION_COLOUR(direction,colourspec) \
    printf("!%*s%d ",curr_decision_level,"",curr_decision_level); \
    printf("%c%u ",direction,nbply); \
    WriteSpec(&output_plaintext_engine, \
              stdout, \
              colourspec, \
              initsquare, \
              true); \
    REPORT_END_LINE;

#define REPORT_DECISION_WALK(direction,walk) \
    printf("!%*s%d ",curr_decision_level,"",curr_decision_level); \
    printf("%c%u ",direction,nbply); \
    WriteWalk(&output_plaintext_engine, \
              stdout, \
              walk); \
    REPORT_END_LINE;

#define REPORT_DECISION_KING_NOMINATION(pos) \
    printf("!%*s%d ",curr_decision_level,"",curr_decision_level); \
    WriteSpec(&output_plaintext_engine, \
              stdout, \
              being_solved.spec[pos], \
              being_solved.board[pos], \
              true); \
    WriteWalk(&output_plaintext_engine, \
              stdout, \
              being_solved.board[pos]); \
    WriteSquare(&output_plaintext_engine, \
                stdout, \
                pos); \
    REPORT_END_LINE;

#define REPORT_DECISION_OUTCOME(format, ...) \
    printf("!%*s%d ",curr_decision_level,"",curr_decision_level); \
    printf("%u ",nbply); \
    printf(format,__VA_ARGS__); \
    printf(" - L:%d",__LINE__); \
    printf("\n"); \
    fflush(stdout);

#else

#define REPORT_DECISION_DECLARE(x)
#define REPORT_DECISION_CONTEXT(context)
#define REPORT_DECISION_MOVE(direction,action)
#define REPORT_DECISION_SQUARE(direction,pos)
#define REPORT_DECISION_COLOUR(direction,colourspec)
#define REPORT_DECISION_WALK(direction,walk)
#define REPORT_DECISION_KING_NOMINATION(pos)
#define REPORT_DECISION_OUTCOME(format, ...)

#endif

#endif
