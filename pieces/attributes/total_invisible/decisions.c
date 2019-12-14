#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "solving/ply.h"
#include "solving/move_effect_journal.h"
#include "debugging/assert.h"

#include <stdio.h>
#include <string.h>

//#define REPORT_DECISIONS

decision_level_type curr_decision_level = 2;
decision_level_type max_decision_level = decision_level_latest;

char decision_level_dir[decision_level_dir_capacity];

unsigned long record_decision_counter;
unsigned long prev_record_decision_counter;

#if defined(REPORT_DECISIONS)

/* the Posix compliant version of this function strangely works with non-const character arrays */
static char const *basename(char const *path)
{
  return strrchr(path, '/') ? strrchr(path, '/') + 1 : path;
}

static void report_endline(char const *file, unsigned int line)
{
  printf(" (K:%u+%u x:%u+%u !:%u+%u ?:%u+%u F:%u+%u)"
         , static_consumption.king[White]
         , static_consumption.king[Black]
         , static_consumption.pawn_victims[White]
         , static_consumption.pawn_victims[Black]
         , current_consumption.claimed[White]
         , current_consumption.claimed[Black]
         , current_consumption.placed[White]
         , current_consumption.placed[Black]
         , current_consumption.fleshed_out[White]
         , current_consumption.fleshed_out[Black]
         );
  printf(" - %s:#%d",basename(file),line);
  printf(" - D:%lu\n",record_decision_counter++);
  fflush(stdout);
}

#endif

void record_decision_context_impl(char const *file, unsigned int line, char const *context)
{
#if defined(REPORT_DECISIONS)
  printf("\n!%s",context);
  write_history_recursive(top_ply_of_regular_play);
  printf(" - %s:#%d",basename(file),line);
  printf(" - D:%lu",record_decision_counter);
  printf(" - %lu",record_decision_counter-prev_record_decision_counter);
  prev_record_decision_counter = record_decision_counter;
  ++record_decision_counter;
  move_numbers_write_history(top_ply_of_regular_play+1);
  fflush(stdout);
#endif
}

void record_decision_random_move_impl(char const *file, unsigned int line, char direction)
{
  decision_level_dir[curr_decision_level] = direction;

#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c%u TI~-~",direction,nbply);
  report_endline(file,line);
#endif
}

void record_decision_square_impl(char const *file, unsigned int line, char direction, square pos)
{
  decision_level_dir[curr_decision_level] = direction;

#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c%u ",direction,nbply);
  WriteSquare(&output_plaintext_engine,
              stdout,
              pos);
  report_endline(file,line);
#endif
}

void record_decision_colour_impl(char const *file, unsigned int line, char direction, Flags colourspec)
{
  decision_level_dir[curr_decision_level] = direction;

#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c%u ",direction,nbply);
  WriteSpec(&output_plaintext_engine,
            stdout,
            colourspec,
            initsquare,
            true);
  report_endline(file,line);
#endif
}

void record_decision_walk_impl(char const *file, unsigned int line, char direction, piece_walk_type walk)
{
  decision_level_dir[curr_decision_level] = direction;

#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c%u ",direction,nbply);
  WriteWalk(&output_plaintext_engine,
            stdout,
            walk);
  report_endline(file,line);
#endif
}

void record_decision_king_nomination_impl(char const *file, unsigned int line, square pos)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  WriteSpec(&output_plaintext_engine,
            stdout,
            being_solved.spec[pos],
            being_solved.board[pos],
            true);
  WriteWalk(&output_plaintext_engine,
            stdout,
            being_solved.board[pos]);
  WriteSquare(&output_plaintext_engine,
              stdout,
              pos);
  report_endline(file,line);
#endif
}

void record_decision_outcome_impl(char const *file, unsigned int line, char const *format, ...)
{
#if defined(REPORT_DECISIONS)
  va_list args;
  va_start(args,format);

  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%u ",nbply);
  vprintf(format,args);
  printf(" - %s:#%d",basename(file),line);
  printf("\n");
  fflush(stdout);

  va_end(args);
#endif
}
