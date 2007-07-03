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
boolean alfilcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean amazcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean antilcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean archcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean bspawncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean bucheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean camhopcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean camridcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean catcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean cardcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean chcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
square  coinequis (square a);
boolean cscheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean dabcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean dcscheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean dragoncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean eaglecheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean echecc (couleur a);
void    ecritcoup (void);
boolean ecurcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean equicheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean eval_ortho (square a, square b, square c);
boolean feebechec (boolean (*evaluate)(square,square,square) );
boolean feenechec (boolean (*evaluate)(square,square,square) );
boolean ferscheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
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
boolean gicheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
void    gkangb (square sq);
void    gkangn (square sq);
void    glocb (square a);
void    glocn (square a);
void    gmaob (square a);
void    gmaon (square a);
void    gnequib (square a);
void    gnequin (square a);
boolean gnoucheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean gnuhopcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean gnuridcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
void    groseb (square a);
void    grosen (square a);
void    gubib (square a, square b);
void    gubin (square a, square b);
void    hardinit (void);
boolean imok (square i, square j);                          /* V2.4d  TM */
boolean impcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
void    initkiller (void);
void    initneutre (couleur a);
boolean jouecoup (void);     /* V3.44  SE/TLi */
void    joueim (smallint diff);                                /* V2.4d  TM */
boolean kangoucheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean last_h_move (couleur a);
boolean legalsquare (square a, square b, square c);
boolean leocheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean libre (square a, boolean b);                  /* V3.44  TLi */
boolean lioncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean loccheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean maocheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean matant (couleur a,smallint b);
boolean mate (couleur a,smallint b);
boolean moosecheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean ncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean nequicheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
void    nextply (void);
boolean nocontact (square i,square j, square k);
boolean nogridcontact (square a);          /* py2.4c  NG */
boolean nsautcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean paocheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean patt (couleur a);
boolean pbcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean princcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean rbcircech (square i, square j, square k);               /* V2.80c  TLi */

boolean (*rbechec)(boolean (*evaluate)(square,square,square)); /* V3.71  TM */
boolean singleboxtype3_rbechec(boolean (*evaluate)(square,square,square)); /* V3.71  TM */

boolean rbimmunech (square a, square b, square c);              /* V2.80c  TLi */
boolean rcardech (square sq, square sqtest, numvec k, piece p, smallint x, boolean (*evaluate)(square,square,square) );
boolean rccinqcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean rcsech (square a, numvec b, numvec c, piece d, boolean (* evaluate)(square,square,square));        /* V2.3c  NG */
boolean refccheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean reffoucheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
void    repcoup (void);
void    restaure (void);
boolean ridimok (square i, square j, smallint diff);        /* V2.4d  TM */
boolean rmhopech (square a, numvec kend, numvec kanf, smallint b, piece c, boolean (* evaluate)(square,square,square));	/* V2.3c, V3.62  NG */
boolean rncircech (square i, square j, square k);               /* V2.80c  TLi */

boolean (*rnechec)(boolean (*evaluate)(square,square,square)); /* V3.71  TM */
boolean singleboxtype3_rnechec(boolean (*evaluate)(square,square,square)); /* V3.71  TM */

boolean rnimmunech (square a, square b, square c);              /* V2.80c  TLi */
boolean rosecheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean rrefcech (square a, square b, smallint c, piece d, boolean (* evaluate)(square,square,square));    /* V2.3c  NG */
boolean rrfouech (square sq, square sqtest, numvec k, piece p, smallint x, boolean (*evaluate)(square,square,square) );
boolean rubiech (square sq, square sqtest, piece p, /* echiquier */ smallint *e_ub, boolean (*evaluate)(square,square,square) );    /* V2.4c  NG, V3.29  NG */
boolean scheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
/* boolean shsol (couleur a, smallint b);  V3.44  TLi */
boolean soutenu (square a, square b, square c);                 /* V2.80c  TLi */
boolean sparrcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean spawncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean ubicheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean vaocheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean vizircheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean warancheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean zcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean zebhopcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */
boolean zebridcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V2.60  NG */

/* V2.70c  TLi:                                       */
boolean dsr_ant (couleur a, smallint b);
boolean dsr_e (couleur a, smallint b);
boolean dsr_parmena (couleur a, smallint b, smallint c);
smallint dsr_def (couleur a, smallint b, smallint c);
void    dsr_vari (couleur a, smallint b, smallint c, boolean d);
void    dsr_sol (couleur a,smallint b, smallint c, boolean restartenabled);   /* V3.44  TLi */

void    (*gen_bl_piece)(square a, piece b); /* V3.71  TM */
void    singleboxtype3_gen_bl_piece(square a, piece b); /* V3.71  TM */
void    (*gen_wh_piece)(square a, piece b); /* V3.71  TM */
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
boolean tritoncheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean nereidecheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean empile (square a, square b, square c);
boolean testempile (square a, square b, square c);
boolean orphancheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean friendcheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean ooorphancheck (square i, piece porph, piece p, boolean (* evaluate)(square,square,square));
void    gorph (square a, couleur b);            /* V3.14  NG */
void    gfriend (square a, couleur b);          /* V3.65  TLi */
boolean edgehcheck (square i, piece p, boolean (* evaluate)(square,square,square));
void    gedgeh (square a, couleur b);           /* V3.14  NG */
void    gmoab (square a);
void    gmoan (square a);
boolean moacheck (square i, piece p, boolean (* evaluate)(square,square,square));       /* V2.60  NG */
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
boolean rhuntcheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean bhuntcheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean stip_steingewinn (couleur a);
boolean VerifieMsg (int a);

/* V3.1  TLi */
boolean stip_ep (couleur a);
boolean moaridercheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean maoridercheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean r_hopcheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean b_hopcheck (square i, piece p, boolean (* evaluate)(square,square,square));
piece   dec_einstein (piece p);
piece   inc_einstein (piece p);
piece   norskpiece (piece p);
boolean rnanticircech (square i, square j, square k);
boolean rbanticircech (square i, square j, square k);
/* boolean rnexactech (square i, square j, square k);
** boolean rbexactech (square i, square j, square k);
*/
boolean rnultraech (square i, square j, square k);
boolean rbultraech (square i, square j, square k);

boolean rnsingleboxtype1ech(square id, square ia, square ip);  /* V3.71 TM */
boolean rbsingleboxtype1ech(square id, square ia, square ip);  /* V3.71 TM */
boolean rnsingleboxtype3ech(square id, square ia, square ip);  /* V3.71 TM */
boolean rbsingleboxtype3ech(square id, square ia, square ip);  /* V3.71 TM */
square next_latent_pawn(square s, couleur c); /* V3.71 TM */
piece next_singlebox_prom(piece p, couleur c); /* V3.71 TM */

square renfile (piece p, Flags pspec, square j, square i, couleur camp);
square renrank (piece p, Flags pspec, square j, square i, couleur camp);
square renspiegelfile (piece p, Flags pspec, square j, square i, couleur camp);
square renpwc (piece p, Flags pspec, square j, square i, couleur camp);
square renequipollents (piece p, Flags pspec, square j, square i, couleur camp);
square rensymmetrie (piece p, Flags pspec, square j, square i, couleur camp);
square renantipoden (piece p, Flags pspec, square j, square i, couleur camp);
square rendiagramm (piece p, Flags pspec, square j, square i, couleur camp);
square rennormal (piece p, Flags pspec, square j, square i, couleur camp);
square renspiegel (piece p, Flags pspec, square j, square i, couleur camp);
square rensuper (piece p, Flags pspec, square j, square i, couleur camp);

boolean bscoutcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.05  NG */
boolean gscoutcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.05  NG */

void pyfputs (char *s, FILE *f);

boolean hopimok(square i, square j, square k, smallint d); /* V3.12  TM */
boolean is_rider (piece p);                                /* TM V3.12 */
boolean is_leaper (piece p);                               /* TM V3.12 */
boolean is_simplehopper (piece p);                         /* TM V3.12 */
boolean is_pawn (piece p);                                 /* V3.22  TLi */

/*****  V3.12  TLi  *****  begin  *****/

boolean skyllacheck (square i, piece p, boolean (* evaluate)(square,square,square));
boolean charybdischeck (square i, piece p, boolean (* evaluate)(square,square,square));
void    geskylla (square i, couleur camp);
void    gecharybdis (square i, couleur camp);

/*****  V3.12  TLi  *****  end  *****/

short len_whforcedsquare(square id, square ia, square ip);  /* V3.20  NG */
short len_blforcedsquare(square id, square ia, square ip);  /* V3.20  NG */

/*****  V3.22  TLi  ***** begin *****/

boolean contragrascheck (square i, piece p, boolean (* evaluate)(square,square,square));
void    geriderhopper(square i, numvec kbeg, numvec kend,     /* V3.34  TLi */
		      smallint run_up, smallint jump, couleur camp);
boolean riderhoppercheck(square i, numvec kanf, numvec kend, piece p, /* V3.34  TLi */
			 smallint run_up, smallint jump,
			 boolean (* evaluate)(square,square,square));
/* boolean mataide (couleur a, smallint b);   V3.44  TLi */

/*****  V3.22  TLi  *****  end  *****/

boolean verifieposition(void);
void    linesolution(void);

boolean roselioncheck(square i, piece p,
		      boolean (* evaluate)(square,square,square));
boolean rosehoppercheck(square i, piece p,
		      boolean (* evaluate)(square,square,square));

/* V3.31  TLi */
/* void introseries(couleur camp);   V3.44  TLi */
boolean introseries(couleur camp, smallint n, boolean restartenabled); 
                               /* V3.44  TLi */

/* V3.32  TLi */
boolean okapicheck(square i, piece p,
		   boolean (* evaluate)(square,square,square));
boolean stip_doublemate(couleur a);

/* V3.34  TLi */
boolean leap37check(square i, piece p,
		   boolean (* evaluate)(square,square,square));
boolean grasshop2check(square i, piece p,
		   boolean (* evaluate)(square,square,square));

/* V3.35  TLi */
boolean grasshop3check(square i, piece p,
		   boolean (* evaluate)(square,square,square));
boolean stip_castling(couleur a);
/* V3.37  NG */
/* void    ProofInitialise(void);  defined in pyproof.h!!!! TLi */
void	pyInitSignal(void);
void	InitCond (void);
void	InitStip(void);
char	*MakeTimeString(void);

/* V3.38  NG */
boolean leap16check(square i, piece p,
		   boolean (* evaluate)(square,square,square));

char *ReadPieces(int cond);   /* V3.41  TLi, V3.64  ThM,NG */

boolean leap24check(square i, piece p,
                   boolean (* evaluate)(square,square,square));
boolean leap35check(square i, piece p,
                   boolean (* evaluate)(square,square,square));

/* V3.44  SE/TLi */
boolean kinghopcheck(square i, piece p,
		   boolean (* evaluate)(square,square,square));
boolean doublegrascheck(square i, piece p,
			boolean (* evaluate)(square,square,square));
boolean eval_isardam(square id, square ia, square ip);
boolean pos_legal(void);                              
void IncrementMoveNbr(void);
square rendiametral(piece p, Flags pspec, square j, square i, couleur camp);

/* V3.44  NG */
void    gequi(square i, couleur camp);
boolean orixcheck (square i, piece p, boolean (* evaluate)(square,square,square));
void    gorix(square i, couleur camp);

/* V3.46  NG */
boolean leap15check(square i, piece p, boolean (* evaluate)(square,square,square));
boolean leap25check(square i, piece p, boolean (* evaluate)(square,square,square));
boolean gralcheck (square i, piece p, boolean (* evaluate)(square,square,square));

void LaTeXOpen(void);
void LaTeXClose(void);
void LaTeXBeginDiagram(void);
void LaTeXEndDiagram(void);

boolean stip_autostale(couleur camp);
square renplus(piece p, Flags pspec, square j, square i, couleur camp);
boolean stip_circuit (couleur a);
boolean stip_exchange (couleur a);
boolean stip_circuitB (couleur a);
boolean stip_exchangeB (couleur a);

boolean nightlocustcheck (square i, piece p, boolean (* evaluate)(square,square,square));

void RotateMirror(int what);
void genrb_cast(void);
void genrn_cast(void);

boolean woohefflibre (square a, square b);	        /* V3.55  TLi */
boolean eval_wooheff(square a, square b, square c);	/* V3.55  TLi */

boolean stip_mate_or_stale(couleur camp);           /* V3.60 SE */

boolean bisoncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.60  TLi */
boolean elephantcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  TLi */
boolean naocheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  TLi */

boolean eval_shielded(square a, square b, square c);       /* V3.62 SE */

boolean rookmoosecheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  NG */
boolean rookeaglecheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  NG */
boolean rooksparrcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  NG */
boolean bishopmoosecheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  NG */
boolean bishopeaglecheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  NG */
boolean bishopsparrcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.62  NG */

boolean scorpioncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.63  NG */
boolean margueritecheck (square i, piece p, boolean (* evaluate)(square,square,square));  /* V3.64  TLi */
boolean leap36check (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.64  TLi */
boolean nightriderlioncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.64  TLi */
boolean maoriderlioncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.64  TLi */
boolean moariderlioncheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.65  TLi */
boolean dolphincheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.70  TLi */

boolean stip_any (couleur a);	/* V3.64  SE */

void    grabbitb (square sq);		/* V3.76  NG */
void    grabbitn (square sq);		/* V3.76  NG */
boolean rabbitcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.76  NG */
void    gbobb (square sq);		/* V3.76  NG */
void    gbobn (square sq);		/* V3.76  NG */
boolean bobcheck (square i, piece p, boolean (* evaluate)(square,square,square));    /* V3.76  NG */

void BeepOnSolution(int NumOfBeeps);

#ifdef WIN32
void WIN32SolvingTimeOver(int *WaitTime);
#endif	/* WIN32 */

#endif  /* PYPROC_H */
