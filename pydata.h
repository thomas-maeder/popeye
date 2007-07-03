/********************** MODIFICATIONS to pydata.h **************************
** This is the list of modifications done to pydata.h
**
** Date       Who  What
**
** 2001/01/14 SE   new specification : ColourChanging
**                 new condition: EchecsAlphabetics
**
** 2001/02/05 TLi  new pieces: Moariderlion, AndernachGrassHopper, Friend
**
** 2001/03/06 NG   condition name changed: HurdleColourChanging
**
** 2001/05/08 SE   new conditions: CirceTurnCoats, CirceDoubleAgents, AMU,
**                 SentBerolina.
**
** 2001/05/20 SE   new stipulation: atob
**
** 2001/08/29 ThM  new condition: SingleBoxType1, Type2 or Type3
**
** 2001/10/02 TLi  new piece: Dolphin (Grashopper + Kangaroo)
**
** 2001/11/10 NG   singleboxtype? changed to singlebox type?
**		   some singlebox variable names changed ...
** 
** 2002/04/04 NG   commandline option -regression for regressiontesting
**
** 2002/05/06 NG   german name of singlebox is "NurPartiesatzSteine"
**
** 2002/05/18 NG   new pieces: rabbit, bob
**
**************************** End of List ******************************/

#ifndef PYDATA_H
#define PYDATA_H

#ifndef EXTERN
#       define EXTERN extern
#       define WE_ARE_EXTERN
#endif  /* EXTERN */

#ifdef  WE_ARE_EXTERN
extern  unsigned long   MaxMemory;
extern  char    *StartUp;
extern  long    MaxPositions;
extern  boolean LaTeXout;
extern  int     GlobalThreadCounter;	/* V3.54  NG */
extern  boolean flag_regression;	/* V3.74  NG */
#else
#	ifdef __TURBOC__ /* TurboC and BorlandC  TLi */
   /* MaxPositions is either set in inithash() using the coreleft function
   ** or entered with the -maxpos option at the DOS prompt.    TLi
   */
   long         MaxPositions;
#	else
   long         MaxPositions    = 1000000000L;
#	endif
unsigned long           MaxMemory       = 0L;
char            *StartUp = VERSIONSTRING;
boolean         LaTeXout= false;
int     	GlobalThreadCounter= 0;	/* V3.54  NG */
boolean         flag_regression= false;	/* V3.74  NG */
#endif  /* WE_ARE_EXTERN */

EXTERN  char            MMString[37];   /* MaxMemory available for diagnostic message */
					/* Safety first, think at the future (:-), V3.37  NG */

EXTERN  long            PositionCnt;

EXTERN  boolean         FlagGenMatingMove, FlagMoveOrientatedStip;    /* V3.33 TLi */
EXTERN  boolean         flagpassive, flagcapture,       /* V3.46  SE/TLi */
			flagactive,                     /* V3.47  NG */
			flagwhitemummer,
			flagblackmummer;                /* V3.0  TLi */

EXTERN  square          rb, rn;         /* V3.12  TLi */
EXTERN  square          bl_royal_sq, wh_royal_sq;       /* V3.50  TLi */

EXTERN  echiquier       e, e_ubi, e_ubi_mad;    /* V3.12c  TLi, V3.29  NG */

EXTERN  boolean         mateallowed[maxply + 1];        /* V3.45  TLi */
EXTERN  boolean         senti[maxply + 1],
			norm_cham_prom[maxply + 1],     /* V3.1  TLi */
			cir_cham_prom[maxply + 1];      /* V3.1  TLi */

EXTERN struct /* V3.72 TM */
{
    square where;
    piece what;
} sb2[maxply+1];

EXTERN  square          ppridia[maxply + 1];            /* V2.90c  TLi */

EXTERN  square
			ep[maxply + 1],                 /* V3.0  TLi */
			ep2[maxply + 1],                /* V3.22 TLi */
			whduell[maxply + 1],            /* V3.0  TLi */
			blduell[maxply + 1],            /* V3.0  TLi */
			RN_[maxply + 1],                /* V3.02 TLi */    /* H.D. 10.02.93 */
			RB_[maxply + 1];                /* V3.03 TLi */

EXTERN  square          pattfld, patti, pattj;          /* V2.70c  TLi */
EXTERN  square          cd[toppile + 1],        /* case dep. */
			ca[toppile + 1],        /* case arr. */
			cp[toppile + 1];        /* case pri. */

extern unsigned int maxinbox[]; /* V3.71 TM */
extern unsigned int boxsize; /* V3.71 TM */
EXTERN struct /* V3.71 TM */
{
    square where;
    piece what;
} sb3[toppile+1];

EXTERN  numecoup        nbcou;
EXTERN  ply             nbply;
EXTERN  square          cdkil,
			cakil,
			cpkil;
EXTERN  boolean         flagkil;
EXTERN  pilecase        kpilcd;
EXTERN  pilecase        kpilca;

EXTERN  smallint        maxflights;             /* V3.12  TLi */

/* For castling:        bit 8:  unused  */      /* V3.35  NG */
/*                      bit 7:  king e1 */
/*                      bit 6:  rook a1 */
/*                      bit 5:  rook h1 */
/*                      bit 4:  unused  */
/*                      bit 3:  king e8 */
/*                      bit 2:  rook a8 */
/*                      bit 1:  rook h8 */
EXTERN  unsigned char   castling_flag[maxply + 1];
EXTERN  unsigned char   no_castling;		/* V3.55  NG */
EXTERN  short           castling_supported;     /* V3.35  NG */
EXTERN  boolean         testcastling;		/* V3.62  NG */

/* Stop solving when a given number of solutions is reached */

EXTERN  int             solutions, maxsolutions,
                        sol_per_matingpos, maxsol_per_matingpos;
EXTERN  boolean         FlagMaxSolsReached;	/* V3.60  NG */
EXTERN  boolean         FlagShortSolsReached;	/* V3.60  NG */

/* Stop solving when a given time (in seconds) for solving is reached */

EXTERN  int             maxsolvingtime;		/* V3.53  NG */
EXTERN  boolean         FlagTimeOut;		/* V3.54  NG */
EXTERN  boolean         FlagTimerInUse;		/* V3.54  NG */

/* Optimierung direkte Aufgaben   V3.02  TLi */

EXTERN  otelement       ot[100];
EXTERN  int             otc;
EXTERN  boolean         optimize;
		
EXTERN  numvec          ortho_opt_queen[2*(haut-bas)+1],
			ortho_opt_rook[2*(haut-bas)+1],
			ortho_opt_bishop[2*(haut-bas)+1],
			ortho_opt_knight[2*(haut-bas)+1];
#define CheckDirQueen  (ortho_opt_queen+(haut-bas))
#define CheckDirRook   (ortho_opt_rook+(haut-bas))
#define CheckDirBishop (ortho_opt_bishop+(haut-bas))
#define CheckDirKnight (ortho_opt_knight+(haut-bas))
EXTERN  boolean         totalortho;

EXTERN  smallint        zzzaa[derbla - dernoi + 1];     /* nbpiece */

EXTERN  piece           pjoue[maxply + 1], pprise[maxply + 1],
			norm_prom[maxply + 1], cir_prom[maxply + 1],
			ren_parrain[maxply + 1],        /* V3.02  TLi */
			jouearr[maxply + 1];            /* V3.1  TLi */

EXTERN  boolean         flende,                 /* flagpatrsout,        */
			SatzFlag,DrohFlag,              
			rex_mad,rex_circe,rex_immun,rex_phan,    /* V2.70c,2.80c  TLi, V3.51  NG  */
                        rex_mess_ex, rex_wooz_ex;       /* V3.55  TLi */
EXTERN  boolean         pwcprom[maxply + 1];            /* V1.6c  NG */

EXTERN  pilecase        sqrenais,
			crenkam,        /* rebirth square for taking kamikaze pieces V2.4d  TM */
			super;          /* supercirce  V3.1  TLi */
EXTERN  Flags           jouespec[maxply + 1];    /* V2.4d  TM */
EXTERN  numecoup        repere[maxply + 1];

EXTERN  boolean         exist[derbla + 1];              /* V3.43  NG */
EXTERN  boolean         promonly[derbla + 1];           /* V3.43  NG */

EXTERN  boolean (* eval_white)(square,square,square),
		(* eval_2)(square,square,square),
		(* eval_black)(square,square,square),
		(* ReciStipulation)(couleur),   /* V3.31  TLi */
		(* NonReciStipulation)(couleur), /* V3.31  TLi */
		(* stipulation)(couleur);       /* V2.90c  TLi */
EXTERN  short   (* white_length)(square,square,square), /* V3.0  TLi */
		(* black_length)(square, square, square);       /* V3.0  TLi */

EXTERN  square  (* immunrenai)(piece, Flags, square, square, couleur),  /* V3.1  TLi */
		(* circerenai)(piece, Flags, square, square, couleur),
		(* antirenai)(piece, Flags, square, square, couleur),
		(* marsrenai)(piece, Flags, square, square, couleur);  /* V3.46  SE/TLi */
EXTERN  boolean         anycirce, anycirprom, anyanticirce, anyimmun, anyclone;  /* V3.1  TLi */

/* new anticirce implementation -- V.38  TLi */
EXTERN  boolean         AntiCirCheylan;   /* V3.38  TLi */
EXTERN  boolean         SingleBoxType;    /* V3.73  NG */
EXTERN  boolean         sbtype1, sbtype2, sbtype3;    /* V3.73  NG */

EXTERN  pilecouleur     trait;
EXTERN  boolean         flagfee,                /* V2.51  NG */
			flagriders, flagleapers, flagsimplehoppers, 
			flagveryfairy,          /* V3.12  TM */
			empilegenre,            /* V2.51  NG */
			flaglegalsquare, jouegenre,     /* V2.60  NG  V2.70c  TLi */
			repgenre, change_moving_piece;  /* V3.1  TLi */

EXTERN  square          cirrenroib, cirrenroin,         /* V3.1  TLi */
			immrenroib, immrenroin;

EXTERN  boolean         PrintReciSolution;              /* V3.31  TLi */

EXTERN  smallint        marge;
EXTERN  numecoup        debut;
EXTERN  numecoup        testdebut;                      /* V3.00  NG */
EXTERN  tab             tabsol;                         /* V2.70c  TLi */

EXTERN  Flags           pprispec[maxply + 1];

EXTERN  couleur         zzzag[haut - bas + 1];  /* sqcolor */           /* V2.60  NG */

EXTERN  Flags           spec[maxsquare+4];     /* V3.33  TLi, V3.55  TLi */
EXTERN  smallint        zzzaj[haut - bas + 1];  /* gridnumber */        /* V2.60  NG */

EXTERN  smallint        NonTrivialNumber, NonTrivialLength;           /* V3.32  TLi */

/*****  V3.20  TLi  *****  begin  *****/
EXTERN  Flags           zzzan[haut - bas + 1];  /* sq_spec */
EXTERN  boolean         we_generate_exact, there_are_consmoves,
			wh_exact, bl_exact, wh_ultra, bl_ultra; /* V3.22  TLi */
/*****  V3.20  TLi  *****   end   *****/

							       /* V4.2d  TM */
EXTERN  smallint        inum[maxply + 1];       /* aktuelle Anzahl Imitatoren */

EXTERN  imarr           isquare;                /* Imitatorstandfelder */

EXTERN  boolean         Iprom[maxply + 1];      /* Imitatorumwandlung? */
EXTERN  square          im0;                    /* Standort des ersten Imitators */

EXTERN  smallint        enonce,
			zugebene, maxdefen, droh;         /* V2.1c  TLi */
EXTERN  smallint        introenonce;            /* V3.44  TLi */
/* EXTERN  smallint        numenroute;    V3.44  TLi */
EXTERN  smallint        MoveNbr, RestartNbr;            /* V3.44  TLi */
EXTERN  boolean         restartenabled;                 /* V3.44  TLi */
EXTERN  couleur         neutcoul;
EXTERN  boolean         calctransmute,                  /* V3.0  TLi */
			orph_refking;                   /* V3.0  TLi */

EXTERN  boolean         flag_testlegality, k_cap,       /* V3.44  SE/TLi */
			flag_writinglinesolution,       /* V3.44  TLi */
			anymars,                        /* V3.46  SE/TLi */
			flag_madrasi;                   /* V3.60  TLi */
EXTERN  boolean         is_phantomchess;                /* V3.47  NG */
EXTERN  square          marsid;                         /* V3.46  SE/TLi */
EXTERN  square          TargetField, ReciTargetField, NonReciTargetField;   /* V3.31  TLi */

EXTERN  boolean         DoubleMate, CounterMate, ReciDoubleMate, NonReciDoubleMate;        /* V3.32  TLi */

EXTERN  piece           getprompiece[derbla + 1];       /* it's a inittable !  V2.60  NG */
EXTERN  piece           checkpieces[derbla - leob + 1]; /* only fairies !  V2.60  NG */

EXTERN  piece           transmpieces[derbla];           /* V3.0  TLi */
EXTERN  piece           orphanpieces[derbla + 1];       /* V3.0  TLi */

EXTERN  boolean         ProofFairy;                     /* V3.36  TLi */

EXTERN  couleur         maincamp;                       /* V3.42  NG */

EXTERN  piece           NextChamCircePiece[PieceCount];  /* V3.45  TLi */
EXTERN  boolean         InitChamCirce;                   /* V3.45  TLi */

EXTERN unsigned char    more_ren;      					    /* V3.50 SE */
EXTERN boolean          IsardamB,PatienceB,SentPionAdverse,SentPionNeutral,
			flagAssassin,flag_nk,jouetestgenre;				 /* V3.50 SE */
EXTERN boolean          flagdiastip, flag_dontaddk;      /* V3.50  SE */
EXTERN boolean          mate_or_stale_patt;		/* V3.60  SE */
EXTERN smallint         max_pn, max_pb;            	/* V3.60  SE */
EXTERN boolean          flagmaxi, flagultraschachzwang, flagparasent; /* V3.62 SE */
EXTERN smallint         max_pt;               /* V3.63  SE */
EXTERN boolean          isapril[derbla + 1];

EXTERN boolean          checkhopim;		/* V3.64 SE */
EXTERN square           chop[toppile + 1];	/* V3.64 SE */

EXTERN piece            sentinelb, sentineln;	/* V3.70 SE */
EXTERN boolean          anytraitor;		/* V3.70 SE */
EXTERN boolean          att_1[maxply + 1];	/* V3.70 SE */
EXTERN boolean          flag_atob;		/* V3.70 SE */

#ifdef WE_ARE_EXTERN
	extern PieTable PieNamString[LangCount];
#else
	PieTable PieNamString[LangCount] = {
	{ /* French PieNamString */
	/*  0*/ {'.',' '}, /* vide */
	/*  1*/ {' ',' '}, /* hors echiquier */
	/*  2*/ {'r',' '}, /* roi */
	/*  3*/ {'p',' '}, /* pion */
	/*  4*/ {'d',' '}, /* dame */
	/*  5*/ {'c',' '}, /* cavalier */
	/*  6*/ {'t',' '}, /* tour */
	/*  7*/ {'f',' '}, /* fou */
	/*  8*/ {'l','e'}, /* leo */
	/*  9*/ {'m','a'}, /* mao */
	/* 10*/ {'p','a'}, /* pao */
	/* 11*/ {'v','a'}, /* vao */
	/* 12*/ {'r','o'}, /* rose */
	/* 13*/ {'s',' '}, /* sauterelle */
	/* 14*/ {'n',' '}, /* noctambule */
	/* 15*/ {'z',' '}, /* zebre */
	/* 16*/ {'c','h'}, /* chameau */
	/* 17*/ {'g','i'}, /* girafe */
	/* 18*/ {'c','c'}, /* cavalier racine carree cinquante */
	/* 19*/ {'b','u'}, /* bucephale  (cheval d'Alexandre le Grand) */
	/* 20*/ {'v','i'}, /* vizir */
	/* 21*/ {'a','l'}, /* alfil */
	/* 22*/ {'f','e'}, /* fers */
	/* 23*/ {'d','a'}, /* dabbabba */
	/* 24*/ {'l','i'}, /* lion */
	/* 25*/ {'e','q'}, /* equisauteur (non-stop) */
	/* 26*/ {'l',' '}, /* locuste */
	/* 27*/ {'p','b'}, /* pion berolina */
	/* 28*/ {'a','m'}, /* amazone */
	/* 29*/ {'i','m'}, /* imperatrice */
	/* 30*/ {'p','r'}, /* princesse */
	/* 31*/ {'g',' '}, /* gnou */
	/* 32*/ {'a','n'}, /* antilope */
	/* 33*/ {'e','c'}, /* ecureuil */
	/* 34*/ {'v',' '}, /* varan */
	/* 35*/ {'d','r'}, /* dragon */
	/* 36*/ {'k','a'}, /* kangourou */
	/* 37*/ {'c','s'}, /* cavalier spirale */
	/* 38*/ {'u','u'}, /* UbiUbi */
	/* 39*/ {'h','a'}, /* hamster: sauterelle a 180 degre */
	/* 40*/ {'e',' '}, /* elan: sauterelle a 45 degre */
	/* 41*/ {'a','i'}, /* aigle: sauterelle a 90 degre */
	/* 42*/ {'m',' '}, /* moineaux: sauterelle a 135 degre */
	/* 43*/ {'a','r'}, /* archeveque */
	/* 44*/ {'f','r'}, /* fou rebondissant */
	/* 45*/ {'c','a'}, /* cardinal */
	/* 46*/ {'s','n'}, /* noctambule sauteur */
	/* 47*/ {'p','i'}, /* piece impuissant */
	/* 48*/ {'c','n'}, /* chameau noctambule */
	/* 49*/ {'z','n'}, /* zebre noctambule */
	/* 50*/ {'g','n'}, /* gnou noctambule */
	/* 51*/ {'s','c'}, /* chameau sauteur */
	/* 52*/ {'s','z'}, /* zebre sauteur */
	/* 53*/ {'s','g'}, /* gnou sauteur */
	/* 54*/ {'c','d'}, /* cavalier spirale diagonale */
	/* 55*/ {'c','r'}, /* cavalier rebondissant */
	/* 56*/ {'e','a'}, /* equisauteur anglais */
	/* 57*/ {'c','t'}, /* CAT (= cavalier de troie) */
	/* 58*/ {'b','s'}, /* berolina superpion */
	/* 59*/ {'s','p'}, /* superpion */
	/* 60*/ {'t','l'}, /* tour-lion */
	/* 61*/ {'f','l'}, /* fou-lion */
	/* 62*/ {'s','i'}, /* sirene */
	/* 63*/ {'t','r'}, /* triton */
	/* 64*/ {'n','e'}, /* nereide */
	/* 65*/ {'o',' '}, /* orphan */
	/* 66*/ {'e','h'}, /* "edgehog", "randschwein" */
	/* 67*/ {'m','o'}, /* moa */
	/* 68*/ {'t','c'}, /* tour/fou chasseur */
	/* 69*/ {'f','c'}, /* fou/tour chasseur */
	/* 70*/ {'a','o'}, /* noctambule mao */
	/* 71*/ {'o','a'}, /* noctambule moa */
	/* 72*/ {'s','t'}, /* sauterelle tour */
	/* 73*/ {'s','f'}, /* sauterelle fou */
	/* 74*/ {'r','e'}, /* roi des elfes */
	/* 75*/ {'b','t'}, /* boy-scout */
	/* 76*/ {'g','t'}, /* girl-scout */
	/* 77*/ {'s','k'}, /* skylla */
	/* 78*/ {'c','y'}, /* charybdis */
	/* 79*/ {'s','a'}, /* sauterelle contA */
	/* 80*/ {'r','l'}, /* rose lion */
	/* 81*/ {'r','s'}, /* rose sauteur */
	/* 82*/ {'o','k'}, /* okapi */
	/* 83*/ {'3','7'}, /* 3:7-cavalier */
	/* 84*/ {'s','2'}, /* sauterelle-2 */
	/* 85*/ {'s','3'}, /* sauterelle-3 */
	/* 86*/ {'1','6'}, /* 1:6-cavalier */
	/* 87*/ {'2','4'}, /* 2:4-cavalier */
	/* 88*/ {'3','5'}, /* 3:5-cavalier */
	/* 89*/ {'d','s'}, /* double sauterelle */
	/* 90*/ {'s','r'}, /* roi sauteur */
	/* 91*/ {'o','r'}, /* orix */
	/* 92*/ {'1','5'}, /* 1:5-cavalier */
	/* 93*/ {'2','5'}, /* 2:5-cavalier */
	/* 94*/ {'g','l'}, /* gral */
        /* 95*/ {'l','t'}, /* tour locuste */
        /* 96*/ {'l','f'}, /* fou locuste */
        /* 97*/ {'l','n'}, /* noctambule locuste */
        /* 98*/ {'v','s'}, /* vizir sauteur */
        /* 99*/ {'f','s'}, /* fers sauteur */
	/*100*/ {'b','i'}, /* bison */
	/*101*/ {'e','l'}, /* elephant */
	/*102*/ {'n','a'}, /* Nao */
	/*103*/ {'e','t'}, /* elan tour: sauterelle tour a 45 degre */
	/*104*/ {'a','t'}, /* aigle tour: sauterelle tour a 90 degre */
	/*105*/ {'m','t'}, /* moineaux tour: sauterelle tour a 135 degre */
	/*106*/ {'e','f'}, /* elan fou: sauterelle fou a 45 degre */
	/*107*/ {'a','f'}, /* aigle fou: sauterelle fou a 90 degre */
	/*108*/ {'m','f'}, /* moineaux fou: sauterelle fou a 135 degre */
	/*109*/ {'r','a'}, /* rao: rose chinois */
	/*110*/ {'s','o'}, /* scorpion: roi des elfes et sauterelle */
	/*111*/ {'m','g'}, /* marguerite */
	/*112*/ {'3','6'}, /* 3:6-cavalier */
        /*113*/ {'n','l'}, /* noctambule lion */
        /*114*/ {'m','l'}, /* noctambule mao lion */
        /*115*/ {'m','m'}, /* noctambule moa lion */
        /*116*/ {'a','s'}, /* sauterelle d'Andernach */
        /*117*/ {'a',' '}, /* ami */
        /*118*/ {'d','n'}, /* dauphin */
        /*119*/ {'l','a'}, /* lapin */
        /*120*/ {'b','o'}  /* bob */
	},{ /* German PieNamString */
	/*  0*/ {'.',' '}, /* leer */
	/*  1*/ {' ',' '}, /* ausserhalb des Brettes */
	/*  2*/ {'k',' '}, /* Koenig     0,1 + 1,1 S */
	/*  3*/ {'b',' '}, /* Bauer */
	/*  4*/ {'d',' '}, /* Dame       0,1 + 1,1 R */
	/*  5*/ {'s',' '}, /* Springer   1,2 S */
	/*  6*/ {'t',' '}, /* Turm       0,1 R */
	/*  7*/ {'l',' '}, /* Laeufer    1,1 R */
	/*  8*/ {'l','e'}, /* Leo */
	/*  9*/ {'m','a'}, /* Mao */
	/* 10*/ {'p','a'}, /* Pao */
	/* 11*/ {'v','a'}, /* Vao */
	/* 12*/ {'r','o'}, /* Rose */
	/* 13*/ {'g',' '}, /* Grashuepfer 0,1 + 1,1 H */
	/* 14*/ {'n',' '}, /* Nachtreiter 1,2 R */
	/* 15*/ {'z',' '}, /* Zebra       2,3 S */
	/* 16*/ {'c','a'}, /* Kamel       1,3 S */
	/* 17*/ {'g','i'}, /* Giraffe     1,4 S */
	/* 18*/ {'w','f'}, /* Wurzel-50-Springer 5,5 + 1,7 S */
	/* 19*/ {'b','u'}, /* Wurzel-25-Springer 3,4 + 5,0 S  (Bukephalos: Pferd Alexanders des Grossen) */
	/* 20*/ {'w','e'}, /* Wesir */
	/* 21*/ {'a','l'}, /* Alfil */
	/* 22*/ {'f','e'}, /* Fers */
	/* 23*/ {'d','a'}, /* Dabbaba */
	/* 24*/ {'l','i'}, /* Lion */
	/* 25*/ {'n','e'}, /* Non-stop Equihopper */
	/* 26*/ {'h',' '}, /* Heuschrecke */
	/* 27*/ {'b','b'}, /* Berolina Bauer */
	/* 28*/ {'a','m'}, /* Amazone    (Dame + Springer) */
	/* 29*/ {'k','n'}, /* Kaiserin   (Turm + Springer) */
	/* 30*/ {'p','r'}, /* Prinzessin (Laeufer + Springer) */
	/* 31*/ {'g','n'}, /* Gnu        (Springer + Kamel) */
	/* 32*/ {'a','n'}, /* Antilope    3,4 S */
	/* 33*/ {'e','i'}, /* Eichhoernchen      (Springer + Alfil + Dabbabba) */
	/* 34*/ {'w','a'}, /* Waran      (Turm + Nachtreiter) */
	/* 35*/ {'d','r'}, /* Drache     (Springer + Bauer) */
	/* 36*/ {'k','a'}, /* Kaenguruh */
	/* 37*/ {'s','s'}, /* Spiralspringer */
	/* 38*/ {'u','u'}, /* UbiUbi */
	/* 39*/ {'h','a'}, /* Hamster: 180 Grad Grashuepfer */
	/* 40*/ {'e','l'}, /* Elch: 45 Grad Grashuepfer */
	/* 41*/ {'a','d'}, /* Adler: 90 Grad Grashuepfer */
	/* 42*/ {'s','p'}, /* Spatz: 135 Grad Grashuepfer */
	/* 43*/ {'e','r'}, /* Erzbischof */
	/* 44*/ {'r','l'}, /* Reflektierender Laeufer */
	/* 45*/ {'k','l'}, /* Kardinal */
	/* 46*/ {'n','h'}, /* Nachtreiterhuepfer */
	/* 47*/ {'d','u'}, /* Dummy */
	/* 48*/ {'c','r'}, /* Kamelreiter */
	/* 49*/ {'z','r'}, /* Zebrareiter */
	/* 50*/ {'g','r'}, /* Gnureiter */
	/* 51*/ {'c','h'}, /* Kamelreiterhuepfer */
	/* 52*/ {'z','h'}, /* Zebrareiterhuepfer */
	/* 53*/ {'g','h'}, /* Gnureiterhuepfer */
	/* 54*/ {'d','s'}, /* DiagonalSpiralSpringer */
	/* 55*/ {'r','s'}, /* Reflektierender Springer */
	/* 56*/ {'e','q'}, /* Equihopper */
	/* 57*/ {'c','t'}, /* CAT (= cavalier de troie) */
	/* 58*/ {'b','s'}, /* Berolina Superbauer */
	/* 59*/ {'s','b'}, /* Superbauer */
	/* 60*/ {'t','l'}, /* Turm-Lion */
	/* 61*/ {'l','l'}, /* Laeufer-Lion */
	/* 62*/ {'s','i'}, /* Sirene */
	/* 63*/ {'t','r'}, /* Triton */
	/* 64*/ {'n','d'}, /* Nereide */
	/* 65*/ {'o',' '}, /* Orphan */
	/* 66*/ {'r','d'}, /* Randschwein */
	/* 67*/ {'m','o'}, /* Moa */
	/* 68*/ {'t','j'}, /* Turm/Laeufer-Jaeger */
	/* 69*/ {'l','j'}, /* Laeufer/Turm-Jaeger */
	/* 70*/ {'a','o'}, /* Maoreiter */
	/* 71*/ {'o','a'}, /* Moareiter */
	/* 72*/ {'t','h'}, /* Turmhuepfer */
	/* 73*/ {'l','h'}, /* Laeuferhuepfer */
	/* 74*/ {'e','k'}, /* Erlkoenig */
	/* 75*/ {'p','f'}, /* Pfadfinder */
	/* 76*/ {'p','n'}, /* Pfadfinderin */
	/* 77*/ {'s','k'}, /* Skylla */
	/* 78*/ {'c','y'}, /* Charybdis */
	/* 79*/ {'k','g'}, /* KontraGrashuepfer */
	/* 80*/ {'r','n'}, /* RosenLion */
	/* 81*/ {'r','h'}, /* RosenHuepfer */
	/* 82*/ {'o','k'}, /* Okapi */
	/* 83*/ {'3','7'}, /* 3:7-Springer */
	/* 84*/ {'g','2'}, /* Grashuepfer-2 */
	/* 85*/ {'g','3'}, /* Grashuepfer-3 */
	/* 86*/ {'1','6'}, /* 1:6-Springer */
	/* 87*/ {'2','4'}, /* 2:4-Springer */
	/* 88*/ {'3','5'}, /* 3:5-Springer */
	/* 89*/ {'d','g'}, /* DoppelGrashuepfer */
	/* 90*/ {'k','h'}, /* Koenighuepfer */
	/* 91*/ {'o','r'}, /* Orix */
	/* 92*/ {'1','5'}, /* 1:5-Springer */
	/* 93*/ {'2','5'}, /* 2:5-Springer */
	/* 94*/ {'g','l'}, /* Gral */
        /* 95*/ {'h','t'}, /* Turmheuschrecke */
        /* 96*/ {'h','l'}, /* Laeuferheuschrecke */
        /* 97*/ {'h','n'}, /* Nachtreiterheuschrecke */
        /* 98*/ {'w','r'}, /* Wesirreiter */
        /* 99*/ {'f','r'}, /* Fersreiter */
	/*100*/ {'b','i'}, /* Bison */
	/*101*/ {'e','t'}, /* Elefant */
	/*102*/ {'n','a'}, /* Nao */
	/*103*/ {'t','c'}, /* Turmelch: 45 Grad Grashuepfer */
	/*104*/ {'t','a'}, /* Turmadler: 90 Grad Grashuepfer */
	/*105*/ {'t','s'}, /* Turmspatz: 135 Grad Grashuepfer */
	/*106*/ {'l','c'}, /* Laeuferelch: 45 Grad Grashuepfer */
	/*107*/ {'l','a'}, /* Laeuferadler: 90 Grad Grashuepfer */
	/*108*/ {'l','s'}, /* Laeuferspatz: 135 Grad Grashuepfer */
	/*109*/ {'r','a'}, /* Rao: Chinesische Rose */
	/*110*/ {'s','o'}, /* Skorpion: Erlkoenig + Grashuepfer */
	/*111*/ {'m','g'}, /* Marguerite */
	/*112*/ {'3','6'}, /* 3:6 Springer */
        /*113*/ {'n','l'}, /* Nachtreiterlion */
        /*114*/ {'m','l'}, /* Maoreiterlion */
        /*115*/ {'m','m'}, /* Moareiterlion */
        /*116*/ {'a','g'}, /* AndernachGrashuepfer */
        /*117*/ {'f',' '}, /* Freund */
        /*118*/ {'d','e'}, /* Delphin */
        /*119*/ {'h','e'}, /* Hase: Lion-Huepfer ueber 2 Boecke */
        /*120*/ {'b','o'}  /* Bob: Lion-Huepfer ueber 4 Boecke */
	},{/* English PieNamString */
	/*  0*/ {'.',' '}, /* empty */
	/*  1*/ {' ',' '}, /* outside board */
	/*  2*/ {'k',' '}, /* king */
	/*  3*/ {'p',' '}, /* pawn */
	/*  4*/ {'q',' '}, /* queen */
	/*  5*/ {'s',' '}, /* knight */
	/*  6*/ {'r',' '}, /* rook */
	/*  7*/ {'b',' '}, /* bishop */
	/*  8*/ {'l','e'}, /* leo */
	/*  9*/ {'m','a'}, /* mao */
	/* 10*/ {'p','a'}, /* pao */
	/* 11*/ {'v','a'}, /* vao */
	/* 12*/ {'r','o'}, /* rose */
	/* 13*/ {'g',' '}, /* grashopper */
	/* 14*/ {'n',' '}, /* nightrider */
	/* 15*/ {'z',' '}, /* zebra */
	/* 16*/ {'c','a'}, /* camel */
	/* 17*/ {'g','i'}, /* giraffe */
	/* 18*/ {'r','f'}, /* root-50-leaper */
	/* 19*/ {'b','u'}, /* root-25-leaper  (bucephale: horse of Alexander the Great) */
	/* 20*/ {'w','e'}, /* wazir */
	/* 21*/ {'a','l'}, /* alfil */
	/* 22*/ {'f','e'}, /* fers */
	/* 23*/ {'d','a'}, /* dabbabba */
	/* 24*/ {'l','i'}, /* lion */
	/* 25*/ {'n','e'}, /* nonstop equihopper */
	/* 26*/ {'l',' '}, /* locust */
	/* 27*/ {'b','p'}, /* berolina pawn */
	/* 28*/ {'a','m'}, /* amazon */
	/* 29*/ {'e','m'}, /* empress */
	/* 30*/ {'p','r'}, /* princess */
	/* 31*/ {'g','n'}, /* gnu */
	/* 32*/ {'a','n'}, /* antelope */
	/* 33*/ {'s','q'}, /* squirrel */
	/* 34*/ {'w','a'}, /* waran */
	/* 35*/ {'d','r'}, /* dragon */
	/* 36*/ {'k','a'}, /* kangaroo */
	/* 37*/ {'s','s'}, /* Spiralspringer */
	/* 38*/ {'u','u'}, /* UbiUbi */
	/* 39*/ {'h','a'}, /* hamster: 180 degree grashopper */
	/* 40*/ {'m',' '}, /* moose: 45 degree grashopper */
	/* 41*/ {'e','a'}, /* eagle: 90 degree grashopper */
	/* 42*/ {'s','w'}, /* sparrow: 135 degree grashopper */
	/* 43*/ {'a','r'}, /* archbishop */
	/* 44*/ {'r','b'}, /* reflecting bishop */
	/* 45*/ {'c',' '}, /* cardinal */
	/* 46*/ {'n','h'}, /* nightrider-hopper */
	/* 47*/ {'d','u'}, /* dummy */
	/* 48*/ {'c','r'}, /* camelrider */
	/* 49*/ {'z','r'}, /* zebrarider */
	/* 50*/ {'g','r'}, /* gnurider */
	/* 51*/ {'c','h'}, /* camelrider hopper */
	/* 52*/ {'z','h'}, /* zebrarider hopper */
	/* 53*/ {'g','h'}, /* gnurider hopper */
	/* 54*/ {'d','s'}, /* DiagonalSpiralSpringer */
	/* 55*/ {'b','k'}, /* bouncy knight */
	/* 56*/ {'e','q'}, /* equihoppper */
	/* 57*/ {'c','t'}, /* CAT  ( cavalier de troie) */
	/* 58*/ {'b','s'}, /* berolina superpawn */
	/* 59*/ {'s','p'}, /* superpawn */
	/* 60*/ {'r','l'}, /* rook-lion */
	/* 61*/ {'b','l'}, /* bishop-lion */
	/* 62*/ {'s','i'}, /* sirene */
	/* 63*/ {'t','r'}, /* triton */
	/* 64*/ {'n','d'}, /* nereide */
	/* 65*/ {'o',' '}, /* orphan */
	/* 66*/ {'e','h'}, /* edgehog */
	/* 67*/ {'m','o'}, /* moa */
	/* 68*/ {'r','r'}, /* rook/bishop-hunter */
	/* 69*/ {'b','r'}, /* bishop/rook-hunter */
	/* 70*/ {'a','o'}, /* maorider */
	/* 71*/ {'o','a'}, /* moarider */
	/* 72*/ {'r','h'}, /* rookhopper */
	/* 73*/ {'b','h'}, /* bishophopper */
	/* 74*/ {'e','k'}, /* erlking */
	/* 75*/ {'b','t'}, /* boyscout */
	/* 76*/ {'g','t'}, /* girlscout */
	/* 77*/ {'s','k'}, /* Skylla */
	/* 78*/ {'c','y'}, /* Charybdis */
	/* 79*/ {'c','g'}, /* contra grashopper */
	/* 80*/ {'r','n'}, /* RoseLion */
	/* 81*/ {'r','p'}, /* RoseHopper */
	/* 82*/ {'o','k'}, /* Okapi */
	/* 83*/ {'3','7'}, /* 3:7-leaper */
	/* 84*/ {'g','2'}, /* grasshopper-2 */
	/* 85*/ {'g','3'}, /* grasshopper-3 */
	/* 86*/ {'1','6'}, /* 1:6-leaper */
	/* 87*/ {'2','4'}, /* 2:4-leaper */
	/* 88*/ {'3','5'}, /* 3:5-leaper */
	/* 89*/ {'d','g'}, /* Double Grasshopper */
	/* 90*/ {'k','h'}, /* Kinghopper */
	/* 91*/ {'o','r'}, /* Orix */
	/* 92*/ {'1','5'}, /* 1:5-leaper */
	/* 93*/ {'2','5'}, /* 2:5-leaper */
	/* 94*/ {'g','l'}, /* Gral */
        /* 95*/ {'l','r'}, /* rook locust */
        /* 96*/ {'l','b'}, /* bishop locust */
        /* 97*/ {'l','n'}, /* nightrider locust */
        /* 98*/ {'w','r'}, /* wazirrider */
        /* 99*/ {'f','r'}, /* fersrider */
	/*100*/ {'b','i'}, /* bison */
	/*101*/ {'e','t'}, /* elephant */
	/*102*/ {'n','a'}, /* Nao */
	/*103*/ {'r','m'}, /* rook moose: 45 degree grashopper */
	/*104*/ {'r','e'}, /* rook eagle: 90 degree grashopper */
	/*105*/ {'r','w'}, /* rook sparrow: 135 degree grashopper */
	/*106*/ {'b','m'}, /* bishop moose: 45 degree grashopper */
	/*107*/ {'b','e'}, /* bishop eagle: 90 degree grashopper */
	/*108*/ {'b','w'}, /* bishop sparrow: 135 degree grashopper */
	/*109*/ {'r','a'}, /* rao: chinese rose */
	/*110*/ {'s','o'}, /* scorpion: erlking + grashopper */
	/*111*/ {'m','g'}, /* marguerite */
	/*112*/ {'3','6'}, /* 3:6 leaper */
        /*113*/ {'n','l'}, /* nightriderlion */
        /*114*/ {'m','l'}, /* maoriderlion */
        /*115*/ {'m','m'}, /* moariderlion */
        /*116*/ {'a','g'}, /* AndernachGrasshopper */
        /*117*/ {'f',' '}, /* friend */
        /*118*/ {'d','o'}, /* dolphin */
        /*119*/ {'r','t'}, /* rabbit */
        /*120*/ {'b','o'}  /* bob */
	}
	};
#endif

#ifdef WE_ARE_EXTERN
	extern Lang     ActLang;
	extern char     ActAuthor[];
	extern char     ActOrigin[];
	extern char     ActTitle[];
	extern char     ActTwin[];
	extern char     ActAward[], ActStip[];          /* V3.46  TLi */
#else
	Lang            ActLang;
	char            ActAuthor[256];
	char            ActOrigin[256];
	char            ActTitle[256];
	char            ActTwin[1532];
	char            ActAward[256], ActStip[37];     /* V3.46  TLi */
#endif

#ifdef WE_ARE_EXTERN
	extern PieceChar *PieceTab;
#else
	PieceChar       *PieceTab=PieNamString[German];
#endif
				/* later set according to language */

#ifdef WE_ARE_EXTERN
	extern boolean  OptFlag[OptCount];
	extern char     *OptString[LangCount][OptCount];
#else
	boolean OptFlag[OptCount];
	char    *OptString[LangCount][OptCount] = {
	{
	/* Francais French Franzoesisch */
	/* 0*/  "Defense",
	/* 1*/  "Apparent",
	/* 2*/  "ApparentSeul",
	/* 3*/  "Variantes",
	/* 4*/  "Trace",
	/* 5*/  "Enroute",
	/* 6*/  "SansRB",
	/* 7*/  "Duplex",
	/* 8*/  "SansRN",
	/* 9*/  "Menace",
	/*10*/  "SansMenace",
	/*11*/  "Essais",
	/*12*/  "MaxSolutions",
	/*13*/  "MaxCasesFuites",
	/*14*/  "Search",
	/*15*/  "MultiSolutions",
	/*16*/  "NonSymetrique",
	/*17*/  "NonTrivial",
	/*18*/  "KeepMatingPiece",
	/*19*/  "EnPassant",
	/*20*/  "SansEchiquier",
	/*21*/  "SansVariantesCourtes",
	/*22*/  "DemiDuplex",
	/*23*/  "ApresCle",
	/*24*/  "Intelligent",
	/*25*/  "MaxTemps",
	/*26*/  "SansRoquer",
	/*27*/  "Quodlibet",
	/*28*/  "FinApresSolutionsCourtes"
	},{
	/* Deutsch German Allemand */
	/* 0*/  "Widerlegung",
	/* 1*/  "Satzspiel",
	/* 2*/  "WeissBeginnt",
	/* 3*/  "Varianten",
	/* 4*/  "Zugnummern",
	/* 5*/  "StartZugnummer",
	/* 6*/  "OhneWk",
	/* 7*/  "Duplex",
	/* 8*/  "OhneSk",
	/* 9*/  "Drohung",
	/*10*/  "OhneDrohung",
	/*11*/  "Verfuehrung",
	/*12*/  "MaxLoesungen",
	/*13*/  "MaxFluchtFelder",
	/*14*/  "Suchen",
	/*15*/  "MehrereLoesungen",
	/*16*/  "KeineSymmetrie",
	/*17*/  "NichtTrivial",
	/*18*/  "HalteMattsetzendenStein",
	/*19*/  "EnPassant",
	/*20*/  "OhneBrett",
	/*21*/  "OhneKurzVarianten",
	/*22*/  "HalbDuplex",
	/*23*/  "NachSchluessel",
	/*24*/  "Intelligent",
	/*25*/  "MaxZeit",
	/*26*/  "KeineRochade",
	/*27*/  "Quodlibet",
	/*28*/  "StopNachKurzloesungen"
	},{
	/* English Anglais Englisch */
	/* 0*/  "Defence",
	/* 1*/  "SetPlay",
	/* 2*/  "WhiteToPlay",
	/* 3*/  "Variations",
	/* 4*/  "MoveNumbers",
	/* 5*/  "StartMoveNumber",
	/* 6*/  "NoWk",
	/* 7*/  "Duplex",
	/* 8*/  "NoBk",
	/* 9*/  "Threat",
	/*10*/  "NoThreat",
	/*11*/  "Try",
	/*12*/  "MaxSolutions",
	/*13*/  "MaxFlightsquares",
	/*14*/  "Search",
	/*15*/  "MultiSolutions",
	/*16*/  "NoSymmetry",
	/*17*/  "NonTrivial",
	/*18*/  "KeepMatingPiece",
	/*19*/  "EnPassant",
	/*20*/  "NoBoard",
	/*21*/  "NoShortVariations",
	/*22*/  "HalfDuplex",
	/*23*/  "PostKeyPlay",
	/*24*/  "Intelligent",
	/*25*/  "MaxTime",
	/*26*/  "NoCastling",
	/*27*/  "Quodlibet",
	/*28*/  "StopOnShortSolutions"
	}
};
#endif

#ifdef WE_ARE_EXTERN
	extern boolean  CondFlag[CondCount];
	extern char     *CondString[LangCount][CondCount];
#else
	boolean CondFlag[CondCount];
	char    *CondString[LangCount][CondCount] = {
	{
	/* French Condition Names */
	/* 0*/  "RexInclusif",
	/* 1*/  "Circe",
	/* 2*/  "CirceMalefique",
	/* 3*/  "Madrasi",
	/* 4*/  "Volage",
	/* 5*/  "Hypervolage",
	/* 6*/  "Bichromatique",
	/* 7*/  "Monochromatique",
	/* 8*/  "Grille",
	/* 9*/  "Koeko",
	/*10*/  "NoirBordCoup",
	/*11*/  "BlancBordCoup",
	/*12*/  "Leofamily",
	/*13*/  "Chinoises",
	/*14*/  "Patrouille",
	/*15*/  "CirceEchange",
	/*16*/  "SansPrises",
	/*17*/  "Immun",
	/*18*/  "ImmunMalefique",
	/*19*/  "ContactGrille",
	/*20*/  "Imitator",             /* V2.4d  TM */
	/*21*/  "CavalierMajeur",       /* V2.60  NG */
	/*22*/  "Haan",                 /* V2.60  NG */
	/*23*/  "CirceCameleon",        /* V2.80c  TLi */
	/*24*/  "CirceCouscous",        /* V3.00  NG */
	/*25*/  "CirceEquipollents",    /* V2.80c  TLi */
	/*26*/  "FileCirce",            /* V2.80c  TLi */
	/*27*/  "NoirMaximum",          /* V2.90c  TLi */
	/*28*/  "NoirMinimum",          /* V2.90c  TLi */
	/*29*/  "BlancMaximum",         /* V2.90c  TLi */
	/*30*/  "BlancMinimum",         /* V2.90c  TLi */
	/*31*/  "CaseMagique",          /* V2.90c  TLi */
	/*32*/  "Sentinelles",          /* V2.90c  TLi */
	/*33*/  "Tibet",                /* V2.90c  TLi */
	/*34*/  "DoubleTibet",          /* V2.90c  TLi */
	/*35*/  "CirceDiagramme",       /* V2.90c  TLi */
	/*36*/  "Trou",                 /* V2.90  NG */
	/*37*/  "NoirPriseForce",       /* V3.0  TLi */
	/*38*/  "BlancPriseForce",      /* V3.0  TLi */
	/*39*/  "RoisReflecteurs",      /* V3.02  TLi */
	/*40*/  "RoisTransmutes",       /* V3.0  TLi */
	/*41*/  "NoirPoursuite",        /* V3.0  TLi */
	/*42*/  "BlancPoursuite",       /* V3.0  TLi */
	/*43*/  "Duelliste",            /* V3.0  TLi */
	/*44*/  "CirceParrain",         /* V3.02  TLi */
	/*45*/  "SansIProm",            /* V3.02  TLi */
	/*46*/  "CirceSymetrique",      /* V3.02  TLi */
	/*47*/  "EchecsVogtlaender",    /* V3.03  TLi, V3.54  NG */
	/*48*/  "EchecsEinstein",       /* V3.1  TLi */
	/*49*/  "Bicolores",            /* V3.1  TLi */
	/*50*/  "KoekoNouveaux",        /* V3.1  TLi */
	/*51*/  "CirceClone",           /* V3.1  TLi */
	/*52*/  "AntiCirce",
	/*53*/  "FileCirceMalefique",
	/*54*/  "CirceAntipoden",
	/*55*/  "CirceCloneMalefique",
	/*56*/  "AntiCirceMalefique",
	/*57*/  "AntiCirceDiagramme",
	/*58*/  "AntiFileCirce",
	/*59*/  "AntiCirceSymetrique",
	/*60*/  "AntiFileCirceMalefique",
	/*61*/  "AntiCirceAntipoden",
	/*62*/  "AntiCirceEquipollents",
	/*63*/  "ImmunFile",
	/*64*/  "ImmunDiagramme",
	/*65*/  "ImmunFileMalefique",
	/*66*/  "ImmunSymmetrique",
	/*67*/  "ImmunAntipoden",
	/*68*/  "ImmunEquipollents",            /* V3.1  TLi */
	/*69*/  "ReversEchecsEinstein",
	/*70*/  "SuperCirce",
	/*71*/  "Degradierung",
	/*72*/  "NorskSjakk",
	/*73*/  "exact",
	/*74*/  "EchecsTraitor",
	/*75*/  "EchecsAndernach",
	/*76*/  "BlancCaseForce",
	/*77*/  "BlancCaseForceConsequent",
	/*78*/  "NoirCaseForce",
	/*79*/  "NoirCaseForceConsequent",
	/*80*/  "ultra",                        /* V3.22  TLi */
	/*81*/  "EchecsChameleon",              /* V3.32  TLi */
	/*82*/  "EchecsFonctionnaire",          /* V3.32  TLi, V3.57  NG */
	/*83*/  "EchecsGlasgow",                /* V3.38  TLi */
	/*84*/  "EchecsAntiAndernach",          /* V3.41a TLi */
	/*85*/  "FrischAufCirce",               /* V3.41b TLi */
	/*86*/  "CirceMalefiqueVerticale",      /* V3.42  NG */
	/*87*/  "Isardam",                      /* V3.44  SE/TLi */
	/*88*/  "SansEchecc",                   /* V3.44  TLi */
	/*89*/  "CirceDiametrale",              /* V3.44  TLi */
	/*90*/  "PromSeul",                     /* V3.44  NG */
	/*91*/  "RankCirce",                    /* V3.45  TLi */
	/*92*/  "EchecsExclusif",               /* V3.45  TLi */
	/*93*/  "MarsCirce",                    /* V3.46  SE/TLi */
	/*94*/  "MarsCirceMalefique",           /* V3.46  TLi */
	/*95*/  "EchecsPhantom",                /* V3.47  NG */
	/*96*/  "BlancRoiReflecteur",           /* V3.47  NG */
	/*97*/  "NoirRoiReflecteur",            /* V3.47  NG */
	/*98*/  "BlancRoiTransmute",            /* V3.47  NG */
	/*99*/  "NoirRoiTransmute",             /* V3.47  NG */
	/*100*/ "EchecsAntiEinstein",           /* V3.50  TLi */
	/*101*/ "CirceCouscousMalefique",       /* V3.50  TLi */
	/*102*/ "NoirCaseRoyal",                /* V3.50  TLi */
	/*103*/ "BlancCaseRoyal",               /* V3.50  TLi */
	/*104*/ "EchecsBrunner",                /* V3.50  SE */
	/*105*/ "EchecsPlus",                   /* V3.50  SE */
	/*106*/ "CirceAssassin",                /* V3.50  SE */
	/*107*/ "EchecsPatience",		/* V3.50  SE */
	/*108*/ "EchecsRepublicains",		/* V3.50  SE */
	/*109*/ "EchecsExtinction",		/* V3.50  SE */
	/*110*/ "EchecsCentral",                /* V3.50  SE */
	/*111*/ "ActuatedRevolvingBoard",       /* V3.53  TLi */
	/*112*/ "EchecsMessigny",               /* V3.55  TLi */
	/*113*/ "Woozles",                      /* V3.55  TLi */
	/*114*/ "BiWoozles",                    /* V3.55  TLi */
	/*115*/ "Heffalumps",                   /* V3.55  TLi */
	/*116*/ "BiHeffalumps",                 /* V3.55  TLi */
	/*117*/ "RexExclusif",                  /* V3.55  TLi */
	/*118*/ "BlancCasePromotion",           /* V3.57  NG */
	/*119*/ "NoirCasePromotion",            /* V3.57  NG */
	/*120*/ "SansBlancPromotion",           /* V3.57  NG */
	/*121*/ "SansNoirPromotion",            /* V3.57  NG */
	/*122*/ "EchecsEiffel",                 /* V3.60  TLi */
	/*123*/ "NoirUltraSchachZwang",		/* V3.62  SE */
	/*124*/ "BlancUltraSchachZwang",	/* V3.62  SE */
	/*125*/ "ActuatedRevolvingCentre",      /* V3.62  SE */
	/*126*/ "ShieldedKings",       		/* V3.62  SE */
	/*127*/ "EchecsSting",      		/* V3.63  NG */
	/*128*/ "EchecsChameleonLigne",         /* V3.64  TLi */
	/*129*/ "BlancSansPrises",              /* V3.64  NG */
	/*130*/ "NoirSansPrises",               /* V3.64  NG */
	/*131*/ "EchecsAvril",                  /* V3.64  ThM */
	/*132*/ "EchecsAlphabetiques",          /* V3.64  SE */
	/*133*/ "CirceTurncoats",               /* V3.70  SE */
	/*134*/ "CirceDoubleAgents",            /* V3.70  SE */
	/*135*/ "AMU",                          /* V3.70  SE */
        /*136*/ "SingleBox"                     /* V3.71  ThM, NG */
	},{
	/* German Condition Names */
	/* 0*/  "RexInklusive",
	/* 1*/  "Circe",
	/* 2*/  "SpiegelCirce",
	/* 3*/  "Madrasi",
	/* 4*/  "Volage",
	/* 5*/  "Hypervolage",
	/* 6*/  "BichromesSchach",
	/* 7*/  "MonochromesSchach",
	/* 8*/  "Gitterschach",
	/* 9*/  "KoeKo",
	/*10*/  "SchwarzerRandzueger",
	/*11*/  "WeisserRandzueger",
	/*12*/  "Leofamily",
	/*13*/  "ChinesischesSchach",
	/*14*/  "PatrouilleSchach",
	/*15*/  "PlatzwechselCirce",
	/*16*/  "Ohneschlag",
	/*17*/  "Immunschach",
	/*18*/  "SpiegelImmunschach",
	/*19*/  "KontaktGitter",
	/*20*/  "Imitator",                     /* V2.4d  TM */
	/*21*/  "CavalierMajeur",               /* V2.60  NG */
	/*22*/  "HaanerSchach",                 /* V2.60  NG */
	/*23*/  "ChamaeleonCirce",              /* V2.80c  TLi */
	/*24*/  "CouscousCirce",                /* V3.00  NG */
	/*25*/  "EquipollentsCirce",            /* V2.80c  TLi */
	/*26*/  "FileCirce",                    /* V2.80c  TLi */
	/*27*/  "SchwarzerLaengstzueger",       /* V2.90c  TLi */
	/*28*/  "SchwarzerKuerzestzueger",      /* V2.90c  TLi */
	/*29*/  "WeisserLaengstzueger",         /* V2.90c  TLi */
	/*30*/  "WeisserKuerzestzueger",        /* V2.90c  TLi */
	/*31*/  "MagischeFelder",               /* V2.90c  TLi */
	/*32*/  "Sentinelles",                  /* V2.90c  TLi */
	/*33*/  "TibetSchach",                  /* V2.90c  TLi */
	/*34*/  "DoppeltibetSchach",            /* V2.90c  TLi */
	/*35*/  "DiagrammCirce",                /* V2.90c  TLi */
	/*36*/  "Loch",                         /* V2.90  NG */
	/*37*/  "SchwarzerSchlagzwang",         /* V3.0  TLi */
	/*38*/  "WeisserSchlagzwang",           /* V3.0  TLi */
	/*39*/  "ReflektierendeKoenige",        /* V3.0  TLi */
	/*40*/  "TransmutierendeKoenige",       /* V3.02 TLi */
	/*41*/  "SchwarzerVerfolgungszueger",   /* V3.0  TLi */
	/*42*/  "WeisserVerfolgungszueger",     /* V3.0  TLi */
	/*43*/  "Duellantenschach",             /* V3.0  TLi */
	/*44*/  "CirceParrain",                 /* V3.02  TLi */
	/*45*/  "OhneIUW",                      /* V3.02  TLi */
	/*46*/  "SymmetrieCirce",               /* V3.02  TLi */
	/*47*/  "VogtlaenderSchach",            /* V3.03  TLi */
	/*48*/  "EinsteinSchach",               /* V3.1  TLi */
	/*49*/  "Bicolores",                    /* V3.1  TLi */
	/*50*/  "NeuKoeko",             /* V3.1  TLi */
	/*51*/  "CirceClone",           /* V3.1  TLi */
	/*52*/  "AntiCirce",            /* V3.1  TLi */
	/*53*/  "SpiegelFileCirce",
	/*54*/  "AntipodenCirce",
	/*55*/  "SpiegelCirceclone",
	/*56*/  "AntiSpiegelCirce",
	/*57*/  "AntiDiagrammCirce",
	/*58*/  "AntiFileCirce",
	/*59*/  "AntiSymmetrieCirce",
	/*60*/  "AntiSpiegelFileCirce",
	/*61*/  "AntiAntipodenCirce",
	/*62*/  "AntiEquipollentsCirce",
	/*63*/  "FileImmunSchach",
	/*64*/  "DiagrammImmunSchach",
	/*65*/  "SpiegelFileImmunSchach",
	/*66*/  "SymmetrieImmunSchach",
	/*67*/  "AntipodenImmunSchach",
	/*68*/  "EquipollentsImmunSchach",
	/*69*/  "ReversesEinsteinSchach",
	/*70*/  "SuperCirce",
	/*71*/  "Degradierung",
	/*72*/  "NorskSjakk",
	/*73*/  "exakt",
	/*74*/  "TraitorChess",
	/*75*/  "AndernachSchach",
	/*76*/  "WeissesZwangsfeld",
	/*77*/  "WeissesKonsequentesZwangsfeld",
	/*78*/  "SchwarzesZwangsfeld",
	/*79*/  "SchwarzesKonsequentesZwangsfeld",
	/*80*/  "Ultra",
	/*81*/  "ChamaeleonSchach",        /* V3.32  TLi */
	/*82*/  "BeamtenSchach",           /* V3.32  TLi */
	/*83*/  "GlasgowSchach",           /* V3.38  TLi */
	/*84*/  "AntiAndernachSchach",          /* V3.41a TLi */
	/*85*/  "FrischAufCirce",          /* V3.41b TLi */
	/*86*/  "VertikalesSpiegelCirce",       /* V3.42 NG */
	/*87*/  "Isardam",                      /* V3.44  SE/TLi */
	/*88*/  "OhneSchach",                   /* V3.44  TLi */
	/*89*/  "DiametralCirce",               /* V3.44  TLi */
	/*90*/  "UWnur",                        /* V3.44  NG */
	/*91*/  "RankCirce",                    /* V3.45  TLi */
	/*92*/  "ExklusivSchach",               /* V3.45  TLi */
	/*93*/  "MarsCirce",                    /* V3.46  SE/TLi */
	/*94*/  "MarsSpiegelCirce",             /* V3.46  TLi */
	/*95*/  "PhantomSchach",                /* V3.47  NG */
	/*96*/  "WeisserReflektierenderKoenig",    /* V3.47  NG */
	/*97*/  "SchwarzerReflektierenderKoenig",  /* V3.47  NG */
	/*98*/  "WeisserTransmutierenderKoenig",   /* V3.47  NG */
	/*99*/  "SchwarzerTransmutierenderKoenig", /* V3.47  NG */
	/*100*/ "AntiEinsteinSchach",           /* V3.50  TLi */
	/*101*/ "SpiegelCouscousCirce",         /* V3.50  TLi */
	/*102*/ "SchwarzesKoeniglichesFeld",    /* V3.50  TLi */
	/*103*/ "WeissesKoeniglichesFeld",      /* V3.50  TLi */
	/*104*/ "BrunnerSchach",                /* V3.50  SE */
	/*105*/ "PlusSchach",                   /* V3.50  SE */
	/*106*/ "AssassinCirce",                /* V3.50  SE */
	/*107*/ "PatienceSchach",		/* V3.50  SE */
	/*108*/ "RepublikanerSchach",           /* V3.50  SE */
	/*109*/ "AusrottungsSchach",            /* V3.50  SE */
	/*110*/ "ZentralSchach",                /* V3.50  SE */
	/*111*/ "ActuatedRevolvingBoard",       /* V3.53  TLi */
	/*112*/ "MessignySchach",               /* V3.55  TLi */
	/*113*/ "Woozles",                      /* V3.55  TLi */
	/*114*/ "BiWoozles",                    /* V3.55  TLi */
	/*115*/ "Heffalumps",                   /* V3.55  TLi */
	/*116*/ "BiHeffalumps",                 /* V3.55  TLi */
	/*117*/ "RexExklusive",                 /* V3.55  TLi */
	/*118*/ "UWFeldWeiss",                  /* V3.57  NG */
	/*119*/ "UWFeldSchwarz",                /* V3.57  NG */
	/*120*/ "OhneWeissUW",                  /* V3.57  NG */
	/*121*/ "OhneSchwarzUW",                /* V3.57  NG */
	/*122*/ "EiffelSchach",                 /* V3.60  TLi */
	/*123*/ "SchwarzerUltraSchachZwang",	/* V3.62  SE */
	/*124*/ "WeisserUltraSchachZwang",	/* V3.62  SE */
	/*125*/ "ActuatedRevolvingCentre",      /* V3.62  SE */
	/*126*/ "SchutzKoenige",       		/* V3.62  SE */
	/*127*/ "StingSchach",                  /* V3.63  NG */
	/*128*/ "LinienChamaeleonSchach",      	/* V3.64  TLi */
	/*129*/ "WeisserOhneSchlag",      	/* V3.64  NG */
	/*130*/ "SchwarzerOhneSchlag",      	/* V3.64  NG */
	/*131*/ "Aprilschach",                  /* V3.64  ThM */
	/*132*/ "AlphabetischesSchach",         /* V3.64  SE */
	/*133*/ "TurncoatCirce",                /* V3.70  SE */
	/*134*/ "DoppelAgentenCirce",           /* V3.70  SE */
	/*135*/ "AMU",                          /* V3.70  SE */
        /*136*/ "NurPartiesatzSteine"           /* V3.71  ThM, NG */
	},{
	/* English Condition Names */
	/* 0*/  "RexInclusiv",
	/* 1*/  "Circe",
	/* 2*/  "MirrorCirce",
	/* 3*/  "Madrasi",
	/* 4*/  "Volage",
	/* 5*/  "Hypervolage",
	/* 6*/  "BichromChess",
	/* 7*/  "MonochromChess",
	/* 8*/  "GridChess",
	/* 9*/  "KoeKo",
	/*10*/  "BlackEdgeMover",
	/*11*/  "WhiteEdgeMover",
	/*12*/  "Leofamily",
	/*13*/  "ChineseChess",
	/*14*/  "Patrouille",
	/*15*/  "PWC",
	/*16*/  "NoCapture",
	/*17*/  "ImmunChess",
	/*18*/  "MirrorImmunChess",
	/*19*/  "ContactGridChess",
	/*20*/  "Imitator",             /* V2.4d  TM */
	/*21*/  "CavalierMajeur",       /* V2.60  NG */
	/*22*/  "HaanerChess",          /* V2.60  NG */
	/*23*/  "ChameleonCirce",       /* V2.80c  TLi */
	/*24*/  "CouscousCirce",        /* V3.00  NG */
	/*25*/  "EquipollentsCirce",    /* V2.80c  TLi */
	/*26*/  "FileCirce",            /* V2.80c  TLi */
	/*27*/  "BlackMaximummer",      /* V2.90c  TLi */
	/*28*/  "BlackMinimummer",      /* V2.90c  TLi */
	/*29*/  "WhiteMaximummer",      /* V2.90c  TLi */
	/*30*/  "WhiteMinimummer",      /* V2.90c  TLi */
	/*31*/  "MagicSquares",         /* V2.90c  TLi */
	/*32*/  "Sentinelles",          /* V2.90c  TLi */
	/*33*/  "Tibet",                /* V2.90c  TLi */
	/*34*/  "DoubleTibet",          /* V2.90c  TLi */
	/*35*/  "DiagramCirce",         /* V2.90c  TLi */
	/*36*/  "Hole",                 /* V2.90  NG */
	/*37*/  "BlackMustCapture",     /* V3.0  TLi */
	/*38*/  "WhiteMustCapture",     /* V3.0  TLi */
	/*39*/  "ReflectiveKings",      /* V3.0  TLi */
	/*40*/  "TransmutedKings",      /* V3.02 TLi */
	/*41*/  "BlackFollowMyLeader",  /* V3.0  TLi */
	/*42*/  "WhiteFollowMyLeader",  /* V3.0  TLi */
	/*43*/  "DuellistChess",        /* V3.0  TLi */
	/*44*/  "ParrainCirce",         /* V3.02  TLi */
	/*45*/  "NoIProm",              /* V3.02  TLi */
	/*46*/  "SymmetryCirce",        /* V3.02  TLi */
	/*47*/  "VogtlaenderChess",     /* V3.03  TLi */
	/*48*/  "EinsteinChess",        /* V3.1  TLi */
	/*49*/  "Bicolores",            /* V3.1  TLi */
	/*50*/  "NewKoeko",             /* V3.1  TLi */
	/*51*/  "CirceClone",           /* V3.1  TLi */
	/*52*/  "AntiCirce",            /* V3.1  TLi */
	/*53*/  "MirrorFileCirce",
	/*54*/  "AntipodeanCirce",
	/*55*/  "MirrorCirceClone",
	/*56*/  "MirrorAntiCirce",
	/*57*/  "DiagramAntiCirce",
	/*58*/  "FileAntiCirce",
	/*59*/  "SymmetryAntiCirce",
	/*60*/  "MirrorAntiCirce",
	/*61*/  "AntipodeanAntiCirce",
	/*62*/  "EquipollentsAntiCirce",
	/*63*/  "FileImmunChess",
	/*64*/  "DiagramImmunChess",
	/*65*/  "MirrorFileImmunChess",
	/*66*/  "SymmetryImmunChess",
	/*67*/  "AntipodeanImmunChess",
	/*68*/  "EquipollentsImmunChess",
	/*69*/  "ReversalEinsteinChess",
	/*70*/  "SuperCirce",
	/*71*/  "RelegationChess",
	/*72*/  "NorskSjakk",
	/*73*/  "exact",
	/*74*/  "TraitorChess",
	/*75*/  "AndernachChess",
	/*76*/  "WhiteForcedSquare",
	/*77*/  "WhiteConsequentForcedSquare",
	/*78*/  "BlackForcedSquare",
	/*79*/  "BlackConsequentForcedSquare",
	/*80*/  "ultra",
	/*81*/  "ChameleonChess",    /* V3.32  TLi */
	/*82*/  "FunctionaryChess",  /* V3.32  TLi, V3.57  NG */
	/*83*/  "GlasgowChess",      /* V3.38  TLi */
	/*84*/  "AntiAndernachChess",  /* V3.41a TLi */
	/*85*/  "FrischAufCirce",      /* V3.41b TLi */
	/*86*/  "VerticalMirrorCirce",  /* V3.42 NG */
	/*87*/  "Isardam",             /* V3.44  SE/TLi */
	/*88*/  "ChecklessChess",      /* V3.44  TLi */
	/*89*/  "DiametralCirce",       /* V3.44  TLi */
	/*90*/  "PromOnly",             /* V3.44  NG */
	/*91*/  "RankCirce",            /* V3.45  TLi */
	/*92*/  "ExclusivChess",        /* V3.45  TLi */
	/*93*/  "MarsCirce",            /* V3.46  SE/TLi */
	/*94*/  "MarsMirrorCirce",      /* V3.46  TLi */
	/*95*/  "PhantomChess",         /* V3.47  NG */
	/*96*/  "WhiteReflectiveKing",  /* V3.47  NG */
	/*97*/  "BlackReflectiveKing",  /* V3.47  NG */
	/*98*/  "WhiteTransmutedKing",  /* V3.47  NG */
	/*99*/  "BlackTransmutedKing",  /* V3.47  NG */
	/*100*/ "AntiEinsteinChess",    /* V3.50  TLi */
	/*101*/ "MirrorCouscousCirce",  /* V3.50  TLi */
	/*102*/ "BlackRoyalSquare",     /* V3.50  TLi */
	/*103*/ "WhiteRoyalSquare",     /* V3.50  TLi */
	/*104*/ "BrunnerChess",         /* V3.50  SE */
	/*105*/ "PlusChess",            /* V3.50  SE */
	/*106*/ "CirceAssassin",        /* V3.50  SE */
	/*107*/ "PatienceChess",	/* V3.50  SE */
	/*108*/ "RepublicanChess",      /* V3.50  SE */
	/*109*/ "ExtinctionChess",      /* V3.50  SE */
	/*110*/ "CentralChess",         /* V3.50  SE */
	/*111*/ "ActuatedRevolvingBoard",/* V3.53  TLi */
	/*112*/ "MessignyChess",        /* V3.55  TLi */
	/*113*/ "Woozles",              /* V3.55  TLi */
	/*114*/ "BiWoozles",            /* V3.55  TLi */
	/*115*/ "Heffalumps",           /* V3.55  TLi */
	/*116*/ "BiHeffalumps",         /* V3.55  TLi */
	/*117*/ "RexExclusiv",          /* V3.55  TLi */
	/*118*/ "WhitePromSquares",     /* V3.57  NG */
	/*119*/ "BlackPromSquares",     /* V3.57  NG */
	/*120*/ "NoWhitePromotion",     /* V3.57  NG */
	/*121*/ "NoBlackPromotion",     /* V3.57  NG */
	/*122*/ "EiffelChess",           /* V3.60  TLi */
	/*123*/ "BlackUltraSchachZwang",	/* V3.62  SE */
	/*124*/ "WhiteUltraSchachZwang",	/* V3.62  SE */
	/*125*/ "ActuatedRevolvingCentre",      /* V3.62  SE */
	/*126*/ "ShieldedKings",      		/* V3.62  SE */
	/*127*/ "StingChess",      		/* V3.63  NG */
	/*128*/ "LineChameleonChess",       	/* V3.64  TLi */
	/*129*/ "NoWhiteCapture",       	/* V3.64  TLi */
	/*130*/ "NoBlackCapture",      		/* V3.64  TLi */
	/*131*/ "AprilChess",                   /* V3.64  ThM */
	/*132*/ "AlphabeticChess",              /* V3.64  SE */
	/*133*/ "CirceTurncoats",               /* V3.70  SE */
	/*134*/ "CirceDoubleAgents",            /* V3.70  SE */
	/*135*/ "AMU",                          /* V3.70  SE */
	/*136*/ "SingleBox"                     /* V3.70  ThM, NG */
	}
	};
#endif

/* The notation of stipulations is international, therefore we need
** not provide one for every language
*/
EXTERN unsigned int StipFlags;

#ifdef WE_ARE_EXTERN
	extern  char    *PieSpString[LangCount][PieSpCount];
	extern  char    **PieSpTab;
	extern  Flags   PieSpExFlags;
#else
	Flags   PieSpExFlags;   /* used for problem-wide piecespecification */
	char    **PieSpTab;
	char    *PieSpString[LangCount][PieSpCount] = {
	{
/* French */
	"Blanc",
	"Noir",
	"Neutre",
	"Kamikaze",
	"Royale",
	"Paralysante",
	"Chameleon",
	"Jigger",
	"Volage",
        "Fonctionnaire",
	"DemiNeutre",
	"CouleurEchangeantSautoir"
	},{
/* German */
	"Weiss",
	"Schwarz",
	"Neutral",
	"Kamikaze",
	"Koeniglich",
	"Paralysierend",
	"Chamaeleon",
	"Jigger",
	"Volage",
        "Beamtet",
	"HalbNeutral",
	"SprungbockFarbeWechselnd"
	},{
/* English */
	"White",
	"Black",
	"Neutral",
	"Kamikaze",
	"Royal",
	"Paralysing",
	"Chameleon",
	"Jigger",
	"Volage",
        "Functionary",
	"HalfNeutral",
	"HurdleColourChanging"
	}
	};
#endif

#ifdef WE_ARE_EXTERN
	extern  char GlobalStr[];
	/* extern  char AlphaEnd[];  V3.31  TLi */
	extern  char *AlphaEnd;    /* V3.31  TLi */
	extern  char ReciAlphaEnd[];  /* V3.31  TLi */
	extern  char NonReciAlphaEnd[];
#else
	/* char    GlobalStr[256]; */
	char    GlobalStr[4*maxply];		/* V3.70  TLi */
	/* char    AlphaEnd[4];   V3.31  TLi */
	char    *AlphaEnd;   /* V3.31  TLi */
	char    ReciAlphaEnd[5];  /* V3.31  TLi */ /* V3.37  NG */
	char    NonReciAlphaEnd[5]; /* V3.31  TLi */ /* V3.37  NG */
#endif

/* V3.50 SE */
#ifdef WE_ARE_EXTERN
	extern char
*VariantTypeString[LangCount][VariantTypeCount];
#else
	char    *VariantTypeString[LangCount][VariantTypeCount] = {
	{
	/* French */
	/* 0*/  "TypeB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "NoirMaximum",
	/* 6*/  "BlancMaximum",
        /* 7*/  "ParaSentinelles",
        /* 8*/  "TotalMaximum",
	/* 9*/  "Berolina",
	/*10*/  "Type1",
	/*11*/  "Type2",
	/*12*/  "Type3"
	},{
	/* German */
	/* 0*/  "TypB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "SchwarzMaximum",
	/* 6*/  "WeissMaximum",
        /* 7*/  "ParaSentinelles",
        /* 8*/  "TotalMaximum",
	/* 9*/  "Berolina",
	/*10*/  "Typ1",
	/*11*/  "Typ2",
	/*12*/  "Typ3"
	},{
	/* English */
	/* 0*/  "TypeB",
	/* 1*/  "PionAdvers",
	/* 2*/  "Cheylan",
	/* 3*/  "Calvet",
	/* 4*/  "PionNeutre",
	/* 5*/  "BlackMaximum",
	/* 6*/  "WhiteMaximum",
        /* 7*/  "ParaSentinelles",
        /* 8*/  "TotalMaximum",
	/* 9*/  "Berolina",
	/*10*/  "Type1",
	/*11*/  "Type2",
	/*12*/  "Type3"
	}
	};
#endif

/* V3.62 SE */
#ifdef WE_ARE_EXTERN
	extern char
*ExtraCondString[LangCount][ExtraCondCount];
#else
	char    *ExtraCondString[LangCount][ExtraCondCount] = {
	{
	/* French */
	/* 0*/  "Maximum",
	/* 1*/  "UltraSchachZwang"
	},{
	/* German */
	/* 0*/  "Laengstzueger",
	/* 1*/  "UltraSchachZwang"
	},{
	/* English */
	/* 0*/  "Maximummer",
	/* 1*/  "UltraSchachZwang"
	}
	};
#endif

#ifdef WE_ARE_EXTERN
	extern  numvec mixhopdata[4][17];
#else
/* This are the vectors for hoppers which change directions after the hurdle */  /* V2.60  NG */
/* and for the CAT (a special concatenation of knight and dabbabba-rider     */  /* V2.60  NG */
	numvec mixhopdata[4][17] = {
	/* moose     45 */ { 0, -23,  25,  23,  25,  23, -25, -23, -25,
				 -1,  24,   1,  24,   1, -24,  -1, -24 },
	/* eagle     90 */ { 0,  24, -24,   1,  -1,  24, -24,   1,  -1,
				 25, -25,  23, -23,  25, -25,  23, -23 },
	/* sparrow  135 */ { 0,  23, -25, -23, -25, -23,  25,  23,  25,
				  1, -24,  -1, -24,  -1,  24,   1,  24 },
	/* CAT          */ { 0, -22,  47,  22, -47,  26,  49, -26, -49,
				 -2,  48,  48,   2,   2, -48, -48,  -2 }
	};
#endif

#ifdef WE_ARE_EXTERN
	extern  numvec vec[maxvec + 1];
#else
/* don't try to delete something like "duplicates" or change
  the order of the vectors.
  whey are all necessary and need this order !!
*/   							/* V2.60  NG */
numvec vec[maxvec + 1] = { 0,
/*   1 -   4 | 0,1 */    1,   24,   -1,  -24,
/*   5 -   8 | 1,1 */   23,   25,  -23,  -25,
/*   9 -  16 | 1,2 */   22,   47,   49,   26,  -22,  -47,  -49,  -26,
/*  17 -  24 | 1,2 */   22,   47,   49,   26,  -22,  -47,  -49,  -26,
/*  25 -  32 | 2,3 */   45,   70,   74,   51,  -45,  -70,  -74,  -51,
/*  33 -  40 | 1,3 */   21,   71,   73,   27,  -21,  -71,  -73,  -27,
/*  41 -  48 | 1,4 */   20,   95,   97,   28,  -20,  -95,  -97,  -28,
/*  49 -  56 | 3,4 */   68,   93,   99,   76,  -68,  -93,  -99,  -76,
/*  57 -  60 | 0,5 */    5,  120,   -5, -120,
/*  61 -  64 | 0,2 */    2,   48,   -2,  -48,
/*  65 -  68 | 2,2 */   46,   50,  -46,  -50,
/*  69 -  76 | 1,7 */   17,  167,  169,   31,  -17, -167, -169,  -31,
/*  77 -  80 | 5,5 */  115,  125, -115, -125,
/*  81 -  88 | 3,7 */   65,  165,  171,   79,  -65, -165, -171,  -79,
/*  89 -  96 | 1,6 */   18,  143,  145,   30,  -18, -143, -145,  -30,
/*  97 - 104 | 2,4 */   44,   94,   98,   52,  -44,  -94,  -98,  -52,
/* 105 - 112 | 3,5 */   67,  117,  123,   77,  -67, -117, -123,  -77,
/* 113 - 120 | 1,5 */   19,  119,  121,   29,  -19, -119, -121,  -29,
/* 121 - 128 | 2,5 */   43,  118,  122,   53,  -43, -118, -122,  -53,
/* 129 - 136 | 3,6 */   66,  141,  147,   78,  -66, -141, -147,  -78,
};
#endif


#ifdef WE_ARE_EXTERN
	extern  square boardnum[65];
#else
/* This are the really used boardnumbers */     /* V2.60  NG */
	square boardnum[65] = {
	/* first   rank */      bas, 201, 202, 203, 204, 205, 206, 207,
	/* second  rank */      224, 225, 226, 227, 228, 229, 230, 231,
	/* third   rank */      248, 249, 250, 251, 252, 253, 254, 255,
	/* fourth  rank */      272, 273, 274, 275, 276, 277, 278, 279,
	/* fifth   rank */      296, 297, 298, 299, 300, 301, 302, 303,
	/* sixth   rank */      320, 321, 322, 323, 324, 325, 326, 327,
	/* seventh rank */      344, 345, 346, 347, 348, 349, 350, 351,
	/* eigth   rank */      368, 369, 370, 371, 372, 373, 374, haut,
	/* end marker   */    0};
#endif


#ifdef WE_ARE_EXTERN
	extern  short move_diff_code[haut - bas + 1];
#else
/* This are the codes for the length-difference */      /* V2.90  NG */
/* between two squares */
/* ATTENTION: use abs(square from - square to) for indexing this table. */
/*        all move_down_codes are mapped this way to move_up_codes !    */

	short move_diff_code[haut - bas + 1]= {
	/* left/right   */        0,   1,   4,   9,  16,  25,  36,  49,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 1 left  up   */            50,  37,  26,  17,  10,   5,   2,
	/* 1 right up   */        1,   2,   5,  10,  17,  26,  37,  50,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 2 left  up   */            53,  40,  29,  20,  13,   8,   5,
	/* 2 right up   */        4,   5,   8,  13,  20,  29,  40,  53,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 3 left  up   */            58,  45,  34,  25,  18,  13,  10,
	/* 3 right up   */        9,  10,  13,  18,  25,  34,  45,  58,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 4 left  up   */            65,  52,  41,  32,  25,  20,  17,
	/* 4 right up   */       16,  17,  20,  25,  32,  41,  52,  65,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 5 left  up   */            74,  61,  50,  41,  34,  29,  26,
	/* 5 right up   */       25,  26,  29,  34,  41,  50,  61,  74,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 6 left  up   */            85,  72,  61,  52,  45,  40,  37,
	/* 6 right up   */       36,  37,  40,  45,  52,  61,  72,  85,
	/* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
	/* 7 left  up   */            98,  85,  74,  65,  58,  53,  50,
	/* 7 right up   */       49,  50,  53,  58,  65,  74,  85,  98
			       };
#endif

#ifdef WE_ARE_EXTERN
	extern  boolean  (* checkfunctions[derbla + 1])(square,piece,boolean (*) (square,square,square) );
#else
/* This are the used checkingfunctions  */     /* V2.60  NG */
	boolean  (* checkfunctions[derbla + 1])(square,piece,boolean (*) (square,square,square) ) = {
/*  0 */        (boolean (*)(square,piece,boolean (*)(square,square,square)))0, /* not used */
/*  1 */        (boolean (*)(square,piece,boolean (*)(square,square,square)))0, /* not used */
/*  2 */        roicheck,
/*  3 */        pioncheck,
/*  4 */        damecheck,
/*  5 */        cavcheck,
/*  6 */        tourcheck,
/*  7 */        foucheck,
/*  8 */        leocheck,
/*  9 */        maocheck,
/* 10 */        paocheck,
/* 11 */        vaocheck,
/* 12 */        rosecheck,
/* 13 */        scheck,
/* 14 */        ncheck,
/* 15 */        zcheck,
/* 16 */        chcheck,
/* 17 */        gicheck,
/* 18 */        rccinqcheck,
/* 19 */        bucheck,
/* 20 */        vizircheck,
/* 21 */        alfilcheck,
/* 22 */        ferscheck,
/* 23 */        dabcheck,
/* 24 */        lioncheck,
/* 25 */        nequicheck,
/* 26 */        loccheck,
/* 27 */        pbcheck,
/* 28 */        amazcheck,
/* 29 */        impcheck,
/* 30 */        princcheck,
/* 31 */        gnoucheck,
/* 32 */        antilcheck,
/* 33 */        ecurcheck,
/* 34 */        warancheck,
/* 35 */        dragoncheck,
/* 36 */        kangoucheck,
/* 37 */        cscheck,
/* 38 */        ubicheck,
/* 39 */        (boolean (*) (square,piece,boolean (*) (square,square,square) ) )0,     /* hamster cannot check */
/* 40 */        moosecheck,
/* 41 */        eaglecheck,
/* 42 */        sparrcheck,
/* 43 */        archcheck,
/* 44 */        reffoucheck,
/* 45 */        cardcheck,
/* 46 */        nsautcheck,
/* 47 */        (boolean (*) (square,piece,boolean (*) (square,square,square) ) )0,     /* dummy cannot check */
/* 48 */        camridcheck,
/* 49 */        zebridcheck,
/* 50 */        gnuridcheck,
/* 51 */        camhopcheck,
/* 52 */        zebhopcheck,
/* 53 */        gnuhopcheck,
/* 54 */        dcscheck,
/* 55 */        refccheck,
/* 56 */        equicheck,
/* 57 */        catcheck,
/* 58 */        bspawncheck,
/* 59 */        spawncheck,
/* 60 */        t_lioncheck,
/* 61 */        f_lioncheck,
/* 62 */        loccheck,               /* sirene checks like locust */
/* 63 */        tritoncheck,
/* 64 */        nereidecheck,
/* 65 */        orphancheck,
/* 66 */        edgehcheck,
/* 67 */        moacheck,
/* 68 */        rhuntcheck,
/* 69 */        bhuntcheck,
/* 70 */        maoridercheck,
/* 71 */        moaridercheck,
/* 72 */        r_hopcheck,
/* 73 */        b_hopcheck,
/* 74 */        roicheck,               /* erlking checks like king */
/* 75 */        bscoutcheck,
/* 76 */        gscoutcheck,
/* 77 */        skyllacheck,
/* 78 */        charybdischeck,
/* 79 */        contragrascheck,
/* 80 */        roselioncheck,
/* 81 */        rosehoppercheck,
/* 82 */        okapicheck,             /* V3.32  TLi */
/* 83 */        leap37check,            /* V3.34  TLi */
/* 84 */        grasshop2check,         /* V3.34  TLi */
/* 85 */        grasshop3check,         /* V3.34  TLi */
/* 86 */        leap16check,            /* V3.38  NG */
/* 87 */        leap24check,            /* V3.42  TLi */
/* 88 */        leap35check,            /* V3.42  TLi */
/* 89 */        doublegrascheck,        /* V3.44  SE */
/* 90 */        kinghopcheck,           /* V3.44  SE */
/* 91 */        orixcheck,              /* V3.44  NG */
/* 92 */        leap15check,            /* V3.46  NG */
/* 93 */        leap25check,            /* V3.46  NG */
/* 94 */        gralcheck,              /* V3.46  NG */
/* 95 */        tritoncheck,            /* V3.53  TLi */
/* 96 */        nereidecheck,           /* V3.53  TLi */
/* 97 */        nightlocustcheck,       /* V3.53  TLi */
/* 98 */        tourcheck,              /* wazirrider checks like rook */
/* 99 */        foucheck,               /* fersrider checks like bishop */
/*100 */        bisoncheck,             /* V3.60  TLi */
/*101 */        elephantcheck,          /* V3.62  TLi */
/*102 */        naocheck,               /* V3.62  TLi */
/*103 */        rookmoosecheck,		/* V3.62  NG */
/*104 */        rookeaglecheck,		/* V3.62  NG */
/*105 */        rooksparrcheck,		/* V3.62  NG */
/*106 */        bishopmoosecheck,	/* V3.62  NG */
/*107 */        bishopeaglecheck,	/* V3.62  NG */
/*108 */        bishopsparrcheck,       /* V3.62  NG */
/*109 */        roselioncheck,  	/* rao checks like roselion */ /* V3.63  NG */
/*110 */        scorpioncheck,  	/* rao checks like roselion */ /* V3.63  NG */
/*111 */        margueritecheck,  	/* V3.64  TLi */
/*112 */        leap36check,    	/* V3.64  TLi */
/*113 */        nightriderlioncheck,    /* V3.64  TLi */
/*114 */        maoriderlioncheck,      /* V3.64  TLi */
/*115 */        moariderlioncheck,      /* V3.65  TLi */
/*116 */        scheck,                 /* V3.65  TLi */
/*117 */        friendcheck,            /* V3.65  TLi */
/*118 */        dolphincheck,           /* V3.70  TLi */
/*119 */        rabbitcheck,            /* V3.76  NG */
/*120 */        bobcheck                /* V3.76  NG */
		};
#endif

#ifdef WE_ARE_EXTERN
	extern  piece PAS[64];
#else
/* This is the InitialGameArray */     /* V3.50  NG */
piece       PAS[64] = {   tb,   cb,   fb,   db, roib,   fb,   cb,   tb,
			  pb,   pb,   pb,   pb,   pb,   pb,   pb,   pb,
			vide, vide, vide, vide, vide, vide, vide, vide,
			vide, vide, vide, vide, vide, vide, vide, vide,
			vide, vide, vide, vide, vide, vide, vide, vide,
			vide, vide, vide, vide, vide, vide, vide, vide,
			  pn,   pn,   pn,   pn,   pn,   pn,   pn,   pn,
			  tn,   cn,   fn,   dn, roin,   fn,   cn,   tn};
#endif

#endif  /* not PYDATA_H */

