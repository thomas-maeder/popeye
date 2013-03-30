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
  WormholesEPKey                                    = 15,
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
  BlackColor                                        = 26,
  WhiteColor                                        = 27,
  SomePiecesAndHeffa                                = 28,
  BigNumMoves                                       = 29,
  RoyalPWCRexCirce                                  = 30,
  ErrUndef                                          = 31,
  OffendingItem                                     = 32,
  InputError                                        = 33,
  WrongPieceName                                    = 34,
  PieSpecNotUniq                                    = 35,
  WBNAllowed                                        = 36,
  UnrecCondition                                    = 37,
  OptNotUniq                                        = 38,
  WrongInt                                          = 39,
  UnrecOption                                       = 40,
  ComNotUniq                                        = 41,
  ComNotKnown                                       = 42,
  NoStipulation                                     = 43,
  WrOpenError                                       = 44,
  RdOpenError                                       = 45,
  TooManyInputs                                     = 46,
  NoColorSpec                                       = 47,
  UnrecStip                                         = 48,
  CondNotUniq                                       = 49,
  EoFbeforeEoP                                      = 50,
  InpLineOverflow                                   = 51,
  NoBegOfProblem                                    = 52,
  InternalError                                     = 53,
  FinishProblem                                     = 54,
  Zugzwang                                          = 55,
  Threat                                            = 56,
  But                                               = 57,
  TimeString                                        = 58,
  MateInOne                                         = 59,
  NewLine                                           = 60,
  ImitWFairy                                        = 61,
  ManyImitators                                     = 62,
  KingKamikaze                                      = 63,
  PercentAndParrain                                 = 64,
  ProblemIgnored                                    = 65,
  SomeCondAndAntiCirce                              = 66,
  EinsteinAndFairyPieces                            = 67,
  SuperCirceAndOthers                               = 68,
  HashedPositions                                   = 69,
  ChameleonPiecesAndChess                           = 70,
  CheckingLevel1                                    = 71,
  CheckingLevel2                                    = 72,
  StipNotSupported                                  = 73,
  MultipleGoalsWithProofGameNotAcceptable           = 74,
  ToManyEpKeySquares                                = 75,
  Abort                                             = 76,
  TransmRoyalPieces                                 = 77,
  UnrecRotMirr                                      = 78,
  PieceOutside                                      = 79,
  ContinuedFirst                                    = 80,
  Refutation                                        = 81,
  NoFrischAufPromPiece                              = 82,
  ProofAndFairyConditions                           = 83,
  IsardamAndMadrasi                                 = 84,
  ChecklessUndecidable                              = 85,
  OverwritePiece                                    = 86,
  MarsCirceAndOthers                                = 87,
  PotentialMates                                    = 88,
  NonsenseCombination                               = 89,
  VogtlanderandIsardam                              = 90,
  AssassinandOthers                                 = 91,
  RepublicanandnotMate                              = 92,
  IncrementHashRateLevel                            = 93,
  DecrementHashRateLevel                            = 94,
  IntelligentRestricted                             = 95,
  NothingToRemove                                   = 96,
  NoMaxTime                                         = 97,
  NoStopOnShortSolutions                            = 98,
  SingleBoxAndFairyPieces                           = 99,
  UndefLatexPiece                                   = 100,
  HunterTypeLimitReached                            = 101,
  IncompatibleRoyalSettings                         = 102,
  LosingChessNotInCheckOrMateStipulations           = 103,
  TakeMakeAndFairy                                  = 104,
  MagicAndFairyPieces                               = 105,
  TooManyMagic                                      = 106,
  NoRepublicanWithConditionsDependingOnCheck        = 107,
  CantDecideWhoIsAtTheMove                          = 108,
  ThreatOptionAndExactStipulationIncompatible       = 109,
  GhostHauntedChessAndCirceKamikazeHaanIncompatible = 110,
  WhiteToPlayNotApplicable                          = 111,
  SetPlayNotApplicable                              = 112,
  PostKeyPlayNotApplicable                          = 113,
  QuodlibetNotApplicable                            = 114,
  GoalIsEndNotApplicable                            = 115,
  GoalIsEndAndQuodlibetIncompatible                 = 116,
  CantDecideOnSideWhichConditionAppliesTo           = 117,
  NoCageImmuneWithoutCage                           = 118,
  ExclusiveRefutedUndecidable                       = 119,

  MsgCount /* THIS MUST BE THE LAST ENUMERATOR */
} message_id_t;

boolean InitMsgTab(Language l);

char const *GetMsgString (message_id_t id);

void VerifieMsg(message_id_t id);
void ErrorMsg(message_id_t id);
void Message(message_id_t id);
void FtlMsg(message_id_t id);

#endif  /* PYMSG_H */
