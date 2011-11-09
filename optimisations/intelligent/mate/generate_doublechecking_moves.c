#include "optimisations/intelligent/mate/generate_doublechecking_moves.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void remember_to_keep_checking_line_open(square from, square to,
                                                piece type, int delta)
{
  int const diff = to-from;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TracePiece(type);
  TraceFunctionParamListEnd();

  assert(type>vide);

  switch (type)
  {
    case Bishop:
    case Rook:
    case Queen:
      remember_to_keep_rider_line_open(from,to,CheckDir[type][diff],delta);
      break;

    case Knight:
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_officer_via(unsigned int index_of_checker,
                                       square via)
{
  piece const checker_type = white[index_of_checker].type;
  Flags const checker_flags = white[index_of_checker].flags;
  square const checker_origin = white[index_of_checker].diagram_square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (e[*bnp]==vide && officer_guards(king_square[Black],checker_type,*bnp)
        && intelligent_reserve_front_check_by_officer(checker_origin,
                                                      via,
                                                      checker_type,
                                                      *bnp))
    {
      TraceSquare(*bnp);TracePiece(e[*bnp]);TraceText("\n");
      SetPiece(checker_type,*bnp,checker_flags);
      remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,+1);
      remember_to_keep_checking_line_open(via,*bnp,checker_type,+1);
      intelligent_guard_flights();
      remember_to_keep_checking_line_open(via,*bnp,checker_type,-1);
      remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,-1);
      e[*bnp] = vide;
      spec[*bnp] = EmptySpec;
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_unpromoted_pawn(unsigned int index_of_checker,
                                           square via,
                                           int dir)
{
  square const check_square = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  if (e[check_square]==vide
      && intelligent_reserve_front_check_by_pawn_with_capture(white[index_of_checker].diagram_square,
                                                              via,
                                                              check_square))
  {
    SetPiece(pb,check_square,white[index_of_checker].flags);
    intelligent_guard_flights();
    e[check_square] = vide;
    spec[check_square] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_promoted_pawn_without_capture(unsigned int index_of_checker,
                                                         square via,
                                                         int dir)
{
  square const check_from = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceValue("%d",dir);
  TraceFunctionParamListEnd();

  if (e[check_from]==vide
      && intelligent_reserve_front_check_by_pawn_without_capture(white[index_of_checker].diagram_square,
                                                                 via))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (officer_guards(king_square[Black],pp,check_from))
      {
        SetPiece(pp,check_from,white[index_of_checker].flags);
        intelligent_guard_flights();
      }

    e[check_from] = vide;
    spec[check_from] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_promoted_pawn_with_capture(unsigned int index_of_checker,
                                                      square via,
                                                      int dir)
{
  square const check_from = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceValue("%d",dir);
  TraceFunctionParamListEnd();

  if (e[check_from]==vide
      && intelligent_reserve_front_check_by_pawn_with_capture(white[index_of_checker].diagram_square,
                                                              via,
                                                              check_from))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (officer_guards(king_square[Black],pp,check_from))
      {
        SetPiece(pp,check_from,white[index_of_checker].flags);
        intelligent_guard_flights();
      }

    e[check_from] = vide;
    spec[check_from] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_pawn(unsigned int index_of_checker, square via)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  if (via+2*dir_up==king_square[Black])
  {
    front_check_by_unpromoted_pawn(index_of_checker,via,dir_up+dir_left);
    front_check_by_unpromoted_pawn(index_of_checker,via,dir_up+dir_right);
  }

  if (square_a7<=via && via<=square_h7)
  {
    front_check_by_promoted_pawn_without_capture(index_of_checker,via,dir_up);
    front_check_by_promoted_pawn_with_capture(index_of_checker,via,dir_up+dir_left);
    front_check_by_promoted_pawn_with_capture(index_of_checker,via,dir_up+dir_right);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_front_check_via(square via, boolean diagonal)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceSquare(via);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();

  for (index = 1; index<MaxPiece[White]; ++index)
    if (white[index].usage==piece_is_unused)
    {
      piece const checker_type = white[index].type;

      TraceValue("%u",index);
      TraceSquare(white[index].diagram_square);
      TracePiece(checker_type);
      TraceText("\n");

      white[index].usage = piece_gives_check;

      switch (checker_type)
      {
        case tb:
          if (diagonal)
            front_check_by_officer_via(index,via);
          break;

        case fb:
          if (!diagonal)
            front_check_by_officer_via(index,via);
          break;

        case cb:
          front_check_by_officer_via(index,via);
          break;

        case pb:
          front_check_by_pawn(index,via);
          break;

        default:
          break;
      }

      white[index].usage = piece_is_unused;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_front_check(square rear_pos)
{
  int const dir = CheckDir[Queen][king_square[Black]-rear_pos];
  square const start = rear_pos+dir;
  boolean const diagonal = SquareCol(rear_pos)==SquareCol(start);
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(rear_pos);
  TraceFunctionParamListEnd();

  assert(dir!=0);

  for (s = start; s!=king_square[Black]; s += dir)
    generate_front_check_via(s,diagonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_promotee(unsigned int index_of_checker,
                                   numvec start, numvec end,
                                   piece checker_type)
{
  Flags const checker_flags = white[index_of_checker].flags;
  numvec k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TracePiece(checker_type);
  TraceFunctionParamListEnd();

  for (k = start; k<=end; ++k)
  {
    int const dir = vec[k];
    TraceValue("%u",k);TraceValue("%d\n",dir);
    if (e[king_square[Black]+dir]==vide)
    {
      square rear_pos;
      for (rear_pos = king_square[Black]+2*dir; e[rear_pos]==vide; rear_pos += dir)
        if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_checker,
                                                                      rear_pos)
            && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_checker].diagram_square,
                                                                      checker_type,
                                                                      rear_pos))
        {
          SetPiece(checker_type,rear_pos,checker_flags);
          TraceSquare(rear_pos);TracePiece(checker_type);TraceText("\n");
          remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,+1);
          generate_front_check(rear_pos);
          remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,-1);
          e[rear_pos] = vide;
          spec[rear_pos] = EmptySpec;
          intelligent_unreserve();
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_promoted_pawn(unsigned int index_of_checker)
{
  piece pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParamListEnd();

  for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
    switch (pp)
    {
      case db:
        rear_check_by_promotee(index_of_checker,vec_queen_start,vec_queen_end,pp);
        break;

      case tb:
        rear_check_by_promotee(index_of_checker,vec_rook_start,vec_rook_end,pp);
        break;

      case fb:
        rear_check_by_promotee(index_of_checker,vec_bishop_start,vec_bishop_end,pp);
        break;

      default:
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_rider(unsigned int index_of_checker,
                                numvec start, numvec end,
                                piece checker_type)
{
  square const checker_origin = white[index_of_checker].diagram_square;
  Flags const checker_flags = white[index_of_checker].flags;
  numvec k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TracePiece(checker_type);
  TraceFunctionParamListEnd();

  for (k = start; k<=end; ++k)
  {
    int const dir = vec[k];
    TraceValue("%u",k);TraceValue("%d\n",dir);
    if (e[king_square[Black]+dir]==vide)
    {
      square rear_pos;
      for (rear_pos = king_square[Black]+2*dir; e[rear_pos]==vide; rear_pos += dir)
        if (intelligent_reserve_officer_moves_from_to(checker_origin,
                                                      checker_type,
                                                      rear_pos))
        {
          TraceSquare(rear_pos);TracePiece(e[rear_pos]);TraceText("\n");
          SetPiece(checker_type,rear_pos,checker_flags);
          remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,+1);
          generate_front_check(rear_pos);
          remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,-1);
          e[rear_pos] = vide;
          spec[rear_pos] = EmptySpec;
          intelligent_unreserve();
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void battery(void)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (index = 1; index<MaxPiece[White]; ++index)
  {
    piece const checker_type = white[index].type;

    TraceValue("%u",index);
    TraceSquare(white[index].diagram_square);
    TracePiece(checker_type);
    TraceText("\n");

    white[index].usage = piece_gives_check;

    switch (checker_type)
    {
      case db:
        rear_check_by_rider(index,vec_queen_start,vec_queen_end,checker_type);
        break;

      case tb:
        rear_check_by_rider(index,vec_rook_start,vec_rook_end,checker_type);
        break;

      case fb:
        rear_check_by_rider(index,vec_bishop_start,vec_bishop_end,checker_type);
        break;

      case pb:
        rear_check_by_promoted_pawn(index);
        break;

      default:
        break;
    }

    white[index].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check_by_rider(unsigned int checker_index,
                                                 square check_from,
                                                 piece rider_type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TracePiece(rider_type);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(white[checker_index].diagram_square,
                                                rider_type,
                                                check_from))
  {
    SetPiece(rider_type,check_from,white[checker_index].flags);
    TraceSquare(check_from);TracePiece(rider_type);TraceText("\n");
    intelligent_guard_flights();
    e[check_from] = vide;
    spec[check_from] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check_by_promoted_pawn(unsigned int checker_index,
                                                         square check_from)
{
  piece pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
    if ((pp==db || pp==tb)
        && intelligent_reserve_promoting_white_pawn_moves_from_to(white[checker_index].diagram_square,
                                                                  pp,
                                                                  check_from))
    {
      SetPiece(pp,check_from,white[checker_index].flags);
      TraceSquare(check_from);TracePiece(pp);TraceText("\n");
      intelligent_guard_flights();
      e[check_from] = vide;
      spec[check_from] = EmptySpec;
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check(int dir_vertical)
{
  square const via_capturer = king_square[Black]+dir_vertical;
  square check_from;
  unsigned int checker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  for (check_from = via_capturer+dir_vertical; e[check_from]==vide; check_from += dir_vertical)
  {
    for (checker_index = 1; checker_index<MaxPiece[White]; ++checker_index)
      if (white[checker_index].usage==piece_is_unused)
      {
        piece const checker_type = white[checker_index].type;

        TraceValue("%u",checker_index);
        TraceSquare(white[checker_index].diagram_square);
        TracePiece(checker_type);
        TraceText("\n");

        white[checker_index].usage = piece_gives_check;

        switch (checker_type)
        {
          case db:
          case tb:
            en_passant_orthogonal_check_by_rider(checker_index,check_from,checker_type);
            break;

          case pb:
            en_passant_orthogonal_check_by_promoted_pawn(checker_index,check_from);
            break;

          default:
            break;
        }

        white[checker_index].usage = piece_is_unused;
      }

    ++nr_reasons_for_staying_empty[check_from];
  }

  for (check_from -= dir_vertical; check_from!=via_capturer; check_from -= dir_vertical)
    --nr_reasons_for_staying_empty[check_from];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check_by_rider(unsigned int checker_index,
                                               square check_from,
                                               piece rider_type,
                                               int dir_vertical)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TracePiece(rider_type);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(white[checker_index].diagram_square,
                                                rider_type,
                                                check_from))
  {
    SetPiece(rider_type,check_from,white[checker_index].flags);
    TraceSquare(check_from);TracePiece(rider_type);TraceText("\n");
    en_passant_orthogonal_check(dir_vertical);
    e[check_from] = vide;
    spec[check_from] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check_by_promoted_pawn(unsigned int checker_index,
                                                       square check_from,
                                                       int dir_vertical)
{
  square const pawn_origin = white[checker_index].diagram_square;
  Flags const pawn_spec = white[checker_index].flags;
  piece pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
    if ((pp==db || pp==fb)
        && intelligent_reserve_promoting_white_pawn_moves_from_to(pawn_origin,
                                                                  pp,
                                                                  check_from))
    {
      SetPiece(pp,check_from,pawn_spec);
      TraceSquare(check_from);TracePiece(pp);TraceText("\n");
      en_passant_orthogonal_check(dir_vertical);
      e[check_from] = vide;
      spec[check_from] = EmptySpec;
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check(square via_capturee, int dir_vertical)
{
  int const dir_diagonal = via_capturee-king_square[Black];
  square check_from;
  unsigned int checker_index;

  TraceFunctionEntry(__func__);
  TraceSquare(via_capturee);
  TraceFunctionParam("%d",dir_diagonal);
  TraceFunctionParamListEnd();

  for (check_from = via_capturee+dir_diagonal; e[check_from]==vide; check_from += dir_diagonal)
  {
    for (checker_index = 1; checker_index<MaxPiece[White]; ++checker_index)
      if (white[checker_index].usage==piece_is_unused)
      {
        piece const checker_type = white[checker_index].type;

        TraceValue("%u",checker_index);
        TraceSquare(white[checker_index].diagram_square);
        TracePiece(checker_type);
        TraceText("\n");

        white[checker_index].usage = piece_gives_check;

        switch (checker_type)
        {
          case db:
          case fb:
            en_passant_diagonal_check_by_rider(checker_index,check_from,checker_type,dir_vertical);
            break;

          case pb:
            en_passant_diagonal_check_by_promoted_pawn(checker_index,check_from,dir_vertical);
            break;

          default:
            break;
        }

        white[checker_index].usage = piece_is_unused;
      }

    ++nr_reasons_for_staying_empty[check_from];
  }

  for (check_from -= dir_diagonal; check_from!=via_capturee; check_from -= dir_diagonal)
    --nr_reasons_for_staying_empty[check_from];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int enpassant_find_capturee(square capturee_origin)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(capturee_origin);
  TraceFunctionParamListEnd();

  for (result = 1; result<MaxPiece[Black]; ++result)
    if (black[result].diagram_square==capturee_origin)
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void en_passant_select_capturee(square via_capturee, int dir_vertical)
{
  TraceFunctionEntry(__func__);
  TraceSquare(via_capturee);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  TracePiece(e[via_capturee]);TraceText("\n");
  if (e[via_capturee]==vide)
  {
    square const capturee_origin = via_capturee+2*dir_up;
    unsigned int const index_capturee = enpassant_find_capturee(capturee_origin);
    if (index_capturee!=MaxPiece[Black]
        && black[index_capturee].type==pn
        && black[index_capturee].usage==piece_is_unused)
    {
      ++nr_reasons_for_staying_empty[via_capturee];
      black[index_capturee].usage = piece_is_captured;
      en_passant_diagonal_check(via_capturee,dir_vertical);
      black[index_capturee].usage = piece_is_unused;
      --nr_reasons_for_staying_empty[via_capturee];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant(square king_row_start, square king_row_end,
                       int dir_vertical)
{
  TraceFunctionEntry(__func__);
  TraceSquare(king_row_start);
  TraceSquare(king_row_end);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  if (king_row_start<=king_square[Black] && king_square[Black]<=king_row_end)
  {
    square const via_capturer = king_square[Black]+dir_vertical;
    if (e[via_capturer]==vide)
    {
      unsigned int capturer_index;

      ++nr_reasons_for_staying_empty[via_capturer];

      for (capturer_index = 1; capturer_index<MaxPiece[White]; ++capturer_index)
        if (white[capturer_index].type==pb
            && white[capturer_index].usage==piece_is_unused
            && intelligent_reserve_double_check_by_enpassant_capture(white[capturer_index].diagram_square,
                                                                     via_capturer))
        {
          white[capturer_index].usage = piece_gives_check;
          en_passant_select_capturee(via_capturer+dir_left,dir_vertical);
          en_passant_select_capturee(via_capturer+dir_right,dir_vertical);
          white[capturer_index].usage = piece_is_unused;
          intelligent_unreserve();
        }

      --nr_reasons_for_staying_empty[via_capturer];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_generate_doublechecking_moves(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,2))
  {
    battery();
    intelligent_unreserve();

    if (intelligent_reserve_masses(White,3))
    {
      en_passant(square_a4,square_h4,dir_up);
      en_passant(square_a6,square_h6,dir_down);
      intelligent_unreserve();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
