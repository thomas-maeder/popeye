/**************************** MODIFICATIONS to py5.c *****************************
**
** A great 'kamikaze'-bug fixed	in jouecoup():
** pawn-promotion is now (!!) possible !		 8. 8.1989	NG
**
** source better readable: defines used	!		12.10.1989	NG
**
** a great patrol-chess	bug fixed.
** (it just didn't work	correct	!).
** therefore function gennoir()	modified.		17.10.1989	NG
**
** a leofamily-bug fixed.
** promotion into knight was possible !			 9.12.1989	NG
**
** a leofamily-bug fixed.
** circe malefique didn't work correct !		 6.01.1990	NG
**
** a cavalier majeur bug fixed !
** promotion into knight was possible !			 6.01.1990	NG
**
** renaicirce modified for PWC.
** jouecoup and	repcoup	modified for PWC.
** Pawn	and Berolina-Pawn modified for PWC.		17.02.1990	NG
**
** an output-bug with option PWC fixed.
** (orthodox pawn-promotion: piece was not visible)	 5.03.1990	NG
**
** due to correction of function empile() in py4.c
** (i == j)-tests in jouecoup() and repcoup() are
** deleted.                                             30.05.1990      NG
**
** a "royal locust"-bug fixed.
** (it lost its royal attribut while moving !)          28.07.1990      NG
**
** Some new functions due to new stipulation "Ziel"     13.08.1990      NG
**
** Output of calculated move with movenumber            19.08.1990      NG
**
** geaendert und in py7.c uebernommen:
** inverse(), definverse(), mate(), reflexe(), matant()   28.8.1990   TLi
**
** changed: stipe(), stipant()                           7.09.1990      NG
**
** all goto's deleted.                                  17.10.1990      NG
**
** functions genrn (), genpn(), gencn(), jouecoup() and
** repcoup() modified for problems with imitators       16.01.1991      TM
**
** gennoir(), jouecoup() and repcoup() modified for circe
** problems with kamikaze pieces                        04.02.1991      TM
**
** made compilable with MPW on macintosh-computers.     14.02.1991      SB
**
** StartMoveNumber bug fixed.                           27.02.1991      NG
**
** contactgridchess: rule-bug in jouecoup() fixed.      17.03.1991      NG
**
** we do not promote to dummy-pieces !!                 29.03.1991      NG
**
** promotion bug of imitator fixed !                    22.05.1991      NG
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** gennoir modifiziert (kamikaze)
** neu: genschwarz, pattencore
** umgekraempelt: patt, gennoir
** stipe/stipant geloescht
** defreflexe nach PY7.C verlagert			18.9.1991	TLi
** jouegenre statt flaggenre in jouecoup und repcoup ausserdem einige
** Begingungen verschoben - schneller wegen vorgeschalteten jouegenre
**							20.9.1991	TLi
** jouecoup() fuer ChamaeleonCirce geaendert
** neu: function champiece				23.9.1991	TLi
** jouecoup() und renaicirce() modifiziert fuer coucou und equipollents
**							23.9.1991	TLi
** due to letter of TLi and rx and rz bug, stipcamp eliminated.
**                                                      14.2.1992       NG
**
** champiece() extended: LeoFamily and CavalierMajeur   22.5.1992       NG
**
** leichter Kamikaze-Bug in repcoup():
** 		nbpiece wurde fehlerhaft inkrementiert
**
** jouecoup fuer neutr. KK modifiziert - wenn renaicirce entsprechend
** 		modifiziert wird sind auch koenigliche Steine in
**		RexCirce moeglich			4. 10. 1992   TLi
**
** modifications for Einstein-Chess:
** genpb, jouecoup and new functions dec_/inc_einstein.
** jouecoup and repcoup modified for better structur.
** jouecoup modified for Circe Clone			9. 4. 1992	TLi
**
** ESC-clause moved to genmove() due to ser-... bug	1.05.1993	NG
**
** Fixed a bug due to missing reset of ep-Flag
**							21.5.1993	TLi
** 1993/5/25  ElB  Moved all remarks on modifications of py5.c to
**                 py5-c.mod. Look there for remarks before this date.
**
** 1993/6/30  NG   GCC with OS/2 or DOS  ported.
**
** 1993/7/1   TLI  PWC and neutral pieces bug fixed in jouecoup.
**
** 1994/04/27 TLi  Orphan bug fixed in genpn()
**
** 1994/04/29 TLi  new implementation of ForcedSquares and magic squares
**
** 1994/06/03 TLi  CirceClone bug spotted by Brian Stephenson fixed
**                 jouecoup() modified, new function is_pawn() added.
**                 Subsequently the latter one should be transfered to
**                 py6.c and be used instead of TESTFLAG(..., APawn)
**
** 1994/07/23 TLi  SuperCirce bug fixed in repcoup()
**
** 1994/07/29 TLi  New implementation of the ep capture in jouecoup()
**
** 1994/07/29 TLi  genpn moved to py4.c
**
** 1994/11/14 TLi  AntiCirce/Kamikaze bug fixed in stip_target
**
** 1995/05/23 NG   Due to hint of TLi stip_ep changed and rearranged
**
** 1995/05/26 TLi  new condition: Chameleon Chess
**
** 1995/06/01 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/06/26 TLi  new stipulation ##, ##! (double mate, counter mate)
**
** 1995/07/02 TLi  jouecoup() modified (change_moving_piece conditions)
**
** 1995/08/04 TLi  bug in stip_doublemate fixed
**
** 1995/08/04 NG   Castling in totalortho play implemented.
**
** 1995/08/07 TLi  new stipulation: 00 -- castling
**
** 1995/08/09 NG   Castling for a lot of conditions implemented.
**
** 1995/08/18 TLi  Einstein ep-bug fixed in jouecoup()
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/29 TLi  new condition: GlasgowChess
**
** 1996/01/01 NG   Moved all remarks on modifications of py5.c to
**                 py5-c.mod. Look there for remarks before this date.
**
** 1996/10/31 NG   new condition: VerticalMirrorCirce
**
** 1996/12/02 TLi  circe parrain promotion bug fixed
**
** 1996/12/19 NG   Einstein castling bug fixed
**
** 1996/12    SE/TLi  new condition: isardam, ohneschach
**
** 1997/02/05 TLi  killer bug fixed in patt()
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/02/10 TLi  new condition: DiametralCirce
**
** 1997/02/17 TLi  adjustments for chamaeleon pawns
**
** 1997/02/27 TLi  Madrasi ep-bug fixed
**
** 1997/03/10 TLi  generalized Chameleon Circe
**
** 1997/03/11 TLi  new condition: RankCirce
**
** 1997/04/22 TLi  exclusive chess
**
** 1997/06/04 SE/TLi  new condition: MarsCirce
**
** 1997/08/11 NG   new condition: PhantomChess
**
** 1997/08/16 NG   conditions  transmuted/reflected kings
**                 additionally splitted into 
**                 white/black transmuted/reflected kings
**
** 1997/09/04 TLi  new condition: AntiEinsteinSchach
**
** 1997/10/02 TLi  new condition: Black/WhiteRoyalSquare
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**                 new stipulation: autostalemate (symbol: !=)
**
** 1997/11/03 NG   initialisation bug fixed in jouecoup (due to CirceAssassin)
**
** 1997/11/05 SE   new condition: Sentinelles (en) PionAdvers
**
** 1997/11/09 SE   new condition: Sentinelles (en) PionNeutre
**
** 1997/11/10 SE   Mars Circe / Neutral pieces bug fixed
**		   Auto & Dbl. Stalemates modified for paralysing pieces
**
** 1997/11/11 SE   Neutral K / ## bug fixed
**
** 1997/11/20 SE   new condition: Patience chess, two types
**					 patches (can't be re-occupied)
**					 holes (TypeB) (can't be re-occupied or traversed)
**
** 1997/12/12 SE   new condition: Extinction Chess
**
** 1997/12/12 SE   new stipulations: stip_circuit, 
**                                  _exchange (& rebirth types)
**
** 1997/12/30 SE   new condition : Republican Chess
**                 Supercirce bug in repcoup fixed
**                 New implementation of pattencore allows nested calls to patt
**
** 1998/01/08 SE   checks in Isardam and Brunner now computed for output
**
** 1998/05/16 NG   ep-capture with phantomchess bug fixed
**
** 1998/05/17 SE   new fix for Supercirce bug above
**
** 1998/06/10 NG   another ep-capture with phantomchess bug fixed
**                 AntiAndernachChess with castling bug fixed
**
** 1998/07/25 SE   Fix for Republican chess due to rule clarification
**                 Hack added to print linesolutions for republican chess
**                 correctly
**                 All stips allowed with republican - no guarantees!
**
** 1998/07/26 TLi  patt() modified to stabilize checkless chess with
**		   neutral pieces
**
** 1998/08/21 TLi  new condition: ActuatedRevolvingBoard
**
** 1999/03/15 TLi  new condition: MessignyChess (RexInclusive)
**
** 1999/03/29 TLi  castling implementation changed
**
** 1999/05/04 TLi  orphan+transking bug fixed in genrn
**
** 1999/05/25 NG   Moved all remarks on modifications of py5.c to
**                 py5-c.mod. Look there for remarks before 1998/01/01.
**
** 1999/06/12 SE   new piece specification : halfneutrals
**
** 1999/09/27 TLi new condition: EiffelChess
**
** 1999/10/07 NG   mao  to  vao  bugfix due to changed orthodox defines.
**
** 1999/10/08 SE   new stipulation: #= mate or stalemate
**
** 1999/10/08 SE   Andernach/AntiAndernach specific castling implemented
**
** 1999/10/08 SE   new Sentinelles variants - max white pawns and max black
**                 pawns
**
** 2000/01/09 SE   new condition: Ultraschachzwang (Bl, W, and quasi versions)
**                 new condition: ActuatedRevolvingCentre
**
** 2000/01/19 SE   new condition : parasentinelles
**
** 2000/01/20 TLi  AntiAndernach + Neutrals bug fixed in jouecoup()
**                 HalfNeutrals changed in jouecoup and() repcoup()
**
** 2000/01/26 TLi  Anticirce bug in py5.c fixed
**
** 2000/04/12 NG   bug fix for marscirce/phantomchess with fairy-kings (hint of SE)
**
** 2000/05/26 NG   new piece: scorpion  new condition: StingChess
**
** 2000/06/05 NG/TLi  bug fix: sentinelles works now with fairy pawns. 
**
** 2000/09/19 TLi  new condition: LineChameleonChess
**
** 2001/01/14 SE   new condition: EchecsAlphabetic
**
** 2001/02/05 TLi  new pieces: Moariderlion, AndernachGrassHopper, Friend
**
** 2001/08/29 ThM  new conditions singleboxtype[123]
**
** 2001/10/26 NG   assertions only with -DASSERT due to performance penalty
**
** 2001/11/10 NG   singleboxtype? changed to singlebox type?
**		   some singlebox variable names changed ...
**
**************************** INFORMATION END ***************************/
