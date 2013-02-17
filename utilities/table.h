#if !defined(UTILITIES_TABLE_H)
#define UTILITIES_TABLE_H

#include "utilities/boolean.h"

/* This module implements a stack of tables of moves.  "Stack" because
 * a table can only be allocated and deallocated from the end of the
 * available tables.
 * Every allocated table can be queried and appended to, however.
 * Tables are add-only, i.e. moves can be added but not removed
 */

/* Alias name for the type representing table identifiers
 */
typedef unsigned int table;

enum
{
  table_nil = 0
};

/* Reset table module (i.e. free all tables)
 */
void reset_tables(void);

/* Allocate a table.
 * @return identifier of allocated table.
 */
table allocate_table(void);

/* Deallocate the table that was last allocated.
 * @param expected_top_table table expected to be freed by caller
 */
void free_table(table expected_top_table);

/* Retrieve the length (number of elements) of a table.
 * @param t table identifier (obtained using allocate_table())
 * @return number of elements of the table
 */
unsigned int table_length(table t);

/* Append the move just played in ply nbply to a table.
 * @param t identifies table where to append to
 */
void append_to_table(table t);

/* Determine whether the move just played in ply nbply is in a table
 * @param t table identifier
 * @return true iff the move just played is in table t
 */
boolean is_current_move_in_table(table t);

#endif
