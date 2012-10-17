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
** 2008/02/25 SE   New piece type: Magic
**
**************************** End of List ******************************/
#if !defined(PYMSG_H)
#define PYMSG_H

#include "pylang.h"

/* This is the file where all language dependent defines
** for messages are coded.
** They belong to the corresponding numbers in the
** language dependent message-files py-????.msg.
**
** Adding new messages:
** Add an enumerator *right* before MsgCount; please indicate its
** value even if C doesn't require it so that the connection of an
** enumerator to the respective entries in the laugage files is
** obvious.
** Add a string for the enumerator in *each* language file.
*/

typedef enum
{
  ErrFatal               = 0,
  MissingKing            = 1,
  NoMemory               = 2,
  InterMessage           = 3,
  NonSenseRexiExact      = 4,
  TooManySol             = 5,
  KamikazeAndHaaner      = 6,
  TryPlayNotApplicable   = 7,
  TooFairyForNeutral     = 8,
  LeoFamAndOrtho         = 9,
  CirceAndDummy          = 10,
  ColourChangeRestricted = 11,
  CavMajAndKnight        = 12,
  RebirthOutside         = 13,
  SetAndCheck            = 14,
  KamikazeAndSomeCond    = 15,
  SomeCondAndVolage      = 16,
  TwoMummerCond          = 17,
  KingCapture            = 18,
  MonoAndBiChrom         = 19,
  WrongChar              = 20,
  OneKing                = 21,
  WrongSquareList        = 22,
  MissngSquareList       = 23,
  WrongRestart           = 24,
  MadrasiParaAndOthers   = 25,
  RexCirceImmun          = 26,
  BlackColor             = 27,
  WhiteColor             = 28,
  SomePiecesAndMaxiHeffa = 29,
  BigNumMoves            = 30,
  RoyalPWCRexCirce       = 31,
  ErrUndef               = 32,
  OffendingItem          = 33,
  InputError             = 34,
  WrongPieceName         = 35,
  PieSpecNotUniq         = 36,
  WBNAllowed             = 37,
  UnrecCondition         = 38,
  OptNotUniq             = 39,
  WrongInt               = 40,
  UnrecOption            = 41,
  ComNotUniq             = 42,
  ComNotKnown            = 43,
  NoStipulation          = 44,
  WrOpenError            = 45,
  RdOpenError            = 46,
  TooManyInputs          = 47,
  NoColorSpec            = 48,
  UnrecStip              = 49,
  CondNotUniq            = 50,
  EoFbeforeEoP           = 51,
  InpLineOverflow        = 52,
  NoBegOfProblem         = 53,
  InternalError          = 54,
  FinishProblem          = 55,
  Zugzwang               = 56,
  Threat                 = 57,
  But                    = 58,
  TimeString             = 59,
  MateInOne              = 60,
  NewLine                = 61,
  ImitWFairy             = 62,
  ManyImitators          = 63,
  KingKamikaze           = 64,
  PercentAndParrain      = 65,
  ProblemIgnored         = 66,
  SomeCondAndAntiCirce   = 67,
  EinsteinAndFairyPieces = 68,
  SuperCirceAndOthers    = 69,
  HashedPositions        = 70,
  ChameleonPiecesAndChess= 71,
  CheckingLevel1         = 72,
  CheckingLevel2         = 73,
  StipNotSupported       = 74,
  MultipleGoalsWithProogGameNotAcceptable = 75,
  ToManyEpKeySquares     = 76,
  Abort                  = 77,
  TransmRoyalPieces      = 78,
  UnrecRotMirr           = 79,
  PieceOutside           = 80,
  ContinuedFirst         = 81,
  Refutation             = 82,
  NoFrischAufPromPiece   = 83,
  ProofAndFairyConditions= 84,
  IsardamAndMadrasi      = 85,
  ChecklessUndecidable   = 86,
  OverwritePiece         = 87,
  MarsCirceAndOthers     = 88,
  PotentialMates         = 89,
  NonsenseCombination    = 90,
  VogtlanderandIsardam   = 91,
  AssassinandOthers      = 92,
  RepublicanandnotMate	 = 93,
  IncrementHashRateLevel = 94,
  DecrementHashRateLevel = 95,
  IntelligentRestricted =  96,
  NothingToRemove       = 97,
  NoMaxTime             = 98,
  NoStopOnShortSolutions= 99,
  SingleBoxAndFairyPieces= 100,
  UndefLatexPiece       = 101,
  HunterTypeLimitReached= 102,
  IncompatibleRoyalSettings = 103,
  LosingChessNotInCheckOrMateStipulations = 104,
  TakeMakeAndFairy      = 105,
  MagicAndFairyPieces   = 106,
  TooManyMagic          = 107,
  NoRepublicanWithConditionsDependingOnCheck = 108,
  CantDecideWhoIsAtTheMove = 109,
  ThreatOptionAndExactStipulationIncompatible = 110,
  GhostHauntedChessAndCirceKamikazeHaanIncompatible = 111,
  WhiteToPlayNotApplicable = 112,
  SetPlayNotApplicable = 113,
  PostKeyPlayNotApplicable = 114,
  QuodlibetNotApplicable = 115,
  GoalIsEndNotApplicable = 116,
  GoalIsEndAndQuodlibetIncompatible = 117,
  CantDecideOnSideWhichConditionAppliesTo = 118,
  NoCageImmuneWithoutCage = 119,

  MsgCount /* THIS MUST BE THE LAST ENUMERATOR */
} message_id_t;

boolean InitMsgTab(Language l);

char const *GetMsgString (message_id_t id);

void VerifieMsg(message_id_t id);
void ErrorMsg(message_id_t id);
void Message(message_id_t id);
void FtlMsg(message_id_t id);

#endif  /* PYMSG_H */
