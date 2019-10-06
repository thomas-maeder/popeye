#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_H

#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "solving/ply.h"
#include "pieces/attributes/total_invisible/goal_validation.h"

extern unsigned int total_invisible_number;

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
void total_invisible_move_sequence_tester_solve(slice_index si);

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
void total_invisible_instrumenter_solve(slice_index si);

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
void total_invisible_invisibles_allocator_solve(slice_index si);

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
void total_invisible_move_repeater_solve(slice_index si);

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
void total_invisible_reserve_king_movement(slice_index si);

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si);

void total_invisible_write_flesh_out_history(void);

typedef enum
{
  play_regular,
  play_rewinding,
  play_detecting_revelations,
  play_validating_mate,
  play_testing_mate,
  play_attacking_mating_piece,
  play_initialising_replay,
  play_replay_validating,
  play_replay_testing,
  play_finalising_replay,
  play_unwinding,
  play_testing_goal
} play_phase_type;

typedef enum
{
  purpose_none,
  purpose_victim,
  purpose_interceptor,
  purpose_random_mover,
  purpose_capturer,
  purpose_castling_partner,
  purpose_attacker
} purpose_type;

typedef struct action_type
{
    purpose_type purpose;
    ply acts_when;
    square on;
} action_type;


extern ply top_ply_of_regular_play;
extern play_phase_type play_phase;
extern PieceIdType top_invisible_piece_id;
extern PieceIdType top_visible_piece_id;

extern ply flesh_out_move_highwater;

extern stip_length_type combined_result;

void report_deadend(char const *s, unsigned int lineno);
void restart_from_scratch(void);
void recurse_into_child_ply(void);
void start_iteration(void);

void rewind_effects(void);
void unrewind_effects(void);

void adapt_pre_capture_effect(void);
void validate_king_placements(void);

void replay_fleshed_out_move_sequence(play_phase_type phase_replay);

//#define REPORT_DEADEND report_deadend("DEADEND",__LINE__)
//#define REPORT_EXIT report_deadend("EXIT",__LINE__)

#define REPORT_DEADEND
#define REPORT_EXIT

#endif
