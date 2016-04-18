#include "pieces/walks/walks.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"

standard_walks_type standard_walks;

/* Determine the orthodox counterpart of a walk
 * @param walk walk to be orthodoxised
 * @return unstandardised (i.e. orthodox) counterpart (one of King..Bishop)
 *         or the walk itself if it is not a standard walk
 */
piece_walk_type orthodoxise_walk(piece_walk_type walk)
{
  piece_walk_type candidate;

  for (candidate = King; candidate<=Bishop; ++candidate)
    if (standard_walks[candidate]==walk)
      return candidate;

  return walk;
}

/* Initialise array standard_walks according to the current fairy conditions
 */
void initalise_standard_walks(void)
{
  piece_walk_type p;

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

  if (CondFlag[argentinian])
  {
    standard_walks[Queen] = Senora;
    standard_walks[Rook] = Faro;
    standard_walks[Bishop] = Loco;
    standard_walks[Knight] = Saltador;
  }
}
