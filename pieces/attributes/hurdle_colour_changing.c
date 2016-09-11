#include "pieces/attributes/hurdle_colour_changing.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hoppers.h"
#include "pieces/walks/pawns/promotion.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "stipulation/structure_traversal.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

typedef enum
{
  none,
  non_changing,
  changing,
  both
} colour_changing_hopper_existance_type;

static colour_changing_hopper_existance_type promote_walk_into[nr_piece_walks];

enum
{
  stack_size = max_nr_promotions_per_ply*maxply+1
};

static boolean next_prom_to_changing_happening[stack_size];

static unsigned int stack_pointer;

static move_effect_journal_index_type horizon;

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
void hurdle_colour_change_initialiser_solve(slice_index si)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    piece_walk_type p;
    for (p = Empty; p!=nr_piece_walks; ++p)
      promote_walk_into[p] = none;
  }

  {
    square const *s;
    for (s = boardnum; *s; ++s)
    {
      piece_walk_type const p = get_walk_of_piece_on_square(*s);
      promote_walk_into[p] |= TSTFLAG(being_solved.spec[*s],ColourChange) ? changing : non_changing;
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find a promotion in the effects of the move being played since we last looked
 * @param base start of set of effects where to look for a promotion
 * @return index of promotion effect; base if there is none
 */
static move_effect_journal_index_type find_promotion(move_effect_journal_index_type base)
{
  move_effect_journal_index_type curr = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type result = base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  while (curr>base)
  {
    --curr;

    if (move_effect_journal[curr].type==move_effect_piece_change
        && move_effect_journal[curr].reason==move_effect_reason_pawn_promotion)
    {
      result = curr;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Delegate solving to the next slice, while remembering the set of effects of
 * this move where prommotions have been considered.
 * @param si identifies the current slice
 */
static void solve_nested(slice_index si)
{
  move_effect_journal_index_type const save_horizon = horizon;

  horizon = move_effect_journal_base[nbply+1];
  ++stack_pointer;
  post_move_iteration_solve_delegate(si);
  --stack_pointer;
  horizon = save_horizon;
}

/* make the promotee of a promotion effect hurdle colour changing
 * @param idx_promotion index of the promotion effect
 */
static void do_change(move_effect_journal_index_type idx_promotion)
{
  square const sq_prom = move_effect_journal[idx_promotion].u.piece_change.on;
  Flags changed = being_solved.spec[sq_prom];
  SETFLAG(changed,ColourChange);
  move_effect_journal_do_flags_change(move_effect_reason_pawn_promotion,
                                      sq_prom,
                                      changed);
}

/* start or continue an iteration over leaving non-changing and changing to
 * changing
 * @param si identifies the current slice
 * @param idx_promotion index of the promotion effect
 */
static void promote_to_both_non_changing_and_changing(slice_index si,
                                                      move_effect_journal_index_type idx_promotion)
{
  if (!post_move_am_i_iterating())
    next_prom_to_changing_happening[stack_pointer] = false;

  if (next_prom_to_changing_happening[stack_pointer])
  {
    do_change(idx_promotion);
    solve_nested(si);
  }
  else
  {
    solve_nested(si);

    if (!post_move_iteration_is_locked())
    {
      next_prom_to_changing_happening[stack_pointer] = true;
      post_move_iteration_lock();
    }
  }
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
void hurdle_colour_change_change_promotee_into_solve(slice_index si)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const idx_promotion = find_promotion(horizon);
    if (idx_promotion==horizon)
      /* no promotion */
      solve_nested(si);
    else
    {
      piece_walk_type const walk_promotee = move_effect_journal[idx_promotion].u.piece_change.to;
      switch (promote_walk_into[walk_promotee])
      {
        case none:
        case non_changing:
          solve_nested(si);
          break;

        case changing:
          do_change(idx_promotion);
          solve_nested(si);
          break;

        case both:
          promote_to_both_non_changing_and_changing(si,idx_promotion);
          break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_hurdle_colour(void)
{
  square const sq_hurdle = hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle;
  piece_walk_type const pi_hurdle = get_walk_of_piece_on_square(sq_hurdle);

  if (pi_hurdle>King && !is_piece_neutral(being_solved.spec[sq_hurdle]))
    move_effect_journal_do_side_change(move_effect_reason_hurdle_colour_changing,
                                       sq_hurdle);
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
void hurdle_colour_changer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    if (TSTFLAG(movingspec,ColourChange))
      update_hurdle_colour();
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
    slice_index const prototype = alloc_pipe(STHurdleColourChangerChangePromoteeInto);
    promotion_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_hurdle_colour_changers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STHurdleColourChanger);

  {
    slice_index const prototype = alloc_pipe(STHurdleColourChangeInitialiser);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STBeforePawnPromotion,&instrument_promoter);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
