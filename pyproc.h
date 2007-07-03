/******************** MODIFICATIONS to pyproc.h ************************
**
** This is the list of modifications done to pyproc.h
**
** Date       Who  What
**
** 2002/05/18 NG   new pieces: rabbit, bob
**
** 2003/05/18 NG   new option: beep    (BeepOnSolution)
**
** 2004-06-20 ElB  add nevercheck
**                 introduce evalfunction_t and checkfunction_t
**
** 2004/07/19 NG   function declarations rearranged
**
**************************** End of List ******************************/

#ifndef PYPROC_H
#define PYPROC_H

#include "py4.h"

char    *GetMsgString (int nr);

void    OpenInput (char *s);
void    CloseInput (void);
void    FtlMsg (int a);              /* V2.3c  NG */
void    MultiCenter (char *s);
void    StartTimer (void);
long    StopTimer (void);
void    Tabulate (void);
void    WritePosition (void);
void    ResetPosition (void);      /* V2.90  NG */
void    StorePosition (void);      /* V2.90  NG */
void    WriteSquare (square a);
void    WritePiece (piece p);
boolean WriteSpec(Flags pspec, piece p);	/* V3.47  NG */
#ifndef MSC
int     abs (int a);
#endif

typedef boolean (evalfunction_t)(square, square, square);
typedef boolean (checkfunction_t)(square, piece, evalfunction_t *);

checkfunction_t alfilcheck;   /* V2.60  NG */
checkfunction_t amazcheck;    /* V2.60  NG */
checkfunction_t antilcheck;   /* V2.60  NG */
checkfunction_t archcheck;    /* V2.60  NG */
checkfunction_t b_hopcheck;
checkfunction_t bhuntcheck;
checkfunction_t bishopbouncercheck;    /* V4.03 SE */
checkfunction_t bishopeaglecheck;    /* V3.62  NG */
checkfunction_t bishopeaglecheck;    /* V3.62  NG */
checkfunction_t bishopmoosecheck;    /* V3.62  NG */
checkfunction_t bishopmoosecheck;    /* V3.62  NG */
checkfunction_t bishopsparrcheck;    /* V3.62  NG */
checkfunction_t bishopsparrcheck;    /* V3.62  NG */
checkfunction_t bisoncheck;    /* V3.60  TLi */
checkfunction_t bobcheck;            /* V3.76  NG */
checkfunction_t bobcheck;            /* V3.76  NG */
checkfunction_t bouncercheck;    /* V4.03 SE */
checkfunction_t bscoutcheck;    /* V3.05  NG */
checkfunction_t bspawncheck;  /* V2.60  NG */
checkfunction_t bucheck;      /* V2.60  NG */
checkfunction_t camhopcheck;  /* V2.60  NG */
checkfunction_t camridcheck;  /* V2.60  NG */
checkfunction_t cardcheck;    /* V2.60  NG */
checkfunction_t catcheck;     /* V2.60  NG */
checkfunction_t charybdischeck;
checkfunction_t chcheck;      /* V2.60  NG */
checkfunction_t contragrascheck;
checkfunction_t cscheck;       /* V2.60  NG */
checkfunction_t dabcheck;      /* V2.60  NG */
checkfunction_t dcscheck;      /* V2.60  NG */
checkfunction_t dolphincheck;        /* V3.70  TLi */
checkfunction_t dolphincheck;        /* V3.70  TLi */
checkfunction_t doublegrascheck;
checkfunction_t dragoncheck;   /* V2.60  NG */
checkfunction_t eaglecheck;    /* V2.60  NG */
checkfunction_t ecurcheck;     /* V2.60  NG */
checkfunction_t edgehcheck;
checkfunction_t elephantcheck;    /* V3.62  TLi */
checkfunction_t equicheck;     /* V2.60  NG */
checkfunction_t equiengcheck;        /* V3.78  SE */
checkfunction_t equiengcheck;        /* V3.78  SE */
checkfunction_t equifracheck;        /* V3.78  SE */
checkfunction_t equifracheck;        /* V3.78  SE */
checkfunction_t ferscheck;     /* V2.60  NG */
checkfunction_t friendcheck;
checkfunction_t gicheck;       /* V2.60  NG */
checkfunction_t gnoucheck;     /* V2.60  NG */
checkfunction_t gnuhopcheck;   /* V2.60  NG */
checkfunction_t gnuridcheck;   /* V2.60  NG */
checkfunction_t gralcheck;	/* V3.46  NG */
checkfunction_t grasshop2check;
checkfunction_t grasshop3check;
checkfunction_t gscoutcheck;    /* V3.05  NG */
checkfunction_t huntercheck;
checkfunction_t impcheck;      /* V2.60  NG */
checkfunction_t kangoucheck;   /* V2.60  NG */
checkfunction_t kinghopcheck;
checkfunction_t leap15check;	/* V3.46  NG */
checkfunction_t leap16check;	/* V3.38  NG */
checkfunction_t leap24check;
checkfunction_t leap25check;	/* V3.46  NG */
checkfunction_t leap35check;
checkfunction_t leap36check;         /* V3.64  TLi */
checkfunction_t leap36check;         /* V3.64  TLi */
checkfunction_t leap37check;
checkfunction_t leocheck;      /* V2.60  NG */
checkfunction_t lioncheck;     /* V2.60  NG */
checkfunction_t loccheck;      /* V2.60  NG */
checkfunction_t maocheck;      /* V2.60  NG */
checkfunction_t maoridercheck;
checkfunction_t maoriderlioncheck;   /* V3.64  TLi */
checkfunction_t maoriderlioncheck;   /* V3.64  TLi */
checkfunction_t margueritecheck;     /* V3.64  TLi */
checkfunction_t margueritecheck;     /* V3.64  TLi */
checkfunction_t moacheck;       /* V2.60  NG */
checkfunction_t moaridercheck;
checkfunction_t moariderlioncheck;   /* V3.65  TLi */
checkfunction_t moariderlioncheck;   /* V3.65  TLi */
checkfunction_t moosecheck;    /* V2.60  NG */
checkfunction_t naocheck;    /* V3.62  TLi */
checkfunction_t ncheck;        /* V2.60  NG */
checkfunction_t nequicheck;    /* V2.60  NG */
checkfunction_t nereidecheck;
checkfunction_t nevercheck;    /* V3.81 ElB */
checkfunction_t nightlocustcheck;
checkfunction_t nightriderlioncheck; /* V3.64  TLi */
checkfunction_t nightriderlioncheck; /* V3.64  TLi */
checkfunction_t nsautcheck;    /* V2.60  NG */
checkfunction_t okapicheck;
checkfunction_t orixcheck;	/* V3.44  NG */
checkfunction_t orphancheck;
checkfunction_t paocheck;      /* V2.60  NG */
checkfunction_t pbcheck;       /* V2.60  NG */
checkfunction_t princcheck;    /* V2.60  NG */
checkfunction_t querquisitecheck;    /* V3.78  SE */
checkfunction_t querquisitecheck;    /* V3.78  SE */
checkfunction_t r_hopcheck;
checkfunction_t rabbitcheck;         /* V3.76  NG */
checkfunction_t rabbitcheck;         /* V3.76  NG */
checkfunction_t rccinqcheck;   /* V2.60  NG */
checkfunction_t refccheck;     /* V2.60  NG */
checkfunction_t reffoucheck;   /* V2.60  NG */
checkfunction_t rhuntcheck;
checkfunction_t rookbouncercheck;    /* V4.03 SE */
checkfunction_t rookeaglecheck;      /* V3.62  NG */
checkfunction_t rookeaglecheck;      /* V3.62  NG */
checkfunction_t rookmoosecheck;      /* V3.62  NG */
checkfunction_t rookmoosecheck;      /* V3.62  NG */
checkfunction_t rooksparrcheck;      /* V3.62  NG */
checkfunction_t rooksparrcheck;      /* V3.62  NG */
checkfunction_t rosecheck;    /* V2.60  NG */
checkfunction_t rosehoppercheck;
checkfunction_t roselioncheck;
checkfunction_t scheck;        /* V2.60  NG */
checkfunction_t scorpioncheck;       /* V3.63  NG */
checkfunction_t scorpioncheck;       /* V3.63  NG */
checkfunction_t skyllacheck;
checkfunction_t sparrcheck;    /* V2.60  NG */
checkfunction_t spawncheck;    /* V2.60  NG */
checkfunction_t tritoncheck;
checkfunction_t ubicheck;      /* V2.60  NG */
checkfunction_t vaocheck;      /* V2.60  NG */
checkfunction_t vizircheck;    /* V2.60  NG */
checkfunction_t warancheck;    /* V2.60  NG */
checkfunction_t zcheck;        /* V2.60  NG */
checkfunction_t zebhopcheck;   /* V2.60  NG */
checkfunction_t zebridcheck;   /* V2.60  NG */

square  coinequis (square a);

boolean echecc (couleur a);
void    ecritcoup (void);

boolean eval_ortho (square a, square b, square c);

boolean feebechec (boolean (*evaluate)(square,square,square) );
boolean feenechec (boolean (*evaluate)(square,square,square) );
void    finkiller (void);
void    finply (void);
void    gchinb (smallint a, numvec b, numvec c);
void    gchinn (smallint a, numvec b, numvec c);
void    gcsb (square a, numvec b, numvec c);
void    gcsn (square a, numvec b, numvec c);
void    gen_wh_ply (void);
void    genmove (couleur a);
void    gen_bl_ply (void);
/* now declared in the header + index file PY4.H
** V3.22  TLi
** void    genpb (square a);
** void    genpbb (square a);
** void    genpbn (square a);
** void    genpn (square a);
*/
void    genrb (square a);
void    genrn (square a);
void    gfeerblanc (square a,piece b);
void    gfeernoir (square a,piece b);
void    gkangb (square sq);
void    gkangn (square sq);
void    glocb (square a);
void    glocn (square a);
void    gmaob (square a);
void    gmaon (square a);
void    gnequib (square a);
void    gnequin (square a);
void    groseb (square a);
void    grosen (square a);
void    gubib (square a, square b);
void    gubin (square a, square b);
void    hardinit (void);
boolean imok (square i, square j);                          /* V2.4d  TM */
void    initkiller (void);
void    initneutre (couleur a);
boolean jouecoup (void);     /* V3.44  SE/TLi */
void    joueim (smallint diff);                                /* V2.4d  TM */
boolean last_h_move (couleur a);
boolean legalsquare (square a, square b, square c);
boolean libre (square a, boolean b);                  /* V3.44  TLi */

boolean matant (couleur a,smallint b);
boolean mate (couleur a,smallint b);
void    nextply (void);
boolean nocontact (square i,square j, square k);
boolean nogridcontact (square a);          /* py2.4c  NG */
boolean patt (couleur a);
boolean rbcircech (square i, square j, square k);               /* V2.80c  TLi */

extern boolean (*rbechec)(boolean (*evaluate)(square,square,square)); /* V3.71  TM */
boolean singleboxtype3_rbechec(boolean (*evaluate)(square,square,square)); /* V3.71  TM */

boolean rbimmunech (square a, square b, square c);              /* V2.80c  TLi */
boolean rcardech (square sq, square sqtest, numvec k, piece p, smallint x, boolean (*evaluate)(square,square,square) );
boolean rcsech (square a, numvec b, numvec c, piece d, boolean (* evaluate)(square,square,square));        /* V2.3c  NG */
void    repcoup (void);
void    restaure (void);
boolean ridimok (square i, square j, smallint diff);        /* V2.4d  TM */
boolean rmhopech (square a, numvec kend, numvec kanf, smallint b, piece c, boolean (* evaluate)(square,square,square));	/* V2.3c, V3.62  NG */
boolean rncircech (square i, square j, square k);               /* V2.80c  TLi */

extern boolean (*rnechec)(boolean (*evaluate)(square,square,square)); /* V3.71  TM */
boolean singleboxtype3_rnechec(boolean (*evaluate)(square,square,square)); /* V3.71  TM */

boolean rnimmunech (square a, square b, square c);              /* V2.80c  TLi */
boolean rrefcech (square a, square b, smallint c, piece d, boolean (* evaluate)(square,square,square));    /* V2.3c  NG */
boolean rrfouech (square sq, square sqtest, numvec k, piece p, smallint x, boolean (*evaluate)(square,square,square) );
boolean rubiech (square sq, square sqtest, piece p, /* echiquier */ smallint *e_ub, boolean (*evaluate)(square,square,square) );    /* V2.4c  NG, V3.29  NG */
boolean soutenu (square a, square b, square c);                 /* V2.80c  TLi */

/* V2.70c  TLi */
boolean dsr_ant (couleur a, smallint b);
boolean dsr_e (couleur a, smallint b);
boolean dsr_parmena (couleur a, smallint b, smallint c);
smallint dsr_def (couleur a, smallint b, smallint c);
void    dsr_vari (couleur a, smallint b, smallint c, boolean d);
void    dsr_sol (couleur a,smallint b, smallint c, boolean restartenabled);   /* V3.44  TLi */

extern void    (*gen_bl_piece)(square a, piece b); /* V3.71  TM */
void    singleboxtype3_gen_bl_piece(square a, piece b); /* V3.71  TM */
extern void    (*gen_wh_piece)(square a, piece b); /* V3.71  TM */
void    singleboxtype3_gen_wh_piece(square a, piece b); /* V3.71  TM */

boolean definvref (couleur a, smallint b);
boolean invref (couleur a, smallint b);
boolean eval_madrasi (square a, square b, square c);    /* V2.80c  TLi */
piece   champiece (piece p);                    /* V2.80c  TLi */

/*------------------ V2.90c   TLi ---------------*/
boolean stip_mate (couleur a);
boolean stip_stale (couleur a);
boolean stip_dblstale (couleur a);
boolean stip_target (couleur a);
boolean stip_check (couleur a);
boolean stip_capture (couleur a);
boolean testparalyse (square a, square b, square c);
boolean paraechecc (square a, square b, square c);
boolean paralysiert (square i);
/*------------------ V2.90  NG ---------------*/
void    PrintTime (int a);
boolean leapcheck (square a, numvec b, numvec c, piece d, boolean (* evaluate)(square,square,square));
boolean ridcheck (square a, numvec b, numvec c, piece d, boolean (* evaluate)(square,square,square));
void    gebleap (square a, numvec b, numvec c);
void    gebrid (square a, numvec b, numvec c);
void    genleap (square a, numvec b, numvec c);
void    genrid (square a, numvec b, numvec c);

/*-----------     V2.90c  TLi     ----------------------*/
boolean roicheck (square a, piece b, boolean (* evaluate)(square,square,square));
boolean pioncheck (square a, piece b, boolean (* evaluate)(square,square,square));
boolean cavcheck (square a, piece b, boolean (* evaluate)(square,square,square));
boolean tourcheck (square a, piece b, boolean (* evaluate)(square,square,square));
boolean damecheck (square a, piece b, boolean (* evaluate)(square,square,square));
boolean foucheck (square a, piece b, boolean (* evaluate)(square,square,square));

boolean t_lioncheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean f_lioncheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean marincheck (square a, numvec b, numvec c, piece d, boolean (* evaluate)(square,square,square));
boolean empile (square a, square b, square c);
boolean testempile (square a, square b, square c);
boolean ooorphancheck (square i, piece porph, piece p, boolean (* evaluate)(square,square,square));
void    gorph (square a, couleur b);            /* V3.14  NG */
void    gfriend (square a, couleur b);          /* V3.65  TLi */
void    gedgeh (square a, couleur b);           /* V3.14  NG */
void    gmoab (square a);
void    gmoan (square a);
boolean InitMsgTab (Lang l, boolean Force);
Token   ReadProblem (Token tk);
void    ErrString (char *s);
void    StdChar (char c);
void    StdString (char *s);
void    ErrorMsg (int nr);
void    Message (int nr);
void    logChrArg (char arg);
void    logStrArg (char *arg);
void    logIntArg (int arg);
void    logLngArg (long arg);

void    GenMatingMove(couleur a);

/* V3.03  TLi */
boolean stip_steingewinn (couleur a);
boolean VerifieMsg (int a);

/* V3.1  TLi */
boolean stip_ep (couleur a);
piece   dec_einstein (piece p);
piece   inc_einstein (piece p);
piece   norskpiece (piece p);
boolean rnanticircech (square i, square j, square k);
boolean rbanticircech (square i, square j, square k);
boolean rnultraech (square i, square j, square k);
boolean rbultraech (square i, square j, square k);

boolean rnsingleboxtype1ech(square id, square ia, square ip);  /* V3.71 TM */
boolean rbsingleboxtype1ech(square id, square ia, square ip);  /* V3.71 TM */
boolean rnsingleboxtype3ech(square id, square ia, square ip);  /* V3.71 TM */
boolean rbsingleboxtype3ech(square id, square ia, square ip);  /* V3.71 TM */
square next_latent_pawn(square s, couleur c); /* V3.71 TM */
piece next_singlebox_prom(piece p, couleur c); /* V3.71 TM */

square renfile (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square renrank (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square renspiegelfile (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square renpwc (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square renequipollents (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square renequipollents_anti (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square rensymmetrie (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square renantipoden (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square rendiagramm (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square rennormal (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square renspiegel (piece p, Flags pspec, square j, square i, square ip, couleur camp);
square rensuper (piece p, Flags pspec, square j, square i, square ip, couleur camp);

void pyfputs (char *s, FILE *f);

boolean hopimok(square i, square j, square k, smallint d); /* V3.12  TM */
boolean is_rider (piece p);                                /* TM V3.12 */
boolean is_leaper (piece p);                               /* TM V3.12 */
boolean is_simplehopper (piece p);                         /* TM V3.12 */
boolean is_pawn (piece p);                                 /* V3.22  TLi */

/*****  V3.12  TLi  *****  begin  *****/
void    geskylla (square i, couleur camp);
void    gecharybdis (square i, couleur camp);
/*****  V3.12  TLi  *****  end  *****/

short len_whforcedsquare(square id, square ia, square ip);  /* V3.20  NG */
short len_blforcedsquare(square id, square ia, square ip);  /* V3.20  NG */

/*****  V3.22  TLi  ***** begin *****/
void    geriderhopper(square i, numvec kbeg, numvec kend,     /* V3.34  TLi */
		      smallint run_up, smallint jump, couleur camp);
boolean riderhoppercheck(square i, numvec kanf, numvec kend, piece p, /* V3.34  TLi */
			 smallint run_up, smallint jump,
			 boolean (* evaluate)(square,square,square));
/*****  V3.22  TLi  *****  end  *****/

boolean verifieposition(void);
void    linesolution(void);

/* V3.31  TLi */
boolean introseries(couleur camp, smallint n, boolean restartenabled); 

/* V3.32  TLi */
boolean stip_doublemate(couleur a);

/* V3.35  TLi */
boolean stip_castling(couleur a);
/* V3.37  NG */
void	pyInitSignal(void);
void	InitCond (void);
void	InitStip(void);
char	*MakeTimeString(void);


char *ReadPieces(int cond);   /* V3.41  TLi, V3.64  ThM,NG */


/* V3.44  SE/TLi */
boolean eval_isardam(square id, square ia, square ip);
boolean pos_legal(void);                              
void IncrementMoveNbr(void);
square rendiametral(piece p, Flags pspec, square j, square i, square ia, couleur camp);

/* V3.44  NG */
void    gequi(square i, couleur camp);
void    gorix(square i, couleur camp);

void LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

boolean stip_autostale(couleur camp);
square renplus(piece p, Flags pspec, square j, square i, square ia, couleur camp);
boolean stip_circuit (couleur a);
boolean stip_exchange (couleur a);
boolean stip_circuitB (couleur a);
boolean stip_exchangeB (couleur a);

void RotateMirror(int what);
void genrb_cast(void);
void genrn_cast(void);

boolean woohefflibre (square a, square b);	        /* V3.55  TLi */
boolean eval_wooheff(square a, square b, square c);	/* V3.55  TLi */

boolean stip_mate_or_stale(couleur camp);           /* V3.60 SE */
boolean eval_shielded(square a, square b, square c);       /* V3.62 SE */
boolean stip_any (couleur a);	     /* V3.64  SE */

void    grabbitb (square sq);	     /* V3.76  NG */
void    grabbitn (square sq);	     /* V3.76  NG */
void    gbobb (square sq);	     /* V3.76  NG */
void    gbobn (square sq);	     /* V3.76  NG */

void BeepOnSolution(int NumOfBeeps);

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
boolean maooaimok(square i, square j, square pass); /* V3.81 SE */
boolean echecc_normal(couleur camp); /* V4.03 SE */
void gen_wh_piece_aux(square z, piece p);
void gen_bl_piece_aux(square z, piece p);


#ifdef WIN32
void WIN32SolvingTimeOver(int *WaitTime);
#endif	/* WIN32 */

#endif  /* PYPROC_H */
