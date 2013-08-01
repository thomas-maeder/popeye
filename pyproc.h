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

#include "pieces/walks/vectors.h"
#include "stipulation/goals/goals.h"
#include "utilities/boolean.h"
#include <stdio.h>

void    OpenInput(char *s);
void    CloseInput(void);
void    MultiCenter(char *s);
void    WritePosition(void);
void    WriteSquare(square a);
void    WritePiece(PieNam p);
boolean WriteSpec(Flags pspec, PieNam p, boolean printcolours);
void    WriteGrid(void);

typedef boolean (evalfunction_t)(void);
typedef boolean (checkfunction_t)(PieNam, evalfunction_t *);

#define INVOKE_EVAL(evaluate,sq_departure,sq_arrival) \
  ( move_generation_stack[current_move[nbply]].departure = (sq_departure), \
    move_generation_stack[current_move[nbply]].arrival = (sq_arrival), \
    (*evaluate)() \
  )

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
checkfunction_t ubiubi_check;
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

boolean eval_ortho(void);

void    hardinit(void);

void copyply(void);
void finply(void);
void nextply(Side side);
void siblingply(Side side);

extern boolean(*is_square_attacked)(square sq_target, evalfunction_t *evaluate);

boolean rcardech(square sq, numvec k, PieNam p, int x, evalfunction_t *evaluate );
boolean rcsech(numvec b, numvec c, PieNam p, evalfunction_t *evaluate);
boolean rcspech(numvec b, numvec c, PieNam p, evalfunction_t *evaluate);
void    restaure(void);

boolean rrefcech(square b, int c, PieNam p, evalfunction_t *evaluate);
boolean rrfouech(square sqtest, numvec k, PieNam p, int x, evalfunction_t *evaluate );

void    PrintTime();
boolean leapcheck(vec_index_type b, vec_index_type c, PieNam p, evalfunction_t *evaluate);
boolean ridcheck(vec_index_type b, vec_index_type c, PieNam p, evalfunction_t *evaluate);

boolean roicheck(PieNam p, evalfunction_t *evaluate);
boolean pioncheck(PieNam p, evalfunction_t *evaluate);
boolean cavcheck(PieNam p, evalfunction_t *evaluate);
boolean tourcheck(PieNam p, evalfunction_t *evaluate);
boolean damecheck(PieNam p, evalfunction_t *evaluate);
boolean foucheck(PieNam p, evalfunction_t *evaluate);

boolean t_lioncheck(PieNam p, evalfunction_t *evaluate);
boolean f_lioncheck(PieNam p, evalfunction_t *evaluate);
boolean marine_rider_check(vec_index_type b, vec_index_type c, PieNam p, evalfunction_t *evaluate);
boolean marine_leaper_check(vec_index_type kanf, vec_index_type kend, PieNam p, evalfunction_t *evaluate);
boolean marine_pawn_check(PieNam p, evalfunction_t *evaluate);
boolean marine_ship_check(PieNam p, evalfunction_t *evaluate);

boolean orphan_find_observation_chain(square i, PieNam porph, evalfunction_t *evaluate);
boolean reversepcheck(PieNam p, evalfunction_t *evaluate);

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

square renfile(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renrank(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegelfile(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renpwc(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents_anti(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rensymmetrie(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renantipoden(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rendiagramm(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rennormal(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegel(PieNam p, Flags pspec, square j, square i, square ip, Side camp);

void pyfputs(char const *s, FILE *f);

boolean is_king(PieNam p);
boolean is_pawn(PieNam p);
boolean is_forwardpawn(PieNam p);
boolean is_reversepawn(PieNam p);
boolean is_short(PieNam p);

void	pyInitSignal(void);
void	InitCond(void);
char	*MakeTimeString(void);

char *ReadPieces(int cond);


square rendiametral(PieNam p, Flags pspec, square j, square i, square ia, Side camp);

void LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

void transformPosition(SquareTransformation transformation);
void generate_castling(void);
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

void WriteBGLNumber(char* a, long int b);
boolean CrossesGridLines(square i, square j);

boolean leapleapcheck(vec_index_type kanf, vec_index_type kend,
                      int hurdletype,
                      boolean leaf,
                      PieNam p,
                      evalfunction_t *evaluate);
checkfunction_t radialknightcheck;

boolean detect_rosecheck_on_line(PieNam p,
                                 vec_index_type k, vec_index_type k1,
                                 numvec delta_k,
                                 evalfunction_t *evaluate);
boolean detect_roselioncheck_on_line(PieNam p,
                                     vec_index_type k, vec_index_type k1,
                                     numvec delta_k,
                                     evalfunction_t *evaluate);
boolean detect_rosehoppercheck_on_line(square sq_hurdle,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate);
boolean detect_roselocustcheck_on_line(square sq_arrival,
                                       PieNam p,
                                       vec_index_type k, vec_index_type k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate);

extern square fromspecificsquare;

boolean eval_fromspecificsquare(void);

#endif  /* PYPROC_H */
