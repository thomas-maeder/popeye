#if !defined(PYCOMPOS_H)
#define PYCOMPOS_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with composite
 * (i.e. non-leaf) stipulation slices.
 */

/* Determine whether attacker can end in n moves of direct play.
 * This is a recursive function.
 * @param attacker attacking side (i.e. side attempting to reach the
 * end)
 * @param n number of moves left until the end state has to be reached
 * @return true iff attacker can end in n moves
 */
boolean d_composite_does_attacker_win(couleur attacker,
                                      int n,
                                      slice_index si);

typedef enum
{
  already_won,
  short_win,
  win,
  loss,
  short_loss,
  already_lost
} d_composite_win_type;

/* Determine whether the defender wins in a self/reflex stipulation in
 * n.
 * @param defender defending side (at move)
 * @param n number of moves until end state has to be reached
 * @return "how much or few" the defending side wins
 */
d_composite_win_type d_composite_does_defender_win(couleur defender,
                                                   int n,
                                                   slice_index si);

/* Determine whether the defender has lost in direct play with his move
 * just played.
 * Assumes that there is no short win for the defending side.
 * @param attacker attacking side
 * @param si slice identifier
 * @return whether there is a short loss
 */
boolean d_composite_has_defender_lost(couleur attacker, slice_index si);

/* Determine whether the defender has won the direct play sequence
 * with his move just played.
 * @param attacker attacking side
 * @param si slice identifier
 * @return whether there is a short win
 */
boolean d_composite_has_defender_won(couleur attacker, slice_index si);

/* Determine whether the attacker has won the composite slice with his
 * last move in direct play. 
 * @param defender defending side
 * @param si slice identifier
 * @return true iff attacker has won
 */
boolean d_composite_has_attacker_won(couleur defender, slice_index si);

/* Determine whether the attacker has lost with his last move in
 * direct play. 
 * Assumes that he has not won with his last move.
 * @param defender defending side
 * @param si slice identifier
 * @return true iff attacker has lost
 */
boolean d_composite_has_attacker_lost(couleur defender, slice_index si);

/* Determine and write set play of a direct/self/reflex stipulation
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached,
 *          including the virtual key move
 * @param si slice index
 */
void d_composite_solve_setplay(couleur attacker, int n, slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations) and write the refutations (if any), starting at the end
 * of a quodlibet slice.
 * @param attacker attacking side (has just played key)
 * @param n number of moves until end state has to be reached
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @param is_try true iff what we are solving is a try
 */
void d_composite_write_key_solve_postkey(couleur attacker,
                                         int n,
                                         int refutations,
                                         slice_index si,
                                         boolean is_try);

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * This is an indirectly recursive function.
 * @param attacker attacking side (i.e. side that has just played)
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param len_threat length of threats
 * @param threats table containing threats
 * @param refutations table containing refutations after move just
 *                    played
 */
void d_composite_solve_variations(couleur attacker,
                                  int n,
                                  int len_threat,
                                  int threats,
                                  int refutations,
                                  slice_index si);

/* Determine and write the solutions and tries in the current position
 * in direct play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 */
void d_composite_solve(couleur attacker,
                       int n,
                       boolean restartenabled,
                       slice_index si);

/* Determine and write the post-key solution in the current position
 * in direct/self/reflex play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param si slice index
 */
void d_composite_solve_postkey(couleur attacker, int n, slice_index si);

/* Determine and write the continuations in the current position in
 * direct/self/reflex play (i.e. attacker's moves winning after a
 * defender's move that refuted the threat).
 * This is an indirectly recursive function.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param continuations table where to store continuing moves
 *                      (i.e. threats)
 * @param si slice index
 */
void d_composite_solve_continuations(couleur attacker,
                                     int n,
                                     int continuations,
                                     slice_index si);

/* Determine and write the solutions in the current position in series
 * play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean ser_composite_solve(couleur attacker,
                            int n,
                            boolean restartenabled,
                            slice_index si);

/* Determine and write the solutions in the current position in help
 * play.
 * @param side_at_move side at move
 * @param n number of moves until end state has to be reached
 * @param restartenabled true iff the written solution should only
 *                       start at the Nth legal move of attacker
 *                       (determined by user input)
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean h_composite_solve(couleur side_at_move,
                          int n,
                          boolean restartenabled,
                          slice_index si);

#endif
