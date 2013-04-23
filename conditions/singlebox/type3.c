#include "conditions/singlebox/type3.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* Determine whether the move just played is legal according to Singlebox Type 3
 * @return true iff the move is legal
 */
static boolean is_last_move_illegal(void)
{
  boolean result = false;

  if (singlebox_type1_illegal())
    result = true;
  else if ((trait[nbply]==White && singlebox_illegal_latent_white_pawn())
           || (trait[nbply]==Black && singlebox_illegal_latent_black_pawn()))
    result = true;

  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type3(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSingleBoxType3LegalityTester);
  stip_instrument_moves(si,STSingleBoxType3PawnPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type singlebox_type3_pawn_promoter_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_generation_stack[coup_id].singlebox_type3_promotion_what!=vide)
    move_effect_journal_do_piece_change(move_effect_reason_singlebox_promotion,
                                        move_generation_stack[coup_id].singlebox_type3_promotion_where,
                                        move_generation_stack[coup_id].singlebox_type3_promotion_what);

  result = solve(slices[si].next1,n);

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
stip_length_type singlebox_type3_legality_tester_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_last_move_illegal())
    result = previous_move_is_illegal;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean singleboxtype3_is_black_king_square_attacked(evalfunction_t *evaluate)
{
  unsigned int promotionstried = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,White);
       sq!=vide;
       sq = next_latent_pawn(sq,White))
  {
    PieNam pprom;
    for (pprom = next_singlebox_prom(Empty,White);
         pprom!=Empty;
         pprom = next_singlebox_prom(pprom,White))
    {
      boolean result;
      ++promotionstried;
      e[sq] = pprom;
      ++nbpiece[pprom];
      result = is_black_king_square_attacked(evaluate);
      --nbpiece[pprom];
      e[sq] = pb;
      if (result) {
        return true;
      }
    }
  }

  return promotionstried==0 && is_black_king_square_attacked(evaluate);
}

boolean singleboxtype3_is_white_king_square_attacked(evalfunction_t *evaluate)
{
  unsigned int promotionstried = 0;
  square sq;

  for (sq = next_latent_pawn(initsquare,Black);
       sq!=vide;
       sq = next_latent_pawn(sq,Black))
  {
    PieNam pprom;
    for (pprom = next_singlebox_prom(Empty,Black);
         pprom!=Empty;
         pprom = next_singlebox_prom(pprom,Black))
    {
      boolean result;
      ++promotionstried;
      e[sq] = -pprom;
      ++nbpiece[-(piece)pprom];
      result = is_white_king_square_attacked(evaluate);
      --nbpiece[-(piece)pprom];
      e[sq] = pn;
      if (result) {
        return true;
      }
    }
  }

  return promotionstried==0 && is_white_king_square_attacked(evaluate);
}

static square find_next_latent_pawn(square sq, Side side)
{
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  while (sq!=square_h8)
  {
    if (sq==square_h1)
      sq = square_a8;
    else
      sq += dir_right;

    {
      PieNam const walk_promotee = abs(e[sq]);
      if (is_pawn(walk_promotee)
          && TSTFLAG(spec[sq],side)
          && PromSq(is_reversepawn(walk_promotee) ? advers(side) : side, sq))
      {
        result = sq;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate the moves for a black/white piece
 * @param p side for which to generate moves
 * @param sq_departure departure square of the moves
 * @param p walk and side of the piece
 */
static void singleboxtype3_generate_moves_for_piece(Side side, square sq_departure, piece p)
{
  numecoup save_nbcou = current_move[nbply];
  unsigned int nr_latent_promotions = 0;
  square sq;
  for (sq = find_next_latent_pawn(square_a1-dir_right,side);
       sq!=initsquare;
       sq = find_next_latent_pawn(sq,side))
  {
    Side promoting_side;
    pieces_pawns_promotion_sequence_type sequence;
    singlebox_type2_initialise_singlebox_promotion_sequence(sq,&promoting_side,&sequence);
    assert(promoting_side==side);
    while (sequence.promotee!=Empty)
    {
      piece const pi_departing = e[sq];
      piece const pi_promotee = sequence.promotee;
      numecoup prev_nbcou = current_move[nbply];
      ++nr_latent_promotions;
      e[sq] = pi_promotee;
      orig_generate_moves_for_piece(side, sq_departure, sq==sq_departure ? pi_promotee : p);
      e[sq] = pi_departing;
      for (++prev_nbcou; prev_nbcou<=current_move[nbply]; ++prev_nbcou)
      {
        move_generation_stack[prev_nbcou].singlebox_type3_promotion_where = sq;
        move_generation_stack[prev_nbcou].singlebox_type3_promotion_what = pi_promotee;
      }
      singlebox_type2_continue_singlebox_promotion_sequence(promoting_side,&sequence);
    }
  }

  if (nr_latent_promotions==0)
  {
    orig_generate_moves_for_piece(side,sq_departure,p);

    for (++save_nbcou; save_nbcou<=current_move[nbply]; ++save_nbcou)
    {
      move_generation_stack[save_nbcou].singlebox_type3_promotion_where = initsquare;
      move_generation_stack[save_nbcou].singlebox_type3_promotion_what = vide;
    }
  }
}

/* Generate the moves for a black/white piece
 * @param sq_departure departure square of the moves
 * @param p walk and side of the piece
 */
void singleboxtype3_generate_moves_for_white_piece(square sq_departure, piece p)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  singleboxtype3_generate_moves_for_piece(White,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void singleboxtype3_generate_moves_for_black_piece(square sq_departure, piece p)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  singleboxtype3_generate_moves_for_piece(Black,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
