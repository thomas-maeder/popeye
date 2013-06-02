#include "conditions/transmuting_kings/transmuting_kings.h"
#include "pydata.h"

PieNam transmpieces[nr_sides][PieceCount];

void init_regular_transmuting_pieces_sequence(Side side)
{
  unsigned int tp = 0;
  PieNam p;

  for (p = King; p<PieceCount; ++p) {
    if (may_exist[p] && p!=Dummy && p!=Hamster)
    {
      transmpieces[side][tp] = p;
      tp++;
    }
  }

  transmpieces[side][tp] = Empty;
}
