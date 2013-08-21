#include "pieces/walks/classification.h"

boolean is_king(PieNam p)
{
  switch (p)
  {
    case King:
    case Poseidon:
      return  true;

    default:
      return  false;
  }
}

boolean is_pawn(PieNam p)
{
  switch (p)
  {
    case Pawn:
    case BerolinaPawn:
    case SuperBerolinaPawn:
    case SuperPawn:
    case ReversePawn:
    case MarinePawn:
      return  true;

    default:
      return  false;
  }
}

boolean is_forwardpawn(PieNam p)
{
  switch (p)
  {
    case Pawn:
    case BerolinaPawn:
    case SuperBerolinaPawn:
    case SuperPawn:
    case MarinePawn:
      return  true;

    default:
      return  false;
  }
}

boolean is_locust(PieNam p)
{
  switch (p)
  {
    case Locust:
    case RookLocust:
    case BishopLocust:
    case NightLocust:
    case RoseLocust:
    case Sirene:
    case Triton:
    case Nereide:
    case Poseidon:
    case MarinePawn:
    case MarineKnight:
    case Skylla:
    case Charybdis:
      return  true;

    default:
      return  false;
  }
}
