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

    TraceValue("%u",nbply);
    TraceValue("%u",top_ply_of_regular_play);
    TraceSquare(sq_departure);
    TraceValue("%u",sq_departure);
    TraceValue("%u",capture_by_invisible);
    TraceSquare(sq_arrival);
    TraceValue("%u",sq_arrival);
    TraceValue("%u",move_by_invisible);
    TraceEOL();

    {
      PieceIdType id;
      for (id = get_top_visible_piece_id()+1; id<=get_top_invisible_piece_id(); ++id)
      {
        TraceValue("%u",id);TraceEOL();
        TraceAction(&motivation[id].first);TraceEOL();
        TraceAction(&motivation[id].last);TraceEOL();
        TraceWalk(get_walk_of_piece_on_square(motivation[id].last.on));
        TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
        TraceEOL();
      }
    }

    if (is_capture_by_invisible_possible())
    {
      if (sq_departure==move_by_invisible
          && sq_arrival==move_by_invisible)
        flesh_out_random_move_by_invisible();
      else if (sq_departure>=capture_by_invisible
               && is_on_board(sq_arrival))
        flesh_out_capture_by_invisible();
      else
      {
        square const first_taboo_violation = find_taboo_violation();
        if (first_taboo_violation==nullsquare)
          insert_invisible_capturer();
        else
        {
          // TODO review
//        assert(is_taboo_violation_acceptable(first_taboo_violation));
        }
      }
    }
    else
    {
      record_decision_outcome("capture in ply %u will not be possible",nbply+1);
      REPORT_DEADEND;
    }
  }
  else
    validate_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_dummy_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                unsigned int nr_check_vectors);

static void place_dummy_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                          unsigned int nr_check_vectors,
                                          square s,
                                          Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if (!(is_taboo(s,side) || was_taboo(s,side) || will_be_taboo(s,side)))
  {
    dynamic_consumption_type const save_consumption = current_consumption;
    PieceIdType const id_placed = GetPieceId(being_solved.spec[s]);

    assert(nr_check_vectors>0);

    push_decision_insertion(id_placed,side,decision_purpose_illegal_check_interceptor);
    decision_levels[id_placed].side = curr_decision_level;

    push_decision_side(id_placed,side,decision_purpose_illegal_check_interceptor);
    decision_levels[id_placed].side = curr_decision_level;

    push_decision_arrival(id_placed,s,decision_purpose_illegal_check_interceptor);
    decision_levels[id_placed].to = curr_decision_level;

    if (allocate_placed(side))
    {
      remember_taboo_on_square(s,side,nbply);

      TraceSquare(s);TraceEnumerator(Side,trait[nbply-1]);TraceEOL();

      CLRFLAG(being_solved.spec[s],advers(side));

      if (nr_check_vectors==1)
        done_intercepting_illegal_checks();
      else
        place_dummy_on_line(check_vectors,nr_check_vectors-1);

      SETFLAG(being_solved.spec[s],advers(side));

      current_consumption = save_consumption;

      pop_decision();

      pop_decision();

      pop_decision();

      if (side==White && can_decision_level_be_continued())
      {
        max_decision_level = decision_level_latest;
        place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,Black);
      }

      forget_taboo_on_square(s,side,nbply);
    }
    else
    {
      remember_taboo_on_square(s,side,nbply);

      record_decision_outcome("not enough available invisibles of side %s for intercepting all illegal checks",Side_names[side]);
      REPORT_DEADEND;

      pop_decision();

      pop_decision();

      pop_decision();

      current_consumption = save_consumption;

      if (side==White)
        place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,Black);

      forget_taboo_on_square(s,side,nbply);
    }
  }
  else if (side==White)
    place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_dummy_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                  unsigned int nr_check_vectors,
                                  square s, numvec dir)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  if (is_square_empty(s))
  {
    Flags spec = BIT(White)|BIT(Black)|BIT(Chameleon);
    PieceIdType const id_placed = initialise_motivation(purpose_interceptor,s,
                                                        purpose_interceptor,s);

    max_decision_level = decision_level_latest;

    decision_levels[id_placed].from = decision_level_latest;

    SetPieceId(spec,id_placed);
    occupy_square(s,Dummy,spec);

    decision_levels[id_placed].walk = decision_level_latest;

    place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,White);

    empty_square(s);

    uninitialise_motivation(id_placed);

    if (can_decision_level_be_continued())
      place_dummy_on_square(check_vectors,nr_check_vectors,s+dir,dir);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_dummy_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                unsigned int nr_check_vectors)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];
  vec_index_type const kcurr = check_vectors[nr_check_vectors-1];
  numvec const dir = vec[kcurr];
  unsigned int const save_counter = record_decision_counter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  place_dummy_on_square(check_vectors,nr_check_vectors,king_pos+dir,dir);

  if (record_decision_counter==save_counter)
  {
    square const s = find_end_of_line(king_pos,dir);
    PieceIdType const id_checker = GetPieceId(being_solved.spec[s]);
    ply const ply_check = motivation[id_checker].last.acts_when;
    record_decision_outcome("no available square found where to intercept check"
                            " from dir:%d"
                            " by id:%u"
                            " in ply:%u",
                            dir,
                            id_checker,
                            ply_check);
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_non_dummy_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                    unsigned int nr_check_vectors);

static void place_piece_of_any_walk_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                                      unsigned int nr_check_vectors,
                                                      Side side,
                                                      square pos,
                                                      PieceIdType id_placed,
                                                      piece_walk_type walk)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  ++being_solved.number_of_pieces[side][walk];

  assert(get_walk_of_piece_on_square(pos)==Dummy);
  replace_walk(pos,walk);
  push_decision_walk(id_placed,walk,decision_purpose_illegal_check_interceptor);
  decision_levels[id_placed].walk = curr_decision_level;

  {
    Side const side_attacked = advers(side);
    square const king_pos = being_solved.king_square[side_attacked];
    vec_index_type const k = is_square_uninterceptably_attacked(side_attacked,
                                                                king_pos);
    if (k==UINT_MAX)
    {
      // TODO accept uninterceptable check if not illegal
      record_decision_outcome("%s","can't allocate necessary interceptor");
      REPORT_DEADEND;
      backtrack_from_failure_to_intercept_illegal_checks(side_attacked);
    }
    else if (k==0 || king_pos+vec[k]!=pos)
    {
      if (nr_check_vectors==1)
        restart_from_scratch();
      else
        place_non_dummy_on_line(check_vectors,nr_check_vectors-1);
    }
    else
    {
      // TODO accept uninterceptable check if not illegal
      record_decision_outcome("%s","interceptor delivers uninterceptable check - TODO: not necessarily a deadend");
      REPORT_DEADEND;
      backtrack_from_failure_to_intercept_illegal_checks(side_attacked);
    }
  }

  pop_decision();

  TraceWalk(get_walk_of_piece_on_square(pos));
  TraceWalk(walk);
  TraceEOL();
  assert(get_walk_of_piece_on_square(pos)==walk);
  replace_walk(pos,Dummy);
  --being_solved.number_of_pieces[side][walk];

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_pawn_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                         unsigned int nr_check_vectors,
                                         Side side,
                                         square pos,
                                         PieceIdType id_placed)
{
  SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  if ((TSTFLAG(sq_spec[pos],basesq) || TSTFLAG(sq_spec[pos],promsq)))
  {
    record_decision_outcome("%s","pawn is placed on impossible square");
    REPORT_DEADEND;
  }
  else
    place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,Pawn);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_king_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                         unsigned int nr_check_vectors,
                                         Side side,
                                         square pos,
                                         PieceIdType id_placed)
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
    SETFLAG(being_solved.spec[pos],Royal);
    place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,King);
    CLRFLAG(being_solved.spec[pos],Royal);
  }

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

  being_solved.king_square[side] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_piece_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                          unsigned int nr_check_vectors,
                                          Side side,
                                          square pos,
                                          PieceIdType id_placed)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParam("%u",id_placed);
  TraceFunctionParamListEnd();

  TraceSquare(being_solved.king_square[side]);
  TraceEOL();

  if (being_solved.king_square[side]==initsquare)
    place_king_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed);

  {
    dynamic_consumption_type const save_consumption = current_consumption;

    if (allocate_flesh_out_unplaced(side))
    {
      if (side==trait[nbply])
      {
        if (can_decision_level_be_continued())
        {
          max_decision_level = decision_level_latest;
          place_pawn_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed);
        }

        if (can_decision_level_be_continued())
        {
          max_decision_level = decision_level_latest;
          place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,Knight);
        }

        if (can_decision_level_be_continued())
        {
          vec_index_type const k = check_vectors[nr_check_vectors-1];
          boolean const is_check_orthogonal = k<=vec_rook_end;

          max_decision_level = decision_level_latest;

          if (is_check_orthogonal)
            place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,Bishop);
          else
            place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,Rook);
        }
      }
      else
      {
        if (can_decision_level_be_continued())
        {
          piece_walk_type const walk_order_after_queen[] =
          {
              Pawn,
              Knight,
              Rook,
              Bishop
          };
          enum { nr_walks = sizeof walk_order_after_queen / sizeof walk_order_after_queen[0] };

          boolean walk_ruled_out[Bishop+1] = { false };

          capture_by_invisible_failed_with_this_walk[curr_decision_level] = false;

          max_decision_level = decision_level_latest;
          place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,Queen);

          if (capture_by_invisible_failed_with_this_walk[curr_decision_level])
          {
            walk_ruled_out[Pawn] = true;
            walk_ruled_out[Rook] = true;
            walk_ruled_out[Bishop] = true;
          }

          {
            unsigned int i;
            for (i = 0;
                 i<nr_walks && can_decision_level_be_continued();
                 ++i)
            {
              piece_walk_type const walk = walk_order_after_queen[i];
              if (!walk_ruled_out[walk])
              {
                max_decision_level = decision_level_latest;
                place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,walk);
              }
            }
          }
        }
      }
    }
    else
    {
      record_decision_outcome("%s","not enough available invisibles for intercepting all illegal checks");
      REPORT_DEADEND;
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_non_dummy_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                              unsigned int nr_check_vectors,
                                              square s,
                                              Side side,
                                              PieceIdType id_placed)
{
  Side const preferred_side = trait[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParam("%u",id_placed);
  TraceFunctionParamListEnd();

  if (!(is_taboo(s,side) || was_taboo(s,side) || will_be_taboo(s,side)))
  {
    remember_taboo_on_square(s,side,nbply);

    max_decision_level = decision_level_latest;

    push_decision_insertion(id_placed,side,decision_purpose_illegal_check_interceptor);
    decision_levels[id_placed].side = curr_decision_level;

    push_decision_side(id_placed,side,decision_purpose_illegal_check_interceptor);
    decision_levels[id_placed].side = curr_decision_level;

    push_decision_arrival(id_placed,s,decision_purpose_illegal_check_interceptor);
    decision_levels[id_placed].to = curr_decision_level;

    CLRFLAG(being_solved.spec[s],advers(side));
    place_piece_of_side_on_square(check_vectors,nr_check_vectors,side,s,id_placed);
    SETFLAG(being_solved.spec[s],advers(side));

    pop_decision();

    pop_decision();

    pop_decision();

    if (side==preferred_side && can_decision_level_be_continued())
      place_non_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,advers(preferred_side),id_placed);

    forget_taboo_on_square(s,side,nbply);
  }
  else if (side==preferred_side)
    place_non_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,advers(preferred_side),id_placed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_non_dummy_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                      unsigned int nr_check_vectors,
                                      square s, numvec dir)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  if (is_square_empty(s))
  {
    Side const preferred_side = trait[nbply-1];
    Flags spec = BIT(White)|BIT(Black)|BIT(Chameleon);
    PieceIdType const id_placed = initialise_motivation(purpose_interceptor,s,
                                                        purpose_interceptor,s);

    max_decision_level = decision_level_latest;

    decision_levels[id_placed].from = decision_level_latest;

    SetPieceId(spec,id_placed);
    occupy_square(s,Dummy,spec);

    place_non_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,preferred_side,id_placed);

    empty_square(s);

    uninitialise_motivation(id_placed);

    if (can_decision_level_be_continued())
      place_non_dummy_on_square(check_vectors,nr_check_vectors,s+dir,dir);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_non_dummy_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                    unsigned int nr_check_vectors)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];
  vec_index_type const kcurr = check_vectors[nr_check_vectors-1];
  numvec const dir = vec[kcurr];
  unsigned int const save_counter = record_decision_counter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  place_non_dummy_on_square(check_vectors,nr_check_vectors,king_pos+dir,dir);

  if (record_decision_counter==save_counter)
  {
    square const s = find_end_of_line(king_pos,dir);
    PieceIdType const id_checker = GetPieceId(being_solved.spec[s]);
    ply const ply_check = motivation[id_checker].last.acts_when;

    record_decision_outcome("no available square found where to intercept check"
                            " from dir:%d"
                            " by id:%u"
                            " in ply:%u",
                            dir,
                            id_checker,
                            ply_check);
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void collect_interceptable_illegal_checks(vec_index_type start, vec_index_type end,
                                                 piece_walk_type walk_checker,
                                                 vec_index_type check_vectors[vec_queen_end-vec_queen_start+1],
                                                 unsigned int *nr_check_vectors)
{
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];
  unsigned int const nr_available = nr_placeable_invisibles_for_both_sides();
  vec_index_type kcurr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",end);
  TraceWalk(walk_checker);
  TraceFunctionParamListEnd();

  for (kcurr = start;
       kcurr<=end && nr_available>=*nr_check_vectors;
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
        && (walk_at_end==Queen || walk_at_end==walk_checker))
      check_vectors[(*nr_check_vectors)++] = kcurr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_interceptable_illegal_checks()
{
  unsigned int const nr_available = nr_placeable_invisibles_for_both_sides();
  vec_index_type check_vectors[vec_queen_end-vec_queen_start+1];
  unsigned int nr_check_vectors = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  collect_interceptable_illegal_checks(vec_rook_start,vec_rook_end,
                                       Rook,
                                       check_vectors,&nr_check_vectors);
  collect_interceptable_illegal_checks(vec_bishop_start,vec_bishop_end,
                                       Bishop,
                                       check_vectors,&nr_check_vectors);

  TraceValue("%u",nr_available);
  TraceValue("%u",nr_check_vectors);
  TraceEOL();

  if (nr_check_vectors==0)
    done_intercepting_illegal_checks();
  else if (nr_available>=nr_check_vectors)
  {
    if (play_phase==play_validating_mate)
      place_dummy_on_line(check_vectors,nr_check_vectors);
    else
      place_non_dummy_on_line(check_vectors,nr_check_vectors);
  }
  else
  {
    TraceText("not enough available invisibles for intercepting all illegal checks\n");
    backtrack_from_failure_to_intercept_illegal_checks(trait[nbply-1]);
    record_decision_outcome("%s","not enough available invisibles for intercepting all illegal checks");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_uninterceptable_illegal_check(vec_index_type k)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];
  numvec const dir_check = vec[k];
  Flags checkerSpec;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",k);
  TraceFunctionParamListEnd();

  uninterceptable_check_delivered_from = king_pos+dir_check;
  checkerSpec = being_solved.spec[uninterceptable_check_delivered_from];

  TraceSquare(uninterceptable_check_delivered_from);
  TraceValue("%x",checkerSpec);
  TraceEOL();

  if (TSTFLAG(checkerSpec,Chameleon))
  {
    PieceIdType const id_checker = GetPieceId(checkerSpec);

    assert(uninterceptable_check_delivered_in_ply==ply_nil);
    uninterceptable_check_delivered_in_ply = motivation[id_checker].last.acts_when;

    record_decision_outcome("uninterceptable illegal check"
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
    record_decision_outcome("%s","uninterceptable check by visible piece");
    REPORT_DEADEND;
  }

  uninterceptable_check_delivered_from = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void deal_with_illegal_checks(void)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (king_pos==initsquare)
    done_intercepting_illegal_checks();
  else
  {
    Side const side_in_check = trait[nbply-1];
    square const king_pos = being_solved.king_square[side_in_check];
    vec_index_type const k = is_square_attacked_by_uninterceptable(side_in_check,king_pos);

    if (k!=0)
      deal_with_uninterceptable_illegal_check(k);
    else
      deal_with_interceptable_illegal_checks();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
