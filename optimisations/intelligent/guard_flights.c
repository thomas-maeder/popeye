#include "optimisations/intelligent/guard_flights.h"
#include "pydata.h"
#include "pyint.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/block_flights.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/intercept_check_from_guard.h"
#include "options/maxsolutions/maxsolutions.h"
#include "platform/maxtime.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

unsigned int index_of_next_guarding_piece = 1;

/* Does a white pawn guard a flight
 * @param from where might the pawn guard?
 * @return a flight guarded by the pawn; initsquare if it doesn't guard
 */
static square white_pawn_guards_flight(square from)
{
  int i;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  e[king_square[Black]]= vide;

  for (i = 8; i!=0; --i)
    if (e[king_square[Black]+vec[i]]!=obs
        && white_pawn_attacks_king_region(from,vec[i]))
    {
      result = king_square[Black]+vec[i];
      break;
    }

  e[king_square[Black]]= roin;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Does the white king guard a flight
 * @param from where might the king guard?
 * @return true iff the king guards a flight
 */
static boolean white_king_guards_flight(square from)
{
  int const diff = move_diff_code[abs(king_square[Black]-from)];
  boolean const result = diff>3 && diff<=8;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Does a a white officer guard any flight?
 * @param officer_type type of officer
 * @param from from where might the officer guard
 * @return a flight guarded by the officer; initsquare if it doesn't guard
 */
static square white_officer_guards_flight(piece officer_type, square from)
{
  int i;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TracePiece(officer_type);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  e[king_square[Black]]= vide;

  for (i = 8; i!=0; --i)
    if (e[king_square[Black]+vec[i]]!=obs
        && officer_guards(king_square[Black]+vec[i],officer_type,from))
    {
      result = king_square[Black]+vec[i];
      break;
    }

  e[king_square[Black]]= roin;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* find out whether to remember to keep a guard line open, and do it
 * @param from start of line
 * @param to end of line
 * @param type type of rider
 * @param delta [+-1] remember resp. forget to keep the line open
 */
static void remember_to_keep_guard_line_open(square from, square to,
                                             int delta)
{
  int const dir = CheckDir[Queen][to-from];
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%d",type);
  TraceFunctionParamListEnd();

  TraceValue("%d\n",dir);

  /* the guard line only needs to be kept open up to the flight closest to
   * from; e.g. reset to to c1 with from:a1 to:e1 king_square[Black]:d2
   */
  for (s = to-dir; s!=from && move_diff_code[abs(king_square[Black]-s)]<=2; s -= dir)
    to = s;

  remember_to_keep_rider_line_open(from,to,dir,delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* fix the white king on its diagram square
 */
static void fix_white_king_on_diagram_square(void)
{
  square const king_diagram_square = white[index_of_king].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (e[king_diagram_square]==vide
      && nr_reasons_for_staying_empty[king_diagram_square]==0)
  {
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;
    intelligent_place_white_king(king_diagram_square,
                                 &intelligent_find_and_block_flights);
    white[index_of_king].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* continue after guarding
 */
static void guarding_done(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused
      && white[index_of_king].diagram_square!=square_e1
      && intelligent_get_nr_remaining_moves(White)==0)
    fix_white_king_on_diagram_square();
  else
    intelligent_find_and_block_flights();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* place a (promoted or original) queen in opposition to the black king
 * @param index_of_queen identifies the queen
 * @param guard_from from what square should the queen guard
 */
static void place_queen_opposition(unsigned int index_of_queen,
                                   square guard_from)
{
  square const to_be_intercepted = (king_square[Black]+guard_from)/2;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_queen);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  SetPiece(db,guard_from,white[index_of_queen].flags);
  if (e[to_be_intercepted]==vide)
  {
    assert(nr_reasons_for_staying_empty[to_be_intercepted]==0);
    intercept_check_on_guarded_square(index_of_queen,to_be_intercepted);
    intelligent_intercept_orthogonal_check_by_pin(to_be_intercepted);
    e[to_be_intercepted]= vide;
    spec[to_be_intercepted]= EmptySpec;
  }
  else
    /* there is already a guard between queen and king */
    intelligent_continue_guarding_flights();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* place a (promoted or original) rider
 * @param index_of_rider identifies the rider
 * @param rider_type type of rider
 * @param guard_from from what square should the rider guard
 */
static void place_rider(unsigned int index_of_rider,
                        piece rider_type,
                        square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_rider);
  TracePiece(rider_type);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  {
    square const guarded = white_officer_guards_flight(rider_type,guard_from);
    if (guarded!=initsquare)
    {
      square const to_be_intercepted = where_to_intercept_check_from_guard(rider_type,
                                                                           guard_from);
      SetPiece(rider_type,guard_from,white[index_of_rider].flags);
      remember_to_keep_guard_line_open(guard_from,guarded,+1);
      if (to_be_intercepted==initsquare)
        intelligent_continue_guarding_flights();
      else
      {
        assert(nr_reasons_for_staying_empty[to_be_intercepted]==0);
        intercept_check_on_guarded_square(index_of_rider,to_be_intercepted);
      }
      remember_to_keep_guard_line_open(guard_from,guarded,-1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* place a (promoted or original) knight
 * @param index_of_knight identifies the knight
 * @param guard_from from what square should the knight guard
 */
static void place_knight(unsigned int index_of_knight, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_knight);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (white_officer_guards_flight(cb,guard_from)!=initsquare)
  {
    SetPiece(cb,guard_from,white[index_of_knight].flags);
    intelligent_continue_guarding_flights();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with an unpromoted pawn
 * @param index_of_pawn identifies the pawn
 * @param guard_from from what square should the pawn guard
 */
static void unpromoted_pawn(unsigned int index_of_pawn, square guard_from)
{
  Flags const pawn_flags = white[index_of_pawn].flags;
  square const starts_from = white[index_of_pawn].diagram_square;

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_pawn);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (square_a2<=guard_from && guard_from<=square_h7
      && !white_pawn_attacks_king_region(guard_from,0)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(starts_from,
                                                                   guard_from))
  {
    square const guarded = white_pawn_guards_flight(guard_from);
    if (guarded!=initsquare)
    {
      SetPiece(pb,guard_from,pawn_flags);
      intelligent_continue_guarding_flights();
      e[guard_from] = vide;
      spec[guard_from] = EmptySpec;
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted queen
 * @param index_of_queen identifies the queen
 * @param guard_from from what square should the queen guard
 */
static void promoted_queen(unsigned int index_of_queen, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_queen);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Qb2 */
    case 1+1: /* e.g. Ka2 Qb3 */
      /* uninterceptable check */
      break;

    case 4+0: /* e.g. Ka2 Qc2 */
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_queen].diagram_square,
                                                                 db,
                                                                 guard_from))
      {
        place_queen_opposition(index_of_queen,guard_from);
        intelligent_unreserve();
      }
      break;

    case 4+1: /* e.g. Ka2 Qc3 - no lines to be kept open */
    case 9+1: /* e.g. Ka2 Qd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_queen].diagram_square,
                                                                 db,
                                                                 guard_from))
      {
        SetPiece(db,guard_from,white[index_of_queen].flags);
        intelligent_continue_guarding_flights();
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_queen].diagram_square,
                                                                 db,
                                                                 guard_from))
      {
        place_rider(index_of_queen,db,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a rook
 * @param index_of_rook identifies the rook
 * @param guard_from from what square should the rook guard
 */
static void promoted_rook(unsigned int index_of_rook, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_rook);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Rb2 */
      /* uninterceptable check */
      break;

    case 1+1: /* e.g. Kc2 Rd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_rook].diagram_square,
                                                                 tb,
                                                                 guard_from))
      {
        SetPiece(tb,guard_from,white[index_of_rook].flags);
        intelligent_continue_guarding_flights();
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_rook].diagram_square,
                                                                 tb,
                                                                 guard_from))
      {
        place_rider(index_of_rook,tb,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted bishop
 * @param index_of_bishop identifies the bishop
 * @param guard_from from what square should the bishop guard
 */
static void promoted_bishop(unsigned int index_of_bishop, square guard_from)
{
  int const diff = move_diff_code[abs(king_square[Black]-guard_from)];

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_bishop);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (diff==1+1) /* e.g. Ka2 Bb3 */
  {
    /* uninterceptable check */
  }
  else if (intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_bishop].diagram_square,
                                                                  fb,
                                                                  guard_from))
  {
    if (diff<=2+2)
    {
      SetPiece(fb,guard_from,white[index_of_bishop].flags);
      intelligent_continue_guarding_flights();
    }
    else
      place_rider(index_of_bishop,fb,guard_from);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted knight
 * @param index_of_knight identifies the knight
 * @param guard_from from what square should the knight guard
 */
static void promoted_knight(unsigned int index_of_knight, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_knight);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (CheckDir[Knight][king_square[Black]-guard_from]==0
      && intelligent_reserve_promoting_white_pawn_moves_from_to(white[index_of_knight].diagram_square,
                                                                cb,
                                                                guard_from))
  {
    place_knight(index_of_knight,guard_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a promoted pawn
 * @param index_of_pawn identifies the pawn
 * @param guard_from from what square should the promotee guard
 */
static void promoted_pawn(unsigned int index_of_pawn, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_pawn);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(index_of_pawn,
                                                                guard_from))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      switch (pp)
      {
        case db:
          promoted_queen(index_of_pawn,guard_from);
          break;

        case tb:
          promoted_rook(index_of_pawn,guard_from);
          break;

        case fb:
          promoted_bishop(index_of_pawn,guard_from);
          break;

        case cb:
          promoted_knight(index_of_pawn,guard_from);
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a queen
 * @param index_of_queen identifies the queen
 * @param guard_from from what square should the queen guard
 */
static void queen(unsigned int index_of_queen, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_queen);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Qb2 */
    case 1+1: /* e.g. Ka2 Qb3 */
      /* uninterceptable check */
      break;

    case 4+0: /* e.g. Ka2 Qc2 */
      if (intelligent_reserve_officer_moves_from_to(white[index_of_queen].diagram_square,
                                                    db,
                                                    guard_from))
      {
        place_queen_opposition(index_of_queen,guard_from);
        intelligent_unreserve();
      }
      break;

    case 4+1: /* e.g. Ka2 Qc3 - no lines to be kept open */
    case 9+1: /* e.g. Ka2 Qd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_officer_moves_from_to(white[index_of_queen].diagram_square,
                                                    db,
                                                    guard_from))
      {
        SetPiece(db,guard_from,white[index_of_queen].flags);
        intelligent_continue_guarding_flights();
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(white[index_of_queen].diagram_square,
                                                    db,
                                                    guard_from))
      {
        place_rider(index_of_queen,db,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a rook
 * @param index_of_rook identifies the rook
 * @param guard_from from what square should the rook guard
 */
static void rook(unsigned int index_of_rook, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_rook);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  switch (move_diff_code[abs(king_square[Black]-guard_from)])
  {
    case 1+0: /* e.g. Ka2 Rb2 */
      /* uninterceptable check */
      break;

    case 1+1: /* e.g. Kc2 Rd3 - 2 guard lines, only 1 needs to remain open */
      if (intelligent_reserve_officer_moves_from_to(white[index_of_rook].diagram_square,
                                                    tb,
                                                    guard_from))
      {
        SetPiece(tb,guard_from,white[index_of_rook].flags);
        intelligent_continue_guarding_flights();
        intelligent_unreserve();
      }
      break;

    default:
      if (intelligent_reserve_officer_moves_from_to(white[index_of_rook].diagram_square,
                                                    tb,
                                                    guard_from))
      {
        place_rider(index_of_rook,tb,guard_from);
        intelligent_unreserve();
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a bishop
 * @param index_of_rider identifies the bishop
 * @param guard_from from what square should the bishop guard
 */
static void bishop(unsigned int index_of_bishop, square guard_from)
{
  int const diff = move_diff_code[abs(king_square[Black]-guard_from)];

  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_bishop);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (diff==1+1) /* e.g. Ka2 Bb3 */
  {
    /* uninterceptable check */
  }
  else if (intelligent_reserve_officer_moves_from_to(white[index_of_bishop].diagram_square,
                                                     fb,
                                                     guard_from))
  {
    if (diff<=2+2)
    {
      SetPiece(fb,guard_from,white[index_of_bishop].flags);
      intelligent_continue_guarding_flights();
    }
    else
      place_rider(index_of_bishop,fb,guard_from);

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with a white knight
 * @param index_of_knight identifies the knight
 * @param guard_from from what square should the knight guard
 */
static void knight(unsigned int index_of_knight, square guard_from)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",index_of_knight);
  TraceSquare(guard_from);
  TraceFunctionParamListEnd();

  if (CheckDir[Knight][king_square[Black]-guard_from]==0
      && intelligent_reserve_officer_moves_from_to(white[index_of_knight].diagram_square,
                                                   cb,
                                                   guard_from))
  {
    place_knight(index_of_knight,guard_from);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard more king flights */
void intelligent_continue_guarding_flights(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",index_of_next_guarding_piece);

  if (!max_nr_solutions_found_in_phase() && !hasMaxtimeElapsed())
  {
    if (intelligent_reserve_masses(White,1))
    {
      unsigned int save_next = index_of_next_guarding_piece;
      unsigned int index_of_current_guarding_piece;
      TraceValue("%u\n",MaxPiece[White]);
      for (index_of_current_guarding_piece = index_of_next_guarding_piece;
           index_of_current_guarding_piece<MaxPiece[White];
           ++index_of_current_guarding_piece)
      {
        TraceValue("%u",index_of_current_guarding_piece);
        TraceEnumerator(piece_usage,white[index_of_current_guarding_piece].usage,"\n");
        if (white[index_of_current_guarding_piece].usage==piece_is_unused)
        {
          square const *bnp;
          white[index_of_current_guarding_piece].usage = piece_guards;

          index_of_next_guarding_piece = index_of_current_guarding_piece+1;

          for (bnp = boardnum; *bnp!=initsquare; bnp++)
            if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0)
            {
              switch (white[index_of_current_guarding_piece].type)
              {
                case db:
                  queen(index_of_current_guarding_piece,*bnp);
                  break;

                case tb:
                  rook(index_of_current_guarding_piece,*bnp);
                  break;

                case fb:
                  bishop(index_of_current_guarding_piece,*bnp);
                  break;

                case cb:
                  knight(index_of_current_guarding_piece,*bnp);
                  break;

                case pb:
                  unpromoted_pawn(index_of_current_guarding_piece,*bnp);
                  promoted_pawn(index_of_current_guarding_piece,*bnp);
                  break;

                default:
                  assert(0);
                  break;
              }

              e[*bnp] = vide;
              spec[*bnp] = EmptySpec;
            }

          white[index_of_current_guarding_piece].usage = piece_is_unused;
        }
      }

      index_of_next_guarding_piece = save_next;

      intelligent_unreserve();
    }

    guarding_done();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard king flights with the white king */
static void king()
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    square const comes_from = white[index_of_king].diagram_square;
    square const *bnp;

    white[index_of_king].usage = piece_guards;

    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
      if (e[*bnp]==vide
          && nr_reasons_for_staying_empty[*bnp]==0
          && white_king_guards_flight(*bnp)
          && intelligent_reserve_white_king_moves_from_to(comes_from,*bnp))
      {
        king_square[White]= *bnp;
        SetPiece(roib,*bnp,white[index_of_king].flags);
        intelligent_continue_guarding_flights();
        e[*bnp] = vide;
        spec[*bnp] = EmptySpec;
        intelligent_unreserve();
      }

    king_square[White] = initsquare;
    white[index_of_king].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* guard the king flights */
void intelligent_guard_flights(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!max_nr_solutions_found_in_phase()
      && !hasMaxtimeElapsed())
  {
    king();

    TraceText("try not using white king for guarding\n");
    intelligent_continue_guarding_flights();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
