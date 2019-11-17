#include "pieces/attributes/total_invisible/attack_mating_piece.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/replay_fleshed_out.h"
#include "pieces/attributes/total_invisible.h"
#include "solving/has_solution_type.h"
#include "position/position.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

square sq_mating_piece_to_be_attacked = initsquare;

static void place_mating_piece_attacker(Side side_attacking,
                                        square s,
                                        piece_walk_type walk)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  if (!was_taboo(s,side_attacking))
  {
    dynamic_consumption_type const save_consumption = current_consumption;

    if (allocate_flesh_out_unplaced(side_attacking))
    {
      PieceIdType const id_placed = initialise_motivation(purpose_attacker,s,purpose_attacker,s);
      Flags spec = BIT(side_attacking)|BIT(Chameleon);

      REPORT_DECISION_SQUARE('>',s);
      ++curr_decision_level;

      ++being_solved.number_of_pieces[side_attacking][walk];
      SetPieceId(spec,id_placed);
      occupy_square(s,walk,spec);
      restart_from_scratch();
      empty_square(s);
      uninitialise_motivation(id_placed);
      --being_solved.number_of_pieces[side_attacking][walk];

      --curr_decision_level;
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void done_placing_mating_piece_attacker(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  play_phase = play_initialising_replay;
  replay_fleshed_out_move_sequence(play_replay_testing);
  play_phase = play_attacking_mating_piece;

  if (solve_result==previous_move_has_not_solved)
    max_decision_level = decision_level_forever;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void use_accidental_attack_on_mating_piece(square s)
{
  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  REPORT_DECISION_SQUARE('>',s);
  ++curr_decision_level;
  done_placing_mating_piece_attacker();
  --curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_rider(Side side_attacking,
                                               square sq_mating_piece,
                                               piece_walk_type walk_rider,
                                               vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && curr_decision_level<=max_decision_level; ++kcurr)
  {
    square s;
    for (s = sq_mating_piece+vec[kcurr]; curr_decision_level<=max_decision_level; s += vec[kcurr])
    {
      max_decision_level = decision_level_latest;
      if (is_square_empty(s))
      {
        TraceSquare(s);
        TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
        TraceEOL();

        if (nr_taboos_accumulated_until_ply[side_attacking][s]==0)
          place_mating_piece_attacker(side_attacking,s,walk_rider);
      }
      else
      {
        if ((get_walk_of_piece_on_square(s)==walk_rider
            || get_walk_of_piece_on_square(s)==Queen)
            && TSTFLAG(being_solved.spec[s],side_attacking))
          use_accidental_attack_on_mating_piece(s);

        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_leaper(Side side_attacking,
                                                square sq_mating_piece,
                                                piece_walk_type walk_leaper,
                                                vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && curr_decision_level<=max_decision_level; ++kcurr)
  {
    square const s = sq_mating_piece+vec[kcurr];

    TraceSquare(s);
    TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
    TraceEOL();

    max_decision_level = decision_level_latest;

    if (get_walk_of_piece_on_square(s)==walk_leaper
        && TSTFLAG(being_solved.spec[s],side_attacking))
      use_accidental_attack_on_mating_piece(s);
    else if (is_square_empty(s)
             && nr_taboos_accumulated_until_ply[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,walk_leaper);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_pawn(Side side_attacking,
                                              square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceFunctionParamListEnd();

  if (curr_decision_level<=max_decision_level)
  {
    square s = sq_mating_piece+dir_up+dir_left;

    TraceSquare(s);
    TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
    TraceEOL();

    max_decision_level = decision_level_latest;

    if (get_walk_of_piece_on_square(s)==Pawn
        && TSTFLAG(being_solved.spec[s],side_attacking))
      use_accidental_attack_on_mating_piece(s);
    else if (is_square_empty(s)
        && nr_taboos_accumulated_until_ply[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  if (curr_decision_level<=max_decision_level)
  {
    square s = sq_mating_piece+dir_up+dir_right;

    TraceSquare(s);
    TraceValue("%u",nr_taboos_accumulated_until_ply[side_attacking][s]);
    TraceEOL();

    max_decision_level = decision_level_latest;

    if (get_walk_of_piece_on_square(s)==Pawn
        && TSTFLAG(being_solved.spec[s],side_attacking))
      use_accidental_attack_on_mating_piece(s);
    else if (is_square_empty(s)
             && nr_taboos_accumulated_until_ply[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void attack_mating_piece(Side side_attacking,
                         square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(sq_mating_piece);
  TraceFunctionParamListEnd();

  REPORT_DECISION_WALK('>',walk_Bishop);
  ++curr_decision_level;
  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Bishop,
                                     vec_bishop_start,vec_bishop_end);
  --curr_decision_level;

  REPORT_DECISION_WALK('>',Rook);
  ++curr_decision_level;
  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Rook,
                                     vec_rook_start,vec_rook_end);
  --curr_decision_level;

  REPORT_DECISION_WALK('>',Knight);
  ++curr_decision_level;
  place_mating_piece_attacking_leaper(side_attacking,
                                      sq_mating_piece,
                                      Knight,
                                      vec_knight_start,vec_knight_end);
  --curr_decision_level;

  REPORT_DECISION_WALK('>',Rook);
  ++curr_decision_level;
  place_mating_piece_attacking_pawn(side_attacking,sq_mating_piece);
  --curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
