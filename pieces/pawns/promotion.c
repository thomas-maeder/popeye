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
    pieces_pawns_promotee_chain[pieces_pawns_promotee_chain_orthodox][p] = Empty;

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
      if (exist[p] && !is_pawn(p))
        is_promotee[p] = true;

    is_promotee[Dummy] = false;

    if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
      is_promotee[(*standard_walks)[King]] = true;

    if (CondFlag[singlebox] && SingleBoxType!=singlebox_type1)
      for (p = King; p<PieceCount; ++p)
        if (exist[p] && is_pawn(p))
          is_promotee[p] = true;

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
  PieNam const walk_moving = abs(e[sq_arrival]);
  boolean const reverse = is_reversepawn(walk_moving);

  /* Some fairy chess (e.g. Protean,Kamikaze) prevents promotion by
   * modifying or removing the pawn before we reach here.
   * Only promote the piece if it is still a pawn belonging to the
   * moving side.
   */
  if (is_pawn(walk_moving)
      && ((PromSq(White,sq_arrival) && TSTFLAG(spec[sq_arrival],White^reverse))
          || (PromSq(Black,sq_arrival) && TSTFLAG(spec[sq_arrival],Black^reverse))))
  {
    sequence->selector = (walk_moving==MarinePawn
                          ? pieces_pawns_promotee_chain_marine
                          : pieces_pawns_promotee_chain_orthodox);
    sequence->promotee = pieces_pawns_promotee_chain[sequence->selector][Empty];
    TracePiece(sequence->promotee);
    TraceValue("%u\n",sequence->selector);
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

/* Has a pawn reached a promotion square
 * @param side the pawn's side
 * @param square_reached square reached by the pawn
 * @return true iff square_reached is a promotion square
 */
boolean has_pawn_reached_promotion_square(Side side, square square_reached)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(square_reached);
  TraceFunctionParamListEnd();

  result = (is_pawn(abs(e[square_reached]))
            && PromSq(is_reversepawn(abs(e[square_reached]))^side,square_reached)
            && ((!CondFlag[protean] && !TSTFLAG(spec[square_reached],Protean))
                || pprise[nbply]==vide));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
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
