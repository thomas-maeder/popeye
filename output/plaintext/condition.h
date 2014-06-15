#if !defined(OUTPUT_PLAINTEXT_CONDITION_H)
#define OUTPUT_PLAINTEXT_CONDITION_H

#include "utilities/boolean.h"

#include <stdio.h>

void WriteBGLNumber(char* a, long int b);

typedef enum
{
  condition_first,
  condition_subsequent,
  condition_end
} condition_rank;

/* Type of function writing a single condition to a file
 * @param file where to write to
 * @param CondLine textual representation to be written
 * @param is_first is this the first of a sequence (i.e. may an intro be useful)?
 */
typedef void (*condition_writer_type)(FILE *file,
                                      char const CondLine[],
                                      condition_rank rank);

/* Write conditions to a file
 * @param file where to write to
 * @param WriteCondition single condition writer
 * @note invokes WriteCondition once per active condition, and once more with
 *       rank==condition_end if at least one condition is active
 */
void WriteConditions(FILE *file, condition_writer_type WriteCondition);

#endif
