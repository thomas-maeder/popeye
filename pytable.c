#include "pytable.h"
#include "pydata.h"
#include "pymsg.h"

#include <assert.h>

enum
{
  tables_max_position = 2048
};

typedef unsigned int table_position;

static unsigned int number_of_tables;

/* current position in a specific table
 */
static table_position current_position[3*maxply];

/* elements of *all* tables
 */
static coup liste[tables_max_position];


/* Reset table module (i.e. free all tables)
 */
void reset_tables(void)
{
  number_of_tables = 0;
  current_position[0] = 0;
  liste[0].push_top = push_colour_change_stack;
}

/* Allocate a table.
 * @return identifier of allocated table.
 */
table allocate_table(void)
{
  table result = ++number_of_tables;
  current_position[result] = current_position[result-1];
  return result;
}

/* Deallocate the table that was last allocated.
 */
void free_table(void)
{
  --number_of_tables;
}

/* Append the move just played in ply nbply to a table.
 * @param t table identifier
 */
void table_append(table n)
{
  if (current_position[n]>=tables_max_position)
    ErrorMsg(TooManySol);
  else
  {
    ++current_position[n];
    current(nbply,&liste[current_position[n]]);
  }

  if (flag_outputmultiplecolourchanges)
  {
    change_rec *rec;
    change_rec ** const sp= &liste[current_position[n]].push_top;
    *sp = liste[current_position[n]-1].push_top;
    liste[current_position[n]].push_bottom = *sp;

    assert(colour_change_sp[nbply-1]<=colour_change_sp[nbply]);
    for (rec = colour_change_sp[nbply-1]; rec!=colour_change_sp[nbply]; ++rec)
      PushChangedColour(*sp,
                        push_colour_change_stack_limit,
                        rec->square,
                        rec->pc);
  }

  coupfort();
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
  coup mov;

  current(nbply,&mov);
  assert(current_position[t]>=current_position[t-1]);
  for (i = current_position[t-1]+1; i<=current_position[t]; i++)
    if (moves_equal(&mov,&liste[i]))
      return true;

  return false;
}

/* Invoke a function on each element of a table
 * @param t table to iterate over
 * @param callback address of function to invoke on each element of t
 */
void table_iterate(table t, table_callback_function_type *callback)
{
  table_position i;

  assert(current_position[t]>=current_position[t-1]);
  for (i = current_position[t]; i>current_position[t-1]; --i)
    (*callback)(&liste[i]);
}
