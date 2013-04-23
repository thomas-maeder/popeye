#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "pydata.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "solving/moving_pawn_promotion.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

disturbance_by_rider_type DisturbMateDirRider[4];
int DisturbMateDirKnight[maxsquare+4];
disturbance_by_pawn_type DisturbMateDirPawn[maxsquare+4];

disturbance_by_rider_index_range_type disturbance_by_rider_index_ranges[Bishop-Queen+1];

static void init_disturb_mate_rider_onedir(square target, int dir, unsigned int index)
{
  square s;
  enum
  {
    nr_DisturbMateDirRider = sizeof DisturbMateDirRider / sizeof DisturbMateDirRider[0]
  };

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%u",index);
  TraceFunctionParamListEnd();

  assert(index<nr_DisturbMateDirRider);

  for (s = target+2*dir; e[s]==vide; s += dir)
  {
    DisturbMateDirRider[index][s].dir = -dir;
    DisturbMateDirRider[index][s].target = target;
  }

  for (s = target-2*dir; e[s]==vide; s -= dir)
  {
    DisturbMateDirRider[index][s].dir = dir;
    DisturbMateDirRider[index][s].target = target;
  }

  DisturbMateDirRider[index][target+dir].dir = disturbance_by_rider_uninterceptable;
  DisturbMateDirRider[index][target-dir].dir = disturbance_by_rider_uninterceptable;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_disturb_mate_dir_rider(square const check_from, int dir)
{
  unsigned int i;
  unsigned int disturb_index = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  disturbance_by_rider_index_ranges[Queen-Queen].start = disturb_index;
  disturbance_by_rider_index_ranges[Rook-Queen].start = disturb_index;
  for (i = vec_rook_start; i<=vec_rook_end; ++i)
    if (vec[i]>0)
    {
      square s;
      for (s = check_from; s!=king_square[Black]; s += dir)
        init_disturb_mate_rider_onedir(s,vec[i],disturb_index);
      ++disturb_index;
    }
  disturbance_by_rider_index_ranges[Rook-Queen].end = disturb_index-1;

  disturbance_by_rider_index_ranges[Bishop-Queen].start = disturb_index;
  for (i = vec_bishop_start; i<=vec_bishop_end; ++i)
    if (vec[i]>0)
    {
      square s;
      for (s = check_from; s!=king_square[Black]; s += dir)
        init_disturb_mate_rider_onedir(s,vec[i],disturb_index);
      ++disturb_index;
    }
  disturbance_by_rider_index_ranges[Bishop-Queen].end = disturb_index-1;
  disturbance_by_rider_index_ranges[Queen-Queen].end = disturb_index-1;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_disturb_mate_dir_knight(square const check_from, int dir)
{
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (s = check_from; s!=king_square[Black]; s += dir)
  {
    unsigned int i;
    for (i = vec_knight_start; i<=vec_knight_end; ++i)
      DisturbMateDirKnight[s+vec[i]] = vec[i];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_disturb_mate_dir_pawn(square const check_from, int dir)
{
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (s = check_from+dir; s!=king_square[Black]; s += dir)
  {
    DisturbMateDirPawn[s+dir_up] = disturbance_by_pawn_interception_single;
    if (square_a5<=s && s<=square_h5)
      DisturbMateDirPawn[s+2*dir_up] = disturbance_by_pawn_interception_double;
  }

  /* only now - capture trumps interception */
  DisturbMateDirPawn[check_from+dir_up+dir_left] = disturbance_by_pawn_capture;
  DisturbMateDirPawn[check_from+dir_up+dir_right] = disturbance_by_pawn_capture;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_disturb_mate_dir(square const check_from, int dir)
{
  init_disturb_mate_dir_rider(check_from,dir);
  init_disturb_mate_dir_knight(check_from,dir);
  init_disturb_mate_dir_pawn(check_from,dir);
}

static void fini_disturb_mate_dir(void)
{
  memset(DisturbMateDirRider, 0, sizeof DisturbMateDirRider);
  memset(DisturbMateDirKnight, 0, sizeof DisturbMateDirKnight);
  memset(DisturbMateDirPawn, 0, sizeof DisturbMateDirPawn);
}

static void remember_mating_line(piece checker_type, square const check_from, int delta)
{
  int const diff = king_square[Black]-check_from;
  int const dir = CheckDir[checker_type][diff];

  TraceFunctionEntry(__func__);
  TracePiece(checker_type);
  TraceSquare(check_from);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  remember_to_keep_rider_line_open(check_from,
                                   king_square[Black],
                                   dir,
                                   delta);

  if (delta==+1)
    init_disturb_mate_dir(check_from,dir);
  else
    fini_disturb_mate_dir();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_promoted_rider(unsigned int index_of_checker,
                              PieNam promotee_type,
                              square const check_from)
{
  int const diff = king_square[Black]-check_from;
  int const dir = CheckDir[promotee_type][diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TracePiece(promotee_type);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_checker].diagram_square,
                                                                promotee_type,
                                                                check_from))
  {
    SetPiece(promotee_type,check_from,white[index_of_checker].flags);
    remember_mating_line(promotee_type,check_from,+1);
    intelligent_guard_flights();
    remember_mating_line(promotee_type,check_from,-1);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_promoted_knight(unsigned int index_of_checker, square const check_from)
{
  int const diff = king_square[Black]-check_from;
  int const dir = CheckDir[cb][diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_checker].diagram_square,
                                                                Knight,
                                                                check_from))
  {
    SetPiece(cb,check_from,white[index_of_checker].flags);
    init_disturb_mate_dir(check_from,king_square[Black]-check_from);
    intelligent_guard_flights();
    fini_disturb_mate_dir();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_promoted_pawn(unsigned int index_of_checker, square const check_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_checker,
                                                                check_from))
  {
    PieNam pp;
    for (pp = pieces_pawns_promotee_chain[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_chain[pieces_pawns_promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
        case Rook:
        case Bishop:
          by_promoted_rider(index_of_checker,pp,check_from);
          break;

        case Knight:
          by_promoted_knight(index_of_checker,check_from);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_unpromoted_pawn(unsigned int index_of_checker, square const check_from)
{
  square const checker_from = white[index_of_checker].diagram_square;
  Flags const checker_flags = white[index_of_checker].flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (check_from>=square_a2 && check_from<=square_h7
      && GuardDir[Pawn-Pawn][check_from].dir==guard_dir_check_uninterceptable
      && intelligent_reserve_white_pawn_moves_from_to_checking(checker_from,check_from))
  {
    SetPiece(pb,check_from,checker_flags);
    init_disturb_mate_dir(check_from,king_square[Black]-check_from);
    intelligent_guard_flights();
    fini_disturb_mate_dir();
    e[check_from] = vide;
    spec[check_from] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_rider(unsigned int index_of_checker, square const check_from)
{
  piece const checker_type = white[index_of_checker].type;
  Flags const checker_flags = white[index_of_checker].flags;
  int const diff = king_square[Black]-check_from;
  int const dir = CheckDir[checker_type][diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_white_officer_moves_from_to_checking(white[index_of_checker].diagram_square,
                                                                  checker_type,
                                                                  check_from))
  {
    SetPiece(checker_type,check_from,checker_flags);
    remember_mating_line(checker_type,check_from,+1);
    intelligent_guard_flights();
    remember_mating_line(checker_type,check_from,-1);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_knight(unsigned int index_of_checker, square const check_from)
{
  int const diff = king_square[Black]-check_from;
  int const dir = CheckDir[cb][diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_white_officer_moves_from_to_checking(white[index_of_checker].diagram_square,
                                                                  cb,
                                                                  check_from))
  {
    SetPiece(cb,check_from,white[index_of_checker].flags);
    init_disturb_mate_dir(check_from,king_square[Black]-check_from);
    intelligent_guard_flights();
    fini_disturb_mate_dir();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_generate_checking_moves(void)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    for (index = 1; index<MaxPiece[White]; ++index)
    {
      square const *bnp;

      white[index].usage = piece_gives_check;

      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (e[*bnp]==vide)
        {
          switch (white[index].type)
          {
            case db:
            case tb:
            case fb:
              by_rider(index,*bnp);
              break;

            case cb:
              by_knight(index,*bnp);
              break;

            case pb:
              by_unpromoted_pawn(index,*bnp);
              by_promoted_pawn(index,*bnp);
              break;

            default:
              assert(0);
              break;
          }

          e[*bnp] = vide;
          spec[*bnp] = EmptySpec;
        }

      white[index].usage = piece_is_unused;
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
