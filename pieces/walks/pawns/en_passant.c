#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/hunters.h"
#include "pieces/walks/classification.h"
#include "position/effects/piece_movement.h"
#include "position/effects/piece_removal.h"
#include "position/effects/utils.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

square en_passant_multistep_over[maxply+1];

unsigned int en_passant_top[maxply+1];

square en_passant_retro_squares[en_passant_retro_capacity];
unsigned int en_passant_nr_retro_squares;

/* Determine whether the retro information concernng en passant is consistent
 * @return true iff the informatoin is consistent
 */
boolean en_passant_are_retro_squares_consistent(void)
{
  if (en_passant_nr_retro_squares>=en_passant_retro_min_squares)
  {
    unsigned int i;
    for (i = 0; i!=en_passant_nr_retro_squares-1; ++i)
      if (get_walk_of_piece_on_square(en_passant_retro_squares[i])!=Empty)
        return false;

    if (get_walk_of_piece_on_square(en_passant_retro_squares[en_passant_nr_retro_squares-1])==Empty)
      return false;
  }

  return true;
}

/* determine whether a retro doublestep has the relevant direction for the side
 * to move
 * @param side side to move
 * @param diff vector from start to end square of the doublestep
 **/
static boolean has_doublestep_right_direction(Side side, numvec diff)
{
  return (side==White) == (diff<0);
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
void en_passant_undo_multistep(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(en_passant_nr_retro_squares>=en_passant_retro_min_squares);

  if (has_doublestep_right_direction(SLICE_STARTER(si),
                                     en_passant_retro_squares[en_passant_nr_retro_squares-1]
                                     -en_passant_retro_squares[0]))
      move_effect_journal_do_piece_movement(move_effect_reason_diagram_setup,
                                            en_passant_retro_squares[en_passant_nr_retro_squares-1],
                                            en_passant_retro_squares[0]);

  pipe_solve_delegate(si);

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
void en_passant_redo_multistep(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(en_passant_nr_retro_squares>2);

  move_effect_journal_do_no_piece_removal();

  if (has_doublestep_right_direction(SLICE_STARTER(si),
                                     en_passant_retro_squares[en_passant_nr_retro_squares-1]
                                     -en_passant_retro_squares[0]))
  {
    unsigned int i;

    move_effect_journal_do_piece_movement(move_effect_reason_moving_piece_movement,
                                          en_passant_retro_squares[0],
                                          en_passant_retro_squares[en_passant_nr_retro_squares-1]);

    for (i = 1; i<en_passant_nr_retro_squares-1; ++i)
      en_passant_remember_multistep_over(en_passant_retro_squares[i]);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember a square avoided by a multistep move of a pawn
 * @param s avoided square
 */
void en_passant_remember_multistep_over(square s)
{
  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(s!=initsquare);

  ++en_passant_top[nbply];
  en_passant_multistep_over[en_passant_top[nbply]] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget the last square remembered by en_passant_remember_multistep_over()
 */
void en_passant_forget_multistep(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  en_passant_top[ply_retro_move] = en_passant_top[ply_diagram_setup];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember a possible en passant capture
 * @param diff adjustment
 */
void move_effect_journal_do_remember_ep(square s)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_remember_ep_capture_potential,move_effect_reason_moving_piece_movement);

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  entry->u.ep_capture_potential.capture_square = s;
  entry->u.ep_capture_potential.ply = nbply;

  ++en_passant_top[nbply];
  TraceValue("%u",nbply);
  TraceValue("%u",en_passant_top[nbply]);
  TraceEOL();
  en_passant_multistep_over[en_passant_top[nbply]] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo remembering a possible en passant capture
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_remember_ep(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",entry->u.ep_capture_potential.ply);
  TraceValue("%u",en_passant_top[entry->u.ep_capture_potential.ply]);
  TraceEOL();
  assert(en_passant_top[entry->u.ep_capture_potential.ply]>0);
  --en_passant_top[entry->u.ep_capture_potential.ply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo remembering a possible en passant capture
 */
void move_effect_journal_redo_remember_ep(move_effect_journal_entry_type const *entry)
{
  square const s = entry->u.ep_capture_potential.capture_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",entry->u.ep_capture_potential.ply);
  TraceValue("%u",en_passant_top[entry->u.ep_capture_potential.ply]);
  TraceEOL();
  ++en_passant_top[entry->u.ep_capture_potential.ply];
  en_passant_multistep_over[en_passant_top[entry->u.ep_capture_potential.ply]] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Was a pawn multistep move played in a certain ply?
 * @param ply the ply
 * @return true iff a multi step move was played in ply ply
 */
boolean en_passant_was_multistep_played(ply ply)
{
  boolean const result = en_passant_top[nbply]>en_passant_top[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the en passant capturee of the current ply. Only meaningful if an en
 * passant capture is actually possible, which isn't tested here.
 * @return position of the capturee
 *         initsquare if the capturee vanished from the board
 */
square en_passant_find_capturee(void)
{
  ply const ply_parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_base[ply_parent];
  move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
  PieceIdType const capturee_id = GetPieceId(move_effect_journal[parent_movement].u.piece_movement.movingspec);
  square const sq_arrival = move_effect_journal[parent_movement].u.piece_movement.to;
  square result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (capturee_id==NullPieceId)
    result = initsquare;
  else
    result = move_effect_journal_follow_piece_through_other_effects(ply_parent,capturee_id,sq_arrival);

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static boolean en_passant_test_check_one_square_crossed(square sq_crossed,
                                                        numvec dir_capture,
                                                        en_passant_check_tester_type tester,
                                                        validator_id evaluate)
{
  square const sq_departure = sq_crossed-dir_capture;
  return ((get_walk_of_piece_on_square(sq_departure)!=Orphan
          && (*tester)(sq_departure,sq_crossed,evaluate)));
}

/* Determine whether side trait[nbply] gives check by p. capture
 * @param dir_capture direction of ep capture
 * @param tester pawn-specific tester function
 * @param evaluate address of evaluater function
 * @return true if side trait[nbply] gives check by ep. capture
 */
boolean en_passant_test_check(numvec dir_capture,
                              en_passant_check_tester_type tester,
                              validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;
  ply const ply_parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,trait[nbply]);
  TraceEnumerator(Side,trait[ply_parent]);
  TraceEOL();

  if (trait[nbply]!=trait[ply_parent] && sq_target==en_passant_find_capturee())
  {
    unsigned int i;

    for (i = en_passant_top[ply_parent-1]+1; i<=en_passant_top[ply_parent]; ++i)
    {
      square const sq_crossed = en_passant_multistep_over[i];
      if (sq_crossed!=initsquare
          && en_passant_test_check_one_square_crossed(sq_crossed,
                                                      dir_capture,
                                                      tester,
                                                      evaluate))
      {
        result = true;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is an en passant capture possible to a specific square?
 * @param side for which side
 * @param s the square
 * @return true iff an en passant capture to s is currently possible
 */
boolean en_passant_is_capture_possible_to(Side side, square s)
{
  boolean result = false;
  ply const ply_parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_parent);
  TraceEnumerator(Side,trait[ply_parent]);
  TraceEOL();

  if (trait[ply_parent]!=side)
  {
    unsigned int i;

    TraceValue("%u",en_passant_top[ply_parent-1]);
    TraceValue("%u",en_passant_top[ply_parent]);
    TraceEOL();
    for (i = en_passant_top[ply_parent-1]+1; i<=en_passant_top[ply_parent]; ++i)
      if (en_passant_multistep_over[i]==s)
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

/* Find out whether the pawn that has just moved (if any) has the potential
 * of being captured en passant
 * @param sq_multistep_departure departure square of pawn move
 * @return the square avoided by the multistep; initsquare if no multistep
 */
square en_passant_find_potential(square sq_multistep_departure)
{
  square result = initsquare;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  piece_walk_type pi_moving = move_effect_journal[movement].u.piece_movement.moving;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_multistep_departure);
  TraceFunctionParamListEnd();

  if (pi_moving>=Hunter0)
    pi_moving = huntertypes[pi_moving-Hunter0].away;

  switch (pi_moving)
  {
    case Pawn:
    case MarinePawn:
    {
      numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
      SquareFlags const double_step = (trait[nbply]==White
                                       ? BIT(WhPawnDoublestepSq)|BIT(WhBaseSq)
                                       : BIT(BlPawnDoublestepSq)|BIT(BlBaseSq));
      if (sq_arrival-sq_multistep_departure==2*dir_forward
          && TSTFLAGMASK(sq_spec(sq_multistep_departure),double_step))
        result = (sq_multistep_departure+sq_arrival) / 2;
      break;
    }

    case BerolinaPawn:
    {
      numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
      SquareFlags const double_step = (trait[nbply]==White
                                       ? BIT(WhPawnDoublestepSq)|BIT(WhBaseSq)
                                       : BIT(BlPawnDoublestepSq)|BIT(BlBaseSq));
      numvec const v = sq_arrival-sq_multistep_departure;
      if ((v==2*dir_forward+2*dir_left || v==2*dir_forward+2*dir_right)
          && TSTFLAGMASK(sq_spec(sq_multistep_departure),double_step))
        result = (sq_multistep_departure+sq_arrival) / 2;
      break;
    }

    case ReversePawn:
    {
      numvec const dir_backward = trait[nbply]==Black ? dir_up : dir_down;
      SquareFlags const double_step = (trait[nbply]==Black
                                       ? BIT(WhPawnDoublestepSq)|BIT(WhBaseSq)
                                       : BIT(BlPawnDoublestepSq)|BIT(BlBaseSq));
      if (sq_arrival-sq_multistep_departure==2*dir_backward
          && TSTFLAGMASK(sq_spec(sq_multistep_departure),double_step))
        result = (sq_multistep_departure+sq_arrival) / 2;
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Does a square value represent an en passant capture?
 * @param sq_capture the square value
 * @return true iff a generated move with capture square sq_capture is an
 *         en passant capture
 */
boolean en_passant_is_ep_capture(square sq_capture)
{
  boolean const result = sq_capture>square_h8 && sq_capture<maxsquare;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_capture-offset_en_passant_capture);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void en_passant_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture==pawn_multistep)
  {
    move_effect_journal_index_type const top = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
    square const multistep_over = en_passant_find_potential(move_effect_journal[movement].u.piece_movement.from);
    if (multistep_over!=initsquare)
      move_effect_journal_do_remember_ep(multistep_over);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with en passant
 * @param si identifies the root slice of the solving machinery
 */
void en_passant_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_set_effect_doers(move_effect_remember_ep_capture_potential,
                                       &move_effect_journal_undo_remember_ep,
                                       &move_effect_journal_redo_remember_ep);

  stip_instrument_moves(si,STEnPassantAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
