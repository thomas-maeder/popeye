#include "pieces/attributes/total_invisible/attack_mating_piece.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/replay_fleshed_out.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible.h"
#include "solving/has_solution_type.h"
#include "position/position.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

square sq_mating_piece_to_be_attacked = initsquare;

static void place_mating_piece_attacker(Side side_attacking,
                                        square s,
                                        PieceIdType id_placed,
                                        piece_walk_type walk)
{
  dynamic_consumption_type const save_consumption = current_consumption;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(s);
  TraceValue("%u",id_placed);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  /* only now that we have decided not to use an accidental attack! */
  if (allocate_flesh_out_unplaced(side_attacking))
  {
    if (!was_taboo(s,side_attacking))
    {
      Flags spec = BIT(side_attacking)|BIT(Chameleon);

      motivation[id_placed].first.on = s;
      motivation[id_placed].last.on = s;

      push_decision_insertion(id_placed,side_attacking,decision_purpose_mating_piece_attacker);

      push_decision_departure(id_placed,s,decision_purpose_mating_piece_attacker);

      ++being_solved.number_of_pieces[side_attacking][walk];
      SetPieceId(spec,id_placed);
      occupy_square(s,walk,spec);
      restart_from_scratch();
      empty_square(s);
      --being_solved.number_of_pieces[side_attacking][walk];

      pop_decision();

      pop_decision();
    }
  }

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

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

  record_decision_result(solve_result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void use_accidental_attack_on_mating_piece(square s)
{
  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  record_decision_outcome("%s","use accidental attack on mating piece");
  done_placing_mating_piece_attacker();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_rider(Side side_attacking,
                                               square sq_mating_piece,
                                               piece_walk_type walk_rider,
                                               vec_index_type kcurr, vec_index_type kend,
                                               PieceIdType id_placed)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParam("%u",id_placed);
  TraceFunctionParamListEnd();

  push_decision_walk(id_placed,walk_rider,decision_purpose_mating_piece_attacker,side_attacking);

  for (; kcurr<=kend && can_decision_level_be_continued(); ++kcurr)
  {
    square s;
    for (s = sq_mating_piece+vec[kcurr]; can_decision_level_be_continued(); s += vec[kcurr])
    {
      if (is_square_empty(s))
        place_mating_piece_attacker(side_attacking,s,id_placed,walk_rider);
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

  pop_decision();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_leaper(Side side_attacking,
                                                square sq_mating_piece,
                                                piece_walk_type walk_leaper,
                                                vec_index_type kcurr, vec_index_type kend,
                                                PieceIdType id_placed)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParam("%u",id_placed);
  TraceFunctionParamListEnd();

  push_decision_walk(id_placed,walk_leaper,decision_purpose_mating_piece_attacker,side_attacking);

  for (; kcurr<=kend && can_decision_level_be_continued(); ++kcurr)
  {
    square const s = sq_mating_piece+vec[kcurr];

    TraceSquare(s);TraceEOL();

    if (get_walk_of_piece_on_square(s)==walk_leaper
        && TSTFLAG(being_solved.spec[s],side_attacking))
      use_accidental_attack_on_mating_piece(s);
    else if (is_square_empty(s))
      place_mating_piece_attacker(side_attacking,s,id_placed,walk_leaper);
  }

  pop_decision();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_pawn(Side side_attacking,
                                              square sq_mating_piece,
                                              PieceIdType id_placed)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceFunctionParam("%u",id_placed);
  TraceFunctionParamListEnd();

  push_decision_walk(id_placed,Pawn,decision_purpose_mating_piece_attacker,side_attacking);

  if (can_decision_level_be_continued())
  {
    square s = sq_mating_piece+dir_up+dir_left;

    TraceSquare(s);TraceEOL();

    if (get_walk_of_piece_on_square(s)==Pawn
        && TSTFLAG(being_solved.spec[s],side_attacking))
      use_accidental_attack_on_mating_piece(s);
    else if (is_square_empty(s))
      place_mating_piece_attacker(side_attacking,s,id_placed,Pawn);
  }

  if (can_decision_level_be_continued())
  {
    square s = sq_mating_piece+dir_up+dir_right;

    TraceSquare(s);TraceEOL();

    if (get_walk_of_piece_on_square(s)==Pawn
        && TSTFLAG(being_solved.spec[s],side_attacking))
      use_accidental_attack_on_mating_piece(s);
    else if (is_square_empty(s))
      place_mating_piece_attacker(side_attacking,s,id_placed,Pawn);
  }

  pop_decision();

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

  {
    PieceIdType const id_placed = initialise_motivation(purpose_attacker,initsquare,
                                                        purpose_attacker,initsquare);

    place_mating_piece_attacking_rider(side_attacking,
                                       sq_mating_piece,
                                       Bishop,
                                       vec_bishop_start,vec_bishop_end,
                                       id_placed);

    if (can_decision_level_be_continued())
      place_mating_piece_attacking_rider(side_attacking,
                                         sq_mating_piece,
                                         Rook,
                                         vec_rook_start,vec_rook_end,
                                         id_placed);

    if (can_decision_level_be_continued())
      place_mating_piece_attacking_leaper(side_attacking,
                                          sq_mating_piece,
                                          Knight,
                                          vec_knight_start,vec_knight_end,
                                          id_placed);

    if (can_decision_level_be_continued())
      place_mating_piece_attacking_pawn(side_attacking,sq_mating_piece,id_placed);

    uninitialise_motivation(id_placed);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
