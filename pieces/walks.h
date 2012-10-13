#if !defined(PIECES_WALKS_H)
#define PIECES_WALKS_H

/* this module deals with how pieces wakl */

#include "py.h"

/* standard walks
 * Maps King..Pawn to the associated standard walks
 * The standard walk is typically King..Pawn as well, but modified by conditions
 * such as Chinese Chess (aka Leofamily), Cavalier Majeur, Marine Chess and the
 * like.
 */
extern PieNam standard_walks[Bishop+1];

/* Initialise array standard_walks according to the current fairy conditions
 */
void initalise_standard_walks(void);

#endif
