#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible.h"
#include "solving/ply.h"
#include "solving/move_effect_journal.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static void flesh_out_capture_by_inserted_invisible(piece_walk_type walk_capturing,
                                                    square sq_departure)
{
  Side const side_playing = trait[nbply];
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const precapture = effects_base;
  Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
  PieceIdType const id_inserted = GetPieceId(flags_inserted);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  // TODO first test allocation, then taboo?

  TraceValue("%u",id_inserted);TraceEOL();

  if (was_taboo(sq_departure,side_playing) || is_taboo(sq_departure,side_playing))
  {
    record_decision_outcome("%s","capturer can't be placed on taboo square");
    REPORT_DEADEND;
    max_decision_level = curr_decision_level;
  }
  else
  {
    dynamic_consumption_type const save_consumption = current_consumption;
    if (allocate_flesh_out_unplaced(side_playing))
    {
      Side const side_in_check = trait[nbply-1];
      square const king_pos = being_solved.king_square[side_in_check];

      TraceConsumption();TraceEOL();
      assert(nr_total_invisbles_consumed()<=total_invisible_number);

      ++being_solved.number_of_pieces[side_playing][walk_capturing];
      occupy_square(sq_departure,walk_capturing,flags_inserted);

      if (is_square_uninterceptably_attacked(side_in_check,king_pos))
      {
        record_decision_outcome("%s","capturer would deliver uninterceptable check");
        REPORT_DEADEND;
        max_decision_level = motivation[id_inserted].levels.from;
        if (max_decision_level<motivation[id_inserted].levels.walk)
          max_decision_level = motivation[id_inserted].levels.walk;
      }
      else
      {
        move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
        square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
        motivation_type const save_motivation = motivation[id_inserted];

        assert(!TSTFLAG(being_solved.spec[sq_departure],advers(trait[nbply])));

        motivation[id_inserted].levels.from = curr_decision_level;
        /* allow backtracking from some deadends to backtrack further, skipping over the remainder of the current vector */
        record_decision_square('<',sq_departure);
        ++curr_decision_level;

        /* adding the total invisible in the pre-capture effect sounds tempting, but
         * we have to make sure that there was no illegal check from it before this
         * move!
         * NB: this works with illegal checks both from the inserted piece and to
         * the inserted king (afert we restart_from_scratch()).
         */
        assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
        move_effect_journal[precapture].type = move_effect_none;

        /* these were set in regular play already: */
        assert(motivation[id_inserted].first.acts_when==nbply);
        assert(motivation[id_inserted].first.purpose==purpose_capturer);
        assert(motivation[id_inserted].last.acts_when==nbply);
        assert(motivation[id_inserted].last.purpose==purpose_capturer);
        /* fill in the rest: */
        motivation[id_inserted].first.on = sq_departure;
        motivation[id_inserted].last.on = sq_arrival;

        move_effect_journal[movement].u.piece_movement.from = sq_departure;
        /* move_effect_journal[movement].u.piece_movement.to unchanged from regular play */
        move_effect_journal[movement].u.piece_movement.moving = walk_capturing;
        move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

        remember_taboos_for_current_move();
        restart_from_scratch();
        forget_taboos_for_current_move();

        motivation[id_inserted] = save_motivation;

        move_effect_journal[precapture].type = move_effect_piece_readdition;

        --curr_decision_level;
      }

      empty_square(sq_departure);
      --being_solved.number_of_pieces[side_playing][walk_capturing];

      TraceConsumption();TraceEOL();
    }
    else
    {
      record_decision_outcome("%s","capturer can't be allocated");
      REPORT_DEADEND;
      max_decision_level = motivation[id_inserted].levels.from;
    }

    current_consumption = save_consumption;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_walk_for_capture(piece_walk_type walk_capturing,
                                       square sq_departure)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  Flags const flags_existing = being_solved.spec[sq_departure];

  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  PieceIdType const id_random = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  CLRFLAG(being_solved.spec[sq_departure],advers(trait[nbply]));
  SetPieceId(being_solved.spec[sq_departure],id_random);

  ++being_solved.number_of_pieces[trait[nbply]][walk_capturing];
  replace_walk(sq_departure,walk_capturing);

  if (is_square_uninterceptably_attacked(side_in_check,king_pos))
  {
    PieceIdType const id_existing = GetPieceId(flags_existing);

    record_decision_outcome("%s","uninterceptable check from the attempted departure square");
    REPORT_DEADEND;

    max_decision_level = motivation[id_existing].levels.walk;
  }
  else
  {
    move_effect_journal_index_type const precapture = effects_base;

    PieceIdType const id_existing = GetPieceId(flags_existing);

    piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
    Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;
    square const save_from = move_effect_journal[movement].u.piece_movement.from;

    motivation_type const motivation_random = motivation[id_random];

    dynamic_consumption_type const save_consumption = current_consumption;

    replace_moving_piece_ids_in_past_moves(id_existing,id_random,nbply-1);

    motivation[id_random].first = motivation[id_existing].first;
    motivation[id_random].last.on = move_effect_journal[movement].u.piece_movement.to;
    motivation[id_random].last.acts_when = nbply;
    motivation[id_random].last.purpose = purpose_capturer;

    /* deactivate the pre-capture insertion of the moving total invisible since
     * that piece is already on the board
     */
    assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
    move_effect_journal[precapture].type = move_effect_none;

    move_effect_journal[movement].u.piece_movement.moving = walk_capturing;
    move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];
    move_effect_journal[movement].u.piece_movement.from = sq_departure;
    /* move_effect_journal[movement].u.piece_movement.to unchanged from regular play */

    remember_taboos_for_current_move();

    allocate_flesh_out_placed(trait[nbply]);

    restart_from_scratch();

    current_consumption = save_consumption;

    forget_taboos_for_current_move();

    move_effect_journal[movement].u.piece_movement.moving = save_moving;
    move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;
    move_effect_journal[movement].u.piece_movement.from = save_from;

    move_effect_journal[precapture].type = move_effect_piece_readdition;

    motivation[id_random] = motivation_random;

    replace_moving_piece_ids_in_past_moves(id_random,id_existing,nbply-1);
  }

  replace_walk(sq_departure,Dummy);
  --being_solved.number_of_pieces[trait[nbply]][walk_capturing];

  being_solved.spec[sq_departure] = flags_existing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_with_matching_walk(piece_walk_type walk_capturing,
                                                    square sq_departure)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const precapture = effects_base;

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  PieceIdType const id_random = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
  motivation_type const motivation_random = motivation[id_random];

  Flags const flags_existing = being_solved.spec[sq_departure];
  PieceIdType const id_existing = GetPieceId(flags_existing);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  SetPieceId(being_solved.spec[sq_departure],id_random);
  replace_moving_piece_ids_in_past_moves(id_existing,id_random,nbply-1);

  /* deactivate the pre-capture insertion of the moving total invisible since
   * that piece is already on the board
   */
  assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
  move_effect_journal[precapture].type = move_effect_none;

  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  /* move_effect_journal[movement].u.piece_movement.to unchanged from regular play */
  move_effect_journal[movement].u.piece_movement.moving = walk_capturing;

  remember_taboos_for_current_move();

  TraceValue("%u",id_random);
  TraceValue("%u",motivation[id_random].first.purpose);
  TraceValue("%u",motivation[id_random].first.acts_when);
  TraceSquare(motivation[id_random].first.on);
  TraceValue("%u",motivation[id_random].last.purpose);
  TraceValue("%u",motivation[id_random].last.acts_when);
  TraceSquare(motivation[id_random].last.on);
  TraceEOL();

  motivation[id_random].first = motivation[id_existing].first;
  motivation[id_random].last.on = move_effect_journal[movement].u.piece_movement.to;
  motivation[id_random].last.acts_when = nbply;
  motivation[id_random].last.purpose = purpose_capturer;

  assert(!TSTFLAG(being_solved.spec[sq_departure],advers(trait[nbply])));
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];
  recurse_into_child_ply();

  motivation[id_random] = motivation_random;

  forget_taboos_for_current_move();

  move_effect_journal[precapture].type = move_effect_piece_readdition;

  replace_moving_piece_ids_in_past_moves(id_random,id_existing,nbply-1);
  being_solved.spec[sq_departure] = flags_existing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_piece_at_end_of_line(piece_walk_type walk_capturing,
                                            square sq_departure)
{
  Flags const flags_existing = being_solved.spec[sq_departure];
  PieceIdType const id_existing = GetPieceId(flags_existing);
  decision_levels_type const save_levels = motivation[id_existing].levels;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  motivation[id_existing].levels.from = curr_decision_level;
  record_decision_square('>',sq_departure);
  ++curr_decision_level;

  if (motivation[id_existing].last.acts_when<nbply
      || ((motivation[id_existing].last.purpose==purpose_interceptor
           || motivation[id_existing].last.purpose==purpose_capturer)
          && motivation[id_existing].last.acts_when<=nbply))
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    PieceIdType const id_random = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);

    motivation_type const motivation_existing = motivation[id_existing];
    piece_walk_type const walk_on_board = get_walk_of_piece_on_square(sq_departure);

    TraceValue("%u",id_existing);
    TraceValue("%u",motivation[id_existing].first.purpose);
    TraceValue("%u",motivation[id_existing].first.acts_when);
    TraceSquare(motivation[id_existing].first.on);
    TraceValue("%u",motivation[id_existing].last.purpose);
    TraceValue("%u",motivation[id_existing].last.acts_when);
    TraceSquare(motivation[id_existing].last.on);
    TraceEOL();

    assert(motivation[id_existing].first.purpose!=purpose_none);
    assert(motivation[id_existing].last.purpose!=purpose_none);

    motivation[id_existing].levels = motivation[id_random].levels;
    motivation[id_existing].last.purpose = purpose_none;

    if (walk_on_board==walk_capturing)
    {
      TraceWalk(get_walk_of_piece_on_square(sq_departure));
      TraceValue("%x",being_solved.spec[sq_departure]);
      TraceEOL();

      capture_by_invisible_with_matching_walk(walk_capturing,sq_departure);
    }
    else if (walk_on_board==Dummy)
    {
      move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

      move_effect_journal_index_type const precapture = effects_base;
      Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
      PieceIdType const id_inserted = GetPieceId(flags_inserted);

      TraceWalk(get_walk_of_piece_on_square(sq_departure));
      TraceValue("%x",being_solved.spec[sq_departure]);
      TraceEOL();

      motivation[id_existing].levels.walk = motivation[id_inserted].levels.walk;

      flesh_out_walk_for_capture(walk_capturing,sq_departure);
    }

    motivation[id_existing] = motivation_existing;
  }
  else
  {
    TraceText("the piece was added to later act from its current square\n");
    record_decision_outcome("%s","the piece was added to later act from its current square");
    REPORT_DEADEND;
    max_decision_level = motivation[id_existing].levels.from;
  }

  --curr_decision_level;

  motivation[id_existing].levels = save_levels;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_rider_inserted_or_existing(piece_walk_type walk_rider,
                                                            vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  if (curr_decision_level<=max_decision_level)
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

    move_effect_journal_index_type const precapture = effects_base;
    Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
    PieceIdType const id_inserted = GetPieceId(flags_inserted);

    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

    max_decision_level = decision_level_latest;

    TraceSquare(sq_arrival);TraceEOL();

    motivation[id_inserted].levels.walk = curr_decision_level;
    record_decision_walk('>',walk_rider);
    ++curr_decision_level;

    TraceValue("%u",curr_decision_level);
    TraceValue("%u",max_decision_level);
    TraceEOL();

    for (; kcurr<=kend && curr_decision_level<=max_decision_level; ++kcurr)
    {
      square sq_departure;

      decision_level_dir[curr_decision_level] = '>';
      ++curr_decision_level;

      max_decision_level = decision_level_latest;

      for (sq_departure = sq_arrival+vec[kcurr];
           is_square_empty(sq_departure) && curr_decision_level<=max_decision_level;
           sq_departure += vec[kcurr])
      {
        max_decision_level = decision_level_latest;
        flesh_out_capture_by_inserted_invisible(walk_rider,sq_departure);
      }

      --curr_decision_level;

      TraceValue("%u",curr_decision_level);
      TraceValue("%u",max_decision_level);
      TraceEOL();

      if (curr_decision_level<=max_decision_level)
      {
        while (is_square_empty(sq_departure))
        {
          sq_departure += vec[kcurr];
          TraceSquare(sq_departure);TraceEOL();
        }

        if (is_on_board(sq_departure))
        {
          TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],Chameleon));
          TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],trait[nbply]));
          TraceEOL();

          if (TSTFLAG(being_solved.spec[sq_departure],Chameleon)
              && TSTFLAG(being_solved.spec[sq_departure],trait[nbply]))
          {
            max_decision_level = decision_level_latest;

            capture_by_piece_at_end_of_line(walk_rider,sq_departure);
          }
        }
      }
    }

    --curr_decision_level;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_king_inserted_or_existing(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const precapture = effects_base;
  Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
  PieceIdType const id_inserted = GetPieceId(flags_inserted);

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal_index_type const king_square_movement = movement+1;
  vec_index_type kcurr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  motivation[id_inserted].levels.walk = curr_decision_level;
  record_decision_walk('>',King);
  ++curr_decision_level;

  TraceValue("%u",curr_decision_level);
  TraceValue("%u",max_decision_level);
  TraceEOL();

  assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
  assert(!TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal));
  assert(move_effect_journal[king_square_movement].type==move_effect_none);

  for (kcurr = vec_queen_start;
       kcurr<=vec_queen_end && curr_decision_level<=max_decision_level;
       ++kcurr)
  {
    square const sq_departure = sq_arrival+vec[kcurr];

    max_decision_level = decision_level_latest;

    if (is_square_empty(sq_departure))
    {
      if (being_solved.king_square[trait[nbply]]==initsquare)
      {
        being_solved.king_square[trait[nbply]] = sq_departure;

        move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
        move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
        move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
        move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

        assert(!TSTFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal));
        SETFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal);

        decision_level_dir[curr_decision_level] = '>';
        ++curr_decision_level;
        flesh_out_capture_by_inserted_invisible(King,sq_departure);
        --curr_decision_level;

        CLRFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal);

        being_solved.king_square[trait[nbply]] = initsquare;

        move_effect_journal[king_square_movement].type = move_effect_none;
      }
    }
    else
    {
      TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],Chameleon));
      TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],trait[nbply]));
      TraceEOL();

      if (TSTFLAG(being_solved.spec[sq_departure],Chameleon)
          && TSTFLAG(being_solved.spec[sq_departure],trait[nbply]))
      {
        move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
        move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
        move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
        move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

        if (get_walk_of_piece_on_square(sq_departure)==King
            && sq_departure==being_solved.king_square[trait[nbply]])
        {
          assert(TSTFLAG(being_solved.spec[sq_departure],Royal));
          capture_by_piece_at_end_of_line(King,sq_departure);
        }
        else if (get_walk_of_piece_on_square(sq_departure)==Dummy
                 && being_solved.king_square[trait[nbply]]==initsquare)
        {
          being_solved.king_square[trait[nbply]] = sq_departure;

          assert(!TSTFLAG(being_solved.spec[sq_departure],Royal));
          SETFLAG(being_solved.spec[sq_departure],Royal);
          capture_by_piece_at_end_of_line(King,sq_departure);
          CLRFLAG(being_solved.spec[sq_departure],Royal);

          being_solved.king_square[trait[nbply]] = initsquare;
        }

        move_effect_journal[king_square_movement].type = move_effect_none;
      }
    }
  }

  --curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_leaper_inserted_or_existing(piece_walk_type walk_leaper,
                                                             vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  if (curr_decision_level<=max_decision_level)
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

    move_effect_journal_index_type const precapture = effects_base;
    Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
    PieceIdType const id_inserted = GetPieceId(flags_inserted);

    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

    max_decision_level = decision_level_latest;

    motivation[id_inserted].levels.walk = curr_decision_level;
    record_decision_walk('>',walk_leaper);
    ++curr_decision_level;

    TraceValue("%u",curr_decision_level);
    TraceValue("%u",max_decision_level);
    TraceEOL();

    for (; kcurr<=kend && curr_decision_level<=max_decision_level; ++kcurr)
    {
      square const sq_departure = sq_arrival+vec[kcurr];

      max_decision_level = decision_level_latest;

      if (is_square_empty(sq_departure))
      {
        decision_level_dir[curr_decision_level] = '>';
        ++curr_decision_level;
        flesh_out_capture_by_inserted_invisible(walk_leaper,sq_departure);
        --curr_decision_level;
      }
      else
      {
        TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],Chameleon));
        TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],trait[nbply]));
        TraceEOL();

        if (TSTFLAG(being_solved.spec[sq_departure],Chameleon)
            && TSTFLAG(being_solved.spec[sq_departure],trait[nbply]))
          capture_by_piece_at_end_of_line(walk_leaper,sq_departure);
      }
    }

    --curr_decision_level;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_pawn_inserted_or_existing_one_dir(int dir_horiz)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  int const dir_vert = trait[nbply]==White ? -dir_up : -dir_down;
  SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;
  square const sq_departure = sq_capture+dir_vert+dir_horiz;

  TraceFunctionEntry(__func__);
  TraceValue("%d",dir_horiz);
  TraceFunctionParamListEnd();

  // TODO en passant capture

  TraceSquare(sq_departure);TraceEOL();
  if (!TSTFLAG(sq_spec[sq_departure],basesq)
      && !TSTFLAG(sq_spec[sq_departure],promsq))
  {
    max_decision_level = decision_level_latest;

    if (is_square_empty(sq_departure))
    {
      decision_level_dir[curr_decision_level] = '>';
      ++curr_decision_level;
      flesh_out_capture_by_inserted_invisible(Pawn,sq_departure);
      --curr_decision_level;
    }
    else
    {
      TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],Chameleon));
      TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],trait[nbply]));
      TraceEOL();

      if (TSTFLAG(being_solved.spec[sq_departure],Chameleon)
          && TSTFLAG(being_solved.spec[sq_departure],trait[nbply]))
        capture_by_piece_at_end_of_line(Pawn,sq_departure);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_pawn_inserted_or_existing(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (curr_decision_level<=max_decision_level)
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

    move_effect_journal_index_type const precapture = effects_base;
    Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
    PieceIdType const id_inserted = GetPieceId(flags_inserted);

    max_decision_level = decision_level_latest;

    motivation[id_inserted].levels.walk = curr_decision_level;
    record_decision_walk('>',Pawn);
    ++curr_decision_level;

    TraceValue("%u",curr_decision_level);
    TraceValue("%u",max_decision_level);
    TraceEOL();

    capture_by_invisible_pawn_inserted_or_existing_one_dir(dir_left);

    if (curr_decision_level<=max_decision_level)
      capture_by_invisible_pawn_inserted_or_existing_one_dir(dir_right);

    --curr_decision_level;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_inserted_or_existing(boolean can_capture)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const precapture = effects_base;
  Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
  PieceIdType const id_inserted = GetPieceId(flags_inserted);
  decision_levels_type const levels_inserted = motivation[id_inserted].levels;

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const save_from = move_effect_journal[movement].u.piece_movement.from;
  piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceValue("%u",can_capture);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (being_solved.king_square[trait[nbply]]==initsquare)
    capture_by_invisible_king_inserted_or_existing();

  if (can_capture)
  {
    capture_by_invisible_pawn_inserted_or_existing();
    capture_by_invisible_leaper_inserted_or_existing(Knight,vec_knight_start,vec_knight_end);
    capture_by_invisible_rider_inserted_or_existing(Bishop,vec_bishop_start,vec_bishop_end);
    capture_by_invisible_rider_inserted_or_existing(Rook,vec_rook_start,vec_rook_end);
    capture_by_invisible_rider_inserted_or_existing(Queen,vec_queen_start,vec_queen_end);
  }

  move_effect_journal[movement].u.piece_movement.from = save_from;
  move_effect_journal[movement].u.piece_movement.moving = save_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;

  motivation[id_inserted].levels = levels_inserted;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_king_for_capture(square sq_departure)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal_index_type const king_square_movement = movement+1;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  assert(!TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal));
  assert(move_effect_journal[king_square_movement].type==move_effect_none);

  move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
  move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
  move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
  move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

  being_solved.king_square[trait[nbply]] = sq_departure;

  assert(!TSTFLAG(being_solved.spec[sq_departure],Royal));
  SETFLAG(being_solved.spec[sq_departure],Royal);
  flesh_out_walk_for_capture(King,sq_departure);
  CLRFLAG(being_solved.spec[sq_departure],Royal);

  being_solved.king_square[trait[nbply]] = initsquare;

  move_effect_journal[king_square_movement].type = move_effect_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_dummy_for_capture_non_king(square sq_departure,
                                                 square sq_arrival,
                                                 PieceIdType id_existing)
{
  int const move_square_diff = sq_departure-sq_arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceValue("%u",id_existing);
  TraceFunctionParamListEnd();

  if (CheckDir[Bishop][move_square_diff]==move_square_diff
      && (trait[nbply]==White ? sq_departure<sq_arrival : sq_departure>sq_arrival))
  {
    SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
    SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;

    if (!TSTFLAG(sq_spec[sq_departure],basesq) && !TSTFLAG(sq_spec[sq_departure],promsq))
    {
      motivation[id_existing].levels.walk = curr_decision_level;
      record_decision_walk('>',Pawn);
      ++curr_decision_level;
      flesh_out_walk_for_capture(Pawn,sq_departure);
      --curr_decision_level;
    }

    // TODO en passant capture
  }

  if (curr_decision_level<=max_decision_level)
  {
    if (CheckDir[Knight][move_square_diff]==move_square_diff)
    {
      max_decision_level = decision_level_latest;

      motivation[id_existing].levels.walk = curr_decision_level;
      record_decision_walk('>',Knight);
      ++curr_decision_level;
      flesh_out_walk_for_capture(Knight,sq_departure);
      --curr_decision_level;
    }

    if (curr_decision_level<=max_decision_level)
    {
      int const dir = CheckDir[Bishop][move_square_diff];
      if (dir!=0 && sq_departure==find_end_of_line(sq_arrival,dir))
      {
        max_decision_level = decision_level_latest;

        motivation[id_existing].levels.walk = curr_decision_level;
        record_decision_walk('>',Bishop);
        ++curr_decision_level;

        flesh_out_walk_for_capture(Bishop,sq_departure);

        /* Don't reduce curr_decision_level yet; if posteriority asks for a
         * different walk, Queen won't do. */
        // TODO is this correct when we detect revelations? cf. capture_by_invisible_rider_inserted_or_existing()

        if (curr_decision_level<=max_decision_level)
        {
          max_decision_level = decision_level_latest;

          record_decision_walk('>',Queen);
          ++curr_decision_level;
          flesh_out_walk_for_capture(Queen,sq_departure);
          --curr_decision_level;
        }

        --curr_decision_level;
      }

      if (curr_decision_level<=max_decision_level)
      {
        int const dir = CheckDir[Rook][move_square_diff];
        if (dir!=0 && sq_departure==find_end_of_line(sq_arrival,dir))
        {
          max_decision_level = decision_level_latest;

          motivation[id_existing].levels.walk = curr_decision_level;
          record_decision_walk('>',Rook);
          ++curr_decision_level;

          flesh_out_walk_for_capture(Rook,sq_departure);

          /* Don't reduce curr_decision_level yet; if posteriority asks for a
           * different walk, Queen won't do. */
          // TODO is this correct when we detect revelations? cf. capture_by_invisible_rider_inserted_or_existing()

          if (curr_decision_level<=max_decision_level)
          {
            max_decision_level = decision_level_latest;

            record_decision_walk('>',Queen);
            ++curr_decision_level;
            flesh_out_walk_for_capture(Queen,sq_departure);
            --curr_decision_level;
          }

          --curr_decision_level;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_dummy_for_capture_king_or_non_king(square sq_departure,
                                                         square sq_arrival,
                                                         PieceIdType id_existing)
{
  int const move_square_diff = sq_departure-sq_arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceValue("%u",id_existing);
  TraceFunctionParamListEnd();

  assert(being_solved.king_square[trait[nbply]]==initsquare);

  if (CheckDir[Queen][move_square_diff]==move_square_diff)
  {
    motivation[id_existing].levels.walk = curr_decision_level;
    record_decision_walk('>',King);
    ++curr_decision_level;
    flesh_out_king_for_capture(sq_departure);
    --curr_decision_level;
  }

  assert(current_consumption.placed[trait[nbply]]>0);

  if (curr_decision_level<=max_decision_level
      && !(nr_total_invisbles_consumed()==total_invisible_number
           && current_consumption.placed[trait[nbply]]==1))
  {
    max_decision_level = decision_level_latest;
    flesh_out_dummy_for_capture_non_king(sq_departure,sq_arrival,id_existing);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_with_defined_walk(piece_walk_type walk_capturer,
                                                   square sq_departure)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;
  square const save_from = move_effect_journal[movement].u.piece_movement.from;

  Flags const flags_existing = being_solved.spec[sq_departure];
  PieceIdType const id_existing = GetPieceId(flags_existing);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturer);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  motivation[id_existing].levels.walk = curr_decision_level;
  record_decision_walk('>',walk_capturer);
  ++curr_decision_level;

  capture_by_invisible_with_matching_walk(walk_capturer,sq_departure);

  move_effect_journal[movement].u.piece_movement.moving = save_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;
  move_effect_journal[movement].u.piece_movement.from = save_from;

  --curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void capture_by_invisible_king(square sq_departure)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  move_effect_journal_index_type const king_square_movement = movement+1;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  assert(!TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal));

  assert(move_effect_journal[king_square_movement].type==move_effect_none);
  move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
  move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
  move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
  move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

  assert(sq_departure==being_solved.king_square[trait[nbply]]);
  assert(TSTFLAG(being_solved.spec[sq_departure],Royal));

  capture_by_invisible_with_defined_walk(King,sq_departure);

  move_effect_journal[king_square_movement].type = move_effect_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_on(square sq_departure,
                                              boolean is_king_dealt_with)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  Flags const flags_existing = being_solved.spec[sq_departure];
  PieceIdType const id_existing = GetPieceId(flags_existing);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceValue("%u",is_king_dealt_with);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%x",flags_existing);
  TraceEOL();

  TraceValue("%u",id_existing);
  TraceValue("%u",motivation[id_existing].first.purpose);
  TraceValue("%u",motivation[id_existing].first.acts_when);
  TraceSquare(motivation[id_existing].first.on);
  TraceValue("%u",motivation[id_existing].last.purpose);
  TraceValue("%u",motivation[id_existing].last.acts_when);
  TraceSquare(motivation[id_existing].last.on);
  TraceValue("%u",motivation[id_existing].levels.from);
  TraceValue("%u",motivation[id_existing].levels.to);
  TraceValue("%u",motivation[id_existing].levels.side);
  TraceValue("%u",motivation[id_existing].levels.walk);
  TraceWalk(get_walk_of_piece_on_square(motivation[id_existing].last.on));
  TraceValue("%u",GetPieceId(being_solved.spec[motivation[id_existing].last.on]));
  TraceEOL();

  if (!TSTFLAG(flags_existing,trait[nbply]))
  {
    /* candidate belongs to wrong side */
  }
  else if (motivation[id_existing].last.purpose==purpose_none)
  {
    /* piece was replaced, e.g. by a revelation */
  }
  else
  {
    piece_walk_type const walk_existing = get_walk_of_piece_on_square(sq_departure);
    motivation_type const motivation_existing = motivation[id_existing];

    assert(motivation[id_existing].first.purpose!=purpose_none);
    assert(motivation[id_existing].last.purpose!=purpose_none);

    motivation[id_existing].levels.from = curr_decision_level;
    record_decision_square('>',sq_departure);
    ++curr_decision_level;

    if (motivation[id_existing].last.acts_when<nbply
        || ((motivation[id_existing].last.purpose==purpose_interceptor
             || motivation[id_existing].last.purpose==purpose_capturer)
            && motivation[id_existing].last.acts_when<=nbply))
    {
      int const move_square_diff = sq_arrival-sq_departure;

      PieceIdType const id_random = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
      decision_level_type const save_level_walk = motivation[id_existing].levels.walk;

      motivation[id_existing].last.purpose = purpose_none;
      motivation[id_random].levels.walk = motivation[id_existing].levels.walk;

      max_decision_level = decision_level_latest;

      switch (walk_existing)
      {
        case King:
          if (is_king_dealt_with)
          {
            record_decision_outcome("%s","the king has already been dealt with");
            REPORT_DEADEND;
          }
          else if (CheckDir[Queen][move_square_diff]==move_square_diff)
            capture_by_invisible_king(sq_departure);
          else
          {
            record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
            REPORT_DEADEND;
          }
          break;

        case Queen:
        case Rook:
        case Bishop:
        {
          int const dir = CheckDir[walk_existing][move_square_diff];
          if (dir!=0 && sq_departure==find_end_of_line(sq_arrival,-dir))
            capture_by_invisible_with_defined_walk(walk_existing,sq_departure);
          else
          {
            record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
            REPORT_DEADEND;
          }
          break;
        }

        case Knight:
          if (CheckDir[Knight][move_square_diff]==move_square_diff)
            capture_by_invisible_with_defined_walk(Knight,sq_departure);
          else
          {
            record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
            REPORT_DEADEND;
          }
          break;

        case Pawn:
          if ((trait[nbply]==White ? move_square_diff>0 : move_square_diff<0)
              && CheckDir[Bishop][move_square_diff]==move_square_diff)
          {
            SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
            SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;

            if (!TSTFLAG(sq_spec[sq_departure],basesq)
                && !TSTFLAG(sq_spec[sq_departure],promsq))
              capture_by_invisible_with_defined_walk(Pawn,sq_departure);
            // TODO en passant capture
          }
          else
          {
            record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
            REPORT_DEADEND;
          }
          break;

        case Dummy:
          if (CheckDir[Queen][move_square_diff]!=0
              || CheckDir[Knight][move_square_diff]==move_square_diff)
          {
            if (is_king_dealt_with
                || being_solved.king_square[trait[nbply]]!=initsquare)
              flesh_out_dummy_for_capture_non_king(sq_departure,sq_arrival,id_existing);
            else
              flesh_out_dummy_for_capture_king_or_non_king(sq_departure,sq_arrival,id_existing);
          }
          else
          {
            record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
            REPORT_DEADEND;
            // TODO do motivation[id_existing].levels = motivation[id_random].levels later
            // so that we can use motivation[id_existing] here?
            if (static_consumption.king[advers(trait[nbply])]+static_consumption.pawn_victims[advers(trait[nbply])]+1
                >=total_invisible_number)
            {
              /* move our single piece to a different square
               * or let another piece be our single piece */
              max_decision_level = motivation_existing.levels.to;
              if (max_decision_level<motivation_existing.levels.side)
                max_decision_level = motivation_existing.levels.side;
            }
          }
          break;

        default:
          assert(0);
          break;
      }

      motivation[id_random].levels.walk = save_level_walk;
    }
    else
    {
      TraceText("the piece was added to later act from its current square\n");
      record_decision_outcome("%s","the piece was added to later act from its current square");
      REPORT_DEADEND;
      max_decision_level = motivation[id_existing].levels.from;
    }

    motivation[id_existing] = motivation_existing;

    --curr_decision_level;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_walk_by_walk(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    dynamic_consumption_type const save_consumption = current_consumption;

    if (allocate_flesh_out_unplaced(trait[nbply]))
    {
      current_consumption = save_consumption;
      /* no problem - we can simply insert a capturer */
      TraceText("we can insert a capturer if needed\n");
      capture_by_invisible_inserted_or_existing(true);
    }
    else
    {
      boolean can_king_be_inserted;

      TraceText("we can't just insert a capturer\n");

      current_consumption = save_consumption;

      can_king_be_inserted = (being_solved.king_square[trait[nbply]]==initsquare
                              && current_consumption.claimed[trait[nbply]]);

      TraceValue("%u",can_king_be_inserted);TraceEOL();

      if (can_king_be_inserted)
      {
        /* no problem - we can simply insert a capturing king */
        move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

        move_effect_journal_index_type const precapture = effects_base;
        Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
        PieceIdType const id_inserted = GetPieceId(flags_inserted);
        decision_levels_type const levels_inserted = motivation[id_inserted].levels;

        move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
        square const save_from = move_effect_journal[movement].u.piece_movement.from;
        piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
        Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;

        assert(move_effect_journal[movement].type==move_effect_piece_movement);

        max_decision_level = decision_level_latest;

        motivation[id_inserted].levels.walk = curr_decision_level;
        motivation[id_inserted].levels.from = curr_decision_level+1;

        capture_by_invisible_king_inserted_or_existing();

        move_effect_journal[movement].u.piece_movement.from = save_from;
        move_effect_journal[movement].u.piece_movement.moving = save_moving;
        move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;

        motivation[id_inserted].levels = levels_inserted;
      }

      {
        PieceIdType id;
        for (id = get_top_visible_piece_id()+1;
             id<=get_top_invisible_piece_id() && curr_decision_level<=max_decision_level;
             ++id)
          flesh_out_capture_by_invisible_on(motivation[id].last.on,
                                            can_king_be_inserted);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void flesh_out_capture_by_invisible(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const precapture = effects_base;
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  piece_walk_type const save_removed_walk = move_effect_journal[capture].u.piece_removal.walk;
  Flags const save_removed_spec = move_effect_journal[capture].u.piece_removal.flags;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  Flags const flags = move_effect_journal[precapture].u.piece_addition.added.flags;
  PieceIdType const id_inserted = GetPieceId(flags);
  decision_levels_type const save_levels = motivation[id_inserted].levels;

  unsigned int const save_counter = record_decision_counter;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_capture);TraceEOL();
  assert(!is_square_empty(sq_capture));

  motivation[id_inserted].levels.side = decision_level_forever;
  motivation[id_inserted].levels.to = decision_level_forever;

  move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(sq_capture);
  move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[sq_capture];

  flesh_out_capture_by_invisible_walk_by_walk();

  move_effect_journal[capture].u.piece_removal.walk = save_removed_walk;
  move_effect_journal[capture].u.piece_removal.flags = save_removed_spec;

  if (record_decision_counter==save_counter)
  {
    record_decision_outcome("%s","no invisible piece found that could capture");
    REPORT_DEADEND;
  }

  motivation[id_inserted].levels = save_levels;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_viable_capturer(PieceIdType id, ply ply_capture)
{
  Side const side_capturing = trait[ply_capture];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_capture);
  TraceFunctionParamListEnd();

  TraceValue("%u",id);TraceEOL();
  TraceAction(&motivation[id].first);TraceEOL();
  TraceAction(&motivation[id].last);TraceEOL();
  TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
  TraceEOL();

  if ((trait[motivation[id].first.acts_when]!=side_capturing && motivation[id].first.purpose==purpose_random_mover)
      || (trait[motivation[id].last.acts_when]!=side_capturing && motivation[id].last.purpose==purpose_random_mover))
  {
    /* piece belongs to wrong side */
    result = false;
  }
  else if (motivation[id].last.acts_when<=nbply && motivation[id].last.purpose==purpose_none)
  {
    /* piece was captured or merged into a capturer from regular play */
    result = false;
  }
  else if ((motivation[id].last.acts_when==nbply || motivation[id].last.acts_when==ply_capture)
           && motivation[id].last.purpose!=purpose_interceptor)
  {
    /* piece is active for another purpose */
    result = false;
  }
  else if (motivation[id].last.acts_when>ply_capture)
  {
    /* piece will be active after the capture */
    result = false;
  }
  else if (motivation[id].first.on==initsquare)
  {
    /* revealed piece - to be replaced by an "actual" piece */
    result = false;
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_capture_by_invisible(ply ply_capture)
{
  boolean result;
  move_effect_journal_index_type const base = move_effect_journal_base[ply_capture];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_capture);
  TraceFunctionParamListEnd();

  if (ply_capture<=top_ply_of_regular_play
      && sq_departure>=capture_by_invisible
      && is_on_board(sq_arrival))
    result = true;
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_capture_by_existing_invisible_possible(ply ply_capture)
{
  /* only captures by existing invisibles are viable - can one of them reach the arrival square at all? */
  boolean result = false; /* not until we have proved it */

  move_effect_journal_index_type const effects_base = move_effect_journal_base[ply_capture];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  PieceIdType id;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_capture);
  TraceFunctionParamListEnd();

  TraceSquare(sq_arrival);
  TraceValue("%u",nbply);
  TraceEOL();

  for (id = get_top_visible_piece_id()+1;
       !result && id<=get_top_invisible_piece_id();
       ++id)
    if (is_viable_capturer(id,ply_capture))
    {
      square const on = motivation[id].last.on;
      Flags const spec = being_solved.spec[on];

      assert(GetPieceId(spec)==id);

      if (TSTFLAG(spec,trait[ply_capture]))
      {
        piece_walk_type const walk = get_walk_of_piece_on_square(on);
        int const diff = sq_arrival-on;

        switch (walk)
        {
          case King:
            if (CheckDir[Queen][diff]==diff)
              result = true;
            break;

          case Queen:
            if (CheckDir[Queen][diff]!=0)
              result = true;
            break;

          case Rook:
            if (CheckDir[Rook][diff]!=0)
              result = true;
            break;

          case Bishop:
            if (CheckDir[Bishop][diff]!=0)
              result = true;
            break;

          case Knight:
            if (CheckDir[Knight][diff]==diff)
              result = true;
            break;

          case Pawn:
            if (CheckDir[Bishop][diff]==diff
                && (trait[nbply]==White ? diff>0 : diff<0))
              result = true;
            break;

          case Dummy:
            if (CheckDir[Queen][diff]!=0 || CheckDir[Knight][diff]==diff)
              result = true;
            break;

          default:
            assert(0);
            break;
        }
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_capture_by_invisible_possible(void)
{
  ply const ply_capture = nbply+1;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_capture_by_invisible(ply_capture))
  {
    dynamic_consumption_type const save_consumption = current_consumption;

    if (allocate_flesh_out_unplaced(trait[ply_capture]))
    {
      /* no problem - we can simply insert a capturer */
    }
    else
    {
      square const save_king_square = being_solved.king_square[trait[ply_capture]];

      /* pretend that the king is placed; necessary if only captures by the invisble king
       * are possisble */
      being_solved.king_square[trait[ply_capture]] = square_a1;

      current_consumption = save_consumption;

      if (allocate_flesh_out_unplaced(trait[ply_capture]))
      {
        /* no problem - we can simply insert a capturing king */
      }
      else
        result = is_capture_by_existing_invisible_possible(ply_capture);

      being_solved.king_square[trait[ply_capture]] = save_king_square;
    }

    current_consumption = save_consumption;
  }
  else
  {
    if (ply_capture<=top_ply_of_regular_play)
    {
      move_effect_journal_index_type const effects_base_capture = move_effect_journal_base[ply_capture];

      move_effect_journal_index_type const capture_capture = effects_base_capture+move_effect_journal_index_offset_capture;
      square const sq_capture_capture = move_effect_journal[capture_capture].u.piece_removal.on;

      move_effect_journal_index_type const movement_capture = effects_base_capture+move_effect_journal_index_offset_movement;
      piece_walk_type const capturer = move_effect_journal[movement_capture].u.piece_movement.moving;

      TraceValue("%u",ply_capture);
      TraceSquare(sq_capture_capture);
      TraceWalk(capturer);
      TraceEOL();

      if (move_effect_journal[capture_capture].type==move_effect_piece_removal
          && capturer==Pawn
          && (is_square_empty(sq_capture_capture)
              || TSTFLAG(being_solved.spec[sq_capture_capture],advers(trait[nbply]))))
      {
        boolean const is_sacrifice_capture = !is_square_empty(sq_capture_capture);
        dynamic_consumption_type const save_consumption = current_consumption;

        TraceText("pawn capture in next move - no victim to be seen yet\n");

        if (allocate_flesh_out_unplaced(trait[nbply]))
        {
          TraceText("allocation of a victim in the next move still possible\n");
        }
        else
        {
          move_effect_journal_index_type const effects_base_now = move_effect_journal_base[nbply];

          move_effect_journal_index_type const movement_now = effects_base_now+move_effect_journal_index_offset_movement;
          square const sq_departure_now = move_effect_journal[movement_now].u.piece_movement.from;
          square const sq_arrival_now = move_effect_journal[movement_now].u.piece_movement.to;

          TraceText("allocation of a victim in the next move impossible - test possibility of sacrifice\n");

          if (sq_departure_now==move_by_invisible
              && sq_arrival_now==move_by_invisible)
          {
            square const *curr;

            TraceText("try to find a potential sacrifice by an invisible\n");

            result = false;

            for (curr = find_next_forward_mover(boardnum);
                 !result && *curr;
                 curr = find_next_forward_mover(curr+1))
            {
              piece_walk_type const walk = get_walk_of_piece_on_square(*curr);
              int const diff = sq_capture_capture-*curr;

              TraceSquare(*curr);
              TraceWalk(walk);
              TraceValue("%d",diff);
              TraceEOL();

              switch (walk)
              {
                case King:
                  if (CheckDir[Queen][diff]==diff)
                    result = true;
                  break;

                case Queen:
                  if (CheckDir[Queen][diff]!=0)
                    result = true;
                  break;

                case Rook:
                  if (CheckDir[Rook][diff]!=0)
                    result = true;
                  break;

                case Bishop:
                  if (CheckDir[Bishop][diff]!=0)
                    result = true;
                  break;

                case Knight:
                  if (CheckDir[Knight][diff]==diff)
                    result = true;
                  break;

                case Pawn:
                  if (is_sacrifice_capture)
                  {
                    if (CheckDir[Bishop][diff]==diff
                        && (trait[nbply]==White ? diff>0 : diff<0))
                      result = true;
                  }
                  else
                  {
                    if ((CheckDir[Rook][diff]==diff || CheckDir[Rook][diff]==diff/2)
                        && (trait[nbply]==White ? diff>0 : diff<0))
                      result = true;
                  }
                  break;

                case Dummy:
                  if (CheckDir[Queen][diff]!=0 || CheckDir[Knight][diff]==diff)
                    result = true;
                  break;

                default:
                  assert(0);
                  break;
              }
            }

            TraceText(result
                      ? "found a potential sacrifice by an invisible\n"
                      : "couldn't find a potential sacrifice by an invisible\n");
          }
          else if (sq_arrival_now==sq_capture_capture)
          {
            TraceText("this move sacrifices a visible\n");
          }
          else
          {
            TraceText("no sacrifice in this move\n");
            result = false;
          }
        }

        current_consumption = save_consumption;
      }
    }
  }

  if (!result)
  {
    max_decision_level = curr_decision_level-1;
    while (decision_level_dir[max_decision_level]=='<')
      --max_decision_level;
    // TODO optimise more aggressively?
    // this will stop at line pieces' vectors - can we rule out all moves by inserted piece?
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void fake_capture_by_invisible(void)
{
  PieceIdType const id_capturer = initialise_motivation(purpose_capturer,capture_by_invisible,
                                                        purpose_capturer,capture_by_invisible);
  ply const save_ply = uninterceptable_check_delivered_in_ply;

  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const precapture = effects_base;
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_entry_type const save_movement_entry = move_effect_journal[movement];

  Side const side = trait[nbply];
  Flags spec = BIT(side)|BIT(Chameleon);

  unsigned int const save_counter = record_decision_counter;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(!is_square_empty(uninterceptable_check_delivered_from));

  SetPieceId(spec,id_capturer);

  assert(move_effect_journal[precapture].type==move_effect_none);
  move_effect_journal[precapture].type = move_effect_piece_readdition;
  move_effect_journal[precapture].u.piece_addition.added.on = capture_by_invisible;
  move_effect_journal[precapture].u.piece_addition.added.walk = Dummy;
  move_effect_journal[precapture].u.piece_addition.added.flags = spec;
  move_effect_journal[precapture].u.piece_addition.for_side = side;

  assert(move_effect_journal[capture].type==move_effect_no_piece_removal);
  move_effect_journal[capture].type = move_effect_piece_removal;
  move_effect_journal[capture].u.piece_removal.on = uninterceptable_check_delivered_from;
  move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(uninterceptable_check_delivered_from);
  move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[uninterceptable_check_delivered_from];

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  move_effect_journal[movement].type = move_effect_piece_movement;
  move_effect_journal[movement].u.piece_movement.from = capture_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = uninterceptable_check_delivered_from;
  move_effect_journal[movement].u.piece_movement.moving = Dummy;
  move_effect_journal[movement].u.piece_movement.movingspec = spec;

  ++being_solved.number_of_pieces[trait[nbply]][Dummy];
  occupy_square(capture_by_invisible,Dummy,spec);

  uninterceptable_check_delivered_from = initsquare;
  uninterceptable_check_delivered_in_ply = ply_nil;

  flesh_out_capture_by_invisible();

  uninterceptable_check_delivered_in_ply = save_ply;
  uninterceptable_check_delivered_from = move_effect_journal[capture].u.piece_removal.on;

  empty_square(capture_by_invisible);
  --being_solved.number_of_pieces[trait[nbply]][Dummy];

  move_effect_journal[movement] = save_movement_entry;
  move_effect_journal[capture].type = move_effect_no_piece_removal;
  move_effect_journal[precapture].type = move_effect_none;

  uninitialise_motivation(id_capturer);

  if (record_decision_counter==save_counter)
  {
    record_decision_outcome("%s","no invisible piece found that could capture the uninterceptable check deliverer");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
