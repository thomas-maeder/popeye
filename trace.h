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


#if defined(DOTRACE)

#include "py.h"

/* Deactivate trace output until program termination.
 * Useful while debugging to suppress trace output from the command line.
 */
void TraceDeactivate();

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
#define TraceFunctionParam(format,name)         \
  TraceValueImpl(" ->" #name ":" format, (int)name)

/* Trace the value of some expression
 */
#define TraceValue(format,name) \
  TraceValueImpl(" " #name ":" format, (int)name)

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
boolean TraceCurrentMove(void);

/* Trace the notation of the current position
 */
void TracePosition(echiquier e, Flags flags[maxsquare+4]);

/* Trace a function result.
 * Works best in SESE style functions.
 */
#define TraceFunctionResult(format,name) \
  TraceValueImpl(" <- " #name ":" format, (int)name)

void TraceValueImpl(char const *format, int value);

#else

#define TraceDeactivate()
#define TraceFunctionEntry(name)
#define TraceFunctionParam(format,name)
#define TraceValue(format,name)
#define TraceText(text)
#define TraceSquare(name)
#define TracePiece(name)
#define TraceCurrentMove() true
#define TracePosition(echiquier,flags)
#define TraceFunctionExit(name)
#define TraceFunctionResult(format,name)

#endif

#endif
