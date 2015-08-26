#if !defined(OUTPUT_MESSAGE_H)
#define OUTPUT_MESSAGE_H

#include "input/plaintext/language.h"

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
  ErrFatal                                          = 0,
  MissingKing                                       = 1,
  NoMemory                                          = 2,
  InterMessage                                      = 3,
  TooManySol                                        = 4,
  KamikazeAndHaaner                                 = 5,
  TryPlayNotApplicable                              = 6,
  TooFairyForNeutral                                = 7,
  LeoFamAndOrtho                                    = 8,
  CirceAndDummy                                     = 9,
  ColourChangeRestricted                            = 10,
  CavMajAndKnight                                   = 11,
  RebirthOutside                                    = 12,
  SetAndCheck                                       = 13,
  KamikazeAndSomeCond                               = 14,
  ZeroPositionNoTwin                                = 15,
  TwoMummerCond                                     = 16,
  KingCapture                                       = 17,
  MonoAndBiChrom                                    = 18,
  WrongChar                                         = 19,
  OneKing                                           = 20,
  WrongSquareList                                   = 21,
  MissngSquareList                                  = 22,
  WrongRestart                                      = 23,
  MadrasiParaAndOthers                              = 24,
  RexCirceImmun                                     = 25,
  SomePiecesAndHeffa                                = 26,
  BigNumMoves                                       = 27,
  RoyalPWCRexCirce                                  = 28,
  ErrUndef                                          = 29,
  OffendingItem                                     = 30,
  InputError                                        = 31,
  WrongPieceName                                    = 32,
  PieSpecNotUniq                                    = 33,
  AnnanChessAndConditionRecoloringPieces            = 34,
  UnrecCondition                                    = 35,
  OptNotUniq                                        = 36,
  WrongInt                                          = 37,
  UnrecOption                                       = 38,
  ComNotUniq                                        = 39,
  ComNotKnown                                       = 40,
  NoStipulation                                     = 41,
  WrOpenError                                       = 42,
  RdOpenError                                       = 43,
  TooManyInputs                                     = 44,
  NoColourSpec                                      = 45,
  UnrecStip                                         = 46,
  CondNotUniq                                       = 47,
  EoFbeforeEoP                                      = 48,
  InpLineOverflow                                   = 49,
  NoBegOfProblem                                    = 50,
  InternalError                                     = 51,
  FinishProblem                                     = 52,
  Zugzwang                                          = 53,
  Threat                                            = 54,
  But                                               = 55,
  TimeString                                        = 56,
  MateInOne                                         = 57,
  NewLine                                           = 58,
  ImitWFairy                                        = 59,
  ManyImitators                                     = 60,
  KingKamikaze                                      = 61,
  PercentAndParrain                                 = 62,
  ProblemIgnored                                    = 63,
  SomeCondAndAntiCirce                              = 64,
  EinsteinAndFairyPieces                            = 65,
  SuperCirceAndOthers                               = 66,
  HashedPositions                                   = 67,
  ChameleonPiecesAndChess                           = 68,
  CheckingLevel1                                    = 69,
  CheckingLevel2                                    = 70,
  StipNotSupported                                  = 71,
  MultipleGoalsWithProofGameNotAcceptable           = 72,
  TooManyEpKeySquares                               = 73,
  Abort                                             = 74,
  TransmRoyalPieces                                 = 75,
  UnrecRotMirr                                      = 76,
  PieceOutside                                      = 77,
  ContinuedFirst                                    = 78,
  Refutation                                        = 79,
  NoFrischAufPromPiece                              = 80,
  ProofAndFairyConditions                           = 81,
  IsardamAndMadrasi                                 = 82,
  ChecklessUndecidable                              = 83,
  OverwritePiece                                    = 84,
  MarsCirceAndOthers                                = 85,
  PotentialMates                                    = 86,
  NonsenseCombination                               = 87,
  VogtlanderandIsardam                              = 88,
  AssassinandOthers                                 = 89,
  RepublicanandnotMate                              = 90,
  IncrementHashRateLevel                            = 91,
  DecrementHashRateLevel                            = 92,
  IntelligentRestricted                             = 93,
  NothingToRemove                                   = 94,
  NoMaxTime                                         = 95,
  NoStopOnShortSolutions                            = 96,
  SingleBoxAndFairyPieces                           = 97,
  UndefLatexPiece                                   = 98,
  HunterTypeLimitReached                            = 99,
  IncompatibleRoyalSettings                         = 100,
  LosingChessNotInCheckOrMateStipulations           = 101,
  TakeMakeAndFairy                                  = 102,
  MagicAndFairyPieces                               = 103,
  TooManyMagic                                      = 104,
  NoRepublicanWithConditionsDependingOnCheck        = 105,
  CantDecideWhoIsAtTheMove                          = 106,
  ThreatOptionAndExactStipulationIncompatible       = 107,
  GhostHauntedChessAndCirceKamikazeHaanIncompatible = 108,
  WhiteToPlayNotApplicable                          = 109,
  SetPlayNotApplicable                              = 110,
  PostKeyPlayNotApplicable                          = 111,
  QuodlibetNotApplicable                            = 112,
  GoalIsEndNotApplicable                            = 113,
  GoalIsEndAndQuodlibetIncompatible                 = 114,
  CantDecideOnSideWhichConditionAppliesTo           = 115,
  NoCageImmuneWithoutCage                           = 116,
  ExclusiveRefutedUndecidable                       = 117,
  KingCaptureDetected                               = 118,
  InconsistentRetroInformation                      = 119,
  InconsistentDuplexOption                          = 120,

  MsgCount /* THIS MUST BE THE LAST ENUMERATOR */
} message_id_t;

/* Initialise message for a language
 * @param language the language
 */
void output_message_initialise_language(Language language);

/* Retrieve the message for a specific id in the current language
 * @param id identifies the message to be retrieved
 * @return the message
 */
char const *output_message_get(message_id_t id);

#endif
