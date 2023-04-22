#! /usr/bin/env tclsh

# Usage: $argv0 INPUTFILEPATH | diff -y --width=200 INPUTFILEPATH -

set inputfile [lindex $argv 0]

if {[llength $argv]>1} {
    set sections [split [lindex $argv 1] ","]
} else {
    set sections {}
}

switch -re $inputfile {
    ".*[.]out$" {
	set language "german"
    }
    ".*[.]tst$" -
    ".*[.]ref$" -
    ".*[.]reg$" {
	set language "english"
    }
    default {
	error "failed to detect language"
	exit 1
    }
}

namespace eval german {
    set endOfSolution "Loesung beendet."
    set partialSolution "Partielle Loesung"
    set white "Weiss"
    set black "Schwarz"
    set zugzwang "Zugzwang."
    set threat "Drohung:"
    set but "Aber"
    # yes, some shortcuts are ambiguous
    set pieceAttributeShortcuts {
	{[wsn]}
	k
	k
	p
	c
	j
	v
	b
	hn
	sfw
	p
	m
	u
	p
	f
    }
    set zeroposition "NullStellung"
    set potentialPositionsIn "moegliche Stellungen in"
    set kingmissing "Es fehlt ein weisser oder schwarzer Koenig"
    set legalityUndecidable "kann nicht entscheiden, ob dieser Zug legal ist."
    set illegalSelfCheck "Die am Zug befindliche Partei kann den Koenig schlagen"
    set roleExchange "Rollentausch"
    set refutes "Widerlegt."
    set toofairy "Zu viel Maerchenschach fuer neutrale Steine"
    set problemignored "Problem uebersprungen"
    set nonsensecombination "Unsinnige Kombination"
    set intelligentAndFairy "Intelligent Modus nur im h#/=, ser-#/= und ser-h#/=, wenigen Maerchenbedingungen und keinen Maerchensteinen."
    set refutationUndecidable "Kann nicht entscheiden, ob dieser Zug widerlegt wird"
    set conditionSideUndecidable "Es ist nicht entscheidbar, ob Bedingung fuer Weiss oder Schwarz gilt"
    set setplayNotApplicable "Satzspiel nicht anwendbar - ignoriert"
    set tryplayNotApplicable "Verfuehrung nicht anwendbar"
    set duplex "Duplex"
    set halfduplex "HalbDuplex"
    set inputError "Eingabe-Fehler"
    set offendingItem "Stoerende Eingabe"
}

namespace eval english {
    set endOfSolution "solution finished."
    set partialSolution "Partial solution"
    set white "White"
    set black "Black"
    set zugzwang "zugzwang."
    set threat "threat:"
    set but "but"
    # yes, some shortcuts are ambiguous - not the same as in German
    set pieceAttributeShortcuts {
	{[wbn]}
	r
	k
	p
	c
	j
	v
	f
	hn
	hcc
	p
	m
	u
	p
	f
    }
    set zeroposition "zeroposition"
    set potentialPositionsIn "potential positions in"
    set kingmissing "both sides need a king"
    set illegalSelfCheck "the side to play can capture the king"
    set toofairy "too much fairy chess for neutral pieces"
    set problemignored "problem ignored"
    set nonsensecombination "nonsense combination"
    set intelligentAndFairy "Intelligent Mode only with h#/=, ser-#/= and ser-h#/=, with a limited set of fairy conditions and without fairy pieces."
    set refutationUndecidable "Can't decide whether this move is refuted"
    set refutes "refutes."
    set conditionSideUndecidable "Can't decide whether condition applies to White or Black"
    set legalityUndecidable "cannot decide if this move is legal or not."
    set roleExchange "RoleExchange"
    set setplayNotApplicable "SetPlay not applicable - ignored"
    set tryplayNotApplicable "try play not applicable"
    set duplex "Duplex"
    set halfduplex "HalfDuplex"
    set inputError "input-error"
    set offendingItem "offending item"
}

# syntactic sugar for looking up language dependant strings
proc l {name} {
    global language
    return [set ${language}::$name]
}

# syntactic sugar for looking up variables in ancestor namespaces
proc v {name} {
    set scope [uplevel namespace current]
    set initialscope $scope
    while {![info exists ${scope}::$name]} {
	if {$scope=="::"} {
	    puts stderr "can't find variable $name from scope $initialscope"
	    exit 1
	} else {
	    set scope [namespace parent $scope]
	}
    }
    return [set ${scope}::$name]
}

namespace eval format {
    set eol {\n}
    set emptyLine {\n}
    set lineText {[^\n]}

    namespace eval remark {
        set remark "[v lineText]+"
        set remarkLine "(?:$remark[v eol])"

        set RE "(?:$remarkLine+[v emptyLine]*)"
    }

    namespace eval authoretc {
        set leadingBlanks { *}

        set authorOriginAwardTitle "[v lineText]+"
        set authorOriginAwardTitleLine "(?:$leadingBlanks$authorOriginAwardTitle[v eol])"

        set RE "[v emptyLine]?$authorOriginAwardTitleLine*[v emptyLine]"
    }

    namespace eval board {
        set nrRows 8
        set nrColumns 8
        
        set cornerSign {[+]}
        set verticalBorderSign {[|]}
        set horizontalBorderSign "-"
        
        set columnName {[a-h]}
        set columnSpec "$horizontalBorderSign$horizontalBorderSign$columnName$horizontalBorderSign"
        set columns "$cornerSign${horizontalBorderSign}(?:$columnSpec){$nrColumns}$horizontalBorderSign$horizontalBorderSign$cornerSign[v eol]"

        set rowNo {[1-8]}
        set gridVertical {[|]}
        set gridHorizontal " ---"
        set noPiece {[.]}
        set squareEmpty "(?: |$gridVertical) $noPiece"
        set hole "(?: |$gridVertical)  "
        set color {[ =-]}
        set pieceChar {[[:upper:][:digit:]]}
        set piece1Char "(?: |$gridVertical)$color$pieceChar"
        set piece2Chars "(?:$color|$gridVertical)$pieceChar{2}"
        set pieceSpecSeparator " "
        set hunterPartsSeparator "/"
        set pieceSpec "(?:$squareEmpty|$hole|$piece1Char|$piece2Chars)(?:$pieceSpecSeparator|$hunterPartsSeparator)"
        set piecesLine "${rowNo} (?:$pieceSpec){$nrColumns}  $rowNo[v eol]"

        set hunter2ndPart "(?:$hole|$piece1Char|$piece2Chars)"
        set spaceLine "$verticalBorderSign (?:$hunter2ndPart$pieceSpecSeparator|$gridHorizontal){$nrColumns}  $verticalBorderSign[v eol]"

        set RE "(?:${columns}(?:$spaceLine$piecesLine){$nrRows}$spaceLine$columns)"
    }

    namespace eval gridboard {
        namespace eval cellsline {
	    set cell {[ [:digit:]][[:digit:]]}
	    set RE "[v board::rowNo](?:  $cell){[v board::nrColumns]}   [v board::rowNo][v eol]"
        }

        set RE "(?:[v emptyLine][v board::columns](?:[v board::spaceLine]$cellsline::RE){[v board::nrRows]}[v board::spaceLine][v board::columns])"
    }

    namespace eval stipulation {
        set goal {(?:\#|=|dia|a=>b|z[a-h][1-8]|ct|<>|<>r|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k[a-h][1-8])}
        set exact {(?:exact-)}
        set intro {(?:[[:digit:]]+->)}
        set series "(?:$intro?(?:ph?)?ser-)"
        set help "h"
        set paren_open {[(]}
        set paren_close {[)]}
        set recigoal "(?:$paren_open$goal$paren_close)"
        set recihelp "(?:reci-h$recigoal?)"
        set self "s"
        set reflex {(?:(?:semi-)?r)}
        set play "(?:(?:$series)?(?:$help|$recihelp|$self|h$self|$reflex|h$reflex)?)"
        set length {(?:[[:digit:]]+(?:[.]5)?)}
        set stipulation_traditional "(?:$exact?$play$goal$length)"

        set side "(?:[set ${language}::white]|[set ${language}::black])"
        set stipulation_structured "(?:$side \[^ ]+)"; # TODO

        set maxthreat {(?:/[[:digit:]]*)}
        set maxflight {(?:/[[:digit:]]+)}
        set nontrivial {(?:;[[:digit:]]+,[[:digit:]]+)}

        set RE "(?:(?:$stipulation_traditional|$stipulation_structured)(?:$maxthreat$maxflight?)?$nontrivial?)?"; # TODO order of suffixes?
    }
        
    namespace eval pieceControl {
        set piecesOfColor {[[:digit:]]+}
        set plus {[+]}
        set RE "$piecesOfColor $plus ${piecesOfColor}(?: $plus ${piecesOfColor}n)?(?: $plus ${piecesOfColor} TI)?"
    }

    namespace eval caption {
        set stip_pieceControl " *[v stipulation::RE] *[v pieceControl::RE][v eol]"
        set RE "${stip_pieceControl}"
    }

    namespace eval boardA {
        # the caption of board A doesn't indicate the stipulation
        set caption " *[v pieceControl::RE][v eol]"
        set tomove "[v stipulation::paren_open][v stipulation::side] ->[v stipulation::paren_close]"
        set RE "(?:[v board::RE]$caption[v eol] *=> $tomove[v eol][v emptyLine][v emptyLine])"
    }

    namespace eval conditions {
        set line " *[v lineText]+[v eol]"
        set RE "(?:$line)*?"
    }

    namespace eval duplex {
        set line " *(?:[l duplex]|[l halfduplex])[v eol]"
        set RE "(?:$line)?"
    }

    namespace eval solution {
        set ordinalNumber {[1-9][0-9]*[.]}
        set paren_open {[(]}
    	set paren_close {[)]}
        set bracket_open {\[}
        set bracket_close {\]}
        set ellipsis {[.][.][.]}
        set walkChar {[[:upper:][:digit:]]}
        set hunterSuffix "(?:/$walkChar{1,2})"
        set walkPawnImplicit "$walkChar{0,2}$hunterSuffix?"
        set walk "(?:$walkChar{1,2}$hunterSuffix?)"
        set square {[a-h][1-8]}
        set capture {[*]}
        set captureOrNot {[-*]}
        set castlingQ "0-0-0"
        set castlingK "0-0"
        set pieceAttributeShortcut "(?:"
        foreach p [l pieceAttributeShortcuts] {
	    if {[string length $p]==1} {
		append pieceAttributeShortcut "$p?"
	    } else {
		append pieceAttributeShortcut "(?:$p)?"
	    }
        }
        append pieceAttributeShortcut ")"
        set movement "(?:(?:$pieceAttributeShortcut$walkPawnImplicit${square}$captureOrNot$square|$castlingQ|$castlingK)(?:$captureOrNot$square)*)"
        set messignyExchange "(?:$walk$square<->$walk$square)"
        set promotion "(?:=$pieceAttributeShortcut$walk?)"
        set enPassant {(?: ep[.])}
        set vulcanization "(?:->v)"
        set pieceChangement "(?:$square$promotion)"
        set pieceSpec "$pieceAttributeShortcut$walk$square"
        set pieceMovement "(?:$pieceSpec->$pieceAttributeShortcut$walk?$square$promotion*$vulcanization?)"
        set pieceAddition "(?:\[+]$pieceSpec$promotion*$vulcanization?)"
        set pieceRemoval "(?:-$pieceSpec)"
        set pieceExchange "(?:$pieceSpec<->$pieceSpec)"
        set pieceEffect "(?:${bracket_open}(?:$pieceMovement|$pieceAddition|$pieceRemoval|$pieceChangement|$pieceExchange)$bracket_close)"
        set imitatorMovement "(?:${bracket_open}I${square}(?:,$square)*$bracket_close)"
        set bglNumber {[[:digit:]]+(?:[.][[:digit:]]{1,2})?}
        set bglBalance "(?: ${paren_open}(?:-|$bglNumber)(?:/$bglNumber)?$paren_close)"
        set checkIndicator {(?: [+])}
        # yes, this is slightly different from stipulation::goal!
        set goal {(?: (?:\#|=|dia|a=>b|z|ct|<>|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k[a-h][1-8]))}
        set castlingPartnerMovement "(?:/$movement)"
        set totalInvisibleMove "TI~-~"
        set totalInvisibleCapture "TI~$capture$square"
        set move "(?: [l roleExchange]| $ellipsis|$pieceEffect*(?:$movement$castlingPartnerMovement?|$totalInvisibleMove|$totalInvisibleCapture|$messignyExchange)$enPassant?$imitatorMovement?$promotion*$pieceEffect*$bglBalance?$checkIndicator?)$goal?"

        set moveNumber {[1-9][0-9]*}
        set moveNumberLineNonIntelligent "(?: *$moveNumber  \[(]$move \[)][v eol])"

        set nrPositions {[[:digit:]]+}
        set nrMoves {[[:digit:]]+[+][[:digit:]]+}
        set moveNumberLineIntelligent "$nrPositions [l potentialPositionsIn] $nrMoves[v eol]"

        set undec "(?: (?:[l legalityUndecidable]|[l refutationUndecidable]))"

        namespace eval twinning {
	    set RE "(?:[v emptyLine]\[+\]?\[a-z]\\) [v lineText]*[v eol](?: [v lineText]+[v eol])*)"; # TODO be more explicit
        }

        namespace eval forcedreflexmove {
	    set indicator {[?]![?]}
	    set RE "(?: +[v ordinalNumber][v move] $indicator[v eol])"
        }

        namespace eval tree {
	    set attackNumber "[v ordinalNumber]"
	    set defenseNumber "[v ordinalNumber]\\.{2}"

	    # in condition "lost pieces", lost pieces of the attacker may be removed
	    set zugzwangOrThreat "(?:[l zugzwang]|[l threat](?:[v pieceEffect])?)"

	    namespace eval keyline {
		set success {(?: [?!])}
		set RE "   [v attackNumber][v move](?:(?:[v undec])|${success}(?: [v zugzwangOrThreat])?)[v eol]"
	    }

            namespace eval attackline {
		set RE " +[v attackNumber][v move](?:(?:[v undec])|(?: [v zugzwangOrThreat])?)[v eol]"
	    }

            set threatLine $attackline::RE

            set defense " +$defenseNumber[v move]"

            namespace eval defenseline {
		set RE "[v defense](?:[v undec])?[v eol]"
	    }

            namespace eval checkOrZugzwangOrThreatLine {
		# TODO Popeye should write an empty line before the check indicator
		set RE "(?:(?: \[+]|[v emptyLine] [v zugzwangOrThreat])[v eol])"
	    }

            namespace eval postkeyplay {
                set RE "(?:(?:[v checkOrZugzwangOrThreatLine::RE])?(?:[v defenseline::RE](?: +[l refutes][v eol])?|[v attackline::RE])*)"
	    }

            namespace eval refutation {
                set RE "(?:[v defense] ![v eol](?:[v forcedreflexmove::RE])?)"
            }

            namespace eval refutationblock {
		set butLine " +[l but][v eol]"
                set RE "(?:$butLine[v refutation::RE]+)"
            }

            namespace eval fullphase {
                set playAfterKey "(?:(?:[v defenseline::RE]|[v attackline::RE])+)"
		set RE "(?:[v keyline::RE]$playAfterKey?[v refutationblock::RE]?[v emptyLine])"
	    }

            namespace eval setplay {
                set RE "[v fullphase::playAfterKey]"
	    }

            set regularplay "(?:[v moveNumberLineNonIntelligent]|$fullphase::RE)+"
            set RE "(?:$postkeyplay::RE|(?:(?:[v emptyLine]$setplay::RE)?(?:[v emptyLine]$regularplay)*))"
        }

        namespace eval line {
	    set subsequentMovePair "(?: +[v ordinalNumber][v move](?:[v undec][v eol]|(?: +[v move])+(?:[v undec][v eol])?))"
	    set moveNumberLine "[v moveNumberLineNonIntelligent]|[v moveNumberLineIntelligent]"

	    namespace eval helpplay {
		set finalMove "(?: +[v ordinalNumber][v move](?:[v undec][v eol])?)"

		# set play of h#n.5
		namespace eval twoEllipsis {
		    set firstMovePairSkipped "1[v ellipsis] +[v ellipsis]"
		    set line "(?: +$firstMovePairSkipped[v subsequentMovePair]*[v finalMove]?[v eol])"
		    set RE "(?:(?:$line|[v moveNumberLine])+)"
		}

                # set play of h#n or regular play of h#n.5
                namespace eval oneEllipsis {
		    set firstMoveSkipped "1[v ellipsis][v move](?:[v undec][v eol])?"
		    set line "(?: +$firstMoveSkipped[v subsequentMovePair]*[v finalMove]?[v eol])"
		    set RE "(?:(?:$line|[v moveNumberLine])+)"
		}

                # regular play of h#n
                namespace eval noEllipsis {
		    set firstMovePair "(?:1.[v move](?:[v undec][v eol]|(?: +[v move])+(?:[v undec][v eol])?))"
		    set line "(?: +$firstMovePair[v subsequentMovePair]*[v finalMove]?[v eol])"
		    set RE "(?:(?:$line|[v moveNumberLine])+)"
		}

                set RE "(?:$twoEllipsis::RE?[v emptyLine]?$oneEllipsis::RE?|$oneEllipsis::RE?[v emptyLine]?$noEllipsis::RE?)"
    	    }

            namespace eval seriesplay {
		set numberedMove "(?: +[v ordinalNumber][v move])"
		set line "(?:(?:$numberedMove|[v subsequentMovePair])+[v eol])"
		set RE "(?:(?:(?:[l setplayNotApplicable]|[l tryplayNotApplicable])[v eol]*)?[v emptyLine](?:$line|[v moveNumberLine])*)"
	    }

            set RE "(?:[v emptyLine]$helpplay::RE|$seriesplay::RE)"
        }

        namespace eval measurements {
	    set line {(?: *[[:alpha:]_]+: *[[:digit:]]+)}
	    set RE "(?:(?:$line[v eol])+)"
        }

        namespace eval kingmissing {
	    set RE "(?:[l kingmissing][v eol])"
        }

        namespace eval untwinned {
	    set problemignored "(?:(?:[l toofairy][v eol]|[l nonsensecombination][v eol]|[l conditionSideUndecidable][v eol][v emptyLine])[l problemignored][v eol])"
	    set simplex "(?:(?:[v emptyLine][l illegalSelfCheck]|[v emptyLine][v forcedreflexmove::RE]+|[v tree::RE]|[v line::RE])+)"
            # the last + should be {1,2}, but that would make the expression too complex
	    set RE "(?:[v kingmissing::RE]?(?:[l intelligentAndFairy])?(?:$problemignored|(?:$simplex[v measurements::RE])+)(?:[v remark::RE])?)"
        }

        namespace eval twinned {
	    # too complex for regexp
	    set RE "(?:[v twinning::RE][v untwinned::RE])+"
	    set separator [v twinning::RE]
        }

        # too complex for regexp
        set RE "(?:$untwinned::RE|$twinned::RE)"
    }

    namespace eval footer {
        set RE "(?:[v eol][l endOfSolution]|[l partialSolution])[v eol][v emptyLine][v emptyLine]"
    }

    # applying this gives an "expression is too complex" error :-(
    # dividing the input at recognized problem footers is also much, much faster...
    namespace eval problem {
        # too complex for regexp
        set RE "([v remark::RE](?:([v authoretc::RE])([v boardA::RE]?)([v board::RE])([v caption::RE])([v conditions::RE])([v duplex::RE])([v gridboard::RE]?))?([v solution::RE])([v footer::RE])"
    }

    namespace eval inputerror {
        set text "[v lineText]+"
        set RE "(?:[l inputError]:$text[v eol][l offendingItem]: $text[v eol])"
    }

    namespace eval zeroposition {
        set RE "(?:[v emptyLine][l zeroposition][v eol][v emptyLine])"
    }

    namespace eval beforesolution {
        set RE "^([v inputerror::RE]*)([v remark::RE ]?)(?:([v authoretc::RE])([v boardA::RE]?)([v board::RE])([v caption::RE])([v conditions::RE])([v duplex::RE])([v gridboard::RE]?))?([v zeroposition::RE]?)"
    }
}

set f [open $inputfile "r"]
set input [read $f]
close $f

proc printSection {debugPrefix section} {
    if {[lindex $::sections 0]=="debug"} {
	foreach line [split [regsub "\n$" $section ""] "\n"] {
	    puts "$debugPrefix:$line@"
	}
    } else {
	puts -nonewline $section
    }
}

proc handleTextBeforeSolution {beforesol} {
    if {[regexp $format::beforesolution::RE $beforesol - inputerrors remark authoretc boardA board caption conditions duplex gridboard zeroposition]
	&& ([regexp -- {[^[:space:]]} $inputerrors]
	    || [regexp -- {[^[:space:]]} $remark]
	    || [regexp -- {[^[:space:]]} $board])} {
	printSection "i" $inputerrors
	printSection "r" $remark
	printSection "a" $authoretc
	printSection "ba" $boardA
	printSection "b" $board
	printSection "ca" $caption
	printSection "co" $conditions
	printSection "d" $duplex
	printSection "g" $gridboard
	printSection "z" $zeroposition
    }
}

proc handleSolutionWithoutTwinning {beforeFooter} {
    set noboardExpr "^()($format::solution::untwinned::RE)\$"
    # make sure that the board caption, conditions or whatever comes last ends with a newline character
    set withBoardExpr "^(.*?\n)($format::solution::untwinned::RE)\$"
    # we have to test using the noboard expression first - the with board expression will match some
    # output created with option noboard and yield "interesting" results
    if {[regexp -- $noboardExpr $beforeFooter -  beforesol solution]
        || [regexp -- $withBoardExpr $beforeFooter - beforesol solution]} {
	handleTextBeforeSolution $beforesol
	printSection "s" $solution
    } else {
	# input error ...
	handleTextBeforeSolution $beforeFooter
    }
}

proc makeSegments {beforeFooter twinningIndices} {
    set segments {}
    set startOfNextSegment 0
    foreach pair $twinningIndices {
	foreach {twinningStart twinningEnd} $pair break
	lappend segments [string range $beforeFooter $startOfNextSegment [expr {$twinningStart-1}]]
	lappend segments [string range $beforeFooter $twinningStart $twinningEnd]
	set startOfNextSegment [expr {$twinningEnd+1}]
    }
    lappend segments [string range $beforeFooter $startOfNextSegment "end"]
    return $segments
}

proc handleSolutionWithPresumableTwinning {beforeFooter twinningIndices} {
    set segments [makeSegments $beforeFooter $twinningIndices]
    set firstTwin true
    set beforeSolution [lindex $segments 0]
    foreach {twinning solution} [lrange $segments 1 "end"] {
	if {[regexp $format::solution::untwinned::RE $solution]} {
	    if {$firstTwin} {
		handleTextBeforeSolution $beforeSolution
		set firstTwin false
	    }
	    printSection "t" $twinning
	    printSection "s" $solution
	} else {
	    append beforeSolution "$twinning$solution"
	}
    }
    if {$firstTwin} {
	# there was some "fake twinning" in a text field
	handleSolutionWithoutTwinning $beforeFooter
    }
}

if {[llength $sections]==0 || [lindex $sections 0]=="debug"} {
    set footerIndices [regexp -all -indices -inline $format::footer::RE $input]
    set nextProblemStart 0
    foreach footerIndexPair $footerIndices {
	foreach {footerStart footerEnd} $footerIndexPair break
	set footer [string range $input $footerStart $footerEnd]
	set beforeFooter [string range $input $nextProblemStart [expr {$footerStart-1}]]
	set nextProblemStart [expr {$footerEnd+1}]
	set twinningIndices [regexp -all -inline -indices $format::solution::twinned::separator $beforeFooter]
	if {[llength $twinningIndices]==0} {
	    handleSolutionWithoutTwinning $beforeFooter
	} else {
	    handleSolutionWithPresumableTwinning $beforeFooter $twinningIndices
	}
	printSection "f" $footer
    }
} else {
    set expr ""
    foreach section $sections {
	append expr [set format::${section}::RE]
    }
    foreach match [regexp -all -inline $expr $input] {
	puts -nonewline $match
	puts "===="
    }
}
