#include "pieces/pawns/promotion.h"
#include "pieces/walks.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

PieNam pieces_pawns_promotee_chain[pieces_pawns_nr_promotee_chains][PieceCount];

static void build_promotee_chain(pieces_pawns_promotee_chain_selector_type selector,
                                 boolean (* const is_promotee)[PieceCount])
{
  PieNam p;
  PieNam prev_prom_piece = Empty;

  for (p = King; p<PieceCount; ++p)
    pieces_pawns_promotee_chain[selector][p] = Empty;

  for (p = King; p<PieceCount; ++p)
    if ((*is_promotee)[p])
    {
      pieces_pawns_promotee_chain[selector][prev_prom_piece] = p;
      prev_prom_piece = p;
    }
}

static void init_promotion_pieces_chain(pieces_pawns_promotee_chain_selector_type selector,
                                        standard_walks_type * const standard_walks)
{

  if (CondFlag[promotiononly])
    build_promotee_chain(selector,&promonly);
  else
  {
    boolean is_promotee[PieceCount] = { false };
    PieNam p;

    for (p = Queen; p<=Bishop; ++p)
      is_promotee[(*standard_walks)[p]] = true;

    for (p = King+1; p<PieceCount; ++p)
      if (exist[p] && !is_pawn(p) && !is_king(p))
        is_promotee[p] = true;

    is_promotee[Dummy] = false;

    if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
    {
      is_promotee[(*standard_walks)[King]] = true;

      for (p = Bishop+1; p<PieceCount; ++p)
        if (exist[p] && is_king(p))
          is_promotee[p] = true;
    }

    if (CondFlag[singlebox] && SingleBoxType!=singlebox_type1)
    {
      is_promotee[(*standard_walks)[Pawn]] = true;

      for (p = Bishop+1; p<PieceCount; ++p)
        if (exist[p] && is_pawn(p))
          is_promotee[p] = true;
    }

    build_promotee_chain(selector,&is_promotee);
  }
}

/* Initialise a sequence of promotions
 * @param sq_arrival arrival square of the move
 * @param sequence address of structure to represent the sequence
 * @note If sq_arrival is a promotion square of a side
 *          and sq_arrival is still occupied by a pawn of that side
 *       then *state is initialised with a promotion sequence.
 *       Otherwise, state->promotee will be ==Empty.
 */
void pieces_pawns_initialise_promotion_sequence(square sq_arrival,
                                                pieces_pawns_promotion_sequence_type *sequence)
{
  /* Some fairy chess (e.g. Protean,Kamikaze) prevents promotion by
   * modifying or removing the pawn before we reach here.
   * Only promote the piece if it is still a pawn belonging to the
   * moving side.
   */
  if (is_square_occupied_by_promotable_pawn(sq_arrival)!=no_side)
  {
    PieNam const walk_moving = abs(e[sq_arrival]);
    sequence->selector = (walk_moving==MarinePawn
                          ? pieces_pawns_promotee_chain_marine
                          : pieces_pawns_promotee_chain_orthodox);
    sequence->promotee = pieces_pawns_promotee_chain[sequence->selector][Empty];
    TracePiece(sequence->promotee);
  }
  else
    sequence->promotee = Empty;
}

/* Continue an iteration over the promotions of a pawn started with an
 * invokation of initialise_pawn_promotion().
 * @param sequence address of structure representing the sequence
 * @note state->promotee==Empty if iteration has ended
 */
void pieces_pawns_continue_promotion_sequence(pieces_pawns_promotion_sequence_type *sequence)
{
  sequence->promotee = pieces_pawns_promotee_chain[sequence->selector][sequence->promotee];
}

/* Is a square occupied by a pawn that is to be promoted?
 * @param square_reached square reached by the pawn
 * @return side for which the pawn has reached the promotion square
 *         no_side if the pawn hasn't
 */
Side is_square_occupied_by_promotable_pawn(square square_reached)
{
  Side result = no_side;
  PieNam const walk_moving = abs(e[square_reached]);

  TraceFunctionEntry(__func__);
  TraceSquare(square_reached);
  TraceFunctionParamListEnd();

  if (is_pawn(walk_moving))
  {
    boolean const forward = is_forwardpawn(walk_moving);
    if ((forward ? ForwardPromSq(White,square_reached) : ReversePromSq(White,square_reached))
        && TSTFLAG(spec[square_reached],White))
      result = White;
    else if ((forward ? ForwardPromSq(Black,square_reached) : ReversePromSq(Black,square_reached))
             && TSTFLAG(spec[square_reached],Black))
      result = Black;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(Side,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the set of promotion pieces for the current twin
 */
void pieces_pawns_init_promotion_pieces(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  init_promotion_pieces_chain(pieces_pawns_promotee_chain_orthodox,&standard_walks);

  if (may_exist[MarinePawn])
  {
    standard_walks_type marine_walks;
    marine_walks[King] = Poseidon;
    marine_walks[Queen] = Sirene;
    marine_walks[Rook] = Triton;
    marine_walks[Bishop] = Nereide;
    marine_walks[Knight] = MarineKnight;
    marine_walks[Pawn] = MarinePawn;

    init_promotion_pieces_chain(pieces_pawns_promotee_chain_marine,&marine_walks);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
