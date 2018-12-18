#include "conditions/make_and_take.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "solving/check.h"
#include "solving/castling.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/pawns/en_passant.h"
#include "position/position.h"
#include "position/effects/piece_movement.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <string.h>

static boolean is_false(numecoup n)
{
  return false;
}

static piece_walk_type max_victim = nr_piece_walks-1;

typedef struct {
    numecoup min_move_id;
    numecoup max_move_id;
} move_ids_with_castling_as_make_one_side_type;

typedef struct {
    move_ids_with_castling_as_make_one_side_type king_side;
    move_ids_with_castling_as_make_one_side_type queen_side;
} move_ids_with_castling_as_make_type;

static move_ids_with_castling_as_make_type move_ids_with_castling_as_make[maxply+1];

static void remember_move_ids_of_castlings_as_makes(square sq_capture,
                                                    numecoup min_move_id,
                                                    numecoup max_move_id,
                                                    ply generating_for)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_capture);
  TraceFunctionParam("%u",min_move_id);
  TraceFunctionParam("%u",max_move_id);
  TraceFunctionParam("%u",generating_for);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);TraceValue("%u",generating_for);TraceEOL();

  assert(generating_for!=ply_nil);

  if (sq_capture==kingside_castling)
  {
    assert(move_ids_with_castling_as_make[generating_for].king_side.min_move_id==0);
    assert(move_ids_with_castling_as_make[generating_for].king_side.max_move_id==0);
    move_ids_with_castling_as_make[generating_for].king_side.min_move_id = min_move_id;
    move_ids_with_castling_as_make[generating_for].king_side.max_move_id = max_move_id;
  }

  if (sq_capture==queenside_castling)
  {
    assert(move_ids_with_castling_as_make[generating_for].queen_side.min_move_id==0);
    assert(move_ids_with_castling_as_make[generating_for].queen_side.max_move_id==0);
    move_ids_with_castling_as_make[generating_for].queen_side.min_move_id = min_move_id;
    move_ids_with_castling_as_make[generating_for].queen_side.max_move_id = max_move_id;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean make_and_take_limit_move_generation_make_walk_is_in_check(slice_index si,
                                                                  Side side_observed)
{
  boolean result;

  piece_walk_type const save_max_victim = max_victim;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_observed);
  TraceFunctionParamListEnd();

  max_victim = King;

  result = pipe_is_in_check_recursive_delegate(si,side_observed);

  max_victim = save_max_victim;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void generate_castling_as_make(void)
{
  castling_rights_type const save_castling_rights = being_solved.castling_rights;
  Side const side_victim = trait[nbply];
  Flags const save_spec = being_solved.spec[being_solved.king_square[side_victim]];
  square const save_king_square = being_solved.king_square[side_victim];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  SETCASTLINGFLAGMASK(side_victim,k_cancastle);
  CLRFLAG(being_solved.spec[being_solved.king_square[side_victim]],Royal);
  being_solved.king_square[side_victim] = initsquare;
  generate_castling();
  being_solved.king_square[side_victim] = save_king_square;
  being_solved.spec[being_solved.king_square[side_victim]] = save_spec;
  being_solved.castling_rights = save_castling_rights;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_make(slice_index si,
                          piece_walk_type walk_victim, square sq_make_departure)
{
  numecoup const base_make = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceWalk(walk_victim);
  TraceSquare(sq_make_departure);
  TraceFunctionParamListEnd();

  generate_moves_different_walk(SLICE_NEXT1(si),walk_victim);
  move_generator_filter_captures(base_make,&is_false);

  if (is_king(walk_victim)
      && is_king(game_array.board[sq_make_departure])
      && TSTFLAG(game_array.spec[sq_make_departure],trait[nbply]))
    generate_castling_as_make();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_take_candidates(slice_index si,
                                     square sq_make_departure,
                                     ply generating_for)
{
  piece_walk_type const walk = being_solved.board[sq_make_departure];
  Flags const flags = being_solved.spec[sq_make_departure];
  numecoup const base_make = CURRMOVE_OF_PLY(nbply-2);
  numecoup const top_make = CURRMOVE_OF_PLY(nbply-1);
  numecoup curr_make;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_make_departure);
  TraceFunctionParam("%u",generating_for);
  TraceFunctionParamListEnd();

  empty_square(sq_make_departure);

  for (curr_make = top_make; curr_make>base_make; --curr_make)
  {
    numecoup const save_id = current_move_id[nbply];
    curr_generation->departure = move_generation_stack[curr_make].arrival;
    occupy_square(curr_generation->departure,walk,flags);
    pipe_move_generation_delegate(si);
    empty_square(curr_generation->departure);
    remember_move_ids_of_castlings_as_makes(move_generation_stack[curr_make].capture,
                                            save_id,current_move_id[nbply],
                                            generating_for);
  }

  curr_generation->departure = sq_make_departure;

  occupy_square(sq_make_departure,walk,flags);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void restrict_to_walk_victim(piece_walk_type walk_victim)
{
  numecoup i;
  numecoup const base_take = CURRMOVE_OF_PLY(nbply-1);
  numecoup top_take = base_take;
  Side const side_victim = advers(trait[nbply]);

  TraceFunctionEntry(__func__);
  TraceWalk(walk_victim);
  TraceFunctionParamListEnd();

  for (i = base_take+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
  {
    square sq_capture = move_generation_stack[i].capture;
    if (en_passant_is_ep_capture(sq_capture))
      sq_capture -= offset_en_passant_capture;
    if (get_walk_of_piece_on_square(sq_capture)==walk_victim
        && TSTFLAG(being_solved.spec[sq_capture],side_victim))
      move_generation_stack[++top_take] = move_generation_stack[i];
  }

  SET_CURRMOVE(nbply,top_take);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static numecoup moves_with_castling_as_make_first(numecoup base,
                                                  ply generating_for)
{
  ply const parent = parent_ply[nbply];
  numecoup curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",generating_for);
  TraceFunctionParamListEnd();

  do
    ++base;
  while (base<=CURRMOVE_OF_PLY(nbply)
         && make_and_take_has_move_castling_as_make(generating_for,move_generation_stack[base].id)!=initsquare);

  for (curr = base+1; curr<=CURRMOVE_OF_PLY(nbply); ++curr)
    if (make_and_take_has_move_castling_as_make(generating_for,move_generation_stack[curr].id)!=initsquare)
    {
      move_generation_elmt const tmp = move_generation_stack[curr];
      move_generation_stack[curr] = move_generation_stack[base];
      move_generation_stack[base] = tmp;
      ++base;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",base);
  TraceFunctionResultEnd();
  return base-1;
}

static void add_take(slice_index si,
                     square sq_make_departure,
                     piece_walk_type walk_victim,
                     ply generating_for)
{
  numecoup const base_make = CURRMOVE_OF_PLY(nbply-1);
  numecoup const top_make = CURRMOVE_OF_PLY(nbply);
  numecoup const base_take = top_make;
  ply const elder = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_make_departure);
  TraceWalk(walk_victim);
  TraceFunctionParam("%u",generating_for);
  TraceFunctionParamListEnd();

  siblingply(advers(trait[nbply]));
  current_move_id[nbply] = current_move_id[elder];

  generate_take_candidates(si,sq_make_departure,generating_for);
  restrict_to_walk_victim(walk_victim);
  remove_duplicate_moves_of_single_piece(moves_with_castling_as_make_first(base_take,generating_for));

  memmove(move_generation_stack+base_make+1,
          move_generation_stack+top_make+1,
          (CURRMOVE_OF_PLY(nbply)-base_take) * sizeof move_generation_stack[0]);
  CURRMOVE_OF_PLY(nbply-1) = CURRMOVE_OF_PLY(nbply) - (top_make-base_make);

  current_move_id[elder] = current_move_id[nbply];
  finply();

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
void make_and_take_generate_captures_by_walk_solve(slice_index si)
{
  Side const side_victim = advers(trait[nbply]);
  square const sq_make_departure = curr_generation->departure;
  piece_walk_type walk_victim;
  ply const generating_for = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  siblingply(side_victim);
  current_move_id[nbply] = current_move_id[generating_for];

  for (walk_victim=King; walk_victim<=max_victim; ++walk_victim)
    if (being_solved.number_of_pieces[side_victim][walk_victim]>0)
    {
      TraceWalk(walk_victim);TraceEOL();
      generate_make(si,walk_victim,sq_make_departure);
      add_take(si,sq_make_departure,walk_victim,generating_for);
      while (CURRMOVE_OF_PLY(nbply-1)<CURRMOVE_OF_PLY(nbply))
        move_generation_stack[++CURRMOVE_OF_PLY(nbply-1)].departure = sq_make_departure;
    }

  current_move_id[generating_for] = current_move_id[nbply];
  finply();

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
void make_and_take_move_castling_partner(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (make_and_take_has_move_castling_as_make(nbply,move_generation_stack[CURRMOVE_OF_PLY(nbply)].id))
  {
    case kingside_castling:
    {
      square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
      square const sq_partner_departure = sq_departure+3*dir_right;
      square const sq_partner_arrival = sq_departure+dir_right;
      move_effect_journal_do_piece_movement(move_effect_reason_castling_partner_movement,
                                            sq_partner_departure,sq_partner_arrival);
      break;
    }

    case queenside_castling:
    {
      square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
      square const sq_partner_departure = sq_departure+4*dir_left;
      square const sq_partner_arrival = sq_departure+dir_left;
      move_effect_journal_do_piece_movement(move_effect_reason_castling_partner_movement,
                                            sq_partner_departure,sq_partner_arrival);
      break;
    }

    default:
      break;
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void make_and_take_reset(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);TraceEOL();

  move_ids_with_castling_as_make[nbply].king_side.min_move_id = 0;
  move_ids_with_castling_as_make[nbply].king_side.max_move_id = 0;

  move_ids_with_castling_as_make[nbply].queen_side.min_move_id = 0;
  move_ids_with_castling_as_make[nbply].queen_side.max_move_id = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a particular move starts with castling as a make part
 * @param ply the ply that the move is played in
 * @param move_id the id of the move
 * @return kingside_castling, queenside_castling or initsquare
 */
square make_and_take_has_move_castling_as_make(ply ply, numecoup move_id)
{
  square result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParam("%u",move_id);
  TraceFunctionParamListEnd();

  if (move_ids_with_castling_as_make[ply].king_side.min_move_id<=move_id
      && move_id<move_ids_with_castling_as_make[ply].king_side.max_move_id)
    result = kingside_castling;
  else if (move_ids_with_castling_as_make[ply].queen_side.min_move_id<=move_id
           && move_id<move_ids_with_castling_as_make[ply].queen_side.max_move_id)
    result = queenside_castling;
  else
    result = initsquare;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_no_capture(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STMoveGeneratorRejectCaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_capture(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMakeTakeGenerateCapturesWalkByWalk)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void solving_insert_make_and_take(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  move_generator_instrument_for_alternative_paths(si,nr_sides);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorStandardPath,
                                             &instrument_no_capture);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_capture);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_moves(si,STMakeTakeMoveCastlingPartner);

  observation_play_move_to_validate(si,nr_sides);

  solving_test_check_playing_moves(si);
  solving_instrument_check_testing(si,STNoKingCheckTester);
  solving_instrument_check_testing(si,STMakeTakeLimitMoveGenerationMakeWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
