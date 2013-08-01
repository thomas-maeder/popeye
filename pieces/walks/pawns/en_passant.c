#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/hunters.h"
#include "solving/move_effect_journal.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

square en_passant_multistep_over[2][maxply+1];

/* Remember a square avoided by a multistep move of a pawn
 * @param index index of square (between 0<=index<en_passant_max_nr_multistep_over)
 * @param s avoided square
 */
void en_passant_remember_multistep_over(unsigned int index, square s)
{
  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(s!=initsquare);
  assert(index<en_passant_max_nr_multistep_over);
  en_passant_multistep_over[index][nbply] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget the last square remembered by en_passant_remember_multistep_over()
 */
void en_passant_forget_multistep(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  en_passant_multistep_over[0][nbply] = initsquare;
  en_passant_multistep_over[1][nbply] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember a possible en passant capture
 * @param diff adjustment
 */
void move_effect_journal_do_remember_ep(unsigned int index, square s)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",index);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_remember_ep_capture_potential;
  top_elmt->reason = move_effect_reason_moving_piece_movement;
  top_elmt->u.ep_capture_potential.index = index;
  top_elmt->u.ep_capture_potential.square = s;
 #if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
 #endif

  ++move_effect_journal_top[nbply];

  en_passant_multistep_over[index][nbply] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo remembering a possible en passant capture
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_remember_ep(move_effect_journal_index_type curr)
{
  unsigned int const index = move_effect_journal[curr].u.ep_capture_potential.index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  en_passant_multistep_over[index][nbply] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo remembering a possible en passant capture
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_remember_ep(move_effect_journal_index_type curr)
{
  unsigned int const index = move_effect_journal[curr].u.ep_capture_potential.index;
  square const s = move_effect_journal[curr].u.ep_capture_potential.square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  en_passant_multistep_over[index][nbply] = s;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Was a pawn multistep move played in a certain ply?
 * @param ply the ply
 * @return true iff a multi step move was played in ply ply
 */
boolean en_passant_was_multistep_played(ply ply)
{
  boolean const result = en_passant_multistep_over[0][ply]!=initsquare;

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
  move_effect_journal_index_type const parent_base = move_effect_journal_top[ply_parent-1];
  move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
  PieceIdType const capturee_id = GetPieceId(move_effect_journal[parent_movement].u.piece_movement.movingspec);
  square const sq_arrival = move_effect_journal[parent_movement].u.piece_movement.to;
  square result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = move_effect_journal_follow_piece_through_other_effects(ply_parent,capturee_id,sq_arrival);

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static boolean en_passant_test_check_one_square_crossed(square sq_crossed,
                                                        numvec dir_capture,
                                                        en_passant_check_tester_type tester,
                                                        PieNam p,
                                                        evalfunction_t *evaluate)
{
  square const sq_departure = sq_crossed-dir_capture;
  return (get_walk_of_piece_on_square(sq_departure)!=Orphan
          && (*tester)(sq_departure,sq_crossed,p,evaluate));
}

/* Determine whether side trait[nbply] gives check by p. capture
 * @param dir_capture direction of ep capture
 * @param tester pawn-specific tester function
 * @param evaluate address of evaluater function
 * @return true if side trait[nbply] gives check by ep. capture
 */
boolean en_passant_test_check(numvec dir_capture,
                              en_passant_check_tester_type tester,
                              PieNam p,
                              evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]-1].capture;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (sq_target==en_passant_find_capturee())
  {
    square const sq_crossed0 = en_passant_multistep_over[0][parent_ply[nbply]];
    if (sq_crossed0!=initsquare)
    {
      if (en_passant_test_check_one_square_crossed(sq_crossed0,dir_capture,tester,p,evaluate))
        result = true;
      else
      {
        square const sq_crossed1 = en_passant_multistep_over[1][parent_ply[nbply]];
        if (sq_crossed1!=initsquare
            && en_passant_test_check_one_square_crossed(sq_crossed1,dir_capture,tester,p,evaluate))
          result = true;
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
  boolean result;
  ply const ply_parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(s);
  TraceFunctionParamListEnd();

  result = (trait[ply_parent]!=side
            && (en_passant_multistep_over[0][ply_parent]==s
                || en_passant_multistep_over[1][ply_parent]==s));

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
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  PieNam pi_moving = move_effect_journal[movement].u.piece_movement.moving;
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;

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
          && TSTFLAGMASK(sq_spec[sq_multistep_departure],double_step))
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
          && TSTFLAGMASK(sq_spec[sq_multistep_departure],double_step))
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
          && TSTFLAGMASK(sq_spec[sq_multistep_departure],double_step))
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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type en_passant_adjuster_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  PieNam pi_moving = move_effect_journal[movement].u.piece_movement.moving;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pi_moving>=Hunter0)
    pi_moving = huntertypes[pi_moving-Hunter0].away;

  if (is_pawn(pi_moving)
      && move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    square const multistep_over = en_passant_find_potential(move_effect_journal[movement].u.piece_movement.from);
    if (multistep_over!=initsquare)
      move_effect_journal_do_remember_ep(0,multistep_over);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_en_passant_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STEnPassantAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
