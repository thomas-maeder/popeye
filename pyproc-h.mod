/**************************** MODIFICATIONS to pyproc.h ****************************
** Added six new prototypes for nocontact, casefnoech,
** casefblech, ser_solution, ser_matsol, ser_patsol
** Look at lines 153-159                                26Jul89         ElB
**
** nicht mehr benoetigt:  editmenaces(), nbinter(), nbmat(),
**           toutcoup() und jeudessai()                 15.8.1990       TLi
** modifiziert:  matsol(), invsol(), refsol()
** hinzugefuegt: invapparent(), invparmena(), invdef(), invvari(),
**               refapparent(), refparmena(), refdef(), refvari(),
**               parmenace()                            25.8.1990       TLi
**
** nicht mehr benoetigt: nbstip(), toutstip() und stiptry()
** modifiziert: stipsol()
** hinzugefuegt: stipparmenace()                         7.09.1990      NG
**
** functions rsautech(), rrfouech(), rcardech() added.
** function encore() deleted. It's now a MAKRO.         16.10.1990      NG
**
** some older not supported functions suppressed and
** some missing prototypes added.
** New prototype: nogridcontact()                       17.11.1990      NG
**
** Prototypes modified: empile() and testempile()
** New prototypes: imok(), ridimok(), joueim()          21.01.1991      TM
**
** New prototypes due to new checktest-system. All
** prototypes are now in alphabetical order !!          27.06.1991      NG
**
** FatalMsg changed to FtlMsg, since uniqnes on first 
** seven characters and new function FtlMsg1		30.07.1991	ElB
** neue Funktionen dsr_ant/e/def/vari/sol/parmena - ersetzen
** stip...,inv...,ref...,mat...                 	17.9.1991   	TLi
** neu:	definvref, invref ersetzen (def)inverse/reflexe	18.9.1991	TLi
** soutenu, r?circech, r?immunech, renaicirce	modifiziert
**							23.9.1991	TLi
** PrintTime added for better user info.                27.11.1991      NG
**
** change(), setneutre(), finligne() jetzt Makros       15.2.1992       TLi
**
** 1993/5/25  ElB  Moved all remarks on modifications of pyproc.h to
**                 pyproc-h.mod. See there for remarks before this date.
**
** 1993/07/10 TLi  new pieces: skylla and charybdis - here declaration of
**                 skylla/charybdischeck()
**
** 1993/07/26 TLi  Hashing for ser-problems - functions createhash,
**                 inithash and closehash declared
**
** 1993/07/27 TLi  gedgeh() replaces gedgeh?()
**
** 1993/07/30 TLi  declaration of ser_dsrsol() and last_dsr_move() added
**
** 1993/08/08 TLi  declaration of last_dsr_move() changed
**
** 1994/02/15 NG   New conditions:  ForcedSquares
**
** 1994/05/23 NG   LogLngArg added due to opyhash.c problems.
**
** 1994/06/03 TLi  New function:  is_pawn.
**
** 1994/07/28 TLi  New functions: contragrascheck and gecontragras
**
** 1995/01/10 NG   Changed: rubiech() due to UbiUbi bug with Madrasi.
**
** 1995/04/26 TLi  new stipulation: seriesmovers with introductory move
**                 new function introseries()
**
** 1995/06/14 TLi  new piece: okapi = knight + zebra
**
** 1995/06/26 TLi  new stipulation: ##, ##! (double mate, counter mate)
**
** 1995/07/19 TLi  new piece: 3:7-leaper
**
** 1995/07/19 TLi  new stipulation: dia == proof game
**
** 1995/08/03 TLi  new piece: grasshopper-3
**
** 1995/08/08 TLi  new stipulation: 00  (castling)
**
** 1995/08/08 NG   ProofInitialise (used in py6.c)
**
** 1995/10/24 ElB  pyInitSignal (used in py6.c)
**
** 1995/11/02 NG   new piece: 1:6-leaper
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/28 TLi  old orthodox optimization stuff and switch OPTIMIZE
**                 deleted
**
** 1996/01/01 NG   Moved all remarks on modifications of pyproc.h to
**                 pyproc-h.mod. Look there for remarks before this date.
**
** 1996/06/10 NG   new stipulation: ser-dia (SeriesProofGame)
**
** 1996/11/06 TLi  new pieces: 2:4-leaper, 3:5-leaper
**
** 1996/11/15 NG   New option:  PromOnly
**
** 1996/12/07 TLi  new: seriesmovers with more than 1 intod. move
**
** 1996/12    SE/TLi new pieces: double grashopper and kinghopper
**
** 1996/12    SE/TLi new condition: Isardam
**
** 1997/02/06 NG   new piece: orix
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/02/10 TLi  new condition: DiametralCirce
**
** 1997/02/27 TLi  ep-bug in Madrasi fixed
**
** 1997/03/11 TLi  new condition: RankCirce
**
** 1997/06/27 NG   new pieces: 1:5-leaper, 2:5-leaper
**                             gral (alfil + rook-hopper)
**
** 1997/07/28 TLi  new: LaTeX output
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**		   new stipulation: autostalemate (symbol: !=)
**
** 1998/07/27 TLi  new pieces: rook/bishop/night locust
**
** 1998/08/21 TLi  new condition: ActuatedRevolvingBoard
**
** 1999/01/17 NG   new option MaxTime improved
**
** 1999/03/29 TLi  castling implementation changed
**
** 1999/04/17 TLi  new condition: (Bi)Woozles/Heffalumps
**
** 1999/05/25 NG   Moved all remarks on modifications of pyproc.h to
**                 pyproc-h.mod. Look there for remarks before 1998/01/01.
**
** 1999/08/24 TLi  new: hashing for series movers with introductory moves
**
** 1999/10/08 SE   new stipulation: #= mate or stalemate
**
** 1999/11/21 TLi  new piece: bison (1,3)+(2,3) leaper
**
** 2000/01/13 TLi  new piece: Elephant
**
** 2000/01/28 TLi  new piece: Nao (Chinese Nightrider)
**
** 2000/04/12 NG   new pieces: (rook/bishop)-(eagle/moose/sparrow)
**
** 2000/05/26 NG   new piece: scorpion   new condition: StingChess
**
** 2000/10/26 TLi  new piece: Marguerite (MG)
**
** 2000/11/29 TLi  new pieces: 3:6 leaper, night-rider lion, mao-rider
**                 lion
**
** 2001/02/05 TLi  new piece: Moariderlion, Friend
**
** 2001/08/29 ThM  new condition: SingleBox Type1, Type2 or Type3
**
** 2001/10/02 TLi  new piece: Dolphin (Grashopper + Kangaroo)
**
**************************** INFORMATION END **************************/
