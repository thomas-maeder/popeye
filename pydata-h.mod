/*************************** MODIFICATIONS pydata.h *****************************
**
** flag  solsermat  for series-mate added.              15. 7.1989      NG
**
** flag  solserpat  for series-stalemate added.         17. 7.1989      NG
**
** flag  flkoeko  for koeko-chess added.                18. 7.1989      NG
**
** flag  flappseul for only(!) setplay in helpplay      23. 7.1989      NG
**
** new definition of boolnoedge for  Randzueger  and
** Doppel-Randzueger !                                  12.10.1989      NG
**
** flags flrandzueger and fldoppelrandzueger            12.10.1989      NG
**
** flag  flduplex for calculation of duplex problems    26.11.1989      NG
**
** flag  flpwc and pwcprom added for calculation
** of PWC  problems                                     18.02.1990      NG
**
** flags flohneschlag and flimmun                       17.03.1990      NG
**
** additional board (e_ubi) for "Ubi-Ubi" calculations. 24.03.1990      NG
**
** new option: sansrn   no black king allowed !          3.06.1990      NG
**
** new datafield mixhopdata[][]
** for the last three of the following
** four new pieces: hamster, eagle, moose and sparrow    4.07.1990      NG
**
** Three new pieces: archbishop, reflecting bishop and
**                   cardinal                            8.07.1990      NG
**
** Two new pieces: nightrider-hopper, dummy-piece       16.07.1990      NG
**
** Three new pieces: gnu-, camel-, zebra-rider          17.07.1990      NG
**
** Four new pieces: gnu-, camel-, zebra-rider-hopper
** and DiagonalSpiralSpringer                           20.07.1990      NG
**
** New piece: bouncy knight                             23.07.1990      NG
**
** Some definitions which are never used removed.       13.08.1990      NG
**
** Some new definitions due to new stipulation "Ziel"   13.08.1990      NG
**
** neue option: solmenaces (zugehoerig: smallint droh)  13.8.1990       TLi
**
** smallint zugebene, maxdefen hinzugefuegt             14.8.1990       TLi
**
** boolean SemiFlag, PattFlag hinzugefuegt fuer
** semi-reflex- und patt-forderungen                    27.8.1990       TLi
**
** due to some speed and code optimizations
** initializations of constants included.               13.10.1990      NG
**
** flags fl1prblanc and fl1prnoir added for
** firstpromotionpiece initialisation.                  18. 7.1989      NG
**
** new condition: contactgridchess                      17.11.90        NG
**
** added some data structures for imitators             21.01.91        TM
**
** some french comments modified                        04.02.91        TM
**
** added crenkam (rebirth square for taking kamikaze
** pieces) and jouekamikaze (testing if a move is done
** by a kamikaze piece isn't just testing if its arrival
** arrival square is empty:
** it may have promoted to an imitator, or in circe its
** or the taken piece's rebirth square may be the
** the move's arrival square                            04.02.91        TM
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** new flag empilegenre.                                11.06.91        NG
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** Added new variable Stipulation			29.07.91	ElB
**
** SemiFlag,flserie removed, both are now macros	29.07.91	ElB
**
** PattFlag stays, new is DblPattFlag			29.07.91	ElB
**
** ueberfluessig - geloescht:    DblPattFlag,
** en_eche?, flmat, listedecoups, tabaide, FlDuplex
** neu: flende,echeccgenre				18.9.1991	TLi
** neu: jouegenre - flaggenre nur noch lokal in verifieposition benoetigt
** neuer Flag: flagpatrsout (ersetzt CondFlag[patrsoutien])	20.9.1991	TLi
** neu:	rex_mad und rex_cir_imm				20.9.1991	TLi
** rex_cir_imm -> rex_circe + rex_immun
** neu:  eval_white, eval_black
** echeccgenre nicht mehr benoetigt			22.9.1991	TLi
** flagpatrsout ueberhaupt nicht benoetigt		25.9.1991	TLi
** new: stipulation					15.11.1991
**
** new: move_diff_code[] for efficient calculation of
** move differences from square to square.              23.11.91        NG
**
** due to letter of TLi and rx and rz bug, stipcamp eliminated.
**                                                      14.02.1992      NG
**
** checkfunctions fuer libre um Orthos erweitert	18.2.1992	TLi
**
** neu: ch????[], totalortho fuer schnelleres Generieren in der letzten
** Zugebene bei ortho-Mattaufgaben			20.2.1992	TLi
**
** Sense of OptFlag[iprom] reversed, so with Imitators, promotion to
** an Imitator is always allowed, if not excluded by Option NoIProm.
** New: testdebut for efficient checking in testempile() due to hint of TLi.
** Changed: Piece names checked for consistency and changed if necessary.
**                                                      05.04.1992      NG
**
** New cpp constant: VERSION should be set in py.h or Makefile.
**							12Apr1992	ElB
**
** New pieces: ErlKing, BoyScout, GirlScout		01.05.1993	NG
**
** 1993/5/25  ElB  Moved all remarks on modifications of pydata.h to
**                 pydata-h.mod. See there for remarks before this date.
**
** 1993/6/20  TM   Imitator with many fairy-pieces.
**
** 1993/07/02 StH  I added the variables: solutions, maxsolutions  
**                 to be able to stop solving when a given number 
**                 of solutions was reached. 
**                 Also new optionstrings for option maxsols.
**
** 1993/07/10 TLi  storing and reseting of the initial position modified
**                 to save memory, therefore declaration of sic_... mod.
**
** 1993/07/10 TLi  new option: maxflightsquares x
**
** 1993/07/10 TLi  new pieces: Skylla and Charybdis
**
** 1994/02/15 NG   New conditions:  ForcedSquares
**
** 1994/03/18 TLi  bug fixed?
**
** 1994/04/29 TLi  new implementation of ForcedSquares
**
** 1994/05/06 NG   abbreviations for knight (kn) and sparrow (s)
**                 changed to knight (s) and sparrow (sw).
**
** 1994/07/28 TLi  new piece: contra grashopper
**
** 1994/07/29 TLi  new structure ep2[..] for ep-capture in Einstein chess
**
** 1994/08/10 TLi  declaration of exist[] had to be changed due to
**                 problems with BorlandC3.1
**
** 1994/12/14 TLi  new pieces: RoseLion, RoseHopper
**
** 1995/01/10 NG   New: e_ubi_mad due to UbiUbi bug with Madrasi.
**
** 1995/04/26 TLi  new stipulation: h#/= etc.
**
** 1995/05/26 TLi  new conditions: Chameleon Chess and Beamten Chess
**
** 1995/06/08 TLi  new option: nontrivial n m for direct/self problems
**
** 1995/06/12 TLi  new option: keep mating piece
**
** 1995/06/14 TLi  new piece: Okapi = Knight + Zebra
**
** 1995/06/26 TLi  new stipulation: ##, ##! (double mate, counter mate)
**
** 1995/06/29 TLi  further attempt to optimize orthodox play
**
** 1995/07/19 TLi  new piece: 3:7-leaper
**
** 1995/07/19 TLi  new piece: grasshopper-2
**
** 1995/08/03 TLi  new piece: grasshopper-3
**
** 1995/08/04 NG   Castling in totalortho play implemented.
**
** 1995/08/16 TLi  new stipulation: dia == proof game
**
** 1995/08/18 TLi  new option: enpassant for ep keys
**
** 1995/08/18 TLi  new conditions: SchachZwang, directreflex
**
** 1995/09/28 NG   (Non)ReciAlphaEnd increased to 5 (Bug !).
**
** 1995/11/02 NG   new piece: 1:6-leaper
**
** 1995/11/03 NG   new option: noboard - do not print the board to screen or file.
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/28 TLi  old orthodox optimization stuff and switch OPTIMIZE
**                 deleted
**
** 1995/12/28 TLi  new anticirce implementation
**
** 1995/12/28 TLi  didn't succeed in implementing SchachZwang and
**                 directreflex -- deleted both
**
** 1995/12/29 TLi  new condition: GlasgowChess
**
** 1995/12/30 TLi  new option: noshort == no short variations
**
** 1996/01/01 NG   Moved all remarks on modifications of pydata.h to
**                 pydata-h.mod. Look there for remarks before this date.
**
** 1996/02/07 NG   new option: halfduplex, ie. stipulation is only
**                             calculated for the 'opponent' side.
**
** 1996/04/23 TLi  new option: postkeyplay
**
** 1996/10/31 NG   new condition: VerticalMirrorCirce
**
** 1996/06/11 TLi  new pieces: 2:4-leaper 3:5-leaper
**
** 1996/08/11 NG   3:5-leaper bug fixed (vec[] was wrong)
**
** 1996/11/15 NG   New option:  PromOnly
**
** 1996/12/07 TLi  new: seriesmovers with more than 1 introductory move
**
** 1996/12    SE   new pieces: double grashopper and kinghopper
**
** 1996/12    SE/TLi  new condition: isardam, ohneschach
**
** 1997/02/06 NG   new piece: orix
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
** 1997/03/14 TLi  new option: intelligent
**
** 1997/04/22 TLi  ExclusiveChess
**
** 1997/06/04 SE/TLi  new condition: MarsCirce
**
** 1997/06/27 NG   new pieces: 1:5-leaper, 2:5-leaper
**                             gral (alfil + rook-hopper)
**
** 1997/07/28 TLi  new condition: MarsMirrorCirce
**
** 1997/07/28 TLi  new: LaTeX output, award
**
** 1997/08/11 NG   new condition: PhantomChess
**
** 1997/08/16 NG   conditions  transmuted/reflected kings
**                 additionally splitted into 
**                 white/black transmuted/reflected kings
**
** 1997/09/04 TLi  new conditions: AntiEinsteinSchach, CousCousMirrorCirce
**
** 1997/10/02 TLi  new condition: Black/WhiteRoyalSquare
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, BrunnerChess
**
** 1997/10/30 SE   new condition: Isardam TypeB
**
** 1997/11/05 SE   new condition: Sentinelles (en) PionAdvers
**                 AntiCirce variants modified
**
** 1997/11/09 SE   new condition: Sentinelles (en) PionNeutre
**
** 1997/12/12 SE   new conditions: Extinction Chess, Echecs Central
**
** 1997/12/12 SE   new stipulations: stip_circuit, stip_exchange 
**                                   (& rebirth types)
**
** 1997/12/30 SE   new condition : Republican Chess
**
** 1998/06/05 NG   new option: MaxTime
**
** 1998/07/26 TLi  new piece specification: beamtet
**
** 1998/07/27 TLi  new pieces: rook/bishop/night locust
**
** 1998/08/21 TLi  new condition: Actuated Revolving Board
**
** 1998/09/17 TLi  option intelligent extended: maximal number of
**                 solutions per mating position can be given
**
** 1999/01/17 NG   option MaxTime improved
**
** 1999/02/19 NG   new pieces: wazir-/fers-rider (think at circe rebirth !)
**                 definitions of queen, rook and knight changed 
**
** 1999/03/15 TLi  new condition: MessignyChess (RexInclusive)
**
** 1999/04/02 NG   New option:  NoCastling
**
** 1999/04/17 TLi  new condition: (Bi)Woozles/Heffalumps
**                 new rex exclusiv (for Woozles, Heffalumps and Messigny Chess)
**
** 1999/05/25 NG   Moved all remarks on modifications of pydata.h to
**                 pydata-h.mod. Look there for remarks before 1998/01/01.
**
** 1999/06/12 SE   new piece specification : halfneutrals
**
** 1999/06/22 NG   new conditions: WhitePromSquares  and  BlackPromSquares
**                 new conditions: NoWhitePromotion  and  NoBlackPromotion
**
** 1999/09/24 NG   bug fix: RoisTransmutes and RoisReflecteurs entries exchanged
**
** 1999/08/30 TLi  new condition: Eiffelchess
**
** 1999/10/07 NG   mao  to  vao  bugfix due to changed orthodox defines.
**
** 1999/11/08 NG   New Flags: FlagMaxSolsReached, FlagShortSolsReached
**                 New option:  StopOnShortSolutions
**
** 1999/11/21 TLi  new piece: Bison
**
** 1999/11/22 NG   keyword changes:
**                 german: RexExklusive, RexInklusive, ExklusivSchach
**                 english: RelegationChess
**
** 2000/01/04 NG   Allocatable memory on 32-Bit systems increased upto 4 GB
**
** 2000/01/09 SE   new quasi-condition: maxi (black or white depending on maincamp
**                 new condition: Ultraschachzwang (Bl, W, and quasi versions)
**                 new conditions: ActuatedRevolvingCentre, ShieldedKings
**
** 2000/01/13 TLi  new piece: Elephant
**
** 2000/01/28 TLi  new piece: Nao (Chinese Nightrider)
**
** 2000/02/02 NG   testcastling "globalized".
**
** 2000/04/12 NG   new pieces: (rook/bishop)-(eagle/moose/sparrow)
**
** 2000/05/09 NG   new piece: Rao (Chinese Rose)
**
** 2000/05/26 NG   new piece: scorpion   new condition: StingChess
**                 german name of ExtinctionChess: AusrottungsSchach
**
** 2000/09/19 TLi  new condition: LineChameleonChess
**
** 2000/10/23 NG   new conditions: NoWhiteCapture, NoBlackCapture
**
** 2000/10/26 TLi  new piece: Marguerite (MG)
**
** 2000/11/29 TLi  new pieces: 3:6 leaper, night-rider lion, mao-rider lion
**
**************************** INFORMATION END ***************************/
