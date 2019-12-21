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

decision_levels_type decision_levels[MaxPieceId+1];

typedef enum
{
  decision_object_side,
  decision_object_random_move,
  decision_object_walk,
  decision_object_move_vector,
  decision_object_departure,
  decision_object_arrival,
  decision_object_king_nomination
} decision_object_type;

typedef struct
{
    ply ply;
    decision_object_type object;
    decision_purpose_type purpose;
    PieceIdType id;
    Side side;
} decision_level_property_type;

static decision_level_property_type decision_level_properties[decision_level_dir_capacity];

unsigned long record_decision_counter;
unsigned long prev_record_decision_counter;

#if defined(REPORT_DECISIONS)

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
  printf(" - %s:#%d",basename(file),line);
  printf(" - D:%lu\n",record_decision_counter++);
  fflush(stdout);
}

#endif

void record_decision_for_inserted_capturer(PieceIdType id)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",id);
  TraceFunctionParamListEnd();

  decision_levels[id].side = curr_decision_level;
  decision_levels[id].walk = curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

void push_decision_random_move_impl(char const *file, unsigned int line, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c %u TI~-~",purpose_symbol[purpose],nbply);
  report_endline(file,line);
#endif

  decision_level_properties[curr_decision_level].ply = nbply;
  decision_level_properties[curr_decision_level].object = decision_object_random_move;
  decision_level_properties[curr_decision_level].purpose = purpose;
  ++curr_decision_level;
  assert(curr_decision_level<decision_level_dir_capacity);
}

void push_decision_departure_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteSquare(&output_plaintext_engine,
              stdout,
              pos);
  report_endline(file,line);
#endif

  decision_level_properties[curr_decision_level].ply = nbply;
  decision_level_properties[curr_decision_level].object = decision_object_departure;
  decision_level_properties[curr_decision_level].purpose = purpose;
  decision_level_properties[curr_decision_level].id = id;

  if (purpose==decision_purpose_illegal_check_interceptor)
  {
    assert(curr_decision_level>0);
    assert(decision_level_properties[curr_decision_level-1].object==decision_object_side);
    assert(decision_level_properties[curr_decision_level-1].purpose==decision_purpose_illegal_check_interceptor);
    assert(decision_level_properties[curr_decision_level-1].id==id);
    decision_level_properties[curr_decision_level].side = decision_level_properties[curr_decision_level-1].side;
  }
  else
    decision_level_properties[curr_decision_level].side = trait[nbply];

  decision_levels[id].from = curr_decision_level;

  ++curr_decision_level;
  assert(curr_decision_level<decision_level_dir_capacity);
}

void push_decision_move_vector_impl(char const *file, unsigned int line, PieceIdType id, int direction, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  printf("direction:%d",direction);
  report_endline(file,line);
#endif

  decision_level_properties[curr_decision_level].ply = nbply;
  decision_level_properties[curr_decision_level].object = decision_object_move_vector;
  decision_level_properties[curr_decision_level].purpose = purpose;
  decision_level_properties[curr_decision_level].id = id;

  if (purpose==decision_purpose_illegal_check_interceptor)
  {
    assert(curr_decision_level>0);
    assert(decision_level_properties[curr_decision_level-1].object==decision_object_side);
    assert(decision_level_properties[curr_decision_level-1].purpose==decision_purpose_illegal_check_interceptor);
    assert(decision_level_properties[curr_decision_level-1].id==id);
    decision_level_properties[curr_decision_level].side = decision_level_properties[curr_decision_level-1].side;
  }
  else
    decision_level_properties[curr_decision_level].side = trait[nbply];

  ++curr_decision_level;
  assert(curr_decision_level<decision_level_dir_capacity);
}

void push_decision_arrival_impl(char const *file, unsigned int line, PieceIdType id, square pos, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteSquare(&output_plaintext_engine,
              stdout,
              pos);
  report_endline(file,line);
#endif

  decision_level_properties[curr_decision_level].ply = nbply;
  decision_level_properties[curr_decision_level].object = decision_object_arrival;
  decision_level_properties[curr_decision_level].purpose = purpose;
  decision_level_properties[curr_decision_level].id = id;

  if (purpose==decision_purpose_random_mover_forward)
    decision_level_properties[curr_decision_level].side = trait[nbply];

  decision_levels[id].to = curr_decision_level;

  ++curr_decision_level;
  assert(curr_decision_level<decision_level_dir_capacity);
}

void push_decision_side_impl(char const *file, unsigned int line, PieceIdType id, Side side, decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteSpec(&output_plaintext_engine,
            stdout,
            BIT(side),
            initsquare,
            true);
  report_endline(file,line);
#endif

  decision_level_properties[curr_decision_level].ply = nbply;
  decision_level_properties[curr_decision_level].object = decision_object_side;
  decision_level_properties[curr_decision_level].purpose = purpose;
  decision_level_properties[curr_decision_level].id = id;
  decision_level_properties[curr_decision_level].side = side;

  decision_levels[id].side = curr_decision_level;

  ++curr_decision_level;
  assert(curr_decision_level<decision_level_dir_capacity);
}

void push_decision_walk_impl(char const *file, unsigned int line,
                             PieceIdType id,
                             piece_walk_type walk,
                             decision_purpose_type purpose)
{
#if defined(REPORT_DECISIONS)
  printf("!%*s%d ",curr_decision_level,"",curr_decision_level);
  printf("%c %u ",purpose_symbol[purpose],nbply);
  WriteWalk(&output_plaintext_engine,
            stdout,
            walk);
  report_endline(file,line);
#endif

  decision_level_properties[curr_decision_level].ply = nbply;
  decision_level_properties[curr_decision_level].object = decision_object_walk;
  decision_level_properties[curr_decision_level].purpose = purpose;
  decision_level_properties[curr_decision_level].id = id;

  if (purpose==decision_purpose_illegal_check_interceptor)
  {
    assert(curr_decision_level>0);
    assert(decision_level_properties[curr_decision_level-1].object==decision_object_side);
    assert(decision_level_properties[curr_decision_level-1].purpose==decision_purpose_illegal_check_interceptor);
    assert(decision_level_properties[curr_decision_level-1].id==id);
    decision_level_properties[curr_decision_level].side = decision_level_properties[curr_decision_level-1].side;
  }
  else
    decision_level_properties[curr_decision_level].side = trait[nbply];

  decision_levels[id].walk = curr_decision_level;

  ++curr_decision_level;
  assert(curr_decision_level<decision_level_dir_capacity);
}

void push_decision_king_nomination_impl(char const *file, unsigned int line, square pos)
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

  decision_level_properties[curr_decision_level].ply = nbply;
  decision_level_properties[curr_decision_level].object = decision_object_king_nomination;
  decision_level_properties[curr_decision_level].purpose = decision_purpose_king_nomination;

  ++curr_decision_level;
  assert(curr_decision_level<decision_level_dir_capacity);
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

void pop_decision(void)
{
  decision_level_properties[curr_decision_level].ply = 0;
  decision_level_properties[curr_decision_level].side = no_side;
  assert(curr_decision_level>0);
  --curr_decision_level;
}

/* Reduce max_decision_level to a value as low as possible considering that we have
 * reached a position where we aren't able to intercept all illegal checks by inserting
 * invisibles.
 * @param side_in_check the side that is in too many illegal checks
 */
void backtrack_from_failure_to_intercept_illegal_checks(Side side_in_check)
{
  unsigned int nr_compound_decisions = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEOL();

  max_decision_level = curr_decision_level-1;

  while (max_decision_level>1)
  {
    boolean skip = false;

    TraceValue("%u",max_decision_level);
    TraceValue("%u",decision_level_properties[max_decision_level].ply);
    TraceValue("%u",decision_level_properties[max_decision_level].purpose);
    TraceValue("%u",decision_level_properties[max_decision_level].object);
    TraceValue("%u",decision_level_properties[max_decision_level].id);
    TraceEnumerator(Side,decision_level_properties[max_decision_level].side);
    TraceEOL();

    assert(decision_level_properties[max_decision_level].ply!=0);

    switch (decision_level_properties[max_decision_level].purpose)
    {
      case decision_purpose_random_mover_backward:
        if (decision_level_properties[max_decision_level].ply<nbply)
          skip = true;
        break;

      case decision_purpose_random_mover_forward:
        // TODO should we distinguish betwen forawd moves by existing vs. inserted pieces?
        if (decision_level_properties[max_decision_level].ply>=nbply)
          skip = true;
        break;

      case decision_purpose_invisible_capturer_inserted:
        if (decision_level_properties[max_decision_level].object==decision_object_walk)
        {
          if (nr_compound_decisions==0)
            skip = true;
          ++nr_compound_decisions;
        }
        else if (decision_level_properties[max_decision_level].object==decision_object_move_vector)
          ;
        else
          skip = true;
        break;

      case decision_purpose_invisible_capturer_existing:
        assert(decision_level_properties[max_decision_level].object!=decision_object_move_vector);
        if (decision_level_properties[max_decision_level].object==decision_object_walk)
        {
          /* a dummy was fleshed out */
        }
        else
          skip = true;
        break;

      case decision_purpose_king_nomination:
        ++nr_compound_decisions;
        break;

      default:
        break;
    }

    if (skip)
      --max_decision_level;
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void backtrack_from_failed_capture_by_invisible(Side side_capturing)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_capturing);
  TraceFunctionParamListEnd();

  max_decision_level = curr_decision_level-1;

  while (max_decision_level>1)
  {
    boolean skip = false;

    TraceValue("%u",max_decision_level);
    TraceValue("%u",decision_level_properties[max_decision_level].ply);
    TraceValue("%u",decision_level_properties[max_decision_level].purpose);
    TraceValue("%u",decision_level_properties[max_decision_level].object);
    TraceValue("%u",decision_level_properties[max_decision_level].id);
    TraceEnumerator(Side,decision_level_properties[max_decision_level].side);
    TraceEOL();

    assert(decision_level_properties[max_decision_level].ply!=0);

    switch (decision_level_properties[max_decision_level].purpose)
    {
      case decision_purpose_random_mover_backward:
        skip = true;
        break;

      case decision_purpose_invisible_capturer_inserted:
        assert(decision_level_properties[max_decision_level].side!=no_side);
        if (decision_level_properties[max_decision_level].side!=side_capturing
            || decision_level_properties[max_decision_level].object==decision_object_move_vector
            || decision_level_properties[max_decision_level].object==decision_object_departure)
          skip = true;
        break;

      case decision_purpose_invisible_capturer_existing:
        assert(decision_level_properties[max_decision_level].side!=no_side);
        assert(decision_level_properties[max_decision_level].object!=decision_object_move_vector);
        if (decision_level_properties[max_decision_level].side!=side_capturing
            || decision_level_properties[max_decision_level].object==decision_object_departure)
          skip = true;
        break;

      default:
        break;
    }

    if (skip)
      --max_decision_level;
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void backtrack_from_failed_capture_of_invisible_by_pawn(Side side_capturing)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_capturing);
  TraceFunctionParamListEnd();

  max_decision_level = curr_decision_level-1;

  while (max_decision_level>1)
  {
    boolean skip = false;

    TraceValue("%u",max_decision_level);
    TraceValue("%u",decision_level_properties[max_decision_level].ply);
    TraceValue("%u",decision_level_properties[max_decision_level].purpose);
    TraceValue("%u",decision_level_properties[max_decision_level].object);
    TraceValue("%u",decision_level_properties[max_decision_level].id);
    TraceEnumerator(Side,decision_level_properties[max_decision_level].side);
    TraceEOL();

    assert(decision_level_properties[max_decision_level].ply!=0);

    switch (decision_level_properties[max_decision_level].purpose)
    {
      case decision_purpose_random_mover_backward:
        skip = true;
        break;

      case decision_purpose_invisible_capturer_inserted:
        assert(decision_level_properties[max_decision_level].side!=no_side);
        skip = true;
        break;

      case decision_purpose_invisible_capturer_existing:
        assert(decision_level_properties[max_decision_level].side!=no_side);
        skip = true;
        break;

      case decision_purpose_random_mover_forward:
        if (decision_level_properties[max_decision_level].object==decision_object_departure)
        {
          assert(decision_level_properties[max_decision_level].side!=no_side);
          if (decision_level_properties[max_decision_level].side==side_capturing)
            skip = true;
        }
        else if (decision_level_properties[max_decision_level].object==decision_object_arrival)
        {
          // TODO is this correct? what if we evacuate so that a pawn can sacrifice itself to the pawn?
          assert(decision_level_properties[max_decision_level].side!=no_side);
          if (decision_level_properties[max_decision_level].side==side_capturing)
            skip = true;
        }
        else if (decision_level_properties[max_decision_level].object==decision_object_random_move
                 || decision_level_properties[max_decision_level].object==decision_object_walk)
          skip = true;
        break;

      default:
        break;
    }

    if (skip)
      --max_decision_level;
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean can_decision_level_be_continued(void)
{
  return curr_decision_level<=max_decision_level;
}
