#include "pieces/attributes/chameleon.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/pawns/promotion.h"
#include "position/position.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

enum
{
  stack_size = max_nr_promotions_per_ply*maxply+1
};

static unsigned int stack_pointer;

static struct
{
    square where;
    post_move_iteration_id_type lock;
} change_into_chameleon[stack_size];

static move_effect_journal_index_type horizon;

piece_walk_type chameleon_walk_sequence[nr_piece_walks];

twin_id_type chameleon_is_squence_explicit;

static void reset_sequence(chameleon_sequence_type* sequence)
{
  piece_walk_type p;
  for (p = Empty; p!=nr_piece_walks; ++p)
    (*sequence)[p] = p;
}

/* Initialise one mapping captured->reborn from an explicit indication
 * @param captured captured walk
 * @param reborn type of reborn walk if a piece with walk captured is captured
 */
void chameleon_set_successor_walk_explicit(twin_id_type *is_explicit,
                                           chameleon_sequence_type* sequence,
                                           piece_walk_type from, piece_walk_type to)
{
  if (*is_explicit != twin_id)
  {
    reset_sequence(sequence);
    *is_explicit = twin_id;
  }

  (*sequence)[from] = to;
}

/* Initialise the reborn pieces if they haven't been already initialised
 * from the explicit indication
 * @note chameleon_init_sequence_implicit() resets *is_explicit to false
 */
void chameleon_init_sequence_implicit(twin_id_type *is_explicit,
                                      chameleon_sequence_type* sequence)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",*is_explicit==twin_id);
  TraceFunctionParamListEnd();

  if (*is_explicit!=twin_id)
  {
    reset_sequence(sequence);

    (*sequence)[standard_walks[Knight]] = standard_walks[Bishop];
    (*sequence)[standard_walks[Bishop]] = standard_walks[Rook];
    (*sequence)[standard_walks[Rook]] = standard_walks[Queen];
    (*sequence)[standard_walks[Queen]] = standard_walks[Knight];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square find_promotion(move_effect_journal_index_type base)
{
  move_effect_journal_index_type curr = move_effect_journal_base[nbply+1];
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  while (curr>base)
  {
    --curr;

    if (move_effect_journal[curr].type==move_effect_piece_change
        && move_effect_journal[curr].reason==move_effect_reason_pawn_promotion)
    {
      result = move_effect_journal[curr].u.piece_change.on;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static void solve_nested(slice_index si)
{
  move_effect_journal_index_type const save_horizon = horizon;

  horizon = move_effect_journal_base[nbply+1];
  ++stack_pointer;
  pipe_solve_delegate(si);

  --stack_pointer;
  horizon = save_horizon;
}

static boolean is_walk_in_chameleon_sequence(piece_walk_type walk)
{
  return chameleon_walk_sequence[walk]!=walk;
}

static square decide_about_change(void)
{
  square result = initsquare;

  if (post_move_iteration_locked[nbply])
    change_into_chameleon[stack_pointer].lock = post_move_iteration_id[nbply];
  else
  {
    square const sq_promotion = find_promotion(horizon);
    if (sq_promotion!=initsquare
        && is_walk_in_chameleon_sequence(being_solved.board[sq_promotion])
        && !TSTFLAG(being_solved.spec[sq_promotion],Chameleon))
    {
      result = sq_promotion;
      lock_post_move_iterations();
      change_into_chameleon[stack_pointer].lock = post_move_iteration_id[nbply];
    }
  }

  return result;
}

static void do_change(void)
{
  Flags changed = being_solved.spec[change_into_chameleon[stack_pointer].where];
  SETFLAG(changed,Chameleon);
  move_effect_journal_do_flags_change(move_effect_reason_pawn_promotion,
                                      change_into_chameleon[stack_pointer].where,
                                      changed);
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
void chameleon_change_promotee_into_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!post_move_am_i_iterating(change_into_chameleon[stack_pointer].lock))
    change_into_chameleon[stack_pointer].where = initsquare;

  if (change_into_chameleon[stack_pointer].where==initsquare)
  {
    solve_nested(si);
    change_into_chameleon[stack_pointer].where = decide_about_change();
  }
  else
  {
    do_change();
    solve_nested(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static piece_walk_type champiece(piece_walk_type walk_arriving)
{
  piece_walk_type const result = chameleon_walk_sequence[walk_arriving];

  TraceFunctionEntry(__func__);
  TraceWalk(walk_arriving);
  TraceFunctionParamListEnd();

  assert(chameleon_walk_sequence[walk_arriving]!=Empty);

  TraceFunctionExit(__func__);
  TraceWalk(result);
  TraceFunctionResultEnd();
  return result;
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
void chameleon_arriving_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    PieceIdType const moving_id = GetPieceId(movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    if (TSTFLAG(movingspec,Chameleon))
      move_effect_journal_do_walk_change(move_effect_reason_chameleon_movement,
                                          pos,
                                          champiece(get_walk_of_piece_on_square(pos)));
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_promoter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STChameleonChangePromoteeInto);
    promotion_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for solving problems with some
 * chameleon pieces
 * @param si identifies root slice of stipulation
 */
void chameleon_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonArrivingAdjuster);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STBeforePawnPromotion,&instrument_promoter);
    stip_traverse_structure(si,&st);
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
void chameleon_chess_arriving_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    move_effect_journal_do_walk_change(move_effect_reason_chameleon_movement,
                                        pos,
                                        champiece(get_walk_of_piece_on_square(pos)));
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery for solving problems with the condition
 * Chameleon Chess
 * @param si identifies root slice of stipulation
 */
void chameleon_chess_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STChameleonChessArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
