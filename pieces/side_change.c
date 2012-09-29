#include "pieces/side_change.h"
#include "pydata.h"

#include <assert.h>

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
