#include "solving/moving_pawn_promotion.h"
#include "pydata.h"
#include "pieces/walks.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/moving_pawn_promotion.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

PieNam getprompiece[PieceCount];
static boolean is_current_promotion_marine[maxply+1];
static PieNam getprompiece_marine[PieceCount];
PieNam current_promotion_of_moving[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static void build_promotee_chain(PieNam (* const chain)[PieceCount],
                                 boolean (* const is_promotee)[PieceCount])
{
  PieNam p;
  PieNam prev_prom_piece = Empty;

  for (p = King; p<PieceCount; ++p)
    getprompiece[p] = Empty;

  for (p = King; p<PieceCount; ++p)
    if ((*is_promotee)[p])
    {
      (*chain)[prev_prom_piece] = p;
      prev_prom_piece = p;
    }
}

static void init_promotion_pieces_chain(PieNam (* const chain)[PieceCount],
                                        standard_walks_type * const standard_walks)
{

  if (CondFlag[promotiononly])
    build_promotee_chain(chain,&promonly);
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

    build_promotee_chain(chain,&is_promotee);
  }
}

/* Initialise the set of promotion pieces for the current twin
 */
void init_promotion_pieces(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  init_promotion_pieces_chain(&getprompiece,&standard_walks);

  if (may_exist[MarinePawn])
  {
    standard_walks_type marine_walks;
    marine_walks[King] = Poseidon;
    marine_walks[Queen] = Sirene;
    marine_walks[Rook] = Triton;
    marine_walks[Bishop] = Nereide;
    marine_walks[Knight] = MarineKnight;
    marine_walks[Pawn] = MarinePawn;

    init_promotion_pieces_chain(&getprompiece_marine,&marine_walks);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type moving_pawn_promoter_solve(slice_index si, stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    boolean const is_prom_square = has_pawn_reached_promotion_square(slices[si].starter,
                                                                     sq_arrival);
    is_current_promotion_marine[nbply] = abs(e[sq_arrival])==MarinePawn;
    current_promotion_of_moving[nbply] = is_prom_square ? (is_current_promotion_marine[nbply] ? getprompiece_marine : getprompiece)[Empty] : Empty;
  }

  if (current_promotion_of_moving[nbply]==Empty)
    result = solve(slices[si].next1,n);
  else
  {
    piece const promotee = (e[sq_arrival]<vide
                            ? -current_promotion_of_moving[nbply]
                            : current_promotion_of_moving[nbply]);

    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        sq_arrival,promotee);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_moving[nbply] = (is_current_promotion_marine[nbply] ? getprompiece_marine : getprompiece)[current_promotion_of_moving[nbply]];
      if (current_promotion_of_moving[nbply]!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_moving_pawn_promoters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMovingPawnPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
