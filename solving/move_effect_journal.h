#if !defined(SOLVING_MOVE_EFFECT_JOURNAL_H)
#define SOLVING_MOVE_EFFECT_JOURNAL_H

/* This module implements a journal of the effects of played moves. The journal
 * is used
 * * to replay the moves for printing a solution
 * * to find derived effects (e.g. imitator piece_movement, placement of Haan Chess
 *   holes etc.)
 */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"
#include "py.h"

#include <limits.h>

/* types of effects */
typedef enum
{
  move_effect_piece_change,
  move_effect_piece_movement,
  move_effect_piece_addition,
  move_effect_piece_removal,
  move_effect_piece_exchange,
  move_effect_side_change,
  move_effect_king_square_movement,
  move_effect_flags_change,
  move_effect_board_transformation,
  move_effect_centre_revolution,
  move_effect_imitator_addition,
  move_effect_imitator_movement,

  nr_move_effect_types
} move_effect_type;

/* reasons for effects */
typedef enum
{
  move_effect_reason_moving_piece_movement,
  move_effect_reason_regular_capture,
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
  move_effect_reason_volage_side_change,
  move_effect_reason_magic_piece,
  move_effect_reason_magic_square,
  move_effect_reason_masand,
  move_effect_reason_einstein_chess,
  move_effect_reason_kobul_king,
  move_effect_reason_summon_ghost,
  move_effect_reason_circe_rebirth,
  move_effect_reason_supercirce_rebirth,
  move_effect_reason_assassin_circe_rebirth,
  move_effect_reason_anticirce_rebirth,
  move_effect_reason_antisupercirce_rebirth,
  move_effect_reason_promotion_of_reborn,
  move_effect_reason_sentinelles,
  move_effect_reason_oscillating_kings,
  move_effect_reason_hurdle_colour_changing,
  move_effect_reason_royal_dynasty,
  move_effect_reason_actuate_revolving_board,
  move_effect_reason_actuate_revolving_centre,
  move_effect_reason_singlebox_promotion
} move_effect_reason_type;

typedef unsigned int move_effect_journal_index_type;

enum
{
  move_effect_journal_index_null = INT_MAX
};

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
            piece from;
            piece to;
        } piece_change;
        struct
        {
            piece moving;
            Flags movingspec;
            square from;
            square to;
        } piece_movement;
        struct
        {
            square on;
            piece added;
            Flags addedspec;
        } piece_addition;
        struct
        {
            square from;
            piece removed;
            Flags removedspec;
            move_effect_journal_index_type capturing_movement; /* ==move_effect_journal_index_null if not applicable */
        } piece_removal;
        struct
        {
            square from;
            square to;
        } piece_exchange;
        struct
        {
            Side side;
            square from;
        } king_square_removal;
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
        } imitator_movement;
        struct
        {
            square on;
        } half_neutral_neutralisation;
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
extern move_effect_journal_index_type move_effect_journal_top[maxply+1];

#if defined(DOTRACE)
extern unsigned long move_effect_journal_next_id;
#endif

/* Add moving a piece to the current move of the current ply
 * @param reason reason for moving the piece
 * @param from current position of the piece
 * @param to where to move the piece
 * @return index of piece piece_movement effect
 */
move_effect_journal_index_type
move_effect_journal_do_piece_movement(move_effect_reason_type reason,
                                      square from,
                                      square to);

/* Add adding a piece to the current move of the current ply
 * @param reason reason for adding the piece
 * @param on where to insert the piece
 * @param added nature of added piece
 * @param addedspec specs of added piece
 */
void move_effect_journal_do_piece_addition(move_effect_reason_type reason,
                                           square on,
                                           piece added,
                                           Flags addedspec);

/* Link the piece_removal and piece_movement just inserted as a capture
 * @param piece_removal index of the piece_removal effect
 * @param piece_removal index of the piece_movement effect
 */
void move_effect_journal_link_capture_to_movement(move_effect_journal_index_type removal,
                                                  move_effect_journal_index_type movement);

/* Add removing a piece to the current move of the current ply
 * @param reason reason for removing the piece
 * @param from current position of the piece
 * @return index of piece piece_removal effect
 */
move_effect_journal_index_type
move_effect_journal_do_piece_removal(move_effect_reason_type reason,
                                     square from);

/* Add changing the nature of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 * @param to new nature of piece
 */
void move_effect_journal_do_piece_change(move_effect_reason_type reason,
                                         square on,
                                         piece to);

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
 * @param to to side
 */
void move_effect_journal_do_side_change(move_effect_reason_type reason,
                                        square on,
                                        Side to);

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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_effect_journal_undoer_attack(slice_index si,
                                                   stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type move_effect_journal_undoer_defend(slice_index si,
                                                   stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_effect_journal_redoer_attack(slice_index si,
                                                   stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type move_effect_journal_redoer_defend(slice_index si,
                                                   stip_length_type n);

#endif
