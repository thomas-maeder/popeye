#if !defined(OPTIMISATION_INTELLIGENT_MATE_GENERATE_CHECKING_MOVES_H)
#define OPTIMISATION_INTELLIGENT_MATE_GENERATE_CHECKING_MOVES_H

#include "position/position.h"
#include "stipulation/stipulation.h"

#include <limits.h>

typedef struct
{
    int dir;       /* direction over which a rider captures or intercepts */
    square target; /* target square of the disturbance */
} disturbance_by_rider_elmt_type;

enum
{
  disturbance_by_rider_uninterceptable = INT_MAX
};

typedef disturbance_by_rider_elmt_type disturbance_by_rider_type[maxsquare+4];

/* disturbances by rider types */
extern disturbance_by_rider_type DisturbMateDirRider[4];

typedef struct
{
    unsigned int start; /* start and ... */
    unsigned int end;   /* ... end index into DisturbMateDirRider */
} disturbance_by_rider_index_range_type;

extern disturbance_by_rider_index_range_type disturbance_by_rider_index_ranges[Bishop-Queen+1];

/* disturbances by Knight */
extern int DisturbMateDirKnight[maxsquare+4];


typedef enum
{
  disturbance_by_pawn_none,
  disturbance_by_pawn_interception_single,
  disturbance_by_pawn_interception_double,
  disturbance_by_pawn_capture
} disturbance_by_pawn_type;

/* disturbances by Pawn */
extern disturbance_by_pawn_type DisturbMateDirPawn[maxsquare+4];


void intelligent_mate_generate_checking_moves(slice_index si);

#endif
