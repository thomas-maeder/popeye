#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

PieNam transmpieces[nr_sides][PieceCount];

/* Initialise the sequence of king transmuters
 * @param side for which side to initialise?
 */
void init_transmuters_sequence(Side side)
{
  unsigned int tp = 0;
  PieNam p;

  for (p = King; p<PieceCount; ++p) {
    if (may_exist[p] && p!=Dummy && p!=Hamster)
    {
      transmpieces[side][tp] = p;
      tp++;
    }
  }

  transmpieces[side][tp] = Empty;
}

/* Does the transmuting king of side trait[nbply] attack a particular square
 * while transmuting?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return how much attack of the transmuting king to sq_target is there?
 */
transmuting_kings_attack_type
transmuting_kings_is_square_attacked_by_transmuting_king(square sq_target,
                                                         evalfunction_t *evaluate)
{
  Side const side_attacking = trait[nbply];
  Side const side_attacked = advers(side_attacking);
  transmuting_kings_attack_type result = king_not_transmuting;

  PieNam *ptrans;

  for (ptrans = transmpieces[side_attacking]; *ptrans; ptrans++)
    if (number_of_pieces[side_attacked][*ptrans]>0)
    {
      boolean is_king_transmuted;

      trait[nbply] = advers(trait[nbply]);
      is_king_transmuted = (*checkfunctions[*ptrans])(king_square[side_attacking],*ptrans,evaluate);
      trait[nbply] = advers(trait[nbply]);

      if (is_king_transmuted)
      {
        if ((*checkfunctions[*ptrans])(sq_target,King,evaluate))
        {
          result = king_transmuting_attack;
          break;
        }
        else
          result = king_transmuting_no_attack;
      }
    }

  return result;
}

/* Does the transmuting king of side trait[nbply] attack a particular square
 * (while transmuting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return true iff a king attacks sq_target?
 */
boolean transmuting_kings_is_square_attacked_by_king(square sq_target,
                                                     evalfunction_t *evaluate)
{
  switch (transmuting_kings_is_square_attacked_by_transmuting_king(sq_target,evaluate))
  {
    case king_not_transmuting:
      return roicheck(sq_target,King,evaluate);

    case king_transmuting_no_attack:
      return false;

    case king_transmuting_attack:
      return true;

    default:
      assert(0);
      return false;
  }
}

static boolean generate_moves_of_transmuting_king(slice_index si,
                                                  square sq_departure)
{
  boolean result = false;
  PieNam const *ptrans;
  Side const side_moving = trait[nbply];

  for (ptrans = transmpieces[side_moving]; *ptrans!=Empty; ++ptrans)
  {
    Side const side_transmuting = advers(side_moving);

    if (number_of_pieces[side_transmuting][*ptrans]>0)
    {
      boolean is_king_transmuted;

      nextply(side_transmuting);
      is_king_transmuted = (*checkfunctions[*ptrans])(sq_departure,
                                                      *ptrans,
                                                      &validate_observation);
      finply();

      if (is_king_transmuted)
      {
        result = true;
        current_trans_gen = *ptrans;
        generate_moves_for_piece(slices[si].next1,sq_departure,*ptrans);
        current_trans_gen = Empty;
      }
    }
  }

  return result;
}

/* Does the reflective king of side trait[nbply] attack a particular square
 * (while transmuting or not)?
 * @param sq_target target square
 * @param evaluate attack evaluator
 * @return true iff a king attacks sq_target?
 */
boolean reflective_kings_is_square_attacked_by_king(square sq_target,
                                                    evalfunction_t *evaluate)
{
  switch (transmuting_kings_is_square_attacked_by_transmuting_king(sq_target,evaluate))
  {
    case king_not_transmuting:
    case king_transmuting_no_attack:
      return roicheck(sq_target,King,evaluate);

    case king_transmuting_attack:
      return true;

    default:
      assert(0);
      return false;
  }
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void transmuting_kings_generate_moves_for_piece(slice_index si,
                                                square sq_departure,
                                                PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (p==King)
  {
    if (!generate_moves_of_transmuting_king(si,sq_departure))
      generate_moves_for_piece(slices[si].next1,sq_departure,King);
  }
  else
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with transmuting kings
 * @param si identifies root slice of solving machinery
 */
void transmuting_kings_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[whtrans_king] || CondFlag[whsupertrans_king])
  {
    solving_instrument_move_generation(si,White,STTransmutingKingsMovesForPieceGenerator);
    instrument_alternative_is_square_observed_king_testing(si,White,STTransmutingKingIsSquareObserved);
  }
  if (CondFlag[bltrans_king] || CondFlag[blsupertrans_king])
  {
    solving_instrument_move_generation(si,Black,STTransmutingKingsMovesForPieceGenerator);
    instrument_alternative_is_square_observed_king_testing(si,Black,STTransmutingKingIsSquareObserved);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a square is observed be the side at the move according to
 * Transmuting Kings
 * @param si identifies next slice
 * @param sq_target the square
 * @return true iff sq_target is observed by the side at the move
 */
boolean transmuting_king_is_square_observed(slice_index si,
                                            square sq_target,
                                            evalfunction_t *evaluate)
{
  if (number_of_pieces[trait[nbply]][King]>0)
  {
    if (transmuting_kings_is_square_attacked_by_king(sq_target,evaluate))
      return true;
    else
      return is_square_observed_recursive(slices[si].next2,sq_target,evaluate);
  }
  else
    return is_square_observed_recursive(slices[si].next1,sq_target,evaluate);
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void reflective_kings_generate_moves_for_piece(slice_index si,
                                               square sq_departure,
                                               PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (p==King)
  {
    numecoup const save_nbcou = current_move[nbply];
    generate_moves_for_piece(slices[si].next1,sq_departure,King);
    if (generate_moves_of_transmuting_king(si,sq_departure))
      remove_duplicate_moves_of_single_piece(save_nbcou);
  }
  else
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with reflective kings
 * @param si identifies root slice of solving machinery
 */
void reflective_kings_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[whrefl_king])
  {
    solving_instrument_move_generation(si,White,STReflectiveKingsMovesForPieceGenerator);
    instrument_alternative_is_square_observed_king_testing(si,White,STReflectiveKingIsSquareObserved);
  }
  if (CondFlag[blrefl_king])
  {
    solving_instrument_move_generation(si,Black,STReflectiveKingsMovesForPieceGenerator);
    instrument_alternative_is_square_observed_king_testing(si,Black,STReflectiveKingIsSquareObserved);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a square is observed be the side at the move according to
 * Reflective Kings
 * @param si identifies next slice
 * @param sq_target the square
 * @return true iff sq_target is observed by the side at the move
 */
boolean reflective_king_is_square_observed(slice_index si,
                                           square sq_target,
                                           evalfunction_t *evaluate)
{
  if (number_of_pieces[trait[nbply]][King]>0)
  {
    if (reflective_kings_is_square_attacked_by_king(sq_target,evaluate))
      return true;
    else
      return is_square_observed_recursive(slices[si].next2,sq_target,evaluate);
  }
  else
    return is_square_observed_recursive(slices[si].next1,sq_target,evaluate);
}

typedef struct
{
    Side side;
    slice_type type;
    slice_index after_king;
} instrumenatation_type;

static void instrument_testing(slice_index si, stip_structure_traversal *st)
{
  instrumenatation_type * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(it->after_king==no_slice);

  if (it->side==nr_sides || it->side==slices[si].starter)
    stip_instrument_is_square_observed_insert_slice(si,it->type);

  stip_traverse_structure_children_pipe(si,st);

  it->after_king = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_after_king(slice_index si, stip_structure_traversal *st)
{
  instrumenatation_type * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  it->after_king = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void connect_to_after_king(slice_index si, stip_structure_traversal *st)
{
  instrumenatation_type const * const it = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (slices[si].next2==no_slice)
  {
    slices[si].next2 = alloc_proxy_slice();
    link_to_branch(slices[si].next2,it->after_king);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the square observation machinery for a side with an alternative
 * slice dealting with observations by kings.
 * @param si identifies the root slice of the solving machinery
 * @param side side for which to instrument the square observation machinery
 * @param type type of slice to insert
 * @note next2 of inserted slices will be set to the position behind the
 *       regular square observation by king handler
 */
void instrument_alternative_is_square_observed_king_testing(slice_index si,
                                                            Side side,
                                                            slice_type type)
{
  instrumenatation_type it = { side, type, no_slice };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side,"");
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&it);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterFindSquareObserverTrackingBackKing,
                                           &remember_after_king);
  stip_structure_traversal_override_single(&st,type,&connect_to_after_king);
  stip_structure_traversal_override_single(&st,
                                           STTestingIfSquareIsObserved,
                                           &instrument_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
