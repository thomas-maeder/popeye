#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "solving/ply.h"
#include "solving/move_effect_journal.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static unsigned int capture_by_invisible_inserted_on(piece_walk_type walk_capturing,
                                                     square sq_departure)
{
  unsigned int result = 0;
  Side const side_playing = trait[nbply];
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const precapture = effects_base;
  Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
  PieceIdType const id_inserted = GetPieceId(flags_inserted);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  TraceValue("%u",id_inserted);TraceEOL();

  assert(is_on_board(sq_departure));

  if (was_taboo(sq_departure,side_playing) || is_taboo(sq_departure,side_playing))
  {
    record_decision_outcome("%s","capturer can't be placed on taboo square");
    REPORT_DEADEND;
  }
  else
  {
    Side const side_in_check = trait[nbply-1];
    square const king_pos = being_solved.king_square[side_in_check];

    TraceConsumption();TraceEOL();
    assert(nr_total_invisbles_consumed()<=total_invisible_number);

    push_decision_departure(id_inserted,sq_departure,decision_purpose_invisible_capturer_inserted);

    ++being_solved.number_of_pieces[side_playing][walk_capturing];
    occupy_square(sq_departure,walk_capturing,flags_inserted);

    if (king_pos!=initsquare && is_square_uninterceptably_attacked(side_in_check,king_pos))
    {
      record_decision_outcome("%s","capturer would deliver uninterceptable check");
      REPORT_DEADEND;
      /* e.g.
begin
pieces TotalInvisible 1 white kc4 ra1b1 pa5 black ka6
stipulation h#1.5
option movenum start 2:2
end

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6  -K   .   .   .   .   .   .   .   6
|                                   |
5   P   .   .   .   .   .   .   .   5
|                                   |
4   .   .   K   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   R   R   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
h#1.5                4 + 1 + 1 TI

2  (Ra1-a4    Time = 0.037 s)

!make_revelations 6:Ra1-a4 7:TI~-a4 - revelations.c:#1430 - D:1 - 0
use option start 2:2 to replay
!  2 X 7 I (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#1051 - D:2
!   3 X 7 P (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#468 - D:4
!    4 X 7 b5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#49 - D:6
!     5 7 capturer would deliver uninterceptable check - capture_by_invisible.c:#56

HERE

!   3 X 7 S (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#444 - D:8
!    4 X 7 b6 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#49 - D:10
!     5 7 capturer would deliver uninterceptable check - capture_by_invisible.c:#56
!    4 X 7 c5 (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+1) - capture_by_invisible.c:#49 - D:12
!     5 8 initialised revelation candidates. 1 found - revelations.c:#1461
!   3 X 7 B (K:0+0 x:0+0 !:0+0 ?:0+0 F:0+0) - capture_by_invisible.c:#342 - D:14
...
       */

      backtrack_definitively();
      backtrack_no_further_than(decision_levels[id_inserted].from);
    }
//    else if (walk_capturing==King
//             && is_square_attacked_by_uninterceptable(side_playing,sq_departure))
//    {
//      record_decision_outcome("%s","capturer would expose itself to check by uninterceptable");
// WRONG - such a check is legal if it has just been delivered!
// TODO can we detect checks that have not just been delivered? should we?
//      REPORT_DEADEND;
//    }
    else
    {
      move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
      square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
      motivation_type const save_motivation = motivation[id_inserted];

      assert(!TSTFLAG(being_solved.spec[sq_departure],advers(trait[nbply])));

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

      ++result;
    }

    empty_square(sq_departure);
    --being_solved.number_of_pieces[side_playing][walk_capturing];

    pop_decision();

    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void flesh_out_dummy_for_capture_as(piece_walk_type walk_capturing,
                                           square sq_departure)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  Flags const flags_existing = being_solved.spec[sq_departure];

  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const precapture = effects_base;
  PieceIdType const id_inserted = GetPieceId(move_effect_journal[precapture].u.piece_addition.added.flags);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  CLRFLAG(being_solved.spec[sq_departure],advers(trait[nbply]));
  SetPieceId(being_solved.spec[sq_departure],id_inserted);

  ++being_solved.number_of_pieces[trait[nbply]][walk_capturing];
  replace_walk(sq_departure,walk_capturing);

  if (king_pos!=initsquare && is_square_uninterceptably_attacked(side_in_check,king_pos))
  {
    record_decision_outcome("%s","uninterceptable check from the attempted departure square");
    REPORT_DEADEND;
  }
  else if (walk_capturing==King
           && is_square_attacked_by_uninterceptable(trait[nbply],sq_departure))
  {
    record_decision_outcome("%s","capturer would expose itself to check by uninterceptable");
    REPORT_DEADEND;
  }
  else
  {
    Side const side_playing = trait[nbply];

    PieceIdType const id_existing = GetPieceId(flags_existing);

    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

    piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
    Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;
    square const save_from = move_effect_journal[movement].u.piece_movement.from;

    motivation_type const motivation_inserted = motivation[id_inserted];

    dynamic_consumption_type const save_consumption = current_consumption;

    push_decision_walk(id_existing,walk_capturing,decision_purpose_invisible_capturer_existing,side_playing);
    decision_levels[id_inserted].walk = decision_levels[id_existing].walk;

    replace_moving_piece_ids_in_past_moves(id_existing,id_inserted,nbply-1);

    motivation[id_inserted].first = motivation[id_existing].first;
    motivation[id_inserted].last.on = move_effect_journal[movement].u.piece_movement.to;
    motivation[id_inserted].last.acts_when = nbply;
    motivation[id_inserted].last.purpose = purpose_capturer;

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

    allocate_flesh_out_placed(side_playing);

    TraceSquare(move_effect_journal[movement].u.piece_movement.to);
    TraceValue("%u",ForwardPromSq(trait[nbply],move_effect_journal[movement].u.piece_movement.to));
    TraceEOL();

    if (walk_capturing==Pawn
        && ForwardPromSq(side_playing,move_effect_journal[movement].u.piece_movement.to))
    {
      move_effect_journal_index_type const promotion = movement+1;
      pieces_pawns_promotion_sequence_type sequence = {
          pieces_pawns_promotee_chain_orthodox,
          pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]
      };

      assert(move_effect_journal[promotion].type==move_effect_none);

      move_effect_journal[promotion].type = move_effect_walk_change;
      move_effect_journal[promotion].u.piece_walk_change.from = Pawn;
      move_effect_journal[promotion].u.piece_walk_change.on = move_effect_journal[movement].u.piece_movement.to;

      do
      {
        push_decision_walk(id_existing,sequence.promotee,decision_purpose_invisible_capturer_existing,side_playing);
        move_effect_journal[promotion].u.piece_walk_change.to = sequence.promotee;
        restart_from_scratch();
        pieces_pawns_continue_promotee_sequence(&sequence);
        pop_decision();
      } while (sequence.promotee!=Empty && can_decision_level_be_continued());

      move_effect_journal[promotion].type = move_effect_none;
    }
    else
      restart_from_scratch();

    current_consumption = save_consumption;

    forget_taboos_for_current_move();

    move_effect_journal[movement].u.piece_movement.moving = save_moving;
    move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;
    move_effect_journal[movement].u.piece_movement.from = save_from;

    move_effect_journal[precapture].type = move_effect_piece_readdition;

    motivation[id_inserted] = motivation_inserted;

    replace_moving_piece_ids_in_past_moves(id_inserted,id_existing,nbply-1);

    pop_decision();
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
  PieceIdType const id_inserted = GetPieceId(move_effect_journal[precapture].u.piece_movement.movingspec);
  motivation_type const motivation_inserted = motivation[id_inserted];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

  Flags const flags_existing = being_solved.spec[sq_departure];
  PieceIdType const id_existing = GetPieceId(flags_existing);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  SetPieceId(being_solved.spec[sq_departure],id_inserted);
  replace_moving_piece_ids_in_past_moves(id_existing,id_inserted,nbply-1);

  decision_levels[id_inserted].walk = decision_levels[id_existing].walk;

  /* deactivate the pre-capture insertion of the moving total invisible since
   * that piece is already on the board
   */
  assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
  move_effect_journal[precapture].type = move_effect_none;

  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  /* move_effect_journal[movement].u.piece_movement.to unchanged from regular play */
  move_effect_journal[movement].u.piece_movement.moving = walk_capturing;

  remember_taboos_for_current_move();

  TraceValue("%u",id_inserted);
  TraceValue("%u",motivation[id_inserted].first.purpose);
  TraceValue("%u",motivation[id_inserted].first.acts_when);
  TraceSquare(motivation[id_inserted].first.on);
  TraceValue("%u",motivation[id_inserted].last.purpose);
  TraceValue("%u",motivation[id_inserted].last.acts_when);
  TraceSquare(motivation[id_inserted].last.on);
  TraceEOL();

  motivation[id_inserted].first = motivation[id_existing].first;
  motivation[id_inserted].last.on = move_effect_journal[movement].u.piece_movement.to;
  motivation[id_inserted].last.acts_when = nbply;
  motivation[id_inserted].last.purpose = purpose_capturer;

  assert(!TSTFLAG(being_solved.spec[sq_departure],advers(trait[nbply])));
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];
  recurse_into_child_ply();

  motivation[id_inserted] = motivation_inserted;

  forget_taboos_for_current_move();

  move_effect_journal[precapture].type = move_effect_piece_readdition;

  replace_moving_piece_ids_in_past_moves(id_inserted,id_existing,nbply-1);
  being_solved.spec[sq_departure] = flags_existing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int capture_by_invisible_rider_inserted(piece_walk_type walk_rider,
                                                        vec_index_type kcurr, vec_index_type kend)
{
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  if (can_decision_level_be_continued())
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

    move_effect_journal_index_type const precapture = effects_base;
    Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
    PieceIdType const id_inserted = GetPieceId(flags_inserted);

    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

    TraceSquare(sq_arrival);TraceEOL();

    push_decision_walk(id_inserted,walk_rider,decision_purpose_invisible_capturer_inserted,trait[nbply]);

    for (; kcurr<=kend && can_decision_level_be_continued(); ++kcurr)
    {
      square sq_departure;

      push_decision_move_vector(id_inserted,kcurr,decision_purpose_invisible_capturer_inserted);

      for (sq_departure = sq_arrival+vec[kcurr];
           is_square_empty(sq_departure) && can_decision_level_be_continued();
           sq_departure += vec[kcurr])
        result += capture_by_invisible_inserted_on(walk_rider,sq_departure);

      pop_decision();
    }

    pop_decision();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int capture_by_inserted_invisible_king(void)
{
  unsigned int result = 0;

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

  push_decision_walk(id_inserted,King,decision_purpose_invisible_capturer_inserted,trait[nbply]);

  assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
  assert(!TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal));
  assert(move_effect_journal[king_square_movement].type==move_effect_none);

  for (kcurr = vec_queen_start;
       kcurr<=vec_queen_end && can_decision_level_be_continued();
       ++kcurr)
  {
    square const sq_departure = sq_arrival+vec[kcurr];

    if (is_square_empty(sq_departure))
    {
      assert(current_consumption.is_king_unplaced[trait[nbply]]);
      being_solved.king_square[trait[nbply]] = sq_departure;
      current_consumption.is_king_unplaced[trait[nbply]] = false;

      move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
      move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
      move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
      move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

      assert(!TSTFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal));
      SETFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal);

      result += capture_by_invisible_inserted_on(King,sq_departure);

      CLRFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal);

      current_consumption.is_king_unplaced[trait[nbply]] = true;
      being_solved.king_square[trait[nbply]] = initsquare;

      move_effect_journal[king_square_movement].type = move_effect_none;
    }
  }

  pop_decision();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int capture_by_invisible_leaper_inserted(piece_walk_type walk_leaper,
                                                         vec_index_type kcurr, vec_index_type kend)
{
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  if (can_decision_level_be_continued())
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

    move_effect_journal_index_type const precapture = effects_base;
    Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
    PieceIdType const id_inserted = GetPieceId(flags_inserted);

    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

    push_decision_walk(id_inserted,walk_leaper,decision_purpose_invisible_capturer_inserted,trait[nbply]);

    for (; kcurr<=kend && can_decision_level_be_continued(); ++kcurr)
    {
      square const sq_departure = sq_arrival+vec[kcurr];

      if (is_square_empty(sq_departure))
        result += capture_by_invisible_inserted_on(walk_leaper,sq_departure);
    }

    pop_decision();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int capture_by_invisible_pawn_inserted_one_dir(PieceIdType id_inserted, int dir_horiz)
{
  unsigned int result = 0;

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

  if (is_on_board(sq_departure))
  {
      if (!TSTFLAG(sq_spec(sq_departure),basesq)
        && !TSTFLAG(sq_spec(sq_departure),promsq))
    {
      if (is_square_empty(sq_departure))
        result += capture_by_invisible_inserted_on(Pawn,sq_departure);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int capture_by_invisible_pawn_inserted(void)
{
  unsigned result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (can_decision_level_be_continued())
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

    move_effect_journal_index_type const precapture = effects_base;
    Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
    PieceIdType const id_inserted = GetPieceId(flags_inserted);

    push_decision_walk(id_inserted,Pawn,decision_purpose_invisible_capturer_inserted,trait[nbply]);

    result += capture_by_invisible_pawn_inserted_one_dir(id_inserted,dir_left);

    if (can_decision_level_be_continued())
      result += capture_by_invisible_pawn_inserted_one_dir(id_inserted,dir_right);

    pop_decision();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int capture_by_inserted_invisible_all_walks(void)
{
  unsigned int result = 0;
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const save_from = move_effect_journal[movement].u.piece_movement.from;
  piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (current_consumption.is_king_unplaced[trait[nbply]])
    result += capture_by_inserted_invisible_king();

  result += capture_by_invisible_pawn_inserted();
  result += capture_by_invisible_leaper_inserted(Knight,vec_knight_start,vec_knight_end);
  result += capture_by_invisible_rider_inserted(Bishop,vec_bishop_start,vec_bishop_end);
  result += capture_by_invisible_rider_inserted(Rook,vec_rook_start,vec_rook_end);
  result += capture_by_invisible_rider_inserted(Queen,vec_queen_start,vec_queen_end);

  move_effect_journal[movement].u.piece_movement.from = save_from;
  move_effect_journal[movement].u.piece_movement.moving = save_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void flesh_out_dummy_for_capture_king(square sq_departure,
                                             PieceIdType id_existing)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal_index_type const king_square_movement = movement+1;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%u",id_existing);
  TraceFunctionParamListEnd();

  assert(!TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal));
  assert(move_effect_journal[king_square_movement].type==move_effect_none);

  move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
  move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
  move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
  move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

  being_solved.king_square[trait[nbply]] = sq_departure;
  current_consumption.is_king_unplaced[trait[nbply]] = false;

  assert(!TSTFLAG(being_solved.spec[sq_departure],Royal));
  SETFLAG(being_solved.spec[sq_departure],Royal);
  flesh_out_dummy_for_capture_as(King,sq_departure);
  CLRFLAG(being_solved.spec[sq_departure],Royal);

  current_consumption.is_king_unplaced[trait[nbply]] = true;
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

  if (CheckDir(Bishop)[move_square_diff]==move_square_diff
      && (trait[nbply]==White ? sq_departure<sq_arrival : sq_departure>sq_arrival))
  {
    SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
    SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;

    if (!TSTFLAG(sq_spec(sq_departure),basesq) && !TSTFLAG(sq_spec(sq_departure),promsq))
      flesh_out_dummy_for_capture_as(Pawn,sq_departure);

    // TODO en passant capture
  }

  if (can_decision_level_be_continued())
  {
    boolean try_queen = false;

    if (CheckDir(Knight)[move_square_diff]==move_square_diff)
      flesh_out_dummy_for_capture_as(Knight,sq_departure);

    if (can_decision_level_be_continued())
    {
      int const dir = CheckDir(Bishop)[move_square_diff];
      if (dir!=0 && sq_departure==find_end_of_line(sq_arrival,dir))
      {
        try_queen = true;
        flesh_out_dummy_for_capture_as(Bishop,sq_departure);
      }
    }

    if (can_decision_level_be_continued())
    {
      int const dir = CheckDir(Rook)[move_square_diff];
      if (dir!=0 && sq_departure==find_end_of_line(sq_arrival,dir))
      {
        try_queen = true;
        flesh_out_dummy_for_capture_as(Rook,sq_departure);
      }
    }

    if (can_decision_level_be_continued() && try_queen)
      flesh_out_dummy_for_capture_as(Queen,sq_departure);
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

  if (CheckDir(Queen)[move_square_diff]==move_square_diff)
    flesh_out_dummy_for_capture_king(sq_departure,id_existing);

  assert(current_consumption.placed[trait[nbply]]>0);

  if (can_decision_level_be_continued()
      && !(nr_total_invisbles_consumed()==total_invisible_number
           && current_consumption.placed[trait[nbply]]==1))
    flesh_out_dummy_for_capture_non_king(sq_departure,sq_arrival,id_existing);

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

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturer);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  capture_by_invisible_with_matching_walk(walk_capturer,sq_departure);

  move_effect_journal[movement].u.piece_movement.moving = save_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;
  move_effect_journal[movement].u.piece_movement.from = save_from;

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

/*
 * @return true iff the existing invisible can geometrically reach the capture square
 */
static boolean capture_by_existing_invisible_on(square sq_departure)
{
  boolean result = false;
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const precapture = effects_base;
  PieceIdType const id_inserted = GetPieceId(move_effect_journal[precapture].u.piece_movement.movingspec);

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  Flags const flags_existing = being_solved.spec[sq_departure];
  PieceIdType const id_existing = GetPieceId(flags_existing);
  motivation_type const motivation_existing = motivation[id_existing];
  piece_walk_type const walk_existing = get_walk_of_piece_on_square(sq_departure);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
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
  TraceValue("%u",decision_levels[id_existing].from);
  TraceValue("%u",decision_levels[id_existing].to);
  TraceValue("%u",decision_levels[id_existing].side);
  TraceValue("%u",decision_levels[id_existing].walk);
  TraceWalk(get_walk_of_piece_on_square(motivation[id_existing].last.on));
  TraceValue("%u",GetPieceId(being_solved.spec[motivation[id_existing].last.on]));
  TraceEOL();

  assert(motivation[id_existing].first.purpose!=purpose_none);
  assert(motivation[id_existing].last.purpose!=purpose_none);

  push_decision_departure(id_inserted,sq_departure,decision_purpose_invisible_capturer_existing);

  if (motivation[id_existing].last.acts_when<nbply
      || ((motivation[id_existing].last.purpose==purpose_interceptor
           || motivation[id_existing].last.purpose==purpose_capturer
           || motivation[id_existing].last.purpose==purpose_random_mover) /* if invoked by fake_capture_by_invisible () */
          && motivation[id_existing].last.acts_when<=nbply))
  {
    int const move_square_diff = sq_arrival-sq_departure;

    motivation[id_existing].last.purpose = purpose_none;

    switch (walk_existing)
    {
      case King:
        if (CheckDir(Queen)[move_square_diff]==move_square_diff)
        {
          capture_by_invisible_king(sq_departure);
          result = true;
        }
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
        int const dir = CheckDir(walk_existing)[move_square_diff];
        if (dir!=0 && sq_departure==find_end_of_line(sq_arrival,-dir))
        {
          capture_by_invisible_with_defined_walk(walk_existing,sq_departure);
          result = true;
        }
        else
        {
          record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
          REPORT_DEADEND;
        }
        break;
      }

      case Knight:
        if (CheckDir(Knight)[move_square_diff]==move_square_diff)
        {
          capture_by_invisible_with_defined_walk(Knight,sq_departure);
          result = true;
        }
        else
        {
          record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
          REPORT_DEADEND;
        }
        break;

      case Pawn:
      {
        Side const side_playing = trait[nbply];

        if ((side_playing==White ? move_square_diff>0 : move_square_diff<0)
            && CheckDir(Bishop)[move_square_diff]==move_square_diff)
        {
          SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
          SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;

          if (!TSTFLAG(sq_spec(sq_departure),basesq)
              && !TSTFLAG(sq_spec(sq_departure),promsq))
          {
            square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
            if (ForwardPromSq(side_playing,sq_arrival))
            {
              move_effect_journal_index_type const promotion = movement+1;
              pieces_pawns_promotion_sequence_type sequence = {
                  pieces_pawns_promotee_chain_orthodox,
                  pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][Empty]
              };

              assert(move_effect_journal[promotion].type==move_effect_none);

              move_effect_journal[promotion].type = move_effect_walk_change;
              move_effect_journal[promotion].u.piece_walk_change.from = Pawn;
              move_effect_journal[promotion].u.piece_walk_change.on = sq_arrival;

              do
              {
                push_decision_walk(id_existing,sequence.promotee,decision_purpose_invisible_capturer_existing,side_playing);
                move_effect_journal[promotion].u.piece_walk_change.to = sequence.promotee;
                capture_by_invisible_with_defined_walk(Pawn,sq_departure);
                pieces_pawns_continue_promotee_sequence(&sequence);
                pop_decision();
              } while (sequence.promotee!=Empty && can_decision_level_be_continued());

              move_effect_journal[promotion].type = move_effect_none;
            }
            else
              capture_by_invisible_with_defined_walk(Pawn,sq_departure);
            // TODO en passant capture
            result = true;
          }
        }
        else
        {
          record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
          REPORT_DEADEND;
        }
        break;
      }

      case Dummy:
        if (CheckDir(Queen)[move_square_diff]!=0
            || CheckDir(Knight)[move_square_diff]==move_square_diff)
        {
          if (current_consumption.is_king_unplaced[trait[nbply]])
            flesh_out_dummy_for_capture_king_or_non_king(sq_departure,sq_arrival,id_existing);
          else
            flesh_out_dummy_for_capture_non_king(sq_departure,sq_arrival,id_existing);
          result = true;
        }
        else
        {
          record_decision_outcome("%s","the piece on the departure square can't reach the arrival square");
          REPORT_DEADEND;
          if (static_consumption.king[advers(trait[nbply])]+static_consumption.pawn_victims[advers(trait[nbply])]+1
              >=total_invisible_number)
          {
            /* move our single piece to a different square
             * or let another piece be our single piece */
            /* e.g.
begin
author Ken Kousaka
origin Sake tourney 2018, announcement
pieces TotalInvisible 1 white kb8 qh1 black ka1 sb1e7
stipulation h#2
option movenum start 3:0:5:1
end


           Ken Kousaka
 Sake tourney 2018, announcement

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   K   .   .   .   .   .   .   8
|                                   |
7   .   .   .   .  -S   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   .   .   .   .   .   .   .   5
|                                   |
4   .   .   .   .   .   .   .   .   4
|                                   |
3   .   .   .   .   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1  -K  -S   .   .   .   .   .   Q   1
|                                   |
+---a---b---c---d---e---f---g---h---+
h#2                  2 + 3 + 1 TI

3  (Ka1-b2    Time = 0.048 s)

!validate_mate 6:Ka1-b2 7:TI~-~ 8:Kb2-c1 9:TI~-e7 - total_invisible.c:#521 - D:31 - 26
use option start 3:0:5:1 to replay
!  2 > 7 TI~-~ (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - random_move_by_invisible.c:#576 - D:32
!   3 + 9 I (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#105 - D:34
!    4 + 9 w (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#107 - D:36
!     5 + 9 d1 (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#109 - D:38
!      6 x 9 d1 (K:0+0 x:0+0 !:0+0 ?:1+0 F:0+0) - capture_by_invisible.c:#808 - D:40
!       7 9 the piece on the departure square can't reach the arrival square - capture_by_invisible.c:#891

HERE

!   3 + 9 I (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#105 - D:42
!    4 + 9 b (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#107 - D:44
!     5 + 9 d1 (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#109 - D:46
!      6 9 not enough available invisibles of side Black for intercepting all illegal checks - intercept_illegal_checks.c:#143
!   3 + 9 I (K:0+0 x:0+0 !:1+0 ?:0+0 F:0+0) - intercept_illegal_checks.c:#105 - D:48
...
             */
          }
        }
        break;

      default:
        assert(0);
        break;
    }
  }
  else
  {
    TraceText("the piece was added to later act from its current square\n");
    record_decision_outcome("%s","the piece was added to later act from its current square");
    REPORT_DEADEND;

      /*
begin
author Kjell Widlert
origin Sake tourney 2018, 5th HM
pieces TotalInvisible 4 white kh8 qh5 bf5 sb5d3 pb3 black kd5 qe4
stipulation h#2
option movenum start 1:2:0:3
end

          Kjell Widlert
    Sake tourney 2018, 5th HM

+---a---b---c---d---e---f---g---h---+
|                                   |
8   .   .   .   .   .   .   .   K   8
|                                   |
7   .   .   .   .   .   .   .   .   7
|                                   |
6   .   .   .   .   .   .   .   .   6
|                                   |
5   .   S   .  -K   .   B   .   Q   5
|                                   |
4   .   .   .   .  -Q   .   .   .   4
|                                   |
3   .   P   .   S   .   .   .   .   3
|                                   |
2   .   .   .   .   .   .   .   .   2
|                                   |
1   .   .   .   .   .   .   .   .   1
|                                   |
+---a---b---c---d---e---f---g---h---+
h#2                  6 + 2 + 4 TI

a)

!validate_mate 6:TI~-~ 7:TI~-e4 8:TI~-~ 9:Pb3-c4 - total_invisible.c:#521 - D:67 - 42
use option start 1:2:0:3 to replay
!  2 > 6 TI~-~ (K:0+0 x:0+1 !:0+1 ?:0+0 F:0+0) - random_move_by_invisible.c:#576 - D:68
!   3 X 7 I (K:0+0 x:0+1 !:0+1 ?:0+0 F:0+0) - capture_by_invisible.c:#915 - D:70
!    4 X 7 P (K:0+0 x:0+1 !:0+1 ?:0+0 F:0+0) - capture_by_invisible.c:#467 - D:72
!     5 X 7 f3 (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+0) - capture_by_invisible.c:#49 - D:74
!      6 < 6 TI~-~ (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+0) - random_move_by_invisible.c:#1026 - D:76
!       7 > 6 TI~-~ (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+0) - random_move_by_invisible.c:#576 - D:78
!        8 > 8 TI~-~ (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+0) - random_move_by_invisible.c:#576 - D:80
!         9 9 adding victim of capture by pawn - total_invisible.c:#374
!         9 < 8 c4 (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#990 - D:82
!          10 < 8 P (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#893 - D:84
...
!          10 < 8 B (K:0+0 x:0+1 !:0+0 ?:0+0 F:1+1) - random_move_by_invisible.c:#893 - D:15222
!         9 < 8 TI~-~ (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#1026 - D:15224
!          10 < 6 c4 (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#990 - D:15226
!           11 < 6 P (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#893 - D:15228
!            12 < 6 c5 (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#620 - D:15230
!             13 > 8 TI~-~ (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#576 - D:15232
!              14 9 uninterceptable illegal check from dir:-23 by id:9 delivered in ply:7 - intercept_illegal_checks.c:#697
!              14 x 8 c4 (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - capture_by_invisible.c:#758 - D:15234
!               15 8 the piece was added to later act from its current square - capture_by_invisible.c:#867

HERE

!           11 < 6 Q (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#893 - D:15236
!            12 < 6 b4 (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#620 - D:15238
!             13 > 8 TI~-~ (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - random_move_by_invisible.c:#576 - D:15240
!              14 9 uninterceptable illegal check from dir:-23 by id:9 delivered in ply:7 - intercept_illegal_checks.c:#697
!              14 x 8 c4 (K:0+0 x:0+1 !:0+1 ?:0+0 F:1+1) - capture_by_invisible.c:#758 - D:15242
!               15 8 the piece was added to later act from its current square - capture_by_invisible.c:#867
       */
  }

  motivation[id_existing] = motivation_existing;

  pop_decision();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_viable_capturer(PieceIdType id)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",id);
  TraceFunctionParamListEnd();

  TraceAction(&motivation[id].first);TraceEOL();
  TraceAction(&motivation[id].last);TraceEOL();
  TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
  TraceEOL();

  if (motivation[id].last.acts_when<=nbply && motivation[id].last.purpose==purpose_none)
  {
    TraceText("piece was captured or merged into a capturer from regular play\n");
    result = false;
  }
  else if (motivation[id].last.acts_when==nbply
           && motivation[id].last.purpose!=purpose_interceptor)
  {
    TraceText("piece is active for a different purpose\n");
    result = false;
  }
  else if (motivation[id].last.acts_when>nbply)
  {
    TraceText("piece will be active after the capture\n");
    result = false;
  }
  else if (id!=GetPieceId(being_solved.spec[motivation[id].last.on]))
  {
    TraceText("piece on square has different id\n");
    result = false;
  }
  else if (motivation[id].first.on==initsquare)
  {
    TraceText("revealed piece - to be replaced by an 'actual' piece\n");
    result = false;
  }
  else if (!TSTFLAG(being_solved.spec[motivation[id].last.on],trait[nbply]))
  {
    TraceText("candidate belongs to wrong side\n");
    result = false;
  }
  else if (!TSTFLAG(being_solved.spec[motivation[id].last.on],Chameleon))
  {
    TraceText("candidate has been revealed\n");
    result = false;
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/*
 * @return number of existing invisibles that can geometrically reach the capture square
 */
static unsigned int capture_by_existing_invisible(void)
{
  PieceIdType id;
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (id = get_top_visible_piece_id()+1;
       id<=get_top_invisible_piece_id() && can_decision_level_be_continued();
       ++id)
    if (is_viable_capturer(id)
        && capture_by_existing_invisible_on(motivation[id].last.on))
      ++result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean insert_capturing_king(Side side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  /* this helps us over the allocation */
  being_solved.king_square[side] = square_a1;

  result = allocate_flesh_out_unplaced(side);

  being_solved.king_square[side] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int capture_by_inserted_invisible(void)
{
  dynamic_consumption_type const save_consumption = current_consumption;

  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const precapture = effects_base;
  Flags const flags_inserted = move_effect_journal[precapture].u.piece_addition.added.flags;
  PieceIdType const id_inserted = GetPieceId(flags_inserted);
  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceConsumption();

  if (allocate_flesh_out_unplaced(trait[nbply]))
  {
    /* no problem - we can simply insert a capturer */

    push_decision_insertion(id_inserted,trait[nbply],decision_purpose_invisible_capturer_inserted);

    result = capture_by_inserted_invisible_all_walks();

    pop_decision();

    current_consumption = save_consumption;
  }
  else
  {
    TraceText("we can't just insert a capturer\n");

    current_consumption = save_consumption;

    TraceEnumerator(Side,trait[nbply]);
    TraceSquare(being_solved.king_square[trait[nbply]]);
    TraceEOL();
    if (current_consumption.is_king_unplaced[trait[nbply]])
    {
      assert(being_solved.king_square[trait[nbply]]==initsquare);

      if (insert_capturing_king(trait[nbply]))
      {
        move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
        square const save_from = move_effect_journal[movement].u.piece_movement.from;
        piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
        Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;

        assert(move_effect_journal[movement].type==move_effect_piece_movement);

        push_decision_insertion(id_inserted,trait[nbply],decision_purpose_invisible_capturer_inserted);

        capture_by_inserted_invisible_king();

        move_effect_journal[movement].u.piece_movement.from = save_from;
        move_effect_journal[movement].u.piece_movement.moving = save_moving;
        move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;

        pop_decision();

        result = 1;
      }
      else
      {
        TraceText("the king has already been placed implicitly (e.g. while intercepting a check)\n");
      }

      current_consumption = save_consumption;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

square need_existing_invisible_as_victim_for_capture_by_pawn(ply ply_capture)
{
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_capture);
  TraceFunctionParamListEnd();

  if (ply_capture<=top_ply_of_regular_play)
  {
    move_effect_journal_index_type const effects_base_capture = move_effect_journal_base[ply_capture];

    move_effect_journal_index_type const capture_capture = effects_base_capture+move_effect_journal_index_offset_capture;
    square const sq_capture_capture = move_effect_journal[capture_capture].u.piece_removal.on;

    move_effect_journal_index_type const movement_capture = effects_base_capture+move_effect_journal_index_offset_movement;
    piece_walk_type const capturer = move_effect_journal[movement_capture].u.piece_movement.moving;
    Flags const capturer_flags = move_effect_journal[movement_capture].u.piece_movement.movingspec;

    TraceValue("%u",ply_capture);
    TraceSquare(sq_capture_capture);
    TraceWalk(capturer);
    TraceEOL();

    if (move_effect_journal[capture_capture].type==move_effect_piece_removal
        && capturer==Pawn && !TSTFLAG(capturer_flags,Chameleon)
        && (is_square_empty(sq_capture_capture)
            || TSTFLAG(being_solved.spec[sq_capture_capture],advers(trait[nbply]))))
    {
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
        square const sq_arrival_now = move_effect_journal[movement_now].u.piece_movement.to;

        TraceText("allocation of a victim in the next move impossible - test possibility of sacrifice\n");

        if (sq_arrival_now==sq_capture_capture)
        {
          TraceText("this move sacrifices a piece (visible or invisible)\n");
        }
        else
        {
          TraceText("no sacrifice in this move\n");
          result = sq_capture_capture;
        }
      }

      current_consumption = save_consumption;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

void flesh_out_capture_by_invisible(void)
{
  ply const ply_capture_by_pawn = nbply+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (need_existing_invisible_as_victim_for_capture_by_pawn(ply_capture_by_pawn)==initsquare)
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

    move_effect_journal_index_type const precapture = effects_base;
    move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
    piece_walk_type const save_removed_walk = move_effect_journal[capture].u.piece_removal.walk;
    Flags const save_removed_spec = move_effect_journal[capture].u.piece_removal.flags;
    square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
    Flags const flags = move_effect_journal[precapture].u.piece_addition.added.flags;
    PieceIdType const id_inserted = GetPieceId(flags);
    decision_levels_type const save_levels = decision_levels[id_inserted];

    unsigned int nr_attempts;

    TraceSquare(sq_capture);TraceEOL();
    assert(!is_square_empty(sq_capture));

    assert(decision_levels[id_inserted].side==decision_level_forever);
    assert(decision_levels[id_inserted].to==decision_level_forever);

    move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(sq_capture);
    move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[sq_capture];

    nr_attempts = capture_by_existing_invisible();

    if (nr_attempts==0 || can_decision_level_be_continued())
      nr_attempts += capture_by_inserted_invisible();

    move_effect_journal[capture].u.piece_removal.walk = save_removed_walk;
    move_effect_journal[capture].u.piece_removal.flags = save_removed_spec;

    if (nr_attempts==0)
    {
      record_decision_outcome("%s","no invisible piece found that could capture");
      REPORT_DEADEND;
      backtrack_from_failed_capture_by_invisible(trait[nbply]);
    }

    decision_levels[id_inserted] = save_levels;
  }
  else
  {
    record_decision_outcome("capture in ply %u will not be possible",ply_capture_by_pawn);
    REPORT_DEADEND;
    backtrack_from_failed_capture_of_invisible_by_pawn(trait[ply_capture_by_pawn]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
