#if !defined(CONDITIONS_UTILITIES_COLOUR_CHANGE_H)
#define CONDITIONS_UTILITIES_COLOUR_CHANGE_H

#include "py.h"

enum
{
  side_change_stack_size = 10000
};

extern change_rec side_change_stack[side_change_stack_size];
extern change_rec const * const side_change_stack_limit;
extern change_rec *side_change_sp[maxply + 1];

/* Push a side change just executed to a side change stack
 * @param stack stack where to push the change to
 * @param limit upper limit of the stack
 * @param sq square where the side change has just taken place
 */
void push_side_change(change_rec **stack, change_rec const *limit, square sq);

/* Copy a segment of a side change stack onto another side change stack
 * @param begin start of segment
 * @param end end of segment (not copied!)
 * @param dest destination stack
 * @param limit upper limit of the destination stack
 */
void copy_side_change_stack_segment(change_rec const *begin, change_rec const *end,
                                    change_rec **dest, change_rec const *limit);

/* Change the side of the piece on a specific square
 * @param p address of piece whose side to change
 */
void piece_change_side(piece *p);

/* Change the side of some piece specs
 * @param spec address of piece specs where to change the side
 */
void spec_change_side(Flags *spec);

#endif
