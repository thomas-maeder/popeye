#include "utilities/table.h"
#include "pydata.h"
#include "pymsg.h"
#include "conditions/republican.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

enum
{
  tables_stack_size = 2048
};

typedef unsigned int table_position;

static unsigned int number_of_tables;

/* current position in a specific table
 */
static table_position current_position[3*maxply];

typedef unsigned int relevant_effects_idx_type;

enum
{
  max_nr_relevant_effects_per_move = 10
};

typedef struct
{
    relevant_effects_idx_type nr_relevant_effects;
    move_effect_journal_entry_type relevant_effects[max_nr_relevant_effects_per_move];
  square sq_capture;
  square sq_rebirth;
  piece football_substitution;
  square sq_rebirth_anti;
  square king_placement;
  square hurdle;
} table_elmt_type;

static table_elmt_type tables_stack[tables_stack_size];

static boolean is_effect_relevant(move_effect_journal_index_type idx)
{
  boolean result = false;

  switch (move_effect_journal[idx].type)
  {
    case move_effect_piece_movement:
      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_moving_piece_movement:
        case move_effect_reason_castling_king_movement:
          result = true;
          break;

        default:
          break;
      }
      break;

    case move_effect_piece_exchange:
      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_messigny_exchange:
          result = true;
          break;

        default:
          break;
      }
      break;

    case move_effect_piece_change:
      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_football_chess_substitution:
        case move_effect_reason_pawn_promotion:
        case move_effect_reason_promotion_of_reborn:
        case move_effect_reason_singlebox_promotion:
          result = true;
          break;

        default:
          break;
      }
      break;

    case move_effect_flags_change:
      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_pawn_promotion:
          result = true;
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return result;
}

static void make_move_snapshot(table_elmt_type *mov)
{
  numecoup const coup_id = current_move[nbply];

  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;
  mov->nr_relevant_effects = 0;
  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
    if (is_effect_relevant(curr))
    {
      assert(mov->nr_relevant_effects<max_nr_relevant_effects_per_move);
      mov->relevant_effects[mov->nr_relevant_effects] = move_effect_journal[curr];
      ++mov->nr_relevant_effects;
    }

  mov->sq_capture = move_generation_stack[coup_id].capture;

  mov->sq_rebirth = current_circe_rebirth_square[nbply];
  mov->sq_rebirth_anti = current_anticirce_rebirth_square[nbply];

  mov->king_placement = republican_king_placement[nbply];
  mov->hurdle = chop[coup_id];
}

static boolean moves_equal(table_elmt_type const *move1, table_elmt_type const *move2)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;

  relevant_effects_idx_type id_relevant = 0;

  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
    if (is_effect_relevant(curr))
    {
      if (id_relevant==move2->nr_relevant_effects)
        return false;
      else if (move_effect_journal[curr].type==move2->relevant_effects[id_relevant].type
               && move_effect_journal[curr].reason==move2->relevant_effects[id_relevant].reason)
      {
        switch (move_effect_journal[curr].type)
        {
          case move_effect_piece_movement:
            if (move_effect_journal[curr].u.piece_movement.from!=move2->relevant_effects[id_relevant].u.piece_movement.from
                || move_effect_journal[curr].u.piece_movement.to!=move2->relevant_effects[id_relevant].u.piece_movement.to)
              return false;
            break;

          case move_effect_piece_exchange:
            if (move_effect_journal[curr].u.piece_exchange.from!=move2->relevant_effects[id_relevant].u.piece_exchange.from
                || move_effect_journal[curr].u.piece_exchange.to!=move2->relevant_effects[id_relevant].u.piece_exchange.to)
              return false;
            break;

          case move_effect_piece_change:
            if (move_effect_journal[curr].u.piece_change.on!=move2->relevant_effects[id_relevant].u.piece_change.on
                || move_effect_journal[curr].u.piece_change.to!=move2->relevant_effects[id_relevant].u.piece_change.to)
              return false;
            break;

          case move_effect_flags_change:
            if (move_effect_journal[curr].u.flags_change.on!=move2->relevant_effects[id_relevant].u.flags_change.on
                || move_effect_journal[curr].u.flags_change.to!=move2->relevant_effects[id_relevant].u.flags_change.to)
              return false;
            break;

          default:
            assert(0);
            break;
        }

        ++id_relevant;
      }
      else
        return false;
    }

  if (id_relevant<move2->nr_relevant_effects)
    return false;

  return (move1->hurdle==move2->hurdle
          && (!CondFlag[takemake] || move1->sq_capture==move2->sq_capture)
          && (!supergenre
              || ((!(CondFlag[supercirce]
                     || CondFlag[april]
                     || CondFlag[circecage])
                   || move1->sq_rebirth==move2->sq_rebirth)
                  && (!CondFlag[republican] || move1->king_placement==move2->king_placement)
                  && (!CondFlag[antisuper] || move1->sq_rebirth_anti==move2->sq_rebirth_anti))
              )
          );
}

/* Reset table module (i.e. free all tables)
 */
void reset_tables(void)
{
  number_of_tables = 0;
  current_position[0] = 0;
}

/* Allocate a table.
 * @return identifier of allocated table.
 */
table allocate_table(void)
{
  table const result = ++number_of_tables;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  current_position[result] = current_position[result-1];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Deallocate the table that was last allocated.
 */
void free_table(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",number_of_tables);
  assert(number_of_tables>0);
  --number_of_tables;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append the move just played in ply nbply to the top table.
 */
void append_to_top_table(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",number_of_tables);

  if (current_position[number_of_tables]>=tables_stack_size)
    ErrorMsg(TooManySol);
  else
  {
    ++current_position[number_of_tables];
    make_move_snapshot(&tables_stack[current_position[number_of_tables]]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Retrieve the identifier of the top table
 * @return identifier of the top table
 */
table get_top_table(void)
{
  return number_of_tables;
}

/* Remove all elements from the top table
 */
void clear_top_table(void)
{
  assert(number_of_tables>0);
  current_position[number_of_tables] = current_position[number_of_tables-1];
}

/* Retrieve the length (number of elements) of a table.
 * @param t table identifier (obtained using allocate_table())
 * @return number of elements of the table
 */
unsigned int table_length(table t)
{
  assert(current_position[t]>=current_position[t-1]);
  return current_position[t]-current_position[t-1];
}

/* Determine whether the move just played in ply nbply is in a table
 * @param t table identifier
 * @return true iff the move just played is in table t
 */
boolean is_current_move_in_table(table t)
{
  table_position i;
  table_elmt_type mov;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  make_move_snapshot(&mov);
  assert(current_position[t]>=current_position[t-1]);
  for (i = current_position[t-1]+1; i<=current_position[t]; i++)
    if (moves_equal(&mov,&tables_stack[i]))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
