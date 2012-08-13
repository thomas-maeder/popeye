#include "conditions/circe/cage.h"
#include "conditions/circe/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/single_piece_move_generator.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];
static boolean cage_found[maxply+1];

static boolean find_non_capturing_move(square sq_departure, Side moving_side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,moving_side,"");
  TraceFunctionParamListEnd();

  init_single_piece_move_generator(sq_departure,e[sq_departure]);
  result = attack(slices[temporary_hack_cagecirce_noncapture_finder[moving_side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void circecage_advance_cage_prom_impl(square cage,
                                             PieNam *circecage_next_cage_prom)
{
  Side const moving_side = trait[nbply];
  Side const prisoner_side = advers(moving_side);
  piece const save_prom = e[cage];

  TraceFunctionEntry(__func__);
  TraceSquare(cage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,prisoner_side,"\n");

  while (true)
  {
    *circecage_next_cage_prom = getprompiece[*circecage_next_cage_prom];
    if (*circecage_next_cage_prom==Empty)
      break;
    else
    {
      TracePiece(*circecage_next_cage_prom);TraceText("\n");
      e[cage] = (prisoner_side==White
                 ? *circecage_next_cage_prom
                 : -*circecage_next_cage_prom);
      if (!find_non_capturing_move(cage,prisoner_side))
        break;
    }
  }

  TracePiece(*circecage_next_cage_prom);TraceText("\n");

  e[cage] = save_prom;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_advance_cage_impl(piece pi_captured,
                                        square *nextcage,
                                        PieNam *circecage_next_cage_prom)
{
  Side const moving_side = trait[nbply];
  Side const prisoner_side = advers(moving_side);

  TraceFunctionEntry(__func__);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,prisoner_side,"\n");

  while (true)
  {
    ++*nextcage;
    if (*nextcage>square_h8)
      break;
    else
    {
      TraceSquare(*nextcage);TraceText("\n");
      if (e[*nextcage]==vide)
      {
        if (is_pawn(pi_captured) && PromSq(prisoner_side,*nextcage))
        {
          circecage_advance_cage_prom_impl(*nextcage,circecage_next_cage_prom);
          if (*circecage_next_cage_prom!=Empty)
            break;
        }
        else
        {
          boolean cage_found;

          e[*nextcage] = pi_captured;

          cage_found = !find_non_capturing_move(*nextcage,prisoner_side);

          e[*nextcage] = vide;

          if (cage_found)
            break;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_advance_norm_prom_impl(square sq_arrival, piece pi_captured,
                                             square *nextcage,
                                             PieNam *circecage_next_cage_prom,
                                             PieNam *circecage_next_norm_prom)
{
  Side const moving_side = trait[nbply];
  piece const save_prom = e[sq_arrival];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  *circecage_next_norm_prom = getprompiece[*circecage_next_norm_prom];
  TracePiece(*circecage_next_norm_prom);TraceText("\n");

  if (*circecage_next_norm_prom!=Empty)
  {
    e[sq_arrival] = (moving_side==White
                     ? *circecage_next_norm_prom
                     : -*circecage_next_norm_prom);
    ++nbpiece[e[sq_arrival]];
    circecage_advance_cage_impl(pi_captured,
                                nextcage,
                                circecage_next_cage_prom);
    --nbpiece[e[sq_arrival]];
  }

  e[sq_arrival] = save_prom;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_find_initial_cage_impl(piece pi_departing,
                                             square sq_arrival, piece pi_captured,
                                             square *nextcage,
                                             PieNam *circecage_next_cage_prom,
                                             PieNam *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TracePiece(pi_departing);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  if (is_pawn(pi_departing) && PromSq(trait[nbply],sq_arrival))
    circecage_advance_norm_prom_impl(sq_arrival,pi_captured,
                                     nextcage,
                                     circecage_next_cage_prom,
                                     circecage_next_norm_prom);
  else
    circecage_advance_cage_impl(pi_captured,
                                nextcage,
                                circecage_next_cage_prom);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean circecage_are_we_finding_cage = false;

void circecage_advance_cage(piece pi_captured,
                            square *nextcage,
                            PieNam *circecage_next_cage_prom)
{
  TraceFunctionEntry(__func__);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *nextcage = square_h8+1;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_cage_impl(pi_captured,nextcage,circecage_next_cage_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void circecage_advance_norm_prom(square sq_arrival, piece pi_captured,
                                 square *nextcage,
                                 PieNam *circecage_next_cage_prom,
                                 PieNam *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *circecage_next_norm_prom = Empty;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_norm_prom_impl(sq_arrival,pi_captured,nextcage,circecage_next_cage_prom,circecage_next_norm_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void circecage_advance_cage_prom(square cage,
                                 PieNam *circecage_next_cage_prom)
{
  TraceFunctionEntry(__func__);
  TraceSquare(cage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *circecage_next_cage_prom = Empty;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_cage_prom_impl(cage,circecage_next_cage_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

square rencage(ply ply_id,
               piece p_captured, Flags p_captured_spec,
               square sq_capture,
               square sq_departure, square sq_arrival,
               Side capturer)
{
  square result = square_a1-1;
  PieNam nextcageprom = vide;
  PieNam nextnormprom = vide;
  piece const pi_departing = e[sq_departure];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(p_captured);
  TraceSquare(sq_capture);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TracePiece(e[sq_departure]);
  TracePiece(e[sq_capture]);
  TracePiece(e[sq_arrival]);
  TraceText("\n");

  if (!circecage_are_we_finding_cage)
  {
    circecage_are_we_finding_cage = true;

    e[sq_departure] = vide;
    e[sq_capture] = vide;
    e[sq_arrival] = pi_departing;

    circecage_find_initial_cage_impl(pi_departing,sq_capture,p_captured,
                                     &result,&nextcageprom,&nextnormprom);

    e[sq_arrival] = vide;
    e[sq_capture] = p_captured;
    e[sq_departure] = pi_departing;

    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static square advance_rebirth_square(square current)
{
  TraceFunctionEntry(__func__);
  TraceSquare(current);
  TraceFunctionParamListEnd();

  do
  {
    ++current;
  } while (current<=square_h8 && e[current]!=vide);

  TraceSquare(current);TraceText("\n");

  TraceFunctionExit(__func__);
  TraceSquare(current);
  TraceFunctionResultEnd();
  return current;
}

static square init_rebirth_square(void)
{
  square result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = advance_rebirth_square(square_a1-1);
  cage_found[nbply] = false;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_rebirth_handler_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  square const pi_captured = pprise[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pi_captured==vide)
  {
    current_circe_rebirth_square[nbply] = initsquare;
    result = attack(slices[si].next1,n);
  }
  else
  {
    square sq_rebirth;
    if (post_move_iteration_id[nbply]==prev_post_move_iteration_id[nbply])
      sq_rebirth = current_circe_rebirth_square[nbply];
    else
      sq_rebirth = init_rebirth_square();

    if (sq_rebirth!=initsquare && sq_rebirth<=square_h8)
    {
      /* rebirth on current cage */
      circe_do_rebirth(sq_rebirth,pi_captured,pprispec[nbply]);
      result = attack(slices[si].next1,n);
      circe_undo_rebirth(sq_rebirth);

      if (!post_move_iteration_locked[nbply])
      {
        current_circe_rebirth_square[nbply] = advance_rebirth_square(sq_rebirth);
        if (current_circe_rebirth_square[nbply]<=square_h8 || !cage_found[nbply])
          lock_post_move_iterations();
      }

      prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
    }
    else if (cage_found[nbply])
      /* all >0 cages have been tried */
      result = n+2;
    else
    {
      /* there is no cage */
      current_circe_rebirth_square[nbply] = initsquare;
      result = attack(slices[si].next1,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type circe_cage_rebirth_handler_defend(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  square const pi_captured = pprise[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pi_captured==vide)
  {
    current_circe_rebirth_square[nbply] = initsquare;
    result = defend(slices[si].next1,n);
  }
  else
  {
    square sq_rebirth;
    if (post_move_iteration_id[nbply]==prev_post_move_iteration_id[nbply])
      sq_rebirth = current_circe_rebirth_square[nbply];
    else
      sq_rebirth = init_rebirth_square();

    if (sq_rebirth!=initsquare && sq_rebirth<=square_h8)
    {
      /* rebirth on current cage */
      circe_do_rebirth(sq_rebirth,pi_captured,pprispec[nbply]);
      result = defend(slices[si].next1,n);
      circe_undo_rebirth(sq_rebirth);

      if (!post_move_iteration_locked[nbply])
      {
        current_circe_rebirth_square[nbply] = advance_rebirth_square(sq_rebirth);
        if (current_circe_rebirth_square[nbply]<=square_h8 || !cage_found[nbply])
          lock_post_move_iterations();
      }

      prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
    }
    else if (cage_found[nbply])
      /* all >0 cages have been tried */
      result = n+2;
    else
    {
      /* there is no cage */
      current_circe_rebirth_square[nbply] = initsquare;
      result = defend(slices[si].next1,n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_non_capturing_move_locked(square sq_departure, Side moving_side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,moving_side,"");
  TraceFunctionParamListEnd();

  circecage_are_we_finding_cage = true;
  result = find_non_capturing_move(sq_departure,moving_side);
  circecage_are_we_finding_cage = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_cage_tester_attack(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  square const sq_cage = current_circe_rebirth_square[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_cage==initsquare)
    result = attack(slices[si].next1,n);
  else if (find_non_capturing_move_locked(sq_cage,advers(slices[si].starter)))
    result = n+2;
  else
  {
    cage_found[nbply] = true;
    result = attack(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type circe_cage_cage_tester_defend(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  square const sq_cage = current_circe_rebirth_square[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_cage==initsquare)
    result = defend(slices[si].next1,n);
  else if (find_non_capturing_move_locked(sq_cage,advers(slices[si].starter)))
    result = slack_length-1;
  else
  {
    cage_found[nbply] = true;
    result = defend(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STCirceCageRebirthHandler),
        alloc_pipe(STCirceRebirthPromoter),
        alloc_pipe(STCirceCageCageTester)
    };

    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,prototypes,3);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,prototypes,3);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,prototypes,3);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static void instrument_move_replay(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STCirceCageRebirthHandler),
        alloc_pipe(STCirceRebirthPromoter)
    };

    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,prototypes,2);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,prototypes,2);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,prototypes,2);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_cage(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move_replay);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
