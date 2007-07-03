/**************************** MODIFICATIONS to pyio.c ****************************
**
** PlatzwechselCirce and Circe i/o-bug fixed.
** SpiegelImmunschach and Immunschach i/o-bug fixed.    28.05.90        NG
**
** Hypervolage and Volage i/o-bug fixed.
** Doppelrandzueger and Randzueger i/o-bug fixed.
** ChineseChess and Leofamily i/o-bug fixed.             4.06.90        NG
**
** Due to new stipulation "Ziel"
** ParseStip() and WritePosition() modified.            13.08.90        NG
**
** eingabe option solmenaces hinzugefuegt               13.8.1990      TLi
**
** WritePosition fuer Ausgabe Droheinschraenkung geaendert   27.8.90   TLi
**
** ParseStip() fuer semi-reflex- und patt-Forderungen ergaenzt
**                                                         27.8.90     TLi
**
** SemiRefMate and SemiRefStaleMate shortened because the first
** 7 characters must (!) be different.                   9.09.1990      NG
**
** Due to new condition ContactGridChess  ParseCond() and
** WritePosition() modified.                            17.11.1990      NG
**
** Ergaenzungen fuer ATARI ST - Eingabe.                06.01.1991      StHoe
**
** In function sncmp() useless curly braces removed     22.01.1991      TM
**
** Added function ReadImitators() and modified ParseCond ()
** for problems with imitators                          24.01.1991      TM
**
** Modified some terms who were entered in awful french 04.02.1991      TM
**
** function ParseFieldList() modified for kamikaze kings
** (possible in Circe rex incl.!)                       05.02.1991      TM
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** Removed the ifdef ATARI for OpenInput                29.07.91        ElB
**
** Rewrite of ParseStip. Necessary due to more
** stipulations.					29.07.91	ElB
**
** ParsStips():  DoubleStaleMate and Target bug fixed.  16.08.91        NG
**
** modifiziert:	ParseCond und WritePosition,
** neu: ParseRex fuer einlesen von RexInclusive		23.9.1991	TLi
** weiter modifiziert fuer ChamaeleonCirce, Circe Equipollents, FileCirce
** 		       	und Circe CouCou		23.9.1991	TLi
**
** ParseCond and WritePosition modified: new Condition  holes. ReadImitators and
** ReadMagicSquares sampled with ReadHoles to ReadSquares.  27.11.91        NG
**
** Sense of OptFlag[iprom] reversed, so with Imitators, promotion to
** an Imitator is always allowed, if not excluded by Option NoIProm.
**                                                      05.04.1992      NG
**
** WritePosition() modified for Chameleon and Jigger-pieces
**							 8. 4. 1993	TLi
** The call's to tolower, prefixed with test of isupper
**						        21. 5.1993	ElB
** OpenInput called with blank file name, due to a bug in the
** clib of 386BSD.
**							20. 5.1993	ElB
**
** 1993/5/25  ElB  Moved all remarks on modifications of pyio.c to
**                 pyio-c.mod. Look there for remarks before this date.
**
** 1993/07/02 StH  I added the input of maxsolutions in the 
**                 function ParseOpt.
**
** 1993/07/10 TLi  new option: solflights x - input of maxflights added
**                 in function ParseOpt() and output of the option in
**                 WritePosition().
**
** 1994/02/15 NG   New conditions:  ForcedSquares
**
** 1994/04/29 TLi  New implementation of forced and magic squares
**
** 1995/04/26 TLi  new stipulation: seriesmovers with introductory move
**
** 1995/04/26 TLi  new stipulation: #/= etc
**
** 1995/05/05 ElB  better board output in WritePosition()
**
** 1995/06/08 TLi  new option: nontrivial n m for direct/self problems
**
** 1995/06/26 TLi  new stipulation: ##, ##! (double mate, counter mate)
**
** 1995/06/30 TLi  neutral pieces are considered to be black and white
**                 at some stages -- ParseFieldList adopted
** 1995/07/19 TLi  new stipulation: diagram == proof game
**
** 1995/08/08 TLi  new stipulation: 00 == castling
**
** 1995/08/18 TLi  new option: enpassant for ep keys
**                 function ReadSquares() and ParseOpt() modified.
**
** 1995/08/18 TLi  new conditions: direct reflex and wh/bl must check
**
** 1995/10/23 NG   Output of MaxMemory (MMString) to TraceFile
**
** 1995/12/28 TLi  new anticirce implementation
**
** 1995/12/28 TLi  didn't succeed in implementing SchachZwang and
**                 directreflex -- deleted both
**
** 1996/02/07 NG   new option: halfduplex, ie. stipulation is only
**                             calculated for the 'opponent' side.
**
** 1995/03/19 TLi  new: twinning supported
**
** 1995/04/23 TLi  new option: postkeyplay
**
** 1993/5/25  ElB  Moved all remarks on modifications of pyio.c to
**                 pyio-c.mod. Look there for remarks before this date.
**
** 1993/07/02 StH  I added the input of maxsolutions in the 
**                 function ParseOpt.
**
** 1993/07/10 TLi  new option: solflights x - input of maxflights added
**                 in function ParseOpt() and output of the option in
**                 WritePosition().
**
** 1994/02/15 NG   New conditions:  ForcedSquares
**
** 1994/04/29 TLi  New implementation of forced and magic squares
**
** 1995/04/26 TLi  new stipulation: seriesmovers with introductory move
**
** 1995/04/26 TLi  new stipulation: #/= etc
**
** 1995/05/05 ElB  better board output in WritePosition()
**
** 1995/06/08 TLi  new option: nontrivial n m for direct/self problems
**
** 1995/06/26 TLi  new stipulation: ##, ##! (double mate, counter mate)
**
** 1995/06/30 TLi  neutral pieces are considered to be black and white
**                 at some stages -- ParseFieldList adopted
** 1995/07/19 TLi  new stipulation: diagram == proof game
**
** 1995/08/08 TLi  new stipulation: 00 == castling
**
** 1995/08/18 TLi  new option: enpassant for ep keys
**                 function ReadSquares() and ParseOpt() modified.
**
** 1995/08/18 TLi  new conditions: direct reflex and wh/bl must check
**
** 1995/10/23 NG   Output of MaxMemory (MMString) to TraceFile
**
** 1995/12/28 TLi  new anticirce implementation
**
** 1995/12/28 TLi  didn't succeed in implementing SchachZwang and
**                 directreflex -- deleted both
**
** 1996/02/07 NG   new option: halfduplex, ie. stipulation is only
**                             calculated for the 'opponent' side.
**
** 1995/03/19 TLi  new: twinning supported
**
** 1995/04/23 TLi  new option: postkeyplay
**
** 1996/06/10 NG   new stipulation: ser-dia (SeriesProofGame)
**
** 1996/07/25 NG   bug fix in ReadSquares
**
** 1996/10/31 NG   New condition:  VerticalMirrorCirce
**
** 1996/11/15 NG   New option:  PromOnly
**
** 1996/12/07 TLi  now: seriesmovers with more than 1 introductory move
**
** 1997/1/4   TLi  output of maximummer conditions in WriteCondition
**                 improved (was not centered!)
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/02/10 TLi  new condition: DiametralCirce
**
** 1997/02/17 NG   Option  PromOnly  changed to Condition  Promonly
**
** 1997/03/10 TLi  generalized Chameleon Circe
**
** 1997/03/11 TLi  new condition: RankCirce
**
** 1997/03/24 TLi  ==-bug in ParsStip fixed
**
** 1997/06/04 SE/TLi  new condition: MarsCirce
**
** 1997/07/25 TLi  ParseStips modified due to difficulties with my
**                 TUBRO-C 1.00 compiler, but the code was buggy anyway.
**
** 1997/08/11 NG   new condition: PhantomChess
**
** 1997/08/16 NG   conditions  transmuted/reflected kings
**                 additionally splitted into 
**                 white/black transmuted/reflected kings
**
** 1997/08/18 NG   ##!-bug in ParsStip fixed
**                 Twinning bugs fixed (due to new LaTeX implementation)
**
** 1997/09/04 TLi  new conditions: AntiEinsteinSchach, MirrorCouscousCirce
**
** 1997/10/02 TLi  new condition: Black/WhiteRoyalSquare
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**
** 1997/10/31 SE   new condition: Isardam TypeB
**
** 1997/11/05 SE   new condition: Sentinelles (en) PionAdvers
**                 new routine ParseVariant to cope with above two
**                 and allow scope for other variants
**                 AntiCirce Cheylan and Calvet done as variants
**
** 1997/11/09 SE   new condition: Sentinelles (en) PionNeutre
**
** 1997/11/20 SE   new condition: Patience chess, two types
**
** 1997/11/24 SE   new keyword: InitialGameArray
**
** 1997/12/12 SE   new stipulations: stip_circuit, stip_exchange
**                                   (& rebirth types)
**
** 1998/01/20 ElB  Added new keyword: LaTeXpieces. With this command
**                 you can predefine the contents of LaTeXPiecesAbbr
**                 and LaTeXPiecesFull. The old syntax of LaTeX is
**                 not changed despite that both arrays are NOT cleared
**                 by the LaTeX keyword. Especially useful with the
**                 files pie-lang.dat.
**
** 1998/05/02 NG   ActTwin buffer overflow fixed. Showed no output
**                 after a lot of successive twins on SUN machines.
**
** 1998/06/05 NG   new option: MaxTime
**
** 1998/06/10 NG   Twinning of multiple conditions: i/o bug fixed
**
** 1998/07/26 TLi  function CenterLine modified to make things REALLY
**		   centered...
**
** 1998/08/21 TLi  new condition: ActuatedRevolvingBoard
**
** 1998/09/17 TLi  option intelligent extended: maximum number of
**                 solutions per mating position can be given
**
** 1998/09/21 TLi  twinning bug fixed: removal of not existing piece
**                                     resulted in a coredump
**
** 1999/01/17 NG   option MaxTime improved
**
** 1999/03/15 TLi  new condition: MessignyChess (RexInclusive)
**
** 1999/03/22 NG   LaTeX I/O bugfix: LaTeXStdPie[8] definition changed.
**
** 1999/03/29 TLi  bug in ParseLaTeXPieces fixed
**                 Some #ifdef NODEF stuff deleted
**                 LaTeX output for German improved
**
** 1999/04/02 NG   New option:  NoCastling
**
** 1999/04/17 TLi  new condition: (Bi)Woozles/Heffalumps
**                 new rex exclusiv (for Woozles, Heffalumps and Messigny Chess)
**
** 1999/05/14 NG   BiHeffalumps bug fixed 
**
** 1999/05/25 NG   Moved all remarks on modifications of pyio.c to
**                 pyio-c.mod. Look there for remarks before 1998/01/01.
**
** 1999/06/22 NG   new conditions: WhitePromSquares  and  BlackPromSquares
**                 new conditions: NoWhitePromotion  and  NoBlackPromotion
**                 modification: PromOnly needs now at least one piece
**
** 1999/08/27 TLi  bug in ParseLaTeXPieces fixed
**
** 1999/09/27 TLi  new condition: EiffelChess
**
** 1999/10/08 SE   new option : Quodlibet
**                 in self- and (semi-)reflex play, allows White to make contract by
**                 direct stip also (eg. #/S# or #/R#).
**
** 1999/10/08 SE   new stipulation: #= mate or stalemate
**
** 1999/16/08 SE   new Sentinelles variants - max white pawns and max black pawns
**
** 2000/01/09 SE   new quasi-condition: maxi (black or white depending on maincamp
**                 new condition: Ultraschachzwang (Bl, W, and quasi versions)
**
** 2000/01/19 SE   new condition : parasentinelles
**
** 2000/04/11 NG   empty  remark  bug fixed
**
** 2000/04/14 TLi  input for options threat and nontrivial changed to
**                 allow input of zero.
**                 functions ParseOpt() and WritePosition() modified
**
** 2000/06/06 TLi  bug fix in ParseTwinRemove
**
** 2000/09/19 TLi  bug fix in ParseTwinMove
**
** 2000/11/26 ThM,NG   Function Read????Pieces generalized ...
**
** 2001/01/14 SE  new condititon: EchecsAlphabetics
**		  new stipulation: any (suggestions for symbol please!)
**		      (I suggest:  "~"	NG)
**
** 2001/02/04 SE   bug fix with condition ~
**
** 2001/05/08 SE   new conditions: CirceTurnCoats, CirceDoubleAgents, AMU,
**		   SentBerolina.   New input format: Forsyth Notation
**
** 2001/05/20 SE   new stipulation: atob
**
** 2001/11/10 NG   singleboxtype? changed to singlebox type?
**                 some singlebox variable names changed ...
**
** 2002/04/29 FCO  LaTeX changes
**
** 2003/05/18 NG   new option: beep    (if solution encountered)
**
** 2004/02/06 SE   New conditions : Oscillating Kings (invented A.Bell)
**                                  Ks swapped after each W &/or Bl move;
**				    TypeB can't self-check before swap
**                                  Koeko Neighbourhood (invented S.Emmerson)
**
** 2004/03/05 SE   New condition : Antimars (and variants) (invented S.Emmerson)
**                 Pieces reborn to move, capture normally
**
** 2004/02/09 SE   Bugfix : Forsyth with neutrals
**
** 2004/04/23 SE   Oscillating Ks TypeC, also allowed A/B/C different for white/black
**
** 2004/05/01 SE   Bugfix : ParseVariant problem e.g. sentinelles pionneutre koeko
**
** 2004/07/19 NG   New condition: SwappingKings
**
** 2005/04/20 NG   assert "eliminated". Check of Hunter0+maxnrhuntertypes added.
**
** 2005/04/25 NG   bugfix: a=>b with Imitators
**
**************************** INFORMATION END **************************/
