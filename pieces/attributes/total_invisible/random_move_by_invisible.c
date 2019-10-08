#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "solving/move_effect_journal.h"
#include "optimisations/orthodox_square_observation.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static void done_fleshing_out_random_move_by_invisible_from(boolean is_dummy_moving)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  PieceIdType const id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
  motivation_type const save_motivation = motivation[id];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_dummy_moving);
  TraceFunctionParamListEnd();

  REPORT_DECISION_MOVE('>','-');
  ++curr_decision_level;

  motivation[id].last.on = sq_arrival;

  update_nr_taboos_for_current_move_in_ply(+1);
  if (is_dummy_moving)
    restart_from_scratch();
  else
    recurse_into_child_ply();
  update_nr_taboos_for_current_move_in_ply(-1);

  motivation[id] = save_motivation;

  --curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_accidental_capture_by_invisible(boolean is_dummy_moving)
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

    TraceValue("%u",id_victim);
    TraceValue("%u",motivation[id_victim].first.purpose);
    TraceValue("%u",motivation[id_victim].first.acts_when);
    TraceSquare(motivation[id_victim].first.on);
    TraceValue("%u",motivation[id_victim].last.purpose);
    TraceValue("%u",motivation[id_victim].last.acts_when);
    TraceSquare(motivation[id_victim].last.on);
    TraceEOL();

    assert(motivation[id_victim].first.purpose!=purpose_none);
    assert(motivation[id_victim].last.purpose!=purpose_none);
    if (motivation[id_victim].last.acts_when>nbply)
    {
      TraceText("the planned victim was added to later act from its current square\n");
      REPORT_DECISION_OUTCOME("%s","the planned victim was added to later act from its current square");
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

      done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);

      move_effect_journal[capture].type = move_effect_no_piece_removal;

      motivation[id_captured].last = save_last;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_pawn_from(boolean is_dummy_moving)
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
      if (!is_taboo(sq_singlestep,side))
      {
        move_effect_journal[movement].u.piece_movement.to = sq_singlestep;
        // TODO promotion
        done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
      }

      if (curr_decision_level<=max_decision_level)
      {
        SquareFlags const doublstepsq = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
        if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublstepsq))
        {
          square const sq_doublestep = sq_singlestep+dir;
          TraceSquare(sq_doublestep);TraceEOL();
          if (is_square_empty(sq_doublestep))
          {
            if (!is_taboo(sq_doublestep,side))
            {
              max_decision_level = decision_level_latest;
              move_effect_journal[movement].u.piece_movement.to = sq_doublestep;
              done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
            }
          }
        }
      }
    }

    // TODO add capture victim if arrival square empty and nr_total...>0

    if (curr_decision_level<=max_decision_level)
    {
      square const sq_arrival = sq_singlestep+dir_right;
      max_decision_level = decision_level_latest;
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      flesh_out_accidental_capture_by_invisible(is_dummy_moving);
    }

    if (curr_decision_level<=max_decision_level)
    {
      square const sq_arrival = sq_singlestep+dir_left;
      max_decision_level = decision_level_latest;
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      flesh_out_accidental_capture_by_invisible(is_dummy_moving);
    }
  }


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_rider_from(vec_index_type kstart,
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
  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    square sq_arrival;
    for (sq_arrival = move_effect_journal[movement].u.piece_movement.from+vec[k];
         curr_decision_level<=max_decision_level;
         sq_arrival += vec[k])
    {
      TraceSquare(sq_arrival);TraceEOL();
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;

      max_decision_level = decision_level_latest;

      /* "factoring out" the invokations of is_taboo() is tempting, but we
       * want to break the loop if sq_arrival is not empty whether or not
       * that square is nr_taboos_accumulated_until_ply!
       */
      if (is_square_empty(sq_arrival))
      {
        if (!is_taboo(sq_arrival,trait[nbply]))
          done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
      }
      else
      {
        if (!is_taboo(sq_arrival,trait[nbply]))
          flesh_out_accidental_capture_by_invisible(is_dummy_moving);
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_leaper_from(vec_index_type kstart,
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
  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    square const sq_arrival = sq_departure+vec[k];
    if (!is_taboo(sq_arrival,trait[nbply]))
    {
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      /* just in case: */
      move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;

      max_decision_level = decision_level_latest;

      if (is_square_empty(sq_arrival))
        done_fleshing_out_random_move_by_invisible_from(is_dummy_moving);
      else
        flesh_out_accidental_capture_by_invisible(is_dummy_moving);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_existing_invisible_from(square sq_departure,
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
      flesh_out_random_move_by_invisible_leaper_from(vec_queen_start,vec_queen_end,
                                                     is_dummy_moving);
      move_effect_journal[king_square_movement].type = move_effect_none;
      break;

    case Queen:
      flesh_out_random_move_by_invisible_rider_from(vec_queen_start,vec_queen_end,
                                                    is_dummy_moving);
      break;

    case Rook:
      flesh_out_random_move_by_invisible_rider_from(vec_rook_start,vec_rook_end,
                                                    is_dummy_moving);
      break;

    case Bishop:
      flesh_out_random_move_by_invisible_rider_from(vec_bishop_start,vec_bishop_end,
                                                    is_dummy_moving);
      break;

    case Knight:
      flesh_out_random_move_by_invisible_leaper_from(vec_knight_start,vec_knight_end,
                                                     is_dummy_moving);
      break;

    case Pawn:
      flesh_out_random_move_by_invisible_pawn_from(is_dummy_moving);
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

static void flesh_out_random_move_by_existing_invisible_as_non_king_from(square sq_departure)
{
  Side const side_playing = trait[nbply];
  Side const side_under_attack = advers(side_playing);
  square const king_pos = being_solved.king_square[side_under_attack];

  PieceIdType const id_moving = GetPieceId(being_solved.spec[sq_departure]);
  decision_level_type const save_level = motivation[id_moving].levels.walk;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  {
    SquareFlags const promsq = side_playing==White ? WhPromSq : BlPromSq;
    SquareFlags const basesq = side_playing==White ? WhBaseSq : BlBaseSq;
    if (!(TSTFLAG(sq_spec[sq_departure],basesq) || TSTFLAG(sq_spec[sq_departure],promsq)))
    {
      motivation[id_moving].levels.walk = curr_decision_level;
      REPORT_DECISION_WALK('>',Pawn);
      ++curr_decision_level;

      ++being_solved.number_of_pieces[side_playing][Pawn];
      replace_walk(sq_departure,Pawn);
      if (!(king_pos!=initsquare && pawn_check_ortho(side_playing,king_pos)))
        flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
      --being_solved.number_of_pieces[side_playing][Pawn];

      --curr_decision_level;
    }
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Knight);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Knight];
    replace_walk(sq_departure,Knight);
    if (!(king_pos!=initsquare && knight_check_ortho(side_playing,king_pos)))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Knight];

    --curr_decision_level;
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Bishop);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Bishop];
    replace_walk(sq_departure,Bishop);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_bishop_start,vec_bishop_end,
                                        Bishop))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Bishop];

    --curr_decision_level;
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Rook);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Rook];
    replace_walk(sq_departure,Rook);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_rook_start,vec_rook_end,
                                        Rook))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Rook];

    --curr_decision_level;
  }

  if (curr_decision_level<=max_decision_level)
  {
    max_decision_level = decision_level_latest;

    motivation[id_moving].levels.walk = curr_decision_level;
    REPORT_DECISION_WALK('>',Queen);
    ++curr_decision_level;

    ++being_solved.number_of_pieces[side_playing][Queen];
    replace_walk(sq_departure,Queen);
    if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                        vec_queen_start,vec_queen_end,
                                        Queen))
      flesh_out_random_move_by_existing_invisible_from(sq_departure,true);
    --being_solved.number_of_pieces[side_playing][Queen];

    --curr_decision_level;
  }

  motivation[id_moving].levels.walk = save_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
static void flesh_out_random_move_by_specific_invisible_from(square sq_departure)
{
  Side const side_playing = trait[nbply];
  piece_walk_type const walk_on_square = get_walk_of_piece_on_square(sq_departure);
  Flags const save_flags = being_solved.spec[sq_departure];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  CLRFLAG(being_solved.spec[sq_departure],advers(side_playing));

  TraceWalk(walk_on_square);
  TraceSquare(sq_departure);
  TraceEOL();

  // TODO use a sibling ply and the regular move generation machinery?

  if (walk_on_square==Dummy)
  {
    Side const side_playing = trait[nbply];
    Side const side_under_attack = advers(side_playing);
    square const king_pos = being_solved.king_square[side_under_attack];
    dynamic_consumption_type const save_consumption = current_consumption;
    PieceIdType const id_moving = GetPieceId(save_flags);

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
        decision_level_type const save_level = motivation[id_moving].levels.walk;

        motivation[id_moving].levels.walk = curr_decision_level;
        REPORT_DECISION_WALK('>',King);
        ++curr_decision_level;

        flesh_out_random_move_by_existing_invisible_from(sq_departure,true);

        --curr_decision_level;

        motivation[id_moving].levels.walk = save_level;
      }
      CLRFLAG(being_solved.spec[sq_departure],Royal);
      --being_solved.number_of_pieces[side_playing][King];
      being_solved.king_square[side_playing] = initsquare;

      if (curr_decision_level<=max_decision_level
          && !are_allocations_exhausted)
      {
        max_decision_level = decision_level_latest;
        flesh_out_random_move_by_existing_invisible_as_non_king_from(sq_departure);
      }
    }
    else
      flesh_out_random_move_by_existing_invisible_as_non_king_from(sq_departure);

    current_consumption = save_consumption;

    replace_walk(sq_departure,walk_on_square);
    being_solved.spec[sq_departure] = save_flags;
  }
  else
    flesh_out_random_move_by_existing_invisible_from(sq_departure,false);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square const *find_next_forward_mover(square const *start_square)
{
  Side const side_playing = trait[nbply];
  square const *result;

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  for (result = boardnum; *result; ++result)
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
static void forward_random_move_by_invisible(square const *start_square)
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

    REPORT_DECISION_SQUARE('>',*s);
    ++curr_decision_level;
    flesh_out_random_move_by_specific_invisible_from(*s);
    --curr_decision_level;

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      forward_random_move_by_invisible(s+1);
    }

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

    TraceText("random move by unplaced invisible\n");

    current_consumption.claimed[trait[nbply]] = true;
    TraceConsumption();TraceEOL();

    if (nr_total_invisbles_consumed()<=total_invisible_number)
    {
      REPORT_DECISION_MOVE('>','-');
      ++curr_decision_level;
      recurse_into_child_ply();
      --curr_decision_level;
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void flesh_out_random_move_by_invisible(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  forward_random_move_by_invisible(boardnum);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_fleshing_out_random_move_by_specific_invisible_to(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  Side const side_moving = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (!was_taboo(move_effect_journal[movement].u.piece_movement.from))
  {
    Side const side_attacked = advers(side_moving);
    square const king_pos = being_solved.king_square[side_attacked];

    /* we can't do undo_move_effects() because we might inadvertently undo a piece
     * revelation!
     */
    empty_square(move_effect_journal[movement].u.piece_movement.to);
    occupy_square(move_effect_journal[movement].u.piece_movement.from,
                  move_effect_journal[movement].u.piece_movement.moving,
                  move_effect_journal[movement].u.piece_movement.movingspec);

    if (!is_square_uninterceptably_attacked(side_attacked,king_pos))
    {
      PieceIdType const id = GetPieceId(being_solved.spec[move_effect_journal[movement].u.piece_movement.from]);
      motivation_type const save_motivation = motivation[id];

      motivation[id].first.purpose = purpose_random_mover;
      motivation[id].first.on = move_effect_journal[movement].u.piece_movement.from;
      motivation[id].first.acts_when = nbply;

      update_nr_taboos_for_current_move_in_ply(+1);
      REPORT_DECISION_MOVE('<','-');
      ++curr_decision_level;
      restart_from_scratch();
      --curr_decision_level;

      /* it is irrevelant where the random move in the 1st ply is played from */
      if (nbply==ply_retro_move+1
          && (play_phase==play_detecting_revelations
              || play_phase==play_validating_mate
              || play_phase==play_testing_mate)
          && max_decision_level>curr_decision_level-1)
        max_decision_level = curr_decision_level-1;

      update_nr_taboos_for_current_move_in_ply(-1);

      motivation[id] = save_motivation;
    }

    empty_square(move_effect_journal[movement].u.piece_movement.from);
    occupy_square(move_effect_journal[movement].u.piece_movement.to,
                  move_effect_journal[movement].u.piece_movement.moving,
                  move_effect_journal[movement].u.piece_movement.movingspec);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_rider_to(vec_index_type kstart,
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

  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    while (curr_decision_level<=max_decision_level
           && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      max_decision_level = decision_level_latest;
      done_fleshing_out_random_move_by_specific_invisible_to();
      move_effect_journal[movement].u.piece_movement.from -= vec[k];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_king_to(void)
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

  for (k = vec_queen_start; k<=vec_queen_end && curr_decision_level<=max_decision_level; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      max_decision_level = decision_level_latest;
      being_solved.king_square[trait[nbply]] = move_effect_journal[movement].u.piece_movement.from;
      move_effect_journal[king_square_movement].u.king_square_movement.from = move_effect_journal[movement].u.piece_movement.from;
      done_fleshing_out_random_move_by_specific_invisible_to();
    }
  }

  being_solved.king_square[trait[nbply]] = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal[king_square_movement].type = move_effect_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_leaper_to(vec_index_type kstart,
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

  for (k = kstart; k<=kend && curr_decision_level<=max_decision_level; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
      max_decision_level = decision_level_latest;
      done_fleshing_out_random_move_by_specific_invisible_to();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_pawn_to(void)
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
    done_fleshing_out_random_move_by_specific_invisible_to();

    if (curr_decision_level<=max_decision_level)
    {
      SquareFlags const doublestepsq = side_moving==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

      move_effect_journal[movement].u.piece_movement.from -= dir;
      if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublestepsq)
          && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
      {
        max_decision_level = decision_level_latest;
        done_fleshing_out_random_move_by_specific_invisible_to();
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_specific_invisible_to_according_to_walk(square sq_arrival)
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
      flesh_out_random_move_by_specific_invisible_king_to();
      break;

    case Queen:
      flesh_out_random_move_by_specific_invisible_rider_to(vec_queen_start,
                                                           vec_queen_end);
      break;

    case Rook:
      flesh_out_random_move_by_specific_invisible_rider_to(vec_rook_start,
                                                           vec_rook_end);
      break;

    case Bishop:
      flesh_out_random_move_by_specific_invisible_rider_to(vec_bishop_start,
                                                           vec_bishop_end);
      break;

    case Knight:
      flesh_out_random_move_by_specific_invisible_leaper_to(vec_knight_start,
                                                            vec_knight_end);
      break;

    case Pawn:
      flesh_out_random_move_by_specific_invisible_pawn_to();
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

void flesh_out_random_move_by_specific_invisible_to(square sq_arrival)
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
    TraceWalk(get_walk_of_piece_on_square(sq_arrival));TraceEOL();
    if (get_walk_of_piece_on_square(sq_arrival)==Dummy)
    {
      Side const side_playing = trait[nbply];
      piece_walk_type walk;
      decision_levels_type const save_levels = motivation[id].levels;

      TraceValue("%u",id);
      TraceValue("%u",motivation[id].levels.walk);
      TraceEOL();
      assert(motivation[id].levels.walk==decision_level_latest);
      assert(motivation[id].levels.from==decision_level_latest);
      motivation[id].levels.walk = curr_decision_level;
      motivation[id].levels.from = curr_decision_level+1;

      for (walk = Pawn; walk<=Bishop && curr_decision_level<=max_decision_level; ++walk)
      {
        max_decision_level = decision_level_latest;

        REPORT_DECISION_WALK('<',walk);
        ++curr_decision_level;

        ++being_solved.number_of_pieces[side_playing][walk];
        replace_walk(sq_arrival,walk);

        flesh_out_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);

        replace_walk(sq_arrival,Dummy);
        --being_solved.number_of_pieces[side_playing][walk];

        --curr_decision_level;
      }

      motivation[id].levels = save_levels;
    }
    else
      flesh_out_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);
  }
  else
  {
    TraceText("the piece has already moved\n");
    REPORT_DECISION_OUTCOME("%s","the piece has already moved");
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

void retract_random_move_by_invisible(square const *start_square)
{
  square const *s;

  REPORT_DECISION_DECLARE(unsigned int const save_counter = report_decision_counter);

  TraceFunctionEntry(__func__);
  TraceSquare(*start_square);
  TraceFunctionParamListEnd();

  // TODO retract pawn captures?

  s = find_next_backward_mover(start_square);

  if (*s)
  {
    PieceIdType const id = GetPieceId(being_solved.spec[*s]);
    ply const save_when = motivation[id].first.acts_when;

    motivation[id].first.acts_when = nbply;

    flesh_out_random_move_by_specific_invisible_to(*s);

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      retract_random_move_by_invisible(s+1);
    }

    motivation[id].first.acts_when = save_when;
  }

#if defined(REPORT_DECISIONS)
  if (report_decision_counter==save_counter)
  {
    REPORT_DECISION_OUTCOME("%s","no retractable random move found - TODO we don't retract pawn captures");
    REPORT_DEADEND;
  }
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void backward_fleshout_random_move_by_invisible(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (uninterceptable_check_delivered_from!=initsquare
      && trait[uninterceptable_check_delivered_in_ply]!=trait[nbply])
  {
    // TODO what about king flights? they can even occur before uninterceptable_check_delivered_in_ply
    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      fake_capture_by_invisible();
    }
  }
  else
  {
    dynamic_consumption_type const save_consumption = current_consumption;

    current_consumption.claimed[trait[nbply]] = true;

    if (nr_total_invisbles_consumed()<=total_invisible_number)
    {
      dynamic_consumption_type const save_consumption = current_consumption;

      TraceText("stick to random move by unplaced invisible\n");
      current_consumption.claimed[trait[nbply]] = true;
      TraceConsumption();TraceEOL();
      if (nr_total_invisbles_consumed()<=total_invisible_number)
      {
        REPORT_DECISION_MOVE('<','-');
        ++curr_decision_level;
        restart_from_scratch();
        --curr_decision_level;
      }
      current_consumption = save_consumption;
      TraceConsumption();TraceEOL();
    }

    current_consumption = save_consumption;

    if (curr_decision_level<=max_decision_level)
    {
      max_decision_level = decision_level_latest;
      retract_random_move_by_invisible(boardnum);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
