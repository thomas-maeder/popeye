/********************** MODIFICATIONS to py.h **************************
** This is the list of modifications done to py.h
**
** Date       Who  What
**
** 2006/05/07 SE   bug fix: StipExch + Duplex 
** 
** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
**
** 2006/07/30 SE   New condition: Schwarzschacher  
**
** 2007/01/28 SE   New condition: NormalPawn 
**
** 2007/01/28 SE   New condition: Annan Chess 
**
**************************** End of List ******************************/

#ifndef PY_H
#define PY_H


/*   Sometimes local variables are used, that can potentially be
     used without any propper initial value. At least the compiler
     cannot assure a propper initialisation.
     The programmer should verify these places. To spot these
     places, I introduced the following macro. It may be defined
     to nothing, to get the old code -- without var intialisation.
 */
#define VARIABLE_INIT(var)	var=0

#ifdef C370
/* On MVS-systems there's the problem that the C/370 compiler and the
   linker only take the first 8 significant characters valid for
   external functions and variables.
   You need the "pragma map" compiler-directive to avoid this conflict
   without renaming the C-identifiers.   HD 07/07/93
*/

#pragma map( renspiegelfile, "RENSPI01" )
#pragma map( renspiegel, "RENSPI02" )
#pragma map( flagwhitemummer, "FLAGWH01" )
#pragma map( flagwhiteexact, "FLAGWH02" )
#pragma map( flagblackmummer, "FLAGBL01" )
#pragma map( flagblackexact, "FLAGBL02" )
#pragma map( cirrenroib, "CIRREN01" )
#pragma map( cirrenroin, "CIRREN02" )
#pragma map( immrenroin, "IMMREN01" )
#pragma map( immrenroib, "IMMREN02" )
#pragma map( hash_value_1, "HASHVAL1" )
#pragma map( hash_value_2, "HASHVAL2" )

#endif /* C370 */

#ifndef OSBIT
#if defined(SIXTEEN) || defined(WIN16)
#	define OSBIT "16"
#else
#ifdef SIXTYFOUR
#	define OSBIT "64"
#else
#	define OSBIT "32"
#endif	/* Default */
#endif	/* SIXTYFOUR */
#endif	/* SIXTEEN || WIN16 */

#ifndef OSTYPE
#ifdef DATABASE
#       define OSTYPE "DATABASE"
#else
#ifdef C370
#       define OSTYPE "MVS"
#else
#ifdef DOS
#       define OSTYPE "DOS"
#else
#ifdef ATARI
#       define OSTYPE "ATARI"
#else
#if defined(WIN98)
#       define OSTYPE "WINDOWS98"
#else
#if defined(WIN16) || defined(WIN32)
#       define OSTYPE "WINDOWS"
#else
#ifdef UNIX
#ifdef __GO32__
#       define OSTYPE "DOS"
#else
#       define OSTYPE "UNIX"
#endif  /* __GO32__ */
#else   /* ! UNIX: use default version string */
#       define OSTYPE "C"
#endif  /* UNIX */
#endif  /* WIN16 || WIN32 */
#endif  /* WIN98 */
#endif  /* ATARI */
#endif  /* DOS */
#endif  /* C370 */
#endif  /* DATABASE */
#endif	/* OSTYPE */

#define STRINGIZEIMPL(x) #x
#define STRINGIZE(x) STRINGIZEIMPL(x)
#define VERSIONSTRING "Popeye "OSTYPE"-"OSBIT"Bit v"STRINGIZE(VERSION)


#ifdef BSD
#       define  strchr  index
#endif

#ifdef DEBUG
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

#if defined(TESTHASH)
#       ifndef HASHRATE
#               define HASHRATE
#       endif   /* !HASHRATE */
#endif  /* TESTHASH */

#define rot90        0
#define rot180       1
#define rot270       2
#define mirra1h1     3
#define mirra1a8     4
#define mirra1h8     5
#define mirra8h1     6

#define maxnrhuntertypes 10

#define vide    0
#define obs     1
#define roib    2
#define pb      3
#define db      4
#define cb      5
#define tb      6
#define fb      7
#define leob    8
#define maob    9
#define paob    10
#define vaob    11
#define roseb   12
#define sb      13
#define nb      14
#define zb      15
#define chb     16
#define gib     17
#define rccinqb 18              /* V2.60  NG */
#define bub     19
#define vizirb  20
#define alfilb  21
#define fersb   22
#define dabb    23
#define lionb   24
#define nequib  25              /* V2.60  NG */
#define locb    26
#define pbb     27
#define amazb   28              /* V1.4c  NG */
#define impb    29              /* V1.4c  NG */
#define princb  30              /* V1.4c  NG */
#define gnoub   31              /* V1.4c  NG */
#define antilb  32              /* V1.5c  NG */
#define ecurb   33              /* V1.5c  NG */
#define waranb  34              /* V1.5c  NG */
#define dragonb 35              /* V1.5c  NG */
#define kangoub 36              /* V1.5c  NG */
#define csb     37              /* V1.5c  NG */
#define ubib    38              /* V1.7c  NG */
#define hamstb  39              /* V2.1c  NG */
#define mooseb  40              /* V2.1c  NG */
#define eagleb  41              /* V2.1c  NG */
#define sparrb  42              /* V2.1c  NG */
#define archb   43              /* V2.1c  NG */
#define reffoub 44              /* V2.1c  NG */
#define cardb   45              /* V2.1c  NG */
#define nsautb  46              /* V2.1c  NG */
#define dummyb  47              /* V2.1c  NG */
#define camridb 48              /* V2.1c  NG */
#define zebridb 49              /* V2.1c  NG */
#define gnuridb 50              /* V2.1c  NG */
#define camhopb 51              /* V2.1c  NG */
#define zebhopb 52              /* V2.1c  NG */
#define gnuhopb 53              /* V2.1c  NG */
#define dcsb    54              /* V2.1c  NG */
#define refcb   55              /* V2.1c  NG */
#define equib   56              /* V2.60  NG */
#define catb    57              /* V2.60  NG */
#define bspawnb 58              /* V2.60  NG */
#define spawnb  59              /* V2.60  NG */
#define tlionb  60              /* V3.0  TLi */
#define flionb  61              /* V3.0  TLi */
#define sireneb 62              /* V3.0  TLi */
#define tritonb 63              /* V3.0  TLi */
#define nereidb 64              /* V3.0  TLi */
#define orphanb 65              /* V3.0  TLi */
#define edgehb  66              /* V3.0  TLi */
#define moab    67              /* V3.0  TLi */
#define rhuntb  68              /* V3.03  TLi */
#define bhuntb  69              /* V3.03  TLi */
#define maoridb 70              /* V3.1  TLi */
#define moaridb 71              /* V3.1  TLi */
#define rookhopb        72      /* V3.1  TLi */
#define bishophopb      73      /* V3.1  TLi */
#define ekingb  74              /* V3.05  NG */
#define bscoutb 75              /* V3.05  NG */
#define gscoutb 76              /* V3.05  NG */
#define skyllab 77              /* V3.12  TLi */
#define charybdisb      78      /* V3.12  TLi */
#define contragrasb     79      /* V3.22  TLi */
#define roselionb       80      /* V3.23  TLi */
#define rosehopperb     81      /* V3.23  TLi */
#define okapib          82
#define leap37b         83      /* V3.34  TLi */
#define g2b             84      /* V3.34  TLi */
#define g3b             85      /* V3.35  TLi */
#define leap16b         86      /* V3.38  NG */
#define leap24b         87      /* V3.42  TLi */
#define leap35b         88      /* V3.42  TLi */
#define doublegb        89      /* V3.44  SE */
#define khb             90      /* V3.44  SE */
#define orixb           91      /* V3.44  NG */
#define leap15b         92      /* V3.46  NG */
#define leap25b         93      /* V3.46  NG */
#define gralb           94      /* V3.46  NG */
#define rooklocustb     95
#define bishoplocustb   96
#define nightlocustb    97
#define vizridb         98
#define fersridb        99
#define bisonb         100      /* V3.60  TLi */
#define elephantb      101      /* V3.62  TLi */
#define naob           102      /* V3.62  TLi */
#define rookmooseb     103      /* V3.62  NG */
#define rookeagleb     104      /* V3.62  NG */
#define rooksparrb     105      /* V3.62  NG */
#define bishopmooseb   106      /* V3.62  NG */
#define bishopeagleb   107      /* V3.62  NG */
#define bishopsparrb   108      /* V3.62  NG */
#define raob           109      /* V3.63  NG */
#define scorpionb      110      /* V3.63  NG */
#define margueriteb    111      /* V3.64  TLi */
#define	leap36b        112	/* V3.64  TLi */
#define nrlionb	       113	/* V3.64  TLi */
#define mrlionb	       114	/* V3.64  TLi */
#define molionb        115      /* V3.65  TLi */
#define andergb        116      /* V3.65  TLi */
#define friendb        117      /* V3.65  TLi */
#define dolphinb       118      /* V3.70  TLi */
#define rabbitb        119      /* V3.76  NG */
#define bobb           120      /* V3.76  NG */
#define equiengb       121	    /* V3.78  SE */
#define equifrab       122	    /* V3.78  SE */
#define querqub        123	    /* V3.78  SE */
#define bouncerb       124      /* V4.03  SE */
#define rookbouncerb   125      /* V4.03  SE */
#define bishopbouncerb 126      /* V4.03  SE */
#define chinesepawnb   127
#define hunter0b       128
#define derbla         (hunter0b+maxnrhuntertypes)
#define roin    -2
#define pn      -3
#define dn      -4
#define cn      -5
#define tn      -6
#define fn      -7
#define leon    -8
#define maon    -9
#define paon    -10
#define vaon    -11
#define rosen   -12
#define sn      -13
#define nn      -14
#define zn      -15
#define chn     -16
#define gin     -17
#define rccinqn -18             /* V2.60  NG */
#define bun     -19
#define vizirn  -20
#define alfiln  -21             /* bug fixed. V1.4c  NG */
#define fersn   -22             /* bug fixed. V1.4c  NG */
#define dabn    -23
#define lionn   -24
#define nequin  -25             /* V2.60  NG */
#define locn    -26
#define pbn     -27
#define amazn   -28             /* V1.4c  NG */
#define impn    -29             /* V1.4c  NG */
#define princn  -30             /* V1.4c  NG */
#define gnoun   -31             /* V1.4c  NG */
#define antiln  -32             /* V1.5c  NG */
#define ecurn   -33             /* V1.5c  NG */
#define warann  -34             /* V1.5c  NG */
#define dragonn -35             /* V1.5c  NG */
#define kangoun -36             /* V1.5c  NG */
#define csn     -37             /* V1.5c  NG */
#define ubin    -38             /* V1.7c  NG */
#define hamstn  -39             /* V2.1c  NG */
#define moosen  -40             /* V2.1c  NG */
#define eaglen  -41             /* V2.1c  NG */
#define sparrn  -42             /* V2.1c  NG */
#define archn   -43             /* V2.1c  NG */
#define reffoun -44             /* V2.1c  NG */
#define cardn   -45             /* V2.1c  NG */
#define nsautn  -46             /* V2.1c  NG */
#define dummyn  -47             /* V2.1c  NG */
#define camridn -48             /* V2.1c  NG */
#define zebridn -49             /* V2.1c  NG */
#define gnuridn -50             /* V2.1c  NG */
#define camhopn -51             /* V2.1c  NG */
#define zebhopn -52             /* V2.1c  NG */
#define gnuhopn -53             /* V2.1c  NG */
#define dcsn    -54             /* V2.1c  NG */
#define refcn   -55             /* V2.1c  NG */
#define equin   -56             /* V2.60  NG */
#define catn    -57             /* V2.60  NG */
#define bspawnn -58             /* V2.60  NG */
#define spawnn  -59             /* V2.60  NG */
#define tlionn  -60             /* V3.0  TLi */
#define flionn  -61             /* V3.0  TLi */
#define sirenen -62             /* V3.0  TLi */
#define tritonn -63             /* V3.0  TLi */
#define nereidn -64             /* V3.0  TLi */
#define orphann -65             /* V3.0  TLi */
#define edgehn  -66             /* V3.0  TLi */
#define moan    -67             /* V3.0  TLi */
#define rhuntn  -68             /* V3.03  TLi */
#define bhuntn  -69             /* V3.03  TLi */
#define maoridn -70             /* V3.1  TLi */
#define moaridn -71             /* V3.1  TLi */
#define rookhopn        -72     /* V3.1  TLi */
#define bishophopn      -73     /* V3.1  TLi */
#define ekingn  -74             /* V3.05  NG */
#define bscoutn -75             /* V3.05  NG */
#define gscoutn -76             /* V3.05  NG */
#define skyllan -77             /* V3.12  TLi */
#define charybdisn      -78     /* V3.12  TLi */
#define contragrasn     -79     /* V3.22  TLi */
#define roselionn       -80     /* V3.23  TLi */
#define rosehoppern     -81     /* V3.23  TLi */
#define okapin          -82     /* V3.32  TLi */
#define leap37n         -83     /* V3.34  TLi */
#define g2n             -84     /* V3.34  TLi */
#define g3n             -85     /* V3.35  TLi */
#define leap16n         -86     /* V3.38  NG */
#define leap24n         -87     /* V3.42  TLi */
#define leap35n         -88     /* V3.42  TLi */
#define doublegn        -89     /* V3.44  SE */
#define khn             -90     /* V3.44  SE */
#define orixn           -91     /* V3.44  NG */
#define leap15n         -92     /* V3.46  NG */
#define leap25n         -93     /* V3.46  NG */
#define graln           -94     /* V3.46  NG */
#define rooklocustn     -95
#define bishoplocustn   -96
#define	nightlocustn    -97
#define vizridn         -98
#define fersridn        -99
#define bisonn         -100     /* V3.60  TLi */
#define elephantn      -101     /* V3.62  TLi */
#define naon           -102     /* V3.62  TLi */
#define rookmoosen     -103     /* V3.62  NG */
#define rookeaglen     -104     /* V3.62  NG */
#define rooksparrn     -105     /* V3.62  NG */
#define bishopmoosen   -106     /* V3.62  NG */
#define bishopeaglen   -107     /* V3.62  NG */
#define bishopsparrn   -108     /* V3.62  NG */
#define raon           -109     /* V3.63  NG */
#define scorpionn      -110     /* V3.63  NG */
#define margueriten    -111     /* V3.64  TLi */
#define	leap36n        -112     /* V3.64  TLi */
#define nrlionn	       -113	/* V3.64  TLi */
#define mrlionn	       -114	/* V3.64  TLi */
#define molionn        -115     /* V3.65  TLi */
#define andergn        -116     /* V3.65  TLi */
#define friendn        -117     /* V3.65  TLi */
#define dolphinn       -118     /* V3.70  TLi */
#define rabbitn        -119     /* V3.76  NG */
#define bobn           -120     /* V3.76  NG */
#define equiengn       -121	    /* V3.78  SE */
#define equifran       -122	    /* V3.78  SE */
#define querqun        -123	    /* V3.78  SE */
#define bouncern       -124     /* V4.03  SE */
#define rookbouncern   -125     /* V4.03  SE */
#define bishopbouncern -126     /* V4.03  SE */
#define chinesepawnn   -127  
#define hunter0n       -128
#define dernoi         (hunter0n-maxnrhuntertypes)

#define maxsquare       576     /* V2.60  NG */
#define haut            375     /* V2.60  NG */
#define bas             200     /* V2.60  NG */

#ifdef  DATABASE
#       define maxply   302
#else
#ifdef UNIX
#       define maxply  1002     /* V3.65  NG */
#else
#ifdef WIN32
#       define maxply   702     /* V3.41  NG */
#elif OS2
#       define maxply   302     /* V3.42  NG */
#else   
#ifdef SIXTEEN /* DOS 16 Bit, ... */
#ifdef MSG_IN_MEM               /* V3.52  TLi, V3.60  NG */
#       define maxply   26
#else
#       define maxply   48      /* V3.39  TLi, V3.60  NG */
#endif  /* MSG_IN_MEM */
#else
#	define maxply   702     /* V3.62  NG */
#endif	/* SIXTEEN */
#endif  /* WIN32 */
#endif  /* UNIX */
#endif /* DATABASE */

#define toppile (60*maxply)     /* V2.90  NG */ /* V3.78  NG */
#define maxvec          136     /* V3.46  NG */ /* V3.73 ElB*/

#define bl      ' '
#define blanc   0
#define noir    1
#define false   0
#define true    1
#define initsquare      0       /* to initialize square-variables  V2.60  NG */
#define maxinum 10      /* max. Anz. Imitatoren V2.4d  TM */
#define nullsquare 1            /* V3.70 SE */

/* These typedefs should be judged for efficiency  2.0a */

typedef int     integer;        /* V1.4c  NG */
typedef short   smallint;       /* V1.4c  NG */

#ifdef SIXTEEN
	typedef char    boolean;        /* V2.4c  NG */
	typedef char    piece;          /* V2.4c  NG */
#else
	typedef int     boolean;        /* must be int on NSC machines. V2.1c  NG */
	typedef int     piece;
#endif

typedef unsigned long   Flags;          /* V3.53  TLi */

typedef smallint        square;
typedef integer         couleur;
typedef integer         numecoup;
typedef integer         ply;
typedef smallint        numvec;         /* V2.1c  NG */

typedef smallint        echiquier [maxsquare+4];  /* V2.60  NG, V3.55  TLi */
typedef square          pilecase [maxply+1];
typedef couleur         pilecouleur [maxply+1];

typedef struct {
    square departure;
    square arrival;
    square capture;
} move_generation_elmt;

typedef struct {
    boolean found;
    move_generation_elmt move;
} killer_state;

typedef struct {
	square          cdzz, cazz, cpzz;
	square          sqren;
	piece           pjzz, ppri, ren_parrain, norm_prom, cir_prom;
						/* V3.02 TLi */
	piece           pjazz;                  /* V3.1  TLi */
	square          renkam;         /* V2.4d  TM */
	smallint        numi;           /* V2.4d  TM */
	smallint        sum;            /* V2.4d  TM */
	boolean         promi,          /* V2.4d  TM */
			echec,
			bool_senti,
			bool_norm_cham_prom,    /* V3.1  TLi */
			bool_cir_cham_prom,     /* V3.1  TLi */
			tr;                     /* V3.1  TLi */
	Flags           speci, ren_spec;
	square		repub_k;		/* V3.50 SE */
	Flags           new_spec;		/* V3.57 SE */ 
	square		hurdle;			/* V3.64 SE */
	square		sb2where;		/* V3.71 TM */
	piece		sb2what;		/* V3.71 TM */
	square		sb3where;		/* V3.72 TM */
	piece		sb3what;		/* V3.72 TM */
	square		mren;
	boolean		osc;            	/* V3.80 SE */
} coup;

#define	tabmaxcp	2048			/* V3.80 TLi */

typedef struct {                                        /* V2.70 TLi */
	smallint        nbr,
			cp[3*maxply];
	coup            liste[tabmaxcp];	/* V3.80 TLi */
} tab;

typedef struct {                                        /* V3.02  TLi */
	move_generation_elmt move;
	int                  nr_opponent_moves;
} empile_optimization_table_elmt;

/* V3.03  TLi */
enum {
  empile_optimization_priorize_killmove_by = 5
};

typedef enum {
  move_generation_optimized_by_nr_opponent_moves,
  move_generation_optimized_by_killer_move,
  move_generation_not_optimized
} move_generation_mode_type;


typedef square imarr[maxinum];        /* Standorte der Imitatoren V2.4d  TM */

/* These are definitions to implement arrays with lower index != 0
** as they are common in PASCAL. The arrays zzza? are defined in
** pydata.h                                                     ElB
*/
#define nbpiece         (zzzaa - dernoi)

/*****  V3.20  TLi  *****  begin  *****/
/* typedef      unsigned char   ShortFlags; V3.22  TLi */

#define MagicSq         0
#define WhForcedSq      1
#define BlForcedSq      2
#define WhConsForcedSq  3
#define BlConsForcedSq  4
#define NoEdgeSq        5
#define SqColor         6
#define WhPromSq        7
#define BlPromSq        8
#define Grid            9

/* to make function ReadSquares in PYIO.C more convinient define
** ReadImitators und ReadHoles and ReadEpSquares too. They can have any positiv
** number, but must not coincide with MagicSq...BlConsForcedSq.   TLi
** Must also not coincide with  WhPromSq  and  BlPromSq.   NG
*/
#define ReadImitators           10
#define ReadHoles               11
#define ReadEpSquares           12
#define ReadFrischAuf           13
#define ReadBlRoyalSq           14
#define ReadWhRoyalSq           15
#define ReadNoCastlingSquares   16

#define sq_spec         (zzzan - bas)
#define sq_num          (zzzao - bas)
#define NoEdge(i)       TSTFLAG(sq_spec[i], NoEdgeSq)
#define SquareCol(i)    TSTFLAG(sq_spec[i], SqColor)
#define GridNum(s)      (sq_spec[s] >> Grid)

/*****  V3.20  TLi  *****   end   *****/

#define BIT(pos)                (1<<(pos))
#define TSTFLAG(bits,pos)       (((bits)&BIT(pos))!=0)
#define CLRFLAG(bits,pos)       ((bits)&= ~BIT(pos))
#define SETFLAG(bits,pos)       ((bits)|= BIT(pos))
#define CLEARFL(bits)           ((bits)=0)

#define TSTFLAGMASK(bits,mask)  ((bits)&(mask))
#define CLRFLAGMASK(bits,mask)  ((bits) &= ~(mask))
#define SETFLAGMASK(bits,mask)  ((bits) |= (mask))

/* The following definitions of enumerated types follow
** a certain template. If you add some more definitions
** be sure not to put other things between lines where
** identifiers for the enumerations occur.
** This certain template of coding enumerations is used
** in an awk script to change the enumerations to #defines
** and backwards. Look at the scripts todef.awk and toenum.awk
** If you want to give some comments to the identifiers, do it
** in pydata.h. The comments there will be included in the
** documentation.
*/

typedef enum {
	False, True
} Boolean;

#define False   0
#define True    1

#include "pylang.h"             /* py3.04 ElB */

/*--- Start of typedef enum {---*/
#define BeginProblem    0
#define EndProblem      1
#define NextProblem     2
#define StipToken       3
#define Author  	4
#define Origin   	5
#define PieceToken      6
#define CondToken       7
#define OptToken        8
#define RemToken        9
#define TraceToken      10
#define InputToken      11
#define SepToken        12
#define TitleToken      13
#define TwinProblem     14
#define ZeroPosition    15
#define LaTeXToken      16
#define LaTeXPieces     17
#define Award           18
#define Array		19
#define Forsyth         20
#define TokenCount      21
typedef int Token;
/*--- End of } Token;---*/

/*--- Start of typedef enum {---*/
#define Invalid 0
#define Empty   1
#define King    2
#define Pawn    3
#define Queen   4
#define Knight  5
#define Rook    6
#define Bishop  7
#define Leo     8
#define Mao     9
#define Pao     10
#define Vao     11
#define Rose    12
#define Grashopper      13
#define NightRider      14
#define Zebra   15
#define Camel   16
#define Giraffe 17
#define RootFiftyLeaper 18
#define Bucephale       19
#define Wesir   20
#define Alfil   21
#define Fers    22
#define Dabbaba 23
#define Lion    24
#define NonStopEquihopper       25
#define Locust  26
#define BerolinaPawn    27
#define Amazone 28
#define Empress 29
#define Princess        30
#define Gnu     31
#define Antilope        32
#define Squirrel        33
#define Waran   34
#define Dragon  35
#define Kangaroo        36
#define SpiralSpringer  37
#define UbiUbi  38
#define Hamster 39
#define Elk     40
#define Eagle   41
#define Sparrow 42
#define Archbishop      43
#define ReflectBishop   44
#define Cardinal        45
#define NightriderHopper        46
#define Dummy   47
#define Camelrider      48
#define Zebrarider      49
#define Gnurider        50
#define CamelHopper     51
#define ZebraHopper     52
#define GnuHopper       53
#define DiagonalSpiralSpringer  54
#define BouncyKnight    55
#define EquiHopper      56
#define CAT     57
#define SuperBerolinaPawn       58
#define SuperPawn       59
#define RookLion        60
#define BishopLion      61
#define Sirene  62
#define Triton  63
#define Nereide 64
#define Orphan  65
#define EdgeHog 66
#define Moa     67
#define RookHunter      68
#define BishopHunter    69
#define MaoRider        70
#define MoaRider        71
#define RookHopper      72
#define BishopHopper    73
#define ErlKing         74
#define BoyScout        75
#define GirlScout       76
#define Skylla          77
#define Charybdis       78
#define ContraGras      79
#define RoseLion        80
#define RoseHopper      81
#define Okapi           82
#define Leap37          83
#define GrassHopper2    84
#define GrassHopper3    85
#define Leap16          86
#define Leap24          87
#define Leap35          88
#define DoubleGras      89
#define KingHopper      90
#define Orix            91
#define Leap15          92
#define Leap25          93
#define Gral            94
#define RookLocust      95
#define BishopLocust    96
#define NightLocust     97
#define WesirRider      98
#define FersRider       99
#define Bison          100
#define Elephant       101
#define Nao            102
#define RookMoose      103
#define RookEagle      104
#define RookSparrow    105
#define BishopMoose    106
#define BishopEagle    107
#define BishopSparrow  108
#define Rao            109
#define Scorpion       110
#define Marguerite     111
#define Leap36         112
#define NightRiderLion 113
#define MaoRiderLion   114
#define MoaRiderLion   115
#define AndernachGrasshopper     116
#define Friend         117
#define Dolphin        118
#define Rabbit         119
#define Bob            120
#define EquiEnglish    121
#define EquiFrench     122
#define Querquisite    123
#define Bouncer        124
#define RookBouncer    125
#define BishopBouncer  126
#define ChinesePawn    127
#define Hunter0        128
#define PieceCount     (Hunter0+maxnrhuntertypes)
typedef int PieNam;
/*--- End of } PieNam;---*/

typedef char PieceChar[2];
typedef PieceChar       PieTable[PieceCount];

/* for multiple variants of conditions -- V3.50  SE */
#define TypeB                   0
#define PionAdverse             1
#define AntiCirTypeCheylan      2
#define AntiCirTypeCalvet       3
#define PionNeutral             4
#define PionNoirMaximum         5
#define PionBlancMaximum        6
#define ParaSent		7
#define PionTotalMaximum        8
#define SentBerolina            9
#define Type1                  10
#define Type2                  11
#define Type3                  12
#define Neighbour              13
#define TypeC                  14
#define TypeD                  15
#define VariantTypeCount       16

/* for intelligent specification of one-sided conditions -- V3.62 SE */
#define maxi			0
#define ultraschachzwang	1
#define ExtraCondCount		2

/* names for flags used to distinguish stipulations */

/*--- Start of typedef enum {---*/
#define Alternate       0
#define Series          1
#define Semi            2
#define Reci            3
#define Exact           4
#define Intro           5
#define FlowCount       6
typedef int Flow;
/*--- End of } Flow;---*/

/*--- Start of typedef enum {---*/
#define Direct          0
#define Help            1
#define Self            2
#define Reflex          3
#define Proof           4
#define SortCount       5
typedef int Sort;
/*--- End of } Sort;---*/

/* Now let's define some BIT-Masks, to check some of the above set conditions */
#define FlowBit(f)      (BIT(f))
#define SortBit(s)      (BIT((FlowCount+(s))))

/* ein paar weitere                                     V2.70  TLi */
#define FlowFlag(s)     ((FlowBit(s)&StipFlags) !=0)
#define SortFlag(s)     ((SortBit(s)&StipFlags) !=0)

/* V3.33  TLi */
#define BorderSpec      0
#define EmptySpec       (BIT(Black)+BIT(White))

/* Some remarks to the efficiency of testing bits in C-code
** Look at the following C-fragment:
**      if (a&8 == 0) {
**              do something
**      }
** On a Risc-Maschine this if-statement compiles to three instructions.
** One for loading the variable a (if its not already in a register),
** one for anding with the constant (setting the flags) and the conditional
** jump instruction.
** What's on a CISC (like ix86,68xxx oe) ?
** Here it depends extremly on the compiler !!
** Best code would a thing like the test-instruction of the ix86.
** Then it compiles to this instruction plus the conditinal jump.
** Real code, generated by compilers, loads first the variable a,
** does the test(=and)-instruction and then the conditional jump.
** Is there any improvement possible, when the condition has to be
** testet a lot of times ? - Look at the following C-Code:
**      b= a&8 == 0;            * One-time Initialisation *
**      if (b) {                * This comes more than once *
**              do something
**      }
** The overhead of the initialisation for b is not counted here.
** We look at the if(b). On a RISC machine, there is no change:
** Load the variable b, test it against 0 and jump if appropriate.
** On a CISC ? Best would be the test-Instruction with a conditional jump.
** In this case it generated by most C-compilers. Improvement is according
** to acutal generated code one instruction. But only because of loosy
** optimizers.
** Other results come up, if you use the macro TestStip.
** Here is always one instruction more neccesary:
** Evaluation of the '&' and then '=='. The result of this operation is
** NOT a `boolean' value as a C runtime system would store it in a char,
** or ** use it in the evaluation of a boolean expression. (Especially when
** '^' is used there).
** Therefore if you need to use the result of TestStip quite often in a
** boolean expression, you can gain speed if you assign the result
** of TestStip to a variable, and use the variable in the expression
** instead of TestStip. The net improvement depends on the architecture
** of your machine. On RISC maschines the improvement is not very dramatic,
** but can be on CISC-maschines.
** But this is also question of the optimizer. If the optimizer would work
** a little bit harder, he could recognize this an do it by himself.
** But I do not know any optimizer which is capable of this.
**                                                      ElB 31.9.1991
*/



/* names for options */
/* Please keep the 3 options 14, 15 and 16. I need them for my database project. TLi */
/*--- Start of typedef enum {---*/
#define soltout          0
#define solapparent      1
#define appseul          2
#define solvariantes     3
#define movenbr          4
#define restart          5
#define sansrb           6
#define duplex           7
#define sansrn           8
#define solmenaces       9
#define nothreat        10
#define solessais       11
#define maxsols         12
#define solflights      13
#define search          14
#define multi           15
#define nosymmetry      16
#define nontrivial      17
#define keepmating      18
#define enpassant       19
#define noboard         20
#define noshort         21
#define halfduplex      22
#define postkeyplay     23
#define intelligent     24
#define maxtime         25
#define nocastling      26
#define quodlibet       27
#define stoponshort     28
#define beep            29
#define OptCount        30
typedef int Opt;
/*--- End of } Opt; ---*/

/* Names for conditions */
/*--- Start of typedef enum {---*/
#define rexincl                  0
#define circe                    1
#define circemalefique           2
#define madras                   3
#define volage                   4
#define hypervolage              5
#define bichro                   6
#define monochro                 7
#define gridchess                8
#define koeko                    9
#define blackedge               10
#define whiteedge               11
#define leofamily               12
#define chinoises               13
#define patrouille              14
#define pwc                     15
#define nocapture               16
#define immun                   17
#define immunmalefique          18
#define contactgrid             19
#define imitators               20
#define cavaliermajeur          21
#define haanerchess             22
#define chamcirce               23
#define couscous                24
#define circeequipollents       25
#define circefile               26
#define blmax                   27
#define blmin                   28
#define whmax                   29
#define whmin                   30
#define magic                   31
#define sentinelles             32
#define tibet                   33
#define dbltibet                34
#define circediagramm           35
#define holes                   36
#define blcapt                  37
#define whcapt                  38
#define refl_king               39
#define trans_king              40
#define blfollow                41
#define whfollow                42
#define duellist                43
#define parrain                 44
#define noiprom                 45
#define circesymmetry           46
#define vogt                    47
#define einstein                48
#define bicolores               49
#define newkoeko                50
#define circeclone              51
#define anti                    52
#define circefilemalefique      53
#define circeantipoden          54
#define circeclonemalefique     55
#define antispiegel             56
#define antidiagramm            57
#define antifile                58
#define antisymmetrie           59
#define antispiegelfile         60
#define antiantipoden           61
#define antiequipollents        62
#define immunfile               63
#define immundiagramm           64
#define immunspiegelfile        65
#define immunsymmetry           66
#define immunantipoden          67
#define immunequipollents       68
#define reveinstein             69
#define supercirce              70
#define degradierung            71
#define norsk                   72
#define exact                   73
#define traitor                 74
#define andernach               75
#define whforsqu                76
#define whconforsqu             77
#define blforsqu                78
#define blconforsqu             79
#define ultra                   80
#define chamchess               81
#define beamten                 82
#define glasgow                 83
#define antiandernach           84
#define frischauf               85
#define circemalefiquevertical  86
#define isardam                 87
#define ohneschach              88
#define circediametral          89
#define promotiononly           90
#define circerank               91
#define exclusive               92
#define mars                    93
#define marsmirror              94
#define phantom                 95
#define whrefl_king             96
#define blrefl_king             97
#define whtrans_king            98
#define bltrans_king            99
#define antieinstein           100
#define couscousmirror         101
#define blroyalsq              102
#define whroyalsq              103
#define brunner                104
#define plus                   105
#define circeassassin          106
#define patience               107
#define republican	       108
#define extinction             109
#define central                110
#define actrevolving           111
#define messigny               112
#define woozles                113
#define biwoozles              114
#define heffalumps             115
#define biheffalumps           116
#define rexexcl                117
#define whprom_sq              118
#define blprom_sq              119
#define nowhiteprom            120
#define noblackprom            121
#define eiffel                 122
#define blackultraschachzwang  123
#define whiteultraschachzwang  124
#define arc                    125
#define shieldedkings          126
#define sting                  127
#define linechamchess          128
#define nowhcapture            129
#define noblcapture            130
#define april                  131
#define alphabetic             132
#define circeturncoats         133
#define circedoubleagents      134
#define amu                    135
#define singlebox              136
#define MAFF                   137
#define OWU                    138
#define white_oscillatingKs    139
#define black_oscillatingKs    140
#define antikings              141
#define antimars               142
#define antimarsmirror         143
#define antimarsantipodean     144
#define whsupertrans_king      145
#define blsupertrans_king      146
#define antisuper              147
#define ultrapatrouille        148
#define swappingkings          149
#define dynasty                150
#define SAT                    151
#define strictSAT              152 
#define takemake               153
#define blacksynchron          154
#define whitesynchron          155
#define blackantisynchron      156
#define whiteantisynchron      157
#define masand                 158
#define BGL                    159
#define schwarzschacher		     160
#define annan                  161
#define normalp                162
#define CondCount              163
typedef int Cond;
/*--- End of } Cond;---*/

/* Some remarks to the conditions:
**      if hypervolage is set, also volage will be set
**      if leofamily is set, also chinoises will be set
**      if patrouille is set, also patrcalcule and patrsoutien will
**              be set.
**      if immunmalefique is set, also immun and malefique will be set
**      if pwc is set, also circe will be set
**      if iprom is set, also imitators will be set      V2.4d  TM
**      if equipollents or coucou is set, also pwc will be set  V2.80  TLi
*/

/* Names for Piece-Specifications
 * umsortiert  V3.1  TLi
 */
/*--- Start of typedef enum {---*/
#define White        0
#define Black        1
#define Neutral      2
#define Kamikaze     3
#define Royal        4
#define Paralyse     5
#define Chameleon    6
#define Jigger       7
#define Volage       8
#define Beamtet      9
#define HalfNeutral  10
#define ColourChange 11
#define PieSpCount   12
typedef int PieSpec;
/*--- End of } PieSpec;---*/

/* MAKROS */

enum {
  /* if square1-square2==onerow, then square1 is one row higher than
   * square2 */
  onerow= 24,

  /* For reasons of code simplification of move generation, square a1
   * doesn't have index 0; there are some slack rows at the top and
   * bottom of the board, and some slack files at the left an right.
   */
  nr_of_slack_files_left_of_board= 8,
  nr_of_slack_rows_below_board= 8,

  nr_files_on_board= 8,
  nr_rows_on_board= 8
};

enum {
  file_rook_queenside,
  file_knight_queenside,
  file_bishop_queenside,
  file_queen,
  file_king,
  file_bishop_kingside,
  file_knight_kingside,
  file_rook_kingside
};

#define DiaCirce        PieSpCount
#define DiaRen(s)       (boardnum[(s >> DiaCirce)])
#define DiaRenMask      ((1<<DiaCirce)-1)
#define SetDiaRen(s, f) (s=((unsigned int)(((f-bas)/onerow)*8+(f-bas)%onerow)<<DiaCirce) + (s&DiaRenMask))
#define FrischAuf       PieSpCount
/* needed for Twinning Reset.  V3.76  NG */
#define ClrDiaRen(s)    (s-=((unsigned int)(s>>DiaCirce)<<DiaCirce))

#define encore()        (nbcou > repere[nbply])
#define advers(camp)    ((camp) ? blanc : noir) /* V2.60  NG */
#define color(piesqu)   (e[(piesqu)] <= roin ? noir : blanc)    /* V2.60  NG */

#define coupfort()      {kpilcd[nbply]= move_generation_stack[nbcou].departure; kpilca[nbply]= move_generation_stack[nbcou].arrival;}   /* V2.60  NG */

/* V3.33  TLi */
#define COLORFLAGS      (BIT(Black)+BIT(White)+BIT(Neutral))
#define SETCOLOR(a,b)   a=(a&~COLORFLAGS)+(b&COLORFLAGS)
#define CHANGECOLOR(a)  a^=BIT(Black)+BIT(White)

	/* V2.4d  TM */
#define imcheck(i, j) (!CondFlag[imitators] || imok((i), (j)))
#define imech(i, j) if (imcheck((i), (j))) return true
#define ridimcheck(sq, j, diff) (!CondFlag[imitators] || ridimok((sq), (j), (diff)))
#define ridimech(sq, j, diff) if (ridimcheck((sq), (j), (diff))) return true
/* V3.12  TM */
/* #define hopimcheck(sq, j, over, diff) (!CondFlag[imitators] || hopimok((sq), (j), (over), (diff))) */	/* V3.64  SE */
#define hopimcheck(sq, j, over, diff) (!checkhopim || hopimok((sq), (j), (over), (diff)))
#define maooaimcheck(sq, j, pass) (!CondFlag[imitators] || maooaimok((sq), (j), (pass)))   
/* V2.90c  TLi */
#define setneutre(i)            do {if (neutcoul != color(i)) change(i);} while(0)
#define change(i)               do {register piece pp; nbpiece[pp= e[i]]--; nbpiece[e[i]= -pp]++;} while (0)
#define finligne(i,k,p,sq)      do {register smallint kk= k; sq= i; while (e[(sq+=kk)]==vide); p= e[sq];} while (0)

/* V3.12  TLi, V3.14  NG */
#define rightcolor(ej, camp)    (camp == blanc ? ej <= roin : ej >= roib)

/* revision due to new piece grasshopper-2   V3.34 TLi */
/* I think it is more convinient to keep some old names */
#define lrhopcheck(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 0, 0, ev)
#define rhopcheck(sq, ka, ke, p, ev)    riderhoppercheck(sq, ka, ke, p, 0, 1, ev)
#define crhopcheck(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 1, 0, ev)

#define gelrhop(sq, ka, ke, camp)       geriderhopper(sq, ka, ke, 0, 0, camp)
#define gerhop(sq, ka, ke, camp)        geriderhopper(sq, ka, ke, 0, 1, camp)
#define gecrhop(sq, ka, ke, camp)       geriderhopper(sq, ka, ke, 1, 0, camp)

#define rhop2check(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 0, 2, ev)
#define gerhop2(sq, ka, ke, camp)      geriderhopper(sq, ka, ke, 0, 2, camp)

#define rhop3check(sq, ka, ke, p, ev)   riderhoppercheck(sq, ka, ke, p, 0, 3, ev)
#define gerhop3(sq, ka, ke, camp)      geriderhopper(sq, ka, ke, 0, 3, camp)

#define shopcheck(sq, ka, ke, p, ev)     riderhoppercheck(sq, ka, ke, p, 1, 1, ev)
#define geshop(sq, ka, ke, camp)        geriderhopper(sq, ka, ke, 1, 1, camp)    /* V3.44  SE/TLi */

#define PromSq(col,sq) (TSTFLAG(sq_spec[sq],col==blanc?WhPromSq:BlPromSq))

#define ChamCircePiece(p)    (((p < vide) ? - NextChamCircePiece[-p] : \
				NextChamCircePiece[p]))  /* V3.45  TLi */

#define EndOfLine(from,dir,end)  {end = from; \
				  do end+= dir; \
				  while (e[end] == vide);}

#define LegalAntiCirceMove(reb, cap, dep)  \
      (e[reb] == vide || (!AntiCirCheylan && reb == cap) || (reb == dep))

#define BGL_infinity 10000000 	/* this will do I expect; e.g. max len = 980 maxply < 1000 */

#ifdef WIN32	
/* V3.76  TBa begin */

typedef struct _MEMORYSTATUS {  /* TBa */
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

__declspec(dllimport)
void
__stdcall
GlobalMemoryStatus(
    LPMEMORYSTATUS lpBuffer
    );
/* V3.76  TBa end */
#endif  /* WIN32 */

#endif  /* PY_H */
