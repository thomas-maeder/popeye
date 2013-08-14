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
typedef boolean (checkfunction_t)(evalfunction_t *);

#define INVOKE_EVAL(evaluate,sq_departure,sq_arrival) \
  ( move_generation_stack[current_move[nbply]-1].departure = (sq_departure), \
    move_generation_stack[current_move[nbply]-1].arrival = (sq_arrival), \
    (*evaluate)() \
  )

checkfunction_t pawnedpiececheck;

checkfunction_t alfilcheck;
checkfunction_t amazcheck;
checkfunction_t antilcheck;
checkfunction_t archcheck;
checkfunction_t bishopper_check;
checkfunction_t bhuntcheck;
checkfunction_t bishopeaglecheck;
checkfunction_t bishopeaglecheck;
checkfunction_t bishopmoosecheck;
checkfunction_t bishopmoosecheck;
checkfunction_t bishopsparrcheck;
checkfunction_t bishopsparrcheck;
checkfunction_t bisoncheck;
checkfunction_t bobcheck;
checkfunction_t bobcheck;
checkfunction_t boyscout_check;
checkfunction_t bspawncheck;
checkfunction_t bucheck;
checkfunction_t camelrider_hopper_check;
checkfunction_t camel_rider_check;
checkfunction_t cardcheck;
checkfunction_t catcheck;
checkfunction_t charybdischeck;
checkfunction_t chcheck;
checkfunction_t contragrasshopper_check;
checkfunction_t spiralspringer_check;
checkfunction_t dabcheck;
checkfunction_t diagonalspiralspringer_check;
checkfunction_t dolphincheck;
checkfunction_t dolphincheck;
checkfunction_t doublegrasshopper_check;
checkfunction_t doublerookhopper_check;
checkfunction_t doublebishopper_check;
checkfunction_t dragoncheck;
checkfunction_t eaglecheck;
checkfunction_t ecurcheck;
checkfunction_t edgehog_check;
checkfunction_t elephantcheck;
checkfunction_t equihopper_check;
checkfunction_t equistopper_check;
checkfunction_t equistopper_check;
checkfunction_t nonstop_equistopper_check;
checkfunction_t nonstop_equistopper_check;
checkfunction_t ferscheck;
checkfunction_t friendcheck;
checkfunction_t gicheck;
checkfunction_t gnoucheck;
checkfunction_t gnurider_hopper_check;
checkfunction_t gnu_rider_check;
checkfunction_t gralcheck;
checkfunction_t grasshopper_2_check;
checkfunction_t grasshopper_3_check;
checkfunction_t gryphoncheck;
checkfunction_t girlscout_check;
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
checkfunction_t maorider_check;
checkfunction_t maoriderlion_check;
checkfunction_t maoriderlion_check;
checkfunction_t margueritecheck;
checkfunction_t margueritecheck;
checkfunction_t moacheck;
checkfunction_t moarider_check;
checkfunction_t moariderlion_check;
checkfunction_t moariderlion_check;
checkfunction_t moosecheck;
checkfunction_t naocheck;
checkfunction_t ncheck;
checkfunction_t nonstop_equihopper_check;
checkfunction_t nereidecheck;
checkfunction_t nevercheck;
checkfunction_t nightlocustcheck;
checkfunction_t nightriderlioncheck;
checkfunction_t nightriderlioncheck;
checkfunction_t nonstop_orix_check;
checkfunction_t nightrider_hopper_check;
checkfunction_t okapicheck;
checkfunction_t orix_check;
checkfunction_t orphancheck;
checkfunction_t paocheck;
checkfunction_t berolina_pawn_check;
checkfunction_t pchincheck;
checkfunction_t princcheck;
checkfunction_t querquisitecheck;
checkfunction_t querquisitecheck;
checkfunction_t rookhopper_check;
checkfunction_t rccinqcheck;
checkfunction_t refccheck;
checkfunction_t refncheck;
checkfunction_t reffoucheck;
checkfunction_t rhuntcheck;
checkfunction_t rookeaglecheck;
checkfunction_t rookeaglecheck;
checkfunction_t rookmoosecheck;
checkfunction_t rookmoosecheck;
checkfunction_t rooksparrcheck;
checkfunction_t rooksparrcheck;
checkfunction_t grasshopper_check;
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
checkfunction_t zebrarider_hopper_check;
checkfunction_t zebra_rider_check;
checkfunction_t zebucheck;
checkfunction_t spiralspringer20_check;
checkfunction_t spiralspringer40_check;
checkfunction_t spiralspringer11_check;
checkfunction_t spiralspringer33_check;
checkfunction_t quintessence_check;
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

boolean rcardech(square sq, numvec k, int x, evalfunction_t *evaluate );
void    restaure(void);

boolean rrefcech(square b, int c, evalfunction_t *evaluate);
boolean rrfouech(square sqtest, numvec k, int x, evalfunction_t *evaluate );

void    PrintTime();
boolean leapcheck(vec_index_type b, vec_index_type c, evalfunction_t *evaluate);

boolean roicheck(evalfunction_t *evaluate);
boolean pioncheck(evalfunction_t *evaluate);
boolean cavcheck(evalfunction_t *evaluate);
boolean tourcheck(evalfunction_t *evaluate);
boolean damecheck(evalfunction_t *evaluate);
boolean foucheck(evalfunction_t *evaluate);

boolean t_lioncheck(evalfunction_t *evaluate);
boolean f_lioncheck(evalfunction_t *evaluate);
boolean marine_rider_check(vec_index_type b, vec_index_type c, evalfunction_t *evaluate);
boolean marine_leaper_check(vec_index_type kanf, vec_index_type kend, evalfunction_t *evaluate);
boolean marine_pawn_check(evalfunction_t *evaluate);
boolean marine_ship_check(evalfunction_t *evaluate);

boolean orphan_find_observation_chain(square i, PieNam porph, evalfunction_t *evaluate);
boolean reversepcheck(evalfunction_t *evaluate);

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
boolean castling_is_intermediate_king_move_legal(Side side, square to);

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

checkfunction_t radialknightcheck;

#endif  /* PYPROC_H */
