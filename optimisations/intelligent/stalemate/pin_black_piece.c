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
static boolean pin_by_officer(unsigned int nr_remaining_white_moves,
                              unsigned int nr_remaining_black_moves,
                              unsigned int max_nr_allowed_captures_by_white,
                              unsigned int max_nr_allowed_captures_by_black,
                              stip_length_type n,
                              piece pinner_orig_type,
                              piece pinner_type,
                              Flags pinner_flags,
                              square pinner_comes_from,
                              square pin_from)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
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
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(pinner_type,pin_from,pinner_flags);
      intelligent_stalemate_test_target_position(nr_remaining_white_moves-time,
                                                 nr_remaining_black_moves,
                                                 max_nr_allowed_captures_by_white,
                                                 max_nr_allowed_captures_by_black-1,
                                                 n);
      result = true;
    }
    else
      result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* @return true iff a pin was actually placed
 */
static boolean pin_by_promoted_pawn(unsigned int nr_remaining_white_moves,
                                    unsigned int nr_remaining_black_moves,
                                    unsigned int max_nr_allowed_captures_by_white,
                                    unsigned int max_nr_allowed_captures_by_black,
                                    stip_length_type n,
                                    Flags pinner_flags,
                                    square pinner_comes_from,
                                    square pin_from,
                                    boolean diagonal)
{
  boolean result = false;
  piece const minor_pinner_type = diagonal ? fb : tb;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceSquare(pinner_comes_from);
  TraceSquare(pin_from);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();

  if (pin_by_officer(nr_remaining_white_moves,
                     nr_remaining_black_moves,
                     max_nr_allowed_captures_by_white,
                     max_nr_allowed_captures_by_black,
                     n,
                     pb,minor_pinner_type,pinner_flags,
                     pinner_comes_from,
                     pin_from))
    result = true;
  if (pin_by_officer(nr_remaining_white_moves,
                     nr_remaining_black_moves,
                     max_nr_allowed_captures_by_white,
                     max_nr_allowed_captures_by_black,
                     n,
                     pb,db,pinner_flags,
                     pinner_comes_from,
                     pin_from))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* @return true iff >=1 pin was actually placed
 */
static boolean pin_specific_piece_on(unsigned int nr_remaining_white_moves,
                                     unsigned int nr_remaining_black_moves,
                                     unsigned int max_nr_allowed_captures_by_white,
                                     unsigned int max_nr_allowed_captures_by_black,
                                     stip_length_type n,
                                     square sq_to_be_pinned,
                                     square pin_on,
                                     unsigned int pinner_index,
                                     boolean diagonal)
{
  boolean result = false;
  piece const pinner_type = white[pinner_index].type;
  Flags const pinner_flags = white[pinner_index].flags;
  square const pinner_comes_from = white[pinner_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
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
      if (!diagonal
          && pin_by_officer(nr_remaining_white_moves,
                            nr_remaining_black_moves,
                            max_nr_allowed_captures_by_white,
                            max_nr_allowed_captures_by_black,
                            n,
                            tb,tb,pinner_flags,
                            pinner_comes_from,
                            pin_on))
        result = true;
      break;

    case fb:
      if (diagonal
          && pin_by_officer(nr_remaining_white_moves,
                            nr_remaining_black_moves,
                            max_nr_allowed_captures_by_white,
                            max_nr_allowed_captures_by_black,
                            n,
                            fb,fb,pinner_flags,
                            pinner_comes_from,
                            pin_on))
        result = true;
      break;

    case db:
      if (pin_by_officer(nr_remaining_white_moves,
                         nr_remaining_black_moves,
                         max_nr_allowed_captures_by_white,
                         max_nr_allowed_captures_by_black,
                         n,
                         db,db,pinner_flags,
                         pinner_comes_from,
                         pin_on))
        result = true;
      break;

    case pb:
      if (pin_by_promoted_pawn(nr_remaining_white_moves,
                               nr_remaining_black_moves,
                               max_nr_allowed_captures_by_white,
                               max_nr_allowed_captures_by_black,
                               n,
                               pinner_flags,
                               pinner_comes_from,
                               pin_on,
                               diagonal))
        result = true;
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* @return true iff >=1 pin was actually placed
 */
static boolean pin_specific_piece(unsigned int nr_remaining_white_moves,
                                  unsigned int nr_remaining_black_moves,
                                  unsigned int max_nr_allowed_captures_by_white,
                                  unsigned int max_nr_allowed_captures_by_black,
                                  stip_length_type n,
                                  square position_of_trouble_maker)
{
  boolean result = false;
  int const dir = CheckDirQueen[position_of_trouble_maker-king_square[Black]];
  piece const pinned_type = e[position_of_trouble_maker];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceSquare(position_of_trouble_maker);
  TraceFunctionParamListEnd();

  if (max_nr_allowed_captures_by_black>0
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

            if (pin_specific_piece_on(nr_remaining_white_moves,
                                      nr_remaining_black_moves,
                                      max_nr_allowed_captures_by_white,
                                      max_nr_allowed_captures_by_black,
                                      n,
                                      position_of_trouble_maker,
                                      pin_on,
                                      pinner_index,
                                      diagonal))
              result = true;

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
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean intelligent_stalemate_immobilise_by_pinning_any_trouble_maker(unsigned int nr_remaining_white_moves,
                                                                      unsigned int nr_remaining_black_moves,
                                                                      unsigned int max_nr_allowed_captures_by_white,
                                                                      unsigned int max_nr_allowed_captures_by_black,
                                                                      stip_length_type n,
                                                                      immobilisation_state_type const *state)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",state->nr_of_trouble_makers);
  for (i = 0; i!=state->nr_of_trouble_makers; ++i)
  {
    TraceSquare(state->positions_of_trouble_makers[i]);TraceText("\n");
    if (pin_specific_piece(nr_remaining_white_moves,
                           nr_remaining_black_moves,
                           max_nr_allowed_captures_by_white,
                           max_nr_allowed_captures_by_black,
                           n,
                           state->positions_of_trouble_makers[i]))
    {
      result = true;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
