#if !defined(CONDITIONS_RETOUR_H)
#define CONDITIONS_RETOUR_H

/* This module implements the fairy condition "Back-Home" */

#include "py.h"

int len_backhome(square sq_departure, square sq_arrival, square sq_capture);

/* Initialise solving in Back-Home
 */
void backhome_initialise_solving(void);

#endif
