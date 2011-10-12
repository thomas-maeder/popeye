#include "optimisations/intelligent/stalemate/pin_black_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>

static boolean is_line_empty(square start, square end, int dir)
{
  boolean result = true;
  square sq;

  TraceFunctionEntry(__func__);
  TraceSquare(start);
  TraceSquare(end);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (sq = start+dir; e[sq]==vide; sq += dir)
  {
    /* nothing */
  }

  result = sq==end;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* @return true iff a pin was actually placed
 */
static void pin_by_officer(stip_length_type n,
                           piece pinner_orig_type,
                           piece pinner_type,
                           Flags pinner_flags,
                           square pinner_comes_from,
                           square pin_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TracePiece(pinner_orig_type);
  TracePiece(pinner_type);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParamListEnd();

  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(pinner_orig_type,
                                                                             pinner_comes_from,
                                                                             pinner_type,
                                                                             pin_from);
    if (time<=Nr_remaining_white_moves)
    {
      --Nr_unused_white_masses;
      Nr_remaining_white_moves -= time;
      TraceValue("%u",Nr_unused_white_masses);
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(pinner_type,pin_from,pinner_flags);
      intelligent_stalemate_test_target_position(n);
      Nr_remaining_white_moves += time;
      ++Nr_unused_white_masses;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* @return true iff a pin was actually placed
 */
static void pin_by_promoted_pawn(stip_length_type n,
                                 Flags pinner_flags,
                                 square pinner_comes_from,
                                 square pin_from,
                                 boolean diagonal)
{
  piece const minor_pinner_type = diagonal ? fb : tb;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();

  pin_by_officer(n,
                 pb,minor_pinner_type,pinner_flags,
                 pinner_comes_from,
                 pin_from);
  pin_by_officer(n,
                 pb,db,pinner_flags,
                 pinner_comes_from,
                 pin_from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* @return true iff >=1 pin was actually placed
 */
static void pin_specific_piece_on(stip_length_type n,
                                  square sq_to_be_pinned,
                                  square pin_on,
                                  unsigned int pinner_index,
                                  boolean diagonal)
{
  piece const pinner_type = white[pinner_index].type;
  Flags const pinner_flags = white[pinner_index].flags;
  square const pinner_comes_from = white[pinner_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(sq_to_be_pinned);
  TraceSquare(pin_on);
  TraceFunctionParam("%u",pinner_index);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();
  switch (pinner_type)
  {
    case cb:
      break;

    case tb:
      if (!diagonal)
        pin_by_officer(n,tb,tb,pinner_flags,pinner_comes_from,pin_on);
      break;

    case fb:
      if (diagonal)
        pin_by_officer(n,fb,fb,pinner_flags,pinner_comes_from,pin_on);
      break;

    case db:
      pin_by_officer(n,db,db,pinner_flags,pinner_comes_from,pin_on);
      break;

    case pb:
      pin_by_promoted_pawn(n,pinner_flags,pinner_comes_from,pin_on,diagonal);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_pin_black_piece(stip_length_type n,
                                           square position_of_trouble_maker)
{
  int const dir = CheckDirQueen[position_of_trouble_maker-king_square[Black]];
  piece const pinned_type = e[position_of_trouble_maker];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(position_of_trouble_maker);
  TraceFunctionParamListEnd();

  if (Nr_unused_white_masses>0
      && dir!=0          /* we can only pin on queen lines */
      && pinned_type!=dn /* queens cannot be pinned */
      /* bishops can only be pined on rook lines and vice versa */
      && !(CheckDirBishop[dir]!=0 && pinned_type==fn)
      && !(CheckDirRook[dir]!=0 && pinned_type==tn)
      && is_line_empty(king_square[Black],position_of_trouble_maker,dir))
  {
    boolean const diagonal = SquareCol(king_square[Black]+dir)==SquareCol(king_square[Black]);

    square pin_on;
    for (pin_on = position_of_trouble_maker+dir; e[pin_on]==vide; pin_on += dir)
    {
      if (nr_reasons_for_staying_empty[pin_on]==0)
      {
        unsigned int pinner_index;
        for (pinner_index = 1; pinner_index<MaxPiece[White]; ++pinner_index)
        {
          if (white[pinner_index].usage==piece_is_unused)
          {
            white[pinner_index].usage = piece_pins;

            pin_specific_piece_on(n,
                                  position_of_trouble_maker,
                                  pin_on,
                                  pinner_index,
                                  diagonal);

            white[pinner_index].usage = piece_is_unused;
          }
        }

        e[pin_on] = vide;
        spec[pin_on] = EmptySpec;
      }

      ++nr_reasons_for_staying_empty[pin_on];
    }

    for (pin_on -= dir; pin_on!=position_of_trouble_maker; pin_on -= dir)
      --nr_reasons_for_staying_empty[pin_on];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
