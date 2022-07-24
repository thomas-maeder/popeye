#if !defined(DEBUGGING_TRACE_H)
#define DEBUGGING_TRACE_H

/* some trace functions
 * - write useful trace output to standard out
 * - activate by #defining DOTRACE
 * - no overhead if not active
 */

#if defined(_MSC_VER)
/* MSVC compilers before VC7 don't have __func__ at all; later ones
 * call it __FUNCTION__. */
#  if _MSC_VER < 1300
#    define __func__ "???"
#  else
#    define __func__ __FUNCTION__
#  endif
#endif


typedef unsigned long trace_level;


#if defined(DOTRACE)

#include <stddef.h>
#include <stdio.h>

#include "pieces/pieces.h"
#include "position/position.h"
#include "solving/ply.h"
#include "stipulation/stipulation.h"

/* Set the maximal level of trace messages to be produced.
* @param max_level maximal level of trace messages to be produced;
*                  pass 0 to suppress all trace messages
*/
void TraceSetMaxLevel(trace_level max_level);

void TraceSuppressPointerValues(void);

/* End the current line */
void TraceEOL(void);

/* Trace function entry
 * e.g. > #17 func
 * (where 17 is the current trace recursion level)
 * NOTE: recursion level tracing only provides useful output if every
 * function that has TraceFunctionEntry() also has TraceFunctionExit()
 * (on every exit!)
 */
void TraceFunctionEntry(char const *name);

/* Trace function exit
 * e.g. < #17 func
 */
void TraceFunctionExit(char const *name);

/* Trace a function parameter
 */
#define TraceFunctionParam(format,name) \
  TraceValueImpl(" ->" #name ":", (format), (name))

/* Trace end of function parameter list
 */
void TraceFunctionParamListEnd(void);

/* Trace the value of some expression
 */
#define TraceValue(format,name) \
  TraceValueImpl(" " #name ":", (format), (name))

/* Trace arbitrary text
 */
void TraceText(char const *text);

void TraceSquareImpl(char const *prefix, square s);

/* Trace a square name
 */
#define TraceSquare(name) \
  TraceSquareImpl(" " #name ":", (name))

void TraceWalkImpl(char const *prefix, piece_walk_type p);

/* Trace a piece
 */
#define TraceWalk(name) \
  TraceWalkImpl(" " #name ":", (name))

/* Trace the notation of the current position
 */
void TracePosition(echiquier e, Flags flags[maxsquare+4]);

/* Trace the content of the hashbuffer of ply nbply
 */
void TraceCurrentHashBuffer(void);

/* Trace a function result.
 * Works best in SESE style functions.
 */
#define TraceFunctionResult(format,name) \
  TraceFunctionResultImpl(" <- " #name ":", (format), (name))

/* Trace end of function return value (if any)
 */
void TraceFunctionResultEnd(void);

/* Trace an enumerator value in both string and numerical form
 * The enumeration type defining type_name must have been generated
 * using utilities/enumeration.h
 */
#define TraceEnumerator(type_name,name) \
  TraceEnumeratorImpl(" " #name ":%s(%u)", \
                      type_name##_names[(name)], \
                      (unsigned int) (name))

/* Trace the current stipulation structure
 * @param start_slice identifies slice where to start tracing
 */
void TraceStipulation(slice_index start_slice);

/* Write the call stack
 * @param file where to write the call stack
 */
void TraceCallStack(FILE *file);

/* Helper functions
 */
void TraceValueImpl(char const *prefix, char const *format, ...);
void TraceFunctionResultImpl(char const *prefix, char const *format, ...);
void TraceEnumeratorImpl(char const *format,
                         char const *enumerator_name,
                         unsigned int value);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void move_tracer_solve(slice_index si);

/* Instrument slices with move tracers
 */
void solving_insert_move_tracers(slice_index si);

#else

#define TraceDeactivate()
#define TraceSuppressPointerValues()
#define TraceEOL()
#define TraceFunctionEntry(name)
#define TraceFunctionParam(format,name)
#define TraceFunctionParamListEnd()
#define TraceValue(format,name)
#define TraceText(text)
#define TraceSquare(name)
#define TraceWalk(name)
#define TracePosition(echiquier,flags)
#define TraceFunctionExit(name)
#define TraceFunctionResult(format,name)
#define TraceFunctionResultEnd()
#define TraceCurrentHashBuffer()
#define TraceEnumerator(type_name,value)
#define TraceStipulation(start_slice)
#define TraceCallStack(file)

#endif

#endif
