#include "pieces/attributes/chameleon.h"
#include "pieces/walks/walks.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

enum
{
  max_nr_promotions_per_ply = 3,
  stack_size = max_nr_promotions_per_ply*maxply+1
};

static square change_into_chameleon[stack_size];
static post_move_iteration_id_type prev_post_move_iteration_id[stack_size];

static unsigned int stack_pointer;

static move_effect_journal_index_type horizon;

PieNam chameleon_walk_sequence[PieceCount];

boolean chameleon_is_squence_implicit;

/* Reset the mapping from captured to reborn walks
 */
void chameleon_reset_sequence(boolean *is_implicit,
                              chameleon_sequence_type* sequence)
{
  PieNam p;
  for (p = Empty; p!=PieceCount; ++p)
    (*sequence)[p] = p;

  *is_implicit = true;
}

/* Initialise one mapping captured->reborn from an explicit indication
 * @param captured captured walk
 * @param reborn type of reborn walk if a piece with walk captured is captured
 */
void chameleon_set_successor_walk_explicit(boolean *is_implicit,
                                                 chameleon_sequence_type* sequence,
                                                 PieNam from, PieNam to)
{
  (*sequence)[from] = to;
  *is_implicit = false;
}

/* Initialise the reborn pieces if they haven't been already initialised
 * from the explicit indication
 */
void chameleon_init_sequence_implicit(boolean is_implicit,
                                      chameleon_sequence_type* sequence)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",is_implicit);
  TraceFunctionParamListEnd();

  if (is_implicit)
  {
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

static stip_length_type solve_nested(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const save_horizon = horizon;

  horizon = move_effect_journal_base[nbply+1];
  ++stack_pointer;
  result = solve(slices[si].next1,n);
  --stack_pointer;
  horizon = save_horizon;

  return result;
}

static square decide_about_change(void)
{
  square result = initsquare;

  square const sq_promotion = find_promotion(horizon);
  if (sq_promotion!=initsquare
      && !TSTFLAG(spec[sq_promotion],Chameleon)
      && !post_move_iteration_locked[nbply])
  {
    result = sq_promotion;
    lock_post_move_iterations();
  }

  return result;
}

static void do_change(void)
{
  Flags changed = spec[change_into_chameleon[stack_pointer]];
  SETFLAG(changed,Chameleon);
  move_effect_journal_do_flags_change(move_effect_reason_pawn_promotion,
                                      change_into_chameleon[stack_pointer],
                                      changed);
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type chameleon_change_promotee_into_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[stack_pointer])
    change_into_chameleon[stack_pointer] = initsquare;

  if (change_into_chameleon[stack_pointer]==initsquare)
  {
    result = solve_nested(si,n);
    change_into_chameleon[stack_pointer] = decide_about_change();
  }
  else
  {
    do_change();
    result = solve_nested(si,n);
  }

  prev_post_move_iteration_id[stack_pointer] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static PieNam champiece(PieNam walk_arriving)
{
  assert(chameleon_walk_sequence[walk_arriving]!=Empty);
  return chameleon_walk_sequence[walk_arriving];
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type chameleon_arriving_adjuster_solve(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
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
      move_effect_journal_do_piece_change(move_effect_reason_chameleon_movement,
                                          pos,
                                          champiece(get_walk_of_piece_on_square(pos)));
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_promoter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STChameleonChangePromoteeInto);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type chameleon_chess_arriving_adjuster_solve(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    move_effect_journal_do_piece_change(move_effect_reason_chameleon_movement,
                                        pos,
                                        champiece(get_walk_of_piece_on_square(pos)));
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
