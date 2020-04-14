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
#include "optimisations/orthodox_square_observation.h"
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

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
      flesh_out_random_move_by_invisible();
    else if (sq_departure==capture_by_invisible
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
    validate_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_dummy_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                unsigned int nr_check_vectors,
                                boolean inserted_fleshed_out);

static void place_dummy_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                          unsigned int nr_check_vectors,
                                          square s,
                                          Side side,
                                          boolean inserted_fleshed_out)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParam("%u",inserted_fleshed_out);
  TraceFunctionParamListEnd();

  if (!(is_taboo(s,side) || was_taboo(s,side) || will_be_taboo(s,side)))
  {
    dynamic_consumption_type const save_consumption = current_consumption;
    PieceIdType const id_placed = GetPieceId(being_solved.spec[s]);

    assert(nr_check_vectors>0);

    if (allocate_placed(side))
    {
      push_decision_side(id_placed,side,decision_purpose_illegal_check_interceptor);

      remember_taboo_on_square(s,side,nbply);

      TraceSquare(s);TraceEnumerator(Side,trait[nbply-1]);TraceEOL();

      CLRFLAG(being_solved.spec[s],advers(side));

      TraceEnumerator(Side,side);
      TraceSquare(being_solved.king_square[side]);
      TraceValue("%u",nr_total_invisbles_consumed());
      TraceValue("%u",total_invisible_number);
      TraceEOL();
      if (being_solved.king_square[side]==initsquare
          && nr_total_invisbles_consumed()>=total_invisible_number)
      {
        TraceText("the only walk that can be allocated is King\n");
        allocate_flesh_out_placed(side);

        being_solved.king_square[side] = s;
        replace_walk(s,King);
        ++being_solved.number_of_pieces[side][King];
        SETFLAG(being_solved.spec[s],Royal);

        if (is_square_uninterceptably_observed_ortho(advers(side),s)==0)
        {
          if (nr_check_vectors==1)
            restart_from_scratch();
          else
            place_dummy_on_line(check_vectors,nr_check_vectors-1,true);
        }
        else
          record_decision_outcome("%s","can't place king because of self-check by uninterceptable");

        CLRFLAG(being_solved.spec[s],Royal);
        --being_solved.number_of_pieces[side][King];
        replace_walk(s,Dummy);
        being_solved.king_square[side] = initsquare;
      }
      else
      {
        if (nr_check_vectors==1)
        {
          if (inserted_fleshed_out)
            restart_from_scratch();
          else
            done_intercepting_illegal_checks();
        }
        else
          place_dummy_on_line(check_vectors,nr_check_vectors-1,inserted_fleshed_out);
      }

      SETFLAG(being_solved.spec[s],advers(side));

      pop_decision();

      current_consumption = save_consumption;

      if (side==White && can_decision_level_be_continued())
        place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,Black,inserted_fleshed_out);

      forget_taboo_on_square(s,side,nbply);
    }
    else
    {
      remember_taboo_on_square(s,side,nbply);

      record_decision_outcome("can't allocate an invisible of side %s for intercepting an illegal check",Side_names[side]);
      REPORT_DEADEND;

      current_consumption = save_consumption;

      if (side==White)
        place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,Black,inserted_fleshed_out);

      forget_taboo_on_square(s,side,nbply);
    }
  }
  else if (side==White)
    place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,Black,inserted_fleshed_out);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_dummy_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                  unsigned int nr_check_vectors,
                                  square s, numvec dir,
                                  boolean inserted_fleshed_out)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(s);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",inserted_fleshed_out);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  if (is_square_empty(s))
  {
    Flags spec = BIT(White)|BIT(Black)|BIT(Chameleon);
    PieceIdType const id_placed = initialise_motivation(purpose_interceptor,s,
                                                        purpose_interceptor,s);

    push_decision_placement(id_placed,s,decision_purpose_illegal_check_interceptor);

    decision_levels[id_placed].from = decision_level_forever;

    SetPieceId(spec,id_placed);
    occupy_square(s,Dummy,spec);

    decision_levels[id_placed].walk = decision_level_forever;

    place_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,White,inserted_fleshed_out);

    empty_square(s);

    pop_decision();

    uninitialise_motivation(id_placed);

    if (can_decision_level_be_continued())
      place_dummy_on_square(check_vectors,nr_check_vectors,s+dir,dir,inserted_fleshed_out);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_dummy_on_line(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                unsigned int nr_check_vectors,
                                boolean inserted_fleshed_out)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];
  vec_index_type const kcurr = check_vectors[nr_check_vectors-1];
  numvec const dir = vec[kcurr];
  unsigned long const save_counter = record_decision_counter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceFunctionParam("%u",inserted_fleshed_out);
  TraceFunctionParamListEnd();

  assert(nr_check_vectors>0);

  place_dummy_on_square(check_vectors,nr_check_vectors,king_pos+dir,dir,inserted_fleshed_out);

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

  push_decision_walk(id_placed,walk,decision_purpose_illegal_check_interceptor,side);

  if (nr_check_vectors==1)
    restart_from_scratch();
  else
    place_non_dummy_on_line(check_vectors,nr_check_vectors-1);

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
                                         square pos,
                                         PieceIdType id_placed,
                                         Side side)
{
  SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(pos);
  TraceFunctionParam("%u",id_placed);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if ((TSTFLAG(sq_spec(pos),basesq) || TSTFLAG(sq_spec(pos),promsq)))
  {
    record_decision_outcome("%s","pawn is placed on impossible square");
    REPORT_DEADEND;
  }
  else
  {
    ++being_solved.number_of_pieces[side][Pawn];

    assert(get_walk_of_piece_on_square(pos)==Dummy);
    replace_walk(pos,Pawn);

    if (pawn_check_ortho(side,being_solved.king_square[advers(side)])==0)
    {
      push_decision_walk(id_placed,Pawn,decision_purpose_illegal_check_interceptor,side);

      if (nr_check_vectors==1)
        restart_from_scratch();
      else
        place_non_dummy_on_line(check_vectors,nr_check_vectors-1);

      pop_decision();
    }

    TraceWalk(get_walk_of_piece_on_square(pos));
    TraceEOL();
    assert(get_walk_of_piece_on_square(pos)==Pawn);
    replace_walk(pos,Dummy);
    --being_solved.number_of_pieces[side][Pawn];
  }

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

    ++being_solved.number_of_pieces[side][King];

    assert(get_walk_of_piece_on_square(pos)==Dummy);
    replace_walk(pos,King);

    if (is_square_attacked_by_uninterceptable(side,pos))
    {
      record_decision_outcome("%s","capturer would expose itself to check by uninterceptable");
      REPORT_DEADEND;
    }
    else if (king_check_ortho(trait[nbply],being_solved.king_square[advers(trait[nbply])])==0)
    {
      push_decision_walk(id_placed,King,decision_purpose_illegal_check_interceptor,side);

      if (nr_check_vectors==1)
        restart_from_scratch();
      else
        place_non_dummy_on_line(check_vectors,nr_check_vectors-1);

      pop_decision();
    }

    TraceWalk(get_walk_of_piece_on_square(pos));
    TraceEOL();
    assert(get_walk_of_piece_on_square(pos)==King);
    replace_walk(pos,Dummy);
    --being_solved.number_of_pieces[side][King];

    CLRFLAG(being_solved.spec[pos],Royal);
  }

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

  being_solved.king_square[side] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_knight_of_checking_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                                    unsigned int nr_check_vectors,
                                                    square pos,
                                                    PieceIdType id_placed)
{
  Side const side_checking = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_check_vectors);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  ++being_solved.number_of_pieces[side_checking][Knight];

  assert(get_walk_of_piece_on_square(pos)==Dummy);
  replace_walk(pos,Knight);

  if (knight_check_ortho(side_checking,being_solved.king_square[advers(side_checking)])==0)
  {
    push_decision_walk(id_placed,Knight,decision_purpose_illegal_check_interceptor,side_checking);

    if (nr_check_vectors==1)
      restart_from_scratch();
    else
      place_non_dummy_on_line(check_vectors,nr_check_vectors-1);

    pop_decision();
  }

  TraceWalk(get_walk_of_piece_on_square(pos));
  TraceEOL();
  assert(get_walk_of_piece_on_square(pos)==Knight);
  replace_walk(pos,Dummy);
  --being_solved.number_of_pieces[side_checking][Knight];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_piece_of_side_on_square(vec_index_type const check_vectors[vec_queen_end-vec_queen_start+1],
                                          unsigned int nr_check_vectors,
                                          Side side,
                                          square pos,
                                          PieceIdType id_placed)
{
  unsigned long const save_counter = record_decision_counter;

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
          place_pawn_of_side_on_square(check_vectors,nr_check_vectors,pos,id_placed,side);

        if (can_decision_level_be_continued())
          place_knight_of_checking_side_on_square(check_vectors,nr_check_vectors,pos,id_placed);

        if (can_decision_level_be_continued())
        {
          vec_index_type const k = check_vectors[nr_check_vectors-1];
          boolean const is_check_orthogonal = k<=vec_rook_end;

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
          piece_walk_type const walk_order_after_pawn[] =
          {
              Knight,
              Rook,
              Bishop
          };
          enum { nr_walks = sizeof walk_order_after_pawn / sizeof walk_order_after_pawn[0] };

          boolean walk_ruled_out[Bishop+1] = { false };

          place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,Queen);

          if (has_decision_failed_capture())
          {
            walk_ruled_out[Rook] = true;
            walk_ruled_out[Bishop] = true;
          }
          else if (can_decision_level_be_continued())
            place_pawn_of_side_on_square(check_vectors,nr_check_vectors,pos,id_placed,side);

          {
            unsigned int i;
            for (i = 0;
                 i!=nr_walks && can_decision_level_be_continued();
                 ++i)
            {
              piece_walk_type const walk = walk_order_after_pawn[i];
              if (!walk_ruled_out[walk])
                place_piece_of_any_walk_of_side_on_square(check_vectors,nr_check_vectors,side,pos,id_placed,walk);
            }
          }
        }
      }
    }

    if (record_decision_counter==save_counter)
    {
      record_decision_outcome("%s","not enough available invisibles for intercepting all illegal checks");
      REPORT_DEADEND;
      backtrack_from_failure_to_intercept_illegal_check(trait[nbply-1],nr_check_vectors,check_vectors[0]);
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

    push_decision_side(id_placed,side,decision_purpose_illegal_check_interceptor);

    CLRFLAG(being_solved.spec[s],advers(side));
    place_piece_of_side_on_square(check_vectors,nr_check_vectors,side,s,id_placed);
    SETFLAG(being_solved.spec[s],advers(side));

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

    push_decision_placement(id_placed,s,decision_purpose_illegal_check_interceptor);

    decision_levels[id_placed].from = decision_level_forever;

    SetPieceId(spec,id_placed);
    occupy_square(s,Dummy,spec);

    place_non_dummy_of_side_on_square(check_vectors,nr_check_vectors,s,preferred_side,id_placed);

    empty_square(s);

    pop_decision();

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
  unsigned long const save_counter = record_decision_counter;

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

static void collect_illegal_checks_by_interceptable(vec_index_type start, vec_index_type end,
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
       kcurr<=end && nr_available+1>=*nr_check_vectors;
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
    {
      Flags const flags_checker = being_solved.spec[sq_end];

      if (TSTFLAG(flags_checker,Chameleon))
      {
        PieceIdType const id_checker = GetPieceId(flags_checker);
        decision_make_relevant(decision_levels[id_checker].to);
        decision_make_relevant(decision_levels[id_checker].walk);
      }

      check_vectors[(*nr_check_vectors)++] = kcurr;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_illegal_checks_by_interceptables(void)
{
  unsigned int const nr_available = nr_placeable_invisibles_for_both_sides();
  vec_index_type check_vectors[vec_queen_end-vec_queen_start+1];
  unsigned int nr_check_vectors = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  collect_illegal_checks_by_interceptable(vec_rook_start,vec_rook_end,
                                          Rook,
                                          check_vectors,&nr_check_vectors);
  collect_illegal_checks_by_interceptable(vec_bishop_start,vec_bishop_end,
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
      place_dummy_on_line(check_vectors,nr_check_vectors,false);
    else
      place_non_dummy_on_line(check_vectors,nr_check_vectors);
  }
  else
  {
    Side const side_in_check = trait[nbply-1];
    TraceText("not enough available invisibles for intercepting all illegal checks\n");
    backtrack_from_failure_to_intercept_illegal_check(side_in_check,nr_check_vectors,check_vectors[0]);
    record_decision_outcome("%s %d %s %d %s","only",nr_available,"available invisibles for intercepting",nr_check_vectors,"illegal checks");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_illegal_check_by_uninterceptable(vec_index_type k)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];
  numvec const dir_check = vec[k];
  Flags checkerSpec;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",k);
  TraceFunctionParamListEnd();

  check_by_uninterceptable_delivered_from = king_pos+dir_check;
  checkerSpec = being_solved.spec[check_by_uninterceptable_delivered_from];

  TraceSquare(check_by_uninterceptable_delivered_from);
  TraceValue("%x",checkerSpec);
  TraceEOL();

  if (TSTFLAG(checkerSpec,Chameleon))
  {
    PieceIdType const id_checker = GetPieceId(checkerSpec);

    assert(check_by_uninterceptable_delivered_in_ply==ply_nil);
    check_by_uninterceptable_delivered_in_ply = nbply;

    record_decision_outcome("illegal check by uninterceptable invisible piece"
                            " from dir:%d"
                            " by id:%u",
                            dir_check,
                            id_checker);

    check_by_uninterceptable_delivered_in_level = decision_levels[id_checker].walk;
    if (nbply<=motivation[id_checker].last.acts_when)
    {
      if (check_by_uninterceptable_delivered_in_level<decision_levels[id_checker].from)
        check_by_uninterceptable_delivered_in_level = decision_levels[id_checker].from;
    }
    else
    {
      if (check_by_uninterceptable_delivered_in_level<decision_levels[id_checker].to)
        check_by_uninterceptable_delivered_in_level = decision_levels[id_checker].to;
    }

    /* taking .from into consideration is relevant if the checker is inserted for a late ply,
     * e.g. as a mating piece attacker:
begin
author Ricardo Vieira, Ofer Comay, Menachem Witztum, Paz Einat
origin Sake tourney 2018, 6th HM
pieces TotalInvisible 5 white kh1 rh6 bb7g5 sg3 pe2f2f4 black kg4 rd1e1 bb3g1 pe3e5
stipulation h#2
option movenum start 12:7:45:43
end

Ricardo Vieira, Ofer Comay, Menachem Witztum, Paz Einat
      Sake tourney 2018, 6th HM

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   B   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   R   6
|                                   |
5   .   .   .   .  -P   .   B   .   5
|                                   |
4   .   .   .   .   .   P  -K   .   4
|                                   |
3   .  -B   .   .  -P   .   S   .   3
|                                   |
2   .   .   .   .   P   P   .   .   2
|                                   |
1   .   .   .  -R  -R   .  -B   K   1
|                                   |
+---a---b---c---d---e---f---g---h---+
  h#2                  8 + 7 + 5 TI

!test_mate 6:Rd1-d8 7:Kh1-g2 8:Rd8-a8 9:Bb7-f3 - total_invisible.c:#546 - D:4 - 0
use option start 12:7:45:43 to replay
!  2 - combined result:3
! >2 # 10 B (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 - attack_mating_piece.c:#107 - D:6
! <2 - r:3 t:0 m:4294967295
! >2 # 10 R (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 - attack_mating_piece.c:#107 - D:8
! <2 - r:3 t:0 m:4294967295
! >2 # 10 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - r:1 t:0 m:4294967295 - attack_mating_piece.c:#149 - D:10
!  >3 # 10 I (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 - attack_mating_piece.c:#39 - D:12
!   >4 # 10 h4 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 - attack_mating_piece.c:#41 - D:14
!     5 8 uninterceptable illegal check by invisible piece from dir:49 by id:16 delivered in ply:10 - intercept_illegal_checks.c:#658
!     5 8 piece delivering uninterceptable check can't be captured by random move - total_invisible.c:#97
!   <4 - r:1 t:1 m:4
!  <3 - r:1 t:1 m:4

HERE

!  >3 # 10 I (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 - attack_mating_piece.c:#39 - D:16
!   >4 # 10 h2 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - r:1 t:0 m:4294967295 - attack_mating_piece.c:#41 - D:18
!     5 10 placed mating piece attacker - king_placement.c:#70
!     5 - combined result:5
!   <4 - r:5 t:1 m:1
!  <3 - r:5 t:1 m:1
! <2 - r:5 t:1 m:1

     */

    TraceValue("%u",nbply);TraceEOL();
    if (nbply==ply_retro_move+1)
    {
      REPORT_DEADEND;
      backtrack_definitively();
      backtrack_no_further_than(check_by_uninterceptable_delivered_in_level);
    }
    else
      restart_from_scratch();

    check_by_uninterceptable_delivered_in_level = decision_level_uninitialised;

    check_by_uninterceptable_delivered_in_ply = ply_nil;
  }
  else
  {
    record_decision_outcome("%s","check by uninterceptable visible piece");
    REPORT_DEADEND;
  }

  check_by_uninterceptable_delivered_from = initsquare;

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
      deal_with_illegal_check_by_uninterceptable(k);
    else
      deal_with_illegal_checks_by_interceptables();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
