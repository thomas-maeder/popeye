#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/orthodox_check_directions.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
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

  for (s = target+2*dir; is_square_empty(s); s += dir)
  {
    DisturbMateDirRider[index][s].dir = -dir;
    DisturbMateDirRider[index][s].target = target;
  }

  for (s = target-2*dir; is_square_empty(s); s -= dir)
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
  vec_index_type i;
  unsigned int disturb_index = 0;
  Flags const mask = BIT(Black)|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  disturbance_by_rider_index_ranges[Queen-Queen].start = disturb_index;
  disturbance_by_rider_index_ranges[Rook-Queen].start = disturb_index;
  for (i = vec_rook_start; i<=vec_rook_end; ++i)
    if (vec[i]>0)
    {
      square s;
      for (s = check_from; !TSTFULLFLAGMASK(being_solved.spec[s],mask); s += dir)
        init_disturb_mate_rider_onedir(s,vec[i],disturb_index);
      ++disturb_index;
    }
  disturbance_by_rider_index_ranges[Rook-Queen].end = disturb_index-1;

  disturbance_by_rider_index_ranges[Bishop-Queen].start = disturb_index;
  for (i = vec_bishop_start; i<=vec_bishop_end; ++i)
    if (vec[i]>0)
    {
      square s;
      for (s = check_from; !TSTFULLFLAGMASK(being_solved.spec[s],mask); s += dir)
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
  Flags const mask = BIT(Black)|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (s = check_from; !TSTFULLFLAGMASK(being_solved.spec[s],mask); s += dir)
  {
    vec_index_type i;
    for (i = vec_knight_start; i<=vec_knight_end; ++i)
      DisturbMateDirKnight[s+vec[i]] = vec[i];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_disturb_mate_dir_pawn(square const check_from, int dir)
{
  square s;
  Flags const mask = BIT(Black)|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (s = check_from+dir; !TSTFULLFLAGMASK(being_solved.spec[s],mask); s += dir)
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

static void remember_mating_line(piece_walk_type checker_type, square const check_from, int delta)
{
  int const diff = being_solved.king_square[Black]-check_from;
  int const dir = CheckDir(checker_type)[diff];

  TraceFunctionEntry(__func__);
  TraceWalk(checker_type);
  TraceSquare(check_from);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  remember_to_keep_rider_line_open(check_from,
                                   being_solved.king_square[Black],
                                   dir,
                                   delta);

  if (delta==+1)
    init_disturb_mate_dir(check_from,dir);
  else
    fini_disturb_mate_dir();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_promoted_rider(slice_index si,
                              unsigned int index_of_checker,
                              piece_walk_type promotee_type,
                              square const check_from)
{
  int const diff = being_solved.king_square[Black]-check_from;
  int const dir = CheckDir(promotee_type)[diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceWalk(promotee_type);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_checker].diagram_square,
                                                                promotee_type,
                                                                check_from))
  {
    occupy_square(check_from,promotee_type,white[index_of_checker].flags);
    remember_mating_line(promotee_type,check_from,+1);
    pipe_solve_delegate(si);
    remember_mating_line(promotee_type,check_from,-1);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_promoted_knight(slice_index si,
                               unsigned int index_of_checker,
                               square const check_from)
{
  int const diff = being_solved.king_square[Black]-check_from;
  int const dir = CheckDir(Knight)[diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_checker].diagram_square,
                                                                Knight,
                                                                check_from))
  {
    occupy_square(check_from,Knight,white[index_of_checker].flags);
    init_disturb_mate_dir(check_from,being_solved.king_square[Black]-check_from);
    pipe_solve_delegate(si);
    fini_disturb_mate_dir();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_promoted_pawn(slice_index si,
                             unsigned int index_of_checker,
                             square const check_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_checker,
                                                                check_from))
  {
    piece_walk_type pp;
    for (pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]; pp!=Empty; pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp])
      switch (pp)
      {
        case Queen:
        case Rook:
        case Bishop:
          by_promoted_rider(si,index_of_checker,pp,check_from);
          break;

        case Knight:
          by_promoted_knight(si,index_of_checker,check_from);
          break;

        case Dummy:
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_unpromoted_pawn(slice_index si,
                               unsigned int index_of_checker,
                               square const check_from)
{
  square const checker_from = white[index_of_checker].diagram_square;
  Flags const checker_flags = white[index_of_checker].flags;
  SquareFlags const prom_square = BIT(WhPromSq)|BIT(BlPromSq);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (!TSTFLAGMASK(sq_spec(check_from),prom_square)
      && GuardDir[Pawn][check_from].dir==guard_dir_check_uninterceptable
      && intelligent_reserve_white_pawn_moves_from_to_checking(checker_from,check_from))
  {
    occupy_square(check_from,Pawn,checker_flags);
    init_disturb_mate_dir(check_from,being_solved.king_square[Black]-check_from);
    pipe_solve_delegate(si);
    fini_disturb_mate_dir();
    empty_square(check_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_rider(slice_index si,
                     unsigned int index_of_checker,
                     square const check_from)
{
  piece_walk_type const checker_type = white[index_of_checker].type;
  Flags const checker_flags = white[index_of_checker].flags;
  int const diff = being_solved.king_square[Black]-check_from;
  int const dir = CheckDir(checker_type)[diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_white_officer_moves_from_to_checking(white[index_of_checker].diagram_square,
                                                                  checker_type,
                                                                  check_from))
  {
    occupy_square(check_from,checker_type,checker_flags);
    remember_mating_line(checker_type,check_from,+1);
    pipe_solve_delegate(si);
    remember_mating_line(checker_type,check_from,-1);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void by_knight(slice_index si,
                      unsigned int index_of_checker,
                      square const check_from)
{
  int const diff = being_solved.king_square[Black]-check_from;
  int const dir = CheckDir(Knight)[diff];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index_of_checker);
  TraceSquare(check_from);
  TraceFunctionParamListEnd();

  if (dir!=0
      && intelligent_reserve_white_officer_moves_from_to_checking(white[index_of_checker].diagram_square,
                                                                  Knight,
                                                                  check_from))
  {
    occupy_square(check_from,Knight,white[index_of_checker].flags);
    init_disturb_mate_dir(check_from,being_solved.king_square[Black]-check_from);
    pipe_solve_delegate(si);
    fini_disturb_mate_dir();
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_generate_checking_moves(slice_index si)
{
  unsigned int index;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1,piece_gives_check))
  {
    for (index = 1; index<MaxPiece[White]; ++index)
    {
      square const *bnp;

      white[index].usage = piece_gives_check;

      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (is_square_empty(*bnp))
        {
          switch (white[index].type)
          {
            case Queen:
            case Rook:
            case Bishop:
              by_rider(si,index,*bnp);
              break;

            case Knight:
              by_knight(si,index,*bnp);
              break;

            case Pawn:
              by_unpromoted_pawn(si,index,*bnp);
              by_promoted_pawn(si,index,*bnp);
              break;

            case Dummy:
              break;

            default:
              assert(0);
              break;
          }

          empty_square(*bnp);
        }

      white[index].usage = piece_is_unused;
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
