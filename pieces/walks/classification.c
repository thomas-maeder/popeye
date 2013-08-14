#include "pieces/walks/classification.h"

boolean is_king(PieNam p)
{
  switch (p)
  {
    case  King:
    case  Poseidon:
      return  true;

    default:
      return  false;
  }
}

boolean is_pawn(PieNam p)
{
  switch (p)
  {
    case  Pawn:
    case  BerolinaPawn:
    case  SuperBerolinaPawn:
    case  SuperPawn:
    case  ReversePawn:
    case  MarinePawn:
      return  true;

    default:
      return  false;
  }
}

boolean is_forwardpawn(PieNam p)
{
  switch (p)
  {
    case  Pawn:
    case  BerolinaPawn:
    case  SuperBerolinaPawn:
    case  SuperPawn:
    case  MarinePawn:
      return  true;

    default:
      return  false;
  }
}
