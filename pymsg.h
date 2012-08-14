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
  NeutralAndOrphanReflKing = 8,
  LeoFamAndOrtho         = 9,
  CirceAndDummy          = 10,
  ColourChangeRestricted = 11,
  CavMajAndKnight        = 12,
  RebirthOutside         = 13,
  SetAndCheck            = 14,
  OthersNeutral          = 15,
  KamikazeAndSomeCond    = 16,
  SomeCondAndVolage      = 17,
  TwoMummerCond          = 18,
  KingCapture            = 19,
  MonoAndBiChrom         = 20,
  WrongChar              = 21,
  OneKing                = 22,
  WrongSquareList        = 23,
  MissngSquareList       = 24,
  WrongRestart           = 25,
  MadrasiParaAndOthers   = 26,
  RexCirceImmun          = 27,
  BlackColor             = 28,
  WhiteColor             = 29,
  SomePiecesAndMaxiHeffa = 30,
  KoeKoCirceNeutral      = 31,
  BigNumMoves            = 32,
  RoyalPWCRexCirce       = 33,
  ErrUndef               = 34,
  OffendingItem          = 35,
  InputError             = 36,
  WrongPieceName         = 37,
  PieSpecNotUniq         = 38,
  WBNAllowed             = 39,
  UnrecCondition         = 40,
  OptNotUniq             = 41,
  WrongInt               = 42,
  UnrecOption            = 43,
  ComNotUniq             = 44,
  ComNotKnown            = 45,
  NoStipulation          = 46,
  WrOpenError            = 47,
  RdOpenError            = 48,
  TooManyInputs          = 49,
  NoColorSpec            = 50,
  UnrecStip              = 51,
  CondNotUniq            = 52,
  EoFbeforeEoP           = 53,
  InpLineOverflow        = 54,
  NoBegOfProblem         = 55,
  InternalError          = 56,
  FinishProblem          = 57,
  Zugzwang               = 58,
  Threat                 = 59,
  But                    = 60,
  TimeString             = 61,
  MateInOne              = 62,
  NewLine                = 63,
  ImitWFairy             = 64,
  ManyImitators          = 65,
  KingKamikaze           = 66,
  PercentAndParrain      = 67,
  ProblemIgnored         = 68,
  NeutralAndBicolor      = 69,
  SomeCondAndAntiCirce   = 70,
  EinsteinAndFairyPieces = 71,
  SuperCirceAndOthers    = 72,
  HashedPositions        = 73,
  ChameleonPiecesAndChess= 74,
  CheckingLevel1         = 75,
  CheckingLevel2         = 76,
  StipNotSupported       = 77,
  MultipleGoalsWithProogGameNotAcceptable = 78,
  ToManyEpKeySquares     = 79,
  Abort                  = 80,
  TransmRoyalPieces      = 81,
  UnrecRotMirr           = 82,
  PieceOutside           = 83,
  ContinuedFirst         = 84,
  Refutation             = 85,
  NoFrischAufPromPiece   = 86,
  ProofAndFairyConditions= 87,
  IsardamAndMadrasi      = 88,
  ChecklessUndecidable   = 89,
  OverwritePiece         = 90,
  MarsCirceAndOthers     = 91,
  PotentialMates         = 92,
  NonsenseCombination    = 93,
  VogtlanderandIsardam   = 94,
  AssassinandOthers      = 95,
  RepublicanandnotMate	 = 96,
  IncrementHashRateLevel = 97,
  DecrementHashRateLevel = 98,
  IntelligentRestricted =  99,
  NothingToRemove       = 100,
  NoMaxTime             = 101,
  NoStopOnShortSolutions= 102,
  SingleBoxAndFairyPieces= 103,
  UndefLatexPiece       = 104,
  HunterTypeLimitReached= 105,
  IncompatibleRoyalSettings = 106,
  LosingChessNotInCheckOrMateStipulations = 107,
  TakeMakeAndFairy      = 108,
  MagicAndFairyPieces   = 109,
  TooManyMagic          = 110,
  NoRepublicanWithConditionsDependingOnCheck = 111,
  CantDecideWhoIsAtTheMove = 112,
  ThreatOptionAndExactStipulationIncompatible = 113,
  GhostHauntedChessAndCirceKamikazeHaanIncompatible = 114,
  WhiteToPlayNotApplicable = 115,
  SetPlayNotApplicable = 116,
  PostKeyPlayNotApplicable = 117,
  QuodlibetNotApplicable = 118,
  GoalIsEndNotApplicable = 119,
  GoalIsEndAndQuodlibetIncompatible = 120,
  CantDecideOnSideWhichConditionAppliesTo = 121,
  NoCageImmuneWithoutCage = 122,

  MsgCount /* THIS MUST BE THE LAST ENUMERATOR */
} message_id_t;

boolean InitMsgTab(Language l);

char const *GetMsgString (message_id_t id);

void VerifieMsg(message_id_t id);
void ErrorMsg(message_id_t id);
void Message(message_id_t id);
void FtlMsg(message_id_t id);

#endif  /* PYMSG_H */
