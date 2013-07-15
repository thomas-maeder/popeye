#include "pieces/walks/walks.h"
#include "pydata.h"

standard_walks_type standard_walks;

/* Initialise array standard_walks according to the current fairy conditions
 */
void initalise_standard_walks(void)
{
  PieNam p;

  for (p = Empty; p<=Bishop; p++)
    standard_walks[p] = p;

  if (CondFlag[leofamily])
  {
    standard_walks[Queen] = Leo;
    standard_walks[Rook] = Pao;
    standard_walks[Bishop] = Vao;
    standard_walks[Knight] = Mao;
  }

  if (CondFlag[marine] || CondFlag[ultramarine])
  {
    standard_walks[Queen] = Sirene;
    standard_walks[Rook] = Triton;
    standard_walks[Bishop] = Nereide;
    standard_walks[Knight] = MarineKnight;
  }

  if (CondFlag[ultramarine])
  {
    standard_walks[King] = Poseidon;
    standard_walks[Pawn] = MarinePawn;
  }

  if (CondFlag[cavaliermajeur])
    standard_walks[Knight] = NightRider;
}
