#include "pyboard.h"

SquareFlags zzzan[square_h8 - square_a1 + 1];

int zzzao[square_h8 - square_a1 + 1];

/* Calculate a square transformation
 * @param sq square to be reflected
 * @param transformation transformation to be performed
 * @return transformed value of sq
 */
square transformSquare(square sq, SquareTransformation transformation)
{
  square ret = sq;

  switch (transformation)
  {
    case rot90:
      ret = onerow*(sq%onerow)-sq/onerow+(onerow-1);
      break;

    case rot180:
      ret = square_h8+square_a1 - sq;
      break;

    case rot270:
      ret = -onerow*(sq%onerow)+sq/onerow-(onerow-1)+square_h8+square_a1;
      break;

    case mirra1a8:
      ret = sq%onerow+onerow*((onerow-1)-sq/onerow);
      break;

    case mirra1h1:
      ret = ((onerow-1)-sq%onerow)+onerow*(sq/onerow);
      break;

    case mirra8h1:
      ret = onerow*(sq%onerow)+sq/onerow;
      break;

    case mirra1h8:
      ret = ((onerow-1)-sq/onerow)+onerow*((onerow-1)-sq%onerow);
      break;
  }

  return ret;
}
