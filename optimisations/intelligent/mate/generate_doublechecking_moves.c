#include "optimisations/intelligent/mate/generate_doublechecking_moves.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void front_check_by_officer_via(unsigned int index_of_checker,
                                       stip_length_type n,
                                       square via)
{
  piece const checker_type = white[index_of_checker].type;
  Flags const checker_flags = white[index_of_checker].flags;
  square const checker_origin = white[index_of_checker].diagram_square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
  {
    TraceSquare(*bnp);
    TracePiece(e[*bnp]);
    TraceText("\n");
    if (e[*bnp]==vide
        && guards(king_square[Black],checker_type,*bnp)
        && (intelligent_count_nr_of_moves_from_to_no_check(checker_type,
                                                           via,
                                                           checker_type,
                                                           *bnp)
            ==1))
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(checker_type,
                                                                                 checker_origin,
                                                                                 checker_type,
                                                                                 via);
        if (time+1<=Nr_remaining_white_moves)
        {
          Nr_remaining_white_moves -= time+1;
          TraceValue("%u\n",Nr_remaining_white_moves);
          SetPiece(checker_type,*bnp,checker_flags);
          remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,+1);
          remember_to_keep_checking_line_open(via,*bnp,checker_type,+1);
          intelligent_guard_flights(n);
          remember_to_keep_checking_line_open(via,*bnp,checker_type,-1);
          remember_to_keep_checking_line_open(*bnp,king_square[Black],checker_type,-1);
          e[*bnp] = vide;
          spec[*bnp] = EmptySpec;
          Nr_remaining_white_moves += time+1;
        }
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_unpromoted_pawn(unsigned int index_of_checker,
                                           stip_length_type n,
                                           square via,
                                           int dir)
{
  square const check_square = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceSquare(via);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  if (e[check_square]==vide)
  {
    Flags const pawn_flags = white[index_of_checker].flags;
    square const pawn_origin = white[index_of_checker].diagram_square;
    unsigned int const diffcol = abs(pawn_origin%onerow - via%onerow);
    if (diffcol+1<=Nr_unused_black_masses)
    {
      unsigned int const time_pawn = intelligent_count_nr_of_moves_from_to_no_check(pb,
                                                                                    pawn_origin,
                                                                                    pb,
                                                                                    via);
      if (time_pawn+1<=Nr_remaining_white_moves)
      {
        unsigned int const time_capturee = intelligent_count_nr_black_moves_to_square(check_square);
        if (time_capturee<=Nr_remaining_black_moves)
        {
          Nr_unused_black_masses -= diffcol+1;
          Nr_remaining_white_moves -= time_pawn+1;
          Nr_remaining_black_moves -= time_capturee;
          TraceValue("%u",Nr_unused_black_masses);
          TraceValue("%u",Nr_remaining_black_moves);
          TraceValue("%u\n",Nr_remaining_white_moves);
          SetPiece(pb,check_square,pawn_flags);
          intelligent_guard_flights(n);
          e[check_square] = vide;
          spec[check_square] = EmptySpec;
          Nr_remaining_black_moves += time_capturee;
          Nr_remaining_white_moves += time_pawn+1;
          Nr_unused_black_masses += diffcol+1;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_promoted_pawn_without_capture(unsigned int index_of_checker,
                                                         stip_length_type n,
                                                         square via,
                                                         int dir)
{
  square const check_from = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceSquare(via);
  TraceValue("%d",dir);
  TraceFunctionParamListEnd();

  if (e[check_from]==vide)
  {
    Flags const pawn_flags = white[index_of_checker].flags;
    square const pawn_origin = white[index_of_checker].diagram_square;
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(pb,
                                                                             pawn_origin,
                                                                             pb,
                                                                             via);
    if (time+1<=Nr_remaining_white_moves)
    {
      unsigned int const diffcol = abs(pawn_origin%onerow - via%onerow);

      Nr_remaining_white_moves -= time+1;

      if (diffcol<=Nr_unused_black_masses)
      {
        piece pp;

        Nr_unused_black_masses -= diffcol;
        TraceValue("%u",Nr_unused_black_masses);
        TraceValue("%u\n",Nr_remaining_white_moves);

        for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
          if (guards(king_square[Black],pp,check_from))
          {
            SetPiece(pp,check_from,pawn_flags);
            intelligent_guard_flights(n);
          }

        e[check_from] = vide;
        spec[check_from] = EmptySpec;

        Nr_unused_black_masses += diffcol;
      }

      Nr_remaining_white_moves += time+1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_promoted_pawn_with_capture(unsigned int index_of_checker,
                                                      stip_length_type n,
                                                      square via,
                                                      int dir)
{
  square const check_from = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceSquare(via);
  TraceValue("%d",dir);
  TraceFunctionParamListEnd();

  if (e[check_from]==vide)
  {
    Flags const pawn_flags = white[index_of_checker].flags;
    square const pawn_origin = white[index_of_checker].diagram_square;
    unsigned int const time_pawn = intelligent_count_nr_of_moves_from_to_no_check(pb,
                                                                                  pawn_origin,
                                                                                  pb,
                                                                                  via);
    if (time_pawn+1<=Nr_remaining_white_moves)
    {
      unsigned int const time_capturee = intelligent_count_nr_black_moves_to_square(check_from);

      if (time_capturee<=Nr_remaining_black_moves)
      {
        unsigned int const diffcol = abs(pawn_origin%onerow - via%onerow) + 1;

        if (diffcol<=Nr_unused_black_masses)
        {
          piece pp;

          Nr_remaining_white_moves -= time_pawn+1;
          Nr_remaining_black_moves -= time_capturee;
          Nr_unused_black_masses -= diffcol;
          TraceValue("%u",Nr_remaining_white_moves);
          TraceValue("%u",Nr_remaining_black_moves);
          TraceValue("%u\n",Nr_unused_black_masses);

          for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
            if (guards(king_square[Black],pp,check_from))
            {
              SetPiece(pp,check_from,pawn_flags);
              intelligent_guard_flights(n);
            }

          e[check_from] = vide;
          spec[check_from] = EmptySpec;

          Nr_unused_black_masses += diffcol;
          Nr_remaining_black_moves += time_capturee;
          Nr_remaining_white_moves += time_pawn+1;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_pawn(unsigned int index_of_checker,
                                stip_length_type n,
                                square via)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  if (via+2*dir_up==king_square[Black])
  {
    front_check_by_unpromoted_pawn(index_of_checker,n,via,dir_up+dir_left);
    front_check_by_unpromoted_pawn(index_of_checker,n,via,dir_up+dir_right);
  }

  if (square_a7<=via && via<=square_h7)
  {
    front_check_by_promoted_pawn_without_capture(index_of_checker,n,via,dir_up);
    front_check_by_promoted_pawn_with_capture(index_of_checker,n,via,dir_up+dir_left);
    front_check_by_promoted_pawn_with_capture(index_of_checker,n,via,dir_up+dir_right);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_front_check_via(stip_length_type n,
                                     square via,
                                     boolean diagonal)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
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
            front_check_by_officer_via(index,n,via);
          break;

        case fb:
          if (!diagonal)
            front_check_by_officer_via(index,n,via);
          break;

        case cb:
          front_check_by_officer_via(index,n,via);
          break;

        case pb:
          front_check_by_pawn(index,n,via);
          break;

        default:
          break;
      }

      white[index].usage = piece_is_unused;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_front_check(stip_length_type n, square rear_pos)
{
  int const dir = CheckDirQueen[king_square[Black]-rear_pos];
  square const start = rear_pos+dir;
  boolean const diagonal = SquareCol(rear_pos)==SquareCol(start);
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(rear_pos);
  TraceFunctionParamListEnd();

  assert(dir!=0);

  for (s = start; s!=king_square[Black]; s += dir)
    generate_front_check_via(n,s,diagonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_promotee(unsigned int index_of_checker,
                                   numvec start, numvec end,
                                   stip_length_type n,
                                   piece checker_type)
{
  Flags const checker_flags = white[index_of_checker].flags;
  numvec k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
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
      {
        /* A rough check whether it is worth thinking about promotions */
        unsigned int const min_nr_moves_by_p = (rear_pos<=square_h7
                                                ? moves_to_white_prom[index_of_checker]+1
                                                : moves_to_white_prom[index_of_checker]);
        if (Nr_remaining_white_moves>=min_nr_moves_by_p)
        {
          square const pawn_origin = white[index_of_checker].diagram_square;
          unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(pawn_origin,
                                                                                         checker_type,
                                                                                         rear_pos);
          TraceSquare(rear_pos);TracePiece(e[rear_pos]);TraceText("\n");
          if (time<=Nr_remaining_white_moves)
          {
            Nr_remaining_white_moves -= time;
            TraceValue("%u\n",Nr_remaining_white_moves);
            SetPiece(checker_type,rear_pos,checker_flags);
            remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,+1);
            generate_front_check(n,rear_pos);
            remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,-1);
            Nr_remaining_white_moves += time;
          }

          e[rear_pos] = vide;
          spec[rear_pos] = EmptySpec;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_promoted_pawn(unsigned int index_of_checker,
                                        stip_length_type n)
{
  piece pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
    switch (pp)
    {
      case db:
        rear_check_by_promotee(index_of_checker,vec_queen_start,vec_queen_end,n,pp);
        break;

      case tb:
        rear_check_by_promotee(index_of_checker,vec_rook_start,vec_rook_end,n,pp);
        break;

      case fb:
        rear_check_by_promotee(index_of_checker,vec_bishop_start,vec_bishop_end,n,pp);
        break;

      default:
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_rider(unsigned int index_of_checker,
                                numvec start, numvec end,
                                stip_length_type n,
                                piece checker_type)
{
  Flags const checker_flags = white[index_of_checker].flags;
  numvec k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParam("%u",n);
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
      {
        square const checker_origin = white[index_of_checker].diagram_square;
        unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(checker_type,
                                                                                 checker_origin,
                                                                                 checker_type,
                                                                                 rear_pos);
        TraceSquare(rear_pos);TracePiece(e[rear_pos]);TraceText("\n");
        if (time<=Nr_remaining_white_moves)
        {
          Nr_remaining_white_moves -= time;
          TraceValue("%u\n",Nr_remaining_white_moves);
          SetPiece(checker_type,rear_pos,checker_flags);
          remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,+1);
          generate_front_check(n,rear_pos);
          remember_to_keep_rider_line_open(rear_pos,king_square[Black],-dir,-1);
          Nr_remaining_white_moves += time;
        }

        e[rear_pos] = vide;
        spec[rear_pos] = EmptySpec;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void battery(stip_length_type n)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
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
        rear_check_by_rider(index,vec_queen_start,vec_queen_end,n,checker_type);
        break;

      case tb:
        rear_check_by_rider(index,vec_rook_start,vec_rook_end,n,checker_type);
        break;

      case fb:
        rear_check_by_rider(index,vec_bishop_start,vec_bishop_end,n,checker_type);
        break;

      case pb:
        rear_check_by_promoted_pawn(index,n);
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
                                                 stip_length_type n,
                                                 piece rider_type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParam("%u",n);
  TracePiece(rider_type);
  TraceFunctionParamListEnd();

  {
    square const rider_origin = white[checker_index].diagram_square;
    Flags const rider_spec = white[checker_index].flags;
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(rider_type,
                                                                             rider_origin,
                                                                             rider_type,
                                                                             check_from);
    if (time<=Nr_remaining_white_moves)
    {
      Nr_remaining_white_moves -= time;
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(rider_type,check_from,rider_spec);
      TraceSquare(check_from);TracePiece(rider_type);TraceText("\n");
      intelligent_guard_flights(n);
      e[check_from] = vide;
      spec[check_from] = EmptySpec;
      Nr_remaining_white_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check_by_promoted_pawn(unsigned int checker_index,
                                                         square check_from,
                                                         stip_length_type n)
{
  piece pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
    if (pp==db || pp==tb)
    {
      square const pawn_origin = white[checker_index].diagram_square;
      Flags const pawn_spec = white[checker_index].flags;
      unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(pawn_origin,
                                                                                     pp,
                                                                                     check_from);
      if (time<=Nr_remaining_white_moves)
      {
        Nr_remaining_white_moves -= time;
        TraceValue("%u\n",Nr_remaining_white_moves);
        SetPiece(pp,check_from,pawn_spec);
        TraceSquare(check_from);TracePiece(pp);TraceText("\n");
        intelligent_guard_flights(n);
        e[check_from] = vide;
        spec[check_from] = EmptySpec;
        Nr_remaining_white_moves += time;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check(stip_length_type n)
{
  square const via_capturer = king_square[Black]+dir_up;
  square check_from;
  unsigned int checker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (check_from = via_capturer+dir_up; e[check_from]==vide; check_from += dir_up)
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
            en_passant_orthogonal_check_by_rider(checker_index,check_from,n,checker_type);
            break;

          case pb:
            en_passant_orthogonal_check_by_promoted_pawn(checker_index,check_from,n);
            break;

          default:
            break;
        }

        white[checker_index].usage = piece_is_unused;
      }

    ++nr_reasons_for_staying_empty[check_from];
  }

  for (check_from -= dir_up; check_from!=via_capturer; check_from -= dir_up)
    --nr_reasons_for_staying_empty[check_from];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check_by_rider(unsigned int checker_index,
                                               square check_from,
                                               stip_length_type n,
                                               piece rider_type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParam("%u",n);
  TracePiece(rider_type);
  TraceFunctionParamListEnd();

  {
    square const rider_origin = white[checker_index].diagram_square;
    Flags const rider_spec = white[checker_index].flags;
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(rider_type,
                                                                             rider_origin,
                                                                             rider_type,
                                                                             check_from);
    if (time<=Nr_remaining_white_moves)
    {
      Nr_remaining_white_moves -= time;
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(rider_type,check_from,rider_spec);
      TraceSquare(check_from);TracePiece(rider_type);TraceText("\n");
      en_passant_orthogonal_check(n);
      e[check_from] = vide;
      spec[check_from] = EmptySpec;
      Nr_remaining_white_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check_by_promoted_pawn(unsigned int checker_index,
                                                       square check_from,
                                                       stip_length_type n)
{
  square const pawn_origin = white[checker_index].diagram_square;
  Flags const pawn_spec = white[checker_index].flags;
  piece pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
    if (pp==db || pp==fb)
    {
      unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(pawn_origin,
                                                                                     pp,
                                                                                     check_from);
      if (time<=Nr_remaining_white_moves)
      {
        Nr_remaining_white_moves -= time;
        TraceValue("%u\n",Nr_remaining_white_moves);
        SetPiece(pp,check_from,pawn_spec);
        TraceSquare(check_from);TracePiece(pp);TraceText("\n");
        en_passant_orthogonal_check(n);
        e[check_from] = vide;
        spec[check_from] = EmptySpec;
        Nr_remaining_white_moves += time;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check(stip_length_type n, square via_capturee)
{
  int const dir = via_capturee-king_square[Black];
  square check_from;
  unsigned int checker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(via_capturee);
  TraceFunctionParamListEnd();

  for (check_from = via_capturee+dir; e[check_from]==vide; check_from += dir)
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
            en_passant_diagonal_check_by_rider(checker_index,check_from,n,checker_type);
            break;

          case pb:
            en_passant_diagonal_check_by_promoted_pawn(checker_index,check_from,n);
            break;

          default:
            break;
        }

        white[checker_index].usage = piece_is_unused;
      }

    ++nr_reasons_for_staying_empty[check_from];
  }

  for (check_from -= dir; check_from!=via_capturee; check_from -= dir)
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

static void en_passant_select_capturee(stip_length_type n, square via_capturee)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(via_capturee);
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
      black[index_capturee].usage = piece_is_captured;
      en_passant_diagonal_check(n,via_capturee);
      black[index_capturee].usage = piece_is_unused;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (square_a4<=king_square[Black]
      && king_square[Black]<=square_h4
      && Nr_remaining_black_moves>=1)
  {
    square const via_capturer = king_square[Black]+dir_up;
    if (e[via_capturer]==vide)
    {
      unsigned int capturer_index;

      Nr_remaining_black_moves -= 1;

      for (capturer_index = 1; capturer_index<MaxPiece[White]; ++capturer_index)
        if (white[capturer_index].type==pb
            && white[capturer_index].usage==piece_is_unused)
        {
          square const capturer_origin = white[capturer_index].diagram_square;
          unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(pb,
                                                                                   capturer_origin,
                                                                                   pb,
                                                                                   via_capturer);
          if (time+1<=Nr_remaining_white_moves)
          {
            unsigned int const diffcol = abs(capturer_origin%onerow - via_capturer%onerow);
            if (diffcol+1<=Nr_unused_black_masses)
            {
              Nr_unused_black_masses -= diffcol+1;
              Nr_remaining_white_moves -= time+1;
              TraceValue("%u",Nr_unused_black_masses);
              TraceValue("%u",Nr_remaining_white_moves);
              TraceValue("%u\n",Nr_remaining_black_moves);
              white[capturer_index].usage = piece_gives_check;
              en_passant_select_capturee(n,via_capturer+dir_left);
              en_passant_select_capturee(n,via_capturer+dir_right);
              white[capturer_index].usage = piece_is_unused;
              Nr_remaining_white_moves += time+1;
              Nr_unused_black_masses += diffcol+1;
            }
          }
        }

      Nr_remaining_black_moves += 1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_generate_doublechecking_moves(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  battery(n);
  en_passant(n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
