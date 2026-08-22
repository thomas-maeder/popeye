#if !defined(POSITION_TOPOLOGY_H)
#define POSITION_TOPOLOGY_H

#include "position/board.h"
#include "pieces/walks/vectors.h"

/* Declarations of types and functions related to board topology.
 * Board topology determines how pieces move across board edges:
 *   TOPOLOGY_STANDARD       - normal 8x8 board with edges
 *   TOPOLOGY_VERTICAL_CYLINDER - a-file and h-file connected
 */

typedef enum
{
    TOPOLOGY_STANDARD,
    TOPOLOGY_VERTICAL_CYLINDER,
    /* Phase 2+:
     *   TOPOLOGY_HORIZONTAL_CYLINDER,
     *   TOPOLOGY_TORUS,
     *   TOPOLOGY_VERTICAL_MOBIUS,
     *   TOPOLOGY_KLEIN_BOTTLE,
     *   TOPOLOGY_REAL_PROJECTIVE_PLANE,
     *   TOPOLOGY_SPHERICAL,
     */
    nrBoardTopologies
} BoardTopology;

/* Active board topology. Set during solving setup (build_solvers2).
 * Defaults to TOPOLOGY_STANDARD. */
extern BoardTopology board_topology;

/* Reset the twist phase at the start of each line traversal.
 * Must be called before a sequence of topology_step() calls along
 * one direction. Required for non-orientable topologies (Phase 3+).
 * For TOPOLOGY_VERTICAL_CYLINDER this is a no-op but harmless. */
void topology_reset_phase(void);

/* Compute the next square in direction dir from sq.
 * Applies wrapping according to the active board topology.
 *
 * Precondition: board_topology != TOPOLOGY_STANDARD
 *               (the caller must guard with a check at function entry)
 *
 * @return a valid board square index if the step lands on the board,
 *         or a blocked sentinel square if the step goes off a
 *         non-wrapping edge.
 */
square topology_step(square sq, numvec dir);

/* Add a direction vector to a square, applying topology wrapping if needed.
 * For standard topology: returns sq + dir (one predictable branch).
 * For non-standard topology: delegates to topology_step().
 * Use this in fairy piece stepping code instead of raw sq + vec[k].
 */
square topology_add(square sq, numvec dir);

#endif
