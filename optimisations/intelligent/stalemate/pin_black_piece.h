#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_PIN_BLACK_PIECE_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_PIN_BLACK_PIECE_H

#include "py.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"

boolean intelligent_stalemate_immobilise_by_pinning_any_trouble_maker(stip_length_type n,
                                                                      immobilisation_state_type const *state);

#endif
