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
    set endlines {(?:\nLoesung beendet[.]|Partielle Loesung)\n}
    set white {Weiss}
    set black {Schwarz}
    set zugzwang {Zugzwang[.]}
    set threat "Drohung:"
    set but {Aber}
    set colorShortcut {(?:[wsn])}
    # TODO order of attributes?
    set pieceAttributeShortcut {(?:[wsn]?k?(?:hn)?c?b?p?f?(?:sfw)?j?v?k?u?m?)}
    set zeroposition "NullStellung"
    set potentialPositionsIn "moegliche Stellungen in"
    # TODO why does this in with a \n in German, but not in English???
    set kingmissing "Es fehlt ein weisser oder schwarzer Koenig\n"
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
}

namespace eval english {
    set endlines {(?:\nsolution finished[.]|Partial solution)\n}
    # TODO why is this written inconsistently?
    set white {[Ww]hite}
    set black {[Bb]lack}
    set zugzwang {zugzwang[.]}
    set threat "threat:"
    set but {but}
    set colorShortcut {(?:[wbn])}
    # TODO is "f" correct for functionary piece?
    # TODO is hcc correct for hurdle colour changing?
    set pieceAttributeShortcut {(?:[wbn]?r?(?:hn)?c?f?p?(?:hcc)?j?v?k?u?m?)}
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

namespace eval remark {
    set emptyLine {\n}

    set remark {[^\n]+}
    set remarkLine "(?:$remark\n)"

    set combined "(?:$remarkLine+$emptyLine*)"
}

namespace eval authoretc {
    set emptyLine {\n}
    set leadingBlanks { *}

    set authorOriginAwardTitle {[^\n]+}
    set authorOriginAwardTitleLine "(?:$leadingBlanks$authorOriginAwardTitle\n)"

    set combined "$emptyLine?$authorOriginAwardTitleLine*$emptyLine"
}

namespace eval board {
    set nrRows 8
    set nrColumns 8
    
    set emptyLine {\n}

    set cornerSign {[+]}
    set verticalBorderSign {[|]}
    set horizontalBorderSign "-"
    
    set columnName {[a-h]}
    set columnSpec "$horizontalBorderSign$horizontalBorderSign$columnName$horizontalBorderSign"
    set columns "$cornerSign${horizontalBorderSign}(?:$columnSpec){$nrColumns}$horizontalBorderSign$horizontalBorderSign$cornerSign\n"

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
    set piecesLine "${rowNo} (?:$pieceSpec){$nrColumns}  $rowNo\n"

    set hunter2ndPart "(?:$hole|$piece1Char|$piece2Chars)"
    set spaceLine "$verticalBorderSign (?:$hunter2ndPart$pieceSpecSeparator|$gridHorizontal){$nrColumns}  $verticalBorderSign\n"

    set combined "(?:${columns}(?:$spaceLine$piecesLine){$nrRows}$spaceLine$columns)"
}

namespace eval gridboard {
    set emptyLine {\n}

    namespace eval cellsline {
	set cell {[ [:digit:]][[:digit:]]}
	set combined "${board::rowNo}(?:  $cell){$board::nrColumns}   $board::rowNo\n"
    }

    set combined "(?:$emptyLine${board::columns}(?:$board::spaceLine$cellsline::combined){$board::nrRows}$board::spaceLine$board::columns)"
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

    set combined "(?:(?:$stipulation_traditional|$stipulation_structured)(?:$maxthreat$maxflight?)?$nontrivial?)?"; # TODO order of suffixes?
}
    
namespace eval pieceControl {
    set piecesOfColor {[[:digit:]]+}
    set plus {[+]}
    set combined "$piecesOfColor $plus ${piecesOfColor}(?: $plus ${piecesOfColor}n)?(?: $plus ${piecesOfColor} TI)?"
}

namespace eval caption {
    set stip_pieceControl " *$stipulation::combined *$pieceControl::combined\n"
    set duplex " *Duplex\n"
    set combined "${stip_pieceControl}(?:$duplex)?"
}

namespace eval boardA {
    set emptyLine {\n}
    # the caption of board A doesn't indicate the stipulation
    set caption " *$pieceControl::combined\n"
    set tomove "$stipulation::paren_open$stipulation::side ->$stipulation::paren_close"
    set combined "(?:$board::combined$boardA::caption\n *=> $tomove\n$emptyLine$emptyLine)"
}

namespace eval conditions {
    set line { *[^\n]+\n}
    set combined "(?:$line)*"
}

namespace eval solution {
    set emptyLine {\n}
    set ordinalNumber {[1-9][0-9]*[.]}
    set paren_open {[(]}
	set paren_close {[)]}
    set bracket_open {\[}
    set bracket_close {\]}
    set ellipsis {[.][.][.]}
    set pieceChar {[[:upper:][:digit:]]}
    set hunterSuffix "(?:/$pieceChar{1,2})"
    set piecePawnImplicit "$pieceChar{0,2}$hunterSuffix?"
    set piece "(?:$pieceChar{1,2}$hunterSuffix?)"
    set square {[a-h][1-8]}
    set capture {[*]}
    set captureOrNot {[-*]}
    set castlingQ "0-0-0"
    set castlingK "0-0"
    set movement "(?:(?:[l pieceAttributeShortcut]$piecePawnImplicit${square}$captureOrNot$square|$castlingQ|$castlingK)(?:$captureOrNot$square)*)"
    set messignyExchange "(?:$piece$square<->$piece$square)"
    # TODO Popeye should write hn here, not just h
    # TODO Popeye should write sfw/hcc? here, not just s resp. h
    # TODO order of h(n), c, s(fw)
    set promotion "(?:=[l pieceAttributeShortcut]h?c?s?$piece?)"
    set enPassant {(?: ep[.])}
    set vulcanization "(?:->v)"
    set pieceChangement "(?:$square$promotion)"
    set pieceSpec "[l pieceAttributeShortcut]$piece$square"
    set pieceMovement "(?:$pieceSpec->[l pieceAttributeShortcut]$piece?$square$promotion*$vulcanization?)"
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
    set moveNumberLineNonIntelligent "(?: *$moveNumber  \[(]$move \[)]\n)"

    set nrPositions {[[:digit:]]+}
    set nrMoves {[[:digit:]]+[+][[:digit:]]+}
    set moveNumberLineIntelligent "$nrPositions [l potentialPositionsIn] $nrMoves\n"

    set undec "(?: (?:[l legalityUndecidable]|[l refutationUndecidable]))"

    namespace eval twinning {
	set combined "(?:[v emptyLine]\[+\]?\[a-z]\\) \[^\n\]*\n(?: \[^\n\]+\n)*)"; # TODO be more explicit
    }

    namespace eval forcedreflexmove {
	set indicator {[?]![?]}
	set combined "(?: +[v ordinalNumber][v move] $indicator\n)"
    }

    namespace eval tree {
	set attackNumber "[v ordinalNumber]"
	set defenseNumber "[v ordinalNumber]\\.{2}"

	# in condition "lost pieces", lost pieces of the attacker may be removed
	set zugzwangOrThreat "(?:[l zugzwang]|[l threat](?:[v pieceEffect])?)"

	namespace eval keyline {
	    set success {(?: [?!])}
	    set combined "   [v attackNumber][v move](?:(?:[v undec])|${success}(?: [v zugzwangOrThreat])?)\n"
	}

        namespace eval attackline {
	    set combined " +[v attackNumber][v move](?:(?:[v undec])|(?: [v zugzwangOrThreat])?)\n"
	}

	set threatLine $attackline::combined

	set defense " +$defenseNumber[v move]"

        namespace eval defenseline {
	    set combined "[v defense](?:[v undec])?\n"
	}

        namespace eval checkOrZugzwangOrThreatLine {
	    # TODO Popeye should write an empty line before the check indicator
	    set combined "(?:(?: \[+]|[v emptyLine] [v zugzwangOrThreat])\n)"
	}

        namespace eval postkeyplay {
            set combined "(?:(?:[v checkOrZugzwangOrThreatLine::combined])?(?:[v defenseline::combined](?: +[l refutes]\n)?|[v attackline::combined])*)"
	}

        namespace eval refutation {
            set combined "(?:[v defense] !\n(?:[v forcedreflexmove::combined])?)"
        }

        namespace eval refutationblock {
	    set butLine " +[l but]\n"
            set combined "(?:$butLine[v refutation::combined]+)"
        }

        namespace eval fullphase {
            set playAfterKey "(?:(?:[v defenseline::combined]|[v attackline::combined])+)"
	    set combined "(?:[v keyline::combined]$playAfterKey?[v refutationblock::combined]?[v emptyLine])"
	}

        namespace eval setplay {
            set combined "[v fullphase::playAfterKey]"
	}

        set regularplay "(?:[v moveNumberLineNonIntelligent]|$fullphase::combined)+"
        set combined "(?:$postkeyplay::combined|(?:(?:[v emptyLine]$setplay::combined)?(?:[v emptyLine]$regularplay)*))"
    }

    namespace eval line {
	set subsequentMovePair "(?: +[v ordinalNumber]${solution::move}(?:[v undec]\n|(?: +[v move])+(?:[v undec]\n)?))"
	set moveNumberLine "[v moveNumberLineNonIntelligent]|[v moveNumberLineIntelligent]"

	namespace eval helpplay {
	    set finalMove "(?: +[v ordinalNumber]${solution::move}(?:[v undec]\n)?)"

	    # set play of h#n.5
	    namespace eval twoEllipsis {
		set firstMovePairSkipped "1[v ellipsis] +[v ellipsis]"
		set line "(?: +$firstMovePairSkipped[v subsequentMovePair]*[v finalMove]?\n)"
		set combined "(?:(?:$line|[v moveNumberLine])+)"
	    }

	    # set play of h#n or regular play of h#n.5
            namespace eval oneEllipsis {
		set firstMoveSkipped "1[v ellipsis][v move](?:[v undec]\n)?"
		set line "(?: +$firstMoveSkipped[v subsequentMovePair]*[v finalMove]?\n)"
		set combined "(?:(?:$line|[v moveNumberLine])+)"
	    }

	    # regular play of h#n
            namespace eval noEllipsis {
		set firstMovePair "(?:1.${solution::move}(?:[v undec]\n|(?: +[v move])+(?:[v undec]\n)?))"
		set line "(?: +$firstMovePair[v subsequentMovePair]*[v finalMove]?\n)"
		set combined "(?:(?:$line|[v moveNumberLine])+)"
	    }

	    set combined "(?:$twoEllipsis::combined+|$oneEllipsis::combined+|$noEllipsis::combined+)"
	}

	namespace eval seriesplay {
	    set numberedMove "(?: +[v ordinalNumber][v move])"
	    set line "(?:(?:$numberedMove|[v subsequentMovePair])+\n)"
	    set combined "(?:(?:$line|[v moveNumberLine])+)"
	}

	set combined "[v emptyLine](?:$helpplay::combined|$seriesplay::combined)"
    }

    namespace eval measurements {
	set line {(?: *[[:alpha:]_]+: *[[:digit:]]+\n)}
	set combined "(?:$line+)"
    }

    namespace eval kingmissing {
	set combined "(?:[l kingmissing]\n)"
    }

    namespace eval untwinned {
	set problemignored "(?:(?:[l toofairy]\n|[l intelligentAndFairy]|[l nonsensecombination]\n|[l conditionSideUndecidable]\n[v emptyLine])[l problemignored]\n)"
	set simplex "(?:[v emptyLine]|(?:[v emptyLine][l illegalSelfCheck]|[v emptyLine][v forcedreflexmove::combined]+|[v tree::combined]|[v line::combined])+)"
        # the last + should be {1,2}, but that would make the expression too complex
	set combined "(?:[v kingmissing::combined]?(?:$problemignored|(?:$simplex[v measurements::combined])+)(?:$remark::combined)?)"
    }

    namespace eval twinned {
	# too complex for regexp
	set combined "(?:[v twinning::combined][v untwinned::combined])+"
	set separator [v twinning::combined]
    }

    # too complex for regexp
    set combined "(?:$untwinned::combined|$twinned::combined)"
}

namespace eval footer {
    set emptyLine "\n"
    set combined "[l endlines]\n$emptyLine"
}

# applying this gives an "expression is too complex" error :-(
# dividing the input at recognized problem footers is also much, much faster...
namespace eval problem {
    # too complex for regexp
    set combined "($remark::combined)(?:($authoretc::combined)($boardA::combined?)($board::combined)($caption::combined)($conditions::combined)($gridboard::combined?))?($solution::combined)($footer::combined)"
}

namespace eval inputerror {
    # TODO i10n
    set combined {(?:input-error:[^\n]+\n[^\n]+\n)}
}

namespace eval zeroposition {
    set emptyLine {\n}
    set combined "(?:$emptyLine[l zeroposition]\n\n)"
}

namespace eval beforesolution {
    set combined "^($remark::combined?)(?:($authoretc::combined)($boardA::combined?)($board::combined)($caption::combined)($conditions::combined)($gridboard::combined?))?($zeroposition::combined?)"
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

proc handleFieldsBeforeSolution {beforesol} {
    if {[regexp $beforesolution::combined $beforesol match remark authoretc boardA board caption conditions gridboard zeroposition]
	&& ([regexp -- {[^[:space:]]} $remark] || [regexp -- {[^[:space:]]} $board])} {
	printSection "r" $remark
	printSection "a" $authoretc
	printSection "ba" $boardA
	printSection "b" $board
	printSection "ca" $caption
	printSection "co" $conditions
	printSection "g" $gridboard
	printSection "z" $zeroposition
    }
}

proc handleTextBeforeSolution {beforesol} {
    if {$beforesol=="\n"} {
	# empty solution
    } else {
	if {[regexp -- "^($inputerror::combined+)(.*)\$" $beforesol - inputerrors beforesol]} {
	    printSection "i" $inputerrors
	}
	handleFieldsBeforeSolution $beforesol
    }
}

proc handleSolutionWithoutTwinning {beforeFooter} {
    if {[regexp -- "^(.*?)($solution::untwinned::combined)\$" $beforeFooter - beforesol solution]} {
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
	if {[regexp $solution::untwinned::combined $solution]} {
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
    set footerIndices [regexp -all -indices -inline $footer::combined $input]
    set nextProblemStart 0
    foreach footerIndexPair $footerIndices {
	foreach {footerStart footerEnd} $footerIndexPair break
	set footer [string range $input $footerStart $footerEnd]
	set beforeFooter [string range $input $nextProblemStart [expr {$footerStart-1}]]
	set nextProblemStart [expr {$footerEnd+1}]
	set twinningIndices [regexp -all -inline -indices $solution::twinned::separator $beforeFooter]
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
	append expr [set ${section}::combined]
    }
    foreach match [regexp -all -inline $expr $input] {
	puts -nonewline $match
	puts "===="
    }
}
