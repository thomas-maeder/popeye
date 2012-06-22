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
#include "stipulation/goals/goals.h"
#include "solving/battle_play/attack_play.h"

#define piece_usageENUMERATORS \
    ENUMERATOR(piece_is_unused), \
    ENUMERATOR(piece_pins), \
    ENUMERATOR(piece_is_fixed_to_diagram_square), \
    ENUMERATOR(piece_intercepts), \
    ENUMERATOR(piece_intercepts_check_from_guard), \
    ENUMERATOR(piece_blocks), \
    ENUMERATOR(piece_guards), \
    ENUMERATOR(piece_gives_check), \
    ENUMERATOR(piece_is_missing), \
    ENUMERATOR(piece_is_captured), \
    ENUMERATOR(piece_is_king)

#define ENUMERATORS piece_usageENUMERATORS
#define ENUMERATION_TYPENAME piece_usage
#define ENUMERATION_DECLARE
#include "utilities/enumeration.h"
#undef piece_usageENUMERATORS

typedef struct
{
    square diagram_square;
    Flags flags;
    piece type;
    piece_usage usage;
} PIECE;

extern PIECE white[nr_squares_on_board];
extern PIECE black[nr_squares_on_board];
extern PIECE target_position[MaxPieceId+1];

enum { index_of_king = 0 };

extern unsigned int MaxPiece[nr_sides];
extern unsigned int CapturesLeft[maxply+1];

extern boolean solutions_found;

extern slice_index current_start_slice;
goal_type goal_to_be_reached;

extern unsigned int nr_reasons_for_staying_empty[maxsquare+4];

extern unsigned int moves_to_white_prom[nr_squares_on_board];

extern boolean testcastling;

extern unsigned int MovesRequired[nr_sides][maxply+1];

extern unsigned int PieceId2index[MaxPieceId+1];

#define SetPiece(P, SQ, SP) {e[SQ]= P; spec[SQ]= SP;}

void IntelligentRegulargoal_types(stip_length_type n);

void solve_target_position(void);

boolean black_pawn_attacks_king(square from);

void remember_to_keep_rider_line_open(square from, square to,
                                      int dir, int delta);

/* Detrmine whether some line is empty
 * @param start start of line
 * @param end end of line
 * @param dir direction from start to end
 * @return true iff the line is empty
 */
boolean is_line_empty(square start, square end, int dir);

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @param si identifies slice where to start
 * @return false iff the user asks for intelligent mode, but the
 * stipulation doesn't support it
 */
boolean init_intelligent_mode(slice_index si);

#endif
