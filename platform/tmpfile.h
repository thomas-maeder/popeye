#if !defined(PLATFORM_TMPFILE_H)
#define PLATFORM_TMPFILE_H

#include <stdio.h>

/* Open and return a temporary file.
 * The file may or may not be automatically deleted at regular program
 * termination.
 * @return pointer to the FILE structure; 0 on failure
 */
FILE *platform_open_tmpfile(void);

/* Close, then remove a temporary file.
 * @param tmpfile return value of a previous platform_open_tmpfile() call
 * @return return value of fclose(tmpfile)
 */
int platform_close_tmpfile(FILE *tmpfile);

#endif
