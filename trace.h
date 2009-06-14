#if !defined(TRACE_H)
#define TRACE_H

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

#include "py.h"

/* Set the maximal level of trace messages to be produced.
* @param max_level maximal level of trace messages to be produced;
*                  pass 0 to suppress all trace messages
*/
void TraceSetMaxLevel(trace_level max_level);

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
  TraceValueImpl(" ->" #name ":" format, (size_t)name)

/* Trace end of function parameter list
 */
void TraceFunctionParamListEnd(void);

/* Trace the value of some expression
 */
#define TraceValue(format,name) \
  TraceValueImpl(" " #name ":" format, (size_t)name)

/* Trace the value of some expression
 */
#define TracePointerValue(format,name) \
  TracePointerValueImpl(" " #name ":" format, (void *)name)

/* Trace arbitrary text
 */
void TraceText(char const *text);

void TraceSquareImpl(char const *prefix, square s);

/* Trace a square name
 */
#define TraceSquare(name) \
  TraceSquareImpl(" " #name ":", name)

void TracePieceImpl(char const *prefix, piece p);

/* Trace a piece
 */
#define TracePiece(name) \
  TracePieceImpl(" " #name ":", name)

/* Trace the current move in the top-level generation; use right after
 * jouecoup() returned true.
 * @return true (useful for if (jouecoup() && TraceCurrentMove()) ...
 */
boolean TraceCurrentMove(ply curent_ply);

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
  TraceFunctionResultImpl(" <- " #name ":" format, (size_t)name)

/* Trace a function result of pointer type
 * Works best in SESE style functions.
 */
#define TracePointerFunctionResult(format,name) \
  TracePointerValueImpl(" <- " #name ":" format, (void*)name)

void TraceValueImpl(char const *format, size_t value);
void TraceFunctionResultImpl(char const *format, size_t value);
void TracePointerValueImpl(char const *format, void const *value);

#else

#define TraceDeactivate()
#define TraceFunctionEntry(name)
#define TraceFunctionParam(format,name)
#define TraceFunctionParamListEnd()
#define TraceValue(format,name)
#define TracePointerValue(format,name)
#define TraceText(text)
#define TraceSquare(name)
#define TracePiece(name)
#define TraceCurrentMove(curent_ply) true
#define TracePosition(echiquier,flags)
#define TraceFunctionExit(name)
#define TraceFunctionResult(format,name)
#define TracePointerFunctionResult(format,name)
#define TraceCurrentHashBuffer()

#endif

#endif
