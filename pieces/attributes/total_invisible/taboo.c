#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "solving/move_effect_journal.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

unsigned int nr_taboos_accumulated_until_ply[nr_sides][maxsquare];

unsigned int nr_taboos_for_current_move_in_ply[maxply+1][nr_sides][maxsquare];

boolean has_been_taboo_since_random_move(square s)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  for (ply = nbply-1; ply>ply_retro_move; --ply)
  {
    TraceValue("%u",ply);
    TraceValue("%u",nr_taboos_for_current_move_in_ply[ply][White][s]);
    TraceValue("%u",nr_taboos_for_current_move_in_ply[ply][Black][s]);
    TraceEOL();
    if (nr_taboos_for_current_move_in_ply[ply][White][s]>0
        || nr_taboos_for_current_move_in_ply[ply][Black][s]>0)
    {
      result = true;
      break;
    }
    else if (is_random_move_by_invisible(ply))
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_taboo_candidate_captured(ply ply, square s)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceValue("%u",ply);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if (sq_arrival==s)
    /* accidental capture */
    result = true;
  else if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
    if (sq_capture==s)
      /* planned capture */
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_taboo(square s, Side side)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  for (ply = nbply; ply<=top_ply_of_regular_play; ++ply)
  {
    TraceValue("%u",ply);
    TraceValue("%u",nr_taboos_for_current_move_in_ply[ply][side][s]);
    TraceEnumerator(Side,trait[ply]);
    TraceEOL();

    if (nr_taboos_for_current_move_in_ply[ply][side][s]>0)
    {
      result = true;
      break;
    }
    else if (side==trait[ply])
    {
      move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];
      move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

      TraceSquare(move_effect_journal[movement].u.piece_movement.from);TraceEOL();

      if (move_effect_journal[movement].u.piece_movement.from==move_by_invisible)
        break;
      else if (move_effect_journal[movement].u.piece_movement.from==capture_by_invisible)
      {
        int const square_diff = s-move_effect_journal[movement].u.piece_movement.to;
        if (CheckDir[Queen][square_diff]!=0 || CheckDir[Knight][square_diff]!=0)
          break;
      }
    }
    else
    {
      if (is_taboo_candidate_captured(ply,s))
        break;
    }
  }
  // TODO what if two taboos have to be lifted in the same ply?
  // TODO what about captures by invisible?

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef unsigned int (*taboo_type)[nr_sides][maxsquare];

#if defined(NDEBUG)
#define ADJUST_TABOO(TABOO,DELTA) \
  (TABOO) += (DELTA);
#else
#define ADJUST_TABOO(TABOO,DELTA) \
  assert(((DELTA)>0) || ((TABOO)>0)), \
  (TABOO) += (DELTA);
#endif

static void update_taboo_piece_movement_rider(int delta,
                                              move_effect_journal_index_type const movement,
                                              taboo_type taboo)
{
  piece_walk_type const walk = move_effect_journal[movement].u.piece_movement.moving;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  int const diff_move = sq_arrival-sq_departure;
  int const dir_move = CheckDir[walk][diff_move];
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,trait[nbply]);
  TraceEnumerator(Side,advers(trait[nbply]));
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceEOL();

  ADJUST_TABOO((*taboo)[advers(trait[nbply])][sq_departure],delta);
  TraceValue("%u",(*taboo)[advers(trait[nbply])][sq_departure]);TraceEOL();

  assert(dir_move!=0);
  for (s = sq_departure+dir_move; s!=sq_arrival; s += dir_move)
  {
    TraceSquare(s);
    ADJUST_TABOO((*taboo)[White][s],delta);
    TraceValue("%u",(*taboo)[White][s]);
    ADJUST_TABOO((*taboo)[Black][s],delta);
    TraceValue("%u",(*taboo)[Black][s]);
    TraceEOL();
  }

  ADJUST_TABOO((*taboo)[trait[nbply]][sq_arrival],delta);
  TraceValue("%u",(*taboo)[trait[nbply]][sq_arrival]);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo_piece_movement_leaper(int delta,
                                               move_effect_journal_index_type const movement,
                                               taboo_type taboo)
{
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  ADJUST_TABOO((*taboo)[advers(trait[nbply])][sq_departure],delta);
  ADJUST_TABOO((*taboo)[trait[nbply]][sq_arrival],delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo_piece_movement_pawn_no_capture(int delta,
                                                        move_effect_journal_index_type const movement,
                                                        taboo_type taboo)
{
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  int const dir_move = trait[nbply]==White ? dir_up : dir_down;
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  ADJUST_TABOO((*taboo)[advers(trait[nbply])][sq_departure],delta);

  for (s = sq_departure+dir_move; s!=sq_arrival; s += dir_move)
  {
    ADJUST_TABOO((*taboo)[White][s],delta);
    ADJUST_TABOO((*taboo)[Black][s],delta);
  }

  /* arrival square must not be blocked by either side */
  ADJUST_TABOO((*taboo)[White][sq_arrival],delta);
  ADJUST_TABOO((*taboo)[Black][sq_arrival],delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo_piece_movement_pawn_capture(int delta,
                                                     move_effect_journal_index_type const movement,
                                                     taboo_type taboo)
{
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  ADJUST_TABOO((*taboo)[advers(trait[nbply])][sq_departure],delta);
  ADJUST_TABOO((*taboo)[trait[nbply]][sq_arrival],delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo_piece_movement_castling(int delta,
                                                 move_effect_journal_index_type const movement,
                                                 taboo_type taboo)
{
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  int const diff_move = sq_arrival-sq_departure;
  int const dir_movement = CheckDir[Rook][diff_move];
  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,trait[nbply]);
  TraceSquare(sq_departure);
  ADJUST_TABOO((*taboo)[advers(trait[nbply])][sq_departure],delta);
  TraceValue("%u",(*taboo)[advers(trait[nbply])][sq_departure]);
  TraceEOL();

  for (s = sq_departure+dir_movement; s!=sq_arrival; s += dir_movement)
  {
    TraceSquare(s);
    ADJUST_TABOO((*taboo)[White][s],delta);
    TraceValue("%u",(*taboo)[White][s]);
    ADJUST_TABOO((*taboo)[Black][s],delta);
    TraceValue("%u",(*taboo)[Black][s]);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void update_nr_taboos_for_current_move_in_ply(int delta)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  piece_walk_type const walk = move_effect_journal[movement].u.piece_movement.moving;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceSquare(sq_departure);
  TraceWalk(walk);
  TraceEOL();

  if (sq_departure==move_by_invisible
      || sq_departure>=capture_by_invisible)
  {
    /* no taboos! */
  }
  else
  {
    assert(walk!=Empty);

    if (is_rider(walk))
      update_taboo_piece_movement_rider(delta,movement,&nr_taboos_for_current_move_in_ply[nbply]);
    else if (is_pawn(walk))
    {
      if (move_effect_journal[capture].type==move_effect_no_piece_removal)
        update_taboo_piece_movement_pawn_no_capture(delta,movement,&nr_taboos_for_current_move_in_ply[nbply]);
      else
        update_taboo_piece_movement_pawn_capture(delta,movement,&nr_taboos_for_current_move_in_ply[nbply]);
    }
    else
    {
      if (walk==King
          && move_effect_journal[movement].reason==move_effect_reason_castling_king_movement)
        update_taboo_piece_movement_castling(delta,movement,&nr_taboos_for_current_move_in_ply[nbply]);
      else
        update_taboo_piece_movement_leaper(delta,movement,&nr_taboos_for_current_move_in_ply[nbply]);
    }

    {
      move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
      move_effect_journal_index_type idx;

      for (idx = base+move_effect_journal_index_offset_other_effects; idx!=top; ++idx)
        if (move_effect_journal[idx].type==move_effect_piece_movement
            && move_effect_journal[idx].reason==move_effect_reason_castling_partner)
          update_taboo_piece_movement_castling(delta,idx,&nr_taboos_for_current_move_in_ply[nbply]);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void update_taboo(int delta)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  piece_walk_type const walk = move_effect_journal[movement].u.piece_movement.moving;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceWalk(walk);
  TraceEOL();

  if (sq_departure==move_by_invisible
      || sq_departure>=capture_by_invisible)
  {
    /* no taboos! */
  }
  else
  {
    if (is_rider(walk))
      update_taboo_piece_movement_rider(delta,movement,&nr_taboos_accumulated_until_ply);
    else if (is_pawn(walk))
    {
      if (move_effect_journal[capture].type==move_effect_no_piece_removal)
        update_taboo_piece_movement_pawn_no_capture(delta,movement,&nr_taboos_accumulated_until_ply);
      else
        update_taboo_piece_movement_pawn_capture(delta,movement,&nr_taboos_accumulated_until_ply);
    }
    else
    {
      if (walk==King
          && move_effect_journal[movement].reason==move_effect_reason_castling_king_movement)
        update_taboo_piece_movement_castling(delta,movement,&nr_taboos_accumulated_until_ply);
      else
        update_taboo_piece_movement_leaper(delta,movement,&nr_taboos_accumulated_until_ply);
    }

    {
      move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
      move_effect_journal_index_type idx;

      for (idx = base+move_effect_journal_index_offset_other_effects; idx!=top; ++idx)
        if (move_effect_journal[idx].type==move_effect_piece_movement
            && move_effect_journal[idx].reason==move_effect_reason_castling_partner)
          update_taboo_piece_movement_castling(delta,idx,&nr_taboos_accumulated_until_ply);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean was_taboo(square s)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  for (ply = ply_retro_move+1; ply<nbply; ++ply)
  {
    TraceValue("%u",ply);
    TraceValue("%u",nr_taboos_for_current_move_in_ply[ply][White][s]);
    TraceValue("%u",nr_taboos_for_current_move_in_ply[ply][Black][s]);
    TraceEOL();
    if (nr_taboos_for_current_move_in_ply[ply][White][s]>0 || nr_taboos_for_current_move_in_ply[ply][Black][s]>0)
    {
      result = true;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
