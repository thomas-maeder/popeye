#if !defined(PYIO_H)
#define PYIO_H

#include "py.h"

/* I/O related declarations
 */

/* Contains the stipulation slice that was active when the move at a
 * specific ply was played.
 */
extern slice_index active_slice[maxply];

#endif
