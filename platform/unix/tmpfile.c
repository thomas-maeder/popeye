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
