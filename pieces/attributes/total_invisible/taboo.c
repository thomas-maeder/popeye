#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "solving/move_effect_journal.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

unsigned int nr_taboos_accumulated_until_ply[nr_sides][maxsquare];

unsigned int nr_taboos_for_current_move_in_ply[maxply+1][nr_sides][maxsquare];

boolean was_taboo(square s, Side side)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  for (ply = nbply-1; ply>ply_retro_move; --ply)
  {
    TraceValue("%u",ply);
    TraceValue("%u",nr_taboos_for_current_move_in_ply[ply][side][s]);
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

boolean will_be_taboo(square s, Side side)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if (side!=trait[nbply]
      && s==move_effect_journal[move_effect_journal_base[nbply]+move_effect_journal_index_offset_movement].u.piece_movement.to)
  {
    TraceText("captured pieces don't violate taboos\n");
  }
  else
    // TODO we should be able to start at ply nbply and get rid of the ugly if above
    for (ply = nbply+1; ply<=top_ply_of_regular_play; ++ply)
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
      else
      {
        move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];
        move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

        TraceSquare(move_effect_journal[movement].u.piece_movement.from);
        TraceSquare(move_effect_journal[movement].u.piece_movement.to);
        TraceEOL();

        if (side==trait[ply])
        {
          if (move_effect_journal[movement].u.piece_movement.from==move_by_invisible)
            break;
          else if (move_effect_journal[movement].u.piece_movement.from==capture_by_invisible)
          {
            int const square_diff = s-move_effect_journal[movement].u.piece_movement.to;
            if (CheckDir[Queen][square_diff]!=0 || CheckDir[Knight][square_diff]!=0)
              break;
          }
        }
        else if (s==move_effect_journal[movement].u.piece_movement.to)
        {
          TraceText("captured pieces don't violate taboos\n");
        }
        else
        {
          if (is_taboo_candidate_captured(ply,s))
            break;
        }
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

    ADJUST_TABOO(nr_taboos_for_current_move_in_ply[nbply+1][White][sq_departure],delta);
    ADJUST_TABOO(nr_taboos_for_current_move_in_ply[nbply+1][Black][sq_departure],delta);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square find_taboo_violation_rider(move_effect_journal_index_type movement,
                                         piece_walk_type walk)
{
  square result = nullsquare;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  int const diff_move = sq_arrival - sq_departure;
  int const dir_move = CheckDir[walk][diff_move];

  square s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",movement);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  TraceSquare(sq_arrival);
  TraceWalk(get_walk_of_piece_on_square(sq_arrival));
  TraceValue("%x",being_solved.spec[sq_arrival]);
  TraceEOL();

  if (!is_square_empty(sq_arrival)
      && !TSTFLAG(being_solved.spec[sq_arrival],advers(trait[nbply])))
  {
    TraceText("arrival square blocked\n");
    result = sq_arrival;
  }
  else
  {
    assert(dir_move!=0);
    for (s = sq_departure+dir_move; s!=sq_arrival; s += dir_move)
      if (!is_square_empty (s))
      {
        TraceText("movement is intercepted\n");
        result = s;
        break;
      }
  }


  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static square find_taboo_violation_leaper(move_effect_journal_index_type movement)
{
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  square result = nullsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",movement);
  TraceFunctionParamListEnd();

  TraceSquare(sq_arrival);
  TraceWalk(get_walk_of_piece_on_square(sq_arrival));
  TraceValue("%x",being_solved.spec[sq_arrival]);
  TraceEOL();
  if (!is_square_empty(sq_arrival)
      && !TSTFLAG(being_solved.spec[sq_arrival],advers(trait[nbply])))
  {
    TraceText("arrival square blocked\n");
    result = sq_arrival;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static square find_taboo_violation_pawn(move_effect_journal_index_type capture,
                                        move_effect_journal_index_type movement)
{
  square result = nullsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",capture);
  TraceFunctionParam("%u",movement);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    Flags const spec = move_effect_journal[movement].u.piece_movement.movingspec;
    int const dir_move = TSTFLAG(spec,White) ? dir_up : dir_down;
    square s = sq_departure;

    do
    {
      s += dir_move;
      if (!is_square_empty(s))
      {
        TraceText("non-capturing move blocked\n");
        result = s;
        break;
      }
    }
    while (s!=sq_arrival);
  }
  else
    result = find_taboo_violation_leaper(movement);

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

square find_taboo_violation(void)
{
  square result = nullsquare;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (curr = base; curr!=top && result==nullsquare; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
    {
      move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
      move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
      piece_walk_type const walk = move_effect_journal[movement].u.piece_movement.moving;
      square const sq_departure = move_effect_journal[movement].u.piece_movement.from;

      assert(sq_departure!=move_by_invisible);
      assert(sq_departure<capture_by_invisible);

      if (is_rider(walk))
        result = find_taboo_violation_rider(movement,walk);
      else if (walk==King)
      {
        if (move_effect_journal[curr].reason==move_effect_reason_castling_king_movement)
          /* faking a rook movement by the king */
          result = find_taboo_violation_rider(movement,Rook);
        else
          result = find_taboo_violation_leaper(movement);
      }
      else if (is_leaper(walk))
        result = find_taboo_violation_leaper(movement);
      else if (is_pawn(walk))
        result = find_taboo_violation_pawn(capture,movement);
      else
        assert(0);
    }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_taboo_violation_acceptable(square first_taboo_violation)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

    PieceIdType const id = GetPieceId(being_solved.spec[first_taboo_violation]);
    TraceValue("%x",being_solved.spec[first_taboo_violation]);
    TraceValue("%u",id);
    TraceValue("%u",motivation[id].last.acts_when);
    TraceSquare(motivation[id].last.on);
    TraceValue("%u",motivation[id].last.purpose);
    TraceEOL();

    if (motivation[id].last.acts_when<nbply
        && motivation[id].last.purpose==purpose_interceptor)
      /* 6:f4-e5 7:~-~ 8:~-~ 9:b2-d4 - 6:Kf4-e5 7:.~-~ 8:.~-~ 9:Qb2-d4
       * 1. an interceptor of side s was placed as interceptor on square sq
       * 2. advers(s) made a random move that might have accidentally captured on sq but didn't
       * 3. Side s moves to sq and is blocked by 1
       */
      // TODO is item 2 relevant for this case? do we miss it even if there is
      // no such random move? I.e. should we tigthen the if()?
    {
//      if (move_generation_stack[CURRMOVE_OF_PLY(nbply-1)].departure==move_by_invisible
//          && move_generation_stack[CURRMOVE_OF_PLY(nbply-1)].arrival==move_by_invisible)
      REPORT_DECISION_OUTCOME("%s","move is blocked by interceptor");
      REPORT_DEADEND;
      result = true;
    }

    if (motivation[id].last.acts_when<nbply
        && motivation[id].last.purpose==purpose_random_mover
        && motivation[id].last.on!=first_taboo_violation
        && move_effect_journal[movement].u.piece_movement.to!=first_taboo_violation)
      /* 6:.~-~(iPg7-g6) 7:.~-~(iSd5-b6) 8:.~-~(iRf3-b3) 9:Rh6-f6
       * 1. an invisible piece of side s was placed
       * 2. a random move of s placed it on square sq
       * 3. s made a random move that could have left sq but didn't
       * 4. the current move is intercepted on sq
       */
      // TODO is item 3 relevant for this case? do we miss it even if there is
      // no such random move? I.e. should we tighten the if()?
    {
      REPORT_DECISION_OUTCOME("%s","move is intercepted by interceptor");
      REPORT_DEADEND;
      result = true;
    }

    if (motivation[id].last.acts_when<nbply
        && motivation[id].last.purpose==purpose_random_mover
        && motivation[id].last.on!=first_taboo_violation
        && move_effect_journal[movement].u.piece_movement.to==first_taboo_violation
        && move_effect_journal[movement].u.piece_movement.moving!=Pawn)
    /* 6:~-~(Bf3-e4) 7:~-~(Pe3-d4) 8:~-b7(Be4-b7) 9:e5-d4(Ke5-d4)
     * 1. an invisible piece of side s was placed
     * 2. a random move of side s placed it on square sq
     * 3. advers(s) dit *not* make a move that could have captured on sq
     * 4. the current pawn move is blocked on sq
     */
    {
      REPORT_DECISION_OUTCOME("%s","pawn move is blocked by invisible piece");
      REPORT_DEADEND;
      result = true;
    }

    if (motivation[id].last.acts_when<nbply
        && motivation[id].last.purpose==purpose_random_mover
        && motivation[id].last.on!=first_taboo_violation
        && move_effect_journal[movement].u.piece_movement.to==first_taboo_violation
        && move_effect_journal[movement].u.piece_movement.moving==Pawn)
      /* 6:.~-~(iRd4-c4) 7:.~-~(iRe4-d4) 8:.~-~(iPe3-e2) 9:c2-c4
       * 1. an invisible piece of side s was placed
       * 2. a random move of side s placed it on square sq
       * 3. s made a random move that could have left sq but didn't
       * 4. the current pawn move is blocked on sq
       */
      // TODO is item 3 relevant for this case? do we miss it even if there is
      // no such random move? I.e. should we tigthen the if()?
    {
      REPORT_DECISION_OUTCOME("%s","pawn move is blocked by invisible piece");
      REPORT_DEADEND;
      result = true;
    }

    if (motivation[id].last.acts_when<nbply
        && motivation[id].last.purpose==purpose_random_mover
        && motivation[id].last.on==first_taboo_violation)
      /* 6:Bh8-f6 7:~-~(Sd5-b6) 8:Bf6-h4  first_taboo_violation:g5
       * 1. an invisible piece of side s was placed on square sq
       * 2. s made a random move that could have left sq but didn't
       * 3. the current move is intercepted on sq
       * 4. the revelation would only happen after the current move
       */
    {
      REPORT_DECISION_OUTCOME("%s","move is intercepted by invisible piece");
      REPORT_DEADEND;
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
