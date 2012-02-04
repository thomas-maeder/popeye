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
#include <stdio.h>

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

/* Trace end of function return value (if any)
 */
void TraceFunctionResultEnd(void);

/* Trace an enumerator value in both string and numerical form
 * The enumeration type defining type_name must have been generated
 * using pyenum.h
 */
#define TraceEnumerator(type_name,name,suffix) \
  TraceEnumeratorImpl(" " #name ":%s(%u)" suffix, \
                      type_name##_names[name], \
                      name)

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
void TraceValueImpl(char const *format, size_t value);
void TraceFunctionResultImpl(char const *format, size_t value);
void TracePointerValueImpl(char const *format, void const *value);
void TraceEnumeratorImpl(char const *format,
                         char const *enumerator_name,
                         unsigned int value);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_tracer_can_attack(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_tracer_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type move_tracer_defend(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type move_tracer_can_defend(slice_index si, stip_length_type n);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type move_tracer_help(slice_index si, stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type move_tracer_can_help(slice_index si, stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_move_tracers(slice_index si);

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
#define TracePosition(echiquier,flags)
#define TraceFunctionExit(name)
#define TraceFunctionResult(format,name)
#define TracePointerFunctionResult(format,name)
#define TraceFunctionResultEnd()
#define TraceCurrentHashBuffer()
#define TraceEnumerator(type_name,value,suffix)
#define TraceStipulation(start_slice)
#define TraceCallStack(file)

#endif

#endif
