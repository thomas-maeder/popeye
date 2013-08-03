#if !defined(SOLVING_CASTLING_H)
#define SOLVING_CASTLING_H

/* This module implements castling.
 */

#include "stipulation/slice_type.h"
#include "solving/solve.h"
#include "solving/castling_rights.h"
#include "solving/move_effect_journal.h"

extern  castling_flag_type castling_flag;
extern castling_flag_type castling_mutual_exclusive[nr_sides][2];
extern castling_flag_type castling_flags_no_castling;

#define TSTCASTLINGFLAGMASK(side,mask) TSTFLAGMASK(castling_flag>>(side)*black_castling_offset,(mask))
#define SETCASTLINGFLAGMASK(side,mask) SETFLAGMASK(castling_flag,(mask)<<((side)*black_castling_offset))
#define CLRCASTLINGFLAGMASK(side,mask) CLRFLAGMASK(castling_flag,(mask)<<((side)*black_castling_offset))

/* Undo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_disabling_castling_right(move_effect_journal_index_type curr);

/* Redo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_disabling_castling_right(move_effect_journal_index_type curr);

/* Undo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_enabling_castling_right(move_effect_journal_index_type curr);

/* Redo removing a castling right
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_enabling_castling_right(move_effect_journal_index_type curr);

/* Enable castling rights for the piece that just arrived (for whatever reason)
 * on a square
 * @param reason why
 * @param on the arrival square
 */
void enable_castling_rights(move_effect_reason_type reason,
                            square sq_arrival);

/* Disable castling rights for the piece that just left (for whatever reason)
 * a square
 * @param reason why
 * @param on the square left
 */
void disable_castling_rights(move_effect_reason_type reason,
                             square sq_departure);

/* Allocate a STCastlingIntermediateMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_intermediate_move_generator_slice(void);

/* Initialise the next move generation
 * @param sq_arrival arrival square of move to be generated
 */
void castling_intermediate_move_generator_init_next(square sq_arrival);

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
                                                             stip_length_type n);


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
stip_length_type castling_player_solve(slice_index si, stip_length_type n);

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
                                                stip_length_type n);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void castling_generator_generate_castling(slice_index si, PieNam p);

/* Instrument the solving machinery with castling
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_castling(slice_index si);

void solving_disable_castling(slice_index si);

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
                                                        stip_length_type n);

/* Instrument a stipulation with alternative move player slices
 * @param si identifies root slice of stipulation
 */
void insert_alternative_move_players(slice_index si, slice_type type);

/* Instrument slices with move tracers
 */
void stip_insert_mutual_castling_rights_adjusters(slice_index si);

#endif
