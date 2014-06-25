#if !defined(OUTPUT_PLAINTEXT_STDIO_H)
#define OUTPUT_PLAINTEXT_STDIO_H

/* this module is intended as a stand-in for <stdio.h>
 * it #includes <stdio.h> and provides some stdio related functions */

#include <stdarg.h>
#include <stdio.h>

/* Like fprintf(), but writes right-aligned into a space of the indicated
 * width.
 * format and argument strings shouldn't contain line breaks (except maybe at
 * the end) or the output will look "interesting".
 * fprintf_r() performs two conversion runs, the first for measuring and the
 * second for writing. If the return value is <0, the caller can't determine
 * which conversion run was the cause.
 */
int fprintf_r(FILE *file, int width, char const *format, ...);

int vfprintf_r(FILE *file, int width, char const *format, va_list args);

/* like fprintf(), but writes centered into a space of the indicated width
 * format and argument strings shouldn't contain line breaks (except maybe at
 * the end) or the output will look "interesting".
 * fprintf_c() performs two conversion runs, the first for measuring and the
 * second for writing. If the return value is <0, the caller can't determine
 * which conversion run was the cause.
 */
int fprintf_c(FILE *file, int width, char const *format, ...);

int vfprintf_c(FILE *file, int width, char const *format, va_list args);

/* write a series of centered lines using fprintf_c()
 * @param lines string consisting of >=1 lines separated by \n
 * @note ignores return values from <stdio.h> functions invoked
 */
void fputs_c_multi(FILE *file, int width, char const *lines);

#endif
