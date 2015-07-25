#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/classification.h"
#include "conditions/singlebox/type1.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"

#include "debugging/assert.h"

boolean promonly[nr_piece_walks];

piece_walk_type pieces_pawns_promotee_sequence[pieces_pawns_nr_promotee_chains][nr_piece_walks];

static void build_promotee_sequence(pieces_pawns_promotee_sequence_selector_type selector,
                                    boolean (* const is_promotee)[nr_piece_walks])
{
  piece_walk_type p;
  piece_walk_type prev_prom_piece = Empty;

  for (p = King; p<nr_piece_walks; ++p)
    pieces_pawns_promotee_sequence[selector][p] = Empty;

  for (p = King; p<nr_piece_walks; ++p)
    if ((*is_promotee)[p])
    {
      pieces_pawns_promotee_sequence[selector][prev_prom_piece] = p;
      prev_prom_piece = p;
    }
}

static void init_promotee_sequence(pieces_pawns_promotee_sequence_selector_type selector,
                                   standard_walks_type * const standard_walks)
{

  if (CondFlag[promotiononly])
    build_promotee_sequence(selector,&promonly);
  else
  {
    boolean is_promotee[nr_piece_walks] = { false };
    piece_walk_type p;

    for (p = Queen; p<=Bishop; ++p)
      is_promotee[(*standard_walks)[p]] = true;

    for (p = King+1; p<nr_piece_walks; ++p)
      if (piece_walk_exists[p] && !is_pawn(p) && !is_king(p))
        is_promotee[p] = true;

    is_promotee[Dummy] = false;

    if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
    {
      is_promotee[(*standard_walks)[King]] = true;

      for (p = Bishop+1; p<nr_piece_walks; ++p)
        if (piece_walk_exists[p] && is_king(p))
          is_promotee[p] = true;
    }

    if (CondFlag[singlebox] && SingleBoxType!=ConditionType1)
    {
      is_promotee[(*standard_walks)[Pawn]] = true;

      for (p = Bishop+1; p<nr_piece_walks; ++p)
        if (piece_walk_exists[p] && is_pawn(p))
          is_promotee[p] = true;
    }

    build_promotee_sequence(selector,&is_promotee);
  }
}

/* Start a sequence of promotees
 * @param sq_arrival arrival square of the move
 * @param as_side side for which the pawn reached the square
 * @param sequence address of structure to represent the sequence
 * @note If sq_arrival is a promotion square of a side
 *          and sq_arrival is still occupied by a pawn of that side
 *       then *state is initialised with a promotion sequence.
 *       Otherwise, state->promotee will be ==Empty.
 */
void pieces_pawns_start_promotee_sequence(square sq_arrival,
                                          Side as_side,
                                          pieces_pawns_promotion_sequence_type *sequence)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceEnumerator(Side,as_side,"");
  TraceFunctionParamListEnd();

  /* Some fairy chess (e.g. Protean, Kamikaze) prevents promotion by
   * modifying or removing the pawn before we reach here.
   * Only promote the piece if it is still a pawn belonging to the
   * moving side.
   */
  if (is_square_occupied_by_promotable_pawn(sq_arrival,as_side))
  {
    piece_walk_type const walk_moving = get_walk_of_piece_on_square(sq_arrival);
    sequence->selector = (walk_moving==MarinePawn
                          ? pieces_pawns_promotee_chain_marine
                          : pieces_pawns_promotee_chain_orthodox);
    sequence->promotee = pieces_pawns_promotee_sequence[sequence->selector][Empty];
  }
  else
    sequence->promotee = Empty;

  TraceWalk(sequence->promotee);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Continue an iteration over the promotions of a pawn started with an
 * invokation of initialise_pawn_promotion().
 * @param sequence address of structure representing the sequence
 * @note state->promotee==Empty if iteration has ended
 */
void pieces_pawns_continue_promotee_sequence(pieces_pawns_promotion_sequence_type *sequence)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  sequence->promotee = pieces_pawns_promotee_sequence[sequence->selector][sequence->promotee];

  TraceWalk(sequence->promotee);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is a square occupied by a pawn that is to be promoted?
 * @param square_reached square reached by the pawn
 * @param as_side side for which the pawn may occupy the square
 * @return true iff the pawn occupies a prmotion square
 */
boolean is_square_occupied_by_promotable_pawn(square square_reached,
                                              Side as_side)
{
  boolean result = false;
  piece_walk_type const walk_moving = get_walk_of_piece_on_square(square_reached);

  TraceFunctionEntry(__func__);
  TraceSquare(square_reached);
  TraceEnumerator(Side,as_side,"");
  TraceFunctionParamListEnd();

  if (is_pawn(walk_moving))
  {
    boolean const forward = is_forwardpawn(walk_moving);
    if ((forward ? ForwardPromSq(as_side,square_reached) : ReversePromSq(as_side,square_reached))
        && TSTFLAG(being_solved.spec[square_reached],as_side))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the set of promotion pieces for the current twin
 */
void pieces_pawns_init_promotees(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  init_promotee_sequence(pieces_pawns_promotee_chain_orthodox,&standard_walks);

  if (piece_walk_may_exist[MarinePawn])
  {
    standard_walks_type marine_walks;
    marine_walks[King] = Poseidon;
    marine_walks[Queen] = Sirene;
    marine_walks[Rook] = Triton;
    marine_walks[Bishop] = Nereide;
    marine_walks[Knight] = MarineKnight;
    marine_walks[Pawn] = MarinePawn;

    init_promotee_sequence(pieces_pawns_promotee_chain_marine,&marine_walks);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
