#if !defined(OUTPUT_PLAINTEXT_PROTOCOL_H)
#define OUTPUT_PLAINTEXT_PROTOCOL_H

#include "utilities/boolean.h"

#include <stdarg.h>
#include <stdio.h>

/* Open a new protocol file
 * @param filename name of protocol file
 * @param open_mode open mode for the file to be opened
 * @param format printf() format string for the intro to be written to the file
 * @param ... values to be converted into the file according to format
 * @return true iff the new file could be successfully opened
 * @note the previous protocol file (if any) is closed
 */
boolean protocol_open(char const *filename, char const *open_mode,
                      char const *format, ...);

/* like putchar().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fputc(int c, FILE *regular);

/* like vprintf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_vfprintf(FILE *regular, char const *format, va_list args);

/* like printf().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fprintf(FILE *regular, char const *format, ...);

/* like fprintf_r().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fprintf_r(FILE *regular, int width, char const *format, ...);

/* like fprintf_c().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fprintf_c(FILE *regular, int width, char const *format, ...);

/* like fputs_c_multi().
 * If a trace file is active, output goes to the trace file as well
 */
void protocol_fputs_c_multi(FILE *regular, int width, char const *lines);

/* like fflush().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fflush(FILE *regular);

#endif
