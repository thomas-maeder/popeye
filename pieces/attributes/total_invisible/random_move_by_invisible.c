#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "solving/move_effect_journal.h"
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static void done_forward_random_move_by_invisible_from(boolean is_dummy_moving)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  PieceIdType const id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
  motivation_type const save_motivation = motivation[id];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  push_decision_arrival(id,sq_arrival,decision_purpose_random_mover_forward);

  motivation[id].last.on = sq_arrival;

  remember_taboos_for_current_move();
  if (is_dummy_moving)
    restart_from_scratch();
  else
    recurse_into_child_ply();
  forget_taboos_for_current_move();

  motivation[id] = save_motivation;

  pop_decision();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_accidental_capture_by_invisible(boolean is_dummy_moving)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  if (is_on_board(sq_arrival)
      && TSTFLAG(being_solved.spec[sq_arrival],advers(trait[nbply]))
      && TSTFLAG(being_solved.spec[sq_arrival],Chameleon))
  {
    PieceIdType const id_victim = GetPieceId(being_solved.spec[sq_arrival]);

    TraceValue("%u",id_victim);TraceEOL();
    TraceAction(&motivation[id_victim].first);TraceEOL();
    TraceAction(&motivation[id_victim].last);TraceEOL();

    assert(motivation[id_victim].first.purpose!=purpose_none);
    assert(motivation[id_victim].last.purpose!=purpose_none);
    if (motivation[id_victim].last.acts_when>nbply)
    {
      TraceText("the planned victim was added to later act from its current square\n");
      record_decision_outcome("%s","the planned victim was added to later act from its current square");
      REPORT_DEADEND;
    }
    else
    {
      PieceIdType const id_captured = GetPieceId(being_solved.spec[sq_arrival]);
      action_type const save_last = motivation[id_captured].last;

      motivation[id_captured].last.purpose = purpose_none;

      assert(move_effect_journal[capture].type==move_effect_no_piece_removal);
      move_effect_journal[capture].type = move_effect_piece_removal;
      move_effect_journal[capture].u.piece_removal.on = sq_arrival;
      move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(sq_arrival);
      move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[sq_arrival];

      done_forward_random_move_by_invisible_from(is_dummy_moving);

      move_effect_journal[capture].type = move_effect_no_piece_removal;

      motivation[id_captured].last = save_last;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_invisible_pawn_from(boolean is_dummy_moving)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  Side const side = trait[nbply];
  int const dir = side==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  {
    square const sq_singlestep = move_effect_journal[movement].u.piece_movement.from+dir;
    TraceSquare(sq_singlestep);TraceEOL();
    if (is_square_empty(sq_singlestep))
    {
      if (!will_be_taboo(sq_singlestep,side))
      {
        move_effect_journal[movement].u.piece_movement.to = sq_singlestep;
        // TODO promotion
        done_forward_random_move_by_invisible_from(is_dummy_moving);
      }

      if (can_decision_level_be_continued())
      {
        SquareFlags const doublstepsq = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
        if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublstepsq))
        {
          square const sq_doublestep = sq_singlestep+dir;
          TraceSquare(sq_doublestep);TraceEOL();
          if (is_square_empty(sq_doublestep))
          {
            if (!will_be_taboo(sq_doublestep,side))
            {
              move_effect_journal[movement].u.piece_movement.to = sq_doublestep;
              done_forward_random_move_by_invisible_from(is_dummy_moving);
            }
          }
        }
      }
    }

    // TODO add capture victim if arrival square empty and nr_total...>0

    if (can_decision_level_be_continued())
    {
      square const sq_arrival = sq_singlestep+dir_right;

      if (!will_be_taboo(sq_arrival,trait[nbply]))
      {
        move_effect_journal[movement].u.piece_movement.to = sq_arrival;
        forward_accidental_capture_by_invisible(is_dummy_moving);
      }
    }

    if (can_decision_level_be_continued())
    {
      square const sq_arrival = sq_singlestep+dir_left;

      if (!will_be_taboo(sq_arrival,trait[nbply]))
      {
        move_effect_journal[movement].u.piece_movement.to = sq_arrival;
        forward_accidental_capture_by_invisible(is_dummy_moving);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_invisible_rider_from(vec_index_type kstart,
                                                        vec_index_type kend,
                                                        boolean is_dummy_moving)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceValue("%u",kstart);
  TraceValue("%u",kend);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && can_decision_level_be_continued(); ++k)
  {
    square sq_arrival;

    TraceValue("%u",k);TraceValue("%d",vec[k]);TraceEOL();

    for (sq_arrival = move_effect_journal[movement].u.piece_movement.from+vec[k];
         is_on_board(sq_arrival) && can_decision_level_be_continued();
         sq_arrival += vec[k])
    {
      TraceSquare(sq_arrival);TraceEOL();
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;

      /* "factoring out" the invokations of is_taboo() is tempting, but we
       * want to break the loop if sq_arrival is not empty whether or not
       * that square is taboo!
       */
      if (is_square_empty(sq_arrival))
      {
        if (!will_be_taboo(sq_arrival,trait[nbply]))
          done_forward_random_move_by_invisible_from(is_dummy_moving);
      }
      else
      {
        if (!will_be_taboo(sq_arrival,trait[nbply]))
          forward_accidental_capture_by_invisible(is_dummy_moving);
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_invisible_leaper_from(vec_index_type kstart,
                                                           vec_index_type kend,
                                                           boolean is_dummy_moving)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kstart);
  TraceFunctionParam("%u",kend);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  TraceWalk(get_walk_of_piece_on_square(sq_departure));TraceEOL();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && can_decision_level_be_continued(); ++k)
  {
    square const sq_arrival = sq_departure+vec[k];
    if (!will_be_taboo(sq_arrival,trait[nbply]))
    {
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      /* just in case: */
      move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;

      if (is_square_empty(sq_arrival))
        done_forward_random_move_by_invisible_from(is_dummy_moving);
      else
        forward_accidental_capture_by_invisible(is_dummy_moving);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_existing_invisible_from(square sq_departure,
                                                           boolean is_dummy_moving)
{
  piece_walk_type const walk_on_square = get_walk_of_piece_on_square(sq_departure);
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  piece_walk_type const save_walk_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_flags_moving = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  TraceWalk(walk_on_square);TraceEOL();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  move_effect_journal[movement].u.piece_movement.moving = walk_on_square;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

  switch (walk_on_square)
  {
    case King:
      assert(move_effect_journal[king_square_movement].type==move_effect_none);
      move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
      move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
      move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];
      forward_random_move_by_invisible_leaper_from(vec_queen_start,vec_queen_end,
                                                   is_dummy_moving);
      move_effect_journal[king_square_movement].type = move_effect_none;
      break;

    case Queen:
      forward_random_move_by_invisible_rider_from(vec_queen_start,vec_queen_end,
                                                  is_dummy_moving);
      break;

    case Rook:
      forward_random_move_by_invisible_rider_from(vec_rook_start,vec_rook_end,
                                                  is_dummy_moving);
      break;

    case Bishop:
      forward_random_move_by_invisible_rider_from(vec_bishop_start,vec_bishop_end,
                                                  is_dummy_moving);
      break;

    case Knight:
      forward_random_move_by_invisible_leaper_from(vec_knight_start,vec_knight_end,
                                                   is_dummy_moving);
      break;

    case Pawn:
      forward_random_move_by_invisible_pawn_from(is_dummy_moving);
      break;

    default:
      break;
  }

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.moving = save_walk_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_flags_moving;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_existing_invisible_as_non_king_from(square sq_departure)
{
  Side const side_playing = trait[nbply];
  Side const side_under_attack = advers(side_playing);
  square const king_pos = being_solved.king_square[side_under_attack];

  PieceIdType const id_moving = GetPieceId(being_solved.spec[sq_departure]);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  {
    SquareFlags const promsq = side_playing==White ? WhPromSq : BlPromSq;
    SquareFlags const basesq = side_playing==White ? WhBaseSq : BlBaseSq;
    if (!(TSTFLAG(sq_spec[sq_departure],basesq) || TSTFLAG(sq_spec[sq_departure],promsq)))
    {
      push_decision_walk(id_moving,Pawn,decision_purpose_random_mover_forward,trait[nbply]);

      ++being_solved.number_of_pieces[side_playing][Pawn];
      replace_walk(sq_departure,Pawn);
      if (!(king_pos!=initsquare && pawn_check_ortho(side_playing,king_pos)))
        forward_random_move_by_existing_invisible_from(sq_departure,true);
      --being_solved.number_of_pieces[side_playing][Pawn];

      pop_decision();
    }
  }

  if (can_decision_level_be_continued())
  {
    push_decision_walk(id_moving,Knight,decision_purpose_random_mover_forward,trait[nbply]);

    ++being_solved.number_of_pieces[side_playing][Knight];
    replace_walk(sq_departure,Knight);
    if (!(king_pos!=initsquare && knight_check_ortho(side_playing,king_pos)))
      forward_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Knight];

    pop_decision();
  }

  if (can_decision_level_be_continued())
  {
    push_decision_walk(id_moving,Bishop,decision_purpose_random_mover_forward,trait[nbply]);

    ++being_solved.number_of_pieces[side_playing][Bishop];
    replace_walk(sq_departure,Bishop);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_bishop_start,vec_bishop_end,
                                        Bishop))
      forward_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Bishop];

    pop_decision();
  }

  if (can_decision_level_be_continued())
  {
    push_decision_walk(id_moving,Rook,decision_purpose_random_mover_forward,trait[nbply]);

    ++being_solved.number_of_pieces[side_playing][Rook];
    replace_walk(sq_departure,Rook);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_rook_start,vec_rook_end,
                                        Rook))
      forward_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Rook];

    pop_decision();
  }

  if (can_decision_level_be_continued())
  {
    push_decision_walk(id_moving,Queen,decision_purpose_random_mover_forward,trait[nbply]);

    ++being_solved.number_of_pieces[side_playing][Queen];
    replace_walk(sq_departure,Queen);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_queen_start,vec_queen_end,
                                        Queen))
      forward_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Queen];

    pop_decision();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_specific_invisible_from(square sq_departure)
{
  piece_walk_type const save_walk_moving = get_walk_of_piece_on_square(sq_departure);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  TraceWalk(save_walk_moving);
  TraceSquare(sq_departure);
  TraceEOL();

  // TODO use a sibling ply and the regular move generation machinery?

  if (save_walk_moving==Dummy)
  {
    Side const side_playing = trait[nbply];
    Side const side_under_attack = advers(side_playing);
    square const king_pos = being_solved.king_square[side_under_attack];
    dynamic_consumption_type const save_consumption = current_consumption;

    assert(play_phase==play_validating_mate);

    assert(TSTFLAG(being_solved.spec[sq_departure],side_playing));
    assert(!TSTFLAG(being_solved.spec[sq_departure],side_under_attack));

    allocate_flesh_out_placed(side_playing);

    if (being_solved.king_square[side_playing]==initsquare)
    {
      boolean are_allocations_exhausted;

      being_solved.king_square[side_playing] = sq_departure;

      are_allocations_exhausted  = nr_total_invisbles_consumed()==total_invisible_number;

      ++being_solved.number_of_pieces[side_playing][King];
      replace_walk(sq_departure,King);
      SETFLAG(being_solved.spec[sq_departure],Royal);
      if (!(king_pos!=initsquare && king_check_ortho(side_playing,king_pos)))
      {
        PieceIdType const id_moving = GetPieceId(being_solved.spec[sq_departure]);
        push_decision_walk(id_moving,King,decision_purpose_random_mover_forward,trait[nbply]);
        forward_random_move_by_existing_invisible_from(sq_departure,true);
        pop_decision();
      }
      CLRFLAG(being_solved.spec[sq_departure],Royal);
      --being_solved.number_of_pieces[side_playing][King];
      being_solved.king_square[side_playing] = initsquare;

      if (can_decision_level_be_continued()
          && !are_allocations_exhausted)
        forward_random_move_by_existing_invisible_as_non_king_from(sq_departure);
    }
    else
      forward_random_move_by_existing_invisible_as_non_king_from(sq_departure);

    current_consumption = save_consumption;

    replace_walk(sq_departure,save_walk_moving);
  }
  else
    forward_random_move_by_existing_invisible_from(sq_departure,false);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

square const *find_next_forward_mover(square const *start_square)
{
  Side const side_playing = trait[nbply];
  square const *result;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  for (result = start_square; *result; ++result)
    if (TSTFLAG(being_solved.spec[*result],Chameleon)
        && TSTFLAG(being_solved.spec[*result],side_playing))
    {
      PieceIdType const id = GetPieceId(being_solved.spec[*result]);

      if (motivation[id].last.acts_when<nbply
          || (motivation[id].last.purpose==purpose_interceptor
              && motivation[id].last.acts_when<=nbply))
        break;
    }

  TraceFunctionExit(__func__);
  TraceSquare(*result);
  TraceFunctionResultEnd();
  return result;
}

/* This function is recursive so that we remember which invisibles have already been
 * visited
 */
static void forward_random_move_by_invisible_from(square const *start_square)
{
  square const *s;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  s = find_next_forward_mover(start_square);

  if (*s)
  {
    PieceIdType const id = GetPieceId(being_solved.spec[*s]);
    action_type const save_last = motivation[id].last;

    motivation[id].last.acts_when = nbply;
    motivation[id].last.purpose = purpose_random_mover;

    push_decision_departure(id,*s,decision_purpose_random_mover_forward);
    forward_random_move_by_specific_invisible_from(*s);
    pop_decision();

    if (can_decision_level_be_continued())
      forward_random_move_by_invisible_from(s+1);

    motivation[id].last = save_last;
  }
  else
  {
    // TODO Strictly speaking, there is no guarantee that such a move exists
    // but we probably save a lot of time by not fleshing it out. As long as we
    // restrict ourselves to h#n, the risk is printing some wrong cooks.
    // Options:
    // * find out how hight the cost would be
    // * fleshing it out
    // * option for activating fleshing out

    dynamic_consumption_type const save_consumption = current_consumption;

    current_consumption.claimed[trait[nbply]] = true;
    TraceConsumption();TraceEOL();

    if (nr_total_invisbles_consumed()<=total_invisible_number)
    {
      TraceText("stick to random move by unplaced invisible\n");
      push_decision_random_move(decision_purpose_random_mover_forward);
      recurse_into_child_ply();
      pop_decision();
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_invisible_leaper_to(square sq_arrival,
                                                       square sq_departure,
                                                       piece_walk_type walk_leaper,
                                                       boolean is_dummy_moving)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_departure]);
  action_type const save_last = motivation[id].last;
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceSquare(sq_departure);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  motivation[id].last.acts_when = nbply;
  motivation[id].last.purpose = purpose_random_mover;

  push_decision_departure(id,sq_departure,decision_purpose_random_mover_forward);

  move_effect_journal[movement].u.piece_movement.to = sq_arrival;
  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  move_effect_journal[movement].u.piece_movement.moving = walk_leaper;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

  assert(!will_be_taboo(sq_arrival,trait[nbply]));

  if (is_square_empty(sq_arrival))
    done_forward_random_move_by_invisible_from(is_dummy_moving);
  else
    forward_accidental_capture_by_invisible(is_dummy_moving);

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;

  pop_decision();

  motivation[id].last = save_last;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_invisible_king_to(square sq_arrival,
                                                     square sq_departure)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[king_square_movement].type==move_effect_none);
  move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
  move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
  move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
  move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

  forward_random_move_by_invisible_leaper_to(sq_arrival,sq_departure,King,false);

  move_effect_journal[king_square_movement].type = move_effect_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_invisible_rider_to(square sq_arrival,
                                                      square sq_departure,
                                                      piece_walk_type walk_rider,
                                                      boolean is_dummy_moving)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_departure]);
  action_type const save_last = motivation[id].last;
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  int const diff = sq_arrival-sq_departure;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceSquare(sq_departure);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  motivation[id].last.acts_when = nbply;
  motivation[id].last.purpose = purpose_random_mover;

  push_decision_departure(id,sq_departure,decision_purpose_random_mover_forward);

  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  move_effect_journal[movement].u.piece_movement.moving = walk_rider;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

  if (find_end_of_line(sq_departure,CheckDir[walk_rider][diff])==sq_arrival)
  {
    move_effect_journal[movement].u.piece_movement.to = sq_arrival;

    assert(!will_be_taboo(sq_arrival,trait[nbply]));

    if (is_square_empty(sq_arrival))
      done_forward_random_move_by_invisible_from(is_dummy_moving);
    else
      forward_accidental_capture_by_invisible(is_dummy_moving);
  }

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;

  pop_decision();

  motivation[id].last = save_last;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_pawn_no_capture_to(square sq_arrival,
                                                      square sq_departure,
                                                      boolean is_dummy_moving)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  PieceIdType const id = GetPieceId(being_solved.spec[sq_departure]);
  action_type const save_last = motivation[id].last;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceSquare(sq_departure);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  motivation[id].last.acts_when = nbply;
  motivation[id].last.purpose = purpose_random_mover;

  push_decision_departure(id,sq_departure,decision_purpose_random_mover_forward);

  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  move_effect_journal[movement].u.piece_movement.to = sq_arrival;
  move_effect_journal[movement].u.piece_movement.moving = Pawn;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

  assert(!will_be_taboo(sq_arrival,trait[nbply]));

  // TODO promotion
  done_forward_random_move_by_invisible_from(is_dummy_moving);

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;

  pop_decision();

  motivation[id].last = save_last;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_pawn_capture_to(square sq_arrival,
                                                   square sq_departure,
                                                   boolean is_dummy_moving)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  PieceIdType const id = GetPieceId(being_solved.spec[sq_departure]);
  action_type const save_last = motivation[id].last;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceSquare(sq_departure);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  motivation[id].last.acts_when = nbply;
  motivation[id].last.purpose = purpose_random_mover;

  push_decision_departure(id,sq_departure,decision_purpose_random_mover_forward);

  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  move_effect_journal[movement].u.piece_movement.to = sq_arrival;
  move_effect_journal[movement].u.piece_movement.moving = Pawn;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

  assert(!will_be_taboo(sq_arrival,trait[nbply]));

  forward_accidental_capture_by_invisible(false);

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;

  pop_decision();

  motivation[id].last = save_last;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_unfleshed_out_non_king_to(square sq_arrival,
                                                             square sq_departure)
{
  Side const side_playing = trait[nbply];
  Side const side_under_attack = advers(side_playing);
  square const king_pos = being_solved.king_square[side_under_attack];

  PieceIdType const id_moving = GetPieceId(being_solved.spec[sq_departure]);

  int const diff = sq_arrival-sq_departure;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (side_playing==White ? diff>0 : diff<0)
  {
    int const dir_singlestep = side_playing==White ? dir_up : dir_down;
    SquareFlags const promsq = side_playing==White ? WhPromSq : BlPromSq;
    SquareFlags const basesq = side_playing==White ? WhBaseSq : BlBaseSq;

    if (!(TSTFLAG(sq_spec[sq_departure],basesq) || TSTFLAG(sq_spec[sq_departure],promsq)))
    {
      push_decision_walk(id_moving,Pawn,decision_purpose_random_mover_forward,trait[nbply]);

      ++being_solved.number_of_pieces[side_playing][Pawn];
      replace_walk(sq_departure,Pawn);

      if (!(king_pos!=initsquare && pawn_check_ortho(side_playing,king_pos)))
      {
        if (is_square_empty(sq_arrival))
        {
          if (diff==dir_singlestep)
            forward_random_move_by_pawn_no_capture_to(sq_arrival,sq_departure,true);
          else if (diff==2*dir_singlestep)
          {
            square const sq_singlestep = sq_departure+dir_singlestep;
            SquareFlags const doublstepsq = side_playing==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

            if (is_square_empty(sq_singlestep) && TSTFLAG(sq_spec[sq_departure],doublstepsq))
              forward_random_move_by_pawn_no_capture_to(sq_arrival,sq_departure,true);
          }
        }
        else
        {
          if (diff==dir_singlestep+dir_right || diff==dir_singlestep+dir_left)
            forward_random_move_by_pawn_capture_to(sq_arrival,sq_departure,true);
        }
      }

      --being_solved.number_of_pieces[side_playing][Pawn];

      pop_decision();
    }
  }

  if (CheckDir[Knight][diff]==diff)
  {
    if (can_decision_level_be_continued())
    {
      ++being_solved.number_of_pieces[side_playing][Knight];
      replace_walk(sq_departure,Knight);

      if (!(king_pos!=initsquare && knight_check_ortho(side_playing,king_pos)))
        forward_random_move_by_invisible_leaper_to(sq_arrival,sq_departure,Knight,true);

      --being_solved.number_of_pieces[side_playing][Knight];
    }
  }

  if (CheckDir[Bishop][diff]!=0)
  {
    if (can_decision_level_be_continued())
    {
      ++being_solved.number_of_pieces[side_playing][Bishop];
      replace_walk(sq_departure,Bishop);

      if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                          vec_bishop_start,vec_bishop_end,
                                          Bishop))
        forward_random_move_by_invisible_rider_to(sq_arrival,sq_departure,Bishop,true);

      --being_solved.number_of_pieces[side_playing][Bishop];
    }
  }

  if (CheckDir[Rook][diff]!=0)
  {
    if (can_decision_level_be_continued())
    {
      ++being_solved.number_of_pieces[side_playing][Rook];
      replace_walk(sq_departure,Rook);

      if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                          vec_rook_start,vec_rook_end,
                                          Rook))
        forward_random_move_by_invisible_rider_to(sq_arrival,sq_departure,Rook,true);

      --being_solved.number_of_pieces[side_playing][Rook];
    }
  }

  if (CheckDir[Queen][diff]!=0)
  {
    if (can_decision_level_be_continued())
    {
      ++being_solved.number_of_pieces[side_playing][Queen];
      replace_walk(sq_departure,Queen);

      if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                          vec_queen_start,vec_queen_end,
                                          Queen))
        forward_random_move_by_invisible_rider_to(sq_arrival,sq_departure,Queen,true);

      --being_solved.number_of_pieces[side_playing][Queen];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_random_move_by_invisible_to(square sq_arrival, boolean is_sacrifice)
{
  boolean is_sq_arrival_reachable = false;
  const square* curr;

  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  piece_walk_type const save_walk_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_flags_moving = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParam("%u",is_sacrifice);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
  // TODO why doesn't this hold?
  //assert(move_effect_journal[movement].u.piece_movement.moving==Dummy);

  assert(!will_be_taboo(sq_arrival,trait[nbply]));

  for (curr = find_next_forward_mover(boardnum);
       *curr && can_decision_level_be_continued();
       curr = find_next_forward_mover(curr+1))
  {
    square const sq_departure = *curr;
    piece_walk_type const walk = get_walk_of_piece_on_square(sq_departure);
    int const diff = sq_arrival-sq_departure;

    TraceSquare (sq_departure);
    TraceWalk (walk);
    TraceValue ("%d",diff);
    TraceEOL ();

    switch (walk)
    {
      case King:
        if (is_sacrifice)
        {
          TraceText("we don't sacrifice the king\n");
        }
        else if (CheckDir[Queen][diff]==diff)
        {
          forward_random_move_by_invisible_king_to(sq_arrival,sq_departure);

          is_sq_arrival_reachable = true;
        }
        break;

      case Knight:
        if (CheckDir[Knight][diff]==diff)
        {
          forward_random_move_by_invisible_leaper_to(sq_arrival,sq_departure,Knight,false);

          is_sq_arrival_reachable = true;
        }
        break;

      case Queen:
      case Rook:
      case Bishop:
        if (CheckDir[walk][diff]!=0)
        {
          forward_random_move_by_invisible_rider_to(sq_arrival,sq_departure,walk,false);

          is_sq_arrival_reachable = true;
        }
        break;

      case Pawn:
      {
        Side const side = trait[nbply];
        int const dir_singlestep = side==White ? dir_up : dir_down;

        assert(!will_be_taboo(sq_arrival,side));

        if (is_square_empty(sq_arrival))
        {
          if (diff==dir_singlestep)
          {
            forward_random_move_by_pawn_no_capture_to(sq_arrival,sq_departure,false);

            is_sq_arrival_reachable = true;
          }
          else if (diff==2*dir_singlestep)
          {
            square const sq_singlestep = sq_departure+dir_singlestep;
            SquareFlags const doublstepsq = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

            if (is_square_empty(sq_singlestep) && TSTFLAG(sq_spec[sq_departure],doublstepsq))
            {
              forward_random_move_by_pawn_no_capture_to(sq_arrival,sq_departure,false);

              is_sq_arrival_reachable = true;
            }
          }
        }
        else
        {
          if (diff==dir_singlestep+dir_right || diff==dir_singlestep+dir_left)
          {
            forward_random_move_by_pawn_capture_to(sq_arrival,sq_departure,false);

            is_sq_arrival_reachable = true;
          }
        }
        break;
      }

      case Dummy:
        assert(play_phase==play_validating_mate);

        if (CheckDir[Queen][diff]!=0 || CheckDir[Knight][diff]==diff)
        {
          Side const side_playing = trait[nbply];
          dynamic_consumption_type const save_consumption = current_consumption;

          allocate_flesh_out_placed(side_playing);

          if (being_solved.king_square[side_playing]==initsquare && CheckDir[Queen][diff]==diff)
          {
            Side const side_under_attack = advers(side_playing);
            square const king_pos = being_solved.king_square[side_under_attack];
            boolean are_allocations_exhausted;

            being_solved.king_square[side_playing] = sq_departure;

            are_allocations_exhausted  = nr_total_invisbles_consumed()==total_invisible_number;

            ++being_solved.number_of_pieces[side_playing][King];
            replace_walk(sq_departure,King);
            SETFLAG(being_solved.spec[sq_departure],Royal);

            if (!(king_pos!=initsquare && king_check_ortho(side_playing,king_pos)))
              forward_random_move_by_invisible_king_to(sq_arrival,sq_departure);

            CLRFLAG(being_solved.spec[sq_departure],Royal);
            --being_solved.number_of_pieces[side_playing][King];
            being_solved.king_square[side_playing] = initsquare;

            if (can_decision_level_be_continued()
                && !are_allocations_exhausted)
              forward_random_move_by_unfleshed_out_non_king_to(sq_arrival,sq_departure);
          }
          else
            forward_random_move_by_unfleshed_out_non_king_to(sq_arrival,sq_departure);

          current_consumption = save_consumption;

          replace_walk(sq_departure,Dummy);

          is_sq_arrival_reachable = true;
        }
        break;

      default:
        assert (0);
        break;
    }
  }

  move_effect_journal[movement].u.piece_movement.moving = save_walk_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_flags_moving;

  if (is_sq_arrival_reachable)
  {
    TraceText ("found a potential sacrifice by an invisible\n");
  }
  else
  {
    TraceText ("couldn't find a potential sacrifice by an invisible\n");
    record_decision_outcome ("capture in ply %u will not be possible",
                             nbply + 1);
    REPORT_DEADEND;
    backtrack_from_failed_capture_of_invisible_by_pawn (advers(trait[nbply]));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void flesh_out_random_move_by_invisible(void)
{
  square sq_required_sacrifice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  sq_required_sacrifice = need_existing_invisible_as_victim_for_capture_by_pawn(nbply+1);

  if (sq_required_sacrifice==initsquare)
    forward_random_move_by_invisible_from(boardnum);
  else
    forward_random_move_by_invisible_to(sq_required_sacrifice,true);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_backward_random_move_by_specific_invisible_to(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  Side const side_moving = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (!was_taboo(sq_departure,side_moving))
  {
    Side const side_attacked = advers(side_moving);
    square const king_pos = being_solved.king_square[side_attacked];
    PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);

    push_decision_departure(id,sq_departure,decision_purpose_random_mover_backward);

    /* we can't do undo_move_effects() because we might inadvertently undo a piece
     * revelation!
     */
    empty_square(sq_arrival);
    occupy_square(sq_departure,
                  move_effect_journal[movement].u.piece_movement.moving,
                  move_effect_journal[movement].u.piece_movement.movingspec);

    if (!is_square_uninterceptably_attacked(side_attacked,king_pos))
    {
      motivation_type const save_motivation = motivation[id];

      motivation[id].first.purpose = purpose_random_mover;
      motivation[id].first.on = sq_departure;
      motivation[id].first.acts_when = nbply;

      remember_taboos_for_current_move();
      restart_from_scratch();

      forget_taboos_for_current_move();

      motivation[id] = save_motivation;
    }

    empty_square(sq_departure);
    occupy_square(sq_arrival,
                  move_effect_journal[movement].u.piece_movement.moving,
                  move_effect_journal[movement].u.piece_movement.movingspec);

    pop_decision();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void backward_random_move_by_specific_invisible_rider_to(vec_index_type kstart,
                                                                 vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kstart);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  for (k = kstart; k<=kend && can_decision_level_be_continued(); ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    while (can_decision_level_be_continued()
           && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      done_backward_random_move_by_specific_invisible_to();
      move_effect_journal[movement].u.piece_movement.from -= vec[k];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void backward_random_move_by_specific_invisible_king_to(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(being_solved.king_square[trait[nbply]]==move_effect_journal[movement].u.piece_movement.to);

  assert(move_effect_journal[king_square_movement].type==move_effect_none);
  move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
  move_effect_journal[king_square_movement].u.king_square_movement.to = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

  for (k = vec_queen_start; k<=vec_queen_end && can_decision_level_be_continued(); ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      being_solved.king_square[trait[nbply]] = move_effect_journal[movement].u.piece_movement.from;
      move_effect_journal[king_square_movement].u.king_square_movement.from = move_effect_journal[movement].u.piece_movement.from;
      done_backward_random_move_by_specific_invisible_to();
    }
  }

  being_solved.king_square[trait[nbply]] = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal[king_square_movement].type = move_effect_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void backward_random_move_by_specific_invisible_leaper_to(vec_index_type kstart,
                                                                  vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kstart);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  for (k = kstart; k<=kend && can_decision_level_be_continued(); ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
      done_backward_random_move_by_specific_invisible_to();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void backward_random_move_by_specific_invisible_pawn_to(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  Side const side_moving = trait[nbply];
  int const dir = side_moving==White ? dir_up : dir_down;
  SquareFlags const promsq = side_moving==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side_moving==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-dir;
  TraceSquare(move_effect_journal[movement].u.piece_movement.from);
  TraceEOL();

  if (is_square_empty(move_effect_journal[movement].u.piece_movement.from)
      && !TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],basesq)
      && !TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],promsq))
  {
    done_backward_random_move_by_specific_invisible_to();

    if (can_decision_level_be_continued())
    {
      SquareFlags const doublestepsq = side_moving==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

      move_effect_journal[movement].u.piece_movement.from -= dir;
      if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublestepsq)
          && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
        done_backward_random_move_by_specific_invisible_to();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void backward_random_move_by_specific_invisible_to_according_to_walk(square sq_arrival)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceSquare(motivation[id].first.on);TraceEOL();
  TraceWalk(get_walk_of_piece_on_square(sq_arrival));TraceEOL();

  assert(motivation[id].first.on==sq_arrival);
  assert(get_walk_of_piece_on_square(sq_arrival)!=Dummy);
  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
  assert(move_effect_journal[movement].u.piece_movement.to==move_by_invisible);
  assert(move_effect_journal[movement].u.piece_movement.moving==Empty);
  assert(move_effect_journal[movement].u.piece_movement.movingspec==0);

  move_effect_journal[movement].u.piece_movement.to = sq_arrival;
  move_effect_journal[movement].u.piece_movement.moving = get_walk_of_piece_on_square(sq_arrival);
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_arrival];

  switch (move_effect_journal[movement].u.piece_movement.moving)
  {
    case King:
      backward_random_move_by_specific_invisible_king_to();
      break;

    case Queen:
      backward_random_move_by_specific_invisible_rider_to(vec_queen_start,
                                                          vec_queen_end);
      break;

    case Rook:
      backward_random_move_by_specific_invisible_rider_to(vec_rook_start,
                                                          vec_rook_end);
      break;

    case Bishop:
      backward_random_move_by_specific_invisible_rider_to(vec_bishop_start,
                                                          vec_bishop_end);
      break;

    case Knight:
      backward_random_move_by_specific_invisible_leaper_to(vec_knight_start,
                                                           vec_knight_end);
      break;

    case Pawn:
      backward_random_move_by_specific_invisible_pawn_to();
      break;

    default:
      break;
  }

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.moving = Empty;
  move_effect_journal[movement].u.piece_movement.movingspec = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void backward_random_move_by_specific_invisible_as_non_king_to(square sq_arrival)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);
  Side const side_playing = trait[nbply];
  SquareFlags const promsq = side_playing==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side_playing==White ? WhBaseSq : BlBaseSq;
  piece_walk_type walk;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  if ((TSTFLAG(sq_spec[sq_arrival],basesq) || TSTFLAG(sq_spec[sq_arrival],promsq)))
  {
    record_decision_outcome("%s","pawn is placed on impossible square");
    REPORT_DEADEND;
  }
  else
  {
    push_decision_walk(id,Pawn,decision_purpose_random_mover_backward,side_playing);

    ++being_solved.number_of_pieces[side_playing][Pawn];
    replace_walk(sq_arrival,Pawn);

    backward_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);

    --being_solved.number_of_pieces[side_playing][Pawn];

    pop_decision();
  }

  for (walk = Pawn+1; walk<=Bishop && can_decision_level_be_continued(); ++walk)
  {
    push_decision_walk(id,walk,decision_purpose_random_mover_backward,side_playing);

    ++being_solved.number_of_pieces[side_playing][walk];
    replace_walk(sq_arrival,walk);

    backward_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);

    --being_solved.number_of_pieces[side_playing][walk];

    pop_decision();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void backward_random_move_by_specific_invisible_to(square sq_arrival)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceValue("%u",id);
  TraceValue("%u",motivation[id].first.purpose);
  TraceValue("%u",motivation[id].first.acts_when);
  TraceSquare(motivation[id].first.on);
  TraceValue("%u",motivation[id].last.purpose);
  TraceValue("%u",motivation[id].last.acts_when);
  TraceSquare(motivation[id].last.on);
  TraceEOL();

  if (motivation[id].first.on==sq_arrival)
  {
    Flags const save_flags_moving = being_solved.spec[sq_arrival];

    TraceWalk(get_walk_of_piece_on_square(sq_arrival));TraceEOL();
    if (get_walk_of_piece_on_square(sq_arrival)==Dummy)
    {
      Side const side_playing = trait[nbply];
      Side const side_under_attack = advers(side_playing);
      square const king_pos = being_solved.king_square[side_under_attack];
      dynamic_consumption_type const save_consumption = current_consumption;
      decision_levels_type const save_levels = decision_levels[id];

      allocate_flesh_out_placed(side_playing);

      if (being_solved.king_square[side_playing]==initsquare)
      {
        boolean are_allocations_exhausted;

        being_solved.king_square[side_playing] = sq_arrival;

        are_allocations_exhausted  = nr_total_invisbles_consumed()==total_invisible_number;

        ++being_solved.number_of_pieces[side_playing][King];
        replace_walk(sq_arrival,King);
        SETFLAG(being_solved.spec[sq_arrival],Royal);
        if (!(king_pos!=initsquare && king_check_ortho(side_playing,king_pos)))
        {
          PieceIdType const id_moving = GetPieceId(save_flags_moving);
          push_decision_walk(id_moving,King,decision_purpose_random_mover_forward,trait[nbply]);
          backward_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);
          pop_decision();
        }
        CLRFLAG(being_solved.spec[sq_arrival],Royal);
        --being_solved.number_of_pieces[side_playing][King];
        being_solved.king_square[side_playing] = initsquare;

        if (can_decision_level_be_continued()
            && !are_allocations_exhausted)
          backward_random_move_by_specific_invisible_as_non_king_to(sq_arrival);
      }
      else
        backward_random_move_by_specific_invisible_as_non_king_to(sq_arrival);

      replace_walk(sq_arrival,Dummy);

      current_consumption = save_consumption;
      decision_levels[id] = save_levels;
    }
    else
      backward_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);
  }
  else
  {
    TraceText("the piece has already moved\n");
    record_decision_outcome("%s","the piece has already moved");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean is_random_move_by_invisible(ply ply)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",ply);
  TraceFunctionParamListEnd();

  TraceSquare(move_effect_journal[movement].u.piece_movement.from);TraceEOL();

  result = move_effect_journal[movement].u.piece_movement.from==move_by_invisible;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static square const *find_next_backward_mover(square const *start_square)
{
  Side const side_playing = trait[nbply];
  square const *result;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  for (result = start_square; *result; ++result)
    if (TSTFLAG(being_solved.spec[*result],Chameleon)
        && TSTFLAG(being_solved.spec[*result],side_playing))
    {
      PieceIdType const id = GetPieceId(being_solved.spec[*result]);
      if (motivation[id].first.acts_when>nbply)
        break;
    }

  TraceFunctionExit(__func__);
  TraceSquare(*result);
  TraceFunctionResultEnd();
  return result;
}

/* This function is recursive so that we remember which invisibles have already been
 * visited
 */
static void backward_random_move_by_invisible_to(square const *start_square)
{
  square const *s;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  // TODO retract pawn captures?

  s = find_next_backward_mover(start_square);

  if (*s)
  {
    PieceIdType const id = GetPieceId(being_solved.spec[*s]);
    ply const save_when = motivation[id].first.acts_when;
    unsigned int const save_counter = record_decision_counter;
    decision_levels_type const save_levels = decision_levels[id];

    motivation[id].first.acts_when = nbply;

    push_decision_arrival(id,*s,decision_purpose_random_mover_backward);

    backward_random_move_by_specific_invisible_to(*s);

    pop_decision();

    decision_levels[id] = save_levels;

    if (can_decision_level_be_continued())
      backward_random_move_by_invisible_to(s+1);

    /* only now - this prevents trying to retract random moves by the same piece in nested levels */
    motivation[id].first.acts_when = save_when;

    if (start_square==boardnum && record_decision_counter<=save_counter+1)
    {
      record_decision_outcome("%s","no retractable random move found - TODO we don't retract pawn captures");
      REPORT_DEADEND;
    }
  }
  else if (can_decision_level_be_continued())
  {
    // TODO Strictly speaking, there is no guarantee that such a move exists
    // but we probably save a lot of time by not fleshing it out. As long as we
    // restrict ourselves to h#n, the risk is printing some wrong cooks.
    // Options:
    // * find out how hight the cost would be
    // * fleshing it out
    // * option for activating fleshing out

    dynamic_consumption_type const save_consumption = current_consumption;

    current_consumption.claimed[trait[nbply]] = true;
    if (nr_total_invisbles_consumed()<=total_invisible_number)
    {
      TraceText("stick to random move by unplaced invisible\n");
      push_decision_random_move(decision_purpose_random_mover_backward);
      restart_from_scratch();
      pop_decision();
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void backward_fleshout_random_move_by_invisible(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (check_by_uninterceptable_delivered_from!=initsquare
      && trait[check_by_uninterceptable_delivered_in_ply]!=trait[nbply])
  {
    // TODO what about king flights? they can even occur before uninterceptable_check_delivered_in_ply
    if (can_decision_level_be_continued())
      fake_capture_by_invisible();
  }
  else
    backward_random_move_by_invisible_to(boardnum);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
