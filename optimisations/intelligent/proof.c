#include "optimisations/intelligent/proof.h"
#include "position/pieceid.h"
#include "conditions/conditions.h"
#include "conditions/circe/circe.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "stipulation/stipulation.h"
#include "solving/proofgames.h"
#include "solving/castling.h"
#include "solving/pipe.h"
#include "solving/has_solution_type.enum.h"
#include "stipulation/pipe.h"
#include "optimisations/observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/moves_left.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "position/effects/piece_movement.h"

#include "debugging/assert.h"
#include "debugging/trace.h"

#include <stdlib.h>

static boolean ProofFairy;

static int KingMoves[nr_sides][maxsquare];

static stip_length_type current_length;

static boolean BlockedQueenBishop[nr_sides];
static boolean BlockedKingBishop[nr_sides];
static boolean BlockedQueen[nr_sides];
static boolean CapturedQueenBishop[nr_sides];
static boolean CapturedKingBishop[nr_sides];
static boolean CapturedQueen[nr_sides];

typedef struct
{
    int     Nbr;
    square  sq[16];
} PieceList;

typedef struct
{
    int     Nbr;
    stip_length_type moves[16];
    stip_length_type captures[16];
    int     id[16];
} PieceList2;

static PieceList ProofPawns[nr_sides];
static PieceList PawnsToBeArranged[nr_sides];
static PieceList ProofOfficers[nr_sides];
static PieceList PiecesToBeArranged[nr_sides];

static unsigned int ProofNbrPieces[nr_sides];

slice_type proof_make_goal_reachable_type(void)
{
  slice_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ProofFairy = (change_moving_piece
                || CondFlag[black_oscillatingKs] || CondFlag[white_oscillatingKs]
                || CondFlag[republican]
                || CondFlag[circe]
                || CondFlag[sentinelles]
                || CondFlag[anticirce]
                || CondFlag[singlebox]
                || CondFlag[blroyalsq] || CondFlag[whroyalsq]
                || TSTFLAG(some_pieces_flags,ColourChange)
                || CondFlag[actrevolving]
                || CondFlag[arc]
                || CondFlag[annan]
                || CondFlag[pointreflection]
                || CondFlag[glasgow]
                || CondFlag[takemake] || CondFlag[maketake]
                || CondFlag[messigny]
                || CondFlag[mars]
                || CondFlag[castlingchess]
                || CondFlag[rokagogo]
                || CondFlag[platzwechselrochade]
                || CondFlag[football]
                || CondFlag[kobulkings]
                || CondFlag[wormholes]
                || CondFlag[dynasty]
                || CondFlag[whsupertrans_king] || CondFlag[blsupertrans_king]
                || CondFlag[lostpieces]
                || CondFlag[breton]);

  /* TODO these can't possibly be the only elements that doesn't
   * allow any optimisation at all.
   */
  if (piece_walk_may_exist_fairy
      || (some_pieces_flags&~PieceIdMask&~BIT(Royal))
      || CondFlag[masand]
      || (CondFlag[circe]
          && circe_variant.on_occupied_rebirth_square
             ==circe_on_occupied_rebirth_square_assassinate))
    result  = no_slice_type;
  else if (ProofFairy)
    result = STGoalReachableGuardFilterProofFairy;
  else
    result = STGoalReachableGuardFilterProof;

  TraceValue("%u",result);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceEnumerator(slice_type,result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STIntelligentProof slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_proof(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentProof);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static int KingMovesNeeded(Side side)
{
  int   needed;
  int   cast;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  TraceSquare(being_solved.king_square[side]);
  TraceEOL();

  if (being_solved.king_square[side]==initsquare)
    /* no king in play, or king can be created by promotion
     * -> no optimisation possible */
    needed = 0;
  else
  {
    needed = KingMoves[side][being_solved.king_square[side]];

    if (TSTCASTLINGFLAGMASK(side,k_cancastle))
    {
      square const square_base = side==White ? square_a1 : square_a8;

      if (TSTCASTLINGFLAGMASK(side,ra_cancastle))
      {
        /* wh long castling */
        /* KingMoves[White] is the number of moves the wh king still
           needs after castling. It takes 1 move to castle, but we
           might save a rook move.
        */
        cast = KingMoves[side][square_base+file_c];
        if (cast<needed)
          needed= cast;
      }
      if (TSTCASTLINGFLAGMASK(side,rh_cancastle))
      {
        /* wh short castling */
        /* KingMoves[White] is the number of moves the wh king still
           needs after castling. It takes 1 move to castle, but we
           might save a rook move
        */
        cast = KingMoves[side][square_base+file_g];
        if (cast<needed)
          needed= cast;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",needed);
  TraceFunctionResultEnd();
  return needed;
}

static void PawnMovesFromTo(Side side,
                            square from, square to,
                            stip_length_type *moves,
                            stip_length_type *captures,
                            stip_length_type captallowed)
{
  SquareFlags const pawn_doublestep_square = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
  int rank_from = from/onerow;
  int rank_to = to/onerow;

  if (side==Black)
  {
    rank_from = nr_rows_on_board-rank_from;
    rank_to = nr_rows_on_board-rank_to;
  }

  /* calculate number of captures */
  *captures= abs(to%onerow-from%onerow);

  /* calculate number of moves */
  if (rank_to<rank_from)
    *moves = current_length;
  else
  {
    *moves = rank_to-rank_from;
    if (*moves<*captures || *captures>captallowed)
      *moves = current_length;
    else if (TSTFLAG(sq_spec[from],pawn_doublestep_square) && *captures<*moves-1)
      /* double step possible */
      --*moves;
  }
}

static stip_length_type PawnMovesNeeded(Side side, square sq)
{
  SquareFlags const double_step = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

  /* The first time ProofWhPawnMovesNeeded is called the following
     test is always false. It has already been checked in
     Impossible. But we need it here for the recursion.
  */
  if ((get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],side))
      && !(proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],side)))
    return 0;

  else if (TSTFLAG(sq_spec[sq],double_step))
    /* there is no pawn at all that can enter this square */
    return current_length;

  else
  {
    stip_length_type MovesNeeded;
    stip_length_type MovesNeeded1;
    numvec const dir_backward = side==White ? dir_down : dir_up;

    /* double step */
    square const sq_double_step_departure = sq+2*dir_backward;
    if (TSTFLAG(sq_spec[sq_double_step_departure],double_step)
        && (get_walk_of_piece_on_square(sq_double_step_departure)==Pawn && TSTFLAG(being_solved.spec[sq_double_step_departure],side))
        && !(proofgames_target_position.board[sq_double_step_departure]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_double_step_departure],side)))
      return 1;

    if (!is_square_blocked(sq+dir_backward+dir_right))
    {
      MovesNeeded = PawnMovesNeeded(side,sq+dir_backward+dir_right);
      if (MovesNeeded==0)
        /* There is a free pawn on sq+dir_down+dir_right
        ** so it takes just 1 move */
        return 1;
    }
    else
      MovesNeeded = current_length;

    if (!is_square_blocked(sq+dir_backward+dir_left))
    {
      MovesNeeded1 = PawnMovesNeeded(side,sq+dir_backward+dir_left);
      if (MovesNeeded1==0)
        /* There is a free pawn on sq+dir_down+dir_left
        ** so it takes just 1 move */
        return 1;
      else if (MovesNeeded1 < MovesNeeded)
        MovesNeeded = MovesNeeded1;
    }

    MovesNeeded1 = PawnMovesNeeded(side,sq+dir_backward);
    if (MovesNeeded1<MovesNeeded)
      MovesNeeded = MovesNeeded1;

    return MovesNeeded+1;
  }
}

static boolean blocked_by_pawn(square sq)
{
  return (((get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],White))
           && (proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],White))
           && PawnMovesNeeded(White,sq)>=current_length)
          || ((get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],Black))
              && (proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],Black))
              && PawnMovesNeeded(Black,sq)>=current_length));
}

static void OfficerMovesFromTo(piece_walk_type p,
                               square from, square to,
                               stip_length_type *moves)
{
  numvec dir;
  int    sqdiff= from-to;

  if (sqdiff==0)
  {
    *moves= 0;
    return;
  }
  switch (p)
  {
    case Knight:
      *moves= minimum_number_knight_moves[abs(sqdiff)];
      if (*moves > 1)
      {
        square    sqi, sqj;
        stip_length_type testmov;
        stip_length_type testmin = current_length;
        vec_index_type i;
        for (i= vec_knight_start; i<=vec_knight_end; ++i)
        {
          sqi= from+vec[i];
          if (!blocked_by_pawn(sqi) && !is_square_blocked(sqi))
          {
            vec_index_type j;
            for (j= vec_knight_start; j<=vec_knight_end; j++)
            {
              sqj= to+vec[j];
              if (!blocked_by_pawn(sqj) && !is_square_blocked(sqj))
              {
                testmov= minimum_number_knight_moves[abs(sqi-sqj)]+2;
                if (testmov == *moves)
                  return;
                if (testmov < testmin)
                  testmin= testmov;
              }
            }
          }
        }
        *moves= testmin;
      }
      break;

    case Bishop:
      if (SquareCol(from) != SquareCol(to))
        *moves= current_length;
      else
      {
        dir= CheckDir[Bishop][sqdiff];
        if (dir)
        {
          do {
            from-= dir;
          } while (to != from && !blocked_by_pawn(from));
          *moves= to == from ? 1 : 3;
        }
        else
          *moves= 2;
      }
      break;

    case Rook:
      dir= CheckDir[Rook][sqdiff];
      if (dir)
      {
        do {
          from-= dir;
        } while (to != from && !blocked_by_pawn(from));
        *moves= to == from ? 1 : 3;
      }
      else
        *moves= 2;
      break;

    case Queen:
      dir= CheckDir[Queen][sqdiff];
      if (dir)
      {
        do {
          from-= dir;
        } while (to != from && !blocked_by_pawn(from));
        *moves= to == from ? 1 : 2;
      }
      else
        *moves= 2;
      break;

    default:
      fprintf(stderr,"error in %s - piece:",__func__);
      WriteWalk(&output_plaintext_engine,stderr,p);
      fputs("\n",stderr);
      break;
  }
}

static void PromPieceMovesFromTo(Side side,
                                 square from, square to,
                                 stip_length_type *moves,
                                 stip_length_type *captures,
                                 stip_length_type captallowed)
{
  stip_length_type i;
  stip_length_type mov1;
  stip_length_type mov2;
  stip_length_type cap1;
  unsigned int const from_file = from%nr_files_on_board;
  square const promsq_a = side==White ? square_a8 : square_a1;
  square const cenpromsq = promsq_a+from_file;

  *moves= current_length;

  PawnMovesFromTo(side,from, cenpromsq, &mov1, &cap1, captallowed);
  OfficerMovesFromTo(proofgames_target_position.board[to], cenpromsq, to, &mov2);
  if (mov1+mov2 < *moves)
    *moves= mov1+mov2;

  for (i = 1; i<=captallowed; ++i)
  {
    if (from_file+i <= file_h)
    {
      /* got out of range sometimes ! */
      PawnMovesFromTo(side,from, cenpromsq+i, &mov1, &cap1, captallowed);
      OfficerMovesFromTo(proofgames_target_position.board[to], cenpromsq+i, to, &mov2);
      if (mov1+mov2 < *moves)
        *moves= mov1+mov2;
    }
    if (from_file>=file_a+i)
    {
      /* got out of range sometimes ! */
      PawnMovesFromTo(side,from, cenpromsq-i, &mov1, &cap1, captallowed);
      OfficerMovesFromTo(proofgames_target_position.board[to], cenpromsq-i, to, &mov2);
      if (mov1+mov2 < *moves)
        *moves= mov1+mov2;
    }
  }

  /* We cannot say for sure how many captures we really need.
  ** We may need 3 moves and 1 capture or 2 moves and 2 captures.
  ** Therefore zero is returned. */
  *captures= 0;
}

static void PieceMovesFromTo(Side side,
                             square from, square to,
                             stip_length_type *moves,
                             stip_length_type *captures,
                             stip_length_type captallowed,
                             int captrequ)
{
  piece_walk_type const pfrom = get_walk_of_piece_on_square(from);
  piece_walk_type const pto = proofgames_target_position.board[to];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%u",captallowed);
  TraceFunctionParam("%d",captrequ);
  TraceFunctionParamListEnd();

  assert(TSTFLAG(being_solved.spec[from],side));
  assert(TSTFLAG(proofgames_target_position.spec[to],side));

  *moves= current_length;

  switch (pto)
  {
    case Pawn:
      if (pfrom==Pawn)
        PawnMovesFromTo(side,from, to, moves, captures, captallowed);
      break;

    default:
      if (pfrom==pto)
      {
        OfficerMovesFromTo(pfrom, from, to, moves);
        *captures= 0;
      }
      else if (pfrom==Pawn)
        PromPieceMovesFromTo(side,
                             from,to,
                             moves,captures,
                             captallowed-captrequ);
      break;
  }

  TraceValue("%u",*moves);
  TraceValue("%u",*captures);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_length_type ArrangeListedPieces(PieceList2 *pl,
                                            int nto,
                                            int nfrom,
                                            boolean *taken,
                                            stip_length_type CapturesAllowed)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",nto);
  TraceFunctionParam("%d",nfrom);
  TraceFunctionParam("%u",CapturesAllowed);
  TraceFunctionParamListEnd();

  if (nto==0)
    result = 0;
  else
  {
    int i;
    result = current_length;
    for (i = 0; i<pl[0].Nbr; ++i)
    {
      stip_length_type Diff2;
      int const id = pl[0].id[i];
      if (taken[id] || pl[0].captures[i]>CapturesAllowed)
      {
        /* nothing */
      }
      else
      {
        taken[id] = true;
        Diff2 = (pl[0].moves[i]
                 + ArrangeListedPieces(pl+1,
                                       nto-1,
                                       nfrom,
                                       taken,
                                       CapturesAllowed-pl[0].captures[i]));

        if (Diff2<result)
          result = Diff2;

        taken[id]= false;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static stip_length_type ArrangePieces(stip_length_type CapturesAllowed,
                                      Side camp,
                                      stip_length_type CapturesRequired)
{
  stip_length_type result;
  PieceList * const to = &ProofOfficers[camp];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",CapturesAllowed);
  TraceEnumerator(Side,camp);
  TraceFunctionParam("%u",CapturesRequired);
  TraceFunctionParamListEnd();

  TraceValue("%u",ProofOfficers[camp].Nbr);TraceEOL();

  if (to->Nbr == 0)
    result = 0;
  else
  {
    PieceList * const from = &PiecesToBeArranged[camp];
    PieceList2 pl[16];
    boolean taken[16];
    int ito;
    int ifrom;

    TraceValue("%u",PiecesToBeArranged[camp].Nbr);TraceEOL();

    for (ito = 0; ito<to->Nbr; ++ito)
    {
      pl[ito].Nbr = 0;
      for (ifrom = 0; ifrom<from->Nbr; ++ifrom)
      {
        stip_length_type moves;
        stip_length_type captures;
        PieceMovesFromTo(camp,
                         from->sq[ifrom],
                         to->sq[ito],
                         &moves,
                         &captures,
                         CapturesAllowed,
                         CapturesRequired);
        if (moves<current_length)
        {
          pl[ito].moves[pl[ito].Nbr] = moves;
          pl[ito].captures[pl[ito].Nbr] = captures;
          pl[ito].id[pl[ito].Nbr] = ifrom;
          ++pl[ito].Nbr;
        }
      }
    }

    for (ifrom = 0; ifrom<from->Nbr; ++ifrom)
      taken[ifrom] = false;

    /* determine minimal number of moves required */
    result = ArrangeListedPieces(pl,to->Nbr,from->Nbr,taken,CapturesAllowed);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static stip_length_type ArrangePawns(stip_length_type CapturesAllowed,
                                     Side   camp,
                                     stip_length_type *CapturesRequired)
{
  int       ifrom, ito;
  stip_length_type moves, captures, Diff;
  PieceList2    pl[8];
  boolean   taken[8];
  PieceList *from = &PawnsToBeArranged[camp];
  PieceList *to = &ProofPawns[camp];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",CapturesAllowed);
  TraceFunctionParam("%u",camp);
  TraceFunctionParamListEnd();

  if (to->Nbr == 0)
  {
    *CapturesRequired= 0;
    Diff = 0;
  }
  else
  {
    for (ito= 0; ito < to->Nbr; ito++)
    {
      pl[ito].Nbr= 0;
      for (ifrom= 0; ifrom<from->Nbr; ifrom++)
      {
        PawnMovesFromTo(camp,from->sq[ifrom],
                        to->sq[ito], &moves, &captures, CapturesAllowed);
        if (moves < current_length)
        {
          pl[ito].moves[pl[ito].Nbr]= moves;
          pl[ito].captures[pl[ito].Nbr]= captures;
          pl[ito].id[pl[ito].Nbr]= ifrom;
          pl[ito].Nbr++;
        }
      }
    }

    for (ifrom= 0; ifrom < from->Nbr; ifrom++)
      taken[ifrom]= false;

    /* determine minimal number of moves required */
    Diff= ArrangeListedPieces(pl,
                              to->Nbr, from->Nbr, taken, CapturesAllowed);

    if (Diff != current_length)
    {
      /* determine minimal number of captures required */
      captures= 0;
      while (ArrangeListedPieces(pl, to->Nbr, from->Nbr, taken, captures)
             == current_length)
        captures++;

      *CapturesRequired= captures;
      TraceValue("%u",*CapturesRequired);
      TraceEOL();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",Diff);
  TraceFunctionResultEnd();
  return Diff;
}

static boolean FairyImpossible(void)
{
  square const *bnp;
  square sq;
  unsigned int   Nbr[nr_sides];
  unsigned int MovesAvailable = MovesLeft[Black]+MovesLeft[White];

  TraceText("FairyImpossible\n");

  Nbr[White] = being_solved.number_of_pieces[White][Pawn]
          + being_solved.number_of_pieces[White][Knight]
          + being_solved.number_of_pieces[White][Rook]
          + being_solved.number_of_pieces[White][Bishop]
          + being_solved.number_of_pieces[White][Queen]
          + being_solved.number_of_pieces[White][King];

  Nbr[Black] = being_solved.number_of_pieces[Black][Pawn]
          + being_solved.number_of_pieces[Black][Knight]
          + being_solved.number_of_pieces[Black][Rook]
          + being_solved.number_of_pieces[Black][Bishop]
          + being_solved.number_of_pieces[Black][Queen]
          + being_solved.number_of_pieces[Black][King];

  /* not enough time to capture the remaining pieces */
  if (change_moving_piece)
  {
    if (Nbr[White] + Nbr[Black]
        > MovesAvailable + ProofNbrPieces[White] + ProofNbrPieces[Black])
      return true;


    if (CondFlag[andernach]
        && !CondFlag[circe] && !CondFlag[sentinelles]) {
      unsigned int count= 0;
      /* in AndernachChess we need at least 1 capture if a pawn
         residing at his initial square has moved and has to be
         reestablished via a capture of the opposite side.
         has a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq <= square_h2; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],White))
            && (proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],White)))
          ++count;

      if (16-count < ProofNbrPieces[Black])
        return true;

      count = 0;

      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],Black))
            && (proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],Black)))
          ++count;

      if (16-count < ProofNbrPieces[White])
        return true;
    }
  }
  else
  {
    if (!CondFlag[masand] && !CondFlag[lostpieces] && !CondFlag[breton])
    {
      /* not enough time to capture the remaining pieces */
      if (Nbr[White] > MovesLeft[Black]+ProofNbrPieces[White]
          || Nbr[Black] > MovesLeft[White]+ProofNbrPieces[Black])
        return true;
    }

    if (!CondFlag[sentinelles])
    {
      /* note, that we are in the !change_moving_piece section
         too many pawns captured or promoted
      */
      boolean parrain_pawn[nr_sides] = { false, false };
      if (circe_variant.relevant_capture==circe_relevant_capture_lastmove)
      {
        move_effect_journal_index_type const top = move_effect_journal_base[nbply];
        move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
        if (move_effect_journal[capture].u.piece_removal.walk==Pawn)
        {
          Flags const removed_spec = move_effect_journal[capture].u.piece_removal.flags;
          if (TSTFLAG(removed_spec,White))
            parrain_pawn[White] = true;
          if (TSTFLAG(removed_spec,Black))
            parrain_pawn[Black] = true;
        }
      }

      if (proofgames_target_position.number_of_pieces[White][Pawn] > being_solved.number_of_pieces[White][Pawn]+parrain_pawn[White]
          || proofgames_target_position.number_of_pieces[Black][Pawn] > being_solved.number_of_pieces[Black][Pawn]+parrain_pawn[Black])
        return true;
    }

    if (CondFlag[anticirce]
        && anticirce_variant.determine_rebirth_square==circe_determine_rebirth_square_from_pas)
    {
      /* note, that we are in the !change_moving_piece section */
      unsigned int count= 0;
      /* in AntiCirce we need at least 2 captures if a pawn
         residing at his initial square has moved and has to be
         reborn via capture because we need a second pawn to do
         the same to the other rank NOT ALWAYS TRUE ! Only if
         there's no pawn of the same colour on the same rank has
         a white pawn on the second rank moved or has it been
         captured?
      */
      for (sq= square_a2; sq<=square_h2; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],White)))
        {
          if ((proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],White)))
          {
            if (!(proofgames_target_position.board[sq+dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+dir_up],White))
                && !(proofgames_target_position.board[sq+2*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+2*dir_up],White))
                && !(proofgames_target_position.board[sq+3*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+3*dir_up],White))
                && !(proofgames_target_position.board[sq+4*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+4*dir_up],White))
                && !(proofgames_target_position.board[sq+5*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+5*dir_up],White)))
              ++count;
          }
          else if ((proofgames_target_position.board[sq+dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+dir_up],White))
                   && !(get_walk_of_piece_on_square(sq+dir_up)==Pawn && TSTFLAG(being_solved.spec[sq+dir_up],White)))
          {
            if (!(proofgames_target_position.board[sq+2*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+2*dir_up],White))
                && !(proofgames_target_position.board[sq+3*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+3*dir_up],White))
                && !(proofgames_target_position.board[sq+4*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+4*dir_up],White))
                && !(proofgames_target_position.board[sq+5*dir_up]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+5*dir_up],White)))
              ++count;
          }
        }

      if (count%2 == 1)
        ++count;

      if (16-count < ProofNbrPieces[Black])
        return true;

      count= 0;
      /* has a black pawn on the seventh rank moved or has it
         been captured?
      */
      for (sq= square_a7; sq <= square_h7; sq++)
        if (!(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],Black)))
        {
          if ((proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],Black)))
          {
            if (!(proofgames_target_position.board[sq+dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+dir_down],Black))
                && !(proofgames_target_position.board[sq+2*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+2*dir_down],Black))
                && !(proofgames_target_position.board[sq+3*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+3*dir_down],Black))
                && !(proofgames_target_position.board[sq+4*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+4*dir_down],Black))
                && !(proofgames_target_position.board[sq+5*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+5*dir_down],Black)))
              ++count;
          }
          else if ((proofgames_target_position.board[sq+dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+dir_down],Black))
                   && !(get_walk_of_piece_on_square(sq+dir_down)==Pawn && TSTFLAG(being_solved.spec[sq+dir_down],Black)))
          {
            if (!(proofgames_target_position.board[sq+2*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+2*dir_down],Black))
                && !(proofgames_target_position.board[sq+3*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+3*dir_down],Black))
                && !(proofgames_target_position.board[sq+4*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+4*dir_down],Black))
                && !(proofgames_target_position.board[sq+5*dir_down]==Pawn && TSTFLAG(proofgames_target_position.spec[sq+5*dir_down],Black)))
              ++count;
          }
        }

      if (count%2 == 1)
        ++count;
      if (16-count < ProofNbrPieces[White])
        return true;
    }
  }

  /* find a solution ... */
  TraceText("testing if there are enough remaining moves");TraceEOL();
  MovesAvailable *= 2;

  for (bnp = boardnum; *bnp; bnp++)
  {
    piece_walk_type const p = proofgames_target_position.board[*bnp];
    if (p!=Empty)
    {
      if (p!=get_walk_of_piece_on_square(*bnp)
          || (proofgames_target_position.spec[*bnp]&COLOURFLAGS)!=(being_solved.spec[*bnp]&COLOURFLAGS))
      {
        if (MovesAvailable==0)
        {
          TraceText("available moves exhausted");TraceEOL();
          return true;
        }
        else
          --MovesAvailable;
      }
    }
  }

  TraceText("FairyImossible returns false");TraceEOL();
  return false;
}

static boolean Impossible(void)
{
  square const *bnp;
  stip_length_type moves_left[nr_sides] = { MovesLeft[White], MovesLeft[Black] };
  stip_length_type to_be_captured[nr_sides];
  stip_length_type captures_required[nr_sides];
  square    sq;
  unsigned int Nbr[nr_sides];

  TraceText("ProofImpossible\n");

  /* too many pawns captured or promoted */
  if (proofgames_target_position.number_of_pieces[White][Pawn] > being_solved.number_of_pieces[White][Pawn])
  {
    TraceValue("%d ",proofgames_target_position.number_of_pieces[White][Pawn]);
    TraceValue("%d",being_solved.number_of_pieces[White][Pawn]);
    TraceEOL();
    return true;
  }

  if (proofgames_target_position.number_of_pieces[Black][Pawn] > being_solved.number_of_pieces[Black][Pawn])
  {
    TraceValue("%d ",being_solved.number_of_pieces[Black][Pawn]);
    TraceValue("%d",being_solved.number_of_pieces[Black][Pawn]);
    TraceEOL();
    return true;
  }

  Nbr[White] = being_solved.number_of_pieces[White][Pawn]
          + being_solved.number_of_pieces[White][Knight]
          + being_solved.number_of_pieces[White][Rook]
          + being_solved.number_of_pieces[White][Bishop]
          + being_solved.number_of_pieces[White][Queen]
          + being_solved.number_of_pieces[White][King];

  Nbr[Black] = being_solved.number_of_pieces[Black][Pawn]
          + being_solved.number_of_pieces[Black][Knight]
          + being_solved.number_of_pieces[Black][Rook]
          + being_solved.number_of_pieces[Black][Bishop]
          + being_solved.number_of_pieces[Black][Queen]
          + being_solved.number_of_pieces[Black][King];

  /* too many pieces captured */
  if (Nbr[White] < ProofNbrPieces[White])
  {
    TraceValue("%d ",Nbr[White]);
    TraceValue("%d",ProofNbrPieces[White]);
    TraceEOL();
    return true;
  }
  if (Nbr[Black] < ProofNbrPieces[Black])
  {
    TraceValue("%d ",Nbr[Black]);
    TraceValue("%d",ProofNbrPieces[Black]);
    TraceEOL();
    return true;
  }

  /* check if there is enough time left to capture the
     superfluos pieces
  */

  /* not enough time to capture the remaining pieces */
  to_be_captured[White] = Nbr[White]-ProofNbrPieces[White];
  TraceValue("%d ",to_be_captured[White]);
  TraceValue("%d ",Nbr[White]);
  TraceValue("%d ",ProofNbrPieces[White]);
  TraceValue("%d",moves_left[Black]);
  TraceEOL();
  if (to_be_captured[White]>moves_left[Black])
    return true;

  to_be_captured[Black] = Nbr[Black] - ProofNbrPieces[Black];
  TraceValue("%d ",to_be_captured[Black]);
  TraceValue("%d ",Nbr[Black]);
  TraceValue("%d ",ProofNbrPieces[Black]);
  TraceValue("%d",moves_left[White]);
  TraceEOL();
  if (to_be_captured[Black]>moves_left[White])
    return true;

  /* has one of the blocked pieces been captured ? */
  if ((BlockedQueenBishop[White] && !(proofgames_target_position.board[square_c1]==Bishop && TSTFLAG(proofgames_target_position.spec[square_c1],White)))
      || (BlockedKingBishop[White] && !(proofgames_target_position.board[square_f1]==Bishop && TSTFLAG(proofgames_target_position.spec[square_f1],White)))
      || (BlockedQueenBishop[Black] && !(proofgames_target_position.board[square_c8]==Bishop && TSTFLAG(proofgames_target_position.spec[square_c8],Black)))
      || (BlockedKingBishop[Black] && !(proofgames_target_position.board[square_f8]==Bishop && TSTFLAG(proofgames_target_position.spec[square_f8],Black)))
      || (BlockedQueen[White]  && !(proofgames_target_position.board[square_d1]==Queen && TSTFLAG(proofgames_target_position.spec[square_d1],White)))
      || (BlockedQueen[Black]  && !(proofgames_target_position.board[square_d8]==Queen && TSTFLAG(proofgames_target_position.spec[square_d8],Black))))
  {
    TraceText("blocked piece was captured\n");
    return true;
  }

  /* has a white pawn on the second rank moved or has it
     been captured?
  */
  for (sq= square_a2; sq<=square_h2; sq+=dir_right)
    if ((proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],White)) && !(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],White)))
      return true;

  /* has a black pawn on the seventh rank moved or has it
     been captured?
  */
  for (sq= square_a7; sq<=square_h7; sq+=dir_right)
    if ((proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],Black)) && !(get_walk_of_piece_on_square(sq)==Pawn && TSTFLAG(being_solved.spec[sq],Black)))
      return true;

  {
    stip_length_type const white_king_moves_needed = KingMovesNeeded(White);
    if (moves_left[White]<white_king_moves_needed)
    {
      TraceText(" white_moves_left<white_king_moves_needed\n");
      return true;
    }
    else
      moves_left[White] -= KingMovesNeeded(White);
  }

  {
    stip_length_type const black_king_moves_needed = KingMovesNeeded(Black);
    if (moves_left[Black]<black_king_moves_needed)
    {
      TraceText("black_moves_left<black_king_moves_needed\n");
      return true;
    }
    else
      moves_left[Black] -= black_king_moves_needed;
  }

  if (CondFlag[haanerchess])
  {
    TraceText("impossible hole created\n");
    return proofgames_target_position.board[move_effect_journal_get_departure_square(nbply)] != Empty;
  }

  /* collect the pieces for further investigations */
  ProofPawns[White].Nbr = 0;
  ProofOfficers[White].Nbr = 0;
  ProofPawns[Black].Nbr = 0;
  ProofOfficers[Black].Nbr = 0;
  PawnsToBeArranged[White].Nbr = 0;
  PiecesToBeArranged[White].Nbr = 0;
  PawnsToBeArranged[Black].Nbr = 0;
  PiecesToBeArranged[Black].Nbr= 0;

  for (bnp = boardnum; *bnp; bnp++)
  {
    piece_walk_type const p1 = proofgames_target_position.board[*bnp];
    piece_walk_type const p2 = get_walk_of_piece_on_square(*bnp);
    Side const side_target = TSTFLAG(proofgames_target_position.spec[*bnp],White) ? White : Black;
    Side const side_current = TSTFLAG(being_solved.spec[*bnp],White) ? White : Black;

    if (p1!=p2 || side_target!=side_current)
    {
      TraceSquare(*bnp);
      TraceWalk(proofgames_target_position.board[*bnp]);
      TraceEnumerator(Side,side_target);
      TraceWalk(get_walk_of_piece_on_square(*bnp));
      TraceEnumerator(Side,side_current);
      TraceEOL();

      switch (p1)
      {
        case Empty:
        case King:
          break;

        case Pawn:
          ProofPawns[side_target].sq[ProofPawns[side_target].Nbr]= *bnp;
          ProofPawns[side_target].Nbr++;
          ProofOfficers[side_target].sq[ProofOfficers[side_target].Nbr]= *bnp;
          ProofOfficers[side_target].Nbr++;
          break;

        default:
          ProofOfficers[side_target].sq[ProofOfficers[side_target].Nbr]= *bnp;
          ProofOfficers[side_target].Nbr++;
          break;
      }

      switch (p2)
      {
        case Empty:
        case King:
          break;

        case Pawn:
          PawnsToBeArranged[side_current].sq[PawnsToBeArranged[side_current].Nbr] = *bnp;
          ++PawnsToBeArranged[side_current].Nbr;

          PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr] = *bnp;
          ++PiecesToBeArranged[side_current].Nbr;
          break;

        case Bishop:
          TraceValue("%u",CapturedQueenBishop[side_current]);
          TraceValue("%u",CapturedKingBishop[side_current]);
          TraceEOL();
          if (side_current==White)
          {
            if (!(CapturedQueenBishop[White] && *bnp == square_c1)
                && !(CapturedKingBishop[White] && *bnp == square_f1))
            {
              PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr] = *bnp;
              ++PiecesToBeArranged[side_current].Nbr;
            }
          }
          else
          {
            if (!(CapturedQueenBishop[Black] && *bnp == square_c8)
                && !(CapturedKingBishop[Black] && *bnp == square_f8))
            {
              PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr] = *bnp;
              ++PiecesToBeArranged[side_current].Nbr;
            }
          }
          break;

        case Queen:
          TraceValue("%u",CapturedQueen[side_current]);
          TraceEOL();
          if (side_current==White)
          {
            if (!(CapturedQueen[White] && *bnp==square_d1))
            {
              PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr] = *bnp;
              ++PiecesToBeArranged[side_current].Nbr;
            }
          }
          else
          {
            if (!(CapturedQueen[Black] && *bnp==square_d8))
            {
              PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr] = *bnp;
              ++PiecesToBeArranged[side_current].Nbr;
            }
          }
          break;

        default:
          PiecesToBeArranged[side_current].sq[PiecesToBeArranged[side_current].Nbr] = *bnp;
          ++PiecesToBeArranged[side_current].Nbr;
          break;
      }

      TraceValue("%u",PawnsToBeArranged[side_current].Nbr);
      TraceValue("%u",PiecesToBeArranged[side_current].Nbr);
      TraceEOL();
    }
  }

  if (ArrangePawns(to_be_captured[Black],White,&captures_required[Black])>moves_left[White])
  {
    TraceText("ArrangePawns(BlPieToBeCapt,White,&BlCapturesRequired)"
              ">white_moves_left\n");
    return true;
  }

  if (ArrangePawns(to_be_captured[White],Black,&captures_required[White])>moves_left[Black])
  {
    TraceValue("%u",moves_left[Black]);
    TraceEOL();
    TraceText("ArrangePawns(WhPieToBeCapt,Black,&WhCapturesRequired)"
              ">black_moves_left\n");
    return true;
  }

  if (ArrangePieces(to_be_captured[Black],
                    White,
                    captures_required[Black])>moves_left[White])
  {
    TraceText("(ArrangePieces(BlPieToBeCapt,White,BlCapturesRequired)"
              ">white_moves_left\n");
    return true;
  }

  if (ArrangePieces(to_be_captured[White],
                    Black,
                    captures_required[White])>moves_left[Black])
  {
    TraceText("ArrangePieces(WhPieToBeCapt,Black,WhCapturesRequired)"
              ">black_moves_left\n");
    return true;
  }

  TraceText("not ProofImpossible\n");
  return false;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void goalreachable_guard_proofgame_solve(slice_index si)
{
  Side const just_moved = advers(SLICE_STARTER(si));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=previous_move_has_solved);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,SLICE_STARTER(si));
  TraceEnumerator(Side,just_moved);
  TraceValue("%u",MovesLeft[SLICE_STARTER(si)]);
  TraceValue("%u",MovesLeft[just_moved]);
  TraceEOL();

  pipe_this_move_doesnt_solve_if(si,Impossible());

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[SLICE_STARTER(si)]);
  TraceValue("%u",MovesLeft[just_moved]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void goalreachable_guard_proofgame_fairy_solve(slice_index si)
{
  Side const just_moved = advers(SLICE_STARTER(si));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=previous_move_has_solved);

  --MovesLeft[just_moved];
  TraceEnumerator(Side,SLICE_STARTER(si));
  TraceEnumerator(Side,just_moved);
  TraceValue("%u",MovesLeft[SLICE_STARTER(si)]);
  TraceValue("%u",MovesLeft[just_moved]);
  TraceEOL();

  pipe_this_move_doesnt_solve_if(si,FairyImpossible());

  ++MovesLeft[just_moved];
  TraceValue("%u",MovesLeft[SLICE_STARTER(si)]);
  TraceValue("%u",MovesLeft[just_moved]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void InitialiseKingMoves(Side side)
{
  square const *bnp;
  square sq;
  int   MoveNbr;
  boolean   GoOn;
  square const square_base = side==White ? square_a1 : square_a8;
  square const square_opponent_base = side==White ? square_a8 : square_a1;
  square const square_pawn_base = side==White ? square_a2 : square_a7;
  square const square_opponent_pawn_base = side==White ? square_a7 : square_a2;
  numvec const dir_backward = side==White ? dir_down : dir_up;
  Cond const trans_king = side==White ? whtrans_king : bltrans_king;
  Cond const supertrans_king = side==White ? whsupertrans_king : blsupertrans_king;
  Cond const vault_king = side==White ? whvault_king : blvault_king;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if (proofgames_target_position.king_square[side]==initsquare)
  {
    /* set all squares to a maximum */
    for (bnp = boardnum; *bnp; ++bnp)
      KingMoves[side][*bnp] = 0;
  }
  else
  {
    boolean const trivial_validation = is_observation_trivially_validated(advers(side));

    /* set all squares to a maximum */
    for (bnp = boardnum; *bnp; ++bnp)
      KingMoves[side][*bnp] = current_length;

    for (sq = square_pawn_base; sq<square_pawn_base+nr_files_on_board; ++sq)
      if (proofgames_target_position.board[sq]==Pawn && TSTFLAG(proofgames_target_position.spec[sq],side))
        KingMoves[side][sq] = -1; /* blocked */

    for (sq = square_opponent_pawn_base; sq<square_opponent_pawn_base+nr_files_on_board; ++sq)
      if (proofgames_target_position.board[sq]==Pawn
          && TSTFLAG(proofgames_target_position.spec[sq],advers(side)))
      {
        KingMoves[side][sq]= -1;    /* blocked */
        if (trivial_validation)
        {
          KingMoves[side][sq+dir_backward+dir_left] = -2;
          KingMoves[side][sq+dir_backward+dir_right] = -2; /* guarded */
        }
      }

    /* cornered bishops */
    if (BlockedQueenBishop[side])
      KingMoves[side][square_base+file_c]= -1;
    if (BlockedKingBishop[side])
      KingMoves[side][square_base+file_f]= -1;
    if (BlockedQueenBishop[advers(side)])
      KingMoves[side][square_opponent_base+file_c]= -1;
    if (BlockedKingBishop[advers(side)])
      KingMoves[side][square_opponent_base+file_f]= -1;

    /* initialise wh king */
    KingMoves[side][proofgames_target_position.king_square[side]]= 0;
    MoveNbr= 0;
    do
    {
      GoOn= false;
      for (bnp= boardnum; *bnp; bnp++)
      {
        if (KingMoves[side][*bnp] == MoveNbr)
        {
          vec_index_type k;
          for (k= vec_queen_end; k>=vec_queen_start; k--)
          {
            sq= *bnp+vec[k];
            if (KingMoves[side][sq] > MoveNbr)
            {
              KingMoves[side][sq]= MoveNbr+1;
              GoOn= true;
            }
            if (CondFlag[trans_king]
                || CondFlag[supertrans_king]
                || (CondFlag[vault_king] && vaulting_kings_transmuting[side]))
            {
              sq= *bnp+vec[k];
              while (!is_square_blocked(sq) && KingMoves[side][sq]!=-1)
              {
                if (KingMoves[side][sq] > MoveNbr)
                {
                  KingMoves[side][sq]= MoveNbr+1;
                  GoOn= true;
                }
                sq += vec[k];
              }
            }
          }
          if (CondFlag[trans_king]
              || CondFlag[supertrans_king]
              || (CondFlag[vault_king] && vaulting_kings_transmuting[side]))
          {
            vec_index_type k;
            for (k= vec_knight_end; k>=vec_knight_start; k--)
            {
              sq= *bnp+vec[k];
              if (!is_square_blocked(sq) && KingMoves[side][sq]>MoveNbr)
              {
                KingMoves[side][sq]= MoveNbr+1;
                GoOn= true;
              }
            }
          }
        }
      }
      MoveNbr++;
    } while(GoOn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void InitialiseIntelligentSide(Side side)
{
  square const square_base = side==White ? square_a1 : square_a8;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  ProofNbrPieces[side] = 0;

  {
    piece_walk_type i;
    for (i = King; i <= Bishop; ++i)
      ProofNbrPieces[side] += proofgames_target_position.number_of_pieces[side][i];
  }

  if (!ProofFairy)
  {
    numvec const dir_forward = side==White ? dir_up : dir_down;
    square const sq_queen_bishop = square_base+file_c;
    square const sq_king_bishop = square_base+file_f;
    square const sq_queen = square_base+file_d;
    square const sq_queen_bishop_block_left = sq_queen_bishop+dir_forward+dir_left;
    square const sq_queen_bishop_block_right = sq_queen_bishop+dir_forward+dir_right;
    square const sq_king_bishop_block_left = sq_king_bishop+dir_forward+dir_left;
    square const sq_king_bishop_block_right = sq_king_bishop+dir_forward+dir_right;
    square const sq_queen_block_left = sq_queen+dir_forward+dir_left;
    /* 2*dir_right is correct
     * together with the bishops, this causes queen and king to be trapped */
    square const sq_queen_block_right = sq_queen+dir_forward+2*dir_right;

    /* determine pieces blocked */
    BlockedQueenBishop[side] = (proofgames_target_position.board[sq_queen_bishop]==Bishop && TSTFLAG(proofgames_target_position.spec[sq_queen_bishop],side))
        && (proofgames_target_position.board[sq_queen_bishop_block_left]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_bishop_block_left],side))
        && (proofgames_target_position.board[sq_queen_bishop_block_right]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_bishop_block_right],side));

    BlockedKingBishop[side] = (proofgames_target_position.board[sq_king_bishop]==Bishop && TSTFLAG(proofgames_target_position.spec[sq_king_bishop],side))
        && (proofgames_target_position.board[sq_king_bishop_block_left]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_king_bishop_block_left],side))
        && (proofgames_target_position.board[sq_king_bishop_block_right]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_king_bishop_block_right],side));

    BlockedQueen[side] = BlockedQueenBishop[side]
        && BlockedKingBishop[side]
        && (proofgames_target_position.board[sq_queen]==Queen && TSTFLAG(proofgames_target_position.spec[sq_queen],side))
        && (proofgames_target_position.board[sq_queen_block_left]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_block_left],side))
        && (proofgames_target_position.board[sq_queen_block_right]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_block_right],side));

    /* determine pieces captured */
    CapturedQueenBishop[side] = !(proofgames_target_position.board[sq_queen_bishop]==Bishop && TSTFLAG(proofgames_target_position.spec[sq_queen_bishop],side))
        && (proofgames_target_position.board[sq_queen_bishop_block_left]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_bishop_block_left],side))
        && (proofgames_target_position.board[sq_queen_bishop_block_right]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_bishop_block_right],side));

    CapturedKingBishop[side] = !(proofgames_target_position.board[sq_king_bishop]==Bishop && TSTFLAG(proofgames_target_position.spec[sq_king_bishop],side))
        && (proofgames_target_position.board[sq_king_bishop_block_left]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_king_bishop_block_left],side))
        && (proofgames_target_position.board[sq_king_bishop_block_right]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_king_bishop_block_right],side));

    CapturedQueen[side] = BlockedQueenBishop[side]
        && BlockedKingBishop[side]
        && !(proofgames_target_position.board[sq_queen]==Queen && TSTFLAG(proofgames_target_position.spec[sq_queen],side))
        && (proofgames_target_position.board[sq_queen_block_left]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_block_left],side))
        && (proofgames_target_position.board[sq_queen_block_right]==Pawn && TSTFLAG(proofgames_target_position.spec[sq_queen_block_right],side));

    TraceEnumerator(Side,side);
    TraceValue("%u",BlockedQueenBishop[side]);
    TraceValue("%u",BlockedKingBishop[side]);
    TraceSquare(sq_queen);
    TraceWalk(proofgames_target_position.board[sq_queen]);
    TraceValue("%u",CapturedQueen[side]);
    TraceEOL();

    /* update castling possibilities */
    if (BlockedQueenBishop[side])
      /* long castling impossible */
      CLRCASTLINGFLAGMASK(side,ra_cancastle);

    if (BlockedKingBishop[side])
      /* short castling impossible */
      CLRCASTLINGFLAGMASK(side,rh_cancastle);

    if (!TSTCASTLINGFLAGMASK(side,ra_cancastle|rh_cancastle))
      /* no wh rook can castle, so the wh king cannot either */
      CLRCASTLINGFLAGMASK(side,k_cancastle);

    /* initialise king diff_move arrays */
    InitialiseKingMoves(side);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void InitialiseIntelligent(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  current_length = MOVE_HAS_SOLVED_LENGTH();

  InitialiseIntelligentSide(White);
  InitialiseIntelligentSide(Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void intelligent_proof_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  InitialiseIntelligent();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
