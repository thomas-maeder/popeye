#include "utilities/table.h"
#include "pydata.h"
#include "pymsg.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/anticirce/rebirth_handler.h"
#include "conditions/football.h"
#include "conditions/republican.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "solving/moving_pawn_promotion.h"
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

typedef struct
{
  square sq_departure;
  square sq_arrival;
  square sq_capture;
  square sq_rebirth;
  piece football_substitution;
  PieNam promotion_of_moving;
  square sq_rebirth_anti;
  PieNam promotion_of_reborn;
  boolean promotion_of_moving_to_chameleon;
  boolean promotion_of_reborn_to_chameleon;
  square king_placement;
  square hurdle;
  square sb2where;
  piece sb2what;
  square sb3where;
  piece sb3what;
} table_elmt_type;

static table_elmt_type tables_stack[tables_stack_size];

static void make_move_snapshot(table_elmt_type *mov)
{
  numecoup const coup_id = current_move[nbply];

  mov->sq_departure = move_generation_stack[coup_id].departure;
  mov->sq_arrival = move_generation_stack[coup_id].arrival;
  mov->sq_capture = move_generation_stack[coup_id].capture;
  /* at most one of the two current_promotion_of_*moving[nbply] is different from vide! */
  mov->promotion_of_moving = current_promotion_of_moving[nbply]+current_promotion_of_reborn_moving[nbply]-Empty;
  mov->football_substitution = current_football_substitution[nbply];
  mov->sq_rebirth = current_circe_rebirth_square[nbply];
  mov->promotion_of_reborn = current_promotion_of_capturee[nbply];

  mov->sq_rebirth_anti = current_anticirce_rebirth_square[nbply];

  /* hope the following works with parrain too */
  mov->promotion_of_moving_to_chameleon = promotion_of_moving_into_chameleon[nbply];
  mov->promotion_of_reborn_to_chameleon = promotion_of_circe_reborn_into_chameleon[nbply];
  mov->king_placement = republican_king_placement[nbply];
  mov->hurdle = chop[coup_id];
  mov->sb3where = singlebox_type3_promotions[coup_id].where;
  mov->sb3what = singlebox_type3_promotions[coup_id].what;
  mov->sb2where = singlebox_type2_latent_pawn_promotions[nbply].where;
  mov->sb2what = singlebox_type2_latent_pawn_promotions[nbply].what;
}

static boolean moves_equal(table_elmt_type const *move1, table_elmt_type const *move2)
{
  return (move1->sq_departure==move2->sq_departure
          && move1->sq_arrival==move2->sq_arrival
          && move1->promotion_of_moving==move2->promotion_of_moving
          && move1->football_substitution==move2->football_substitution
          && move1->promotion_of_reborn==move2->promotion_of_reborn
          && move1->promotion_of_reborn_to_chameleon==move2->promotion_of_reborn_to_chameleon
          && move1->promotion_of_moving_to_chameleon==move2->promotion_of_moving_to_chameleon
          && move1->sb3where==move2->sb3where
          && move1->sb3what==move2->sb3what
          && move1->sb2where==move2->sb2where
          && move1->sb2what==move2->sb2what
          && move1->hurdle==move2->hurdle
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
