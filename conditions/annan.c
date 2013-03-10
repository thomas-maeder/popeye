#include "conditions/annan.h"
#include "position/position.h"

#include <assert.h>

annan_type_type annan_type;

boolean annanises(Side side, square rear, square front)
{
  if (TSTFLAG(spec[rear],side))
    switch(annan_type)
    {
      case annan_type_A:
        return true;

      case annan_type_B:
        return rear!=king_square[side];

      case annan_type_C:
        return front!=king_square[side];

      case annan_type_D:
        return rear!=king_square[side] && front!=king_square[side];

      default:
        assert(0);
        return true;
    }
  else
    return false;
}
