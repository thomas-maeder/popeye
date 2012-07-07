#include "conditions/circe/cage.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/temporary_hacks.h"
#include "solving/single_piece_move_generator.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>


static boolean find_non_capturing_move(ply ply_id,
                                       square sq_departure,
                                       Side moving_side,
                                       piece p_moving)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,moving_side,"");
  TracePiece(p_moving);
  TraceFunctionParamListEnd();

  init_single_piece_move_generator(sq_departure,p_moving);
  result = attack(slices[temporary_hack_cagecirce_noncapture_finder[moving_side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void circecage_advance_cage_prom_impl(ply ply_id,
                                             square cage,
                                             piece *circecage_next_cage_prom)
{
  Side const moving_side = trait[ply_id];
  Side const prisoner_side = advers(moving_side);
  piece const save_prom = e[cage];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(cage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,prisoner_side,"\n");

  while (true)
  {
    *circecage_next_cage_prom = getprompiece[*circecage_next_cage_prom];
    if (*circecage_next_cage_prom==vide)
      break;
    else
    {
      TracePiece(*circecage_next_cage_prom);TraceText("\n");
      e[cage] = (prisoner_side==White
                 ? *circecage_next_cage_prom
                 : -*circecage_next_cage_prom);
      if (!find_non_capturing_move(ply_id,cage,prisoner_side,e[cage]))
        break;
    }
  }

  TracePiece(*circecage_next_cage_prom);TraceText("\n");

  e[cage] = save_prom;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_advance_cage_impl(ply ply_id,
                                        piece pi_captured,
                                        square *nextcage,
                                        piece *circecage_next_cage_prom)
{
  Side const moving_side = trait[ply_id];
  Side const prisoner_side = advers(moving_side);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
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
          circecage_advance_cage_prom_impl(ply_id,*nextcage,circecage_next_cage_prom);
          if (*circecage_next_cage_prom!=vide)
            break;
        }
        else
        {
          boolean cage_found;

          e[*nextcage] = pi_captured;

          cage_found = !find_non_capturing_move(ply_id,
                                                *nextcage,
                                                prisoner_side,
                                                pi_captured);

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

static void circecage_advance_norm_prom_impl(ply ply_id,
                                             square sq_arrival, piece pi_captured,
                                             square *nextcage,
                                             piece *circecage_next_cage_prom,
                                             piece *circecage_next_norm_prom)
{
  Side const moving_side = trait[ply_id];
  piece const save_prom = e[sq_arrival];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  *circecage_next_norm_prom = getprompiece[*circecage_next_norm_prom];
  TracePiece(*circecage_next_norm_prom);TraceText("\n");

  if (*circecage_next_norm_prom!=vide)
  {
    e[sq_arrival] = (moving_side==White
                     ? *circecage_next_norm_prom
                     : -*circecage_next_norm_prom);
    ++nbpiece[e[sq_arrival]];
    circecage_advance_cage_impl(nbply,
                                pi_captured,
                                nextcage,
                                circecage_next_cage_prom);
    --nbpiece[e[sq_arrival]];
  }

  e[sq_arrival] = save_prom;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_find_initial_cage_impl(ply ply_id,
                                             piece pi_departing,
                                             square sq_arrival, piece pi_captured,
                                             square *nextcage,
                                             piece *circecage_next_cage_prom,
                                             piece *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(pi_departing);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  if (is_pawn(pi_departing) && PromSq(trait[ply_id],sq_arrival))
    circecage_advance_norm_prom_impl(ply_id,
                                     sq_arrival,pi_captured,
                                     nextcage,
                                     circecage_next_cage_prom,
                                     circecage_next_norm_prom);
  else
    circecage_advance_cage_impl(ply_id,
                                pi_captured,
                                nextcage,
                                circecage_next_cage_prom);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean circecage_are_we_finding_cage = false;

void circecage_advance_cage(ply ply_id,
                            piece pi_captured,
                            square *nextcage,
                            piece *circecage_next_cage_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *nextcage = square_h8+1;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_cage_impl(ply_id,pi_captured,nextcage,circecage_next_cage_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void circecage_advance_norm_prom(ply ply_id,
                                 square sq_arrival, piece pi_captured,
                                 square *nextcage,
                                 piece *circecage_next_cage_prom,
                                 piece *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *circecage_next_norm_prom = vide;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_norm_prom_impl(ply_id,sq_arrival,pi_captured,nextcage,circecage_next_cage_prom,circecage_next_norm_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void circecage_advance_cage_prom(ply ply_id,
                                 square cage,
                                 piece *circecage_next_cage_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceSquare(cage);
  TracePiece(*circecage_next_cage_prom);
  TraceFunctionParamListEnd();

  if (circecage_are_we_finding_cage)
    *circecage_next_cage_prom = vide;
  else
  {
    circecage_are_we_finding_cage = true;
    circecage_advance_cage_prom_impl(ply_id,cage,circecage_next_cage_prom);
    circecage_are_we_finding_cage = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void circecage_find_initial_cage(ply ply_id,
                                        piece pi_departing,
                                        square sq_arrival, piece pi_captured,
                                        square *nextcage,
                                        piece *circecage_next_cage_prom,
                                        piece *circecage_next_norm_prom)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TracePiece(pi_departing);
  TraceSquare(sq_arrival);
  TracePiece(pi_captured);
  TraceSquare(*nextcage);
  TracePiece(*circecage_next_cage_prom);
  TracePiece(*circecage_next_norm_prom);
  TraceFunctionParamListEnd();


  *nextcage = superbas;

  if (!circecage_are_we_finding_cage)
  {
    circecage_are_we_finding_cage = true;
    circecage_find_initial_cage_impl(ply_id,
                                     pi_departing,sq_arrival,pi_captured,
                                     nextcage,
                                     circecage_next_cage_prom,
                                     circecage_next_norm_prom);
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
  square result = superbas;
  piece nextcageprom = vide;
  piece nextnormprom = vide;
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

    circecage_find_initial_cage_impl(ply_id,
                                     pi_departing,sq_capture,p_captured,
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

static void handle_rebirth(Side trait_ply)
{
  square const pi_captured = pprise[nbply];
  square const pi_departing = pjoue[nbply];
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  if (pi_captured!=vide && current_super_circe_rebirth_square[nbply]==superbas)
  {
    current_promotion_of_moving[nbply] = vide;
    current_promotion_of_reborn[nbply] = vide;
    circecage_find_initial_cage(nbply,
                                pi_departing,sq_arrival,pi_captured,
                                &current_super_circe_rebirth_square[nbply],
                                &current_promotion_of_reborn[nbply],
                                &current_promotion_of_moving[nbply]);
  }
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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  handle_rebirth(slices[si].starter);
  result = attack(slices[si].next1,n);

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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  handle_rebirth(slices[si].starter);
  result = defend(slices[si].next1,n);

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

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STCirceCageRebirthHandler);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
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
void stip_insert_circe_cage_rebirth_handlers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
