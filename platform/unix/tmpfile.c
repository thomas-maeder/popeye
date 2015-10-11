#include "platform/tmpfile.h"

/* Open and return a temporary file.
 * The file may or may not be automatically deleted at regular program
 * termination.
 * @return pointer to the FILE structure; 0 on failure
 */
FILE *platform_open_tmpfile(void)
{
  return tmpfile();
}

/* Close, then remove a temporary file.
 * @param tmpfile return value of a previous platform_open_tmpfile() call
 * @return return value of fclose(tmpfile)
 */
int platform_close_tmpfile(FILE *tmpfile)
{
  return fclose(tmpfile);
}
