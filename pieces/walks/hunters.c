#include "pieces/walks/hunters.h"
#include "pieces/walks/generate_moves.h"
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

static void filter(square sq_departure, numecoup prevnbcou, UPDOWN u)
{
  numecoup s = prevnbcou;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
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

static void generate_one_dir(square sq_departure, PieNam part, UPDOWN updown)
{
  numecoup const savenbcou = current_move[nbply];
  generate_moves_for_piece_based_on_walk(sq_departure,part);
  filter(sq_departure,savenbcou,updown);
}

void hunter_generate_moves(square sq_departure, PieNam walk)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(walk);
  TraceFunctionParamListEnd();

  assert(walk>=Hunter0);
  assert(walk<Hunter0+maxnrhuntertypes);

  {
    unsigned int const typeofhunter = walk-Hunter0;
    HunterType const * const huntertype = huntertypes+typeofhunter;

    if (trait[nbply]==White)
    {
      generate_one_dir(sq_departure,huntertype->home,DOWN);
      generate_one_dir(sq_departure,huntertype->away,UP);
    }
    else
    {
      generate_one_dir(sq_departure,huntertype->away,DOWN);
      generate_one_dir(sq_departure,huntertype->home,UP);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void rook_hunter_generate_moves(square sq_departure)
{
  generate_one_dir(sq_departure,Bishop,DOWN);
  generate_one_dir(sq_departure,Rook,UP);
}

void bishop_hunter_generate_moves(square sq_departure)
{
  generate_one_dir(sq_departure,Rook,DOWN);
  generate_one_dir(sq_departure,Bishop,UP);
}
