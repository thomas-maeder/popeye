#if !defined(OUTPUT_PLAINTEXT_PROTOCOL_H)
#define OUTPUT_PLAINTEXT_PROTOCOL_H

#include "utilities/boolean.h"

#include <stdarg.h>

/* Open a new protocol file
 * @param filename name of protocol file
 * @return true iff the new file could be successfully opened
 * @note the previous protocol file (if any) is closed
 */
boolean protocol_open(char const *filename);

/* like putchar().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to standard output
 */
int protocol_putchar(int c);

/* like vprintf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to standard output
 */
int protocol_vprintf(char const *format, va_list args);

/* like printf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to standard output
 */
int protocol_printf(char const *format, ...);

int protocol_printf_r(int width, char const *format, ...);

int protocol_printf_c(int width, char const *format, ...);

void protocol_puts_c_multi(int width, char const *lines);

/* like fflush().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to standard output
 */
int protocol_flush(void);

#endif
