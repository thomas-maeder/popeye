#include "utilities/table.h"
#include "conditions/conditions.h"
#include "output/plaintext/message.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <string.h>

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
} table_elmt_type;

static table_elmt_type tables_stack[tables_stack_size];

static boolean is_effect_relevant(move_effect_journal_index_type idx)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  TraceValue("%u",move_effect_journal[idx].type);
  TraceEOL();

  switch (move_effect_journal[idx].type)
  {
    case move_effect_piece_movement:
      TraceValue("%u",move_effect_journal[idx].reason);
      TraceEOL();

      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_moving_piece_movement:
        case move_effect_reason_castling_king_movement:
        case move_effect_reason_series_capture:
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

    case move_effect_walk_change:
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

    case move_effect_piece_readdition:
      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_rebirth_choice:
          result = true;
          break;

        default:
          break;
      }
      break;

    case move_effect_piece_creation:
      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_republican_king_insertion:
          result = true;
          break;

        default:
          break;
      }
      break;

    case move_effect_piece_removal:
      switch (move_effect_journal[idx].reason)
      {
        case move_effect_reason_regular_capture:
        case move_effect_reason_ep_capture:
        case move_effect_reason_transfer_choice:
          result = CondFlag[takemake];
          break;

        case move_effect_reason_breton:
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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void make_move_snapshot(table_elmt_type *snapshot)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  snapshot->nr_relevant_effects = 0;
  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    if (is_effect_relevant(curr))
    {
      assert(snapshot->nr_relevant_effects<max_nr_relevant_effects_per_move);
      snapshot->relevant_effects[snapshot->nr_relevant_effects] = move_effect_journal[curr];
      ++snapshot->nr_relevant_effects;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean moves_equal(table_elmt_type const *snapshot)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  relevant_effects_idx_type id_relevant = 0;

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    if (is_effect_relevant(curr))
    {
      if (id_relevant==snapshot->nr_relevant_effects)
        return false;
      else if (move_effect_journal[curr].type==snapshot->relevant_effects[id_relevant].type
               && move_effect_journal[curr].reason==snapshot->relevant_effects[id_relevant].reason)
      {
        switch (move_effect_journal[curr].type)
        {
          case move_effect_piece_movement:
            if (move_effect_journal[curr].u.piece_movement.from!=snapshot->relevant_effects[id_relevant].u.piece_movement.from
                || move_effect_journal[curr].u.piece_movement.to!=snapshot->relevant_effects[id_relevant].u.piece_movement.to)
              return false;
            break;

          case move_effect_piece_exchange:
            if (move_effect_journal[curr].u.piece_exchange.from!=snapshot->relevant_effects[id_relevant].u.piece_exchange.from
                || move_effect_journal[curr].u.piece_exchange.to!=snapshot->relevant_effects[id_relevant].u.piece_exchange.to)
              return false;
            break;

          case move_effect_walk_change:
            if (move_effect_journal[curr].u.piece_walk_change.on!=snapshot->relevant_effects[id_relevant].u.piece_walk_change.on
                || move_effect_journal[curr].u.piece_walk_change.to!=snapshot->relevant_effects[id_relevant].u.piece_walk_change.to)
              return false;
            break;

          case move_effect_piece_readdition:
          case move_effect_piece_creation:
            if (move_effect_journal[curr].u.piece_addition.added.on!=snapshot->relevant_effects[id_relevant].u.piece_addition.added.on
                || move_effect_journal[curr].u.piece_addition.added.walk!=snapshot->relevant_effects[id_relevant].u.piece_addition.added.walk
                || move_effect_journal[curr].u.piece_addition.added.flags!=snapshot->relevant_effects[id_relevant].u.piece_addition.added.flags)
              return false;
            break;

          case move_effect_piece_removal:
            if (move_effect_journal[curr].u.piece_removal.on!=snapshot->relevant_effects[id_relevant].u.piece_removal.on)
              return false;
            break;

          case move_effect_flags_change:
            if (move_effect_journal[curr].u.flags_change.on!=snapshot->relevant_effects[id_relevant].u.flags_change.on
                || move_effect_journal[curr].u.flags_change.to!=snapshot->relevant_effects[id_relevant].u.flags_change.to)
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

  TraceValue("%u",id_relevant);
  TraceValue("%u",snapshot->nr_relevant_effects);
  TraceEOL();

  return id_relevant==snapshot->nr_relevant_effects;
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
 * @param expected_top_table table expected to be freed by caller
 */
void free_table(table expected_top_table)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",number_of_tables);
  TraceEOL();
  assert(number_of_tables>0);
  assert(number_of_tables==expected_top_table);
  --number_of_tables;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append the move just played in ply nbply to a table.
 * @param t identifies the table
 */
void append_to_table(table t)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",number_of_tables);
  TraceEOL();

  if (current_position[number_of_tables]>=tables_stack_size)
    output_plaintext_error_message(TooManySol);
  else
  {
    unsigned int i;
    for (i = t; i<=number_of_tables; ++i)
      ++current_position[i];
    memmove(&tables_stack[current_position[t]+1],
            &tables_stack[current_position[t]],
            (current_position[number_of_tables]-current_position[t])*sizeof tables_stack[0]);
    make_move_snapshot(&tables_stack[current_position[t]]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(current_position[t]>=current_position[t-1]);
  for (i = current_position[t-1]+1; i<=current_position[t]; i++)
    if (moves_equal(&tables_stack[i]))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
