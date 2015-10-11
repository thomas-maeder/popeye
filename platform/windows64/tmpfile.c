#include "platform/tmpfile.h"
#include "debugging/assert.h"

#include <stdlib.h>
#include <string.h>

char const * const prefix = "pyinput";

typedef struct tmpfiles_list
{
    FILE *file;
    char *name;
    struct tmpfiles_list *next;
} tmpfiles_list_type;

static tmpfiles_list_type *tmpfiles_list;

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
  char const *result_name = tempnam(0,prefix);
  if (result_name==0)
  {
    perror("error creating temporary input mirror file name");
    return 0;
  }
  else
  {
    tmpfiles_list_type * const elmt = malloc(sizeof(*elmt));

    elmt->file = fopen(result_name,"w+b");

    elmt->name = malloc(strlen(result_name)+1);
    strcpy(elmt->name,result_name);

    elmt->next = tmpfiles_list;

    tmpfiles_list = elmt;

    return elmt->file;
  }
}

static tmpfiles_list_type **find_elmt(FILE *file)
{
  tmpfiles_list_type **result = &tmpfiles_list;

  while ((*result)!=0)
    if ((*result)->file==file)
      break;
    else
      result = &(*result)->next;

  return result;
}

/* Close, then remove a temporary file.
 * @param tmpfile return value of a previous platform_open_tmpfile() call
 * @return return value of fclose(tmpfile)
 */
int platform_close_tmpfile(FILE *tmpfile)
{
  int result;

  tmpfiles_list_type ** const elmt_addr = find_elmt(tmpfile);
  tmpfiles_list_type * const elmt = *elmt_addr;
  assert(elmt!=0);

  result = fclose(tmpfile);

  *elmt_addr = elmt->next;

  remove(elmt->name);
  free(elmt->name);
  free(elmt);

  return result;
}
