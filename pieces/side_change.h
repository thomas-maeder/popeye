#if !defined(CONDITIONS_UTILITIES_SIDE_CHANGE_H)
#define CONDITIONS_UTILITIES_SIDE_CHANGE_H

#include "py.h"

/* Change the side of the piece on a specific square
 * @param p address of piece whose side to change
 */
void piece_change_side(piece *p);

/* Change the side of some piece specs
 * @param spec address of piece specs where to change the side
 */
void spec_change_side(Flags *spec);

#endif
