#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "position/effects/piece_readdition.h"
#include "position/effects/piece_movement.h"
#include "position/effects/null_move.h"
#include "position/effects/walk_change.h"
#include "position/effects/flags_change.h"
#include "position/effects/king_square.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/slice_insertion.h"
#include "solving/check.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/castling.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <string.h>

unsigned int total_invisible_number;

typedef struct
{
    unsigned int placed_fleshed_out[nr_sides];
    unsigned int placed_not_fleshed_out[nr_sides];
    boolean claimed[nr_sides];
} consumption_type;

static consumption_type current_consumption = { 0 };

static unsigned int nr_total_invisbles_consumed(void)
{
  unsigned int result = (current_consumption.placed_fleshed_out[White]
                         + current_consumption.placed_fleshed_out[Black]
                         + current_consumption.placed_not_fleshed_out[White]
                         + current_consumption.placed_not_fleshed_out[Black]
                         + current_consumption.claimed[White]
                         + current_consumption.claimed[Black]);

  if (!current_consumption.claimed[White]
      && current_consumption.placed_not_fleshed_out[White]==0
      && being_solved.king_square[White]==initsquare)
    ++result;

  if (!current_consumption.claimed[Black]
      && current_consumption.placed_not_fleshed_out[Black]==0
      && being_solved.king_square[Black]==initsquare)
    ++result;

  return result;
}

static void TraceConsumption(void)
{
  TraceValue("%u",current_consumption.placed_fleshed_out[White]);
  TraceValue("%u",current_consumption.placed_fleshed_out[Black]);
  TraceValue("%u",current_consumption.placed_not_fleshed_out[White]);
  TraceValue("%u",current_consumption.placed_not_fleshed_out[Black]);
  TraceValue("%u",current_consumption.claimed[White]);
  TraceValue("%u",current_consumption.claimed[Black]);
}

/* Determine the maximum number of placement allocations possible for both sides
 * @return the maximum number
 */
static unsigned int nr_placeable_invisibles_for_both_sides(void)
{
  assert(total_invisible_number
         >=(current_consumption.placed_fleshed_out[White]
            +current_consumption.placed_fleshed_out[Black]
            +current_consumption.placed_not_fleshed_out[White]
            +current_consumption.placed_not_fleshed_out[Black]
            + (current_consumption.claimed[White] && current_consumption.claimed[Black])
           )
        );

  return (total_invisible_number
          - ((current_consumption.placed_fleshed_out[White]
              +current_consumption.placed_fleshed_out[Black])
              +current_consumption.placed_not_fleshed_out[White]
              +current_consumption.placed_not_fleshed_out[Black]
             + (current_consumption.claimed[White] && current_consumption.claimed[Black])
            )
         );
}

/* Determine the maximum number of placement allocations possible for a specific side
 * @param side the side
 * @return the maximum number
 */
static unsigned int nr_placeable_invisibles_for_side(Side side)
{
  assert(total_invisible_number
         >= (current_consumption.placed_fleshed_out[White]
             +current_consumption.placed_fleshed_out[Black]
             +current_consumption.placed_not_fleshed_out[White]
             +current_consumption.placed_not_fleshed_out[Black]
             +current_consumption.claimed[advers(side)]
            )
        );

  return (total_invisible_number
          - ((current_consumption.placed_fleshed_out[White]
              +current_consumption.placed_fleshed_out[Black]
              +current_consumption.placed_not_fleshed_out[White]
              +current_consumption.placed_not_fleshed_out[Black])
              +current_consumption.claimed[advers(side)]
            )
         );
}

/* Allocate placement of an invisible, which may have already been claimed by
 * the side for a random move
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
static boolean allocate_placement_of_claimed_fleshed_out(Side side)
{
  ++current_consumption.placed_fleshed_out[side];
  current_consumption.claimed[side] = false;
  TraceConsumption();TraceEOL();

  return nr_total_invisbles_consumed()<=total_invisible_number;
}

/* Allocate placement of an invisible, which may have already been claimed by
 * the side for a random move
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
static boolean allocate_placement_of_claimed_not_fleshed_out(Side side)
{
  ++current_consumption.placed_not_fleshed_out[side];
  current_consumption.claimed[side] = false;
  TraceConsumption();TraceEOL();

  return nr_total_invisbles_consumed()<=total_invisible_number;
}

/* Allocate placement of an unclaimed invisible
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
static boolean allocate_placement_of_unclaimed_fleshed_out(Side side)
{
  ++current_consumption.placed_fleshed_out[side];
  TraceConsumption();TraceEOL();

  return nr_total_invisbles_consumed()<=total_invisible_number;
}

/* Adapt bookkeeping to the fleshing out of an invisible of a side */
static void reallocate_fleshing_out(Side side)
{
  --current_consumption.placed_not_fleshed_out[side];
  ++current_consumption.placed_fleshed_out[side];
}

static ply top_ply_of_regular_play;
static slice_index tester_slice;

static ply ply_replayed;

static stip_length_type combined_result;

static boolean end_of_iteration;

static unsigned int taboo[nr_sides][maxsquare];

static unsigned int taboo_arrival[maxply+1][nr_sides][maxsquare];

typedef enum
{
  play_regular,
  play_rewinding,
  play_detecting_revelations,
  play_validating_mate,
  play_testing_mate,
  play_initialising_replay,
  play_replay_validating,
  play_replay_testing,
  play_finalising_replay,
  play_unwinding,
  play_testing_goal
} play_phase_type;

static play_phase_type play_phase = play_regular;

typedef enum
{
  no_mate,
  mate_attackable,
  mate_defendable_by_interceptors,
  mate_with_2_uninterceptable_doublechecks,
  mate_unvalidated
} mate_validation_type;

static mate_validation_type mate_validation_result;
static mate_validation_type combined_validation_result;

static square sq_mating_piece_to_be_attacked = initsquare;

typedef struct
{
    square pos;
    piece_walk_type walk;
    Flags spec;
    square pos_first;
} revelation_status_type;

static boolean revelation_status_is_uninitialised;
static unsigned int nr_potential_revelations;
static revelation_status_type revelation_status[nr_squares_on_board];

typedef struct
{
    square pos;
    piece_walk_type walk;
    Flags spec;
    boolean is_allocated;
} knowledge_type;

static knowledge_type knowledge;

typedef unsigned int iteration_index_type;

static iteration_index_type current_iteration;
/* 0: "pre-history" including insertion of mating piece attackers
 * 1..n: flesh out invisibles
 */

/* avoid duplication when fleshing out random moves by invisibles: */
/* after they have fulfilled their purpose: */
/* in which iteration have we fleshed out the random move by inivisible of a certain ply? */
static iteration_index_type fleshed_out_random_move_last_time[maxply+1];
/* before they fulfill their purpose: */
static PieceIdType committed_to_fleshing_out_random_move_by[maxply+1];

typedef enum
{
  purpose_none,
  purpose_victim,
  purpose_interceptor,
  purpose_random_mover,
  purpose_capturer,
  purpose_castling_partner,
  purpose_attacker
} purpose_type;

typedef struct action_type
{
    purpose_type purpose;
    ply acts_when;
    square on;
} action_type;

typedef struct
{
    iteration_index_type insertion_iteration;
    action_type first;
    action_type last;
} motivation_type;

static motivation_type motivation[MaxPieceId+1];

static ply flesh_out_move_highwater = ply_retro_move;

static PieceIdType next_invisible_piece_id;

static move_effect_journal_index_type top_before_relevations[maxply+1];

static void report_deadend(char const *s, unsigned int lineno)
{
  printf("%s;%u;%u\n",s,lineno,play_phase);
}

#define REPORT_DEADEND report_deadend("DEADEND",__LINE__)
#define REPORT_EXIT report_deadend("EXIT",__LINE__)

#define REPORT_DEADEND
#define REPORT_EXIT

static void write_history_recursive(ply ply)
{
  if (parent_ply[ply]>ply_retro_move)
    write_history_recursive(parent_ply[ply]);

  {
    move_effect_journal_index_type const base = move_effect_journal_base[ply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    fprintf(stdout," %u:",ply);
    WriteWalk(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.from);
    fputs("-",stdout);
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.to);
  }
}

void total_invisible_write_flesh_out_history(void)
{
  if (nbply!=ply_nil)
  {
    fprintf(stdout," -");
    write_history_recursive(top_ply_of_regular_play);
  }
}

static boolean was_taboo(square s)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  for (ply = ply_retro_move+1; ply<nbply; ++ply)
  {
    TraceValue("%u",ply);
    TraceValue("%u",taboo_arrival[ply][White][s]);
    TraceValue("%u",taboo_arrival[ply][Black][s]);
    TraceEOL();
    if (taboo_arrival[ply][White][s]>0 || taboo_arrival[ply][Black][s]>0)
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

static boolean is_random_move_by_invisible(ply ply)
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

static boolean is_taboo(square s, Side side)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  for (ply = nbply; ply<=top_ply_of_regular_play; ++ply)
    if (taboo_arrival[ply][side][s])
    {
      TraceValue("%u",ply);
      TraceValue("%u",taboo_arrival[ply][side][s]);
      TraceEOL();
      result = true;
      break;
    }
    else if (side==trait[ply])
    {
      if (is_random_move_by_invisible(ply))
        break;
    }
    else
    {
      if (is_taboo_candidate_captured(ply,s))
        break;
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

static void update_taboo_arrival(int delta)
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
      update_taboo_piece_movement_rider(delta,movement,&taboo_arrival[nbply]);
    else if (is_pawn(walk))
    {
      if (move_effect_journal[capture].type==move_effect_no_piece_removal)
        update_taboo_piece_movement_pawn_no_capture(delta,movement,&taboo_arrival[nbply]);
      else
        update_taboo_piece_movement_pawn_capture(delta,movement,&taboo_arrival[nbply]);
    }
    else
    {
      if (walk==King
          && move_effect_journal[movement].reason==move_effect_reason_castling_king_movement)
        update_taboo_piece_movement_castling(delta,movement,&taboo_arrival[nbply]);
      else
        update_taboo_piece_movement_leaper(delta,movement,&taboo_arrival[nbply]);
    }

    {
      move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
      move_effect_journal_index_type idx;

      for (idx = base+move_effect_journal_index_offset_other_effects; idx!=top; ++idx)
        if (move_effect_journal[idx].type==move_effect_piece_movement
            && move_effect_journal[idx].reason==move_effect_reason_castling_partner)
          update_taboo_piece_movement_castling(delta,idx,&taboo_arrival[nbply]);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo(int delta)
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

  update_taboo_arrival(delta);

  if (sq_departure==move_by_invisible
      || sq_departure>=capture_by_invisible)
  {
    /* no taboos! */
  }
  else
  {
    if (is_rider(walk))
      update_taboo_piece_movement_rider(delta,movement,&taboo);
    else if (is_pawn(walk))
    {
      if (move_effect_journal[capture].type==move_effect_no_piece_removal)
        update_taboo_piece_movement_pawn_no_capture(delta,movement,&taboo);
      else
        update_taboo_piece_movement_pawn_capture(delta,movement,&taboo);
    }
    else
    {
      if (walk==King
          && move_effect_journal[movement].reason==move_effect_reason_castling_king_movement)
        update_taboo_piece_movement_castling(delta,movement,&taboo);
      else
        update_taboo_piece_movement_leaper(delta,movement,&taboo);
    }

    {
      move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
      move_effect_journal_index_type idx;

      for (idx = base+move_effect_journal_index_offset_other_effects; idx!=top; ++idx)
        if (move_effect_journal[idx].type==move_effect_piece_movement
            && move_effect_journal[idx].reason==move_effect_reason_castling_partner)
          update_taboo_piece_movement_castling(delta,idx,&taboo);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_rider_check_uninterceptable_on_vector(Side side_checking, square king_pos,
                                                        vec_index_type k, piece_walk_type rider_walk)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",k);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    square s = king_pos+vec[k];
    while (is_square_empty(s)
           && (was_taboo(s) || (is_taboo(s,White) && is_taboo(s,Black))))
      s += vec[k];

    {
      piece_walk_type const walk = get_walk_of_piece_on_square(s);
      result = ((walk==rider_walk || walk==Queen)
                && TSTFLAG(being_solved.spec[s],side_checking));
    }
    TraceSquare(s);
    TraceValue("%u",is_square_empty(s));
    TraceValue("%u",taboo[White][s]);
    TraceValue("%u",taboo[Black][s]);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_rider_check_uninterceptable(Side side_checking, square king_pos,
                                                     vec_index_type kanf, vec_index_type kend, piece_walk_type rider_walk)
{
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",kanf);
  TraceValue("%u",kend);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    vec_index_type k;
    for (k = kanf; !result && k<=kend; k++)
      if (is_rider_check_uninterceptable_on_vector(side_checking,king_pos,k,rider_walk))
      {
        result = k;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean can_interceptor_be_allocated(void)
{
  consumption_type const save_consumption = current_consumption;
  boolean result = allocate_placement_of_claimed_fleshed_out(White);
  current_consumption = save_consumption;
  result = result || allocate_placement_of_claimed_fleshed_out(Black);
  current_consumption = save_consumption;
  return result;
}

static vec_index_type is_square_uninterceptably_attacked(Side side_under_attack, square sq_attacked)
{
  Side const side_checking = advers(side_under_attack);
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (can_interceptor_be_allocated())
  {
    if (being_solved.king_square[side_under_attack]!=initsquare)
    {
      if (!result && being_solved.number_of_pieces[side_checking][King]>0)
        result = king_check_ortho(side_checking,sq_attacked);

      if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
        result = pawn_check_ortho(side_checking,sq_attacked);

      if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
        result = knight_check_ortho(side_checking,sq_attacked);

      if (!result && being_solved.number_of_pieces[side_checking][Rook]+being_solved.number_of_pieces[side_checking][Queen]>0)
        result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_rook_start,vec_rook_end, Rook);

      if (!result && being_solved.number_of_pieces[side_checking][Bishop]+being_solved.number_of_pieces[side_checking][Queen]>0)
        result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_bishop_start,vec_bishop_end, Bishop);
    }
  }
  else
    result = is_square_observed_ortho(side_checking,sq_attacked);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_square_attacked_by_uninterceptable(Side side_under_attack, square sq_attacked)
{
  vec_index_type result = 0;
  Side const side_checking = advers(side_under_attack);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (being_solved.king_square[side_under_attack]!=initsquare)
  {
    if (!result && being_solved.number_of_pieces[side_checking][King]>0)
      result = king_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
      result = pawn_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
      result = knight_check_ortho(side_checking,sq_attacked);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void do_revelation_of_new_invisible(move_effect_reason_type reason,
                                           square on,
                                           piece_walk_type walk,
                                           Flags spec)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_revelation_of_new_invisible,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(play_phase==play_regular);

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = walk;
  entry->u.piece_addition.added.flags = spec;

  if (TSTFLAG(spec,White))
    ++being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    ++being_solved.number_of_pieces[Black][walk];
  occupy_square(on,walk,spec);

  if (TSTFLAG(spec,Royal) && walk==King)
  {
    Side const side = TSTFLAG(spec,White) ? White : Black;
    being_solved.king_square[side] = on;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void reveal_new(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;
  Side const side_revealed = TSTFLAG(spec,White) ? White : Black;
  PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec,White))
    ++being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    ++being_solved.number_of_pieces[Black][walk];
  replace_walk(on,walk);
  being_solved.spec[on] = spec;
  SetPieceId(being_solved.spec[on],id_on_board);
  if (TSTFLAG(spec,Royal) && walk==King)
  {
    TraceSquare(being_solved.king_square[side_revealed]);
    being_solved.king_square[side_revealed] = on;
  }
  TraceValue("%x",being_solved.spec[on]);TraceEOL();
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unreveal_new(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;
  Side const side_revealed = TSTFLAG(spec,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(play_phase==play_validating_mate);
  assert(get_walk_of_piece_on_square(on)==walk);
  assert(((being_solved.spec[on])&PieSpMask)==((spec)&PieSpMask));
  TraceText("substituting dummy for revealed piece\n");
  if (TSTFLAG(spec,Royal) && walk==King)
  {
    CLRFLAG(being_solved.spec[on],Royal);
    being_solved.king_square[side_revealed] = initsquare;
  }
  SETFLAG(being_solved.spec[on],Chameleon);
  replace_walk(on,Dummy);
  if (TSTFLAG(spec,White))
    --being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    --being_solved.number_of_pieces[Black][walk];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceWalk(walk);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",spec);
  TraceValue("%x",being_solved.spec[on]);
  TraceValue("%u",play_phase);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    case play_rewinding:
      if (TSTFLAG(spec,White))
        --being_solved.number_of_pieces[White][walk];
      if (TSTFLAG(spec,Black))
        --being_solved.number_of_pieces[Black][walk];

      empty_square(on);

      if (TSTFLAG(spec,Royal) && walk==King)
      {
        Side const side = TSTFLAG(spec,White) ? White : Black;
        being_solved.king_square[side] = initsquare;
      }
      break;

    case play_validating_mate:
      assert(!is_square_empty(on));
      SETFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
      break;

    case play_detecting_revelations:
    case play_testing_mate:
    case play_unwinding:
    case play_finalising_replay:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;
  Side const side_revealed = TSTFLAG(spec,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceWalk(walk);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",spec);
  TraceValue("%x",being_solved.spec[on]);
  TraceValue("%u",play_phase);
  TraceEnumerator(Side,side_revealed);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    case play_unwinding:
      assert(is_square_empty(on));
      if (TSTFLAG(spec,White))
        ++being_solved.number_of_pieces[White][walk];
      if (TSTFLAG(spec,Black))
        ++being_solved.number_of_pieces[Black][walk];
      occupy_square(on,walk,spec);

      if (TSTFLAG(spec,Royal) && walk==King)
      {
        Side const side = TSTFLAG(spec,White) ? White : Black;
        being_solved.king_square[side] = on;
      }
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      break;

    case play_validating_mate:
      assert(!is_square_empty(on));
      assert(TSTFLAG(being_solved.spec[on],side_revealed));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_finalising_replay:
    case play_replay_validating:
    case play_replay_testing:
      break;

    case play_detecting_revelations:
    case play_testing_mate:
    case play_rewinding:
    case play_initialising_replay:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_revelation_of_castling_partner(move_effect_reason_type reason,
                                              square on,
                                              piece_walk_type walk,
                                              Flags spec)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_revelation_of_castling_partner,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(play_phase==play_regular);

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = walk;
  entry->u.piece_addition.added.flags = spec;

  assert(get_walk_of_piece_on_square(on)==Rook);
  assert(walk==Rook);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (play_phase)
  {
    case play_regular:
    case play_rewinding:
      TraceSquare(on);
      TraceValue("%x",being_solved.spec[on]);
      TraceEOL();
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      SETFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      SETFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_unwinding:
      assert(0);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    case play_unwinding:
      TraceSquare(on);
      TraceValue("%x",being_solved.spec[on]);
      TraceEOL();
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_rewinding:
      assert(0);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_testing_mate:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      assert(!is_square_empty(on));
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void taint_history_of_placed_piece(move_effect_journal_index_type idx)
{
  move_effect_journal_index_type const total_base = move_effect_journal_base[ply_retro_move+1];
  square pos = move_effect_journal[idx].u.revelation_of_placed_piece.on;
  PieceIdType const id = GetPieceId(move_effect_journal[idx].u.revelation_of_placed_piece.flags_revealed);
  piece_walk_type const walk_to = move_effect_journal[idx].u.revelation_of_placed_piece.walk_revealed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  TraceSquare(pos);
  TraceWalk(walk_to);
  TraceValue("%u",id);
  TraceEOL();

  assert(move_effect_journal[idx].type==move_effect_revelation_of_placed_invisible);

  while (idx>=total_base)
  {
    TraceValue("%u",idx);
    TraceValue("%u",move_effect_journal[idx].type);
    TraceEOL();

    switch (move_effect_journal[idx].type)
    {
      case move_effect_piece_movement:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec));
        TraceSquare(move_effect_journal[idx].u.piece_movement.to);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec))
        {
          assert(pos==move_effect_journal[idx].u.piece_movement.to);
          TraceSquare(move_effect_journal[idx].u.piece_movement.from);
          TraceWalk(move_effect_journal[idx].u.piece_movement.moving);
          TraceEOL();
          assert(move_effect_journal[idx].u.piece_movement.moving==Dummy);
          pos = move_effect_journal[idx].u.piece_movement.from;
          move_effect_journal[idx].u.piece_movement.moving = walk_to;
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_movement.to);
        }
        break;

      case move_effect_piece_readdition:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags));
        TraceSquare(move_effect_journal[idx].u.piece_addition.added.on);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags))
        {
          assert(pos==move_effect_journal[idx].u.piece_addition.added.on);
          if (pos>=capture_by_invisible)
          {
            TraceWalk(move_effect_journal[idx].u.piece_addition.added.walk);
            TraceEOL();
            move_effect_journal[idx].u.piece_addition.added.walk = walk_to;
          }
          idx = 1;
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_addition.added.on);
        }
        break;

      default:
        break;
    }
    --idx;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void untaint_history_of_placed_piece(move_effect_journal_index_type idx)
{
  move_effect_journal_index_type const total_base = move_effect_journal_base[ply_retro_move+1];
  square pos = move_effect_journal[idx].u.revelation_of_placed_piece.on;
  PieceIdType const id = GetPieceId(move_effect_journal[idx].u.revelation_of_placed_piece.flags_revealed);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  TraceSquare(pos);
  TraceWalk(move_effect_journal[idx].u.revelation_of_placed_piece.walk_revealed);
  TraceValue("%u",id);
  TraceEOL();

  assert(move_effect_journal[idx].type==move_effect_revelation_of_placed_invisible);

  while (idx>=total_base)
  {
    switch (move_effect_journal[idx].type)
    {
      case move_effect_piece_movement:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec));
        TraceSquare(move_effect_journal[idx].u.piece_movement.to);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec))
        {
          assert(pos==move_effect_journal[idx].u.piece_movement.to);
          pos = move_effect_journal[idx].u.piece_movement.from;
          move_effect_journal[idx].u.piece_movement.moving = Dummy;
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_movement.to);
        }
        break;

      case move_effect_piece_readdition:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags));
        TraceSquare(move_effect_journal[idx].u.piece_addition.added.on);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags))
        {
          assert(pos==move_effect_journal[idx].u.piece_addition.added.on);
          move_effect_journal[idx].u.piece_addition.added.walk = Dummy;
          idx = 1;
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_addition.added.on);
        }
        break;

      default:
        break;
    }

    --idx;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_revelation_of_placed_invisible(move_effect_reason_type reason,
                                              square on,
                                              piece_walk_type walk,
                                              Flags spec)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_revelation_of_placed_invisible,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(play_phase==play_regular);
  assert(TSTFLAG(being_solved.spec[on],Chameleon));

  entry->u.revelation_of_placed_piece.on = on;
  entry->u.revelation_of_placed_piece.walk_original = get_walk_of_piece_on_square(on);
  entry->u.revelation_of_placed_piece.flags_original = being_solved.spec[on];
  entry->u.revelation_of_placed_piece.walk_revealed = walk;
  entry->u.revelation_of_placed_piece.flags_revealed = spec;

  if (TSTFLAG(spec,Royal) && walk==King)
  {
    Side const side = TSTFLAG(spec,White) ? White : Black;
    being_solved.king_square[side] = on;
  }

  replace_walk(on,walk);
  being_solved.spec[on] = spec;

  if (TSTFLAG(spec,White))
    ++being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    ++being_solved.number_of_pieces[Black][walk];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void reveal_placed(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
  PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(TSTFLAG(being_solved.spec[on],Chameleon));
  being_solved.spec[on] = flags_revealed;
  SetPieceId(being_solved.spec[on],id_on_board);
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unreveal_placed(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(get_walk_of_piece_on_square(on)==walk_revealed);
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));
  assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
  SETFLAG(being_solved.spec[on],Chameleon);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  piece_walk_type const walk_original = entry->u.revelation_of_placed_piece.walk_original;
  Flags const flags_original = entry->u.revelation_of_placed_piece.flags_original;
  piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",play_phase);
  TraceSquare(on);
  TraceWalk(walk_original);
  TraceWalk(walk_revealed);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",flags_original);
  TraceValue("%x",flags_revealed);
  TraceValue("%x",being_solved.spec[on]);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
      assert(!is_square_empty(on));

      if (TSTFLAG(flags_revealed,Royal) && walk_revealed==King)
      {
        Side const side = TSTFLAG(flags_revealed,White) ? White : Black;
        being_solved.king_square[side] = initsquare;
      }

      replace_walk(on,walk_original);

      if (TSTFLAG(being_solved.spec[on],White))
        --being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
      if (TSTFLAG(being_solved.spec[on],Black))
        --being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      SETFLAG(being_solved.spec[on],Chameleon);
      /* just in case */
      CLRFLAG(being_solved.spec[on],Royal);
      being_solved.spec[on] = flags_original;
      break;

    case play_rewinding:
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      taint_history_of_placed_piece(entry-&move_effect_journal[0]);
      assert(!is_square_empty(on));
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      SETFLAG(being_solved.spec[on],Chameleon);
      if (TSTFLAG(being_solved.spec[on],Royal))
      {
        Side const side = TSTFLAG(flags_revealed,White) ? White : Black;
        assert(being_solved.king_square[side]==on);
        assert(TSTFLAG(flags_revealed,Royal));
        being_solved.king_square[side] = initsquare;
        CLRFLAG(being_solved.spec[on],Royal);
      }
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      unreveal_placed(entry);
      break;

    case play_unwinding:
      assert(0);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
  Side const side_revealed = TSTFLAG(flags_revealed,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",play_phase);
  TraceSquare(on);
  TraceWalk(walk_revealed);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",flags_revealed);
  TraceValue("%x",being_solved.spec[on]);
  TraceEnumerator(Side,side_revealed);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    {
      PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);
      assert(get_walk_of_piece_on_square(on)==Dummy);

      if (TSTFLAG(flags_revealed,Royal) && walk_revealed==King)
      {
        Side const side = TSTFLAG(flags_revealed,White) ? White : Black;
        being_solved.king_square[side] = on;
      }

      replace_walk(on,walk_revealed);

      if (TSTFLAG(being_solved.spec[on],White))
        ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
      if (TSTFLAG(being_solved.spec[on],Black))
        ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

      being_solved.spec[on] = flags_revealed;
      SetPieceId(being_solved.spec[on],id_on_board);
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      break;
    }

    case play_rewinding:
      assert(0);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      assert(!is_square_empty(on));
      assert(get_walk_of_piece_on_square(on)==walk_revealed
             && TSTFLAG(being_solved.spec[on],side_revealed));
      reveal_placed(entry);
      break;

    case play_unwinding:
    {
      PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);
      assert(!is_square_empty(on));
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      being_solved.spec[on] = flags_revealed;
      SetPieceId(being_solved.spec[on],id_on_board);
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      untaint_history_of_placed_piece(entry-&move_effect_journal[0]);
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      if (TSTFLAG(flags_revealed,Royal))
      {
        assert(being_solved.king_square[side_revealed]==initsquare);
        being_solved.king_square[side_revealed] = on;
      }
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void add_revelation_effect(square s, piece_walk_type walk, Flags spec)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(TSTFLAG(spec,Chameleon));
  CLRFLAG(spec,Chameleon);

  if (is_square_empty(s))
  {
    TraceValue("%u",nbply);
    TraceConsumption();
    TraceText("revelation of a hitherto unplaced invisible (typically a king)\n");
    SetPieceId(spec,++next_invisible_piece_id);
    do_revelation_of_new_invisible(move_effect_reason_revelation_of_invisible,
                                   s,walk,spec);
  }
  else
  {
    if (move_effect_journal[base].type==move_effect_piece_readdition
        && move_effect_journal[base].reason==move_effect_reason_castling_partner
        && (GetPieceId(move_effect_journal[base].u.piece_addition.added.flags)
            ==GetPieceId(spec)))
    {
      TraceText("pseudo revelation of a castling partner\n");
      assert(TSTFLAG(being_solved.spec[s],Chameleon));
      CLRFLAG(being_solved.spec[s],Chameleon);
      assert(GetPieceId(spec)==GetPieceId(being_solved.spec[s]));
      do_revelation_of_castling_partner(move_effect_reason_revelation_of_invisible,
                                        s,walk,spec);
    }
    else
    {
      TraceText("revelation of a placed invisible\n");
      SetPieceId(spec,GetPieceId(being_solved.spec[s]));
      do_revelation_of_placed_invisible(move_effect_reason_revelation_of_invisible,
                                        s,walk,spec);
    }
  }

  assert(!TSTFLAG(being_solved.spec[s],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void setup_revelations(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square sq_ep_capture = initsquare;

  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].reason==move_effect_reason_ep_capture)
    sq_ep_capture = move_effect_journal[capture].u.piece_removal.on;

  nr_potential_revelations = 0;

  for (s = boardnum; *s; ++s)
    if (*s!=sq_ep_capture
        && (is_square_empty(*s) || TSTFLAG(being_solved.spec[*s],Chameleon)))
    {
      revelation_status[nr_potential_revelations].pos = *s;
      ++nr_potential_revelations;
    }

  revelation_status_is_uninitialised = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void initialise_revelations(void)
{
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (i!=nr_potential_revelations)
  {
    square const s = revelation_status[i].pos;
    piece_walk_type const walk = get_walk_of_piece_on_square(s);
    if (walk==Empty)
    {
      memmove(&revelation_status[i],&revelation_status[i+1],
              (nr_potential_revelations-i-1)*sizeof revelation_status[0]);
      --nr_potential_revelations;
    }
    else
    {
      TraceSquare(s);
      TraceWalk(walk);
      TraceValue("%x",being_solved.spec[s]);
      TraceEOL();
      revelation_status[i].walk = walk;
      revelation_status[i].spec = being_solved.spec[s];
      revelation_status[i].pos_first = motivation[GetPieceId(being_solved.spec[s])].first.on;
      ++i;
    }
  }

  revelation_status_is_uninitialised = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_revelations(void)
{
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (i!=nr_potential_revelations)
  {
    square const s = revelation_status[i].pos;
    if (get_walk_of_piece_on_square(s)!=revelation_status[i].walk
        || (being_solved.spec[s]&PieSpMask)!=(revelation_status[i].spec&PieSpMask))
    {
      TraceSquare(s);
      TraceWalk(get_walk_of_piece_on_square(s));
      TraceValue("%x",being_solved.spec[s]);
      TraceEOL();
      memmove(&revelation_status[i],&revelation_status[i+1],
              (nr_potential_revelations-i-1)*sizeof revelation_status[0]);
      --nr_potential_revelations;
    }
    else
    {
      PieceIdType const id = GetPieceId(being_solved.spec[s]);
      square const first_on = motivation[id].first.on;
      assert(id!=NullPieceId);
      assert(is_on_board(first_on));
      if (first_on!=revelation_status[i].pos_first)
        revelation_status[i].pos_first = initsquare;
      ++i;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void evaluate_revelations(void)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_potential_revelations; ++i)
  {
    square const s = revelation_status[i].pos;
    TraceSquare(s);TraceWalk(revelation_status[i].walk);TraceEOL();
    if (revelation_status[i].walk!=Empty)
    {
      add_revelation_effect(s,revelation_status[i].walk,revelation_status[i].spec);
      TraceSquare(revelation_status[i].pos_first);TraceEOL();
      if (revelation_status[i].pos_first!=initsquare)
      {
        PieceIdType const id = GetPieceId(revelation_status[i].spec);
        knowledge.pos = revelation_status[i].pos_first;
        knowledge.walk = revelation_status[i].walk;
        knowledge.spec = revelation_status[i].spec;
        knowledge.is_allocated = motivation[id].first.purpose==purpose_castling_partner;
        TraceWalk(knowledge.walk);
        TraceSquare(knowledge.pos);
        TraceValue("%x",knowledge.spec);
        TraceEOL();
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void replay_fleshed_out_move_sequence(play_phase_type phase_replay)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",phase_replay);
  TraceFunctionParamListEnd();

  if (nbply>ply_retro_move+1)
  {
    --nbply;
    undo_move_effects();
    replay_fleshed_out_move_sequence(phase_replay);
    redo_move_effects();
    ++nbply;
  }
  else
  {
    ply_replayed = nbply;
    nbply = top_ply_of_regular_play;

    TracePosition(being_solved.board,being_solved.spec);

    play_phase = phase_replay;

    mate_validation_result = mate_unvalidated;

    pipe_solve_delegate(tester_slice);

    if (solve_result>combined_result)
      combined_result = solve_result;
    TracePosition(being_solved.board,being_solved.spec);

    play_phase = play_finalising_replay;

    nbply = ply_replayed;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_validating_king_placements(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TracePosition(being_solved.board,being_solved.spec);

  REPORT_EXIT;

  switch (play_phase)
  {
    case play_detecting_revelations:
      if (revelation_status_is_uninitialised)
        initialise_revelations();
      else
        update_revelations();
      if (nr_potential_revelations==0)
        end_of_iteration = true;
      break;

    case play_validating_mate:
      play_phase = play_initialising_replay;
      replay_fleshed_out_move_sequence(play_replay_validating);
      play_phase = play_validating_mate;

      if (mate_validation_result<combined_validation_result)
        combined_validation_result = mate_validation_result;
      if (mate_validation_result<=mate_attackable)
        end_of_iteration = true;

      break;

    case play_testing_mate:
      play_phase = play_initialising_replay;
      replay_fleshed_out_move_sequence(play_replay_testing);
      play_phase = play_testing_mate;

      /* This:
       * assert(solve_result>=previous_move_has_solved);
       * held surprisingly long, especially since it's wrong.
       * E.g. mate by castling: if we attack the rook, the castling is not
       * even playable */
      if (solve_result==previous_move_has_not_solved)
        end_of_iteration = true;

      break;

    default:
      assert(0);
      break;
  }

//  printf("%u:",play_phase);
//  total_invisible_write_flesh_out_history();
//  printf(" - ");
//  {
//    square const *bnp;
//    for (bnp = boardnum; *bnp!=initsquare; ++bnp)
//      if (!is_square_empty(*bnp) && !is_square_blocked(*bnp))
//      {
//        WriteSpec(&output_plaintext_engine,
//                  stdout,being_solved.spec[*bnp],
//                  being_solved.board[*bnp],true);
//        WriteWalk(&output_plaintext_engine,
//                  stdout,
//                  get_walk_of_piece_on_square(*bnp));
//        WriteSquare(&output_plaintext_engine,stdout,*bnp);
//        printf("(%lu) ",GetPieceId(being_solved.spec[*bnp]));
//      }
//  }
//  switch (play_phase)
//  {
//    case play_detecting_revelations:
//      printf("-");
//      {
//        unsigned int i;
//        for (i = 0; i!=nr_potential_revelations; ++i)
//        {
//          printf(TSTFLAG(revelation_status[i].spec,White) ? " w" : " s");
//          WriteWalk(&output_plaintext_engine,
//                    stdout,
//                    revelation_status[i].walk);
//          WriteSquare(&output_plaintext_engine,stdout,revelation_status[i].pos);
//          printf("(%lu)",GetPieceId(revelation_status[i].spec));
//        }
//      }
//      break;
//
//    default:
//      break;
//  }
//  printf(":%u\n",end_of_iteration);

  TracePosition(being_solved.board,being_solved.spec);
  TraceValue("%u",end_of_iteration);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_iteration(void);

static void restart_from_scratch(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply==ply_retro_move+1)
  {
    TraceValue("%u",nbply);TraceEOL();
    start_iteration();
  }
  else
  {
    --nbply;
    undo_move_effects();
    restart_from_scratch();
    redo_move_effects();
    ++nbply;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void nominate_king_invisible_by_invisible(void)
{
  Side const side_to_be_mated = Black;
  Side const side_mating = White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const *s;
    TraceText("Try to make one of the placed invisibles the king to be mated\n");
    for (s = boardnum; *s && !end_of_iteration; ++s)
      if (get_walk_of_piece_on_square(*s)==Dummy
          && TSTFLAG(being_solved.spec[*s],side_to_be_mated))
      {
        Flags const save_flags = being_solved.spec[*s];
        CLRFLAG(being_solved.spec[*s],side_mating);
        SETFLAG(being_solved.spec[*s],Royal);
        ++being_solved.number_of_pieces[side_to_be_mated][King];
        being_solved.board[*s] = King;
        being_solved.king_square[side_to_be_mated] = *s;

        TraceSquare(*s);TraceEOL();
        restart_from_scratch();

        being_solved.board[*s] = Dummy;
        --being_solved.number_of_pieces[side_to_be_mated][King];
        being_solved.spec[*s] = save_flags;
      }

    being_solved.king_square[side_to_be_mated] = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_king_placements(void)
{
  Side const side_to_be_mated = Black;
  Side const side_mating = White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,side_to_be_mated);
  TraceSquare(being_solved.king_square[side_to_be_mated]);
  TraceConsumption();
  TraceEnumerator(Side,side_mating);
  TraceSquare(being_solved.king_square[side_mating]);
  TraceEOL();
  TracePosition(being_solved.board,being_solved.spec);

  if (being_solved.king_square[side_to_be_mated]==initsquare)
  {
    being_solved.king_square[side_to_be_mated] = nullsquare;
    if (nr_total_invisbles_consumed()==total_invisible_number)
    {
      being_solved.king_square[side_to_be_mated] = initsquare;
      if (current_consumption.placed_not_fleshed_out[side_to_be_mated]>0)
      {
        consumption_type const save_consumption = current_consumption;

        reallocate_fleshing_out(side_to_be_mated);
        nominate_king_invisible_by_invisible();
        current_consumption = save_consumption;
      }
    }
    else
    {
      being_solved.king_square[side_to_be_mated] = initsquare;
      TraceText("The king to be mated can be anywhere\n");
      REPORT_DEADEND;

      switch (play_phase)
      {
        case play_detecting_revelations:
          if (revelation_status_is_uninitialised)
            initialise_revelations();
          else
            update_revelations();

          if (nr_potential_revelations==0)
            end_of_iteration = true;

          break;

        case play_validating_mate:
          combined_validation_result = no_mate;
          combined_result = previous_move_has_not_solved;
          end_of_iteration = true;
          break;

        default:
          combined_result = previous_move_has_not_solved;
          end_of_iteration = true;
          break;
      }
    }
  }
  else if (being_solved.king_square[side_mating]==initsquare
           && nr_total_invisbles_consumed()==total_invisible_number)
  {
    combined_result = previous_move_is_illegal;
    REPORT_DEADEND;
  }
  else
    done_validating_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_and_execute_revelations(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

  if (curr==move_effect_journal_base[nbply+1])
  {
    if (nbply<=flesh_out_move_highwater)
    {
      ++nbply;
      TraceValue("%u",nbply);TraceEOL();
      start_iteration();
      --nbply;
    }
    else
    {
      ply const save_highwater = flesh_out_move_highwater;
      flesh_out_move_highwater = nbply;
      ++nbply;
      TraceValue("%u",nbply);TraceEOL();
      start_iteration();
      flesh_out_move_highwater = save_highwater;
      --nbply;
    }
  }
  else
  {
    move_effect_journal_entry_type * const entry = &move_effect_journal[curr];
    switch (entry->type)
    {
      case move_effect_revelation_of_new_invisible:
      {
        square const on = entry->u.piece_addition.added.on;
        piece_walk_type const walk = entry->u.piece_addition.added.walk;
        Flags const spec = entry->u.piece_addition.added.flags;
        Side const side_revealed = TSTFLAG(spec,White) ? White : Black;

        if (is_square_empty(on))
        {
          TraceText("revelation expected, but square is empty - aborting\n");
          REPORT_DEADEND;
        }
        else if (play_phase==play_validating_mate && get_walk_of_piece_on_square(on)==Dummy)
        {
          if (TSTFLAG(spec,Royal)
              && walk==King
              && being_solved.king_square[side_revealed]!=initsquare)
          {
            // TODO can we avoid this situation?
            TraceText("revelation of king - but king has already been placed - aborting\n");
            REPORT_DEADEND;
          }
          else if (TSTFLAG(being_solved.spec[on],side_revealed))
          {
            reveal_new(entry);
            test_and_execute_revelations(curr+1);
            unreveal_new(entry);
          }
          else
          {
            TraceText("revealed piece belongs to different side than actual piece\n");
            REPORT_DEADEND;
          }
        }
        else if (get_walk_of_piece_on_square(on)==walk
                 && TSTFLAG(being_solved.spec[on],side_revealed))
        {
          entry->type = move_effect_revelation_of_placed_invisible;
          entry->u.revelation_of_placed_piece.on = on;
          entry->u.revelation_of_placed_piece.walk_original = get_walk_of_piece_on_square(on);
          entry->u.revelation_of_placed_piece.flags_original = being_solved.spec[on];
          entry->u.revelation_of_placed_piece.walk_revealed = walk;
          entry->u.revelation_of_placed_piece.flags_revealed = spec;
          reveal_placed(entry);
          test_and_execute_revelations(curr+1);
          unreveal_placed(entry);
          entry->type = move_effect_revelation_of_new_invisible;
          entry->u.piece_addition.added.on = on;
          entry->u.piece_addition.added.walk = walk;
          entry->u.piece_addition.added.flags = spec;
        }
        else
        {
          TraceText("revelation expected - but walk of present piece is different - aborting\n");
          REPORT_DEADEND;
        }
        break;
      }

      case move_effect_revelation_of_placed_invisible:
      {
        square const on = entry->u.revelation_of_placed_piece.on;
        piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
        Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
        Side const side_revealed = TSTFLAG(flags_revealed,White) ? White : Black;

        assert(!is_square_empty(on));
        if (get_walk_of_piece_on_square(on)==walk_revealed
            && TSTFLAG(being_solved.spec[on],side_revealed))
        {
          reveal_placed(entry);
          test_and_execute_revelations(curr+1);
          unreveal_placed(entry);
        }
        else
        {
          TraceText("the revelation has been violated - terminating redoing effects with this ply\n");
          REPORT_DEADEND;
        }
        break;
      }

      case move_effect_revelation_of_castling_partner:
        redo_revelation_of_castling_partner(entry);
        test_and_execute_revelations(curr+1);
        undo_revelation_of_castling_partner(entry);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void recurse_into_child_ply(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  move_effect_journal_index_type const save_top = move_effect_journal_base[nbply+1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(sq_departure==move_by_invisible
         || GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec)!=NullPieceId);

  move_effect_journal_base[nbply+1] = top_before_relevations[nbply];
  assert(top_before_relevations[nbply]>move_effect_journal_base[nbply]);
  redo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

  test_and_execute_revelations(top_before_relevations[nbply]);

  move_effect_journal_base[nbply+1] = top_before_relevations[nbply];
  assert(top_before_relevations[nbply]>move_effect_journal_base[nbply]);
  undo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void adapt_capture_effect(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(to);
  TraceWalk(get_walk_of_piece_on_square(to));
  TraceValue("%x",being_solved.spec[to]);
  TraceEOL();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    if (is_square_empty(to))
    {
      TraceText("no capture planned and destination square empty - no need for adaptation\n");
      recurse_into_child_ply();
    }
    else
    {
      TraceText("capture of a total invisible that happened to land on the arrival square\n");

      assert(TSTFLAG(being_solved.spec[to],advers(trait[nbply])));
      assert(move_effect_journal[movement].u.piece_movement.moving!=Pawn);
      assert(!TSTFLAG(being_solved.spec[to],Royal));

      move_effect_journal[capture].type = move_effect_piece_removal;
      move_effect_journal[capture].reason = move_effect_reason_regular_capture;
      move_effect_journal[capture].u.piece_removal.on = to;
      move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
      move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];
      recurse_into_child_ply();
      move_effect_journal[capture].type = move_effect_no_piece_removal;
    }
  }
  else if (move_effect_journal[base].type==move_effect_piece_readdition)
  {
    assert(move_effect_journal[base].u.piece_addition.added.on==to);

    TraceText("capture of invisible victim added for the purpose\n");

    if (is_square_empty(to))
      recurse_into_child_ply();
    else
    {
      assert(move_effect_journal[movement].u.piece_movement.moving==Pawn);
      TraceText("another total invisible has moved to the arrival square\n");

      if (TSTFLAG(being_solved.spec[to],advers(trait[nbply])))
      {
        piece_walk_type const walk_victim_orig = move_effect_journal[capture].u.piece_removal.walk;
        /* if the piece to be captured is royal, then our tests for self check have failed */
        assert(!TSTFLAG(being_solved.spec[to],Royal));
        move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
        recurse_into_child_ply();
        move_effect_journal[capture].u.piece_removal.walk = walk_victim_orig;
      }
      else
      {
        TraceText("move is now blocked\n");
        REPORT_DEADEND;
      }
    }
  }
  else if (is_square_empty(to))
  {
    TraceText("original capture victim was captured by a TI that has since left\n");
    if (is_pawn(move_effect_journal[movement].u.piece_movement.moving))
    {
      TraceText("bad idea if the capturer is a pawn!\n");
      REPORT_DEADEND;
    }
    else
    {
      move_effect_journal[capture].type = move_effect_no_piece_removal;
      recurse_into_child_ply();
      move_effect_journal[capture].type = move_effect_piece_removal;
    }
  }
  else
  {
    piece_walk_type const orig_walk_removed = move_effect_journal[capture].u.piece_removal.walk;
    Flags const orig_flags_removed = move_effect_journal[capture].u.piece_removal.flags;

    TraceText("adjusting removal to actual victim, which may be different from planned victim\n");

    move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
    move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];
    recurse_into_child_ply();
    move_effect_journal[capture].u.piece_removal.walk = orig_walk_removed;
    move_effect_journal[capture].u.piece_removal.flags = orig_flags_removed;
  }

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

      update_taboo_arrival(+1);
      restart_from_scratch();
      update_taboo_arrival(-1);

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

  for (k = kstart; k<=kend && !end_of_iteration; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    while (!end_of_iteration
           && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
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

  for (k = vec_queen_start; k<=vec_queen_end && !end_of_iteration; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
    {
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

  for (k = kstart; k<=kend && !end_of_iteration; ++k)
  {
    move_effect_journal[movement].u.piece_movement.from = move_effect_journal[movement].u.piece_movement.to-vec[k];
    TraceSquare(move_effect_journal[movement].u.piece_movement.from);
    TraceEOL();

    if (is_square_empty(move_effect_journal[movement].u.piece_movement.from))
      done_fleshing_out_random_move_by_specific_invisible_to();
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

    if (!end_of_iteration)
    {
      SquareFlags const doublestepsq = side_moving==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

      move_effect_journal[movement].u.piece_movement.from -= dir;
      if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublestepsq)
          && is_square_empty(move_effect_journal[movement].u.piece_movement.from))
        done_fleshing_out_random_move_by_specific_invisible_to();
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

static void flesh_out_random_move_by_specific_invisible_to(square sq_arrival)
{
  PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceSquare(motivation[id].first.on);TraceEOL();

  if (motivation[id].first.on==sq_arrival)
  {
    TraceWalk(get_walk_of_piece_on_square(sq_arrival));TraceEOL();
    if (get_walk_of_piece_on_square(sq_arrival)==Dummy)
    {
      Side const side_playing = trait[nbply];
      consumption_type const save_consumption = current_consumption;
      piece_walk_type walk;

      reallocate_fleshing_out(side_playing);

      for (walk = Pawn; walk<=Bishop && !end_of_iteration; ++walk)
      {
        ++being_solved.number_of_pieces[side_playing][walk];
        replace_walk(sq_arrival,walk);
        flesh_out_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);
        replace_walk(sq_arrival,Dummy);
        --being_solved.number_of_pieces[side_playing][walk];
      }

      current_consumption = save_consumption;
    }
    else
      flesh_out_random_move_by_specific_invisible_to_according_to_walk(sq_arrival);
  }
  else
  {
    TraceText("the piece has already moved\n");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_time_in_history_when_to_place_victim(square sq_addition,
                                                      piece_walk_type walk_victim,
                                                      Flags flags_victim)
{
  Side const side = TSTFLAG(flags_victim,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_addition);
  TraceWalk(walk_victim);
  TraceValue("%x",flags_victim);
  TraceFunctionParamListEnd();

  --nbply;
  TraceValue("%u",nbply);TraceEOL();

  if (nbply==ply_retro_move)
  {
    TraceText("victim has been here \"forever\"\n");
    occupy_square(sq_addition,walk_victim,flags_victim);
    ++nbply;
    start_iteration();
    --nbply;
    empty_square(sq_addition);
  }
  else if (is_taboo(sq_addition,side))
  {
    /* end of recursion */
  }
  else
  {
    undo_move_effects();

    if (is_square_empty(sq_addition))
    {
      if (side==trait[nbply])
      {
        move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
        move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
        square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
        square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

        if (sq_departure==move_by_invisible
            && sq_arrival==move_by_invisible)
        {
          TraceText("victim moved here\n");
          occupy_square(sq_addition,walk_victim,flags_victim);
          flesh_out_random_move_by_specific_invisible_to(sq_addition);
          empty_square(sq_addition);
        }
      }

      find_time_in_history_when_to_place_victim(sq_addition,walk_victim,flags_victim);
    }
    else
    {
      /* end of recursion */
    }

    redo_move_effects();
  }

  ++nbply;
  TraceValue("%u",nbply);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void adapt_pre_capture_effect(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const pre_capture = effects_base;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[pre_capture].type==move_effect_piece_readdition)
  {
    if (move_effect_journal[pre_capture].u.piece_addition.added.on==to)
    {
      TraceText("capture of invisible victim added for the purpose\n");

      if (is_square_empty(to))
      {
        if (nbply<=flesh_out_move_highwater)
        {
          TraceText("victim was placed in previous iteration\n");
          adapt_capture_effect();
        }
        else
        {
          consumption_type const save_consumption = current_consumption;

          if (allocate_placement_of_claimed_fleshed_out(advers(trait[nbply])))
          {
            square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
            piece_walk_type const walk_added = move_effect_journal[pre_capture].u.piece_addition.added.walk;
            Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;

            assert(move_effect_journal[pre_capture].type==move_effect_piece_readdition);
            move_effect_journal[pre_capture].type = move_effect_none;
            find_time_in_history_when_to_place_victim(sq_addition,walk_added,flags_added);
            move_effect_journal[pre_capture].type = move_effect_piece_readdition;
          }

          current_consumption = save_consumption;
          TraceConsumption();TraceEOL();
        }
      }
      else
      {
        PieceIdType const id = GetPieceId(being_solved.spec[to]);
        motivation_type const save_motivation = motivation[id];

        TraceText("another total invisible has moved to the arrival square - "
                  "no need for addition any more!\n");
        move_effect_journal[pre_capture].type = move_effect_none;
        motivation[id].last.acts_when = nbply;
        motivation[id].last.purpose = purpose_victim;
        motivation[id].last.on = to;
        adapt_capture_effect();
        motivation[id] = save_motivation;
        move_effect_journal[pre_capture].type = move_effect_piece_readdition;
      }
    }
    else
    {
      square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
      piece_walk_type const walk_added = move_effect_journal[pre_capture].u.piece_addition.added.walk;
      TraceText("addition of a castling partner - must have happened at diagram setup time\n");
      if (!is_square_empty(sq_addition)
          && sq_addition==knowledge.pos
          && walk_added==knowledge.walk)
      {
        // TODO this will always be the case once we fully use our knowledge
        TraceText("castling partner was added as part of applying our knowledge\n");
        move_effect_journal[pre_capture].type = move_effect_none;
        adapt_capture_effect();
        move_effect_journal[pre_capture].type = move_effect_piece_readdition;
      }
      else if (was_taboo(sq_addition))
      {
        TraceText("Hmm - some invisible piece must have travelled through the castling partner's square\n");
        REPORT_DEADEND;
      }
      else
      {
        Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;
        Side const side_added = TSTFLAG(flags_added,White) ? White : Black;

        assert(move_effect_journal[pre_capture].type==move_effect_piece_readdition);
        move_effect_journal[pre_capture].type = move_effect_none;
        ++being_solved.number_of_pieces[side_added][walk_added];
        occupy_square(sq_addition,walk_added,flags_added);
        restart_from_scratch();
        empty_square(sq_addition);
        --being_solved.number_of_pieces[side_added][walk_added];
        move_effect_journal[pre_capture].type = move_effect_piece_readdition;
      }
    }
  }
  else
  {
    TraceText("no piece addition to be adapted\n");
    assert(move_effect_journal[pre_capture].type==move_effect_none);
    adapt_capture_effect();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_fleshing_out_random_move_by_invisible_from(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  update_taboo_arrival(+1);
  restart_from_scratch();
  update_taboo_arrival(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_accidental_capture_by_invisible(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  if (is_on_board(sq_arrival)
      && TSTFLAG(being_solved.spec[sq_arrival],advers(trait[nbply]))
      && TSTFLAG(being_solved.spec[sq_arrival],Chameleon))
  {
    PieceIdType const id = GetPieceId(being_solved.spec[sq_arrival]);

    TraceValue("%u",id);
    TraceValue("%u",motivation[id].last.purpose);
    TraceValue("%u",motivation[id].last.acts_when);
    TraceEOL();
    assert(motivation[id].first.purpose!=purpose_none);
    assert(motivation[id].last.purpose!=purpose_none);
    if (motivation[id].last.acts_when>nbply)
    {
      TraceText("the planned victim was added to later act from its current square\n");
      REPORT_DEADEND;
    }
    else
    {
      assert(move_effect_journal[capture].type==move_effect_no_piece_removal);
      move_effect_journal[capture].type = move_effect_piece_removal;
      move_effect_journal[capture].u.piece_removal.on = sq_arrival;
      move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(sq_arrival);
      move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[sq_arrival];

      done_fleshing_out_random_move_by_invisible_from();

      move_effect_journal[capture].type = move_effect_no_piece_removal;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_pawn_from(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  Side const side = trait[nbply];
  int const dir = side==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const sq_singlestep = move_effect_journal[movement].u.piece_movement.from+dir;
    TraceSquare(sq_singlestep);TraceEOL();
    if (is_square_empty(sq_singlestep))
    {
      if (!is_taboo(sq_singlestep,side))
      {
        move_effect_journal[movement].u.piece_movement.to = sq_singlestep;
        done_fleshing_out_random_move_by_invisible_from();
      }

      if (!end_of_iteration)
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
              move_effect_journal[movement].u.piece_movement.to = sq_doublestep;
              done_fleshing_out_random_move_by_invisible_from();
            }
          }
        }
      }
    }

    // TODO add capture victim if arrival square empty and nr_total...>0

    if (!end_of_iteration)
    {
      square const sq_arrival = sq_singlestep+dir_right;
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      flesh_out_accidental_capture_by_invisible();
    }

    if (!end_of_iteration)
    {
      square const sq_arrival = sq_singlestep+dir_left;
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      flesh_out_accidental_capture_by_invisible();
    }
  }


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_rider_from(vec_index_type kstart,
                                                          vec_index_type kend)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceValue("%u",kstart);
  TraceValue("%u",kend);
  TraceFunctionParamListEnd();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && !end_of_iteration; ++k)
  {
    square sq_arrival;
    for (sq_arrival = move_effect_journal[movement].u.piece_movement.from+vec[k];
         !end_of_iteration;
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
        if (!is_taboo(sq_arrival,trait[nbply]))
          done_fleshing_out_random_move_by_invisible_from();
      }
      else
      {
        if (!is_taboo(sq_arrival,trait[nbply]))
          flesh_out_accidental_capture_by_invisible();
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible_leaper_from(vec_index_type kstart,
                                                           vec_index_type kend)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;

  TraceFunctionEntry(__func__);
  TraceValue("%u",kstart);
  TraceValue("%u",kend);
  TraceFunctionParamListEnd();

  TraceWalk(get_walk_of_piece_on_square(sq_departure));TraceEOL();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && !end_of_iteration; ++k)
  {
    square const sq_arrival = sq_departure+vec[k];
    if (!is_taboo(sq_arrival,trait[nbply]))
    {
      move_effect_journal[movement].u.piece_movement.to = sq_arrival;
      /* just in case: */
      move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;

      if (is_square_empty(sq_arrival))
        done_fleshing_out_random_move_by_invisible_from();
      else
        flesh_out_accidental_capture_by_invisible();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_existing_invisible_from(square sq_departure)
{
  piece_walk_type const walk_on_square = get_walk_of_piece_on_square(sq_departure);
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const king_square_movement = movement+1;
  piece_walk_type const save_walk_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_flags_moving = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
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
      flesh_out_random_move_by_invisible_leaper_from(vec_queen_start,vec_queen_end);
      move_effect_journal[king_square_movement].type = move_effect_none;
      break;

    case Queen:
      flesh_out_random_move_by_invisible_rider_from(vec_queen_start,vec_queen_end);
      break;

    case Rook:
      flesh_out_random_move_by_invisible_rider_from(vec_rook_start,vec_rook_end);
      break;

    case Bishop:
      flesh_out_random_move_by_invisible_rider_from(vec_bishop_start,vec_bishop_end);
      break;

    case Knight:
      flesh_out_random_move_by_invisible_leaper_from(vec_knight_start,vec_knight_end);
      break;

    case Pawn:
      flesh_out_random_move_by_invisible_pawn_from();
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

static void flesh_out_random_move_by_specific_invisible_from(square sq_departure)
{
  Side const side_playing = trait[nbply];
  PieceIdType const id = GetPieceId(being_solved.spec[sq_departure]);
  piece_walk_type const walk_on_square = get_walk_of_piece_on_square(sq_departure);
  Flags const flags_on_square = being_solved.spec[sq_departure];
  motivation_type const save_motivation = motivation[id];
  Flags const save_flags = being_solved.spec[sq_departure];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  motivation[id].last.purpose = purpose_random_mover;
  motivation[id].last.acts_when = nbply;

  CLRFLAG(being_solved.spec[sq_departure],advers(side_playing));

  TraceWalk(walk_on_square);
  TraceValue("%x",flags_on_square);
  TraceEOL();

  // TODO use a sibling ply and the regular move generation machinery?

  if (walk_on_square==Dummy)
  {
    Side const side_playing = trait[nbply];
    Side const side_under_attack = advers(side_playing);
    square const king_pos = being_solved.king_square[side_under_attack];
    consumption_type const save_consumption = current_consumption;

    assert(play_phase==play_validating_mate);

    assert(TSTFLAG(being_solved.spec[sq_departure],side_playing));
    assert(!TSTFLAG(being_solved.spec[sq_departure],side_under_attack));

    reallocate_fleshing_out(side_playing);

    if (!end_of_iteration)
    {
      if (being_solved.king_square[side_playing]==initsquare)
      {
        being_solved.king_square[side_playing] = sq_departure;
        ++being_solved.number_of_pieces[side_playing][King];
        replace_walk(sq_departure,King);
        SETFLAG(being_solved.spec[sq_departure],Royal);
        if (!(king_pos!=initsquare && king_check_ortho(side_playing,king_pos)))
          flesh_out_random_move_by_existing_invisible_from(sq_departure);
        CLRFLAG(being_solved.spec[sq_departure],Royal);
        --being_solved.number_of_pieces[side_playing][King];
        being_solved.king_square[side_playing] = initsquare;
      }
    }

    if (!end_of_iteration)
    {
      SquareFlags const promsq = side_playing==White ? WhPromSq : BlPromSq;
      SquareFlags const basesq = side_playing==White ? WhBaseSq : BlBaseSq;
      if (!(TSTFLAG(sq_spec[sq_departure],basesq) || TSTFLAG(sq_spec[sq_departure],promsq)))
      {
        ++being_solved.number_of_pieces[side_playing][Pawn];
        replace_walk(sq_departure,Pawn);
        if (!(king_pos!=initsquare && pawn_check_ortho(side_playing,king_pos)))
          flesh_out_random_move_by_existing_invisible_from(sq_departure);
        --being_solved.number_of_pieces[side_playing][Pawn];
      }
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[side_playing][Knight];
      replace_walk(sq_departure,Knight);
      if (!(king_pos!=initsquare && knight_check_ortho(side_playing,king_pos)))
        flesh_out_random_move_by_existing_invisible_from(sq_departure);
      --being_solved.number_of_pieces[side_playing][Knight];
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[side_playing][Bishop];
      replace_walk(sq_departure,Bishop);
      if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                          vec_bishop_start,vec_bishop_end,
                                          Bishop))
        flesh_out_random_move_by_existing_invisible_from(sq_departure);
      --being_solved.number_of_pieces[side_playing][Bishop];
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[side_playing][Rook];
      replace_walk(sq_departure,Rook);
      if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                          vec_rook_start,vec_rook_end,
                                          Rook))
        flesh_out_random_move_by_existing_invisible_from(sq_departure);
      --being_solved.number_of_pieces[side_playing][Rook];
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[side_playing][Queen];
      replace_walk(sq_departure,Queen);
      if (!is_rider_check_uninterceptable(side_playing,king_pos,
                                          vec_queen_start,vec_queen_end,
                                          Queen))
        flesh_out_random_move_by_existing_invisible_from(sq_departure);
      --being_solved.number_of_pieces[side_playing][Queen];
    }

    current_consumption = save_consumption;
  }
  else
    flesh_out_random_move_by_existing_invisible_from(sq_departure);

  replace_walk(sq_departure,walk_on_square);
  being_solved.spec[sq_departure] = flags_on_square;

  being_solved.spec[sq_departure] = save_flags;

  motivation[id] = save_motivation;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean can_invisible_used_later_move(PieceIdType id)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",id);
  TraceFunctionParamListEnd();

  assert(motivation[id].last.acts_when>nbply);

  /* basically, invisibles used later as castling partners can *not* move */
  switch (motivation[id].last.purpose)
  {
    case purpose_interceptor:
    case purpose_attacker:
    case purpose_random_mover:
    case purpose_victim:
    case purpose_capturer:
      result = true;
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_there_random_move_until(ply when)
{
  boolean result = false;
  ply ply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",when);
  TraceFunctionParamListEnd();

  for (ply = nbply+2; ply<when; ply +=2)
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];
    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
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

static void flesh_out_random_move_by_specific_invisible_from_or_to(square pos,
                                                                   iteration_index_type if_inserted_since)
{
  Side const side_playing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(pos);
  TraceFunctionParam("%u",if_inserted_since);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[pos],Chameleon)
      && TSTFLAG(being_solved.spec[pos],side_playing))
  {
    PieceIdType const id = GetPieceId(being_solved.spec[pos]);

    TraceValue("%u",id);
    TraceValue("%u",motivation[id].first.purpose);
    TraceValue("%u",motivation[id].first.acts_when);
    TraceValue("%u",motivation[id].last.purpose);
    TraceValue("%u",motivation[id].last.acts_when);
    TraceValue("%u",motivation[id].insertion_iteration);
    TraceEOL();
    assert(motivation[id].first.purpose!=purpose_none);
    assert(motivation[id].last.purpose!=purpose_none);

    if (motivation[id].last.acts_when<=nbply)
    {
      /* piece was used on pos - fleshing out moves *from* pos */
      if (motivation[id].insertion_iteration>=if_inserted_since)
        flesh_out_random_move_by_specific_invisible_from(pos);
    }
    else if (motivation[id].first.acts_when>nbply)
    {
      /* piece will be used on pos - fleshing out moves *to* pos */
      if (can_invisible_used_later_move(id))
      {
        if (is_there_random_move_until(motivation[id].first.acts_when))
        {
          /* let posteriority act first, possibly unmoving piece to pos "first",
           * but remember what we are at */
          committed_to_fleshing_out_random_move_by[nbply] = id;
          recurse_into_child_ply();
          committed_to_fleshing_out_random_move_by[nbply] = NullPieceId;
        }
        else
          flesh_out_random_move_by_specific_invisible_to(pos);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_random_move_by_invisible(square first_taboo_violation)
{
  TraceFunctionEntry(__func__);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  TraceValue("%u",committed_to_fleshing_out_random_move_by[nbply]);
  TraceEOL();

  if (committed_to_fleshing_out_random_move_by[nbply]==NullPieceId)
  {
    iteration_index_type const save_last_time = fleshed_out_random_move_last_time[nbply];

    fleshed_out_random_move_last_time[nbply] = current_iteration;

    if (first_taboo_violation==nullsquare)
    {
      square const *s;

      for (s = boardnum; *s && !end_of_iteration; ++s)
        flesh_out_random_move_by_specific_invisible_from_or_to(*s,save_last_time);

      TraceText("random move by unplaced invisible\n");
      // TODO Strictly speaking, there is no guarantee that such a move exists
      // but we probably save a lot of time by not fleshing it out. As long as we
      // restrict ourselves to h#n, the risk is printing some wrong cooks.
      // Options:
      // * find out how hight the cost would be
      // * fleshing it out
      // * option for activating fleshing out
      {
        consumption_type const save_consumption = current_consumption;

        current_consumption.claimed[trait[nbply]] = true;
        TraceConsumption();TraceEOL();

        if (nr_total_invisbles_consumed()<=total_invisible_number)
          recurse_into_child_ply();

        current_consumption = save_consumption;
        TraceConsumption();TraceEOL();
      }
    }
    else
      flesh_out_random_move_by_specific_invisible_from_or_to(first_taboo_violation,save_last_time);

    fleshed_out_random_move_last_time[nbply] = save_last_time;
  }
  else
  {
    PieceIdType const id = committed_to_fleshing_out_random_move_by[nbply];

    /* we are committed to only fleshing out moves by one piece to the square where
     * it will later fulfill its purpose */
    if (first_taboo_violation==nullsquare
        || motivation[id].first.on==first_taboo_violation)
    {
      assert(can_invisible_used_later_move(id));
      if (is_there_random_move_until(motivation[id].first.acts_when))
        /* let posteriority act first, possibly unmoving piece to pos "first" */
        recurse_into_child_ply();
      else
        flesh_out_random_move_by_specific_invisible_to(motivation[id].first.on);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_inserted_invisible(piece_walk_type walk_capturing,
                                                    square sq_departure)
{
  Side const side_playing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (!was_taboo(sq_departure) && !is_taboo(sq_departure,side_playing))
  {
    consumption_type const save_consumption = current_consumption;
    if (allocate_placement_of_claimed_fleshed_out(side_playing))
    {
      move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
      move_effect_journal_index_type const precapture = effects_base;
      move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

      Flags const flags = move_effect_journal[precapture].u.piece_addition.added.flags;
      Side const side_in_check = trait[nbply-1];
      square const king_pos = being_solved.king_square[side_in_check];
      PieceIdType const id = GetPieceId(flags);
      motivation_type const save_motivation = motivation[id];

      TraceConsumption();TraceEOL();
      assert(nr_total_invisbles_consumed()<=total_invisible_number);

      /* adding the total invisible in the pre-capture effect sounds tempting, but
       * we have to make sure that there was no illegal check from it before this
       * move!
       * NB: this works with illegal checks both from the inserted piece and to
       * the inserted king (afert we restart_from_scratch()).
       */
      assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
      move_effect_journal[precapture].type = move_effect_none;

      /* these were set in regular play already: */
      assert(motivation[id].first.acts_when==nbply);
      assert(motivation[id].first.purpose==purpose_capturer);
      assert(motivation[id].last.acts_when==nbply);
      assert(motivation[id].last.purpose==purpose_capturer);
      /* fill in the rest: */
      motivation[id].insertion_iteration = current_iteration;
      motivation[id].first.on = sq_departure;
      motivation[id].last.on = sq_departure;

      ++being_solved.number_of_pieces[side_playing][walk_capturing];
      occupy_square(sq_departure,walk_capturing,flags);

      if (!is_square_uninterceptably_attacked(side_in_check,king_pos))
      {
        assert(!TSTFLAG(being_solved.spec[sq_departure],advers(trait[nbply])));

        move_effect_journal[movement].u.piece_movement.from = sq_departure;
        /* move_effect_journal[movement].u.piece_movement.to unchanged from regular play */
        move_effect_journal[movement].u.piece_movement.moving = walk_capturing;
        move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

        update_taboo_arrival(+1);
        restart_from_scratch();
        update_taboo_arrival(-1);
      }

      empty_square(sq_departure);
      --being_solved.number_of_pieces[side_playing][walk_capturing];

      motivation[id] = save_motivation;

      move_effect_journal[precapture].type = move_effect_piece_readdition;

      TraceConsumption();TraceEOL();
    }

    current_consumption = save_consumption;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_fleshing_out_capture_by_existing_invisible(piece_walk_type walk_capturing,
                                                            square sq_departure)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  move_effect_journal[movement].u.piece_movement.from = sq_departure;
  /* move_effect_journal[movement].u.piece_movement.to unchanged from regular play */
  move_effect_journal[movement].u.piece_movement.moving = walk_capturing;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_departure];

  update_taboo_arrival(+1);
  restart_from_scratch();
  update_taboo_arrival(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_existing_invisible(piece_walk_type walk_capturing,
                                                    square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],Chameleon));
  TraceValue("%u",TSTFLAG(being_solved.spec[sq_departure],trait[nbply]));
  TraceWalk(get_walk_of_piece_on_square(sq_departure));
  TraceEOL();

  if (TSTFLAG(being_solved.spec[sq_departure],Chameleon)
      && TSTFLAG(being_solved.spec[sq_departure],trait[nbply]))
  {
    PieceIdType const id = GetPieceId(being_solved.spec[sq_departure]);

    TraceValue("%u",id);
    TraceValue("%u",motivation[id].last.purpose);
    TraceValue("%u",motivation[id].last.acts_when);
    TraceEOL();
    assert(motivation[id].first.purpose!=purpose_none);
    assert(motivation[id].last.purpose!=purpose_none);
    if (motivation[id].last.acts_when>nbply)
    {
      TraceText("the piece was added to later act from its current square\n");
      REPORT_DEADEND;
    }
    else
    {
      move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
      move_effect_journal_index_type const precapture = effects_base;
      motivation_type const save_motivation = motivation[id];

      /* deactivate the pre-capture insertion of the moving total invisible since
       * that piece is already on the board
       */
      assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
      move_effect_journal[precapture].type = move_effect_none;

      motivation[id].last.purpose = purpose_capturer;
      motivation[id].last.acts_when = nbply;

      if (get_walk_of_piece_on_square(sq_departure)==walk_capturing)
      {
        assert(!TSTFLAG(being_solved.spec[sq_departure],advers(trait[nbply])));
        done_fleshing_out_capture_by_existing_invisible(walk_capturing,sq_departure);
      }
      else if (get_walk_of_piece_on_square(sq_departure)==Dummy)
      {
        Side const side_in_check = trait[nbply-1];
        square const king_pos = being_solved.king_square[side_in_check];
        Flags const save_flags = being_solved.spec[sq_departure];
        consumption_type const save_consumption = current_consumption;

        reallocate_fleshing_out(trait[nbply]);

        ++being_solved.number_of_pieces[trait[nbply]][walk_capturing];
        replace_walk(sq_departure,walk_capturing);
        CLRFLAG(being_solved.spec[sq_departure],advers(trait[nbply]));

        if (!is_square_uninterceptably_attacked(side_in_check,king_pos))
          done_fleshing_out_capture_by_existing_invisible(walk_capturing,sq_departure);

        being_solved.spec[sq_departure] = save_flags;
        replace_walk(sq_departure,Dummy);
        --being_solved.number_of_pieces[trait[nbply]][walk_capturing];

        current_consumption = save_consumption;
      }

      move_effect_journal[precapture].type = move_effect_piece_readdition;

      motivation[id] = save_motivation;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_rider(piece_walk_type walk_rider,
                                                 vec_index_type kcurr, vec_index_type kend,
                                                 square first_taboo_violation)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  TraceSquare(sq_arrival);TraceEOL();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square sq_departure;
    for (sq_departure = sq_arrival+vec[kcurr];
         is_square_empty(sq_departure) && !end_of_iteration;
         sq_departure += vec[kcurr])
      if (first_taboo_violation==nullsquare)
        flesh_out_capture_by_inserted_invisible(walk_rider,sq_departure);

    if (!end_of_iteration)
      if (first_taboo_violation==nullsquare || first_taboo_violation==sq_departure)
        flesh_out_capture_by_existing_invisible(walk_rider,sq_departure);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_king(square first_taboo_violation)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const precapture = effects_base;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  move_effect_journal_index_type const king_square_movement = movement+1;
  vec_index_type kcurr;

  TraceFunctionEntry(__func__);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
  assert(!TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal));
  assert(move_effect_journal[king_square_movement].type==move_effect_none);

  for (kcurr = vec_queen_start; kcurr<=vec_queen_end && !end_of_iteration; ++kcurr)
  {
    square const sq_departure = sq_arrival+vec[kcurr];
    if (first_taboo_violation==nullsquare || first_taboo_violation==sq_departure)
    {
      move_effect_journal[king_square_movement].type = move_effect_king_square_movement;
      move_effect_journal[king_square_movement].u.king_square_movement.from = sq_departure;
      move_effect_journal[king_square_movement].u.king_square_movement.to = sq_arrival;
      move_effect_journal[king_square_movement].u.king_square_movement.side = trait[nbply];

      if (get_walk_of_piece_on_square(sq_departure)==King
          && sq_departure==being_solved.king_square[trait[nbply]])
      {
        assert(TSTFLAG(being_solved.spec[sq_departure],Royal));
        flesh_out_capture_by_existing_invisible(King,sq_departure);
      }
      else if (being_solved.king_square[trait[nbply]]==initsquare)
      {
        being_solved.king_square[trait[nbply]] = sq_departure;

        if (is_square_empty(sq_departure))
        {
          assert(!TSTFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal));
          SETFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal);
          flesh_out_capture_by_inserted_invisible(King,sq_departure);
          CLRFLAG(move_effect_journal[precapture].u.piece_addition.added.flags,Royal);
        }
        else if (get_walk_of_piece_on_square(sq_departure)==Dummy)
        {
          assert(!TSTFLAG(being_solved.spec[sq_departure],Royal));
          SETFLAG(being_solved.spec[sq_departure],Royal);
          flesh_out_capture_by_existing_invisible(King,sq_departure);
          CLRFLAG(being_solved.spec[sq_departure],Royal);
        }

        being_solved.king_square[trait[nbply]] = initsquare;
      }

      move_effect_journal[king_square_movement].type = move_effect_none;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_leaper(piece_walk_type walk_leaper,
                                                  vec_index_type kcurr, vec_index_type kend,
                                                  square first_taboo_violation)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square const sq_departure = sq_arrival+vec[kcurr];
    if (first_taboo_violation==nullsquare || first_taboo_violation==sq_departure)
    {
      if (is_square_empty(sq_departure))
        flesh_out_capture_by_inserted_invisible(walk_leaper,sq_departure);
      else
        flesh_out_capture_by_existing_invisible(walk_leaper,sq_departure);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_pawn(square first_taboo_violation)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  int const dir_vert = trait[nbply]==White ? -dir_up : -dir_down;
  SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  // TODO en passant capture

  if (!end_of_iteration)
  {
    square sq_departure = sq_capture+dir_vert+dir_left;
    if (first_taboo_violation==nullsquare || first_taboo_violation==sq_departure)
    {
      if (!TSTFLAG(sq_spec[sq_departure],basesq) && !TSTFLAG(sq_spec[sq_departure],promsq))
      {
        if (is_square_empty(sq_departure))
          flesh_out_capture_by_inserted_invisible(Pawn,sq_departure);
        else
          flesh_out_capture_by_existing_invisible(Pawn,sq_departure);
      }
    }
  }

  if (!end_of_iteration)
  {
    square sq_departure = sq_capture+dir_vert+dir_right;
    if (first_taboo_violation==nullsquare || first_taboo_violation==sq_departure)
    {
      if (!TSTFLAG(sq_spec[sq_departure],basesq) && !TSTFLAG(sq_spec[sq_departure],promsq))
      {
        if (is_square_empty(sq_departure))
          flesh_out_capture_by_inserted_invisible(Pawn,sq_departure);
        else
          flesh_out_capture_by_existing_invisible(Pawn,sq_departure);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_walk_by_walk(square first_taboo_violation)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const save_from = move_effect_journal[movement].u.piece_movement.from;
  piece_walk_type const save_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;

  TraceFunctionEntry(__func__);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  flesh_out_capture_by_invisible_king(first_taboo_violation);
  TraceValue("%u",end_of_iteration);TraceEOL();
  flesh_out_capture_by_invisible_pawn(first_taboo_violation);
  TraceValue("%u",end_of_iteration);TraceEOL();
  flesh_out_capture_by_invisible_leaper(Knight,vec_knight_start,vec_knight_end,first_taboo_violation);
  TraceValue("%u",end_of_iteration);TraceEOL();
  flesh_out_capture_by_invisible_rider(Bishop,vec_bishop_start,vec_bishop_end,first_taboo_violation);
  TraceValue("%u",end_of_iteration);TraceEOL();
  flesh_out_capture_by_invisible_rider(Rook,vec_rook_start,vec_rook_end,first_taboo_violation);
  TraceValue("%u",end_of_iteration);TraceEOL();
  flesh_out_capture_by_invisible_rider(Queen,vec_queen_start,vec_queen_end,first_taboo_violation);

  move_effect_journal[movement].u.piece_movement.from = save_from;
  move_effect_journal[movement].u.piece_movement.moving = save_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible(square first_taboo_violation)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];

  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  piece_walk_type const save_removed_walk = move_effect_journal[capture].u.piece_removal.walk;
  Flags const save_removed_spec = move_effect_journal[capture].u.piece_removal.flags;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceSquare(first_taboo_violation);
  TraceFunctionParamListEnd();

  TraceSquare(sq_capture);TraceEOL();
  assert(!is_square_empty(sq_capture));

  move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(sq_capture);
  move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[sq_capture];

  flesh_out_capture_by_invisible_walk_by_walk(first_taboo_violation);

  move_effect_journal[capture].u.piece_removal.walk = save_removed_walk;
  move_effect_journal[capture].u.piece_removal.flags = save_removed_spec;

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

static square find_taboo_violation(void)
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

      if (sq_departure!=move_by_invisible
          && sq_departure<capture_by_invisible)
      {
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
    }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_taboo_violation_acceptable(square first_taboo_violation)
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
    TraceValue("%u",motivation[id].insertion_iteration);
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
      REPORT_DEADEND;
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
    square const first_taboo_violation = find_taboo_violation();

    TraceValue("%u",nbply);
    TraceValue("%u",top_ply_of_regular_play);
    TraceSquare(sq_departure);
    TraceValue("%u",sq_departure);
    TraceValue("%u",capture_by_invisible);
    TraceValue("%u",flesh_out_move_highwater);
    TraceSquare(sq_arrival);
    TraceValue("%u",sq_arrival);
    TraceValue("%u",move_by_invisible);
    TraceEOL();

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
      flesh_out_random_move_by_invisible(first_taboo_violation);
    else if (nbply<=flesh_out_move_highwater)
    {
      if (first_taboo_violation==nullsquare)
        adapt_pre_capture_effect();
    }
    else if (sq_departure>=capture_by_invisible
             && is_on_board(sq_arrival))
      flesh_out_capture_by_invisible(first_taboo_violation);
    else if (first_taboo_violation==nullsquare)
      adapt_pre_capture_effect();
    else
      assert(is_taboo_violation_acceptable(first_taboo_violation));
  }
  else
    validate_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_any_walk(Side side,
                                      square pos,
                                      piece_walk_type walk)
{
  Flags spec = BIT(side)|BIT(Chameleon);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  ++being_solved.number_of_pieces[side][walk];

  SetPieceId(spec,next_invisible_piece_id);
  occupy_square(pos,walk,spec);
  if (walk==King)
  {
    SETFLAG(being_solved.spec[pos],Royal);
    {
      Side const side_attacked = side;
      // TODO use pos instead of king_pos (or assert that they are equal)
      square const king_pos = being_solved.king_square[side_attacked];
      vec_index_type const k = is_square_uninterceptably_attacked(side_attacked,
                                                                  king_pos);
      if (k==0)
        restart_from_scratch();
    }
  }
  else
  {
    Side const side_attacked = advers(side);
    square const king_pos = being_solved.king_square[side_attacked];
    vec_index_type const k = is_square_uninterceptably_attacked(side_attacked,
                                                                king_pos);
    if (k==0 || king_pos+vec[k]!=pos)
      restart_from_scratch();
  }
  TraceWalk(get_walk_of_piece_on_square(pos));
  TraceWalk(walk);
  TraceEOL();
  assert(get_walk_of_piece_on_square(pos)==walk);
  empty_square(pos);
  --being_solved.number_of_pieces[side][walk];

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_pawn(Side side, square pos)
{
  SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  if (!(TSTFLAG(sq_spec[pos],basesq) || TSTFLAG(sq_spec[pos],promsq)))
    walk_interceptor_any_walk(side,pos,Pawn);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_king(Side side, square pos)
{
  consumption_type const save_consumption = current_consumption;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  being_solved.king_square[side] = pos;

  if (allocate_placement_of_claimed_fleshed_out(side))
    walk_interceptor_any_walk(side,pos,King);

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

  being_solved.king_square[side] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor(Side side, square pos)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  TraceSquare(pos);
  TraceSquare(being_solved.king_square[side]);
  TraceEOL();
  assert(is_square_empty(pos));

  if (being_solved.king_square[side]==initsquare)
    walk_interceptor_king(side,pos);

  {
    piece_walk_type walk;
    consumption_type const save_consumption = current_consumption;

    if (allocate_placement_of_claimed_fleshed_out(side))
    {
      if (!end_of_iteration)
        walk_interceptor_pawn(side,pos);

      for (walk = Queen; walk<=Bishop && !end_of_iteration; ++walk)
        walk_interceptor_any_walk(side,pos,walk);
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_interceptor(vec_index_type kcurr,
                               square pos,
                               piece_walk_type const walk_at_end)
{
  Side const preferred_side = trait[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceSquare(pos);
  TraceWalk(walk_at_end);
  TraceFunctionParamListEnd();

  if (!is_taboo(pos,preferred_side))
    walk_interceptor(preferred_side,pos);

  if (!end_of_iteration)
  {
    if (!is_taboo(pos,advers(preferred_side)))
      walk_interceptor(advers(preferred_side),pos);
  }

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef void intercept_checks_fct(vec_index_type kcurr);

static void place_interceptor_of_side_on_square(vec_index_type kcurr,
                                                square s,
                                                piece_walk_type const walk_at_end,
                                                intercept_checks_fct *recurse,
                                                Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceSquare(s);
  TraceWalk(walk_at_end);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if (!is_taboo(s,side))
  {
    consumption_type const save_consumption = current_consumption;

    TraceSquare(s);TraceEnumerator(Side,trait[nbply-1]);TraceEOL();

    CLRFLAG(being_solved.spec[s],advers(side));

    if (allocate_placement_of_claimed_not_fleshed_out(side))
      (*recurse)(kcurr+1);
    current_consumption = save_consumption;

    SETFLAG(being_solved.spec[s],advers(side));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_square(vec_index_type kcurr,
                                        square s,
                                        piece_walk_type const walk_at_end,
                                        intercept_checks_fct *recurse)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceSquare(s);
  TraceWalk(walk_at_end);
  TraceFunctionParamListEnd();

  motivation[next_invisible_piece_id].first.on = s;
  motivation[next_invisible_piece_id].last.on = s;

  if (play_phase==play_validating_mate)
  {
    Flags spec = BIT(White)|BIT(Black)|BIT(Chameleon);

    SetPieceId(spec,next_invisible_piece_id);
    occupy_square(s,Dummy,spec);

    place_interceptor_of_side_on_square(kcurr,s,walk_at_end,recurse,White);

    if (!end_of_iteration)
      place_interceptor_of_side_on_square(kcurr,s,walk_at_end,recurse,Black);

    TraceConsumption();TraceEOL();

    empty_square(s);
  }
  else
    colour_interceptor(kcurr,s,walk_at_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_line(vec_index_type kcurr,
                                      piece_walk_type const walk_at_end,
                                      intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceWalk(walk_at_end);
  TraceFunctionParamListEnd();

  ++next_invisible_piece_id;

  TraceValue("%u",next_invisible_piece_id);TraceEOL();
  assert(motivation[next_invisible_piece_id].last.purpose==purpose_none);
  motivation[next_invisible_piece_id].insertion_iteration = current_iteration;
  motivation[next_invisible_piece_id].first.purpose = purpose_interceptor;
  motivation[next_invisible_piece_id].first.acts_when = nbply;
  motivation[next_invisible_piece_id].last.purpose = purpose_interceptor;
  motivation[next_invisible_piece_id].last.acts_when = nbply;

  {
    square s;
    for (s = king_pos+vec[kcurr];
         is_square_empty(s) && !end_of_iteration;
         s += vec[kcurr])
    {
      TraceSquare(s);
      TraceValue("%u",taboo[White][s]);
      TraceValue("%u",taboo[Black][s]);
      TraceEOL();
      if (!was_taboo(s))
        place_interceptor_on_square(kcurr,s,walk_at_end,recurse);
    }
    TraceSquare(s);
    TraceValue("%u",end_of_iteration);
    TraceEOL();
  }

  motivation[next_invisible_piece_id].last.purpose = purpose_none;
  --next_invisible_piece_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_line_if_check(vec_index_type kcurr,
                                    piece_walk_type walk_rider,
                                    intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];
  int const dir = vec[kcurr];
  square const sq_end = find_end_of_line(king_pos,dir);
  piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,side_in_check);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceSquare(sq_end);
  TraceSquare(king_pos+dir);
  TraceWalk(walk_at_end);
  TraceEOL();

  if ((walk_at_end==walk_rider || walk_at_end==Queen)
      && TSTFLAG(being_solved.spec[sq_end],side_checking))
  {
    TraceConsumption();;TraceEOL();
    if (sq_end!=king_pos+dir
        && nr_placeable_invisibles_for_both_sides()>0)
      place_interceptor_on_line(kcurr,walk_at_end,recurse);
  }
  else
    (*recurse)(kcurr+1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks_diagonal(vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_bishop_end)
    done_intercepting_illegal_checks();
  else
    intercept_line_if_check(kcurr,Bishop,&intercept_illegal_checks_diagonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks_orthogonal(vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_rook_end)
    intercept_illegal_checks_diagonal(vec_bishop_start);
  else
    intercept_line_if_check(kcurr,Rook,&intercept_illegal_checks_orthogonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks(void)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (king_pos==initsquare)
    done_intercepting_illegal_checks();
  else if (!is_square_attacked_by_uninterceptable(side_in_check,king_pos))
    intercept_illegal_checks_orthogonal(vec_rook_start);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_iteration(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++current_iteration;

  intercept_illegal_checks();

  assert(current_iteration>0);
  --current_iteration;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacker(Side side_attacking,
                                        square s,
                                        piece_walk_type walk)
{
  consumption_type const save_consumption = current_consumption;
  Flags spec = BIT(side_attacking)|BIT(Chameleon);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  if (allocate_placement_of_claimed_fleshed_out(side_attacking))
  {
    ++being_solved.number_of_pieces[side_attacking][walk];
    SetPieceId(spec,++next_invisible_piece_id);
    assert(motivation[next_invisible_piece_id].last.purpose==purpose_none);
    motivation[next_invisible_piece_id].insertion_iteration = current_iteration;
    motivation[next_invisible_piece_id].first.purpose = purpose_attacker;
    motivation[next_invisible_piece_id].first.acts_when = top_ply_of_regular_play;
    motivation[next_invisible_piece_id].first.on = s;
    motivation[next_invisible_piece_id].last.purpose = purpose_attacker;
    motivation[next_invisible_piece_id].last.acts_when = top_ply_of_regular_play;
    motivation[next_invisible_piece_id].last.on = s;
    TraceValue("%u",next_invisible_piece_id);
    TraceValue("%u",motivation[next_invisible_piece_id].last.purpose);
    TraceEOL();
    occupy_square(s,walk,spec);
    start_iteration();
    empty_square(s);
    motivation[next_invisible_piece_id].last.purpose = purpose_none;
    --next_invisible_piece_id;
    --being_solved.number_of_pieces[side_attacking][walk];
  }
  else
  {
    assert(0);
  }

  current_consumption = save_consumption;
  TraceConsumption();TraceEOL();

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

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square s;
    for (s = sq_mating_piece+vec[kcurr];
         !is_square_blocked(s) && !end_of_iteration;
         s += vec[kcurr])
      if (is_square_empty(s))
      {
        TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
        if (taboo[side_attacking][s]==0)
          place_mating_piece_attacker(side_attacking,s,walk_rider);
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

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square const s = sq_mating_piece+vec[kcurr];
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
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

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_left;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_right;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void attack_mating_piece(Side side_attacking,
                                square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(sq_mating_piece);
  TraceFunctionParamListEnd();

  /* We are defending against a verified mate. If we can't attack the mate,
   * we are dealing with a solution. */
  combined_result = previous_move_has_solved;

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Bishop,
                                     vec_bishop_start,vec_bishop_end);

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Rook,
                                     vec_rook_start,vec_rook_end);

  place_mating_piece_attacking_leaper(side_attacking,
                                      sq_mating_piece,
                                      Knight,
                                      vec_knight_start,vec_knight_end);

  place_mating_piece_attacking_pawn(side_attacking,sq_mating_piece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_mate(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[top_ply_of_regular_play];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",top_ply_of_regular_play);
  TraceSquare(sq_departure);
  TraceValue("%u",sq_departure);
  TraceSquare(sq_arrival);
  TraceValue("%u",sq_arrival);
  TraceValue("%u",move_by_invisible);
  TraceEOL();

  if (sq_departure==move_by_invisible
      && sq_arrival==move_by_invisible)
    combined_validation_result = mate_defendable_by_interceptors;
  else
  {
    combined_validation_result = mate_unvalidated;
    combined_result = previous_move_is_illegal;
    end_of_iteration = false;
    start_iteration();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_mate(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",combined_result);
  TraceValue("%u",combined_validation_result);
  TraceEOL();

  switch (combined_validation_result)
  {
    case mate_unvalidated:
      assert(combined_result==previous_move_is_illegal);
      break;

    case no_mate:
      assert(combined_result==previous_move_has_not_solved);
      break;

    case mate_attackable:
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      attack_mating_piece(advers(trait[top_ply_of_regular_play]),sq_mating_piece_to_be_attacked);
      break;

    case mate_defendable_by_interceptors:
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      start_iteration();
      break;

    case mate_with_2_uninterceptable_doublechecks:
      /* we only replay moves for TI revelation */
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      start_iteration();
      assert(combined_result==previous_move_has_solved);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (nbply!=ply_retro_move)
  {
    assert(nr_total_invisbles_consumed()<=total_invisible_number);
    undo_move_effects();
    --nbply;
  }

  ++nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unrewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --nbply;

  while (nbply!=top_ply_of_regular_play)
  {
    ++nbply;
    redo_move_effects();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void apply_knowledge(void (*next_step)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(knowledge.pos);
  TraceEOL();

  if (knowledge.pos==initsquare)
    (*next_step)();
  else
  {
    consumption_type const save_consumption = current_consumption;
    square const s = knowledge.pos;
    Side const side = TSTFLAG(knowledge.spec,White) ? White : Black;

    TraceWalk(knowledge.walk);
    TraceValue("%x",knowledge.spec);
    TraceEnumerator(Side,side);
    TraceEOL();

    if ((knowledge.is_allocated
         || allocate_placement_of_claimed_not_fleshed_out(side))
        && !is_taboo(s,side))
    {
      ++next_invisible_piece_id;
      TraceValue("%u",next_invisible_piece_id);
      TraceEOL();

      assert(is_square_empty(knowledge.pos));
      assert(TSTFLAG(knowledge.spec,Chameleon));
      motivation[next_invisible_piece_id].first.purpose = purpose_interceptor;
      motivation[next_invisible_piece_id].first.acts_when = 0;
      motivation[next_invisible_piece_id].first.on = s;
      motivation[next_invisible_piece_id].last.purpose = purpose_interceptor;
      motivation[next_invisible_piece_id].last.acts_when = 0;
      motivation[next_invisible_piece_id].last.on = s;
      ++being_solved.number_of_pieces[side][knowledge.walk];
      occupy_square(knowledge.pos,knowledge.walk,knowledge.spec);
      SetPieceId(being_solved.spec[s],next_invisible_piece_id);

      if (TSTFLAG(being_solved.spec[s],Royal))
      {
        TraceSquare(being_solved.king_square[side]);
        TraceWalk(get_walk_of_piece_on_square(being_solved.king_square[side]));
        TraceEOL();
        assert(being_solved.king_square[side]==initsquare);
        being_solved.king_square[side] = s;
        (*next_step)();
        being_solved.king_square[side] = initsquare;
      }
      else
        (*next_step)();

      assert(s==knowledge.pos);
      empty_square(knowledge.pos);
      --being_solved.number_of_pieces[side][knowledge.walk];

      motivation[next_invisible_piece_id].last.purpose = purpose_none;

      --next_invisible_piece_id;
    }
    else
    {
      combined_result = previous_move_has_not_solved;
      TraceText("allocation for application of knowledge not possible\n");
      REPORT_DEADEND;
    }

    current_consumption = save_consumption;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void make_revelations(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  update_taboo(+1);

  top_ply_of_regular_play = nbply;
  setup_revelations();
  play_phase = play_rewinding;
  rewind_effects();
  play_phase = play_detecting_revelations;
  end_of_iteration = false;
  apply_knowledge(&start_iteration);
  play_phase = play_unwinding;
  unrewind_effects();
  play_phase = play_regular;

  update_taboo(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_and_test(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  play_phase = play_validating_mate;
  validate_mate();
  play_phase = play_testing_mate;
  test_mate();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply-ply_retro_move);TraceEOL();

  update_taboo(+1);

  /* necessary for detecting checks by pawns and leapers */
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else
  {
    /* make sure that our length corresponds to the length of the tested move sequence
     * (which may vary if STFindShortest is used)
     */
    assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
    slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

    TraceValue("%u",SLICE_NEXT2(si));
    TraceValue("%u",slices[SLICE_NEXT2(si)].u.branch.length);
    TraceEOL();

    top_ply_of_regular_play = nbply;
    tester_slice = si;

    top_before_relevations[nbply] = move_effect_journal_base[nbply+1];

    play_phase = play_rewinding;
    rewind_effects();

    apply_knowledge(&validate_and_test);

    play_phase = play_unwinding;
    unrewind_effects();
    play_phase = play_regular;

    solve_result = combined_result==immobility_on_next_move ? previous_move_has_not_solved : combined_result;
  }

  update_taboo(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_reveal_after_mating_move(slice_index si)
{
  consumption_type const save_consumption = current_consumption;
  PieceIdType const save_next_invisible_piece_id = next_invisible_piece_id;
  knowledge_type const save_knowledge = knowledge;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  make_revelations();

  if (!revelation_status_is_uninitialised)
    evaluate_revelations();

  pipe_solve_delegate(si);

  knowledge = save_knowledge;
  current_consumption = save_consumption;
  next_invisible_piece_id = save_next_invisible_piece_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_move_still_playable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side = SLICE_STARTER(si);
    move_effect_journal_index_type const base = move_effect_journal_base[ply_replayed];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture;

    TraceValue("%u",ply_replayed);
    TraceSquare(sq_departure);
    TraceSquare(sq_arrival);
    TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture);
    TraceEOL();

    // TODO redo (and undo) the pre-capture effect?

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
    {
      TraceText("we assume that un-fleshed-out random moves by total invisibles are always playable\n");
      result = true;
    }
    else
    {
      assert(TSTFLAG(being_solved.spec[sq_departure],side));

      if (sq_capture==queenside_castling)
      {
        SETCASTLINGFLAGMASK(side,ra_cancastle);
        generate_moves_for_piece(sq_departure);
        CLRCASTLINGFLAGMASK(side,ra_cancastle);
      }
      else if (sq_capture==kingside_castling)
      {
        SETCASTLINGFLAGMASK(side,rh_cancastle);
        generate_moves_for_piece(sq_departure);
        CLRCASTLINGFLAGMASK(side,rh_cancastle);
      }
      else
        generate_moves_for_piece(sq_departure);

      {
        numecoup start = MOVEBASE_OF_PLY(nbply);
        numecoup i;
        numecoup new_top = start;
        for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
        {
          assert(move_generation_stack[i].departure==sq_departure);
          if (move_generation_stack[i].arrival==sq_arrival)
          {
            ++new_top;
            move_generation_stack[new_top] = move_generation_stack[i];
            break;
          }
        }

        SET_CURRMOVE(nbply,new_top);
      }

      result = CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void copy_move_effects(void)
{
  move_effect_journal_index_type const replayed_base = move_effect_journal_base[ply_replayed];
  move_effect_journal_index_type const replayed_top = move_effect_journal_base[ply_replayed+1];
  move_effect_journal_index_type replayed_curr;
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_replayed);
  TraceValue("%u",move_effect_journal_base[ply_replayed]);
  TraceValue("%u",move_effect_journal_base[ply_replayed+1]);
  TraceValue("%u",nbply);
  TraceEOL();

  move_effect_journal_base[nbply+1] += (replayed_top-replayed_base);
  curr = move_effect_journal_base[nbply+1];
  replayed_curr = replayed_top;

  while (replayed_curr>replayed_base)
    move_effect_journal[--curr] = move_effect_journal[--replayed_curr];

  assert(curr==move_effect_journal_base[nbply]);

  TraceValue("%u",move_effect_journal_base[nbply]);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_move_repeater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));

  if (!(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture==kingside_castling
        || move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture==queenside_castling)
      || is_move_still_playable(si))
  {
    copy_move_effects();
    redo_move_effects();
    ++ply_replayed;
    pipe_solve_delegate(si);
    --ply_replayed;
    undo_move_effects();
  }
  else
    solve_result = previous_move_is_illegal;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO separate slice type for update taboo?
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else if (nbply>ply_retro_move)
  {
    update_taboo(+1);

    top_before_relevations[nbply] = move_effect_journal_base[nbply+1];

    make_revelations();

    if (revelation_status_is_uninitialised)
      pipe_solve_delegate(si);
    else
    {
      unsigned int nr_revealed_unplaced_invisibles[nr_sides] = { 0, 0 };
      {
        unsigned int i;
        for (i = 0; i!=nr_potential_revelations; ++i)
        {
          square const s = revelation_status[i].pos;
          TraceValue("%u",i);
          TraceSquare(s);
          TraceWalk(revelation_status[i].walk);
          TraceValue("%x",revelation_status[i].spec);
          TraceEOL();
          if (revelation_status[i].walk!=Empty && is_square_empty(s))
          {
            Side const side = TSTFLAG(revelation_status[i].spec,White) ? White : Black;
            ++nr_revealed_unplaced_invisibles[side];
          }
        }
      }
      // TODO there must be a simpler, understandable way to do this
      if (current_consumption.claimed[White] && nr_revealed_unplaced_invisibles[White]>0)
        --nr_revealed_unplaced_invisibles[White];
      if (current_consumption.claimed[Black] && nr_revealed_unplaced_invisibles[Black]>0)
        --nr_revealed_unplaced_invisibles[Black];
      if ((current_consumption.placed_fleshed_out[White]
           +current_consumption.placed_fleshed_out[Black]
           +current_consumption.placed_not_fleshed_out[White]
           +current_consumption.placed_not_fleshed_out[Black]
           +nr_revealed_unplaced_invisibles[White]
           +nr_revealed_unplaced_invisibles[Black])
          <=total_invisible_number)
      {
        knowledge_type const save_knowledge = knowledge;
        PieceIdType const save_next_invisible_piece_id = next_invisible_piece_id;
        evaluate_revelations();
        pipe_solve_delegate(si);
        TraceConsumption();TraceEOL();
        next_invisible_piece_id = save_next_invisible_piece_id;
        knowledge = save_knowledge;
      }
    }

    update_taboo(-1);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int find_nr_interceptors_needed(Side side_checking,
                                                square potential_flight,
                                                unsigned int nr_interceptors_available,
                                                vec_index_type start, vec_index_type end,
                                                piece_walk_type walk_rider)
{
  unsigned int result = 0;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(potential_flight);
  TraceValue("%u",nr_placeable_invisibles_for_side);
  TraceValue("%u",start);
  TraceValue("%u",end);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  for (k = start; k<=end && result<=nr_interceptors_available; ++k)
  {
    square const end = find_end_of_line(potential_flight,vec[k]);
    piece_walk_type const walk = get_walk_of_piece_on_square(end);
    Flags const flags = being_solved.spec[end];
    if ((walk==Queen || walk==walk_rider) && TSTFLAG(flags,side_checking))
    {
      square s;
      for (s = potential_flight+vec[k]; s!=end; s += vec[k])
        if (taboo[White][s]==0 || taboo[Black][s]==0)
        {
          ++result;
          break;
        }

      if (s==end)
      {
        /* line can't be intercepted - all guard are not interceptable */
        result = nr_interceptors_available+1;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_all_guards_interceptable(Side side_in_check, square potential_flight)
{
  Side const side_checking = advers(side_in_check);
  unsigned int nr_available;
  unsigned int nr_interceptors_needed;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(potential_flight);
  TraceFunctionParamListEnd();

  nr_available = nr_placeable_invisibles_for_side(side_in_check);
  TraceConsumption();
  TraceValue("%u",nr_available);
  TraceEOL();

  nr_interceptors_needed = find_nr_interceptors_needed(side_checking,
                                                       potential_flight,
                                                       nr_available,
                                                       vec_rook_start,vec_rook_end,
                                                       Rook);

  if (nr_interceptors_needed<=nr_available)
    nr_interceptors_needed += find_nr_interceptors_needed(side_checking,
                                                          potential_flight,
                                                          nr_available-nr_interceptors_needed,
                                                          vec_bishop_start,vec_bishop_end,
                                                          Bishop);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",(nr_interceptors_needed<=nr_available));
  TraceFunctionResultEnd();
  return nr_interceptors_needed<=nr_available;
}

static boolean make_flight(Side side_in_check, square s)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if ((is_square_empty(s)
      || TSTFLAG(being_solved.spec[s],advers(side_in_check)))
      && !is_square_uninterceptably_attacked(side_in_check,s)
      && are_all_guards_interceptable(side_in_check,s))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_a_flight(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_in_check = advers(trait[nbply]);
    square const king_pos = being_solved.king_square[side_in_check];
    int dir_vert;
    int dir_horiz;

    piece_walk_type const walk = get_walk_of_piece_on_square(king_pos);
    Flags const flags = being_solved.spec[king_pos];

    TraceWalk(walk);
    TraceValue("%x",flags);
    TraceEOL();

    empty_square(king_pos);

    for (dir_vert = dir_down; dir_vert<=dir_up && !result; dir_vert += dir_up)
      for (dir_horiz = dir_left; dir_horiz<=dir_right; dir_horiz += dir_right)
      {
        square const flight = king_pos+dir_vert+dir_horiz;
        if (flight!=king_pos && make_flight(side_in_check,flight))
        {
          result = true;
          break;
        }
      }

    occupy_square(king_pos,walk,flags);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void attack_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_delivering_check = trait[nbply];
    Side const side_in_check = advers(side_delivering_check);
    square const king_pos = being_solved.king_square[side_in_check];
    vec_index_type const k = is_square_uninterceptably_attacked(side_in_check,king_pos);
    if (k==0)
    {
      if (are_all_guards_interceptable(side_in_check,king_pos))
      {
        TraceText("interceptable check\n");
        /* the king square can be made a "flight" */
        mate_validation_result = no_mate;
        solve_result = previous_move_has_not_solved;
      }
      else
      {
        TraceText("mate can be refuted by interceptors or free TIs (if any)\n");
        mate_validation_result = mate_defendable_by_interceptors;
      }
    }
    else if (nr_placeable_invisibles_for_side(side_in_check)>0)
    {
      square const sq_attacker = find_end_of_line(king_pos,vec[k]);
      TraceSquare(king_pos);TraceValue("%u",k);TraceValue("%d",vec[k]);TraceSquare(sq_attacker);TraceEOL();
      assert(TSTFLAG(being_solved.spec[sq_attacker],side_delivering_check));
      CLRFLAG(being_solved.spec[sq_attacker],side_delivering_check);
      if (is_square_uninterceptably_attacked(side_in_check,king_pos))
      {
        TraceText("mate can not be defended\n");
        mate_validation_result = mate_with_2_uninterceptable_doublechecks;
      }
      else
      {
        TraceText("mate can be refuted by free TIs\n");
        mate_validation_result = mate_attackable;
        sq_mating_piece_to_be_attacked = sq_attacker;
      }
      SETFLAG(being_solved.spec[sq_attacker],side_delivering_check);
    }
    else
    {
      TraceText("mate can be refuted by interceptors - no free TI avaliable\n");
      mate_validation_result = mate_defendable_by_interceptors;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_goal_guard_solve(slice_index si)
{
  play_phase_type const save_play_phase = play_phase;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* make sure that we don't generate pawn captures total invisible */
  assert(play_phase==play_replay_validating || play_phase==play_replay_testing);

  play_phase = play_testing_goal;
  pipe_solve_delegate(si);
  play_phase = save_play_phase;

  if (play_phase==play_replay_validating)
  {
    if (solve_result==previous_move_has_not_solved)
      mate_validation_result = no_mate;
    else if (make_a_flight())
    {
      solve_result = previous_move_has_not_solved;
      mate_validation_result = no_mate;
    }
    else
      attack_checks();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_generate_moves_by_invisible(slice_index si)
{
  Side const side_moving = trait[nbply];
  Side const side_capturee = advers(side_moving);
  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  curr_generation->departure = capture_by_invisible;

  // TODO refuse capturing a rook that has just castled
  for (s = boardnum; *s; ++s)
  {
    if (!is_square_empty(*s))
    {
      TraceWalk(get_walk_of_piece_on_square(*s));
      TraceValue("%x",being_solved.spec[*s]);
      TraceEOL();
    }
    if (TSTFLAG(being_solved.spec[*s],side_capturee)
        && get_walk_of_piece_on_square(*s)!=Dummy
        && !TSTFLAG(being_solved.spec[*s],Chameleon)
        && !TSTFLAG(being_solved.spec[*s],Royal))
    {
      curr_generation->arrival = *s;
      push_move_regular_capture();
    }
  }

  curr_generation->departure = move_by_invisible;
  curr_generation->arrival = move_by_invisible;
  push_move_capture_extra(move_by_invisible);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_right(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO what kind of taboo can we apply here?
  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    pipe_move_generation_delegate(si);
    empty_square(s);
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_left(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_left;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO what kind of taboo can we apply here?
  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    generate_pawn_capture_right(si,dir_vertical);
    empty_square(s);
  }
  else
    generate_pawn_capture_right(si,dir_vertical);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_king_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;
  square const square_h = square_a+file_h;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if ((nr_total_invisbles_consumed()<total_invisible_number)
      && is_square_empty(square_h)
      && taboo[White][square_h]==0
      && taboo[Black][square_h]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_h,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,rh_cancastle);
    pipe_move_generation_delegate(si);
    CLRCASTLINGFLAGMASK(side,rh_cancastle);
    empty_square(square_h);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_queen_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if ((nr_total_invisbles_consumed()<total_invisible_number)
      && is_square_empty(square_a)
      && taboo[White][square_a]==0
      && taboo[Black][square_a]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_a,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,ra_cancastle);
    prepare_king_side_castling_generation(si);
    CLRCASTLINGFLAGMASK(side,ra_cancastle);
    empty_square(square_a);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    prepare_king_side_castling_generation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void total_invisible_generate_special_moves(slice_index si)
{
  square const sq_departure = curr_generation->departure ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (play_phase==play_regular)
    switch (being_solved.board[sq_departure])
    {
      case Pawn:
      {
        Side const side_playing = trait[nbply];
        Side const side_victim = advers(side_playing);
        if (nr_placeable_invisibles_for_side(side_victim)>0)
          generate_pawn_capture_left(si,side_playing==White ? dir_up : dir_down);
        else
          pipe_move_generation_delegate(si);
        break;
      }

      case King:
        if (TSTCASTLINGFLAGMASK(trait[nbply],castlings)>=k_cancastle)
          prepare_queen_side_castling_generation(si);
        else
          pipe_move_generation_delegate(si);
        break;

      default:
        pipe_move_generation_delegate(si);
        break;
    }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void play_castling_with_invisible_partner(slice_index si,
                                                 square square_partner)
{
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(square_partner);
  TraceFunctionParamListEnd();

  if (is_square_empty(square_partner))
  {
    consumption_type const save_consumption = current_consumption;
    if (allocate_placement_of_unclaimed_fleshed_out(trait[nbply]))
    {
      Flags spec = BIT(side)|BIT(Chameleon);
      SetPieceId(spec,++next_invisible_piece_id);
      move_effect_journal_do_piece_readdition(move_effect_reason_castling_partner,
                                              square_partner,Rook,spec,side);
      assert(motivation[next_invisible_piece_id].last.purpose==purpose_none);
      motivation[next_invisible_piece_id].insertion_iteration = current_iteration;
      motivation[next_invisible_piece_id].first.purpose = purpose_castling_partner;
      motivation[next_invisible_piece_id].first.acts_when = nbply;
      motivation[next_invisible_piece_id].first.on = square_partner;
      motivation[next_invisible_piece_id].last.purpose = purpose_castling_partner;
      motivation[next_invisible_piece_id].last.acts_when = nbply;
      motivation[next_invisible_piece_id].last.on = square_partner;
      pipe_solve_delegate(si);
      motivation[next_invisible_piece_id].last.purpose = purpose_none;
      --next_invisible_piece_id;
    }
    else
    {
      /* we have checked acts_when we generated the castling */
      assert(0);
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }
  else
  {
    move_effect_journal_do_null_effect();
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_special_moves_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;
    square const sq_capture = move_gen_top->capture;

    TraceSquare(sq_departure);
    TraceSquare(move_gen_top->arrival);
    TraceSquare(sq_capture);
    TraceEOL();

    if (sq_departure==move_by_invisible)
      pipe_solve_delegate(si);
    else if (sq_departure>=capture_by_invisible)
    {
      Side const side = trait[nbply];
      Flags spec = BIT(side)|BIT(Chameleon);
      SetPieceId(spec,++next_invisible_piece_id);
      assert(motivation[next_invisible_piece_id].last.purpose==purpose_none);
      motivation[next_invisible_piece_id].insertion_iteration = current_iteration;
      motivation[next_invisible_piece_id].first.purpose = purpose_capturer;
      motivation[next_invisible_piece_id].first.acts_when = nbply;
      motivation[next_invisible_piece_id].first.on = sq_departure;
      motivation[next_invisible_piece_id].last.purpose = purpose_capturer;
      motivation[next_invisible_piece_id].last.acts_when = nbply;
      motivation[next_invisible_piece_id].last.on = sq_departure;
      move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                              sq_departure,Dummy,spec,side);

      /* No adjustment of current_consumption.placed here!
       * The capture may be done by an existing invisible. We can only do the
       * adjustment when we flesh out this capture by inserting a new invisible.
       */
      pipe_solve_delegate(si);
      motivation[next_invisible_piece_id].last.purpose = purpose_none;
      --next_invisible_piece_id;
    }
    else
      switch (sq_capture)
      {
        case pawn_multistep:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case messigny_exchange:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case kingside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_h = square_a+file_h;

          TraceText("kingside_castling\n");
          play_castling_with_invisible_partner(si,square_h);
          break;
        }

        case queenside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;

          TraceText("queenside_castling\n");
          play_castling_with_invisible_partner(si,square_a);
          break;
        }

        case no_capture:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        default:
          /* pawn captures total invisible? */
          if (is_square_empty(sq_capture))
          {
            Side const side_victim = advers(SLICE_STARTER(si));
            Flags spec = BIT(side_victim)|BIT(Chameleon);

            SetPieceId(spec,++next_invisible_piece_id);
            assert(motivation[next_invisible_piece_id].last.purpose==purpose_none);
            motivation[next_invisible_piece_id].insertion_iteration = current_iteration;
            motivation[next_invisible_piece_id].first.purpose = purpose_victim;
            motivation[next_invisible_piece_id].first.acts_when = nbply;
            motivation[next_invisible_piece_id].first.on = sq_capture;
            motivation[next_invisible_piece_id].last.purpose = purpose_victim;
            motivation[next_invisible_piece_id].last.acts_when = nbply;
            motivation[next_invisible_piece_id].last.on = sq_capture;
            move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                                    sq_capture,Dummy,spec,side_victim);

            /* No adjustment of current_consumption.placed here! Another invisible may
             * have moved to sq_capture and serve as a victim.
             */
            pipe_solve_delegate(si);

            motivation[next_invisible_piece_id].last.purpose = purpose_none;
            --next_invisible_piece_id;
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_reserve_king_movement(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* reserve a spot in the move effect journal for the case that a move by an invisible
   * turns out to move a side's king square
   */
  move_effect_journal_do_null_effect();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleMoveSequenceMoveRepeater);
    SLICE_STARTER(prototype) = SLICE_STARTER(si);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_the_pipe(slice_index si,
                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_castling_player(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  dealloc_slice(SLICE_NEXT2(si));
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_self_check_guard(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const remembered = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *remembered = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_goal_guard(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index remembered = no_slice;

    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,&remembered);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remember_self_check_guard);
    stip_traverse_structure(SLICE_NEXT2(si),&st_nested);

    if (remembered!=no_slice)
    {
      slice_index prototype = alloc_pipe(STTotalInvisibleGoalGuard);
      SLICE_STARTER(prototype) = SLICE_STARTER(remembered);
      goal_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
      pipe_remove(remembered);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_replay_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;

    stip_structure_traversal_init_nested(&st_nested,st,0);
    // TODO prevent instrumentation in the first place?
    stip_structure_traversal_override_single(&st_nested,
                                             STFindShortest,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STFindAttack,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveEffectJournalUndoer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPostMoveIterationInitialiser,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STCastlingPlayer,
                                             &remove_castling_player);
    stip_structure_traversal_override_single(&st_nested,
                                             STMovePlayer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPawnPromoter,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveGenerator,
                                             &subsitute_generator);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachedTester,
                                             &subsitute_goal_guard);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remove_the_pipe);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void replace_self_check_guard(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* This iteration ends at STTotalInvisibleMoveSequenceTester. We can therefore
   * blindly tamper with all STSelfCheckGuard slices that we meet.
   */
  stip_traverse_structure_children_pipe(si,st);

  SLICE_TYPE(si) = STTotalInvisibleUninterceptableSelfCheckGuard;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STTotalInvisibleMoveSequenceTester,
                                             &instrument_replay_branch);
    stip_structure_traversal_override_single(&st,
                                             STSelfCheckGuard,
                                             &replace_self_check_guard);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  output_plaintext_check_indication_disabled = true;

  solving_instrument_move_generation(si,nr_sides,STTotalInvisibleSpecialMoveGenerator);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInvisiblesAllocator);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        ++taboo[White][*s];
        ++taboo[Black][*s];
      }
  }

  pipe_solve_delegate(si);

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        --taboo[White][*s];
        --taboo[Black][*s];
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_invisibles_allocator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  next_invisible_piece_id = being_solved.currPieceId;

  being_solved.currPieceId += total_invisible_number;

  pipe_solve_delegate(si);

  being_solved.currPieceId -= total_invisible_number;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean instrumenting;
    slice_index the_copy;
    stip_length_type length;
} insertion_state_type;

static void insert_copy(slice_index si,
                        stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->the_copy==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const substitute = alloc_pipe(STTotalInvisibleMoveSequenceTester);
    pipe_link(proxy,substitute);
    link_to_branch(substitute,state->the_copy);
    SLICE_NEXT2(substitute) = state->the_copy;
    state->the_copy = no_slice;
    dealloc_slices(SLICE_NEXT2(si));
    SLICE_NEXT2(si) = proxy;

    assert(state->length!=no_stip_length);
    if (state->length%2!=0)
      pipe_append(proxy,alloc_pipe(STMoveInverter));

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleRevealAfterFinalMove)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      help_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_help_branch(slice_index si,
                             stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->instrumenting);
  TraceEOL();

  state->length = slices[si].u.branch.length;

  if (state->instrumenting)
    stip_traverse_structure_children(si,st);
  else
  {
    state->instrumenting = true;
    state->the_copy = stip_deep_copy(si);
    stip_traverse_structure_children(si,st);
    assert(state->the_copy==no_slice);

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement),
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement)
      };
      enum { nr_protypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_protypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // later:
  // - in original
  //   - insert revelation logic
  // - in copy
  //   - logic for iteration over all possibilities of invisibles
  //     - special case of invisible king
  //     - special case: position has no legal placement of all invisibles may have to be dealt with:
  //       - not enough empty squares :-)
  //   - substitute for STFindShortest

  // bail out at STAttackAdapter

  // input for total_invisible_number, initialize to 0

  // what about:
  // - structured stipulations?
  // - goals that don't involve immobility
  // ?

  // we shouldn't need to set the starter of
  // - STTotalInvisibleMoveSequenceMoveRepeater
  // - STTotalInvisibleGoalGuard

  // check indication should also be deactivated in tree output

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInstrumenter);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    insertion_state_type state = { false, no_slice, no_stip_length };

    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_single(&st,
                                             STHelpAdapter,
                                             &copy_help_branch);
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &insert_copy);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  move_effect_journal_register_pre_capture_effect();

  move_effect_journal_set_effect_doers(move_effect_revelation_of_new_invisible,
                                       &undo_revelation_of_new_invisible,
                                       &redo_revelation_of_new_invisible);

  move_effect_journal_set_effect_doers(move_effect_revelation_of_castling_partner,
                                       &undo_revelation_of_castling_partner,
                                       &redo_revelation_of_castling_partner);

  move_effect_journal_set_effect_doers(move_effect_revelation_of_placed_invisible,
                                       &undo_revelation_of_placed_invisible,
                                       &redo_revelation_of_placed_invisible);

  TraceFunctionResultEnd();
  TraceFunctionExit(__func__);
}
