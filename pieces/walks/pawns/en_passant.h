#if !defined(PIECES_PAWNS_EN_PASSANT_H)
#define PIECES_PAWNS_EN_PASSANT_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"

/* This module provides implements en passant captures
 */

enum
{
  en_passant_max_nr_multistep_over = 2
};

extern square en_passant_multistep_over[en_passant_max_nr_multistep_over][maxply+1];

/* Remember a square avoided by a multistep move of a pawn
 * @param index index of square (between 0<=index<en_passant_max_nr_multistep_over)
 * @param s avoided square
 */
void en_passant_remember_multistep_over(unsigned int index, square s);

/* Forget the last square remembered by en_passant_remember_multistep_over()
 */
void en_passant_forget_multistep(void);

/* Remember a possible en passant capture
 * @param diff adjustment
 */
void move_effect_journal_do_remember_ep(unsigned int index, square s);

/* Undo remembering a possible en passant capture
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_remember_ep(move_effect_journal_index_type curr);

/* Redo remembering a possible en passant capture
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_remember_ep(move_effect_journal_index_type curr);

/* Was a pawn multistep move played in a certain ply?
 * @param ply the ply
 * @return true iff a multi step move was played in ply ply
 */
boolean en_passant_was_multistep_played(ply ply);

/* Is an en passant capture possible to a specific square?
 * @param side for which side
 * @param s the square
 * @return true iff an en passant capture to s is currently possible
 */
boolean en_passant_is_capture_possible_to(Side side, square s);

/* Find the en passant capturee of the current ply. Only meaningful if an en
 * passant capture is actually possible, which isn't tested here.
 * @return position of the capturee
 *         initsquare if the capturee vanished from the board
 */
square en_passant_find_capturee(void);

/* Type of pawn type-specific check by ep. tester functions
 * @param sq_departure departure square of ep capture
 * @param sq_crossed square crossed by multistep move
 * @param p type of pawn
 * @param evaluate address of evaluater function
 * @return true iff side trait[nbply] gives check by ep. capture to sq_arrival
 */
typedef boolean (*en_passant_check_tester_type)(square sq_departure,
                                                square sq_crossed,
                                                evalfunction_t *evaluate);

/* Determine whether side trait[nbply] gives check by p. capture
 * @param dir_capture direction of ep capture
 * @param tester pawn-specific tester function
 * @param evaluate address of evaluater function
 * @return true if side trait[nbply] gives check by ep. capture
 */
boolean en_passant_test_check(numvec dir_capture,
                              en_passant_check_tester_type tester,
                              evalfunction_t *evaluate);

/* Adjust en passant possibilities of the following move after a non-capturing
 * move
 * @param sq_multistep_departure departure square of pawn move
 * @return the square avoided by the multistep; initsquare if no multistep
 */
square en_passant_find_potential(square sq_multistep_departure);

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
stip_length_type en_passant_adjuster_solve(slice_index si, stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_en_passant_adjusters(slice_index si);

#endif
