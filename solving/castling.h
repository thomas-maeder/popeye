#if !defined(SOLVING_CASTLING_H)
#define SOLVING_CASTLING_H

/* This module implements castling.
 */

#include "stipulation/slice_type.h"
#include "solving/solve.h"

enum
{
  black_castling_offset = 4
};

/* symbols for bits and their combinations in castling_flag */
typedef enum
{
  rh_cancastle = 0x01,
  ra_cancastle = 0x02,
  k_cancastle = 0x04,

  k_castling = k_cancastle|rh_cancastle,
  q_castling = k_cancastle|ra_cancastle,
  castlings = k_cancastle|ra_cancastle|rh_cancastle,

  whk_castling = k_cancastle|rh_cancastle,
  whq_castling = k_cancastle|ra_cancastle,
  wh_castlings = k_cancastle|ra_cancastle|rh_cancastle,

  blk_castling = whk_castling<<black_castling_offset,
  blq_castling = whq_castling<<black_castling_offset,
  bl_castlings = wh_castlings<<black_castling_offset
} castling_flag_type;
/* NOTE: k_cancastle must be larger than the respective
 * r[ah]_cancastle or evaluations of the form
 * TSTCASTLINGFLAGMASK(nbply,White,castlings)<=k_cancastle
 * stop working. */

extern  castling_flag_type castling_flag[maxply+2];
enum { castlings_flags_no_castling = maxply+1 };
extern  boolean castling_supported;
extern castling_flag_type castling_mutual_exclusive[nr_sides][2];

#define TSTCASTLINGFLAGMASK(ply_id,side,mask) TSTFLAGMASK(castling_flag[(ply_id)]>>(side)*black_castling_offset,(mask))
#define SETCASTLINGFLAGMASK(ply_id,side,mask) SETFLAGMASK(castling_flag[(ply_id)],(mask)<<((side)*black_castling_offset))
#define CLRCASTLINGFLAGMASK(ply_id,side,mask) CLRFLAGMASK(castling_flag[(ply_id)],(mask)<<((side)*black_castling_offset))

/* Restore the castling rights of a piece
 * @param sq_arrival position of piece for which to restore castling rights
 */
void restore_castling_rights(square sq_arrival);

/* Enable castling rights for the piece that just arrived (for whatever reason)
 * on a square
 * @param on the arrival square
 */
void enable_castling_rights(square sq_arrival);

/* Disable castling rights for the piece that just left (for whatever reason)
 * a square
 * @param on the square left
 */
void disable_castling_rights(square sq_departure);

/* Allocate a STCastlingIntermediateMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_intermediate_move_generator_slice(void);

/* Initialise the next move generation
 * @param sq_departure departure square of move to be generated
 * @param sq_arrival arrival square of move to be generated
 */
void castling_intermediate_move_generator_init_next(square sq_departure,
                                                    square sq_arrival);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
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
 *                                     uninted immobility on the next move
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
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type castling_rights_adjuster_solve(slice_index si,
                                                 stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_castling(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
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
