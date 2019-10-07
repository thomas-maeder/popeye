#include "pieces/attributes/total_invisible/intercept_illegal_checks.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/king_placement.h"
#include "pieces/attributes/total_invisible.h"
#include "solving/ply.h"
#include "solving/move_effect_journal.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static void done_intercepting_illegal_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply<=top_ply_of_regular_play)
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    square const first_taboo_violation = find_taboo_violation();

    TraceValue("%u",nbply);
    TraceValue("%u",top_ply_of_regular_play);
    TraceSquare(sq_departure);
    TraceValue("%u",sq_departure);
    TraceValue("%u",capture_by_invisible);
    TraceValue("%u",flesh_out_move_highwater);
    TraceSquare(sq_arrival);
    TraceValue("%u",sq_arrival);
    TraceValue("%u",move_by_invisible);
    TraceEOL();

    {
      PieceIdType id;
      for (id = top_visible_piece_id+1; id<=top_invisible_piece_id; ++id)
      {
        TraceValue("%u",id);

        TraceValue("%u",motivation[id].first.purpose);
        TraceValue("%u",motivation[id].first.acts_when);
        TraceSquare(motivation[id].first.on);

        TraceValue("%u",motivation[id].last.purpose);
        TraceValue("%u",motivation[id].last.acts_when);
        TraceSquare(motivation[id].last.on);

        TraceWalk(get_walk_of_piece_on_square(motivation[id].last.on));
        TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
        TraceEOL();
      }
    }

    {
      PieceIdType id;
      for (id = top_visible_piece_id+1; id<=top_invisible_piece_id; ++id)
      {
        TraceValue("%u",id);TraceEOL();
        assert((motivation[id].first.acts_when>nbply) // active in the future
               || (motivation[id].first.acts_when==nbply && motivation[id].first.purpose!=purpose_interceptor) // to become active later in this ply
               || (motivation[id].first.acts_when==nbply && motivation[id].first.purpose==purpose_interceptor) // revealed interceptor - not necessarly present
               || (motivation[id].first.acts_when<nbply && motivation[id].last.acts_when>=nbply) // in action
               || (motivation[id].last.purpose==purpose_none && motivation[id].last.acts_when<nbply) // put on hold by a revelation or capture
               || (GetPieceId(being_solved.spec[motivation[id].last.on])==id));
      }
    }

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
    {
      assert(first_taboo_violation==nullsquare);
      flesh_out_random_move_by_invisible();
    }
    else if (nbply<=flesh_out_move_highwater)
    {
      if (first_taboo_violation==nullsquare)
        adapt_pre_capture_effect();
      else
      {
        TraceText("can't resolve taboo violation\n");
        REPORT_DECISION_OUTCOME("%s","can't resolve taboo violation");
        REPORT_DEADEND;
      }
    }
    else if (sq_departure>=capture_by_invisible
             && is_on_board(sq_arrival))
      flesh_out_capture_by_invisible(first_taboo_violation);
    else if (first_taboo_violation==nullsquare)
      adapt_pre_capture_effect();
    // TODO review
//    else
//      assert(is_taboo_violation_acceptable(first_taboo_violation));
  }
  else
    validate_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                      unsigned int nr_check_vectors);

static void walk_interceptor_any_walk(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                      unsigned int nr_check_vectors,
                                      Side side,
                                      square pos,
                                      piece_walk_type walk,
                                      Flags spec)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceWalk(walk);
  TraceValue("%x",spec);
  TraceFunctionParamListEnd();

  ++being_solved.number_of_pieces[side][walk];

  TraceValue("%u",top_invisible_piece_id);TraceEOL();

  SetPieceId(spec,top_invisible_piece_id);
  occupy_square(pos,walk,spec);
  REPORT_DECISION_WALK('>',walk);
  ++curr_decision_level;

  {
    Side const side_attacked = advers(side);
    square const king_pos = being_solved.king_square[side_attacked];
    vec_index_type const k = is_square_uninterceptably_attacked(side_attacked,
                                                                king_pos);
    if (k==UINT_MAX)
    {
      // TODO accept uninterceptable check if not illegal
      REPORT_DECISION_OUTCOME("%s","interceptor delivers uninterceptable check - TODO: not necessarily a deadend");
      REPORT_DEADEND;
    }
    else if (k==0 || king_pos+vec[k]!=pos)
    {
      if (nr_check_vectors==1)
        restart_from_scratch();
      else
        place_interceptor_on_line(check_vectors,nr_check_vectors-1);
    }
    else
    {
      // TODO accept uninterceptable check if not illegal
      REPORT_DECISION_OUTCOME("%s","interceptor delivers uninterceptable check - TODO: not necessarily a deadend");
      REPORT_DEADEND;
    }
  }

  --curr_decision_level;

  TraceWalk(get_walk_of_piece_on_square(pos));
  TraceWalk(walk);
  TraceEOL();
  assert(get_walk_of_piece_on_square(pos)==walk);
  empty_square(pos);
  --being_solved.number_of_pieces[side][walk];

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_pawn(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                  unsigned int nr_check_vectors,
                                  Side side,
                                  square pos,
                                  Flags spec)
{
  SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceValue("%x",spec);
  TraceFunctionParamListEnd();

  if ((TSTFLAG(sq_spec[pos],basesq) || TSTFLAG(sq_spec[pos],promsq)))
  {
    REPORT_DECISION_OUTCOME("%s","pawn is placed on impossible square");
    REPORT_DEADEND;
  }
  else
    walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Pawn,spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_king(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                  unsigned int nr_check_vectors,
                                  Side side,
                                  square pos)
{
  dynamic_consumption_type const save_consumption = current_consumption;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  /* this removes the implicit allocation for the invisible king ...*/
  being_solved.king_square[side] = pos;

  /* ... and thus allows this to succeed: */
  if (allocate_flesh_out_unplaced(side))
  {
    Flags const spec = BIT(side)|BIT(Royal)|BIT(Chameleon);
    walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,King,spec);
  }

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

  being_solved.king_square[side] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                             unsigned int nr_check_vectors,
                             Side side,
                             square pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  TraceSquare(being_solved.king_square[side]);
  TraceEOL();
  assert(is_square_empty(pos));

  motivation[top_invisible_piece_id].levels.walk = curr_decision_level;

  if (being_solved.king_square[side]==initsquare)
    walk_interceptor_king(check_vectors,nr_check_vectors,side,pos);

  {
    dynamic_consumption_type const save_consumption = current_consumption;

    if (allocate_flesh_out_unplaced(side))
    {
      Flags const spec = BIT(side)|BIT(Chameleon);

      if (curr_decision_level<=max_decision_level)
      {
        max_decision_level = decision_level_latest;
        walk_interceptor_pawn(check_vectors,nr_check_vectors,side,pos,spec);
      }

      if (side==trait[nbply])
      {
        if (curr_decision_level<=max_decision_level)
        {
          max_decision_level = decision_level_latest;
          walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Knight,spec);
        }

        if (curr_decision_level<=max_decision_level)
        {
          vec_index_type const k = check_vectors[nr_check_vectors-1];
          boolean const is_check_orthogonal = k<=vec_rook_end;

          max_decision_level = decision_level_latest;

          if (is_check_orthogonal)
            walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Bishop,spec);
          else
            walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,Rook,spec);
        }
      }
      else
      {
        piece_walk_type walk;
        for (walk = Queen;
             walk<=Bishop && curr_decision_level<=max_decision_level;
             ++walk)
        {
          max_decision_level = decision_level_latest;
          walk_interceptor_any_walk(check_vectors,nr_check_vectors,side,pos,walk,spec);
        }
      }
    }
    else
    {
      REPORT_DECISION_OUTCOME("%s","not enough available invisibles for intercepting all illegal checks");
      REPORT_DEADEND;
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_interceptor(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                               unsigned int nr_check_vectors,
                               square pos)
{
  Side const preferred_side = trait[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  if (curr_decision_level<=max_decision_level)
  {
    if (!is_taboo(pos,preferred_side))
    {
      max_decision_level = decision_level_latest;
      REPORT_DECISION_COLOUR('>',BIT(preferred_side));
      ++curr_decision_level;
      walk_interceptor(check_vectors,nr_check_vectors,preferred_side,pos);
      --curr_decision_level;
    }
  }

  if (curr_decision_level<=max_decision_level)
  {
    if (!is_taboo(pos,advers(preferred_side)))
    {
      max_decision_level = decision_level_latest;
      REPORT_DECISION_COLOUR('>',BIT(advers(preferred_side)));
      ++curr_decision_level;
      walk_interceptor(check_vectors,nr_check_vectors,advers(preferred_side),pos);
      --curr_decision_level;
    }
  }

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                                unsigned int nr_check_vectors,
                                                square s,
                                                Side side)
{
  dynamic_consumption_type const save_consumption = current_consumption;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  REPORT_DECISION_COLOUR('>',BIT(side));
  ++curr_decision_level;

  if (allocate_placed(side))
  {
    if (is_taboo(s,side))
    {
      REPORT_DECISION_OUTCOME("%s","taboo violation");
      REPORT_DEADEND;
    }
    else
    {
      TraceSquare(s);TraceEnumerator(Side,trait[nbply-1]);TraceEOL();

      CLRFLAG(being_solved.spec[s],advers(side));

      if (nr_check_vectors==1)
        done_intercepting_illegal_checks();
      else
        place_interceptor_on_line(check_vectors,nr_check_vectors-1);

      SETFLAG(being_solved.spec[s],advers(side));
    }
  }
  else
  {
    REPORT_DECISION_OUTCOME("%s","not enough available invisibles for intercepting all illegal checks");
    REPORT_DEADEND;
  }

  --curr_decision_level;

  current_consumption = save_consumption;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                        unsigned int nr_check_vectors,
                                        square s)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  motivation[top_invisible_piece_id].first.on = s;
  motivation[top_invisible_piece_id].last.on = s;

  motivation[top_invisible_piece_id].levels.from = decision_level_latest;
  motivation[top_invisible_piece_id].levels.side = curr_decision_level;

  if (play_phase==play_validating_mate)
  {
    Flags spec = BIT(White)|BIT(Black)|BIT(Chameleon);

    SetPieceId(spec,top_invisible_piece_id);
    occupy_square(s,Dummy,spec);

    motivation[top_invisible_piece_id].levels.walk = decision_level_latest;

    place_interceptor_of_side_on_square(check_vectors,nr_check_vectors,s,White);

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      place_interceptor_of_side_on_square(check_vectors,nr_check_vectors,s,Black);
    }

    TraceConsumption();TraceEOL();

    empty_square(s);
  }
  else
    colour_interceptor(check_vectors,nr_check_vectors,s);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                      unsigned int nr_check_vectors)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];
  vec_index_type const kcurr = check_vectors[nr_check_vectors-1];
  int const dir = vec[kcurr];
  REPORT_DECISION_DECLARE(unsigned int const save_counter = report_decision_counter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  ++top_invisible_piece_id;

  TraceValue("%u",top_invisible_piece_id);
  TraceValue("%u",motivation[top_invisible_piece_id].last.purpose);
  TraceEOL();
  assert(motivation[top_invisible_piece_id].last.purpose==purpose_none);
  motivation[top_invisible_piece_id].first.purpose = purpose_interceptor;
  motivation[top_invisible_piece_id].first.acts_when = nbply;
  motivation[top_invisible_piece_id].last.purpose = purpose_interceptor;
  motivation[top_invisible_piece_id].last.acts_when = nbply;
  motivation[top_invisible_piece_id].levels.to = curr_decision_level;

  {
    square s;
    for (s = king_pos+dir;
         is_square_empty(s) && curr_decision_level<=max_decision_level;
         s += dir)
    {
      max_decision_level = decision_level_latest;

      /* use the taboo machinery to avoid attempting to intercept on the same
       * square in different iterations.
       * nbply minus 1 is correct - this taboo is equivalent to those deduced from
       * the previous move.
       */
      if (nr_taboos_for_current_move_in_ply[nbply-1][White][s]==0
          && nr_taboos_for_current_move_in_ply[nbply-1][Black][s]==0)
      {
        TraceSquare(s);
        TraceValue("%u",nr_taboos_accumulated_until_ply[White][s]);
        TraceValue("%u",nr_taboos_accumulated_until_ply[Black][s]);
        TraceEOL();
        if (!was_taboo(s))
        {
          // TODO if REPORT_DECISION_SQUARE() faked level 1 less, we could adjust
          // curr_decision_level outside of the loop
          REPORT_DECISION_SQUARE('>',s);
          ++curr_decision_level;
          place_interceptor_on_square(check_vectors,nr_check_vectors,s);
          --curr_decision_level;
        }
      }
      ++nr_taboos_for_current_move_in_ply[nbply-1][White][s];
      ++nr_taboos_for_current_move_in_ply[nbply-1][Black][s];
    }
    {
      square s2;
      for (s2 = king_pos+vec[kcurr]; s2!=s; s2 += vec[kcurr])
      {
        --nr_taboos_for_current_move_in_ply[nbply-1][White][s2];
        --nr_taboos_for_current_move_in_ply[nbply-1][Black][s2];
      }
    }

    TraceSquare(s);TraceEOL();

    motivation[top_invisible_piece_id] = motivation_null;
    --top_invisible_piece_id;

#if defined(REPORT_DECISIONS)
    if (report_decision_counter==save_counter)
    {
      REPORT_DECISION_DECLARE(PieceIdType const id_checker = GetPieceId(being_solved.spec[s]));
      REPORT_DECISION_DECLARE(ply const ply_check = motivation[id_checker].last.acts_when);
      REPORT_DECISION_OUTCOME("no available square found where to intercept check"
                              " from dir:%d"
                              " by id:%u"
                              " in ply:%u",
                              dir,
                              id_checker,
                              ply_check);
      REPORT_DEADEND;
    }
#endif
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void deal_with_illegal_checks(void)
{
  vec_index_type check_vectors[vec_queen_end-vec_queen_start+1];
  unsigned int nr_check_vectors = 0;
  vec_index_type kcurr;
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];
  unsigned int const nr_available = nr_placeable_invisibles_for_both_sides();

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (king_pos==initsquare)
    done_intercepting_illegal_checks();
  else
  {
    vec_index_type const k = is_square_attacked_by_uninterceptable(side_in_check,king_pos);
    if (k!=0)
    {
      int const dir_check = vec[k];
      Flags checkerSpec;

      uninterceptable_check_delivered_from = king_pos+dir_check;
      checkerSpec = being_solved.spec[uninterceptable_check_delivered_from];
      if (TSTFLAG(checkerSpec,Chameleon))
      {
        PieceIdType const id_checker = GetPieceId(checkerSpec);
        assert(uninterceptable_check_delivered_in_ply==ply_nil);
        uninterceptable_check_delivered_in_ply = motivation[id_checker].last.acts_when;

        REPORT_DECISION_OUTCOME("uninterceptable illegal check"
                                " from dir:%d"
                                " by id:%u"
                                " delivered in ply:%u",
                                dir_check,
                                id_checker,
                                uninterceptable_check_delivered_in_ply);

        TraceValue("%u",nbply);TraceEOL();
        if (nbply==ply_retro_move+1)
        {
          REPORT_DEADEND;
        }
        else
          restart_from_scratch();

        uninterceptable_check_delivered_in_ply = ply_nil;
      }
      else
      {
        REPORT_DECISION_OUTCOME("%s","uninterceptable check by visible piece");
        REPORT_DEADEND;
      }

      uninterceptable_check_delivered_from = initsquare;
    }
    else
    {
      /* there are only interceptable checks - deal with them */
      for (kcurr = vec_rook_start;
           kcurr<=vec_rook_end && nr_available>=nr_check_vectors;
           ++kcurr)
      {
        int const dir = vec[kcurr];
        square const sq_end = find_end_of_line(king_pos,dir);
        piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

        TraceValue("%d",dir);
        TraceSquare(sq_end);
        TraceWalk(walk_at_end);
        TraceEOL();

        if (TSTFLAG(being_solved.spec[sq_end],side_checking)
            && (walk_at_end==Queen || walk_at_end==Rook))
          check_vectors[nr_check_vectors++] = kcurr;
      }

      for (kcurr = vec_bishop_start;
           kcurr<=vec_bishop_end && nr_available>=nr_check_vectors;
           ++kcurr)
      {
        int const dir = vec[kcurr];
        square const sq_end = find_end_of_line(king_pos,dir);
        piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

        TraceValue("%d",dir);
        TraceSquare(sq_end);
        TraceWalk(walk_at_end);
        TraceEOL();

        if (TSTFLAG(being_solved.spec[sq_end],side_checking)
            && (walk_at_end==Queen || walk_at_end==Bishop))
          check_vectors[nr_check_vectors++] = kcurr;
      }

      TraceValue("%u",nr_available);
      TraceValue("%u",nr_check_vectors);
      TraceEOL();

      if (nr_check_vectors==0)
        done_intercepting_illegal_checks();
      else if (nr_available>=nr_check_vectors)
        place_interceptor_on_line(check_vectors,nr_check_vectors);
      else
      {
        TraceText("not enough available invisibles for intercepting all illegal checks\n");
        REPORT_DECISION_OUTCOME("%s","not enough available invisibles for intercepting all illegal checks");
        REPORT_DEADEND;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
