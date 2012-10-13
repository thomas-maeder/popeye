#include "pieces/walks.h"
#include "pydata.h"

PieNam standard_walks[Bishop+1];
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

  if (CondFlag[cavaliermajeur])
    standard_walks[Knight] = NightRider;
}
