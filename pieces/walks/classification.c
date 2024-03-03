#include "pieces/walks/classification.h"

boolean is_king(piece_walk_type p)
{
  switch (p)
  {
    case King:
    case Poseidon:
    case Sting:
      return  true;

    default:
      return  false;
  }
}

boolean is_pawn(piece_walk_type p)
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

boolean is_forwardpawn(piece_walk_type p)
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

boolean is_locust(piece_walk_type p)
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

boolean is_rider(piece_walk_type p)
{
  switch (p)
  {
    case    NightRider:
    case    Amazone:
    case    Empress:
    case    Princess:
    case    Waran:
    case    Camelrider:
    case    Zebrarider:
    case    Gnurider:
    case    RookHunter:
    case    BishopHunter:
    case    Rook:
    case    Queen:
    case    Bishop:
    case    WesirRider:
    case    FersRider:
    case    Elephant:
      return true;

    default:
      return false;
  }
}

boolean is_leaper(piece_walk_type p)
{
  switch (p)
  {
    case    Zebra:
    case    Camel:
    case    Giraffe:
    case    RootFiftyLeaper:
    case    Bucephale:
    case    Wesir:
    case    Alfil:
    case    Fers:
    case    Dabbaba:
    case    Gnu:
    case    Antilope:
    case    Squirrel:
    case    ErlKing:
    case    Okapi:
    case    Bison:
    case    Knight:
    case    Leap15:
    case    Leap16:
    case    Leap24:
    case    Leap25:
    case    Leap35:
    case    Leap37:
    case    Leap36:
      return true;
    default:  return false;
  }
}

boolean is_simplehopper(piece_walk_type p)
{
  switch (p)
  {
    case Grasshopper:
    case NightriderHopper:
    case CamelRiderHopper:
    case ZebraRiderHopper:
    case GnuRiderHopper:
    case RookHopper:
    case BishopHopper:
    case EquiHopper:
    case NonStopEquihopper:
    case GrassHopper2:
    case GrassHopper3:
    case KingHopper:
    case Orix:
    case NonStopOrix:
    case Elk:
    case RookMoose:
    case BishopMoose:
    case Eagle:
    case RookEagle:
    case BishopEagle:
    case Sparrow:
    case RookSparrow:
    case BishopSparrow:
    case KnightHopper:
    case EquiStopper:
    case NonstopEquiStopper:
      return true;

    default:
      return false;
  }
}

boolean is_chineserider(piece_walk_type p)
{
  switch (p)
  {
    case Leo:
    case Pao:
    case Vao:
    case Nao:
      return true;

    default:
      return false;
  }
}

boolean is_lion(piece_walk_type p)
{
  switch (p)
  {
    case Lion:
    case RookLion:
    case BishopLion:
    case NightRiderLion:
      return true;

    default:
      return false;
  }
}

boolean is_simpledecomposedleaper(piece_walk_type p)
{
  switch (p)
  {
    case Mao:
    case Moa:
      return true;

    default:
      return false;
  }
}

boolean is_symmetricfairy(piece_walk_type p)
{
  /* any piece where, if p captures X is legal, then it's also legal if p and X are swapped */
  switch (p)
  {
    case Leo:
    case Vao:
    case Pao:
    case Lion:
    case RookLion:
    case BishopLion:
      return true;

    default:
      return false;
  }
}
