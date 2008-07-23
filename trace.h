#if !defined(TRACE_H)
#define TRACE_H

/* some trace functions - activate by #defining DOTRACE */

#if defined(DOTRACE)

void TraceFunctionEntry(char const *name);

void TraceValueImpl(char const *format, int value);

#define TraceFunctionParam(format,name) \
  TraceValueImpl(" ->" #name ":" format, name)

#define TraceValue(format,name) \
  TraceValueImpl(#name ":" format, name)

void TraceText(char const *text);

void TraceCurrentMove();

void TraceFunctionExit(char const *name);

#define TraceFunctionResult(format,name) \
  TraceValueImpl(" <- " #name ":" format, name)

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
