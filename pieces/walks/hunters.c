#include "pieces/walks/hunters.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>
#include <string.h>

HunterType huntertypes[maxnrhuntertypes];
unsigned int nrhuntertypes;

typedef enum
{
  UP,
  DOWN
} UPDOWN;


PieNam hunter_make_type(PieNam away, PieNam home)
{
  unsigned int i;
  for (i = 0; i!=nrhuntertypes; ++i)
    if (huntertypes[i].away==away && huntertypes[i].home==home)
      return Hunter0+i;

  if (nrhuntertypes<maxnrhuntertypes)
  {
    PieNam const result = Hunter0+nrhuntertypes;
    HunterType * const huntertype = huntertypes+nrhuntertypes;
    huntertype->away = away;
    huntertype->home = home;
    ++nrhuntertypes;
    return result;
  }
  else
    return Invalid;
}

static void filter(numecoup prevnbcou, UPDOWN u)
{
  square const sq_departure = curr_generation->departure;
  numecoup s = prevnbcou;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (s<current_move[nbply])
    if ((u==DOWN && move_generation_stack[s].arrival-sq_departure>-nr_files_on_board)
        || (u==UP && move_generation_stack[s].arrival-sq_departure<nr_files_on_board))
    {
      memmove(move_generation_stack+s,
              move_generation_stack+s+1,
              (current_move[nbply]-1-s) * sizeof move_generation_stack[s]);
      --current_move[nbply];
    }
    else
      ++s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_one_dir(PieNam part, UPDOWN updown)
{
  numecoup const savenbcou = current_move[nbply];
  generate_moves_for_piece_based_on_walk(part);
  filter(savenbcou,updown);
}

void hunter_generate_moves(PieNam walk)
{
  TraceFunctionEntry(__func__);
  TracePiece(walk);
  TraceFunctionParamListEnd();

  assert(walk>=Hunter0);
  assert(walk<Hunter0+maxnrhuntertypes);

  {
    unsigned int const typeofhunter = walk-Hunter0;
    HunterType const * const huntertype = huntertypes+typeofhunter;

    if (trait[nbply]==White)
    {
      generate_one_dir(huntertype->home,DOWN);
      generate_one_dir(huntertype->away,UP);
    }
    else
    {
      generate_one_dir(huntertype->away,DOWN);
      generate_one_dir(huntertype->home,UP);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void rook_hunter_generate_moves(void)
{
  generate_one_dir(Bishop,DOWN);
  generate_one_dir(Rook,UP);
}

void bishop_hunter_generate_moves(void)
{
  generate_one_dir(Rook,DOWN);
  generate_one_dir(Bishop,UP);
}
