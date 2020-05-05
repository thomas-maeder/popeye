#if !defined(POSITION_CASTLING_RIGHTS_H)
#define POSITION_CASTLING_RIGHTS_H

enum
{
  black_castling_rights_offset = 4u
};

/* symbols for bits and their combinations in castling rights */
typedef enum
{
  rh_cancastle = 0x01,
  ra_cancastle = 0x02,
  k_cancastle = 0x04,

  k_castling = k_cancastle|rh_cancastle,
  q_castling = k_cancastle|ra_cancastle,
  castlings = k_cancastle|ra_cancastle|rh_cancastle,

  whk_castling = k_cancastle|rh_cancastle,
  whq_castling = k_cancastle|ra_cancastle,
  wh_castlings = k_cancastle|ra_cancastle|rh_cancastle,

  blk_castling = whk_castling<<black_castling_rights_offset,
  blq_castling = whq_castling<<black_castling_rights_offset,
  bl_castlings = wh_castlings<<black_castling_rights_offset,

  wh_bl_castlings = wh_castlings|bl_castlings
} castling_rights_type;
/* NOTE: k_cancastle must be larger than the respective
 * r[ah]_cancastle or evaluations of the form
 * TSTCASTLINGFLAGMASK(nbply,White,castlings)<=k_cancastle
 * stop working. */

#endif
