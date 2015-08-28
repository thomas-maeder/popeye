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
#include <stdio.h>

/* types of effects */
typedef enum
{
  move_effect_none,
  move_effect_piece_change,
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

  move_effect_input_condition,
  move_effect_input_stipulation,
  move_effect_input_sstipulation,
  move_effect_twinning_polish,
  move_effect_twinning_substitute,
  move_effect_twinning_shift,

  move_effect_snapshot_proofgame_target_position,

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
  move_effect_reason_castling_partner_movement,
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
  move_effect_reason_snek,

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
        } piece_change;
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
            square square;
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
            square square;
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
            square from;
            square to;
        } twinning_shift;
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

/* Add moving a piece to the current move of the current ply
 * @param reason reason for moving the piece
 * @param from current position of the piece
 * @param to where to move the piece
 */
void move_effect_journal_do_piece_movement(move_effect_reason_type reason,
                                           square from,
                                           square to);

/* Readd an already existing piece to the current move of the current ply
 * @param reason reason for adding the piece
 * @param on where to insert the piece
 * @param added nature of added piece
 * @param addedspec specs of added piece
 * @param for_side for which side is the (potientally neutral) piece re-added
 */
void move_effect_journal_do_piece_readdition(move_effect_reason_type reason,
                                             square on,
                                             piece_walk_type added,
                                             Flags addedspec,
                                             Side for_side);

/* Add an newly created piece to the current move of the current ply
 * @param reason reason for creating the piece
 * @param on where to insert the piece
 * @param created nature of created piece
 * @param createdspec specs of created piece
 * @param for which side is the (potentially neutral) piece created
 */
void move_effect_journal_do_piece_creation(move_effect_reason_type reason,
                                           square on,
                                           piece_walk_type created,
                                           Flags createdspec,
                                           Side for_side);

/* Fill the capture gap at the head of each move by no capture
 */
void move_effect_journal_do_no_piece_removal(void);

/* Add removing a piece to the current move of the current ply
 * @param reason reason for removing the piece
 * @param from current position of the piece
 * @note use move_effect_journal_do_capture_move(), not
 * move_effect_journal_do_piece_removal() for regular captures
 */
void move_effect_journal_do_piece_removal(move_effect_reason_type reason,
                                          square from);

/* Add changing the walk of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 * @param to new nature of piece
 */
void move_effect_journal_do_walk_change(move_effect_reason_type reason,
                                        square on,
                                        piece_walk_type to);

/* Add exchanging two pieces to the current move of the current ply
 * @param reason reason for exchanging the two pieces
 * @param from position of primary piece
 * @param to position of partner piece
 */
void move_effect_journal_do_piece_exchange(move_effect_reason_type reason,
                                           square from,
                                           square to);

/* Add changing the side of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 */
void move_effect_journal_do_side_change(move_effect_reason_type reason,
                                        square on);

/* Add king square piece_movement to the current move of the current ply
 * @param reason reason for moving the king square
 * @param side whose king square to move
 * @param to where to move the king square
 */
void move_effect_journal_do_king_square_movement(move_effect_reason_type reason,
                                                 Side side,
                                                 square to);

/* Add changing the flags of a piece to the current move of the current ply
 * @param reason reason for moving the king square
 * @param on position of pieces whose flags to piece_change
 * @param to changed flags
 */
void move_effect_journal_do_flags_change(move_effect_reason_type reason,
                                         square on,
                                         Flags to);

/* Add transforming the board to the current move of the current ply
 * @param reason reason for moving the king square
 * @param transformation how to transform the board
 */
void move_effect_journal_do_board_transformation(move_effect_reason_type reason,
                                                 SquareTransformation transformation);

/* Complete blocking of a square
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 */
void move_effect_journal_do_square_block(move_effect_reason_type reason,
                                         square square);

/* Add the effects of a capture move to the current move of the current ply
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture position of the captured piece
 * @param removal_reason reason for the capture (ep or regular?)
 */
void move_effect_journal_do_capture_move(square sq_departure,
                                         square sq_arrival,
                                         square sq_capture,
                                         move_effect_reason_type removal_reason);

/* Add a null effect to the current move of the current ply
 */
void move_effect_journal_do_null_effect(void);

/* Add the effects of a null move to the current move of the current ply
 */
void move_effect_journal_do_null_move(void);

/* Execute a Polish type twinning
 */
void move_effect_journal_do_twinning_polish(void);

/* Execute a twinning that substitutes a walk for another
 */
void move_effect_journal_do_twinning_substitute(piece_walk_type from,
                                                piece_walk_type to);

/* Remember the original condition for restoration after the condition has been
 * modified by a twinning
 * @param start input position at start of parsing the condition
 */
void move_effect_journal_do_remember_condition(fpos_t start);

/* Remember the original stipulation for restoration after the stipulation has
 * been modified by a twinning
 * @param start input position at start of parsing the stipulation
 */
void move_effect_journal_do_remember_stipulation(slice_index start_index,
                                                 fpos_t start);
void move_effect_journal_do_remember_sstipulation(slice_index start_index,
                                                  fpos_t start);

/* Execute a twinning that shifts the entire position
 */
void move_effect_journal_do_twinning_shift(square from, square to);

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param ply ply in which the move was played
 * @param followed_id id of the piece to be followed
 * @param pos position of the piece after the inital capture removal and piece movement have taken place
 * @return the position of the piece with the "other" effect applied
 *         initsquare if the piece is not on the board after the "other" effects
 */
square move_effect_journal_follow_piece_through_other_effects(ply ply,
                                                              PieceIdType followed_id,
                                                              square pos);

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


extern move_effect_journal_index_type king_square_horizon;

/* Update the king squares according to the effects since king_square_horizon
 * @note Updates king_square_horizon; solvers invoking this function should
 *       reset king_square_horizon to its previous value before returning
 */
void update_king_squares(void);

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
void king_square_updater_solve(slice_index si);

/* Determine the departure square of a moveplayed
 * Assumes that the move has a single moving piece (i.e. is not a castling).
 * @param ply identifies the ply where the move is being or was played
 * @return the departure square; initsquare if the last move didn't have a movement
 */
square move_effect_journal_get_departure_square(ply ply);

#endif
