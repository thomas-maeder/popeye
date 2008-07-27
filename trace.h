#if !defined(TRACE_H)
#define TRACE_H

/* some trace functions
 * - write useful trace output to standard out
 * - activate by #defining DOTRACE
 * - no overhead if not active
 */

#if defined(DOTRACE)

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
  TraceValueImpl(" ->" #name ":" format, name)

/* Trace the value of some expression
 */
#define TraceValue(format,name) \
  TraceValueImpl(" " #name ":" format, name)

/* Trace arbitrary text
 */
void TraceText(char const *text);

/* Trace the current move in the top-level generation
 * Only tested right before calling jouecoup; writes the correct
 * departure and arrival square and sometimes the correct piece etc,
 * but still very useful.
 */
void TraceCurrentMove();

/* Trace a function result.
 * Works best in SESE style functions.
 */
#define TraceFunctionResult(format,name) \
  TraceValueImpl(" <- " #name ":" format, name)

void TraceValueImpl(char const *format, int value);

#else

#define TraceFunctionEntry(name)
#define TraceFunctionParam(format,name)
#define TraceValue(format,name)
#define TraceText(text)
#define TraceCurrentMove()
#define TraceFunctionExit(name)
#define TraceFunctionResult(format,name);

#endif

#endif
