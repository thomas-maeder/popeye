#if !defined(OUTPUT_PLAINTEXT_PROTOCOL_H)
#define OUTPUT_PLAINTEXT_PROTOCOL_H

#include <stdarg.h>

/* like vprintf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to standard output
 */
int protocol_vprintf(char const *format, va_list args);

#endif
