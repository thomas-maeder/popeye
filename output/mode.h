#if !defined(OUTPUT_MODE_H)
#define OUTPUT_MODE_H

/* Mode of output:
 * - like a tree (typically used for e.g. direct stipulations)
 * - 1 line per solution (typically used for e.g. help and series stipulations)
 */

#define ENUMERATION_TYPENAME output_mode
#define ENUMERATORS \
  ENUMERATOR(output_mode_tree), \
    ENUMERATOR(output_mode_line), \
    ENUMERATOR(output_mode_none)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

#endif
