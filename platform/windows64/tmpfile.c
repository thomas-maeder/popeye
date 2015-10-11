#include "platform/tmpfile.h"

char const * const prefix = "pyinput";

/* We can't use tmpfile because (http://msdn.microsoft.com/en-us/library/x8x7sakw.aspx):
 * "The temporary file is created in the root directory."
 * This seems to be one of the most stupid directories that one could think of.
 * In particular, there is no reason to assume that this directory is writable.
 * tempnam() with first parameter 0 returns a name in the user's temporary
 * directory.
 */

/* Open and return a temporary file.
 * The file may or may not be automatically deleted at regular program
 * termination.
 * @return pointer to the FILE structure; 0 on failure
 */
FILE *platform_open_tmpfile(void)
{
  char const *mirrorName = tempnam(0,prefix);
  if (mirrorName==0)
  {
    perror("error creating temporary input mirror file name");
    return 0;
  }
  else
    return fopen(mirrorName,"w+b");
}
