#if !defined(PYTABLE_H)
#define PYTABLE_H

#include "boolean.h"
#include "py.h"

/* This module implements a stack of tables of moves.
 * "Stack" because a table can only be allocated and deallocated from
 * the end of the available tables.
 * Every allocated table can be manipuilated, however.
 * Tables are add-only, i.e. moves can be added but not removed
 */

/* Alias name for the type representing table identifiers
 */
typedef unsigned int table;

/* Reset table module (i.e. free all tables)
 */
void reset_tables(void);

/* Allocate a table.
 * @return identifier of allocated table.
 */
table allocate_table(void);

/* Deallocate the table that was last allocated.
 */
void free_table(void);

/* Retrieve the length (number of elements) of a table.
 * @param t table identifier (obtained using allocate_table())
 * @return number of elements of the table
 */
unsigned int table_length(table t);

/* Append the move just played in ply nbply to the top table.
 */
void append_to_top_table(void);

/* Remove all elements from the top table
 */
void clear_top_table(void);

/* Determine whether the move just played in ply nbply is in a table
 * @param t table identifier
 * @return true iff the move just played is in table t
 */
boolean is_current_move_in_table(table t);


/* Alias name for table_iterate() callback functions
 */
typedef void table_callback_function_type(coup *);

/* Invoke a function on each element of a table
 * @param t table to iterate over
 * @param callback address of function to invoke on each element of t
 */
void table_iterate(table t, table_callback_function_type *callback);

#endif
