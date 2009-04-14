/******************** MODIFICATIONS to py1.h **************************
**
** Date       Who  What
** 
** 2001       TLi  Original
** 
**************************** End of List ******************************/ 
 
#if !defined(PY1_INCLUDED)
#define PY1_INCLUDED

#include "pyproc.h"

boolean fffriendcheck(
    square        i,
    piece         pfr,
    piece         p,
    evalfunction_t *evaluate);

void InitCheckDir(void);
void InitBoard(void);
void InitOpt(void);
void InitAlways(void);

typedef struct
{
    numecoup nbcou;
    square ep;
    square ep2;
    piece norm_prom;
    piece cir_prom;
    boolean norm_cham_prom;
    boolean cir_cham_prom;
    boolean Iprom;
    piece pprise;
    square super;
    unsigned char castling_flag;
} captured_ply_type;

/* Capture the current state of a ply (i.e. the current move played in
 * the ply) for later comparison.
 * @param snapshot holds the captured state
 * @param ply identifies the ply to be captured
 */
void capture_ply(captured_ply_type *snapshot, ply ply);

/* Compare the current state of a ply (i.e. the current move played in
 * the ply) to a state that was captured earlier.
 * @param snapshot ply captured earlier
 * @param ply ply to be compared
 * @return true iff the current state of ply ply is equal to that
 *         captured in *snapshot
 */
boolean is_ply_equal_to_captured(captured_ply_type const *snapshot, ply ply);

/* Make sure that a ply snapshot object isn't equal (as per
 * is_ply_equal_to_captured()) to a current ply.
 * @param snapshot address of snapshot to be evaluated
 */
void invalidate_ply_snapshot(captured_ply_type *snapshot);

#endif /*PY1_INCLUDED*/
