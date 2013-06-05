#include "pieces/side_change.h"
#include "pydata.h"

#include <assert.h>
#include <stdlib.h>

/* Change the side of some piece specs
 * @param spec address of piece specs where to change the side
 */
void spec_change_side(Flags *spec)
{
  *spec ^= BIT(Black)+BIT(White);
}
