/**************************** MODIFICATIONS to py6.c ****************************
** boolkamikazezz and boolneutrezz changed to
** bool_kamikaze  and bool_neutre because my C-Compiler
** looks only for the first 7 chars per identifier !    13. 7.1989    NG
**
** series-mate in n moves added:
** function ser_matsol(camp,n)  and  ser_solution(),
** flag  solsermat  and  input-string "ser-#*" added.   15. 7.1989    NG
**
** function ser_matsol() tuned. 1% faster !!            17. 7.1989    NG
**
** series-stalemate in n moves added:
** function ser_patsol(camp,n), flag  solserpat,
** input-strings "ser-=*" and "ser-==*" added.          17. 7.1989    NG
**
** input-string "koeko" added,
** function litposition() and
** function verifieposition() modified for koeko-chess  18. 7.1989    NG
**
** function verifieposition() modified for fairy pieces
** in monochrome chess, bichrome chess,
** grid-chess and koeko-chess                           21. 7.1989    NG
**
** function verifieposition() modified for alfil and equihopper
** in monochrome chess, bichrome chess,
** grid-chess and koeko-chess.                          21. 7.1989    NG
**
** function verifieposition() modified for circe
** and (!) koeko-chess.                                 21. 7.1989    NG
**
** Four new pieces: amazon     (am),
**                  empress    (im),
**                  princess   (pr),
**                  gnu        (gn)                     23. 7.1989    NG
**
** flag  flappseul for only(!) setplay in helpplay      23. 7.1989    NG
**
** input-strings "rand" and "doppelrand" added,
** function litposition() and
** function verifieposition() modified for
** randzueger and doppelrandzueger.                     12.10.1989    NG
**
** a great patrol-chess bug fixed.
** (it just didn't work correct !).
** therefore initialisation of patrcalcule and
** patrsoutien added.                                   17.10.1989    NG
**
** Five new pieces: antelope    (an),
**                  dragon      (dr),
**                  squirrel    (ec),
**                  kangaroo    (ka),
**                  waran       (wa)                    20.11.1989    NG
**
** function verifieposition() modified for fairy
** pieces in madrasi.                                   22.11.1989    NG
**
** function litpo3() modified due to
** "a suivre" - error.                                  25.11.1989    NG
**
** functions verifieposition(), editcoup() and main()
** modified for calculation of duplex-problems.         27.11.1989    NG
**
** New piece:   "Spiralspringer"        (cs)             4.12.1989    NG
**
** calculation of checking-tries in 2#.                 21.12.1989    NG
**
** a leofamily-bug fixed. If there were no fairy
** pieces on board, flagfee wasn't set !                 6.01.1990    NG
**
** functions current() , editcoup(), main(),
**           verifieposition():
** modifications for pwc !                              18.02.1990    NG
**
** functions litposition(), main(),
**           verifieposition():
** modifications for "immunschach" and "ohneschlag".    17.03.1990    NG
**
** modifications:
**      flmalefique also used with "immunschach".       20.03.1990    NG
**
** New piece:   "Ubi-Ubi"               (uu)            24.03.1990    NG
**
** ser_patsol() modified due to double-stalemate bug.   30.05.1990    NG
**
** new option: sansrn   no black king allowed !          3.06.1990    NG
**
** function verifieposition() modified for dummy
** pieces in circe.                                     16.07.1990    NG
**
** function editcoup() modified for better
** piece-color-readability.                              1.08.1990     NG
**
** function jeudessais() tuned. Flag for defense-
** calculation-break established again.                  8.08.1990     NG
**
** Some new functions due to new stipulation "Ziel"     13.08.1990      NG
**
** geloescht, da nicht mehr benoetigt:
**      nbinter(), nbmat(), editmenaces(), jeudessais(),
**      toutcoup()                                      30.8.1990   TLi
**
** geloescht, da ueberarbeitet und in py7.c uebernommen:
**      matsol(), variantes(), nbdef(), jeuapparent(),
**      invsol(), refsol(), main()                      30.8.1990   TLi
**
** deleted because not needed any more:
**               nbstip(), stiptry(), toutstip()
** new:         stipparmenace()
** modified: all "stip"-functions                        7.9.1990   NG
**
** function verifieposition() modified:
** firstpromotionpiece initialized for black and white. 21. 7.1989     NG
**
** functions current(), verifieposition() and editcoup()
** modified for problems with imitators                 21.01.1991      TM
**
** functions current(), verifieposition() and editcoup()
** modified for circe problems with kamikaze pieces     05.02.1991      TM
**
** added function WriteGeneralSquare() to save code     05.02.1991      TM
**
** changed function aidesolution(); there was an error
** when outputting solutions for h#n                    05.02.1991      TM
**
** made compilable with MPW on macintosh-computers.     14.02.1991      SB
**
** in PWC neutral pawns on first and (!) eightth        29.03.1991      NG
** rank allowed.
**
** initialisation of empilegenre in verifieposition     11.06.91        NG
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** geloescht:           uebernommen in:
** ------------------------------------
** shsolution
** ser_solution
** aidesolution         linesolution
** last_ph_move         last_h_move
** pataide              mataide
** shpatsol             shsol
** last_ss_move
** last_sr_move         last_dsr_move
** ser_patsol
** ser_matsol
** sssol
** srsol                ser_dsrsol
** ----------------------------------           15.02./5.5.1991  TLi
**
** editcoup() - FlDuplex gestrichen, flende eingearbeitet
** current()  - mov->mat gestrichen
**
** videtabsol()  ueberarbeitet                  18.02.1991  TLi
** alle stip-Funktionen in dsr_... eingearbeitet -> PY7.C    TLi
** function WriteGeneralSquare() wieder geloescht, da duplex
** nun anders laeuft                                            TLi
** verifieposition modifiziert: kamikaze & empilegenre
**                              ser-r/s jetzt auch mit enroute/trace
**                              self/reflex nicht mit ziel
**                              flaflegalsquare gestrafft
**                              echeccgenre eingefuegt          TLi
** neue Funktionen fuer Zugtabellen: alloctab, freetab; nowdanstab etc.
**                              modifiziert                     TLi
** verifieposition fuer jouegenre modifiziert                   TLi
**                      maxi-minimummer                         TLi
**
** verifieposition modified: some pieces are not compatible
** with maxi-/mini-mummer                               23.11.91        NG
**
** (PWC or rexinclusif) and haanerchess allowed together.
** "randzueger"-Conditions changed to whiteedge and blackedge.  28.11.91    NG
**
** due to letter of TLi and rx and rz bug, stipcamp eliminated.
**                                                      14.2.1992       NG
**
** totalortho in verifieposition(), last_???(), matant()        20.2.92  TLi
**
** Some known bug-combinations excluded inside revised
** function verifieposition.                            23.2.1992       NG
**
** The initial message is not taken from the *.str file
** We try to open any *.str file on the very first beginning
**                                                      12Apr1992       ElB
**
** printing of en passant capture added in editcoup().  08May1992       NG
**
** new: change_moving_piece to indicate conditions for jouecoup() wher
**	the moving piece may changed			9. 4. 1993	TLi
**
** New pieces: BoyScout, GirlScout                       1.05.1993      NG
**
** 1993/5/25  ElB  Moved all remarks on modifications of py6.c to
**                 py6-c.mod. Look there for remarks before this date.
**
** 1993/6/20  TM   imitator with lot of fairy-pieces.
**
** 1993/07/02 StH  I changed the functions: mataide, last_h_move,
**                 shsol, dsr_sol, dsr_vari, ser_dsrsol  to stop
**                 solving when a given number of solutions has
**                 encountered. Look for     OptFlag[maxsols]
**
** 1993/07/10 TLi  new option: solflights x - mate() and dsr_def() modified
**
** 1993/07/26 TLi  Hashing for ser-problems - shsol, ser_dsrsol (see PYHASH.C)
**                 and main modified
**
** 1993/08/08 TLi  last_dsr_move and last_h_move must be changed too
**                 (see also PYHASH.C)
**
** 1994/02/15 NG   changed: current(), verifie...() due to new conditions
**                 ForcedSquares and I/O-bug with condition  exact  .
**
** 1994/04/29 TLi  new implementation of ForcedSquares
**
** 1994/05/03 ElB  Added two commandline options. They are only compiled
**                 when -DHASH is given. They are only usefull in virtual
**                 memory environments: where more memory is given to the
**                 process than pysical available.
**                  -maxpos num: num gives the maximum positions to
**                    hold in the HashTable. If this limit is reached
**                    some positions with less information will be
**                    purged from the hashtable.
**                  -maxmem Kbytes: If this option is given, popeye
**                    tries to determine an appropriate value for maxpos.
**                    But this will only be a crude calculation. The value
**                    for maxpos determined by this way shoould not be
**                    expected to be in any sense accurate.
**                    This option takes precedence over -maxpos. Beside
**                    this, the behaviour of these options is equivalent.
**
** 1994/05/06 NG   Some "NODEF" deleted. HashStats() moved.
**                 Endless loop-bug due to commandline options under DOS fixed.
**
** 1994/07/31 TLi  Popeye can handle maxi/minimummer with locust type pieces!
**
** 1995/04/26 TLi  new stipulation: seriesmovers with introductory move
**                 new function: introseries()
**
** 1995/05/23 NG   ep-output in editcoup corrected
**
** 1995/05/26 TLi  new conditions: Chameleon Chess and Beamten Chess
**
** 1995/06/01 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/06/09 TLi  hashing in introseries improved
**
** 1995/06/09 TLi  new option: nontrivial
**
** 1995/06/13 TLi  hashing for series movers improved - options trace and
**                 enroute had to be deactivated for in some cases
**
** 1995/06/14 TLi  PatrolChess/Beamten and Madrasi/paraly. pieces allowed now
**
** 1995/06/14 TLi  new piece: Okapi = Knight + Zebra
**
** 1995/06/26 TLi  new stipulation: ##, ##! (double mate, counter mate)
**
** 1995/06/30 TLi  further optimisation of orthodox play does not allow
**                 AndernachChess and related conditions to be considered
**                 orthodox anymore.
**
** 1995/07/19 TLi  new stipulation: dia == proof game
**
** 1995/08/04 NG   Castling in totalortho play implemented.
**
** 1995/08/08 TLi  new stipulation: 00 == castling
**
** 1995/08/09 NG   Castling for a lot of conditions implemented.
**
** 1995/08/18 TLi  new conditions: direct reflex, wh/bl must check
**
** 1995/10/23 NG   MaxMemory assignment now here due to StartUp Info addition.
**
** 1995/10/30 NG   helpselfmate bug fixed.
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/28 TLi  old orthodox optimization stuff and switch OPTIMIZE
**                 deleted
**
** 1995/12/28 TLi  didn't succeed in implementing SchachZwang and
**                 directreflex -- deleted both
**
** 1995/12/29 TLi  new condition: GlasgowChess
**
** 1996/01/01 NG   Moved all remarks on modifications of py6.c to
**                 py6-c.mod. Look there for remarks before this date.
**
** 1996/01/18 TLi  verifieposition(): transmuting/reflecting royal pieces
**                 excluded
**
** 1996/02/07 NG   new option: halfduplex, ie. stipulation is only
**                             calculated for the 'opponent' side.
**
** 1996/03/19 TLi  new: twinning supported
**
** 1996/04/13 TLi  bug fixed in SolveSeriesProblems & introseries
**
** 1996/04/23 TLi  new option: postkeyplay
**
** 1996/04/25 TLi  twinning supported
**
** 1996/06/10 NG   new stipulation: ser-dia (SeriesProofGame)
**
** 1996/11/15 NG   New option:  PromOnly
**
** 1996/12/07 TLi  now: seriesmovers with more than 1 introductory move
**
** 1996/12/09 NG   is_simplehopper: grashopper-2 and -3 added.
**
** 1996/12/10 TBa  Changes for WIN16 Popeye
**
** 1996/12/19 NG   Einstein and castling output in editcoup added
**
** 1996/12    SE/TLi  new condition: isardam
**
** 1996/01/28 NG   is_simplehopper: kinghopper added.
**
** 1997/02/06 TLi  implementation of help- and series play revised
**
** 1997/02/06 NG   new piece: orix
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/02/17 NG   Option  PromOnly  changed to Condition  Promonly
**
** 1997/03/10 TLi  generalized Chameleon Circe
**
** 1997/03/14 TLi  new option: intelligent
**
** 1997/04/08 TLi  linesolution adapted for intelligent mode
**
** 1997/04/22 TLi  exclusive chess
**
** 1997/06/04 SE/TLi  new condition: MarsCirce
**
** 1997/06/23 NG   Interrupt message if maxsolutions are reached
**
** 1997/06/27 NG   new pieces: 1:5-leaper, 2:5-leaper
**
** 1997/07/28 TLi  new option: LaTeX
**
** 1997/08/11 NG   new condition: PhantomChess
**
** 1997/08/16 NG   conditions  transmuted/reflected kings
**                 additionally splitted into 
**                 white/black transmuted/reflected kings
**
** 1997/09/04 TLi  WriteSpec modified
**
** 1997/10/02 TLi  new condition: Black/WhiteRoyalSquare
**
** 1997/10/09 TLi  Parrain output bug spotted by NG fixed
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**
** 1997/10/28 NG   intelligent + duplex + serieshelpmate bug fixed.
**
** 1997/10/31 SE   new condition: Isardam TypeB
**
** 1997/11/05 SE   new condition: Sentinelles (en) PionAdvers
**
** 1997/11/09 SE   new condition: Sentinelles (en) PionNeutre
**
** 1997/11/20 SE   new condition: Patience chess, two types
**
** 1997/12/12 SE   new stipulations: stip_circuit, _exchange (& rebirth
**						 types)
**
** 1997/12/30 SE   new condition : Republican Chess
**                 problems with supercirce in nowdanstab fixed
**
** 1998/01/08 SE   checks in Isardam and Brunner now computed for output
**
** 1998/01/14 NG   castling now allowed for transmuted kings
**
** 1998/01/20 ElB  changed calls to HashStats.
**
** 1998/05/19 NG   Intelligent mode restricted to h# and ser-h#
**                 totalortho bug in verifieposition() fixed
**
** 1998/06/05 NG   new option: MaxTime
**
** 1998/06/10 NG   better usage of  castling_supported
**
** 1998/07/25 SE   All stips allowed with republican - no guarantees!
**
** 1998/07/26 TLi  new piece specification: Beamtet
**
** 1998/10/26 SE   royal pieces now allowed with some circe RI, immune RI
**
** 1999/01/17 NG   option MaxTime improved
**
** 1999/02/19 NG   new pieces: wazir-/fers-rider (think at circe rebirth !)
**                 definitions of queen, rook and knight changed 
**
** 1999/03/15 TLi  new condition: MessignyChess (RexInclusive)
**
** 1999/03/27 NG   castling disallowed again for transmuted kings
**
** 1999/03/29 TLi  editcoup changed for castling, 
**                 castling allowed again for transmuted kings
**
** 1999/04/02 NG   New option:  NoCastling
**
** 1999/04/17 TLi  new conditions: (Bi)Woozles/Heffalumps
**
** 1999/05/05 TLi  output bug for series movers with introductory play
**                 in linesolution fixed
**
** 1999/05/25 NG   Moved all remarks on modifications of py6.c to
**                 py6-c.mod. Look there for remarks before 1998/01/01.
**
** 1999/06/12 SE   new piece specification : halfneutrals
**
** 1999/06/17 NG   halfneutral output "internationalized"
**
** 1999/06/22 NG   new conditions: WhitePromSquares  and  BlackPromSquares
**
** 1999/08/24 TLi  new: hashing for series movers with introductory
**                 moves; function introseries moved to pyhash.c
**
** 1999/09/29 TLi  bug fix: BrunnerChess
**
** 1999/10/07 NG   mao  to  vao  bugfix due to changed orthodox defines.
**
** 1999/10/08 SE   new option : Quodlibet
**                 in self- and (semi-)reflex play, allows White to make contract by
**                 direct stip also (eg. #/S# or #/R#).
**
** 1999/10/08 SE   new stipulation: #= mate or stalemate
**
** 1999/10/21 NG   position of StartTimer changed, so that inputtime isn't calculated
**
** 1999/11/08 NG   MaxSolutions now calculated also for twins, duplex, ...
**                 New option:  StopOnShortSolutions
**
** 1999/12/22 TLi  intelligent mode for orthodox ser-# enabled
**
** 2000/01/04 NG   Allocatable memory on 32-Bit systems increased upto 4 GB
**
** 2000/01/09 SE   new quasi-condition: maxi (black or white depending on maincamp
**                 new condition: Ultraschachzwang (Bl, W, and quasi versions)
**                 new conditions: ActuatedRevolvingCentre, ShieldedKings
**
** 2000/01/20 TLi  WriteSpec() and editcoup() modified
**
** 2000/01/26 TLi  Anticirce bug fixed
**
** 2000/02/02 NG   Intelligent helpplay allowed if castling is not possible.
**                 Therefore testcastling "globalized".
**
** 2000/04/12 NG   new pieces: (rook/bishop)-(eagle/moose/sparrow)
**
** 2000/04/13 TLi  nontrivial implementation changed -- did not work
**                 properly before
**
** 2000/04/13 TLi  maxmem input changed; now accepts, e.g., -maxmem 1G
**
** 2000/05/09 NG   new piece: Rao (Chinese Rose)
**
** 2000/05/09 TLi  optimization of black moves in MarsCirce disabled
**                 yielded mistakes
**
** 2000/05/25 TLi  bug fix in nontrivial mode
**
** 2000/05/26 NG   new piece: scorpion  new condition: StingChess
**                 Message "PawnFirstRank" obsolete now.
**
** 2000/09/19 TLi  new condition: LineChameleonChess
**
** 2000/10/21 NG   implementation of condition No?????Prom changed
**
** 2000/10/23 NG   new conditions: NoWhiteCapture, NoBlackCapture
**
** 2000/11/29 TLi  refutations to s#1 enabled
**                 silly chamchess+linechamchess bug fixed in
**                 verifyposition
**
** 2000/11/29 TLi  new piece: 3:6 leaper
**
** 2001/01/14 SE   new specification : ColourChanging
**
** 2001/02/05 TLi  new piece: Friend
**
** 2001/05/08 SE   new conditions: CirceTurnCoats, CirceDoubleAgents, AMU,
**		   SentBerolina.
**
** 2001/09/29 ThM  new condition: SingleBox Type1, Type2 or Type3
**
** 2001/10/23 TLi  bug-fix: reflexmate
**
** 2001/10/26 NG   assertions only with -DASSERT due to performance penalty
**
** 2001/11/10 NG   singleboxtype? changed to singlebox type?
**		   some singlebox variable names changed ...
**
** 2002/03/03 NG   duellistchess moved outside jouegenre
**
** 2002/04/04 NG   commandline option -regression for regressiontesting
**
** 2002/20/08 TLi  a=>b bug fixed
**
** 2002/20/08 TLi  maxsolutions bug fixed
**
**************************** INFORMATION END **************************/
