#if !defined(OUTPUT_PLAINTEXT_PLAINTEXT_H)
#define OUTPUT_PLAINTEXT_PLAINTEXT_H

#include "py.h"

/* Write the current move of a ply
 * @param ply_id identifies the ply of which to write the current move
 */
void output_plaintext_write_move(ply ply_id);

#endif
