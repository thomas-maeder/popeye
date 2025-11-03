#if !defined(PIECES_ANGLE_ANGLES_H)
#define PIECES_ANGLE_ANGLES_H

#include "pieces/walks/vectors.h"

typedef enum
{
  angle_27, /* scarabeus */
  angle_45,
  angle_90,
  angle_135,

  nr_angles
} angle_t;

/* This are the vectors for hoppers which change directions after the hurdle
 * The first index corresponds to the angle_t enumerators.
 */
extern numvec const angle_vectors[nr_angles][17];

#endif

