#if !defined(SOLVING_MACHINERY_TWIN_H)
#define SOLVING_MACHINERY_TWIN_H

#include "stipulation/stipulation.h"
#include "solving/move_effect_journal.h"

typedef unsigned long twin_id_type;

/* Sequence number of the current twin (and its duplex if any).
 * Mainly used for boolean twin-related flags: flag==twin_id means true,
 * and everything else means false. This allows us to not reset the flag at the
 * beginning (or end) of a twin.
 */
extern twin_id_type twin_id;

/* is the current twin a continued twin? */
extern boolean twin_is_continued;

typedef enum
{
  twin_original_position_no_twins,
  twin_zeroposition,
  twin_regular
} twin_stage_type;
extern twin_stage_type twin_stage;

enum
{
  twin_a,
  twin_b
  /* etc. */
};
extern unsigned int twin_number;

/* Validate whether shifting the entire position would move >=1 piece off board
 * @return true iff it doesn't
 */
boolean twin_twinning_shift_validate(square from, square to);

/* Initialise the piece walk caches.
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
void piece_walk_caches_initialiser_solve(slice_index si);

/* Create the builder setup ply.
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
void create_builder_setup_ply_solve(slice_index si);

/* Assign every piece of the position their id
 */
void initialise_piece_ids(void);

/* Find the entry slice into the stipulation
 * @param start entry slice into the input branch
 * @return entry slice into the stipulation
 */
slice_index input_find_stipulation(slice_index start);

void input_instrument_with_stipulation(slice_index start,
                                       slice_index stipulation_root_hook);

void input_uninstrument_with_stipulation(slice_index start);

#endif
