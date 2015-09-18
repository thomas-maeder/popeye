#include "optimisations/intelligent/mate/generate_doublechecking_moves.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "optimisations/orthodox_check_directions.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static void remember_to_keep_checking_line_open(square from, square to,
                                                piece_walk_type type, int delta)
{
  int const diff = to-from;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceWalk(type);
  TraceFunctionParamListEnd();

  assert(type>Empty);

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

static void front_check_by_rider_via(slice_index si,
                                     unsigned int index_of_checker,
                                     square via)
{
  piece_walk_type const checker_type = white[index_of_checker].type;
  Flags const checker_flags = white[index_of_checker].flags;
  square const checker_origin = white[index_of_checker].diagram_square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (is_square_empty(*bnp))
    {
      int const dir = CheckDir[checker_type][being_solved.king_square[Black]-*bnp];
      if (dir!=0
          && is_line_empty(*bnp,being_solved.king_square[Black],dir)
          && intelligent_reserve_front_check_by_officer(checker_origin,
                                                        via,
                                                        checker_type,
                                                        *bnp))
      {
        TraceSquare(*bnp);TraceWalk(being_solved.board[*bnp]);TraceEOL();
        occupy_square(*bnp,checker_type,checker_flags);
        remember_to_keep_checking_line_open(*bnp,being_solved.king_square[Black],checker_type,+1);
        remember_to_keep_checking_line_open(via,*bnp,checker_type,+1);
        intelligent_guard_flights(si);
        remember_to_keep_checking_line_open(via,*bnp,checker_type,-1);
        remember_to_keep_checking_line_open(*bnp,being_solved.king_square[Black],checker_type,-1);
        empty_square(*bnp);
        intelligent_unreserve();
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_knight_via(slice_index si,
                                      unsigned int index_of_checker,
                                      square via)
{
  Flags const checker_flags = white[index_of_checker].flags;
  square const checker_origin = white[index_of_checker].diagram_square;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (is_square_empty(*bnp)
        && CheckDir[Knight][being_solved.king_square[Black]-*bnp]!=0
        && intelligent_reserve_front_check_by_officer(checker_origin,
                                                      via,
                                                      Knight,
                                                      *bnp))
    {
      TraceSquare(*bnp);TraceWalk(being_solved.board[*bnp]);TraceEOL();
      occupy_square(*bnp,Knight,checker_flags);
      intelligent_guard_flights(si);
      empty_square(*bnp);
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_promotee_rider(slice_index si,
                                          unsigned int index_of_checker,
                                          piece_walk_type promotee_type,
                                          square via,
                                          vec_index_type vec_start,
                                          vec_index_type vec_end)
{
  Flags const checker_flags = white[index_of_checker].flags;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParam("%u",vec_start);
  TraceFunctionParam("%u",vec_end);
  TraceFunctionParamListEnd();

  for (i = vec_start; i<=vec_end; ++i)
  {
    numvec const dir = vec[i];
    square to_square;
    for (to_square = via+dir; is_square_empty(to_square); to_square += dir)
    {
      int const check_dir = CheckDir[promotee_type][being_solved.king_square[Black]-to_square];
      if (check_dir!=0)
      {
        if (is_line_empty(to_square,being_solved.king_square[Black],check_dir))
        {
          TraceSquare(to_square);TraceWalk(being_solved.board[to_square]);TraceEOL();
          occupy_square(to_square,promotee_type,checker_flags);
          remember_to_keep_checking_line_open(to_square,being_solved.king_square[Black],promotee_type,+1);
          remember_to_keep_checking_line_open(via,to_square,promotee_type,+1);
          intelligent_guard_flights(si);
          remember_to_keep_checking_line_open(via,to_square,promotee_type,-1);
          remember_to_keep_checking_line_open(to_square,being_solved.king_square[Black],promotee_type,-1);
          empty_square(to_square);
        }
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_promotee_knight(slice_index si,
                                           unsigned int index_of_checker,
                                           square via)
{
  Flags const checker_flags = white[index_of_checker].flags;
  vec_index_type i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  for (i = vec_knight_start; i<=vec_knight_end; ++i)
  {
    square const to_square = via+vec[i];
    if (is_square_empty(to_square)
        && CheckDir[Knight][being_solved.king_square[Black]-to_square]!=0)
    {
      TraceSquare(to_square);TraceWalk(being_solved.board[to_square]);TraceEOL();
      occupy_square(to_square,Knight,checker_flags);
      intelligent_guard_flights(si);
      empty_square(to_square);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_promotee(slice_index si,
                                    unsigned int index_of_checker,
                                    square via)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      if (pp!=Queen
          && intelligent_reserve_front_check_by_promotee(white[index_of_checker].diagram_square,
                                                         pp,
                                                         via))
      {
        switch (pp)
        {
          case Rook:
            front_check_by_promotee_rider(si,index_of_checker,Rook,via,vec_rook_start,vec_rook_end);
            break;

          case Bishop:
            front_check_by_promotee_rider(si,index_of_checker,Bishop,via,vec_bishop_start,vec_bishop_end);
            break;

          case Knight:
            front_check_by_promotee_knight(si,index_of_checker,via);
            break;

          default:
            assert(0);
            break;
        }

        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_unpromoted_pawn(slice_index si,
                                           unsigned int index_of_checker,
                                           square via,
                                           int dir)
{
  square const check_square = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  if (is_square_empty(check_square)
      && intelligent_reserve_front_check_by_pawn_with_capture(white[index_of_checker].diagram_square,
                                                              via,
                                                              check_square))
  {
    occupy_square(check_square,Pawn,white[index_of_checker].flags);
    intelligent_guard_flights(si);
    empty_square(check_square);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_pawn_promotion_without_capture(slice_index si,
                                                          unsigned int index_of_checker,
                                                          square via,
                                                          int dir)
{
  square const check_from = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceValue("%d",dir);
  TraceFunctionParamListEnd();

  if (is_square_empty(check_from)
      && intelligent_reserve_front_check_by_pawn_without_capture(white[index_of_checker].diagram_square,
                                                                 via))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      /* geometry doesn't allow for an interceptable check by a pawn that
       * doesn't capture */
      if (GuardDir[pp-Pawn][check_from].dir==guard_dir_check_uninterceptable)
      {
        occupy_square(check_from,pp,white[index_of_checker].flags);
        intelligent_guard_flights(si);
      }

    empty_square(check_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_pawn_promotion_with_capture(slice_index si,
                                                       unsigned int index_of_checker,
                                                       square via,
                                                       int dir)
{
  square const check_from = via+dir;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceValue("%d",dir);
  TraceFunctionParamListEnd();

  if (is_square_empty(check_from)
      && intelligent_reserve_front_check_by_pawn_with_capture(white[index_of_checker].diagram_square,
                                                              via,
                                                              check_from))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
    {
      int const dir = CheckDir[pp][being_solved.king_square[Black]-check_from];
      if (dir!=0)
        switch (pp)
        {
          case Queen:
          case Rook:
            if (is_line_empty(check_from,being_solved.king_square[Black],dir))
            {
              occupy_square(check_from,pp,white[index_of_checker].flags);
              remember_to_keep_checking_line_open(check_from,being_solved.king_square[Black],pp,+1);
              intelligent_guard_flights(si);
              remember_to_keep_checking_line_open(check_from,being_solved.king_square[Black],pp,-1);
            }
            break;

          case Bishop:
          case Knight:
            occupy_square(check_from,pp,white[index_of_checker].flags);
            intelligent_guard_flights(si);
            break;

          default:
            assert(0);
            break;
        }
    }

    empty_square(check_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void front_check_by_pawn(slice_index si,
                                unsigned int index_of_checker,
                                square via)
{
  Flags const mask = BIT(Black)|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(via);
  TraceFunctionParamListEnd();

  if (TSTFULLFLAGMASK(being_solved.spec[via+2*dir_up],mask))
  {
    front_check_by_unpromoted_pawn(si,index_of_checker,via,dir_up+dir_left);
    front_check_by_unpromoted_pawn(si,index_of_checker,via,dir_up+dir_right);
  }

  if (TSTFLAG(sq_spec[via+dir_up],WhPromSq))
  {
    front_check_by_pawn_promotion_without_capture(si,index_of_checker,via,dir_up);
    front_check_by_pawn_promotion_with_capture(si,index_of_checker,via,dir_up+dir_left);
    front_check_by_pawn_promotion_with_capture(si,index_of_checker,via,dir_up+dir_right);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_front_check_via(slice_index si,
                                     square via,
                                     boolean diagonal)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceSquare(via);
  TraceFunctionParam("%u",diagonal);
  TraceFunctionParamListEnd();

  for (index = 1; index<MaxPiece[White]; ++index)
    if (white[index].usage==piece_is_unused)
    {
      piece_walk_type const checker_type = white[index].type;

      TraceValue("%u",index);
      TraceSquare(white[index].diagram_square);
      TraceWalk(checker_type);
      TraceEOL();

      white[index].usage = piece_gives_check;

      switch (checker_type)
      {
        case Rook:
          if (diagonal)
            front_check_by_rider_via(si,index,via);
          break;

        case Bishop:
          if (!diagonal)
            front_check_by_rider_via(si,index,via);
          break;

        case Knight:
          front_check_by_knight_via(si,index,via);
          break;

        case Pawn:
          front_check_by_pawn(si,index,via);
          front_check_by_promotee(si,index,via);
          break;

        default:
          break;
      }

      white[index].usage = piece_is_unused;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_front_check(slice_index si,
                                 square rear_pos)
{
  int const dir = CheckDir[Queen][being_solved.king_square[Black]-rear_pos];
  square const start = rear_pos+dir;
  boolean const diagonal = SquareCol(rear_pos)==SquareCol(start);
  Flags const mask = BIT(Black)|BIT(Royal);
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(rear_pos);
  TraceFunctionParamListEnd();

  assert(dir!=0);

  for (s = start; !TSTFULLFLAGMASK(being_solved.spec[s],mask); s += dir)
    generate_front_check_via(si,s,diagonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_promotee(slice_index si,
                                   unsigned int index_of_checker,
                                   vec_index_type start, vec_index_type end,
                                   piece_walk_type checker_type)
{
  Flags const checker_flags = white[index_of_checker].flags;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceWalk(checker_type);
  TraceFunctionParamListEnd();

  for (k = start; k<=end; ++k)
  {
    int const dir = vec[k];
    TraceValue("%u",k);TraceValue("%d\n",dir);
    if (is_square_empty(being_solved.king_square[Black]+dir))
    {
      square rear_pos;
      for (rear_pos = being_solved.king_square[Black]+2*dir; is_square_empty(rear_pos); rear_pos += dir)
        if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_checker,
                                                                      rear_pos)
            && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_checker].diagram_square,
                                                                      checker_type,
                                                                      rear_pos))
        {
          occupy_square(rear_pos,checker_type,checker_flags);
          TraceSquare(rear_pos);TraceWalk(checker_type);TraceEOL();
          remember_to_keep_rider_line_open(rear_pos,being_solved.king_square[Black],-dir,+1);
          generate_front_check(si,rear_pos);
          remember_to_keep_rider_line_open(rear_pos,being_solved.king_square[Black],-dir,-1);
          empty_square(rear_pos);
          intelligent_unreserve();
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_promoted_pawn(slice_index si,
                                        unsigned int index_of_checker)
{
  piece_walk_type pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceFunctionParamListEnd();

  for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
    switch (pp)
    {
      case Queen:
        rear_check_by_promotee(si,index_of_checker,vec_queen_start,vec_queen_end,pp);
        break;

      case Rook:
        rear_check_by_promotee(si,index_of_checker,vec_rook_start,vec_rook_end,pp);
        break;

      case Bishop:
        rear_check_by_promotee(si,index_of_checker,vec_bishop_start,vec_bishop_end,pp);
        break;

      default:
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rear_check_by_rider(slice_index si,
                                unsigned int index_of_checker,
                                vec_index_type start, vec_index_type end,
                                piece_walk_type checker_type)
{
  square const checker_origin = white[index_of_checker].diagram_square;
  Flags const checker_flags = white[index_of_checker].flags;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceWalk(checker_type);
  TraceFunctionParamListEnd();

  for (k = start; k<=end; ++k)
  {
    int const dir = vec[k];
    TraceValue("%u",k);TraceValue("%d\n",dir);
    if (is_square_empty(being_solved.king_square[Black]+dir))
    {
      square rear_pos;
      for (rear_pos = being_solved.king_square[Black]+2*dir; is_square_empty(rear_pos); rear_pos += dir)
        if (intelligent_reserve_officer_moves_from_to(White,
                                                      checker_origin,
                                                      checker_type,
                                                      rear_pos))
        {
          TraceSquare(rear_pos);TraceWalk(being_solved.board[rear_pos]);TraceEOL();
          occupy_square(rear_pos,checker_type,checker_flags);
          remember_to_keep_rider_line_open(rear_pos,being_solved.king_square[Black],-dir,+1);
          generate_front_check(si,rear_pos);
          remember_to_keep_rider_line_open(rear_pos,being_solved.king_square[Black],-dir,-1);
          empty_square(rear_pos);
          intelligent_unreserve();
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void battery(slice_index si)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (index = 1; index<MaxPiece[White]; ++index)
  {
    piece_walk_type const checker_type = white[index].type;

    TraceValue("%u",index);
    TraceSquare(white[index].diagram_square);
    TraceWalk(checker_type);
    TraceEOL();

    white[index].usage = piece_gives_check;

    switch (checker_type)
    {
      case Queen:
        rear_check_by_rider(si,index,vec_queen_start,vec_queen_end,checker_type);
        break;

      case Rook:
        rear_check_by_rider(si,index,vec_rook_start,vec_rook_end,checker_type);
        break;

      case Bishop:
        rear_check_by_rider(si,index,vec_bishop_start,vec_bishop_end,checker_type);
        break;

      case Pawn:
        rear_check_by_promoted_pawn(si,index);
        break;

      default:
        break;
    }

    white[index].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check_by_rider(slice_index si,
                                                 unsigned int checker_index,
                                                 square check_from,
                                                 piece_walk_type rider_type)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceWalk(rider_type);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(White,
                                                white[checker_index].diagram_square,
                                                rider_type,
                                                check_from))
  {
    occupy_square(check_from,rider_type,white[checker_index].flags);
    TraceSquare(check_from);TraceWalk(rider_type);TraceEOL();
    intelligent_guard_flights(si);
    empty_square(check_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check_by_promoted_pawn(slice_index si,
                                                         unsigned int checker_index,
                                                         square check_from)
{
  piece_walk_type pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
    if ((pp==Queen || pp==Rook)
        && intelligent_reserve_promoting_white_pawn_moves_from_to(white[checker_index].diagram_square,
                                                                  pp,
                                                                  check_from))
    {
      occupy_square(check_from,pp,white[checker_index].flags);
      TraceSquare(check_from);TraceWalk(pp);TraceEOL();
      intelligent_guard_flights(si);
      empty_square(check_from);
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_orthogonal_check(slice_index si, int dir_vertical)
{
  square const via_capturer = being_solved.king_square[Black]+dir_vertical;
  square check_from;
  unsigned int checker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  for (check_from = via_capturer+dir_vertical; is_square_empty(check_from); check_from += dir_vertical)
  {
    for (checker_index = 1; checker_index<MaxPiece[White]; ++checker_index)
      if (white[checker_index].usage==piece_is_unused)
      {
        piece_walk_type const checker_type = white[checker_index].type;

        TraceValue("%u",checker_index);
        TraceSquare(white[checker_index].diagram_square);
        TraceWalk(checker_type);
        TraceEOL();

        white[checker_index].usage = piece_gives_check;

        switch (checker_type)
        {
          case Queen:
          case Rook:
            en_passant_orthogonal_check_by_rider(si,
                                                 checker_index,
                                                 check_from,
                                                 checker_type);
            break;

          case Pawn:
            en_passant_orthogonal_check_by_promoted_pawn(si,
                                                         checker_index,
                                                         check_from);
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

static void en_passant_diagonal_check_by_rider(slice_index si,
                                               unsigned int checker_index,
                                               square check_from,
                                               piece_walk_type rider_type,
                                               int dir_vertical)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceWalk(rider_type);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_officer_moves_from_to(White,
                                                white[checker_index].diagram_square,
                                                rider_type,
                                                check_from))
  {
    occupy_square(check_from,rider_type,white[checker_index].flags);
    TraceSquare(check_from);TraceWalk(rider_type);TraceEOL();
    en_passant_orthogonal_check(si,dir_vertical);
    empty_square(check_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check_by_promoted_pawn(slice_index si,
                                                       unsigned int checker_index,
                                                       square check_from,
                                                       int dir_vertical)
{
  square const pawn_origin = white[checker_index].diagram_square;
  Flags const pawn_spec = white[checker_index].flags;
  piece_walk_type pp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",checker_index);
  TraceSquare(check_from);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
    if ((pp==Queen || pp==Bishop)
        && intelligent_reserve_promoting_white_pawn_moves_from_to(pawn_origin,
                                                                  pp,
                                                                  check_from))
    {
      occupy_square(check_from,pp,pawn_spec);
      TraceSquare(check_from);TraceWalk(pp);TraceEOL();
      en_passant_orthogonal_check(si,dir_vertical);
      empty_square(check_from);
      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant_diagonal_check(slice_index si,
                                      square via_capturee,
                                      int dir_vertical)
{
  int const dir_diagonal = via_capturee-being_solved.king_square[Black];
  square check_from;
  unsigned int checker_index;

  TraceFunctionEntry(__func__);
  TraceSquare(via_capturee);
  TraceFunctionParam("%d",dir_diagonal);
  TraceFunctionParamListEnd();

  for (check_from = via_capturee+dir_diagonal; is_square_empty(check_from); check_from += dir_diagonal)
  {
    for (checker_index = 1; checker_index<MaxPiece[White]; ++checker_index)
      if (white[checker_index].usage==piece_is_unused)
      {
        piece_walk_type const checker_type = white[checker_index].type;

        TraceValue("%u",checker_index);
        TraceSquare(white[checker_index].diagram_square);
        TraceWalk(checker_type);
        TraceEOL();

        white[checker_index].usage = piece_gives_check;

        switch (checker_type)
        {
          case Queen:
          case Bishop:
            en_passant_diagonal_check_by_rider(si,
                                               checker_index,
                                               check_from,
                                               checker_type,
                                               dir_vertical);
            break;

          case Pawn:
            en_passant_diagonal_check_by_promoted_pawn(si,
                                                       checker_index,
                                                       check_from,
                                                       dir_vertical);
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

static void en_passant_select_capturee(slice_index si,
                                       square via_capturee,
                                       int dir_vertical)
{
  TraceFunctionEntry(__func__);
  TraceSquare(via_capturee);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  TraceWalk(being_solved.board[via_capturee]);TraceEOL();
  if (is_square_empty(via_capturee))
  {
    square const capturee_origin = via_capturee+2*dir_up;
    unsigned int const index_capturee = enpassant_find_capturee(capturee_origin);
    if (index_capturee!=MaxPiece[Black]
        && black[index_capturee].type==Pawn
        && black[index_capturee].usage==piece_is_unused)
    {
      ++nr_reasons_for_staying_empty[via_capturee];
      black[index_capturee].usage = piece_is_captured;
      en_passant_diagonal_check(si,via_capturee,dir_vertical);
      black[index_capturee].usage = piece_is_unused;
      --nr_reasons_for_staying_empty[via_capturee];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void en_passant(slice_index si,
                       square king_row_start, square king_row_end,
                       int dir_vertical)
{
  TraceFunctionEntry(__func__);
  TraceSquare(king_row_start);
  TraceSquare(king_row_end);
  TraceFunctionParam("%d",dir_vertical);
  TraceFunctionParamListEnd();

  if (king_row_start<=being_solved.king_square[Black] && being_solved.king_square[Black]<=king_row_end)
  {
    square const via_capturer = being_solved.king_square[Black]+dir_vertical;
    if (is_square_empty(via_capturer))
    {
      unsigned int capturer_index;

      ++nr_reasons_for_staying_empty[via_capturer];

      for (capturer_index = 1; capturer_index<MaxPiece[White]; ++capturer_index)
        if (white[capturer_index].type==Pawn
            && white[capturer_index].usage==piece_is_unused
            && intelligent_reserve_double_check_by_enpassant_capture(white[capturer_index].diagram_square,
                                                                     via_capturer))
        {
          white[capturer_index].usage = piece_gives_check;
          en_passant_select_capturee(si,via_capturer+dir_left,dir_vertical);
          en_passant_select_capturee(si,via_capturer+dir_right,dir_vertical);
          white[capturer_index].usage = piece_is_unused;
          intelligent_unreserve();
        }

      --nr_reasons_for_staying_empty[via_capturer];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_generate_doublechecking_moves(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,2,piece_gives_check))
  {
    battery(si);
    en_passant(si,square_a4,square_h4,dir_up);
    en_passant(si,square_a6,square_h6,dir_down);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
