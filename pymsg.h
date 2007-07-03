/******************** MODIFICATIONS to pymsg.h **************************
**
** Date       Who  What
** 
** 1991       ElB  Original
** 
** 1991/09/18 TLi  neu belegt: 3,4,5
**                 nicht mehr benoetigt: 45,21,46,81,51
** 1991/09/22 TLi  frei: 20,26
** 1992/02/23 NG   neu belegt: 4, 20, 22, 51
** 1992/03/30 TLi  neu belegt: 11
** 1992/09/30 TLi  frei: 31, 6, 32, 24, 25, 27
** 1998/01/20 ElB  added 98,99 (Inc/DecrementHashRateLevel)
** 1998/05/19 NG   added 100 (IntelligentRestricted)
** 2000/05/26 NG   not needed any more: 2 (PawnFirstRank)
** 2000/05/27 NG   new used: 2 (NoMemory)
** 2001/01/18 NG   new used: 11 (ColourChangeRestricted)
**
**************************** End of List ******************************/ 
#ifndef PYMSG_H
#define PYMSG_H

/* This is the file where all language dependent defines
** for messages are coded.
** They belong to the corresponding numbers in the
** language dependent message-files py-????.msg.
**
** Adding new messages:
** To add a new Message, search for an Unused*, and redefine
** it to your requirements. Be sure to change the define and
** the corresponding entries in all language-files !
** Do not redefine the Message with number 53 this message IS used
** If there is no Unused, add one at the END, right before
** the definition of MsgCount; afterwards adjust MsgCount
** accordingly!
*/

#define ErrFatal                0
#define MissingKing             1
#define NoMemory                2
#define InterMessage            3
#define NonSenseRexiExact       4
#define TooManySol              5
#define KamikazeAndHaaner       6
#define TryInLessTwo            7
#define NeutralAndOrphanReflKing        8
#define LeoFamAndOrtho          9
#define CirceAndDummy           10
#define ColourChangeRestricted  11
#define CavMajAndKnight         12
#define RebirthOutside          13
#define SetAndCheck             14
#define OthersNeutral           15
#define KamikazeAndSomeCond     16
#define SomeCondAndVolage       17
#define TwoMummerCond           18
#define KingCapture             19
#define MonoAndBiChrom          20
#define WrongChar               21
#define OneKing                 22
#define WrongFieldList          23
#define MissngFieldList         24
#define WrongRestart            25
#define MadrasiParaAndOthers    26
#define RexCirceImmun           27
#define BlackColor              28
#define WhiteColor              29
#define SomePiecesAndMaxiHeffa  30
#define KoeKoCirceNeutral       31
#define BigNumMoves             32
#define RoyalPWCRexCirce        33
#define ErrUndef                34
#define OffendingItem           35
#define InputError              36
#define WrongPieceName          37
#define PieSpecNotUniq          38
#define WBNAllowed              39
#define UnrecCondition          40
#define OptNotUniq              41
#define WrongInt                42
#define UnrecOption             43
#define ComNotUniq              44
#define ComNotKnown             45
#define NoStipulation           46
#define WrOpenError             47
#define RdOpenError             48
#define TooManyInputs           49
#define NoColorSpec             50
#define UnrecStip               51
#define CondNotUniq             52
#define EoFbeforeEoP            53
#define InpLineOverflow         54
#define NoBegOfProblem          55
#define InternalError           56
#define FinishProblem           57
#define Zugzwang                58
#define Threat                  59
#define But                     60
#define TimeString              61
#define MateInOne               62
#define NewLine                 63
#define ImitWFairy              64
#define ManyImitators           65
#define KingKamikaze            66
#define PercentAndParrain       67
#define ProblemIgnored          68
#define NeutralAndBicolor       69
#define SomeCondAndAntiCirce    70
#define EinsteinAndFairyPieces  71
#define SuperCirceAndOthers     72
#define HashedPositions         73
#define ChameleonPiecesAndChess 74
#define CheckingLevel1          75
#define CheckingLevel2          76
#define StipNotSupported        77
#define ProofAndFairyPieces     78
#define ToManyEpKeySquares      79
#define Abort                   80
#define TransmRoyalPieces       81
#define UnrecRotMirr            82
#define PieceOutside            83
#define ContinuedFirst          84
#define Refutation              85
#define NoFrischAufPromPiece    86
#define ProofAndFairyConditions 87
#define IsardamAndMadrasi       88
#define ChecklessUndecidable    89
#define OverwritePiece          90
#define MarsCirceAndOthers      91
#define PotentialMates          92
#define NonsenseCombination     93
#define VogtlanderandIsardam    94
#define AssassinandOthers       95
#define DiaStipandsomeCond      96
#define RepublicanandnotMate	97
#define	IncrementHashRateLevel	98
#define	DecrementHashRateLevel	99
#define IntelligentRestricted  100
#define NothingToRemove        101
#define NoMaxTime              102
#define NoStopOnShortSolutions 103
#define SingleBoxAndFairyPieces 104  /* V3.71 TM */
#define MsgCount               105
#endif  /* PYMSG_H */
