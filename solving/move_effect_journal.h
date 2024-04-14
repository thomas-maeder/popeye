#if !defined(SOLVING_MOVE_EFFECT_JOURNAL_H)
#define SOLVING_MOVE_EFFECT_JOURNAL_H

/* This module implements a journal of the effects of played moves. The journal
 * is used
 * * to replay the moves for printing a solution
 * * to find derived effects (e.g. imitator piece_movement, placement of Haan Chess
 *   holes etc.)
 */

#include "solving/machinery/solve.h"
#include "position/castling_rights.h"
#include "solving/ply.h"
#include "position/pieceid.h"
#include "position/board.h"
#include "pieces/pieces.h"

#include <limits.h>

/* types of effects */
typedef enum
{
  move_effect_none,
  move_effect_walk_change,
  move_effect_piece_movement,
  move_effect_piece_creation,
  move_effect_no_piece_removal,
  move_effect_piece_readdition,
  move_effect_piece_removal,
  move_effect_piece_exchange,
  move_effect_side_change,
  move_effect_king_square_movement,
  move_effect_flags_change,
  move_effect_board_transformation,
  move_effect_centre_revolution,
  move_effect_imitator_addition,
  move_effect_imitator_movement,
  move_effect_remember_ghost,
  move_effect_forget_ghost,
  move_effect_half_neutral_deneutralisation,
  move_effect_half_neutral_neutralisation,
  move_effect_square_block,
  move_effect_bgl_adjustment,
  move_effect_strict_sat_adjustment,
  move_effect_disable_castling_right,
  move_effect_enable_castling_right,
  move_effect_remember_ep_capture_potential,
  move_effect_remember_duellist,
  move_effect_remember_parachuted,
  move_effect_remember_volcanic,
  move_effect_swap_volcanic,
  move_effect_revelation_of_castling_partner,
  move_effect_revelation_of_placed_invisible,
  move_effect_revelation_of_new_invisible,

  move_effect_input_condition,
  move_effect_input_stipulation,
  move_effect_remove_stipulation,
  move_effect_input_sstipulation,
  move_effect_total_side_exchange,
  move_effect_twinning_substitute,
  move_effect_twinning_shift,

  move_effect_snapshot_proofgame_target_position,
  move_effect_atob_reset_position_for_target,

  move_effect_hunter_type_definition,

  nr_move_effect_types
} move_effect_type;

/* reasons for effects */
typedef enum
{
  move_effect_no_reason,
  move_effect_reason_moving_piece_movement,
  move_effect_reason_regular_capture,
  move_effect_reason_ep_capture,
  move_effect_reason_castling_king_movement,
  move_effect_reason_castling_partner,
  move_effect_reason_pawn_promotion,
  move_effect_reason_messigny_exchange,
  move_effect_reason_exchange_castling_exchange,
  move_effect_reason_movement_imitation,
  move_effect_reason_king_transmutation,
  move_effect_reason_adaption_to_neutral_side,
  move_effect_reason_half_neutral_king_movement,
  move_effect_reason_republican_king_insertion,
  move_effect_reason_kamikaze_capturer,
  move_effect_reason_half_neutral_neutralisation,
  move_effect_reason_half_neutral_deneutralisation,
  move_effect_reason_football_chess_substitution,
  move_effect_reason_chameleon_movement,
  move_effect_reason_norsk_chess,
  move_effect_reason_protean_adjustment,
  move_effect_reason_frischauf_mark_promotee,
  move_effect_reason_degradierung,
  move_effect_reason_andernach_chess, /* includes Anti-Andernach, Tibet, Chameleon pursuit */
  move_effect_reason_traitor_defection,
  move_effect_reason_circe_turncoats,
  move_effect_reason_volage_side_change,
  move_effect_reason_magic_piece,
  move_effect_reason_magic_square,
  move_effect_reason_masand,
  move_effect_reason_influencer,
  move_effect_reason_einstein_chess,
  move_effect_reason_kobul_king,
  move_effect_reason_summon_ghost,
  move_effect_reason_rebirth_no_choice,
  move_effect_reason_rebirth_choice,
  move_effect_reason_assassin_circe_rebirth,
  move_effect_reason_volcanic_remember,
  move_effect_reason_volcanic_uncover,
  move_effect_reason_transfer_no_choice,
  move_effect_reason_transfer_choice,
  move_effect_reason_promotion_of_reborn,
  move_effect_reason_sentinelles,
  move_effect_reason_oscillating_kings,
  move_effect_reason_hurdle_colour_changing,
  move_effect_reason_royal_dynasty,
  move_effect_reason_actuate_revolving_board,
  move_effect_reason_actuate_revolving_centre,
  move_effect_reason_singlebox_promotion,
  move_effect_reason_phantom_movement,
  move_effect_reason_sat_adjustment,
  move_effect_reason_zeroed_in,
  move_effect_reason_snek,
  move_effect_reason_breton,
  move_effect_reason_removal_of_invisible,
  move_effect_reason_revelation_of_invisible,
  move_effect_reason_role_exchange,
  move_effect_reason_series_capture,
  move_effect_reason_darkside,
  move_effect_reason_bul,

  move_effect_reason_diagram_setup,
  move_effect_reason_twinning
} move_effect_reason_type;

typedef unsigned int move_effect_journal_index_type;

enum
{
  move_effect_journal_index_null = INT_MAX
};

typedef struct
{
    square on;
    piece_walk_type walk;
    Flags flags;
} piece_type;

/* type of structure holding one effect */
typedef struct
{
    move_effect_type type;
    move_effect_reason_type reason;
    union
    {
        struct
        {
            square on;
            piece_walk_type from;
            piece_walk_type to;
        } piece_walk_change;
        struct
        {
            piece_walk_type moving;
            Flags movingspec;
            square from;
            square to;
        } piece_movement;
        struct
        {
            piece_type added;
            Side for_side;
        } piece_addition;
        piece_type piece_removal;
        struct {
            square on;
            piece_walk_type walk_original;
            Flags flags_original;
            piece_walk_type walk_revealed;
            Flags flags_revealed;
        } revelation_of_placed_piece;
        struct
        {
            square from;
            Flags fromflags;
            square to;
            Flags toflags;
        } piece_exchange;
        struct
        {
            square on;
            Side to;
        } side_change;
        struct
        {
            Side side;
            square from;
            square to;
        } king_square_movement;
        struct
        {
            square on;
            Flags from;
            Flags to;
        } flags_change;
        struct
        {
            SquareTransformation transformation;
        } board_transformation;
        struct
        {
            square to;
        } imitator_addition;
        struct
        {
            int delta;
            unsigned int nr_moved;
        } imitator_movement;
        struct
        {
            square on;
            Side side;
        } half_neutral_phase_change;
        struct
        {
            square blocked_square;
        } square_block;
        struct
        {
            unsigned int pos;
            piece_type ghost;
        } handle_ghost;
        struct
        {
            Side side;
            long int diff;
        } bgl_adjustment;
        struct
        {
            Side side;
        } strict_sat_adjustment;
        struct
        {
            Side side;
            castling_rights_type right;
        } castling_rights_adjustment;
        struct
        {
            square capture_square;
            ply ply;
        } ep_capture_potential;
        struct
        {
            Side side;
            square from;
            square to;
        } duellist;
        struct
        {
            slice_index start_index;
            fpos_t start;
        } input_complex;
        struct
        {
            slice_index start_index;
            slice_index stipulation;
        } input_stipulation;
        struct
        {
            slice_index start;
            slice_index first_removed;
            slice_index last_removed;
        } remove_stipulation;
        struct
        {
            square from;
            square to;
        } twinning_shift;

        struct
        {
            PieceIdType currPieceId;
        } reset_position;

        struct
        {
            piece_walk_type type;
        } hunter_type_definition;
    } u;
#if defined(DOTRACE)
      unsigned long id;
#endif
} move_effect_journal_entry_type;

enum
{
  /* length of journal */
  move_effect_journal_size = 10*maxply
};

/* the journal */
extern move_effect_journal_entry_type move_effect_journal[move_effect_journal_size];

/* index of next effect per ply (i.e. 1 position beyond the last effect) */
extern move_effect_journal_index_type move_effect_journal_base[maxply+1];

extern move_effect_journal_index_type move_effect_journal_index_offset_capture;
extern move_effect_journal_index_type move_effect_journal_index_offset_movement;
extern move_effect_journal_index_type move_effect_journal_index_offset_other_effects;

#if defined(DOTRACE)
extern unsigned long move_effect_journal_next_id;
#endif

/* Reserve space for an effect in each move before the capture (e.g. for
 * Singlebox Type 3 promotions). Conditions that do this have to make sure
 * that every move has such an effect, possibly by adding a null effect to
 * fill the reserved gap.
 */
void move_effect_journal_register_pre_capture_effect(void);

/* Reset the move effects journal from pre-capture effect reservations
 */
void move_effect_journal_reset(slice_index si);

/* Allocate an entry
 * @param type type of the effect
 * @param reason reason of the effect
 * @return address of allocated entry
 * @note terminates the program if the entries are exhausted
 */
move_effect_journal_entry_type *move_effect_journal_allocate_entry(move_effect_type type,
                                                                   move_effect_reason_type reason);

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
void move_effect_journal_undoer_solve(slice_index si);

/* Undo the effects of the current move in ply nbply
 */
void undo_move_effects(void);

/* Redo the effects of the current move in ply nbply
 */
void redo_move_effects(void);

void move_effect_journal_pop_effect(void);

typedef void (*move_effect_doer)(move_effect_journal_entry_type const *);

void move_effect_journal_init_move_effect_doers(void);

void move_effect_journal_set_effect_doers(move_effect_type type,
                                          move_effect_doer undoer,
                                          move_effect_doer redoer);


extern move_effect_journal_index_type king_square_horizon;

#endif
