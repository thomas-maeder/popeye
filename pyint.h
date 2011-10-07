/******************** MODIFICATIONS to pyint.h **************************
**
** Date       Who  What
**
** 1997/04/04 TLi  Original
**
**************************** End of List ******************************/

#if !defined(PYINT_H)
#define PYINT_H

#include "pyposit.h"
#include "stipulation/help_play/play.h"

#define piece_usageENUMERATORS \
    ENUMERATOR(piece_is_unused), \
    ENUMERATOR(piece_pins), \
    ENUMERATOR(piece_is_fixed_to_diagram_square), \
    ENUMERATOR(piece_intercepts), \
    ENUMERATOR(piece_blocks), \
    ENUMERATOR(piece_guards), \
    ENUMERATOR(piece_gives_check), \
    ENUMERATOR(piece_is_missing), \
    ENUMERATOR(piece_is_king)

#define ENUMERATORS piece_usageENUMERATORS
#define ENUMERATION_TYPENAME piece_usage
#define ENUMERATION_DECLARE
#include "pyenum.h"

typedef struct
{
    square diagram_square;
    Flags flags;
    piece type;
    piece_usage usage;
} PIECE;

extern PIECE white[nr_squares_on_board];
extern PIECE black[nr_squares_on_board];

enum { index_of_king = 0 };

extern unsigned int MaxPiece[nr_sides];
extern unsigned int CapturesLeft[maxply+1];

extern boolean solutions_found;

extern slice_index current_start_slice;
goal_type goal_to_be_reached;

extern unsigned int nr_reasons_for_staying_empty[maxsquare+4];

extern unsigned int moves_to_white_prom[nr_squares_on_board];

extern boolean testcastling;

#define SetPiece(P, SQ, SP) {e[SQ]= P; spec[SQ]= SP;}

void IntelligentRegulargoal_types(stip_length_type n);

void solve_target_position(stip_length_type n);

boolean uninterceptably_attacks_king(Side side, square from, piece p);

boolean rider_guards(square to_be_guarded, square guarding_from, int dir);
boolean guards(square to_be_guarded, piece guarding, square guarding_from);
boolean would_white_king_guard_from(square white_king_square);
boolean is_white_king_uninterceptably_attacked_by_non_king(square s);
boolean is_white_king_interceptably_attacked(void);

void intelligent_guard_flights(unsigned int nr_remaining_white_moves,
                        unsigned int nr_remaining_black_moves,
                        stip_length_type n,
                        unsigned int min_nr_captures_by_white);

void remember_to_keep_rider_line_open(square from, square to,
                                      int dir, int delta);

unsigned int find_check_directions(Side side, int check_directions[8]);

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @param si identifies slice where to start
 * @return false iff the user asks for intelligent mode, but the
 * stipulation doesn't support it
 */
boolean init_intelligent_mode(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_mate_help(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_mate_can_help(slice_index si,
                                                   stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_stalemate_help(slice_index si,
                                                    stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_stalemate_can_help(slice_index si,
                                                        stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_stalemate_help(slice_index si,
                                                    stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_stalemate_can_help(slice_index si,
                                                        stip_length_type n);

#endif
