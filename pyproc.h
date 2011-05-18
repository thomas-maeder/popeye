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
#include "stipulation/goals/goals.h"
#include "boolean.h"
#include <stdio.h>

void    OpenInput(char *s);
void    CloseInput(void);
void    MultiCenter(char *s);
void    WritePosition(void);
void    ResetPosition(void);
void    StorePosition(void);
void    WriteSquare(square a);
void    WritePiece(piece p);
boolean WriteSpec(Flags pspec, boolean printcolours);
void    WriteGrid(void);

typedef boolean (evalfunction_t)(square departure, square arrival, square capture);
typedef boolean (checkfunction_t)(square, piece, evalfunction_t *);
typedef void (attackfunction_t)(square, square);

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
checkfunction_t gscoutcheck;
checkfunction_t huntercheck;
checkfunction_t impcheck;
checkfunction_t kangoucheck;
checkfunction_t kinghopcheck;
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
checkfunction_t nsautcheck;
checkfunction_t okapicheck;
checkfunction_t orixcheck;
checkfunction_t orphancheck;
checkfunction_t paocheck;
checkfunction_t pbcheck;
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

square  coinequis(square a);

boolean echecc(ply ply_id, Side a);

boolean moves_equal(coup const *move1, coup const *move2);
void current(ply ply_id, coup *mov);

boolean eval_ortho(square departure, square arrival, square capture);

boolean feebechec(evalfunction_t *evaluate );
boolean feenechec(evalfunction_t *evaluate );
void    gchinb(int a, numvec b, numvec c);
void    gchinn(int a, numvec b, numvec c);
void    gcsb(square a, numvec b, numvec c);
void    gcsn(square a, numvec b, numvec c);
void    gen_wh_ply(void);
void    genmove(Side a);
void    gen_bl_ply(void);

void    genrb(square a);
void    genrn(square a);
void    gfeerblanc(square a,piece b);
void    gfeernoir(square a,piece b);
void    gkangb(square sq);
void    gkangn(square sq);
void    glocb(square a);
void    glocn(square a);
void    gmaob(square a);
void    gmaon(square a);
void    gnequib(square a);
void    gnequin(square a);
void    groseb(square a);
void    grosen(square a);
void    gubib(square a, square b);
void    gubin(square a, square b);
void    hardinit(void);
boolean imok(square i, square j);
void    initneutre(Side a);
void jouecoup_no_test(ply ply_id);
boolean jouecoup_ortho_test(ply ply_id);

typedef enum
{
  first_play,
  replay
} joue_type;

boolean jouecoup(ply ply_id, joue_type jt);

void    joueim(int diff);
boolean legalsquare(square departure, square arrival, square capture);
boolean libre(square a, boolean b);

void finply(void);
void nextply(ply parent);

boolean nocontact(square departure, square arrival, square capture, nocontactfunc_t nocontactfunc);
boolean nogridcontact(square a);
boolean immobile(Side a);
boolean rbcircech(square departure, square arrival, square capture);

extern boolean (*rbechec)(ply ply_id, evalfunction_t *evaluate);
boolean singleboxtype3_rbechec(ply ply_id, evalfunction_t *evaluate);
boolean annan_rbechec(ply ply_id, evalfunction_t *evaluate);
boolean losingchess_rbnechec(ply ply_id, evalfunction_t *evaluate);
boolean orig_rbechec(ply ply_id, evalfunction_t *evaluate);

boolean rbimmunech(square departure, square arrival, square capture);
boolean rcardech(square sq, square sqtest, numvec k, piece p, int x, evalfunction_t *evaluate );
boolean rcsech(square a, numvec b, numvec c, piece d, evalfunction_t *evaluate);
boolean rcspech(square a, numvec b, numvec c, piece d, evalfunction_t *evaluate);
void    repcoup(void);
void    restaure(void);
boolean ridimok(square i, square j, int diff);

typedef enum {
  angle_45,
  angle_90,
  angle_135
} angle_t;

boolean rmhopech(square a, numvec kend, numvec kanf, angle_t angle, piece c, evalfunction_t *evaluate);
boolean rncircech(square departure, square arrival, square capture);

extern boolean(*rnechec)(ply ply_id, evalfunction_t *evaluate);
boolean singleboxtype3_rnechec(ply ply_id, evalfunction_t *evaluate);
boolean annan_rnechec(ply ply_id, evalfunction_t *evaluate);
boolean orig_rnechec(ply ply_id, evalfunction_t *evaluate);

boolean rnimmunech(square departure, square arrival, square capture);
boolean rrefcech(square a, square b, int c, piece d, evalfunction_t *evaluate);
boolean rrfouech(square sq, square sqtest, numvec k, piece p, int x, evalfunction_t *evaluate );
boolean rubiech(square sq, square sqtest, piece p,  echiquier e_ub, evalfunction_t *evaluate );
boolean soutenu(square departure, square arrival, square capture);

extern void   (*gen_bl_piece)(square a, piece b);
void    singleboxtype3_gen_bl_piece(square a, piece b);
extern void   (*gen_wh_piece)(square a, piece b);
void    singleboxtype3_gen_wh_piece(square a, piece b);

boolean eval_madrasi(square departure, square arrival, square capture);
piece   champiece(piece p);

boolean testparalyse(square departure, square arrival, square capture);
boolean paraechecc(square departure, square arrival, square capture);

void    PrintTime();
boolean leapcheck(square a, numvec b, numvec c, piece d, evalfunction_t *evaluate);
boolean ridcheck(square a, numvec b, numvec c, piece d, evalfunction_t *evaluate);
void    gebleap(square a, numvec b, numvec c);
void    gebrid(square a, numvec b, numvec c);
void    genleap(square a, numvec b, numvec c);
void    genrid(square a, numvec b, numvec c);

boolean roicheck(square a, piece b, evalfunction_t *evaluate);
boolean pioncheck(square a, piece b, evalfunction_t *evaluate);
boolean cavcheck(square a, piece b, evalfunction_t *evaluate);
boolean tourcheck(square a, piece b, evalfunction_t *evaluate);
boolean damecheck(square a, piece b, evalfunction_t *evaluate);
boolean foucheck(square a, piece b, evalfunction_t *evaluate);

boolean t_lioncheck(square i, piece p, evalfunction_t *evaluate);
boolean f_lioncheck(square i, piece p, evalfunction_t *evaluate);
boolean marincheck(square a, numvec b, numvec c, piece d, evalfunction_t *evaluate);
boolean empile(square departure, square arrival, square capture);

/* Remove duplicate moves at the top of the move_generation_stack.
 * @param start start position of range where to look for duplicates
 */
void remove_duplicate_moves(numecoup start);

boolean ooorphancheck(square i, piece porph, piece p, evalfunction_t *evaluate);
boolean reversepcheck(square a, piece b, evalfunction_t *evaluate);
void    gorph(square a, Side b);
void    gfriend(square a, Side b);
void    gedgeh(square a, Side b);

void    gmoab(square a);
void    gmoan(square a);
Token   ReadTwin(Token tk, slice_index root_slice_hook);
void WriteTwinNumber(void);
void    ErrString(char const *s);
void    StdChar(char c);
void    StdString(char const *s);
void    logChrArg(char arg);
void    logStrArg(char *arg);
void    logIntArg(int arg);
void    logLngArg(long arg);

piece   dec_einstein(piece p);
piece   inc_einstein(piece p);
piece   norskpiece(piece p);
boolean rnanticircech(square departure, square arrival, square capture);
boolean rbanticircech(square departure, square arrival, square capture);
boolean rnultraech(square departure, square arrival, square capture);
boolean rbultraech(square departure, square arrival, square capture);

boolean rnsingleboxtype1ech(square departure, square arrival, square capture);
boolean rbsingleboxtype1ech(square departure, square arrival, square capture);
boolean rnsingleboxtype3ech(square departure, square arrival, square capture);
boolean rbsingleboxtype3ech(square departure, square arrival, square capture);
square next_latent_pawn(square s, Side c);
piece next_singlebox_prom(piece p, Side c);

square renfile(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square renrank(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegelfile(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square renpwc(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents_anti(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square rensymmetrie(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square renantipoden(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square rendiagramm(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square rennormal(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegel(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square rensuper(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);
square rencage(ply ply_id, piece p, Flags pspec, square j, square i, square ip, Side camp);

void pyfputs(char const *s, FILE *f);

boolean hopimok(square i, square j, square k, numvec d, numvec d1);
boolean is_rider(piece p);
boolean is_leaper(piece p);
boolean is_simplehopper(piece p);
boolean is_pawn(piece p);
boolean is_forwardpawn(piece p);
boolean is_reversepawn(piece p);
boolean is_short(piece p);

void    geskylla(square i, Side camp);
void    gecharybdis(square i, Side camp);

int len_whforcedsquare(square departure, square arrival, square capture);
int len_blforcedsquare(square departure, square arrival, square capture);

void    geriderhopper(square i, numvec kbeg, numvec kend,
		      int run_up, int jump, Side camp);
boolean riderhoppercheck(square i, numvec kanf, numvec kend, piece p,
                         int run_up, int jump,
                         evalfunction_t *evaluate);

void	pyInitSignal(void);
void	InitCond(void);
char	*MakeTimeString(void);

char *ReadPieces(int cond);


boolean eval_isardam(square departure, square arrival, square capture);
boolean pos_legal(void);
square rendiametral(ply ply_id, piece p, Flags pspec, square j, square i, square ia, Side camp);

void    gequi(square i, Side camp);
void    gorix(square i, Side camp);

void LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

square renplus(ply ply_id, piece p, Flags pspec, square j, square i, square ia, Side camp);

void transformPosition(SquareTransformation transformation);
void genrb_cast(void);
void genrn_cast(void);

boolean woohefflibre(square a, square b);
boolean eval_wooheff(square departure, square arrival, square capture);

boolean eval_shielded(square departure, square arrival, square capture);

void    grabbitb(square sq);
void    grabbitn(square sq);
void    gbobb(square sq);
void    gbobn(square sq);

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
boolean echecc_normal(ply ply_id, Side camp);
void gen_wh_piece_aux(square z, piece p);
void gen_bl_piece_aux(square z, piece p);
void change_observed(ply ply, square z, boolean push);
boolean observed(square a, square b);
boolean eval_BGL(square departure, square arrival, square capture);
char *WriteBGLNumber(char* a, long int b);
boolean whannan(square rear, square front);
boolean blannan(square rear, square front);
boolean CrossesGridLines(square i, square j);

boolean leapleapcheck(square	 sq_king,
                      numvec	 kanf,
                      numvec	 kend,
                      int hurdletype,
                      piece	 p,
                      evalfunction_t *evaluate);
checkfunction_t radialknightcheck;

boolean detect_rosecheck_on_line(square sq_king,
                                 piece p,
                                 numvec k, numvec k1,
                                 numvec delta_k,
                                 evalfunction_t *evaluate);
boolean detect_roselioncheck_on_line(square sq_king,
                                     piece p,
                                     numvec k, numvec k1,
                                     numvec delta_k,
                                     evalfunction_t *evaluate);
boolean detect_rosehoppercheck_on_line(square sq_king,
                                       square sq_hurdle,
                                       piece p,
                                       numvec k, numvec k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate);
boolean detect_roselocustcheck_on_line(square sq_king,
                                       square sq_arrival,
                                       piece p,
                                       numvec k, numvec k1,
                                       numvec delta_k,
                                       evalfunction_t *evaluate);
void    init_move_generation_optimizer(void);
void    finish_move_generation_optimizer(void);

/* analogon to finligne() for circle  lines */
square fin_circle_line(square sq_departure,
                       numvec k1, numvec *k2, numvec delta_k);

boolean eval_fromspecificsquare(square departure, square arrival, square capture);
void PushMagicViews(void);
void ChangeMagic(int ply, boolean push);

boolean eval_disp(square sq_departure, square sq_arrival, square sq_capture);

attackfunction_t GetRoseAttackVectors;
attackfunction_t GetRoseLionAttackVectors;
attackfunction_t GetRoseHopperAttackVectors;
attackfunction_t GetRoseLocustAttackVectors;
attackfunction_t GetMooseAttackVectors;
attackfunction_t GetRookMooseAttackVectors;
attackfunction_t GetBishopMooseAttackVectors;
attackfunction_t GetEagleAttackVectors;
attackfunction_t GetRookEagleAttackVectors;
attackfunction_t GetBishopEagleAttackVectors;
attackfunction_t GetSparrowAttackVectors;
attackfunction_t GetRookSparrowAttackVectors;
attackfunction_t GetBishopSparrowAttackVectors;
attackfunction_t unsupported_uncalled_attackfunction;
attackfunction_t GetMargueriteAttackVectors;
attackfunction_t GetBoyscoutAttackVectors;
attackfunction_t GetGirlscoutAttackVectors;
attackfunction_t GetSpiralSpringerAttackVectors;
attackfunction_t GetDiagonalSpiralSpringerAttackVectors;

void ChangeColour(square sq);

#endif  /* PYPROC_H */
