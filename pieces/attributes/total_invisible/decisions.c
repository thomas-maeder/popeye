#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "solving/ply.h"
#include "solving/has_solution_type.h"
#include "solving/move_effect_journal.h"
#include "debugging/assert.h"

#include <stdio.h>
#include <string.h>

//#define REPORT_DECISIONS

static decision_level_type next_decision_level = decision_level_initial;

decision_levels_type decision_levels[MaxPieceId+1];

typedef enum
{
  decision_object_side,
  decision_object_insertion,
  decision_object_random_move,
  decision_object_walk,
  decision_object_move_vector,
  decision_object_departure,
  decision_object_arrival,
  decision_object_placement,
  decision_object_king_nomination
} decision_object_type;

typedef enum
{
  relevance_unknown,
  relevance_irrelevant,
  relevance_relevant
} relevance_type;

typedef struct
{
    ply ply;
    decision_object_type object;
    decision_purpose_type purpose;
    PieceIdType id;
    Side side;
    relevance_type relevance;
} decision_level_property_type;

static decision_level_property_type decision_level_properties[decision_level_dir_capacity];

typedef enum
{
  backtrack_none,
  backtrack_until_level,
  backtrack_revelation,
  backtrack_failure_to_intercept_illegal_checks,
  backtrack_failure_to_capture_uninterceptable_checker,
  backtrack_failture_to_capture_by_invisible,
  backtrack_failture_to_capture_invisible_by_pawn
} backtrack_type;

static struct
{
    backtrack_type type;
    decision_level_type max_level;
    has_solution_type result;
    unsigned int nr_check_vectors;
    ply ply_failure;
    Side side_failure;
    PieceIdType id_failure;
} backtracking[decision_level_dir_capacity];

unsigned long record_decision_counter;

static boolean try_to_avoid_insertion[nr_sides] = { false, false };

#if defined(REPORT_DECISIONS)

static unsigned long prev_record_decision_counter;

static char const purpose_symbol[] = {
    'x' /* decision_purpose_invisible_capturer_existing */
    , 'X' /* decision_purpose_invisible_capturer_inserted */
    , '#' /* decision_purpose_mating_piece_attacker */
    , '+' /* decision_purpose_illegal_check_interceptor */
    , '>' /* decision_purpose_random_mover_forward */
    , '<' /* decision_purpose_random_mover_backward */
};

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
  printf(" - r:%u t:%u m:%d n:%u i:%u",
         backtracking[next_decision_level].result,
         backtracking[next_decision_level].type,
         (int)backtracking[next_decision_level].max_level,
         backtracking[next_decision_level].nr_check_vectors,
         decision_level_properties[next_decision_level].id);
  printf(" - %s:#%d",basename(file),line);
  printf(" - D:%lu\n",record_decision_counter);
  fflush(stdout);
}

#endif

void decision_levels_init(PieceIdType id)
{
  decision_levels[id].side = decision_level_forever;
  decision_levels[id].walk = decision_level_forever;
  decision_levels[id].from = decision_level_forever;
  decision_levels[id].to = decision_level_forever;
}

void initialise_decision_context_impl(char const *file, unsigned int line, char const *context)
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

  prev_record_decision_counter = record_decision_counter;
#endif

  backtracking[next_decision_level-1].max_level = decision_level_latest;
  backtracking[next_decision_level-1].type = backtrack_none;
  backtracking[next_decision_level-1].result = previous_move_is_illegal;
}

void record_decision_for_inserted_invisible(PieceIdType id)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",id);
  TraceFunctionParamListEnd();

  decision_levels[id].side = next_decision_level-1;
  decision_levels[id].walk = next_decision_level-1;
  decision_levels[id].to = next_decision_level-1;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static decision_level_type push_decision_common(char const *file, unsigned int line)
{
  decision_level_properties[next_decision_level].ply = nbply;
  decision_level_properties[next_decision_level].relevance = relevance_unknown;

  backtracking[next_decision_level].max_level = decision_level_latest;
  backtracking[next_decision_level].type = backtrack_none;
  backtracking[next_decision_level].result = previous_move_is_illegal;
  backtracking[next_decision_level].nr_check_vectors = UINT_MAX;
  backtracking[next_decision_level+1].ply_failure = ply_nil;
  backtracking[next_decision_level+1].side_failure = no_side;
  backtracking[next_decision_level+1].id_failure = NullPieceId;

  ++record_decision_counter;

  assert(next_decision_level<decision_level_dir_capacity);
#if defined(REPORT_DECISIONS)
  report_endline(file,line);
#endif

  return next_decision_level++;
}

void push_decision_random_move_impl(char const *file, unsigned int line, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u TI~-~",purpose_symbol[purpose],nbply);
#endif

  decision_level_properties[next_decision_level].object = decision_object_random_move;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].side = trait[nbply];

  push_decision_common(file,line);
}

void push_decision_departure_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteSquare(&output_plaintext_engine,
              stdout,
              pos);
#endif

  decision_level_properties[next_decision_level].object = decision_object_departure;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].id = id;

  if (purpose==decision_purpose_illegal_check_interceptor)
  {
    assert(next_decision_level>0);
    assert(decision_level_properties[next_decision_level-1].object==decision_object_side);
    assert(decision_level_properties[next_decision_level-1].purpose==decision_purpose_illegal_check_interceptor);
    assert(decision_level_properties[next_decision_level-1].id==id);
    decision_level_properties[next_decision_level].side = decision_level_properties[next_decision_level-1].side;
  }
  else
    decision_level_properties[next_decision_level].side = trait[nbply];

  decision_levels[id].from = push_decision_common(file,line);
}

// TODO  do we still need to do record decisions regarding move vectors?
void push_decision_move_vector_impl(char const *file, unsigned int line, PieceIdType id, int direction, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  printf("direction:%d",direction);
#endif

  decision_level_properties[next_decision_level].object = decision_object_move_vector;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].id = id;

  if (purpose==decision_purpose_illegal_check_interceptor)
  {
    assert(next_decision_level>0);
    assert(decision_level_properties[next_decision_level-1].object==decision_object_side);
    assert(decision_level_properties[next_decision_level-1].purpose==decision_purpose_illegal_check_interceptor);
    assert(decision_level_properties[next_decision_level-1].id==id);
    decision_level_properties[next_decision_level].side = decision_level_properties[next_decision_level-1].side;
  }
  else
    decision_level_properties[next_decision_level].side = trait[nbply];

  push_decision_common(file,line);
}

void push_decision_arrival_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteSquare(&output_plaintext_engine,
              stdout,
              pos);
#endif

  assert(purpose==decision_purpose_random_mover_forward
         || purpose==decision_purpose_random_mover_backward);

  decision_level_properties[next_decision_level].object = decision_object_arrival;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].id = id;
  decision_level_properties[next_decision_level].side = trait[nbply];

  decision_levels[id].to = push_decision_common(file,line);
}

void push_decision_placement_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteSquare(&output_plaintext_engine,
              stdout,
              pos);
#endif

  assert(purpose==decision_purpose_illegal_check_interceptor);

  decision_level_properties[next_decision_level].object = decision_object_placement;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].id = id;
  decision_level_properties[next_decision_level].side = no_side;

  decision_levels[id].to = push_decision_common(file,line);
}

void push_decision_side_impl(char const *file, unsigned int line, PieceIdType id, Side side, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteSpec(&output_plaintext_engine,
            stdout,
            BIT(side),
            initsquare,
            true);
#endif

  decision_level_properties[next_decision_level].object = decision_object_side;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].id = id;
  decision_level_properties[next_decision_level].side = side;

  decision_levels[id].side = push_decision_common(file,line);
}

void push_decision_insertion_impl(char const *file, unsigned int line, PieceIdType id, Side side, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  printf("I");
#endif

  decision_level_properties[next_decision_level].object = decision_object_insertion;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].id = id;
  decision_level_properties[next_decision_level].side = side;

  decision_levels[id].side = push_decision_common(file,line);
}

void push_decision_walk_impl(char const *file, unsigned int line,
                             PieceIdType id,
                             piece_walk_type walk,
                             decision_purpose_type purpose,
                             Side side)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteWalk(&output_plaintext_engine,
            stdout,
            walk);
#endif

  decision_level_properties[next_decision_level].object = decision_object_walk;
  decision_level_properties[next_decision_level].purpose = purpose;
  decision_level_properties[next_decision_level].id = id;
  decision_level_properties[next_decision_level].side = side;

  decision_levels[id].walk = push_decision_common(file,line);
}

void push_decision_king_nomination_impl(char const *file, unsigned int line, PieceIdType id, square pos)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",next_decision_level,">",next_decision_level);
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
#endif

  decision_level_properties[next_decision_level].object = decision_object_king_nomination;
  decision_level_properties[next_decision_level].purpose = decision_purpose_king_nomination;
  decision_level_properties[next_decision_level].side = TSTFLAG(being_solved.spec[pos],White) ? White : Black;

  decision_levels[id].walk = push_decision_common(file,line);
}

void record_decision_outcome_impl(char const *file, unsigned int line, char const *format, ...)
{
#if defined(REPORT_DECISIONS)
  va_list args;
  va_start(args,format);

  printf("!%*s%d ",next_decision_level,"",next_decision_level);
  printf("%u ",nbply);
  vprintf(format,args);
  printf(" - %s:#%d",basename(file),line);
  printf("\n");
  fflush(stdout);

  va_end(args);
#endif
}

void record_decision_result(has_solution_type recorded_result)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",recorded_result);
  TraceFunctionParamListEnd();

  TraceValue("%u",next_decision_level);
  TraceValue("%u",recorded_result);
  TraceEOL();

  assert(recorded_result<=previous_move_has_not_solved);

  if (recorded_result>backtracking[next_decision_level-1].result)
  {
    backtracking[next_decision_level-1].result = recorded_result;

#if defined(REPORT_DECISIONS)
    printf("!%*s%d",next_decision_level," ",next_decision_level);
    printf(" - combined result:%u\n",
           backtracking[next_decision_level-1].result);
    fflush(stdout);
#endif
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

has_solution_type get_decision_result(void)
{
  has_solution_type const result = backtracking[next_decision_level-1].result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void pop_decision(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  decision_level_properties[next_decision_level].ply = ply_nil;
  decision_level_properties[next_decision_level].side = no_side;
  decision_level_properties[next_decision_level].relevance = relevance_unknown;

  assert(next_decision_level>0);
  --next_decision_level;

  TraceValue("%u",next_decision_level);
  TraceValue("%u",backtracking[next_decision_level].result);
  TraceValue("%u",backtracking[next_decision_level].type);
  TraceValue("%u",backtracking[next_decision_level].nr_check_vectors);
  TraceValue("%u",backtracking[next_decision_level].ply_failure);
  TraceValue("%u",backtracking[next_decision_level-1].result);
  TraceValue("%u",backtracking[next_decision_level-1].type);
  TraceValue("%u",backtracking[next_decision_level-1].nr_check_vectors);
  TraceValue("%u",backtracking[next_decision_level-1].ply_failure);
  TraceEOL();

  if ((backtracking[next_decision_level].result
       >backtracking[next_decision_level-1].result)
      || ((backtracking[next_decision_level].result
           ==backtracking[next_decision_level-1].result)
          && (backtracking[next_decision_level].ply_failure
              >=backtracking[next_decision_level-1].ply_failure)))
    backtracking[next_decision_level-1] = backtracking[next_decision_level];

  TraceValue("%u",next_decision_level);
  TraceValue("%u",decision_level_properties[next_decision_level].ply);
  TraceValue("%u",decision_level_properties[next_decision_level].purpose);
  TraceValue("%u",decision_level_properties[next_decision_level].object);
  TraceEnumerator(Side,decision_level_properties[next_decision_level].side);
  TraceValue("%u",backtracking[next_decision_level].max_level);
  TraceEOL();

  switch (backtracking[next_decision_level].type)
  {
    case backtrack_failure_to_intercept_illegal_checks:
      assert(backtracking[next_decision_level].nr_check_vectors!=UINT_MAX);
      switch (decision_level_properties[next_decision_level].object)
      {
        case decision_object_insertion:
          assert(decision_level_properties[next_decision_level].side!=no_side);

          /* remember which side may save an insertion */
          try_to_avoid_insertion[decision_level_properties[next_decision_level].side] = true;

          if (decision_level_properties[next_decision_level].purpose==decision_purpose_invisible_capturer_inserted)
            try_to_avoid_insertion[advers(decision_level_properties[next_decision_level].side)] = true;
          break;

        case decision_object_arrival:
          if ((backtracking[next_decision_level-2].type
               ==backtrack_failure_to_intercept_illegal_checks)
              && (decision_level_properties[next_decision_level].purpose
                  ==decision_purpose_random_mover_forward)
              && (decision_level_properties[next_decision_level].side
                  ==backtracking[next_decision_level].side_failure))
          {
            if (backtracking[next_decision_level].nr_check_vectors
                <=backtracking[next_decision_level-2].nr_check_vectors)
            {
              PieceIdType const id_moving = decision_level_properties[next_decision_level].id;
              decision_level_properties[decision_levels[id_moving].walk].relevance = relevance_relevant;
                /* e.g.
    begin
    author Michel Caillaud
    origin Sake tourney 2018, 1st HM, cooked (author's solution relies on retro and is not shown)
    pieces TotalInvisible 3 white kg4 rh6 ba2 pd4d6 black ke6 qe2 re8g8 bd7h8 pd2
    stipulation h#2
    option movenum start 51:9:31:0
    end

               Michel Caillaud
    Sake tourney 2018, 1st HM, cooked (author's solution relies on retro and is not shown)

    +---a---b---c---d---e---f---g---h---+
    |                                   |
    8   .   .   .   .  -R   .  -R  -B   8
    |                                   |
    7   .   .   .  -B   .   .   .   .   7
    |                                   |
    6   .   .   .   P  -K   .   .   R   6
    |                                   |
    5   .   .   .   .   .   .   .   .   5
    |                                   |
    4   .   .   .   P   .   .   K   .   4
    |                                   |
    3   .   .   .   .   .   .   .   .   3
    |                                   |
    2   B   .   .  -P  -Q   .   .   .   2
    |                                   |
    1   .   .   .   .   .   .   .   .   1
    |                                   |
    +---a---b---c---d---e---f---g---h---+
      h#2                  5 + 7 + 3 TI

    399,404c399,400
    <   1.Rg8-g7 Ba2-c4   2.Ke6-f7 TI~-~[f5=wR] #

    !test_mate 6:Rg8-g7 7:Ba2-c4 8:Ke6-f7 9:TI~-~ - total_invisible.c:#540 - D:417 - 250
    use option start 51:9:31:0 to replay
    ! >2 + 6 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 i:13 - intercept_illegal_checks.c:#487 - D:419
    !  >3 + 6 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 i:13 - intercept_illegal_checks.c:#450 - D:421
    !   >4 + 6 Q (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 i:13 - intercept_illegal_checks.c:#253 - D:423
    ...
    !   <4 - r:1 t:3 m:7
    !   >4 + 6 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 i:13 - intercept_illegal_checks.c:#253 - D:555
    !    >5 + 6 g5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 i:14 - intercept_illegal_checks.c:#487 - D:557
    ...
    !    <5 - r:1 t:3 m:7
    !    >5 + 6 g6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 i:14 - intercept_illegal_checks.c:#487 - D:575
    !     >6 + 6 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 i:14 - intercept_illegal_checks.c:#450 - D:577
    !      >7 + 6 Q (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:4294967295 i:14 - intercept_illegal_checks.c:#253 - D:579
    ...
    !      >7 + 6 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:4294967295 i:14 - intercept_illegal_checks.c:#253 - D:589
    !       >8 + 7 d5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:4294967295 i:15 - intercept_illegal_checks.c:#487 - D:591
    !        >9 + 7 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:4294967295 i:15 - intercept_illegal_checks.c:#450 - D:593
    ...
    !        <9 - r:1 t:3 m:12
    !        >9 + 7 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:4294967295 i:15 - intercept_illegal_checks.c:#450 - D:685
    !         >10 + 7 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:4294967295 i:15 - intercept_illegal_checks.c:#253 - D:687
    ...
    !         <10 - r:1 t:1 m:10
    !         >10 + 7 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:4294967295 i:15 - intercept_illegal_checks.c:#253 - D:689
    !          >11 > 9 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:4294967295 i:13 - random_move_by_invisible.c:#544 - D:691
    ...
    !          <11 - r:1 t:3 m:12
    !          >11 > 9 d5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:4294967295 i:15 - random_move_by_invisible.c:#544 - D:695
    !           >12 > 9 b6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:4294967295 i:15 - random_move_by_invisible.c:#25 - D:697
    !             13 10 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#619
    !           <12 - r:1 t:3 m:12
    ...

    MOVING THIS PIECE MAKES ITS WALK RELEVANT...

    !          <11 - r:1 t:3 m:12
    !          >11 > 9 g6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:4294967295 i:14 - random_move_by_invisible.c:#544 - D:713
    ...
    !          <11 - r:1 t:3 m:12
    !         <10 - r:1 t:3 m:12

    ... SO LET'S TRY OTHER WALKS HERE - ROOK WILL WORK
                 */
            }
          }
          if (backtracking[next_decision_level-1].type==backtrack_failure_to_intercept_illegal_checks
              && (backtracking[next_decision_level].nr_check_vectors
                  >backtracking[next_decision_level-1].nr_check_vectors))
          {
            decision_level_properties[next_decision_level].relevance = relevance_irrelevant;
            /* moves by this piece won't lead to a position where we can intercept all illegal checks */
            /* e.g.
begin
author Ofer Comay
origin Sake tourney 2018, 3rd HM, cooked (and 1 author's solution doesn't deliver mate)
pieces TotalInvisible 3 white ke5 qh8 bc1 pb7c2h4 black rb4e1 ba1f1 sf2
stipulation h#2
option movenum start 1:1:5:1
end

           Ofer Comay
Sake tourney 2018, 3rd HM, cooked (and 1 authors solution doesnt deliver mate)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   Q   8
|                                   |
7   .   P   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   K   .   .   .   5
|                                   |
4   .  -R   .   .   .   .   .   P   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   P   .   .  -S   .   .   2
|                                   |
1  -B   .   B   .  -R  -B   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
h#2                  6 + 5 + 3 TI

!validate_mate 6:TI~-~ 7:TI~-~ 8:TI~-c2 9:TI~-b4 - total_invisible.c:#514 - D:4080 - 3440
use option start 1:1:5:1 to replay
! >2 + 6 d4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#215 - D:4082

! >2 + 6 c3 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#215 - D:19927
!  >3 + 6 w (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#111 - D:19928
!   >4 + 6 e4 (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#215 - D:19929

!   >4 + 6 e3 (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#215 - D:22675
!    >5 + 6 w (K:0+1 x:0+0 !:0+0 ?:2+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#111 - D:22676

!    >5 + 6 b (K:0+1 x:0+0 !:0+0 ?:1+1 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#111 - D:23366
!     >6 > 6 e3 (K:0+1 x:0+0 !:0+0 ?:1+1 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#544 - D:23367
!      >7 > 6 K (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#469 - D:23368

!      >7 > 6 B (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#375 - D:24793
!       >8 > 6 d4 (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:24794

!       >8 > 6 f4 (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:25123
!        >9 > 7 c3 (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#544 - D:25124
!         >10 > 7 P (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#348 - D:25125
!          >11 > 7 c4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#25 - D:25126
!            12 8 only 1 available invisibles for intercepting 3 illegal checks - intercept_illegal_checks.c:#770
!          <11 - r:1 t:3 m:11 n:3 i:14
!         <10 - r:1 t:3 m:11 n:3 i:14
!         >10 > 7 S (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#362 - D:25127
!          >11 > 7 a4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#25 - D:25128
!            12 8 only 1 available invisibles for intercepting 3 illegal checks - intercept_illegal_checks.c:#770
!          <11 - r:1 t:3 m:11 n:3 i:14
!          >11 > 7 b5 (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#25 - D:25129
!            12 8 only 1 available invisibles for intercepting 3 illegal checks - intercept_illegal_checks.c:#770
!          <11 - r:1 t:3 m:11 n:3 i:14
!          >11 > 7 d5 (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#25 - D:25130
!            12 8 only 1 available invisibles for intercepting 3 illegal checks - intercept_illegal_checks.c:#770
!          <11 - r:1 t:3 m:11 n:3 i:14
!          >11 > 7 e4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#25 - D:25131
!            12 8 only 1 available invisibles for intercepting 2 illegal checks - intercept_illegal_checks.c:#770

HERE - NO NEED TO TRY OTHER MOVES BY THIS KNIGHT

!          <11 - r:1 t:3 m:11 n:2 i:14
             */
          }
          break;

        default:
          break;
      }
      break;

    case backtrack_failure_to_capture_uninterceptable_checker:
      switch (decision_level_properties[next_decision_level].object)
      {
        case decision_object_insertion:
          assert(decision_level_properties[next_decision_level].side!=no_side);

          /* remember which side may save an insertion */
          try_to_avoid_insertion[decision_level_properties[next_decision_level].side] = true;

          if (decision_level_properties[next_decision_level].purpose==decision_purpose_invisible_capturer_inserted)
            try_to_avoid_insertion[advers(decision_level_properties[next_decision_level].side)] = true;
          break;

        case decision_object_arrival:
          if ((backtracking[next_decision_level-2].type
               ==backtrack_failure_to_capture_uninterceptable_checker)
              && (decision_level_properties[next_decision_level].purpose
                  ==decision_purpose_random_mover_forward)
              && (decision_level_properties[next_decision_level].side
                  ==backtracking[next_decision_level].side_failure))
          {
            PieceIdType const id_moving = decision_level_properties[next_decision_level].id;
            decision_level_properties[decision_levels[id_moving].walk].relevance = relevance_relevant;
          }
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

#if defined(REPORT_DECISIONS)
  printf("!%*s%d",next_decision_level,"<",next_decision_level);
  printf(" - r:%u t:%u m:%u n:%u i:%u\n",
         backtracking[next_decision_level].result,
         backtracking[next_decision_level].type,
         backtracking[next_decision_level].max_level,
         backtracking[next_decision_level].nr_check_vectors,
         decision_level_properties[next_decision_level].id);
  fflush(stdout);
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void decision_make_relevant(decision_level_type level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  decision_level_properties[level].relevance = relevance_relevant;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean has_decision_failed_capture(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(decision_level_properties[next_decision_level-1].object==decision_object_side);

  switch (backtracking[next_decision_level-1].type)
  {
    case backtrack_failture_to_capture_by_invisible:
      if (decision_level_properties[next_decision_level-1].id
          ==backtracking[next_decision_level-1].id_failure)
        result = true;
      break;

    case backtrack_failture_to_capture_invisible_by_pawn:
      if (decision_level_properties[next_decision_level-1].id
          ==backtracking[next_decision_level-1].id_failure)
        result = true;
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_random_move_of_side_between_plies(ply ply_start,
                                                      ply ply_failure,
                                                      Side side_in_check)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_start);
  TraceFunctionParam("%u",ply_failure);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  /* this copes with the case that ply_failure<ply_start */
  for (ply = ply_start; ply<=ply_failure; ++ply)
    if (trait[ply]==side_in_check)
    {
      move_effect_journal_index_type const base = move_effect_journal_base[ply];
      move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
      square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
      if (sq_departure==capture_by_invisible
          || sq_departure==move_by_invisible)
      {
        result = true;
        break;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean failure_to_intercept_illegal_checks_continue_level(decision_level_type curr_level)
{
  boolean skip = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr_level);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_failure);
  TraceEnumerator(Side,side_failure);
  TraceValue("%u",decision_level_properties[curr_level].ply);
  TraceValue("%u",decision_level_properties[curr_level].purpose);
  TraceValue("%u",decision_level_properties[curr_level].object);
  TraceValue("%u",decision_level_properties[curr_level].id);
  TraceEnumerator(Side,decision_level_properties[curr_level].side);
  TraceValue("%u",backtracking[curr_level].ply_failure);
  TraceValue("%u",try_to_avoid_insertion[White]);
  TraceValue("%u",try_to_avoid_insertion[Black]);
  TraceEOL();

  assert(decision_level_properties[curr_level].ply!=0);

  switch (decision_level_properties[curr_level].purpose)
  {
    case decision_purpose_invisible_capturer_existing:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_walk:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            /* try harder.
             * a future decision may select
             * - a walk that allows us to eventually intercept the check
             */
          }
          else
            skip = true;
          break;

        case decision_object_departure:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            /* try harder.
             * a future decision may
             * - select a piece that can intercept the check
             */
             /* e.g.
             Michel Caillaud
  Sake tourney 2018, 1st HM, cooked (author's solution relies on retro and is not shown)

  +---a---b---c---d---e---f---g---h---+
  |                                   |
  8   .   .   .   .  -R   .  -R  -B   8
  |                                   |
  7   .   .   .  -B   .   .   .   .   7
  |                                   |
  6   .   .   .   P  -K   .   .   R   6
  |                                   |
  5   .   .   .   .   .   .   .   .   5
  |                                   |
  4   .   .   .   P   .   .   K   .   4
  |                                   |
  3   .   .   .   .   .   .   .   .   3
  |                                   |
  2   B   .   .  -P  -Q   .   .   .   2
  |                                   |
  1   .   .   .   .   .   .   .   .   1
  |                                   |
  +---a---b---c---d---e---f---g---h---+
    h#2                  5 + 7 + 3 TI

  >   1.TI~-~ TI~-~   2.TI~-~[d5=bR][f3=wR] TI~-~[g7=wP] #

  !test_mate 6:TI~-~ 7:TI~-~ 8:TI~-~ 9:TI~-~ - total_invisible.c:#551 - D:50211 - 38436
  use option start 1:1:1:1 to replay
  !  2 + 6 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#512 - D:50212
  !   3 + 6 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#475 - D:62222
  !    4 + 6 R (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#253 - D:70672
  !     5 + 6 g7 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#512 - D:87214
  !      6 + 6 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#475 - D:88572
  !       7 + 6 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#253 - D:88574
  !        8 > 6 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+2) - random_move_by_invisible.c:#552 - D:88576
  !         9 > 6 f1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+2) - random_move_by_invisible.c:#25 - D:88862
  !          10 7 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#644

  HERE

  !        8 > 6 g7
             */
          }
          else
          {
            /* try harder.
             * a future decision may
             * - select a departure square where this piece intercepted the check before moving
             */
          }
          break;

        default:
          assert(0);
          break;
      }
      break;

    case decision_purpose_invisible_capturer_inserted:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_insertion:
          assert(0);
          break;

        case decision_object_walk:
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
          {
            // TODO rather than calculating nbply-2, we should backtrack to the last random move of the side
            if (decision_level_properties[curr_level].ply
                <backtracking[curr_level].ply_failure-2)
            {
              /* try harder.
               * a future decision may select
               * - a walk that allows us to eventually intercept the check
               */
              /* e.g.

  begin
  author Michel Caillaud
  origin Sake tourney 2018, 1st prize, corrected
  pieces TotalInvisible 2 white kd1 qb2 black kf4 rh1 be1 pe4f5h3
  stipulation h#2
  option movenum start 4:4:4:21
  end

             Michel Caillaud
  Sake tourney 2018, 1st prize, corrected

  +---a---b---c---d---e---f---g---h---+
  |                                   |
  8   .   .   .   .   .   .   .   .   8
  |                                   |
  7   .   .   .   .   .   .   .   .   7
  |                                   |
  6   .   .   .   .   .   .   .   .   6
  |                                   |
  5   .   .   .   .   .  -P   .   .   5
  |                                   |
  4   .   .   .   .  -P  -K   .   .   4
  |                                   |
  3   .   .   .   .   .   .   .  -P   3
  |                                   |
  2   .   Q   .   .   .   .   .   .   2
  |                                   |
  1   .   .   .   K  -B   .   .  -R   1
  |                                   |
  +---a---b---c---d---e---f---g---h---+
    h#2                  2 + 6 + 2 TI

  !validate_mate 6:Be1-g3 7:TI~-g3 8:Rh1-g1 9:Qb2-b8 - total_invisible.c:#521 - D:165 - 122
  use option start 4:4:4:21 to replay
  !  2 X 7 I (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#1154 - D:166
  !   3 X 7 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#517 - D:168
  ...
  !   3 X 7 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#456 - D:184
  !    4 X 7 h5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - capture_by_invisible.c:#49 - D:186
  !     5 7 capturer would deliver uninterceptable check - capture_by_invisible.c:#56
  !    4 X 7 f1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - capture_by_invisible.c:#49 - D:188
  !     5 + 8 e1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#171 - D:190
  !     5 + 8 f1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#171 - D:192
  !     5 + 8 g1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#171 - D:194
  !      6 + 8 w (K:0+0 x:0+0 !:0+0 ?:1+0 F:1+0) - intercept_illegal_checks.c:#107 - D:196
  !       7 10 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#644

  Here! BTW: ply_skip-3 would be too strong

  !   3 X 7 B (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#354 - D:198
  ...
  !   3 X 7 R (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#354 - D:218
  ...
               */
            }
            else
              skip = true;
          }
          else
          {
            /* try harder.
             * a future decision may select
             * - a walk that doesn't deliver check
             *   - from the departure square (if the capture was after ply_failure)
             *   - from the arrival square (if the capture was before ply_failure)
             */
          }
          break;

        case decision_object_departure:
          if (decision_level_properties[curr_level].ply
              >backtracking[curr_level].ply_failure)
          {
            if (decision_level_properties[curr_level].side
                ==backtracking[curr_level].side_failure)
            {
              /* try harder.
               * a future decision may select
               * - a square where we aren't in check
               */
               /* e.g.

               Ofer Comay
  Sake tourney 2018, 3rd HM, cooked (and 1 authors solution doesnt deliver mate)

  +---a---b---c---d---e---f---g---h---+
  |                                   |
  8   .   .   .   .   .   .   .   Q   8
  |                                   |
  7   .   P   .   .   .   .   .   .   7
  |                                   |
  6   .   .   .   .   .   .   .   .   6
  |                                   |
  5   .   .   .   .   K   .   .   .   5
  |                                   |
  4   .  -R   .   .   .   .   .   P   4
  |                                   |
  3   .   .   .   .   .   .   .   .   3
  |                                   |
  2   .   .   P   .   .  -S   .   .   2
  |                                   |
  1  -B   .   B   .  -R  -B   .   .   1
  |                                   |
  +---a---b---c---d---e---f---g---h---+
    h#2                  6 + 5 + 3 TI

  !validate_mate 6:TI~-~ 7:TI~-~ 8:TI~-c2 9:Ke5-d4 - total_invisible.c:#521 - D:2960009 - 16400
  use option start 1:1:5:15 to replay

  !validate_mate 6:TI~-~ 7:TI~-~ 8:TI~-c2 9:Ke5-d4 - total_invisible.c:#521 - D:3365 - 2414
  use option start 1:1:5:15 to replay
  !  2 + 6 d4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#171 - D:3366
  !   3 + 6 w (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+0) - intercept_illegal_checks.c:#107 - D:3368
  !    4 + 6 e4 (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+0) - intercept_illegal_checks.c:#171 - D:3370
  !     5 + 6 w (K:0+1 x:0+0 !:0+0 ?:2+0 F:0+0) - intercept_illegal_checks.c:#107 - D:3372
  !      6 > 6 TI~-~ (K:0+1 x:0+0 !:0+1 ?:2+0 F:0+0) - random_move_by_invisible.c:#579 - D:3374
  !       7 > 7 d4 (K:0+1 x:0+0 !:0+1 ?:2+0 F:0+0) - random_move_by_invisible.c:#552 - D:3376
  !        8 > 7 P (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#349 - D:3378
  ...
  !        8 > 7 B (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#376 - D:3670
  !         9 > 7 c5 (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#25 - D:3672
  ...
  !         9 > 7 c3 (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#25 - D:3816
  !          10 < 6 TI~-~ (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#1029 - D:3818
  !           11 > 6 TI~-~ (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#579 - D:3820
  !            12 X 8 I (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - capture_by_invisible.c:#1154 - D:3822
  !             13 X 8 K (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - capture_by_invisible.c:#393 - D:3824
  !              14 X 8 d2 (K:0+1 x:0+0 !:0+0 ?:1+0 F:1+1) - capture_by_invisible.c:#49 - D:3826
  !               15 < 6 d2 (K:0+1 x:0+0 !:0+0 ?:1+0 F:1+1) - random_move_by_invisible.c:#993 - D:3828
  !                16 < 6 d1 (K:0+1 x:0+0 !:0+0 ?:1+0 F:1+1) - random_move_by_invisible.c:#623 - D:3830
  !                 17 7 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#642

  HERE

  !              14 X 8 b2 (K:0+1 x:0+0 !:0+0 ?:1+0 F:1+1) - capture_by_invisible.c:#49 - D:3700
  ...
  !              14 X 8 d1 (K:0+1 x:0+0 !:0+0 ?:1+0 F:1+1) - capture_by_invisible.c:#49 - D:3736

               */
            }
            else
            {
              /* try harder.
               * a future decision may select
               * - a square from where we don't deliver check
               */
            }
          }
          else
            skip = true;
          break;

        case decision_object_move_vector:
          if ((decision_level_properties[curr_level].ply
               >backtracking[curr_level].ply_failure)
              && (decision_level_properties[curr_level].side!=backtracking[curr_level].side_failure))
          {
            /* try harder.
             * a future decision may select
             * - a move vector from where we don't deliver check
             */
             /* e.g.
  begin
  author Michel Caillaud
  origin Sake tourney 2018, 2nd HM, cooked
  pieces TotalInvisible 2 white kf6 rd3 bd2 sd6 pf2f5g5 black ka4 qh1
  stipulation h#2
  option movenum start 8:11:15:1
  end

             Michel Caillaud
    Sake tourney 2018, 2nd HM, cooked

  +---a---b---c---d---e---f---g---h---+
  |                                   |
  8   .   .   .   .   .   .   .   .   8
  |                                   |
  7   .   .   .   .   .   .   .   .   7
  |                                   |
  6   .   .   .   S   .   K   .   .   6
  |                                   |
  5   .   .   .   .   .   P   P   .   5
  |                                   |
  4  -K   .   .   .   .   .   .   .   4
  |                                   |
  3   .   .   .   R   .   .   .   .   3
  |                                   |
  2   .   .   .   B   .   P   .   .   2
  |                                   |
  1   .   .   .   .   .   .   .  -Q   1
  |                                   |
  +---a---b---c---d---e---f---g---h---+
    h#2                  7 + 2 + 2 TI

  !validate_mate 6:Qh1-a8 7:Pf2-g3 8:Qa8-e4 9:TI~-e4 - total_invisible.c:#521 - D:3 - 0
  use option start 8:11:15:1 to replay
  !  2 7 adding victim of capture by pawn - total_invisible.c:#374
  !  2 X 9 I (K:0+0 x:0+1 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#1154 - D:4
  !   3 X 9 P (K:0+0 x:0+1 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#517 - D:6
  !    4 9 capturer can't be placed on taboo square - capture_by_invisible.c:#35
  ...
  !   3 X 9 R (K:0+0 x:0+1 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#354 - D:24
  !    4 X 9 direction:1 (K:0+0 x:0+1 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#360 - D:26
  !     5 X 9 f4 (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - capture_by_invisible.c:#49 - D:28
  !      6 7 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#644
  !     5 X 9 g4 (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - capture_by_invisible.c:#49 - D:30
  !      6 7 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#644
  !     5 X 9 h4 (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - capture_by_invisible.c:#49 - D:32
  !      6 7 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#644

  HERE

  !    4 X 9 direction:2 (K:0+0 x:0+1 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#360 - D:34
  ...
  !    4 X 9 direction:4 (K:0+0 x:0+1 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#360 - D:52
  !     5 X 9 e3 (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - capture_by_invisible.c:#49 - D:54
  !      6 10 Replaying moves for validation - king_placement.c:#29
  !     5 X 9 e2 (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - capture_by_invisible.c:#49 - D:56
  !      6 10 Replaying moves for validation - king_placement.c:#29
  !     5 X 9 e1 (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - capture_by_invisible.c:#49 - D:58
  !      6 10 Replaying moves for validation - king_placement.c:#29
               */
          }
          else
            skip = true;
          break;

        default:
          assert(0);
          break;
      }
      break;

    case decision_purpose_mating_piece_attacker:
      assert(decision_level_properties[curr_level].side!=no_side);
      break;

    case decision_purpose_illegal_check_interceptor:
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_walk:
          assert(decision_level_properties[curr_level].side!=no_side);
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
          {
            if (find_random_move_of_side_between_plies(decision_level_properties[curr_level].ply,
                                                       backtracking[curr_level].ply_failure,
                                                       backtracking[curr_level].side_failure))
            {
              /* try harder
               * a random move by invisible according to the selected walk may help us out
               */
              /* e.g.
begin
author Michel Caillaud
origin Sake tourney 2018, 1st HM, cooked (author's solution relies on retro and is not shown)
pieces TotalInvisible 3 white kg4 rh6 ba2 pd4d6 black ke6 qe2 re8g8 bd7h8 pd2
stipulation h#2
end

           Michel Caillaud
Sake tourney 2018, 1st HM, cooked (author's solution relies on retro and is not shown)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .  -R   .  -R  -B   8
|                                   |
7   .   .   .  -B   .   .   .   .   7
|                                   |
6   .   .   .   P  -K   .   .   R   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   P   .   .   K   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   B   .   .  -P  -Q   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  5 + 7 + 3 TI

!test_mate 6:Ke6-f6 7:TI~-~ 8:Rg8-g6 9:TI~-g6 - total_invisible.c:#539 - D:3280 - 584
use option start 38:0:59:4 to replay
! >2 + 6 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#533 - D:3282
!  >3 + 6 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#496 - D:3283
!   >4 + 6 Q (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#299 - D:3284
...
!   <4 - r:1 t:3 m:7 n:4294967295 i:14
!   >4 + 6 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#299 - D:3513
!    >5 + 6 g5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#533 - D:3514
!     >6 + 6 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#496 - D:3515
!      >7 + 6 Q (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#299 - D:3516
...
!      >7 + 6 R (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:-1 n:4294967295 i:15 - intercept_illegal_checks.c:#299 - D:3531
!       >8 + 7 g6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:-1 n:4294967295 i:16 - intercept_illegal_checks.c:#533 - D:3532
!        >9 + 7 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:2+0) - r:1 t:0 m:-1 n:4294967295 i:16 - intercept_illegal_checks.c:#496 - D:3533
!         >10 + 7 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:16 - intercept_illegal_checks.c:#299 - D:3534
!          >11 > 7 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#544 - D:3535
!           >12 > 7 f4 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:14 - random_move_by_invisible.c:#25 - D:3536
!             13 8 only 0 available invisibles for intercepting 2 illegal checks - intercept_illegal_checks.c:#666
!           <12 - r:1 t:3 m:12 n:2 i:14
!          <11 - r:1 t:3 m:12 n:2 i:14
!          >11 > 7 g5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#544 - D:3537
!           >12 > 7 h5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:3538
!             13 8 only 0 available invisibles for intercepting 1 illegal checks - intercept_illegal_checks.c:#666
!           <12 - r:1 t:3 m:12 n:1 i:15
!           >12 > 7 f5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:3539
!             13 8 capture in ply 9 will not be possible - intercept_illegal_checks.c:#74
!           <12 - r:1 t:4 m:4294967295 n:4294967295 i:15
!           >12 > 7 e5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:3540
!             13 8 only 0 available invisibles for intercepting 1 illegal checks - intercept_illegal_checks.c:#666
!           <12 - r:1 t:3 m:12 n:1 i:15
!           >12 > 7 d5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:3541
!             13 8 only 0 available invisibles for intercepting 1 illegal checks - intercept_illegal_checks.c:#666
!           <12 - r:1 t:3 m:12 n:1 i:15
!           >12 > 7 c5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:3542
!             13 8 only 0 available invisibles for intercepting 1 illegal checks - intercept_illegal_checks.c:#666
!           <12 - r:1 t:3 m:12 n:1 i:15
!           >12 > 7 b5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:3543
!             13 8 only 0 available invisibles for intercepting 1 illegal checks - intercept_illegal_checks.c:#666
!           <12 - r:1 t:3 m:12 n:1 i:15
!           >12 > 7 a5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:15 - random_move_by_invisible.c:#25 - D:3544
!             13 8 only 0 available invisibles for intercepting 1 illegal checks - intercept_illegal_checks.c:#666
!           <12 - r:1 t:3 m:12 n:1 i:15
!          <11 - r:1 t:3 m:12 n:1 i:15
!          >11 > 7 g6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:3+0) - r:1 t:0 m:-1 n:4294967295 i:16 - random_move_by_invisible.c:#544 - D:3545
!          <11 - r:1 t:0 m:4294967295 n:4294967295 i:16
!         <10 - r:1 t:3 m:12 n:1 i:16

WE HAVE TO TRY OTHER WALKS - E.G. BISHOP TO ALLOW BG6-F5 IN PLY 7
               */
            }
            else
            {
              skip = true;
              /* e.g.
begin
origin 1...Rh1-c1   2..~-~ Bh8-c3 # is not a solution because a bTI can have done d4-b2
pieces TotalInvisible 2 white ka5 rh1 bh8 black ka1 pa2
stipulation h#1.5
option movenum start 1:2
end

1...Rh1-c1   2..~-~ Bh8-c3 # is not a solution because a bTI can have done d4-b2

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   B   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   K   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2  -P   .   .   .   .   .   .   .   2
|                                   |
1  -K   .   .   .   .   .   .   R   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#1.5                3 + 2 + 2 TI

!make_revelations 6:TI~-~ 7:TI~-h8 - revelations.c:#1430 - D:35 - 34
use option start 1:2 to replay
! >2 + 6 b2 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 i:7 - intercept_illegal_checks.c:#487 - D:37
!  >3 + 6 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 i:7 - intercept_illegal_checks.c:#450 - D:39
!   >4 + 6 Q (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 i:7 - intercept_illegal_checks.c:#253 - D:41
!    >5 + 6 b1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 i:8 - intercept_illegal_checks.c:#487 - D:43
...
!    >5 + 6 g1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 i:8 - intercept_illegal_checks.c:#487 - D:309
!     >6 + 6 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 i:8 - intercept_illegal_checks.c:#450 - D:311
...
!     >6 + 6 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 i:8 - intercept_illegal_checks.c:#450 - D:313
!       7 6 pawn is placed on impossible square - intercept_illegal_checks.c:#292
!      >7 + 6 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 i:8 - intercept_illegal_checks.c:#253 - D:315
...
!      >7 + 6 B (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 i:8 - intercept_illegal_checks.c:#253 - D:331
!       >8 > 6 g1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 i:8 - random_move_by_invisible.c:#544 - D:333
!        >9 > 6 f2 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 i:8 - random_move_by_invisible.c:#25 - D:335
...
!        >9 > 6 h2 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 i:8 - random_move_by_invisible.c:#25 - D:359
!         >10 x 7 b2 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 i:6 - capture_by_invisible.c:#800 - D:361
!           11 8 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#619
!         <10 - r:1 t:3 m:10
!        <9 - r:1 t:3 m:10
!       <8 - r:1 t:3 m:10
!      <7 - r:1 t:3 m:10
!     <6 - r:1 t:3 m:10
!    <5 - r:1 t:3 m:10
!   <4 - r:1 t:3 m:10

HERE - no need to try other walks on b2
             */
            }
          }
          break;

        default:
          break;
      }
      break;

    case decision_purpose_random_mover_forward:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_walk:
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
          {
            if (decision_level_properties[curr_level].ply
                >backtracking[curr_level].ply_failure)
              skip = true;
            else
            {
              /* try harder.
               * a future decision may select
               * - a walk that allows us to eventually intercept the check
               */
            }
          }
          else
          {
            /* try harder.
             * a future decision may select
             * - a walk that allows us to eventually intercept the check
             * - a walk that doesn't deliver the check
             */
            /*
             * e.g. (walk that doesn't deliver the check)
begin
author Vlaicu Crisan, Eric Huber
origin Sake tourney 2018, 7th HM, cooked. Missing solutions because of missing Popeye feature
pieces TotalInvisible 2 white ke1 qd3 pf2 black kg2
stipulation h#2
option movenum start 6:1:0:35
end

      Vlaicu Crisan, Eric Huber
Sake tourney 2018, 7th HM, cooked. Missing solutions because of missing Popeye feature

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   Q   .   .   .   .   3
|                                   |
2   .   .   .   .   .   P  -K   .   2
|                                   |
1   .   .   .   .   K   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  3 + 1 + 2 TI

!validate_mate 6:Kg2-h3 7:Ke1-c1 8:TI~-~ 9:Qd3-g3 - total_invisible.c:#521 - D:55 - 22
use option start 6:1:0:35 to replay
!  2 7 adding castling partner - total_invisible.c:#451
!  2 + 7 g3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#171 - D:56
...
!  2 + 7 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#171 - D:142
!   3 + 7 b (K:0+0 x:0+0 !:0+0 ?:0+1 F:1+0) - intercept_illegal_checks.c:#107 - D:144
!    4 > 8 f3 (K:0+0 x:0+0 !:0+0 ?:0+1 F:1+0) - random_move_by_invisible.c:#552 - D:146
!     5 > 8 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#349 - D:148
!     5 > 8 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#363 - D:150
!      6 > 8 d4 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#25 - D:152
!       7 6 uninterceptable illegal check by invisible piece from dir:49 by id:6 delivered in ply:8 - intercept_illegal_checks.c:#658

HERE! bS delivers check from f3, but B and (more importantly) R don't

!     5 > 8 B (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#376 - D:154
...
!     5 > 8 R (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#391 - D:176
!      6 > 8 g3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#25 - D:178
!       7 10 Replaying moves for validation - king_placement.c:#29
             */
            // TODO take the checker's id into consideration to tell the cases apart?
            // TODO take the ply into consideration in the first case?
          }
          break;

        case decision_object_departure:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            /* try harder.
             * a future decision may
             * - select a piece that can intercept the check
             */
             /* e.g.
             Michel Caillaud
  Sake tourney 2018, 1st HM, cooked (author's solution relies on retro and is not shown)

  +---a---b---c---d---e---f---g---h---+
  |                                   |
  8   .   .   .   .  -R   .  -R  -B   8
  |                                   |
  7   .   .   .  -B   .   .   .   .   7
  |                                   |
  6   .   .   .   P  -K   .   .   R   6
  |                                   |
  5   .   .   .   .   .   .   .   .   5
  |                                   |
  4   .   .   .   P   .   .   K   .   4
  |                                   |
  3   .   .   .   .   .   .   .   .   3
  |                                   |
  2   B   .   .  -P  -Q   .   .   .   2
  |                                   |
  1   .   .   .   .   .   .   .   .   1
  |                                   |
  +---a---b---c---d---e---f---g---h---+
    h#2                  5 + 7 + 3 TI

  >   1.TI~-~ TI~-~   2.TI~-~[d5=bR][f3=wR] TI~-~[g7=wP] #

  !test_mate 6:TI~-~ 7:TI~-~ 8:TI~-~ 9:TI~-~ - total_invisible.c:#551 - D:50211 - 38436
  use option start 1:1:1:1 to replay
  !  2 + 6 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#512 - D:50212
  !   3 + 6 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#475 - D:62222
  !    4 + 6 R (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#253 - D:70672
  !     5 + 6 g7 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#512 - D:87214
  !      6 + 6 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#475 - D:88572
  !       7 + 6 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#253 - D:88574
  !        8 > 6 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+2) - random_move_by_invisible.c:#552 - D:88576
  !         9 > 6 f1 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+2) - random_move_by_invisible.c:#25 - D:88862
  !          10 7 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#644

  HERE

  !        8 > 6 g7
             */
          }
          else
          {
            /* try harder.
             * a future decision may
             * - select a departure square where this piece intercepted the check before moving
             */
          }
          break;

        case decision_object_arrival:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            /* try harder.
             * a future decision may select
             * - an arrival square from where the check can be intercepted
             */
          }
          else
            skip = true;
          break;

        default:
          assert(0);
          break;
      }
      break;

    case decision_purpose_random_mover_backward:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_walk:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            /* try harder.
             * a future decision may select
             * - a walk that allows us to eventually intercept the check
             */
          }
          else
            skip = true;
          break;

        case decision_object_departure:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            skip = true;
            /* e.g.
  begin
  author Ofer Comay
  origin Sake tourney 2018, 3rd HM, cooked (and 1 author's solution doesn't deliver mate)
  pieces TotalInvisible 3 white ke5 qh8 bc1 pb7c2h4 black rb4e1 ba1f1 sf2
  stipulation h#2
  option movenum start  1:1:5:17
  end

               Ofer Comay
  Sake tourney 2018, 3rd HM, cooked (and 1 authors solution doesnt deliver mate)

  +---a---b---c---d---e---f---g---h---+
  |                                   |
  8   .   .   .   .   .   .   .   Q   8
  |                                   |
  7   .   P   .   .   .   .   .   .   7
  |                                   |
  6   .   .   .   .   .   .   .   .   6
  |                                   |
  5   .   .   .   .   K   .   .   .   5
  |                                   |
  4   .  -R   .   .   .   .   .   P   4
  |                                   |
  3   .   .   .   .   .   .   .   .   3
  |                                   |
  2   .   .   P   .   .  -S   .   .   2
  |                                   |
  1  -B   .   B   .  -R  -B   .   .   1
  |                                   |
  +---a---b---c---d---e---f---g---h---+
    h#2                  6 + 5 + 3 TI

  !validate_mate 6:TI~-~ 7:TI~-~ 8:TI~-c2 9:Ke5-f6 - total_invisible.c:#521 - D:3365 - 2414
  use option start 1:1:5:17 to replay
  !  2 + 6 d4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#171 - D:3366
  !   3 + 6 w (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+0) - intercept_illegal_checks.c:#107 - D:3368
  !    4 + 6 e4 (K:0+1 x:0+0 !:0+0 ?:1+0 F:0+0) - intercept_illegal_checks.c:#171 - D:3370
  !     5 + 6 w (K:0+1 x:0+0 !:0+0 ?:2+0 F:0+0) - intercept_illegal_checks.c:#107 - D:3372
  !      6 > 6 TI~-~ (K:0+1 x:0+0 !:0+1 ?:2+0 F:0+0) - random_move_by_invisible.c:#579 - D:3374
  !       7 > 7 d4 (K:0+1 x:0+0 !:0+1 ?:2+0 F:0+0) - random_move_by_invisible.c:#552 - D:3376
  !        8 > 7 P (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#349 - D:3378
  !         9 > 7 d5 (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#25 - D:3380
  !          10 < 6 TI~-~ (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#1029 - D:3382
  !           11 > 6 TI~-~ (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - random_move_by_invisible.c:#579 - D:3384
  !            12 + 8 d4 (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - intercept_illegal_checks.c:#171 - D:3386
  !            12 + 8 c3 (K:0+1 x:0+0 !:0+1 ?:1+0 F:1+0) - intercept_illegal_checks.c:#171 - D:3388
  !             13 8 not enough available invisibles of side White for intercepting all illegal checks - intercept_illegal_checks.c:#135
  !             13 + 8 b (K:0+1 x:0+0 !:0+0 ?:1+1 F:1+0) - intercept_illegal_checks.c:#107 - D:3390
  !              14 x 8 c3 (K:0+1 x:0+0 !:0+0 ?:1+1 F:1+0) - capture_by_invisible.c:#808 - D:3392
  !               15 x 8 K (K:0+1 x:0+0 !:0+0 ?:1+1 F:1+0) - capture_by_invisible.c:#215 - D:3394
  !                16 < 6 c3 (K:0+1 x:0+0 !:0+0 ?:1+0 F:1+1) - random_move_by_invisible.c:#993 - D:3396
  !                 17 < 6 b3 (K:0+1 x:0+0 !:0+0 ?:1+0 F:1+1) - random_move_by_invisible.c:#623 - D:3398
  !                  18 10 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#642

  HERE! no need to try other departure squares

  !              14 X 8 I (K:0+1 x:0+0 !:0+0 ?:1+1 F:1+0) - capture_by_invisible.c:#1154 - D:3400
             */
          }
          break;

        default:
          break;
      }
      break;

    case decision_purpose_king_nomination:
      break;

    default:
      break;
  }

  if (decision_level_properties[curr_level].object==decision_object_departure
    || decision_level_properties[curr_level].object==decision_object_arrival
    || decision_level_properties[curr_level].object==decision_object_walk)
  {
    assert(decision_level_properties[curr_level].side!=no_side);
    if (try_to_avoid_insertion[decision_level_properties[curr_level].side])
    {
      if (skip)
      {
#if defined(REPORT_DECISIONS)
        printf("!%*s%d ",next_decision_level,"",next_decision_level);
        printf("trying to avoid an insertion so that we can intercept the check with an insertion\n");
#endif

        skip = false;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",skip);
  TraceFunctionResultEnd();
  return skip;
}

/* Optimise backtracking considering that we have
 * reached a position where we aren't able to intercept all illegal checks by
 * inserting invisibles.
 * @param side_in_check the side that is in too many illegal checks
 */
void backtrack_from_failure_to_intercept_illegal_check(Side side_in_check,
                                                       unsigned int nr_check_vectors)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceFunctionParamListEnd();

  assert(backtracking[next_decision_level-1].type==backtrack_none);
  assert(backtracking[next_decision_level-1].max_level==decision_level_latest);

  backtracking[next_decision_level-1].type = backtrack_failure_to_intercept_illegal_checks;
  backtracking[next_decision_level-1].nr_check_vectors = nr_check_vectors;
  backtracking[next_decision_level-1].ply_failure = nbply;
  backtracking[next_decision_level-1].side_failure = side_in_check;

  try_to_avoid_insertion[Black] = false;
  try_to_avoid_insertion[White] = false;

  if (decision_level_properties[next_decision_level-1].purpose==decision_purpose_random_mover_forward
      /* restrict this to fleshed out random moves */
      && decision_level_properties[next_decision_level-1].object==decision_object_arrival)
  {
    if (nr_check_vectors>nr_placeable_invisibles_for_both_sides()+1)
      /* the situation is hopeless */
      decision_level_properties[next_decision_level-1].relevance = relevance_irrelevant;

    if (backtracking[next_decision_level-2].type==backtrack_failure_to_intercept_illegal_checks
        && (nr_check_vectors>backtracking[next_decision_level-2].nr_check_vectors)
        && decision_level_properties[next_decision_level].ply+1==nbply)
      /* moving the currently moving pieces makes the situation worse */
      decision_level_properties[next_decision_level-1].relevance = relevance_irrelevant;
  }

  if (decision_level_properties[next_decision_level-1].purpose==decision_purpose_random_mover_backward
      && decision_level_properties[next_decision_level-1].object==decision_object_departure
      && nr_check_vectors>nr_placeable_invisibles_for_both_sides()+1)
    decision_level_properties[next_decision_level-1].relevance = relevance_irrelevant;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void backtrack_from_failure_to_capture_uninterceptable_checker(Side side_in_check,
                                                               unsigned int nr_check_vectors)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceFunctionParamListEnd();

  assert(backtracking[next_decision_level-1].type==backtrack_none);
  assert(backtracking[next_decision_level-1].max_level==decision_level_latest);

  backtracking[next_decision_level-1].type = backtrack_failure_to_capture_uninterceptable_checker;
  backtracking[next_decision_level-1].ply_failure = nbply;
  backtracking[next_decision_level-1].side_failure = side_in_check;

  try_to_avoid_insertion[Black] = false;
  try_to_avoid_insertion[White] = false;

  if (decision_level_properties[next_decision_level-1].purpose==decision_purpose_random_mover_forward
      /* restrict this to fleshed out random moves */
      && decision_level_properties[next_decision_level-1].object==decision_object_arrival)
    decision_level_properties[next_decision_level-1].relevance = relevance_irrelevant;

  if (decision_level_properties[next_decision_level-1].purpose==decision_purpose_random_mover_backward
      && decision_level_properties[next_decision_level-1].object==decision_object_departure)
    decision_level_properties[next_decision_level-1].relevance = relevance_irrelevant;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean failure_to_capture_by_invisible_continue_level(decision_level_type curr_level)
{
  boolean skip = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr_level);
  TraceFunctionParamListEnd();

  TraceValue("%u",curr_level);
  TraceValue("%u",ply_failure);
  TraceEnumerator(Side,side_failure);
  TraceValue("%u",decision_level_properties[curr_level].ply);
  TraceValue("%u",decision_level_properties[curr_level].purpose);
  TraceValue("%u",decision_level_properties[curr_level].object);
  TraceValue("%u",decision_level_properties[curr_level].id);
  TraceEnumerator(Side,decision_level_properties[curr_level].side);
  TraceValue("%u",try_to_avoid_insertion[White]);
  TraceValue("%u",try_to_avoid_insertion[Black]);
  TraceEOL();

  assert(decision_level_properties[curr_level].ply!=0);

  switch (decision_level_properties[curr_level].purpose)
  {
    case decision_purpose_random_mover_backward:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_walk:
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
          {
            if (decision_level_properties[curr_level].ply
                <=backtracking[curr_level].ply_failure)
            {
              /* try harder.
               * a future decision may
               * - select a better walk
               */
            }
            else
              skip = true;
          }
          else
            skip = true;
          break;

        default:
          skip = true;
          break;
      }
      break;

    case decision_purpose_random_mover_forward:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_departure:
        case decision_object_walk:
        case decision_object_arrival:
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
          {
            if (decision_level_properties[curr_level].ply
                <=backtracking[curr_level].ply_failure)
            {
              /* try harder.
               * a future decision may
               * - select a better walk
               * - select a better arrival square
               * - leave alone the mover that can eventually do the capture
               */
            }
            else
              skip = true;
          }
          else
          {
            if (decision_level_properties[curr_level].ply
                <=backtracking[curr_level].ply_failure)
            {
              /* try harder.
               * a future decision may
               * - avoid capturing a viable capturer
               */
              // TODO doesn't this only apply to decision_object_arrival?
            }
            else
              skip = true;
          }
          break;

        case decision_object_random_move:
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
            skip = true;
          else
          {
            if (decision_level_properties[curr_level].ply
                <=backtracking[curr_level].ply_failure)
            {
              /* try harder.
               * a future decision may
               * - avoid capturing a viable capturer
               */
            }
            else
              skip = true;
          }
          break;

        default:
          assert(0);
          break;
      }
      break;

    case decision_purpose_invisible_capturer_existing:
    case decision_purpose_invisible_capturer_inserted:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_walk:
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
          {
            if (decision_level_properties[curr_level].ply
                <=backtracking[curr_level].ply_failure)
            {
              /* try harder.
               * a future decision may
               * - select a better walk
               */
            }
            else
              skip = true;
          }
          else
          {
            if (decision_level_properties[curr_level].ply
                <backtracking[curr_level].ply_failure-1)
            {
              /* try harder
               * a different walk may allow a capturer more possibilities
               */
              /* e.g.
begin
author Manfred Rittirsch
origin Sake tourney 2018, 3rd prize, correction
pieces TotalInvisible 2 white kf1 bd6 sb3 pc2d2g6h5 black ka4 pc3d7
stipulation h#2
option movenum
end

          Manfred Rittirsch
Sake tourney 2018, 3rd prize, correction

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .  -P   .   .   .   .   7
|                                   |
6   .   .   .   B   .   .   P   .   6
|                                   |
5   .   .   .   .   .   .   .   P   5
|                                   |
4  -K   .   .   .   .   .   .   .   4
|                                   |
3   .   S  -P   .   .   .   .   .   3
|                                   |
2   .   .   P   P   .   .   .   .   2
|                                   |
1   .   .   .   .   .   K   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  7 + 3 + 2 TI

348,350c348,349
<   1.TI~*h5 Kf1-e2   2.TI~*g6 TI~*d7[d7=wB][g6=bQ] #

!validate_mate 6:TI~-h5 7:Kf1-e2 8:TI~-g6 9:TI~-d7 - total_invisible.c:#514 - D:46 - 20
use option start 4:4:1:1 to replay
! >2 X 6 I (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#1097 - D:48
!  >3 X 6 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#521 - D:49
!  <3 - r:1 t:0 m:4294967295 n:4294967295 i:11
!  >3 X 6 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#460 - D:50
!   >4 X 6 f6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#49 - D:51
!     5 7 capture in ply 8 will not be possible - intercept_illegal_checks.c:#74
!   <4 - r:1 t:4 m:4294967295 n:4294967295 i:11
!  <3 - r:1 t:4 m:4294967295 n:4294967295 i:11
!  >3 X 6 B (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#358 - D:52
!   >4 X 6 direction:5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#364 - D:53
!   <4 - r:1 t:0 m:4294967295 n:4294967295 i:11
!   >4 X 6 direction:6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#364 - D:54
!   <4 - r:1 t:0 m:4294967295 n:4294967295 i:11
!   >4 X 6 direction:7 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#364 - D:55
!   <4 - r:1 t:0 m:4294967295 n:4294967295 i:11
!   >4 X 6 direction:8 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#364 - D:56
!    >5 X 6 g4 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:11 - capture_by_invisible.c:#49 - D:57
!     >6 + 8 f3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#215 - D:58
!      >7 + 8 w (K:0+0 x:0+0 !:0+0 ?:1+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#111 - D:59
!        8 8 capture in ply 9 will not be possible - intercept_illegal_checks.c:#74
!      <7 - r:1 t:4 m:4294967295 n:4294967295 i:14
!     <6 - r:1 t:4 m:4294967295 n:4294967295 i:14
!     >6 + 8 g4 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:-1 n:4294967295 i:14 - intercept_illegal_checks.c:#215 - D:60
!     <6 - r:1 t:0 m:4294967295 n:4294967295 i:14
!    <5 - r:1 t:4 m:4294967295 n:4294967295 i:11
!   <4 - r:1 t:4 m:4294967295 n:4294967295 i:11
!  <3 - r:1 t:4 m:4294967295 n:4294967295 i:11

HERE - TRY ROOK AND QUEEN AS WELL

! <2 - r:1 t:4 m:4294967295 n:4294967295 i:11
               */
            }
            else
              skip = true;
          }
          break;

        case decision_object_departure:
          if (decision_level_properties[curr_level].side
              ==backtracking[curr_level].side_failure)
          {
            if (decision_level_properties[curr_level].ply
                <backtracking[curr_level].ply_failure)
              skip = true;
            else
            {
              /* try harder.
               * a future decision may
               * - leave alone the mover that can eventually do the capture
               */
            }
          }
          else
            skip = true;
          break;

        default:
          skip = true;
          break;
      }
      break;

    case decision_purpose_illegal_check_interceptor:
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_side:
        if (decision_level_properties[curr_level].side
            ==backtracking[curr_level].side_failure)
        {
          if ((decision_level_properties[curr_level].ply
               ==backtracking[curr_level].ply_failure)
              && (being_solved.king_square[advers(backtracking[curr_level].side_failure)]
                  !=initsquare))
            skip = true;
          /* the test being_solved.king_square[advers(side_failure)]!=initsquare is relevant!
           * E.g.
begin
author Ofer Comay
origin Sake tourney 2018, 3rd HM, cooked (and 1 author's solution doesn't deliver mate)
pieces TotalInvisible 3 white ke5 qh8 bc1 pb7c2h4 black rb4e1 ba1f1 sf2
stipulation h#2
option movenum start 38:1:15:29
end

             Ofer Comay
Sake tourney 2018, 3rd HM, cooked (and 1 authors solution doesnt deliver mate)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   Q   8
|                                   |
7   .   P   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   K   .   .   .   5
|                                   |
4   .  -R   .   .   .   .   .   P   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   P   .   .  -S   .   .   2
|                                   |
1  -B   .   B   .  -R  -B   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  6 + 5 + 3 TI

!test_mate 6:Rb4-e4 7:TI~-e4 8:Bf1-g2 9:Pb7-c8 - total_invisible.c:#543 - D:376 - 230
use option start 38:1:15:29 to replay
! >2 + 6 d4 (K:0+1 x:0+1 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#487 - D:378
! <2 - r:1 t:0 m:4294967295
! >2 + 6 c3 (K:0+1 x:0+1 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#487 - D:380
!  >3 + 6 w (K:0+1 x:0+1 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#450 - D:382
!   >4 + 6 Q (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#253 - D:384
...
!   >4 + 6 B (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#253 - D:630
!    >5 + 6 e4 (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#487 - D:632
...
!    >5 + 6 e2 (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#487 - D:650
!     >6 + 6 w (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#450 - D:652
!       7 6 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#419
!     <6 - r:1 t:0 m:4294967295
!     >6 + 6 b (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#450 - D:654
!      >7 + 6 K (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#253 - D:656
...
!      >7 + 6 B (K:0+1 x:0+1 !:0+0 ?:0+0 F:1+1) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#253 - D:662
!        8 6 capture in ply 7 will not be possible - intercept_illegal_checks.c:#73
!      <7 - r:1 t:4 m:7
!     <6 - r:1 t:4 m:7
!    <5 - r:1 t:4 m:7
!   <4 - r:1 t:4 m:7
!  <3 - r:1 t:4 m:7

HERE

!  >3 + 6 b (K:0+1 x:0+1 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#450 - D:664
!   >4 + 6 K (K:0+1 x:0+1 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 - intercept_illegal_checks.c:#253 - D:666
           */
        }
        break;

        case decision_object_placement:
        case decision_object_walk:
          break;

        default:
          if (decision_level_properties[curr_level].side
              !=backtracking[curr_level].side_failure)
          {
            /* decision concerning the other side can't contribute to being able for this side
             * to capture ...
             * ... but we still have to make sure that this side is examined too!
             */
            skip = true;
          }
          break;
      }
      /* placement:
begin
author Ken Kousaka
origin Sake tourney 2018, announcement
pieces TotalInvisible 1 white kb8 qh1 black ka1 sb1e7
stipulation h#2
option movenum start 10:3:7:1
end

             Ken Kousaka
   Sake tourney 2018, announcement

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   K   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .  -S   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1  -K  -S   .   .   .   .   .   Q   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  2 + 3 + 1 TI

!test_mate 6:Se7-f5 7:Qh1-a8 8:Sf5-d4 9:TI~-d4 - total_invisible.c:#551 - D:54 - 42
use option start 10:3:7:1 to replay
!  2 + 9 a2 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#510 - D:55
!   3 + 9 b (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#473 - D:57
!    4 9 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#442
!   3 + 9 w (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#473 - D:59
!    4 + 9 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#253 - D:61
!     5 8 capture in ply 9 will not be possible - intercept_illegal_checks.c:#73
!    4 + 9 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#253 - D:63
!     5 8 capture in ply 9 will not be possible - intercept_illegal_checks.c:#73
!    4 + 9 B (K:0+0 x:0+0 !:0+0 ?:0+0 F:1+0) - intercept_illegal_checks.c:#253 - D:65
!     5 8 capture in ply 9 will not be possible - intercept_illegal_checks.c:#73

HERE!

!  2 + 9 a3 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#510 - D:67
       */
      /* walk:
begin
author Ofer Comay
origin Sake tourney 2018, 3rd HM, cooked (and 1 author's solution doesn't deliver mate)
pieces TotalInvisible 3 white ke5 qh8 bc1 pb7c2h4 black rb4e1 ba1f1 sf2
stipulation h#2
option movenum start 17:46:23:1
end

             Ofer Comay
Sake tourney 2018, 3rd HM, cooked (and 1 author's solution doesn't deliver mate)

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   Q   8
|                                   |
7   .   P   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   K   .   .   .   5
|                                   |
4   .  -R   .   .   .   .   .   P   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   P   .   .  -S   .   .   2
|                                   |
1  -B   .   B   .  -R  -B   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  6 + 5 + 3 TI

>   1.Bf1-a6 Qh8-c8   2.Rb4-b1 TI~*a6[a6=wS][c3=bK] #

Das schwarze Ding auf der e-Linie kann verstellen!

!test_mate 6:Bf1-a6 7:Qh8-c8 8:Rb4-b1 9:TI~-a6 - total_invisible.c:#551 - D:2060 - 1990
use option start 17:46:23:1 to replay
!  2 + 6 d4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#510 - D:2061
...
!  2 + 6 c3 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#510 - D:2385
!   3 + 6 w (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#473 - D:2387
...
!   3 + 6 b (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#473 - D:2585
!    4 + 6 K (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#253 - D:2587
!     5 + 6 e4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#510 - D:2589
!      6 + 6 w (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#473 - D:2591
...
!      6 + 6 b (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+1) - intercept_illegal_checks.c:#473 - D:2859
!       7 + 6 P (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#253 - D:2861
!        8 + 9 c4 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#510 - D:2863
...
!        8 + 9 c7 (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#510 - D:2895
!         9 + 9 b (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#473 - D:2897
!          10 9 not enough available invisibles for intercepting all illegal checks - intercept_illegal_checks.c:#442
!         9 + 9 w (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#473 - D:2899
!          10 + 9 P (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+2) - intercept_illegal_checks.c:#253 - D:2901
...
!          10 + 9 B (K:0+1 x:0+0 !:0+0 ?:0+0 F:1+2) - intercept_illegal_checks.c:#253 - D:2911
!           11 8 capture in ply 9 will not be possible - intercept_illegal_checks.c:#73

HERE

!       7 + 6 S (K:0+1 x:0+0 !:0+0 ?:0+0 F:0+2) - intercept_illegal_checks.c:#253 - D:3133
       */
      break;

    default:
      break;
  }

  if (decision_level_properties[curr_level].object==decision_object_departure
    || decision_level_properties[curr_level].object==decision_object_arrival
    || decision_level_properties[curr_level].object==decision_object_walk)
  {
    assert(decision_level_properties[curr_level].side!=no_side);
    if (try_to_avoid_insertion[decision_level_properties[curr_level].side])
    {
      if (skip)
      {
#if defined(REPORT_DECISIONS)
        printf("!%*s%d ",next_decision_level,"",next_decision_level);
        printf("trying to avoid an insertion so that we can intercept the check with an insertion\n");
#endif

        skip = false;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",skip);
  TraceFunctionResultEnd();
  return skip;
}

/* Optimise backtracking considering that we have
 * reached a position where we won't able to execute the planned capture by an invisble
 * in the subsequent move because
 * - no existing invisible of the relevant side can reach the capture square
 * - no invisible of the relevant side can be inserted
 * @param side_capturing the side that is supposed to capture
 */
void backtrack_from_failed_capture_by_invisible(Side side_capturing)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_capturing);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEOL();

  backtracking[next_decision_level-1].type = backtrack_failture_to_capture_by_invisible;
  backtracking[next_decision_level-1].ply_failure = nbply;
  backtracking[next_decision_level-1].side_failure = side_capturing;
  backtracking[next_decision_level-1].id_failure = decision_level_properties[next_decision_level-1].id;

  try_to_avoid_insertion[Black] = false;
  try_to_avoid_insertion[White] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean failure_to_capture_invisible_by_pawn_continue_level(decision_level_type curr_level)
{
  boolean skip = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr_level);
  TraceFunctionParamListEnd();

  TraceValue("%u",curr_level);
  TraceValue("%u",ply_failure);
  TraceEnumerator(Side,side_failure);
  TraceValue("%u",decision_level_properties[curr_level].ply);
  TraceValue("%u",decision_level_properties[curr_level].purpose);
  TraceValue("%u",decision_level_properties[curr_level].object);
  TraceValue("%u",decision_level_properties[curr_level].id);
  TraceEnumerator(Side,decision_level_properties[curr_level].side);
  TraceValue("%u",try_to_avoid_insertion[White]);
  TraceValue("%u",try_to_avoid_insertion[Black]);
  TraceEOL();

  assert(decision_level_properties[curr_level].ply!=0);

  switch (decision_level_properties[curr_level].purpose)
  {
    case decision_purpose_random_mover_backward:
    case decision_purpose_invisible_capturer_inserted:
    case decision_purpose_invisible_capturer_existing:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_walk:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            /* depending on the walk, this piece may eventually sacrifice itself
             * to allow the capture by pawn
             */
          }
          else
            skip = true;
          break;

        case decision_object_departure:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
            skip = true;
          else
          {
            /* we may be able to sacrifice ourselves, either to the capturing pawn or
             * a pawn sacrificing itself to the capturing pawn
             * - by staying where we are (and let another piece move)
             * - by moving away to allow a pawn to sacrifice itself
             */
          }
          break;

        default:
          skip = true;
          break;
      }
      break;

    case decision_purpose_random_mover_forward:
      assert(decision_level_properties[curr_level].side!=no_side);
      switch (decision_level_properties[curr_level].object)
      {
        case decision_object_departure:
        case decision_object_arrival:
        case decision_object_walk:
          if (decision_level_properties[curr_level].ply
              <backtracking[curr_level].ply_failure)
          {
            /* we may be able to sacrifice ourselves, either to the capturing pawn or
             * a pawn sacrificing itself to the capturing pawn
             * - by staying where we are (and let another piece move)
             * - by moving away to allow a pawn to sacrifice itself
             * - by moving to the capture square
             * - by selecting a walk that allows us to eventually move the the capture square
             * - by not accidentally capturing a piece that can eventually sacrifice itself
             */
          }
          else
            skip = true;
          break;

        case decision_object_random_move:
          skip = true;
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  if (decision_level_properties[curr_level].object==decision_object_departure
    || decision_level_properties[curr_level].object==decision_object_arrival
    || decision_level_properties[curr_level].object==decision_object_walk)
  {
    assert(decision_level_properties[curr_level].side!=no_side);
    if (try_to_avoid_insertion[decision_level_properties[curr_level].side])
    {
      if (skip)
      {
#if defined(REPORT_DECISIONS)
        printf("!%*s%d ",next_decision_level,"",next_decision_level);
        printf("trying to avoid an insertion so that we can intercept the check with an insertion\n");
#endif

        skip = false;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",skip);
  TraceFunctionResultEnd();
  return skip;
}

/* Optimise backtracking considering that we have
 * reached a position where we won't able to execute the planned capture of an invisible
 * by a pawn in the subsequent move because
 * - no existing invisible of the relevant side can sacrifice itself on the capture square
 * - no invisible of the relevant side can be inserted
 * @param side_capturing the side that is supposed to capture
 */
void backtrack_from_failed_capture_of_invisible_by_pawn(Side side_capturing)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_capturing);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEOL();

  backtracking[next_decision_level-1].type = backtrack_failture_to_capture_invisible_by_pawn;
  backtracking[next_decision_level-1].ply_failure = nbply;
  backtracking[next_decision_level-1].side_failure = advers(side_capturing);
  backtracking[next_decision_level-1].id_failure = decision_level_properties[next_decision_level-1].id;

  try_to_avoid_insertion[Black] = false;
  try_to_avoid_insertion[White] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reduce max_decision_level to a value as low as possible considering that we have
 * determined that the we are done testing the current move sequence
 */
void backtrack_definitively(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  backtracking[next_decision_level-1].type = backtrack_until_level;
  backtracking[next_decision_level-1].max_level = decision_level_forever;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Optimise backtracking considering that we have
 * updated the set of potential revelations
 */
void backtrack_from_revelation_update(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  backtracking[next_decision_level-1].type = backtrack_revelation;
  backtracking[next_decision_level-1].max_level = decision_level_forever;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* To be invoked after backtrack_definitively(), possibly multiple times, to make that "definititively"
 * a bit more relative.
 * @param level level to which to backtrack at most
 */
void backtrack_no_further_than(decision_level_type level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  assert(level!=decision_level_uninitialised);

  if (level>backtracking[next_decision_level-1].max_level)
    backtracking[next_decision_level-1].max_level = level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean can_decision_level_be_continued(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",next_decision_level);
  TraceValue("%u",backtracking[next_decision_level-1].type);
  TraceValue("%u",backtracking[next_decision_level-1].max_level);
  TraceValue("%u",decision_level_properties[next_decision_level].relevance);
  TraceEOL();

  if (backtracking[next_decision_level-1].result==previous_move_has_not_solved)
    result = false;
  else if (decision_level_properties[next_decision_level].relevance==relevance_relevant)
    result = true;
  else if (decision_level_properties[next_decision_level].relevance==relevance_irrelevant)
    result = false;
  else
    switch (backtracking[next_decision_level-1].type)
    {
      case backtrack_none:
        assert(backtracking[next_decision_level-1].max_level==decision_level_latest);
        result = true;
        break;

      case backtrack_until_level:
        assert(backtracking[next_decision_level-1].max_level<decision_level_latest);
        result = next_decision_level<=backtracking[next_decision_level-1].max_level;
        break;

      case backtrack_revelation:
        assert(backtracking[next_decision_level-1].max_level<decision_level_latest);
        result = next_decision_level<=backtracking[next_decision_level-1].max_level;
        if (decision_level_properties[next_decision_level].object==decision_object_move_vector)
          result = false;
        break;

      case backtrack_failure_to_intercept_illegal_checks:
      case backtrack_failure_to_capture_uninterceptable_checker:
        assert(backtracking[next_decision_level-1].max_level==decision_level_latest);
        result = !failure_to_intercept_illegal_checks_continue_level(next_decision_level);
        break;

      case backtrack_failture_to_capture_by_invisible:
        assert(backtracking[next_decision_level-1].max_level==decision_level_latest);
        result = !failure_to_capture_by_invisible_continue_level(next_decision_level);
        break;

      case backtrack_failture_to_capture_invisible_by_pawn:
        assert(backtracking[next_decision_level-1].max_level==decision_level_latest);
        result = !failure_to_capture_invisible_by_pawn_continue_level(next_decision_level);
        break;

      default:
        assert(0);
        result = true;
        break;
    };

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
