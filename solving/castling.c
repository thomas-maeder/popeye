#include "solving/castling.h"
#include "pieces/walks/walks.h"
#include "solving/move_generator.h"
#include "solving/check.h"
#include "solving/conditional_pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/temporary_hacks.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "pieces/walks/classification.h"
#include "position/effects/piece_movement.h"
#include "position/effects/piece_removal.h"
#include "conditions/conditions.h"

#include "debugging/assert.h"

castling_rights_type castling_mutual_exclusive[nr_sides][2];
castling_rights_type castling_flags_no_castling;

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean suspend_castling_is_in_check(slice_index si, Side side_observed)
{
  boolean result;
  Side const side_observing = advers(side_observed);
  castling_rights_type const save_castling_rights = being_solved.castling_rights;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_observed);
  TraceFunctionParamListEnd();

  CLRCASTLINGFLAGMASK(side_observing,k_cancastle);

  result = pipe_is_in_check_recursive_delegate(si,side_observed);

  being_solved.castling_rights = save_castling_rights;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void castle(square sq_departure, square sq_arrival,
                   square sq_partner_departure, square sq_partner_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_partner_departure);
  TraceSquare(sq_partner_arrival);
  TraceFunctionParamListEnd();

  assert(sq_arrival!=nullsquare);

  move_effect_journal_do_no_piece_removal();
  move_effect_journal_do_piece_movement(move_effect_reason_castling_king_movement,
                                        sq_departure,sq_arrival);
  move_effect_journal_do_piece_movement(move_effect_reason_castling_partner,
                                        sq_partner_departure,sq_partner_arrival);

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
void castling_player_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (move_gen_top->capture)
  {
    case kingside_castling:
    {
      square const sq_partner_departure = sq_departure+3*dir_right;
      square const sq_partner_arrival = sq_departure+dir_right;

      castle(sq_departure,sq_arrival,sq_partner_departure,sq_partner_arrival);
      fork_solve_delegate(si);

      break;
    }

    case queenside_castling:
    {
      square const sq_partner_departure = sq_departure+4*dir_left;
      square const sq_partner_arrival = sq_departure+dir_left;

      castle(sq_departure,sq_arrival,sq_partner_departure,sq_partner_arrival);
      fork_solve_delegate(si);

      break;
    }

    default:
      pipe_solve_delegate(si);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Disable a castling right
 * @param reason why
 * @param side from whom
 * @param right which right
 */
static void do_disable_castling_right(move_effect_reason_type reason,
                                      Side side,
                                      castling_rights_type right)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_disable_castling_right,reason);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParam("%u",right);
  TraceFunctionParamListEnd();

  entry->u.castling_rights_adjustment.side = side;
  entry->u.castling_rights_adjustment.right = right;

  CLRCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_disabling_castling_right(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.castling_rights_adjustment.side;
  castling_rights_type const right = entry->u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  SETCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_disabling_castling_right(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.castling_rights_adjustment.side;
  castling_rights_type const right = entry->u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  CLRCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* ensable a castling right
 * @param reason why
 * @param side from whom
 * @param right which right
 */
static void do_enable_castling_right(move_effect_reason_type reason,
                                     Side side,
                                     castling_rights_type right)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_enable_castling_right,reason);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParam("%u",right);
  TraceFunctionParamListEnd();

  entry->u.castling_rights_adjustment.side = side;
  entry->u.castling_rights_adjustment.right = right;

  SETCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo removing a castling right
 * @param curr identifies the adjustment effect
 */
static void move_effect_journal_undo_enabling_castling_right(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.castling_rights_adjustment.side;
  castling_rights_type const right = entry->u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  CLRCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo removing a castling right
 * @param curr identifies the adjustment effect
 */
static void move_effect_journal_redo_enabling_castling_right(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.castling_rights_adjustment.side;
  castling_rights_type const right = entry->u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  SETCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void enable_castling_right(move_effect_reason_type reason,
                                  Side side,
                                  castling_rights_type right)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceValue("%2x",right);
  TraceFunctionParamListEnd();

  if (!TSTCASTLINGFLAGMASK(side,right))
    do_enable_castling_right(reason,side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Enable castling rights for the piece that just arrived (for whatever reason)
 * on a square
 * @param reason why
 * @param on the arrival square
 */
void enable_castling_rights(move_effect_reason_type reason,
                            square sq_arrival)
{
  piece_walk_type const p = get_walk_of_piece_on_square(sq_arrival);
  Flags const specs = being_solved.spec[sq_arrival];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  if (p==standard_walks[Rook])
  {
    if (TSTFLAG(specs,White))
    {
      if (sq_arrival==square_h1)
        enable_castling_right(reason,White,rh_cancastle);
      else if (sq_arrival==square_a1)
        enable_castling_right(reason,White,ra_cancastle);
    }
    if (TSTFLAG(specs,Black))
    {
      if (sq_arrival==square_h8)
        enable_castling_right(reason,Black,rh_cancastle);
      else if (sq_arrival==square_a8)
        enable_castling_right(reason,Black,ra_cancastle);
    }
  }
  else if (p==standard_walks[King])
  {
    if (TSTFLAG(specs,White) && sq_arrival==square_e1)
      enable_castling_right(reason,White,k_cancastle);
    else if (TSTFLAG(specs,Black) && sq_arrival==square_e8)
      enable_castling_right(reason,Black,k_cancastle);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void disable_castling_right(move_effect_reason_type reason,
                                   Side side,
                                   castling_rights_type right)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceValue("%2x",right);
  TraceFunctionParamListEnd();

  if (TSTCASTLINGFLAGMASK(side,right))
    do_disable_castling_right(reason,side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Disable castling rights for the piece that just left (for whatever reason)
 * a square
 * @param reason why
 * @param on the square left
 */
void disable_castling_rights(move_effect_reason_type reason,
                             square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (sq_departure==square_e1)
    disable_castling_right(reason,White,k_cancastle);
  else if (sq_departure==square_e8)
    disable_castling_right(reason,Black,k_cancastle);
  else if (sq_departure==square_h1)
    disable_castling_right(reason,White,rh_cancastle);
  else if (sq_departure==square_a1)
    disable_castling_right(reason,White,ra_cancastle);
  else if (sq_departure==square_h8)
    disable_castling_right(reason,Black,rh_cancastle);
  else if (sq_departure==square_a8)
    disable_castling_right(reason,Black,ra_cancastle);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remove the castling rights according to the current move
 */
static void adjust_castling_rights(Side trait_ply)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply]<=top);

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_movement:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.piece_movement.from);
        break;

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
        enable_castling_rights(move_effect_journal[curr].reason,
                               move_effect_journal[curr].u.piece_addition.added.on);
        break;

      case move_effect_piece_removal:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.piece_removal.on);
        break;

      case move_effect_side_change:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.side_change.on);
        enable_castling_rights(move_effect_journal[curr].reason,
                               move_effect_journal[curr].u.side_change.on);
        break;

      case move_effect_walk_change:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.piece_walk_change.on);
        enable_castling_rights(move_effect_journal[curr].reason,
                               move_effect_journal[curr].u.piece_walk_change.on);
        break;

      default:
        break;
    }

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
void castling_rights_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  adjust_castling_rights(SLICE_STARTER(si));
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void castling_generator_generate_castling(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  if (is_king(move_generation_current_walk))
    generate_castling();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    slice_type const type;
    slice_index landing;
} alternative_move_player_installation_state_type;

static void insert_handler(slice_index si, stip_structure_traversal *st)
{
  alternative_move_player_installation_state_type const * const state = st->param;
  slice_index const proxy = alloc_proxy_slice();
  slice_index const prototype = alloc_fork_slice(state->type,proxy);

  assert(state->landing!=no_slice);
  link_to_branch(proxy,state->landing);

  move_insert_slices(si,st->context,&prototype,1);
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterMovingPieceMovement);
  move_insert_slices(si,st->context,&prototype,1);
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  alternative_move_player_installation_state_type * const state = st->param;
  slice_index const save_landing = state->landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->landing = no_slice;
  insert_landing(si,st);

  stip_traverse_structure_children_pipe(si,st);

  insert_handler(si,st);
  state->landing = save_landing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  alternative_move_player_installation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(state->landing==no_slice);
  stip_traverse_structure_children_pipe(si,st);
  assert(state->landing==no_slice);
  state->landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void insert_alternative_move_players(slice_index si, slice_type type)
{
  stip_structure_traversal st;
  alternative_move_player_installation_state_type state = { type, no_slice };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterMovingPieceMovement,
                                           &remember_landing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with castling
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_castling(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_set_effect_doers(move_effect_disable_castling_right,
                                       &move_effect_journal_undo_disabling_castling_right,
                                       &move_effect_journal_redo_disabling_castling_right);
  move_effect_journal_set_effect_doers(move_effect_enable_castling_right,
                                       &move_effect_journal_undo_enabling_castling_right,
                                       &move_effect_journal_redo_enabling_castling_right);
  solving_instrument_move_generation(si,nr_sides,STCastlingGenerator);

  /* TODO both not necessary behind STCastlingIntermediateMoveLegalityTester */
  insert_alternative_move_players(si,STCastlingPlayer);
  stip_instrument_moves(si,STCastlingRightsAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void castling_generate_test_departure(slice_index si)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  /* TODO this should be the standard implementation, and the one in
   * castling_generator_generate_castling() an optimisation.
   */
  if (is_king(move_generation_current_walk)
      && game_array.board[curr_generation->departure]==move_generation_current_walk)
    generate_castling();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* only generate castling for the king on the original king square */
static void substitute_castling_generator(slice_index si, stip_structure_traversal*st)
{
  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(STCastlingGeneratorTestDeparture));
}

/* make sure that the king's departure square is tested before a castling is
 * generated for a king
 */
void castling_generation_test_departure(slice_index si)
{
  stip_structure_traversal st;
  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCastlingGenerator,
                                           &substitute_castling_generator);
  stip_traverse_structure(si,&st);
}

static void remove_castling_player(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  assert(SLICE_TYPE(SLICE_NEXT2(si))==STProxy);
  dealloc_slice(SLICE_NEXT2(si));

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_pipe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with castling
 * @param si identifies root slice of solving machinery
 */
void solving_disable_castling(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STCastlingPlayer,&remove_castling_player);
  stip_structure_traversal_override_single(&st,STCastlingRightsAdjuster,&remove_pipe);
  stip_structure_traversal_override_single(&st,STCastlingGenerator,&remove_pipe);
  stip_traverse_structure(si,&st);

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
void mutual_castling_rights_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture)
  {
    case kingside_castling:
    {
      castling_rights_type const effectively_disabled = TSTCASTLINGFLAGMASK(advers(SLICE_STARTER(si)),
                                                                          castling_mutual_exclusive[SLICE_STARTER(si)][kingside_castling-min_castling]);
      if (effectively_disabled)
        do_disable_castling_right(move_effect_reason_castling_king_movement,
                                  advers(SLICE_STARTER(si)),
                                  effectively_disabled);
      break;
    }

    case queenside_castling:
    {
      castling_rights_type const effectively_disabled = TSTCASTLINGFLAGMASK(advers(SLICE_STARTER(si)),
                                                                          castling_mutual_exclusive[SLICE_STARTER(si)][queenside_castling-min_castling]);
      if (effectively_disabled)
        do_disable_castling_right(move_effect_reason_castling_king_movement,
                                  advers(SLICE_STARTER(si)),
                                  effectively_disabled);
      break;
    }

    default:
      break;
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with mutual castling right adjusters
 * @param si identifies the root slice of the solving machinery
 */
void solving_insert_mutual_castling_rights_adjusters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMutualCastlingRightsAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Determine whether a sequence of squares are empty
 * @param from start of sequence
 * @param to end of sequence
 * @param direction delta to (repeatedly) apply to reach to from from
 * @return true if the squares between (and not including) from and to are empty
 */
static boolean are_squares_empty(square from, square to, int direction)
{
  square s;
  for (s = from+direction; s!=to; s += direction)
    if (!is_square_empty(s))
      return false;

  return true;
}

boolean castling_is_intermediate_king_move_legal(Side side, square to)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  // TODO there should be a more explicit mechanism that determines whether we have to execute
  // the full move to the intermediate square
  if (CondFlag[imitators] || CondFlag[influencer])
  {
    siblingply(trait[nbply]);

    curr_generation->arrival = to;
    push_move_no_capture();

    result = (conditional_pipe_solve_delegate(temporary_hack_castling_intermediate_move_legality_tester[side])
              ==previous_move_has_solved);

    finply();
  }
  else
  {
    square const from = curr_generation->departure;
    TraceSquare(from);
    TraceEOL();

    occupy_square(to,get_walk_of_piece_on_square(from),being_solved.spec[from]);
    empty_square(from);

    if (being_solved.king_square[side]==initsquare)
      result = !is_in_check(side);
    else
    {
      being_solved.king_square[side] = to;
      result = !is_in_check(side);
      being_solved.king_square[side] = from;
    }

    occupy_square(from,get_walk_of_piece_on_square(to),being_solved.spec[to]);
    empty_square(to);

    curr_generation->departure = from;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void generate_castling(void)
{
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (TSTCASTLINGFLAGMASK(side,castlings)>k_cancastle)
  {
    castling_rights_type allowed_castlings = 0;

    square const square_a = side==White ? square_a1 : square_a8;
    square const square_c = square_a+file_c;
    square const square_d = square_a+file_d;
    square const square_e = square_a+file_e;
    square const square_f = square_a+file_f;
    square const square_g = square_a+file_g;
    square const square_h = square_a+file_h;

    /* avoid castling with the wrong king in conditions like Royal dynasty */
    if (curr_generation->departure==square_e)
    {
      /* 0-0 */
      if (TSTCASTLINGFLAGMASK(side,k_castling)==k_castling
          && are_squares_empty(square_e,square_h,dir_right))
         allowed_castlings |= rh_cancastle;

      /* 0-0-0 */
      if (TSTCASTLINGFLAGMASK(side,q_castling)==q_castling
          && are_squares_empty(square_e,square_a,dir_left))
        allowed_castlings |= ra_cancastle;

      if (allowed_castlings!=0 && !is_in_check(side))
      {
        if ((allowed_castlings&rh_cancastle)
            && castling_is_intermediate_king_move_legal(side,square_f))
        {
          curr_generation->arrival = square_g;
          push_special_move(kingside_castling);
        }

        if ((allowed_castlings&ra_cancastle)
            && castling_is_intermediate_king_move_legal(side,square_d))
        {
          curr_generation->arrival = square_c;
          push_special_move(queenside_castling);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
