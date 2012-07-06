#if !defined(CONDITIONS_CIRCE_CAGE_H)
#define CONDITIONS_CIRCE_CAGE_H

#include "py.h"

void circecage_find_initial_cage(ply ply_id,
                                 piece pi_departing,
                                 square sq_arrival, piece pi_captured,
                                 square *nextcage,
                                 piece *circecage_next_cage_prom,
                                 piece *circecage_next_norm_prom);

void circecage_advance_cage(ply ply_id,
                            piece pi_captured,
                            square *nextcage,
                            piece *circecage_next_cage_prom);

void circecage_advance_cage_prom(ply ply_id,
                                 square cage,
                                 piece *circecage_next_cage_prom);

void circecage_advance_norm_prom(ply ply_id,
                                 square sq_arrival, piece pi_captured,
                                 square *nextcage,
                                 piece *circecage_next_cage_prom,
                                 piece *circecage_next_norm_prom);

#endif
