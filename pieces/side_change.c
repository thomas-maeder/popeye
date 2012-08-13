#include "pieces/side_change.h"
#include "pydata.h"

#include <assert.h>

change_rec side_change_stack[side_change_stack_size];
change_rec *side_change_sp[maxply + 1];
change_rec const * const side_change_stack_limit = &side_change_stack[side_change_stack_size];

/* Push a side change just executed to a side change stack
 * @param stack stack where to push the change to
 * @param limit upper limit of the stack
 * @param sq square where the side change has just taken place
 */
void push_side_change(change_rec **stack, change_rec const *limit, square sq)
{
  assert(*stack<limit);
  (*stack)->square = sq;
  (*stack)->pc = e[sq];
  ++*stack;
}

/* Copy a segment of a side change stack onto another side change stack
 * @param begin start of segment
 * @param end end of segment (not copied!)
 * @param dest destination stack
 * @param limit upper limit of the destination stack
 */
void copy_side_change_stack_segment(change_rec const *begin, change_rec const *end,
                                    change_rec **dest, change_rec const *limit)
{
  change_rec const *rec;
  for (rec = begin; rec!=end; ++rec)
  {
    assert(*dest<limit);
    **dest = *rec;
    ++*dest;
  }
}

/* Change the side of the piece on a specific square
 * @param p address of piece whose side to change
 */
void piece_change_side(piece *p)
{
  --nbpiece[*p];
  *p = -*p;
  ++nbpiece[*p];
}

/* Change the side of some piece specs
 * @param spec address of piece specs where to change the side
 */
void spec_change_side(Flags *spec)
{
  *spec ^= BIT(Black)+BIT(White);
}
