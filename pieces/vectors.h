#if !defined(PIECES_VECTORS_H)
#define PIECES_VECTORS_H

typedef int numvec;

extern  numvec const mixhopdata[4][17];

enum
{
  maxvec = 232
};

extern  numvec vec[maxvec + 1];

/* Symbols indicating start and end of a range inside vec - please
 * ALWAYS use these rather than int literals. */
typedef enum
{
  vec_rook_start= 1,              vec_rook_end=   4,
  vec_bishop_start=5,             vec_bishop_end=  8,
  vec_queen_start=1,              vec_queen_end=  8,
  vec_knight_start=9,             vec_knight_end=  16,
  vec_elephant_start=1,           vec_elephant_end=  16,
  vec_zebre_start=25,             vec_zebre_end=  32,
  vec_okapi_start=17,             vec_okapi_end=32,
  vec_chameau_start=33,           vec_chameau_end=  40,
  vec_bison_start=25,             vec_bison_end=  40,
  vec_equi_nonintercept_start=17, vec_equi_nonintercept_end=40,
  vec_girafe_start=41,            vec_girafe_end=  48,
  vec_antilope_start=49,          vec_antilope_end=  56,
  vec_leap05_start=57,            vec_leap05_end=  60,
  vec_bucephale_start=49,         vec_bucephale_end=  60,
  vec_dabbaba_start=61,           vec_dabbaba_end=  64,
  vec_alfil_start= 65,            vec_alfil_end=   68,
  vec_ecureuil_start= 61,         vec_ecureuil_end= 68, /* +knight vecs */
  vec_leap17_start= 69,           vec_leap17_end=   76,
  vec_leap55_start= 77,           vec_leap55_end=   80,
  vec_rccinq_start= 69,           vec_rccinq_end=   80,
  vec_leap37_start=81,            vec_leap37_end=  88,
  vec_leap16_start=89,            vec_leap16_end=  96,
  vec_leap24_start=97,            vec_leap24_end=  104,
  vec_leap35_start=105,           vec_leap35_end=  112,
  vec_leap15_start=113,           vec_leap15_end=  120,
  vec_leap25_start=121,           vec_leap25_end=  128,
  vec_leap36_start=129,           vec_leap36_end=  136,
  vec_leap03_start=137,           vec_leap03_end=  140,
  vec_leap04_start=141,           vec_leap04_end=  144,
  vec_leap06_start=145,           vec_leap06_end=  148,
  vec_leap07_start=149,           vec_leap07_end=  152,
  vec_leap33_start=153,           vec_leap33_end=  156,
  vec_leap44_start=157,           vec_leap44_end=  160,
  vec_leap66_start=161,           vec_leap66_end=  164,
  vec_leap77_start=165,           vec_leap77_end=  168,
  vec_leap26_start=169,           vec_leap26_end=  176,
  vec_leap45_start=177,           vec_leap45_end=  184,
  vec_leap46_start=185,           vec_leap46_end=  192,
  vec_leap47_start=193,           vec_leap47_end=  200,
  vec_leap56_start=201,           vec_leap56_end=  208,
  vec_leap57_start=209,           vec_leap57_end=  216,
  vec_leap67_start=217,           vec_leap67_end=  224,
  vec_leap27_start=225,           vec_leap27_end=  232
} vec_index_type;


#endif
