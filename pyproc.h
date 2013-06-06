/******************** MODIFICATIONS to pyproc.h ************************
**
** This is the list of modifications done to pyproc.h
**
** Date       Who  What
**
** 2007/01/28 SE   New condition: Annan Chess
**
** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
**
** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
**
** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
**
** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
**
** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
**
** 2008/01/11 SE   New variant: Special Grids
**
** 2008/01/24 SE   New variant: Gridlines
**
** 2008/02/20 SE   Bugfix: Annan
**
** 2008/02/19 SE   New condition: AntiKoeko
**
** 2008/02/19 SE   New piece: RoseLocust
**
** 2008/02/25 SE   New piece type: Magic
**
** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
**
** 2009/02/24 SE   New pieces: 2,0-Spiralknight
**                             4,0-Spiralknight
**                             1,1-Spiralknight
**                             3,3-Spiralknight
**                             Quintessence (invented Joerg Knappen)
** 2009/06/27 SE   Extended imitators/hurdlecolorchanging to moose etc.
**
**
**************************** End of List ******************************/

#if !defined(PYPROC_H)
#define PYPROC_H

#include "py4.h"
#include "pieces/vectors.h"
#include "stipulation/goals/goals.h"
#include "utilities/boolean.h"
#include <stdio.h>

void    OpenInput(char *s);
void    CloseInput(void);
void    MultiCenter(char *s);
void    WritePosition(void);
void    WriteSquare(square a);
void    WritePiece(piece p);
boolean WriteSpec(Flags pspec, piece p, boolean printcolours);
void    WriteGrid(void);

typedef boolean (evalfunction_t)(square departure, square arrival, square capture);
typedef boolean (checkfunction_t)(square, PieNam, evalfunction_t *);

checkfunction_t pawnedpiececheck;

checkfunction_t alfilcheck;
checkfunction_t amazcheck;
checkfunction_t antilcheck;
checkfunction_t archcheck;
checkfunction_t b_hopcheck;
checkfunction_t bhuntcheck;
checkfunction_t bishopbouncercheck;
checkfunction_t bishopeaglecheck;
checkfunction_t bishopeaglecheck;
checkfunction_t bishopmoosecheck;
checkfunction_t bishopmoosecheck;
checkfunction_t bishopsparrcheck;
checkfunction_t bishopsparrcheck;
checkfunction_t bisoncheck;
checkfunction_t bobcheck;
checkfunction_t bobcheck;
checkfunction_t bouncercheck;
checkfunction_t bscoutcheck;
checkfunction_t bspawncheck;
checkfunction_t bucheck;
checkfunction_t camhopcheck;
checkfunction_t camridcheck;
checkfunction_t cardcheck;
checkfunction_t catcheck;
checkfunction_t charybdischeck;
checkfunction_t chcheck;
checkfunction_t contragrascheck;
checkfunction_t cscheck;
checkfunction_t dabcheck;
checkfunction_t dcscheck;
checkfunction_t dolphincheck;
checkfunction_t dolphincheck;
checkfunction_t doublegrasshoppercheck;
checkfunction_t doublerookhoppercheck;
checkfunction_t doublebishoppercheck;
checkfunction_t dragoncheck;
checkfunction_t eaglecheck;
checkfunction_t ecurcheck;
checkfunction_t edgehcheck;
checkfunction_t elephantcheck;
checkfunction_t equicheck;
checkfunction_t equiengcheck;
checkfunction_t equiengcheck;
checkfunction_t equifracheck;
checkfunction_t equifracheck;
checkfunction_t ferscheck;
checkfunction_t friendcheck;
checkfunction_t gicheck;
checkfunction_t gnoucheck;
checkfunction_t gnuhopcheck;
checkfunction_t gnuridcheck;
checkfunction_t gralcheck;
checkfunction_t grasshop2check;
checkfunction_t grasshop3check;
checkfunction_t gryphoncheck;
checkfunction_t gscoutcheck;
checkfunction_t huntercheck;
checkfunction_t impcheck;
checkfunction_t kangoucheck;
checkfunction_t kanglioncheck;
checkfunction_t kinghopcheck;
checkfunction_t knighthoppercheck;
checkfunction_t leap15check;
checkfunction_t leap16check;
checkfunction_t leap24check;
checkfunction_t leap25check;
checkfunction_t leap35check;
checkfunction_t leap36check;
checkfunction_t leap36check;
checkfunction_t leap37check;
checkfunction_t leocheck;
checkfunction_t lioncheck;
checkfunction_t loccheck;
checkfunction_t maocheck;
checkfunction_t maoridercheck;
checkfunction_t maoriderlioncheck;
checkfunction_t maoriderlioncheck;
checkfunction_t margueritecheck;
checkfunction_t margueritecheck;
checkfunction_t moacheck;
checkfunction_t moaridercheck;
checkfunction_t moariderlioncheck;
checkfunction_t moariderlioncheck;
checkfunction_t moosecheck;
checkfunction_t naocheck;
checkfunction_t ncheck;
checkfunction_t nequicheck;
checkfunction_t nereidecheck;
checkfunction_t nevercheck;
checkfunction_t nightlocustcheck;
checkfunction_t nightriderlioncheck;
checkfunction_t nightriderlioncheck;
checkfunction_t norixcheck;
checkfunction_t nsautcheck;
checkfunction_t okapicheck;
checkfunction_t orixcheck;
checkfunction_t orphancheck;
checkfunction_t paocheck;
checkfunction_t berolina_pawn_check;
checkfunction_t pchincheck;
checkfunction_t princcheck;
checkfunction_t querquisitecheck;
checkfunction_t querquisitecheck;
checkfunction_t r_hopcheck;
checkfunction_t rabbitcheck;
checkfunction_t rabbitcheck;
checkfunction_t rccinqcheck;
checkfunction_t refccheck;
checkfunction_t refncheck;
checkfunction_t reffoucheck;
checkfunction_t rhuntcheck;
checkfunction_t rookbouncercheck;
checkfunction_t rookeaglecheck;
checkfunction_t rookeaglecheck;
checkfunction_t rookmoosecheck;
checkfunction_t rookmoosecheck;
checkfunction_t rooksparrcheck;
checkfunction_t rooksparrcheck;
checkfunction_t rosecheck;
checkfunction_t rosehoppercheck;
checkfunction_t roselioncheck;
checkfunction_t roselocustcheck;
checkfunction_t scheck;
checkfunction_t scorpioncheck;
checkfunction_t scorpioncheck;
checkfunction_t shipcheck;
checkfunction_t skyllacheck;
checkfunction_t sparrcheck;
checkfunction_t spawncheck;
checkfunction_t tritoncheck;
checkfunction_t ubicheck;
checkfunction_t vaocheck;
checkfunction_t vizircheck;
checkfunction_t warancheck;
checkfunction_t zcheck;
checkfunction_t zebhopcheck;
checkfunction_t zebridcheck;
checkfunction_t zebucheck;
checkfunction_t sp20check;
checkfunction_t sp40check;
checkfunction_t sp11check;
checkfunction_t sp33check;
checkfunction_t sp31check;
checkfunction_t treehoppercheck;
checkfunction_t leafhoppercheck;
checkfunction_t greatertreehoppercheck;
checkfunction_t greaterleafhoppercheck;
checkfunction_t marine_knight_check;
checkfunction_t poseidon_check;

square  coinequis(square a);

boolean echecc(Side a);

boolean eval_ortho(square departure, square arrival, square capture);

void    gen_wh_ply(void);
void    gen_bl_ply(void);
void    genmove(Side a);

void    king_generate_moves(Side side, square sq_departure);
void    piece_generate_moves(Side side, square sq_departure, PieNam b);
void    hardinit(void);
boolean imok(square i, square j);

void copyply(void);
void finply(void);
void nextply(void);

boolean nocontact(square departure, square arrival, square capture, nocontactfunc_t nocontactfunc);
boolean nogridcontact(square a);

extern boolean(*is_square_attacked)(Side side_attacking,
                                    square sq_target,
                                    evalfunction_t *evaluate);

boolean is_a_square_attacked(Side side_in_check,
                             square sq_target,
                             evalfunction_t *evaluate);

boolean rcardech(square sq, square sqtest, numvec k, PieNam p, int x, evalfunction_t *evaluate );
boolean rcsech(square a, numvec b, numvec c, PieNam p, evalfunction_t *evaluate);
boolean rcspech(square a, numvec b, numvec c, PieNam p, evalfunction_t *evaluate);
void    restaure(void);
boolean ridimok(square i, square j, int diff);

typedef enum {
  angle_45,
  angle_90,
  angle_135
} angle_t;

boolean rmhopech(square a, vec_index_type kend, vec_index_type kanf, angle_t angle, PieNam p, evalfunction_t *evaluate);

boolean rrefcech(square a, square b, int c, PieNam p, evalfunction_t *evaluate);
boolean rrfouech(square sq, square sqtest, numvec k, PieNam p, int x, evalfunction_t *evaluate );
boolean rubiech(square sq, square sqtest, PieNam p,  echiquier e_ub, evalfunction_t *evaluate );

void orig_generate_moves_for_piece(Side side, square sq_departure, piece p);

extern void   (*generate_moves_for_piece)(Side side, square a, piece b);

void    PrintTime();
boolean leapcheck(square a, vec_index_type b, vec_index_type c, PieNam p, evalfunction_t *evaluate);
boolean ridcheck(square a, vec_index_type b, vec_index_type c, PieNam p, evalfunction_t *evaluate);

boolean roicheck(square a, PieNam p, evalfunction_t *evaluate);
boolean pioncheck(square a, PieNam p, evalfunction_t *evaluate);
boolean cavcheck(square a, PieNam p, evalfunction_t *evaluate);
boolean tourcheck(square a, PieNam p, evalfunction_t *evaluate);
boolean damecheck(square a, PieNam p, evalfunction_t *evaluate);
boolean foucheck(square a, PieNam p, evalfunction_t *evaluate);

boolean t_lioncheck(square i, PieNam p, evalfunction_t *evaluate);
boolean f_lioncheck(square i, PieNam p, evalfunction_t *evaluate);
boolean marine_rider_check(square a, vec_index_type b, vec_index_type c, PieNam p, evalfunction_t *evaluate);
boolean marine_leaper_check(square sq_king, vec_index_type kanf, vec_index_type kend, PieNam p, evalfunction_t *evaluate);
boolean marine_pawn_check(square sq_king, PieNam p, evalfunction_t *evaluate);
boolean marine_ship_check(square sq_king, PieNam p, evalfunction_t *evaluate);

boolean empile(square departure, square arrival, square capture);
void add_to_move_generation_stack(square sq_departure,
                                  square sq_arrival,
                                  square sq_capture);

/* Remove duplicate moves at the top of the move_generation_stack.
 * @param start start position of range where to look for duplicates
 */
void remove_duplicate_moves_of_single_piece(numecoup start);

boolean orphan_find_observation_chain(square i, PieNam porph, evalfunction_t *evaluate);
boolean reversepcheck(square a, PieNam p, evalfunction_t *evaluate);
void    gorph(square a, Side b);
void    gfriend(square a, Side b);
void    gedgeh(square a, Side b);

Token   ReadTwin(Token tk, slice_index root_slice_hook);
void WriteTwinNumber(void);
void    ErrString(char const *s);
void    StdChar(char c);
void    StdString(char const *s);
void    logChrArg(char arg);
void    logStrArg(char *arg);
void    logIntArg(int arg);
void    logLngArg(long arg);

boolean rnsingleboxtype3ech(square departure, square arrival, square capture);
boolean rbsingleboxtype3ech(square departure, square arrival, square capture);

square renfile(piece p, Flags pspec, square j, square i, square ip, Side camp);
square renrank(piece p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegelfile(piece p, Flags pspec, square j, square i, square ip, Side camp);
square renpwc(piece p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents(piece p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents_anti(piece p, Flags pspec, square j, square i, square ip, Side camp);
square rensymmetrie(piece p, Flags pspec, square j, square i, square ip, Side camp);
square renantipoden(piece p, Flags pspec, square j, square i, square ip, Side camp);
square rendiagramm(piece p, Flags pspec, square j, square i, square ip, Side camp);
square rennormal(piece p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegel(piece p, Flags pspec, square j, square i, square ip, Side camp);

void pyfputs(char const *s, FILE *f);

boolean hopimok(square i, square j, square k, numvec d, numvec d1);
boolean is_king(PieNam p);
boolean is_pawn(PieNam p);
boolean is_forwardpawn(PieNam p);
boolean is_reversepawn(PieNam p);
boolean is_short(PieNam p);

void    geskylla(square i, Side camp);
void    gecharybdis(square i, Side camp);

int len_whforcedsquare(square departure, square arrival, square capture);
int len_blforcedsquare(square departure, square arrival, square capture);

boolean riderhoppercheck(square i, vec_index_type kanf, vec_index_type kend, PieNam p,
                         int run_up, int jump,
                         evalfunction_t *evaluate);

void	pyInitSignal(void);
void	InitCond(void);
char	*MakeTimeString(void);

char *ReadPieces(int cond);


square rendiametral(piece p, Flags pspec, square j, square i, square ia, Side camp);

void LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

void transformPosition(SquareTransformation transformation);
void generate_castling(Side side);
boolean castling_is_intermediate_king_move_legal(Side side, square from, square to);

boolean nokingcontact(square ia);
boolean nowazircontact(square ia);
boolean noferscontact(square ia);
boolean noknightcontact(square ia);
boolean nocamelcontact(square ia);
boolean noalfilcontact(square ia);
boolean nozebracontact(square ia);
boolean nodabbabacontact(square ia);
boolean nogiraffecontact(square ia);
boolean noantelopecontact(square ia);

boolean castlingimok(square i, square j);
boolean maooaimok(square i, square j, square pass);
void gen_piece_aux(Side side, square sq_departure, PieNam p);
void WriteBGLNumber(char* a, long int b);
boolean CrossesGridLines(square i, square j);

boolean leapleapcheck(square	 sq_king,
                      vec_index_type kanf, vec_index_type kend,
                      int hurdletype,
                      boolean leaf,
                      PieNam p,
                      evalfunction_t *evaluate);
checkfunction_t radialknightcheck;

boolean detect_rosecheck_on_line(square sq_king,
                                 PieNam p,
                                 vec_index_type k, vec_index_type k1,
                                 numvec delta_k,
                                 evalfunction_t *evaluate);
boolean detect_roselioncheck_on_line(square sq_king,
                                     PieNam p,
                                     vec_index_type k, vec_index_type k1,
                                     numvec delta_k,
                                     evalfunction_t *evaluate);
boolean detect_rosehoppercheck_on_line(square sq_king,
                                       square sq_hurdle,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate);
boolean detect_roselocustcheck_on_line(square sq_king,
                                       square sq_arrival,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate);

extern square fromspecificsquare;

boolean eval_fromspecificsquare(square departure, square arrival, square capture);

#endif  /* PYPROC_H */
