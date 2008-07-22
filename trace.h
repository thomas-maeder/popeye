#if !defined(TRACE_H)
#define TRACE_H

/* some trace functions - activate by #defining DOTRACE */

#if defined(DOTRACE)

void TraceFunctionEntry(char const *name);
void TraceFunctionParam(char const *format, ...);
void TraceValue(char const *format, ...);
void TraceText(char const *text);
void TraceCurrentMove();
void TraceFunctionExit(char const *format, ...);
void TraceFunctionResult(char const *format, ...);

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
