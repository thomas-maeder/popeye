#include "solving/castling.h"
#include "pieces/walks/walks.h"
#include "solving/move_generator.h"
#include "solving/check.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

castling_flag_type castling_flag;
castling_flag_type castling_mutual_exclusive[nr_sides][2];
castling_flag_type castling_flags_no_castling;

static square intermediate_move_square_arrival;

/* Allocate a STCastlingIntermediateMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_intermediate_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCastlingIntermediateMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the next1 move generation
 * @param sq_arrival arrival square of move to be generated
 */
void castling_intermediate_move_generator_init_next(square sq_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  intermediate_move_square_arrival = sq_arrival;

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
stip_length_type castling_intermediate_move_generator_solve(slice_index si,
                                                            stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  numecoup const save_repere = current_move[parent_ply[nbply]-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* We work within a ply for which moves are being generated right now.
   * That's why we don't do nextply()/finply() - we just trick our successor
   * slices into believing that this intermediate move is the only one in the
   * ply.
   */
  current_move[parent_ply[nbply]-1] = current_move[nbply]-1;
  curr_generation->arrival = intermediate_move_square_arrival;
  push_move();
  result = solve(next,n);
  current_move[parent_ply[nbply]-1] = save_repere;

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
  move_effect_journal_do_piece_movement(move_effect_reason_castling_partner_movement,
                                        sq_partner_departure,sq_partner_arrival);

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
stip_length_type castling_player_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply]-1;
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (move_gen_top->capture)
  {
    case kingside_castling:
    {
      square const sq_partner_departure = sq_departure+3*dir_right;
      square const sq_partner_arrival = sq_departure+dir_right;

      castle(sq_departure,sq_arrival,sq_partner_departure,sq_partner_arrival);
      result = solve(slices[si].next2,n);

      break;
    }

    case queenside_castling:
    {
      square const sq_partner_departure = sq_departure+4*dir_left;
      square const sq_partner_arrival = sq_departure+dir_left;

      castle(sq_departure,sq_arrival,sq_partner_departure,sq_partner_arrival);
      result = solve(slices[si].next2,n);

      break;
    }

    default:
      result = solve(slices[si].next1,n);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Disable a castling right
 * @param reason why
 * @param side from whom
 * @param right which right
 */
static void do_disable_castling_right(move_effect_reason_type reason,
                                      Side side,
                                      castling_flag_type right)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",right);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_disable_castling_right;
  top_elmt->reason = reason;
  top_elmt->u.castling_rights_adjustment.side = side;
  top_elmt->u.castling_rights_adjustment.right = right;
 #if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
 #endif

  ++move_effect_journal_top[nbply];

  CLRCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_disabling_castling_right(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.castling_rights_adjustment.side;
  castling_flag_type const right = move_effect_journal[curr].u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  SETCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_disabling_castling_right(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.castling_rights_adjustment.side;
  castling_flag_type const right = move_effect_journal[curr].u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

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
                                     castling_flag_type right)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",right);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_enable_castling_right;
  top_elmt->reason = reason;
  top_elmt->u.castling_rights_adjustment.side = side;
  top_elmt->u.castling_rights_adjustment.right = right;
 #if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
 #endif

  ++move_effect_journal_top[nbply];

  SETCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_enabling_castling_right(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.castling_rights_adjustment.side;
  castling_flag_type const right = move_effect_journal[curr].u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  CLRCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_enabling_castling_right(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.castling_rights_adjustment.side;
  castling_flag_type const right = move_effect_journal[curr].u.castling_rights_adjustment.right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  SETCASTLINGFLAGMASK(side,right);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void enable_castling_right(move_effect_reason_type reason,
                                  Side side,
                                  castling_flag_type right)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
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
  PieNam const p = get_walk_of_piece_on_square(sq_arrival);
  Flags const specs = spec[sq_arrival];

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
                                   castling_flag_type right)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
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
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply-1]<=top);

  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_movement:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.piece_movement.from);
        break;

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
        enable_castling_rights(move_effect_journal[curr].reason,
                               move_effect_journal[curr].u.piece_addition.on);
        break;

      case move_effect_piece_removal:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.piece_removal.from);
        break;

      case move_effect_side_change:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.side_change.on);
        enable_castling_rights(move_effect_journal[curr].reason,
                               move_effect_journal[curr].u.side_change.on);
        break;

      case move_effect_piece_change:
        disable_castling_rights(move_effect_journal[curr].reason,
                                move_effect_journal[curr].u.piece_change.on);
        enable_castling_rights(move_effect_journal[curr].reason,
                               move_effect_journal[curr].u.piece_change.on);
        break;

      default:
        break;
    }

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
stip_length_type castling_rights_adjuster_solve(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  adjust_castling_rights(slices[si].starter);
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void castling_generator_generate_castling(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,p);

  if (p==King || p==Poseidon)
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

  branch_insert_slices_contextual(si,st->context,&prototype,1);
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterMovingPieceMovement);
  branch_insert_slices_contextual(si,st->context,&prototype,1);
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

  solving_instrument_move_generation(si,nr_sides,STCastlingGenerator);
  insert_alternative_move_players(si,STCastlingPlayer);
  stip_instrument_moves(si,STCastlingRightsAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_castling_player(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  assert(slices[slices[si].next2].type==STProxy);
  dealloc_slice(slices[si].next2);

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
stip_length_type mutual_castling_rights_adjuster_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  switch (move_generation_stack[current_move[nbply]-1].capture)
  {
    case kingside_castling:
    {
      castling_flag_type const effectively_disabled = TSTCASTLINGFLAGMASK(advers(slices[si].starter),
                                                                          castling_mutual_exclusive[slices[si].starter][kingside_castling-min_castling]);
      if (effectively_disabled)
        do_disable_castling_right(move_effect_reason_castling_king_movement,
                                  advers(slices[si].starter),
                                  effectively_disabled);
      break;
    }

    case queenside_castling:
    {
      castling_flag_type const effectively_disabled = TSTCASTLINGFLAGMASK(advers(slices[si].starter),
                                                                          castling_mutual_exclusive[slices[si].starter][queenside_castling-min_castling]);
      if (effectively_disabled)
        do_disable_castling_right(move_effect_reason_castling_king_movement,
                                  advers(slices[si].starter),
                                  effectively_disabled);
      break;
    }

    default:
      break;
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_mutual_castling_rights_adjusters(slice_index si)
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
  boolean result = false;

  if (CondFlag[imitators])
  {
    castling_intermediate_move_generator_init_next(to);
    result = solve(slices[temporary_hack_castling_intermediate_move_legality_tester[side]].next2,length_unspecified)==next_move_has_solution;
  }
  else
  {
    square const from = curr_generation->departure;
    occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
    empty_square(from);

    if (king_square[side]!=initsquare)
      king_square[side] = to;

    result = !echecc(side);

    occupy_square(from,get_walk_of_piece_on_square(to),spec[to]);
    empty_square(to);

    if (king_square[side]!=initsquare)
      king_square[side] = from;
  }

  return result;
}

void generate_castling(void)
{
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (TSTCASTLINGFLAGMASK(side,castlings)>k_cancastle)
  {
    castling_flag_type allowed_castlings = 0;

    square const square_a = side==White ? square_a1 : square_a8;
    square const square_c = square_a+file_c;
    square const square_d = square_a+file_d;
    square const square_e = square_a+file_e;
    square const square_f = square_a+file_f;
    square const square_g = square_a+file_g;
    square const square_h = square_a+file_h;

    /* 0-0 */
    if (TSTCASTLINGFLAGMASK(side,k_castling)==k_castling
        && are_squares_empty(square_e,square_h,dir_right))
       allowed_castlings |= rh_cancastle;

    /* 0-0-0 */
    if (TSTCASTLINGFLAGMASK(side,q_castling)==q_castling
        && are_squares_empty(square_e,square_a,dir_left))
      allowed_castlings |= ra_cancastle;

    if (allowed_castlings!=0 && !echecc(side))
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
