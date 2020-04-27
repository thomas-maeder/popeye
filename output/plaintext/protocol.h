#if !defined(OUTPUT_PLAINTEXT_PROTOCOL_H)
#define OUTPUT_PLAINTEXT_PROTOCOL_H

#include "stipulation/stipulation.h"

#include <stdarg.h>
#include <stdio.h>

/* Remember for this run's protocol (if any) to overwrite (rather than append
 * to) a previous run's protocol (if any)
 */
void protocol_overwrite(void);

/* Open a new protocol file
 * @param filename name of protocol file
 * @return the opened file (for writing some intro text)
 *         0 if it couln't be opened
 * @note the previous protocol file (if any) is closed
 */
FILE *protocol_open(char const *filename);

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

/* write a stipulation
 * If a trace file is active, output goes to the trace file as well
 */
unsigned int protocol_write_stipulation(FILE *regular, slice_index si);
unsigned int protocol_write_sstipulation(FILE *regular, slice_index si);

/* like fflush().
 * If a trace file is active, output goes to the trace file as well
 * @return the result of writing to *regular
 */
int protocol_fflush(FILE *regular);

#endif
