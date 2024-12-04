#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_H

#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "solving/ply.h"
#include "pieces/attributes/total_invisible/goal_validation.h"
#include "debugging/trace.h"

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

#define TraceAction(action) \
    do { \
        TraceValue("%u",(action)->acts_when); \
        TraceSquare((action)->on); \
        TraceValue("%u",(action)->purpose); \
    } while (0)


extern ply top_ply_of_regular_play;
extern play_phase_type play_phase;

extern slice_index tester_slice;

void restart_from_scratch(void);
void adapt_pre_capture_effect(void);
void recurse_into_child_ply(void);

void rewind_effects(void);
void unrewind_effects(void);

void insert_invisible_capturer(void);

void report_deadend(char const *s, unsigned int lineno);

//#define REPORT_DEADEND report_deadend("DEADEND",__LINE__)
//#define REPORT_EXIT report_deadend("EXIT",__LINE__)

#define REPORT_DEADEND
#define REPORT_EXIT

void write_history_recursive(ply ply);

#endif
