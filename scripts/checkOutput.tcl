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
    set legailtyUndecidable "kann nicht entscheiden, ob dieser Zug legal ist."
    set illegalSelfCheck "Die am Zug befindliche Partei kann den Koenig schlagen"
    set roleExchange "Rollentausch"
    set refutes "Widerlegt."
    set toofairy "Zu viel Maerchenschach fuer neutrale Steine\nProblem uebersprungen"
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
    set toofairy "too much fairy chess for neutral pieces\nproblem ignored"
    # TODO correct English sentences?
    set legailtyUndecidable "can't decide whether this move is legal."
    set roleExchange "role exchange"
    set refutes "Refutes."
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
    set ellipsis {[.][.][.]}
    set pieceChar {[[:upper:][:digit:]]}
    set hunterSuffix "(?:/$pieceChar{1,2})"
    set piecePawnImplicit "$pieceChar{0,2}$hunterSuffix?"
    set piece "(?:$pieceChar{1,2}$hunterSuffix?)"
    set square {[a-h][1-8]}
    set captureOrNot {[-*]}
    set castlingQ "0-0-0"
    set castlingK "0-0"
    set movement "(?:[set ${language}::pieceAttributeShortcut]?$piecePawnImplicit$square$captureOrNot$square|$castlingQ|$castlingK)"
    set antimarsMovement "-$square"
    set messignyExchange "(?:$piece$square<->$piece$square)"
    set takeAndMakeAndTake "(?:$captureOrNot$square)"
    # TODO Popeye should write hn here, not just h
    # TODO Popeye should write sfw/hcc? here, not just s resp. h
    # TODO order of h(n), c, s(fw)
    set changeOfColor "(?:=[set ${language}::colorShortcut]h?c?s?)"
    set changeOfColorOtherPiece "(?:.$square$changeOfColor.)"
    set promotion "(?:=[set ${language}::colorShortcut]?h?c?s?$piece?)"
    set enPassant {(?: ep[.])}
    # TODO replace . by []
    # TODO why no brackets if on its own?? because we don't brackets if already inside brackets
    set chameleonization "(?:=[set ${language}::pieceAttributeShortcut]?$piece?)"
    # TODO why do we allow modification of arriving piece after -> and after =??
    set pieceMovement "(?:.[set ${language}::pieceAttributeShortcut]?$piece$square->[set ${language}::pieceAttributeShortcut]?$piece?${square}(?:=$piece$chameleonization?)?.)"
    set pieceAddition "(?:.\[+][set ${language}::pieceAttributeShortcut]?$piece${square}(?:=[set ${language}::colorShortcut]?$piece?$chameleonization?)?(?:->v)?.)"
    set pieceRemoval "(?:.-[set ${language}::pieceAttributeShortcut]?$piece$square.)"
    set imitatorMovement "(?:.I${square}(?:,$square)*.)"
    set paren_open {[(]}
    set paren_close {[)]}
    set bglNumber {[[:digit:]]+(?:[.][[:digit:]]{1,2})?}
    set bglBalance "(?: ${paren_open}(?:-|$bglNumber)(?:/$bglNumber)?$paren_close)"
    set checkIndicator {(?: [+])}
    # yes, this is slightly different from stipulation::goal!
    set goal {(?: (?:\#|=|dia|a=>b|z|ct|<>|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k[a-h][1-8]))}
    set castlingPartnerMovement $movement
    set kingOscillation "(?:.$piece$square<->$piece$square.)"
    set singleBoxPromotion "(?:.$square=$piece.)"
    set kobulChange "(?:.$square=[set ${language}::pieceAttributeShortcut]?$piece.)"
    set totalInvisibleMove "TI~-~"
    set totalInvisibleCapture "TI~\\*$square"
    set totalInvisibleInsertion "(?:.\\+[set ${language}::colorShortcut]$piece$square.)"
    set totalInvisibleRevelation "(?:.$square=[set ${language}::colorShortcut]$piece.)"
    set move "(?: [set ${language}::roleExchange]| $ellipsis|(?:$singleBoxPromotion?$totalInvisibleInsertion?(?:$movement|$totalInvisibleMove|$totalInvisibleCapture)(?:$antimarsMovement)?|$messignyExchange)(?:/$castlingPartnerMovement)?$takeAndMakeAndTake?$enPassant?$imitatorMovement?$promotion?$chameleonization?$changeOfColor?$kobulChange?$pieceMovement?$pieceAddition*$pieceRemoval?$changeOfColorOtherPiece*$kingOscillation?$singleBoxPromotion?$totalInvisibleRevelation*$bglBalance?$checkIndicator?)$goal?"

    set moveNumber {[1-9][0-9]*}
    set moveNumberLine "(?: *$moveNumber  \[(]$move \[)]\n)"

    set nrPositions {[[:digit:]]+}
    set nrMoves {[[:digit:]]+[+][[:digit:]]+}
    set moveNumberLineIntelligent "$nrPositions [set ${language}::potentialPositionsIn] $nrMoves\n"

    namespace eval twinning {
	set combined "(?:$solution::emptyLine\[+\]?\[a-z]\\) \[^\n\]*\n(?: \[^\n\]+\n)*)"; # TODO be more explicit
    }

    namespace eval tree {
	set ordinalNumber {[1-9][0-9]*[.]}
	set attackNumber $ordinalNumber
	set defenseNumber "$ordinalNumber\\.{2}"

	set zugzwangOrThreat "(?:[set ${language}::zugzwang]|[set ${language}::threat](?:$solution::pieceRemoval)?)"

	namespace eval keyline {
	    set success {(?: [?!])}
	    set combined "   $solution::tree::attackNumber${solution::move}(?:(?: [set ${language}::legailtyUndecidable])|${success}(?: $solution::tree::zugzwangOrThreat)?)\n"
	}

        namespace eval attackline {
	    set combined " +$solution::tree::attackNumber${solution::move}(?:(?: [set ${language}::legailtyUndecidable])|(?: $solution::tree::zugzwangOrThreat)?)\n"
	}

	set threatLine $attackline::combined

	set defense " +$defenseNumber$solution::move"

        namespace eval defenseline {
	    set combined "${solution::tree::defense}(?: [set ${language}::legailtyUndecidable])?\n"
	}

        namespace eval zugzwangOrThreatLine {
	    set combined "(?: $solution::tree::zugzwangOrThreat\n)"
	}

        namespace eval postkeyplay {
            set combined "(?:$solution::tree::zugzwangOrThreatLine::combined?(?:$solution::tree::defenseline::combined|$solution::tree::attackline::combined| +[set ${language}::refutes]\n)+)"
	}

        namespace eval forcedreflexmove {
	    set indicator {[?]![?]}
	    set combined "(?: +$solution::tree::attackNumber$solution::move $indicator\n)"
	}

        namespace eval refutationline {
            set combined "(?:$solution::tree::defense !\n(?:$solution::tree::forcedreflexmove::combined)?)"
        }

        namespace eval refutationblock {
	    set butLine " +[set ${language}::but]\n"
            set combined "(?:$butLine$solution::tree::refutationline::combined+)"
        }

        namespace eval fullphase {
            set playAfterKey "(?:(?:$solution::tree::defenseline::combined|$solution::tree::attackline::combined)+)"
	    set combined "(?:$solution::tree::keyline::combined$playAfterKey?$solution::tree::refutationblock::combined?$solution::emptyLine)"
	}

        namespace eval setplay {
            set combined "(?:(?:$solution::tree::defenseline::combined|$solution::tree::attackline::combined)+)"
	}

        set combined "(?:(?:$solution::checkIndicator)?\n(?:$forcedreflexmove::combined+|$postkeyplay::combined|(?:$setplay::combined$solution::emptyLine)?(?:$solution::moveNumberLine|$fullphase::combined)+))"
    }

    namespace eval line {
	set ordinalNumber {[1-9][0-9]*[.]}
	set undec " [set ${language}::legailtyUndecidable]"

	set firstMovePair "(?:1.${solution::move}(?:$undec\n| +${solution::move}(?:$undec\n)?))"
	set firstMoveSkipped "1$solution::ellipsis${solution::move}(?:$undec\n)?"
	set subsequentMovePair "(?: +$ordinalNumber${solution::move}(?:$undec\n| +${solution::move}(?:$undec\n)?))"
	set finalMove "(?: +$ordinalNumber${solution::move}(?:$undec\n)?)"

	namespace eval regularplay {
	    set firstMovePair "(?:$solution::line::firstMoveSkipped|$solution::line::firstMovePair)"
	    set line "(?: +$firstMovePair$solution::line::subsequentMovePair*$solution::line::finalMove?\n)"
	    set combined "(?:${solution::emptyLine}(?:$line|$solution::moveNumberLine|$solution::moveNumberLineIntelligent)+)"
	}

	namespace eval setplay {
	    set firstMovePairSkipped "1$solution::ellipsis +$solution::ellipsis"
	    set firstMovePair "(?:$firstMovePairSkipped|$solution::line::firstMoveSkipped|$solution::line::firstMovePair)"
	    set line "(?: +$firstMovePair$solution::line::subsequentMovePair*\n)"
	    set combined "(?:${solution::emptyLine}(?:$line|$solution::moveNumberLineIntelligent)+)"
	}

	namespace eval seriesplay {
	    set numberedMove "(?: +$solution::line::ordinalNumber$solution::move)"
	    set line "(?:(?:$numberedMove|$solution::line::subsequentMovePair)+\n)"
	    set combined "(?:${solution::emptyLine}(?:$line|$solution::moveNumberLine|$solution::moveNumberLineIntelligent)+)"
	}

	set combined "$setplay::combined?(?:$regularplay::combined|$seriesplay::combined)"
    }

    namespace eval measurements {
	set line {(?: *[[:alpha:]_ ]+: *[[:digit:]]+\n)}
	set combined "(?:$line+)"
    }

    namespace eval kingmissing {
	set combined "(?:[set ${language}::kingmissing]\n)"
    }

    namespace eval untwinned {
        # the last + should be {1,2}, but that would make the expression too complex
	set combined "(?:$solution::kingmissing::combined?(?:[set ${language}::toofairy])?(?:(?:${solution::emptyLine}(?:[set ${language}::illegalSelfCheck] +)?|(?:$solution::tree::combined*|$solution::line::combined)+)$solution::measurements::combined)+(?:$remark::combined)?)"
    }

    namespace eval twinned {
	# too complex for regexp
	set combined "(?:$solution::twinning::combined$solution::untwinned::combined)+"
	set separator $solution::twinning::combined
    }

    # too complex for regexp
    set combined "(?:$untwinned::combined|$twinned::combined)"
}

namespace eval footer {
    set emptyLine "\n"
    set combined "[set ${language}::endlines]\n$emptyLine"
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
    set combined "(?:$emptyLine[set ${language}::zeroposition]\n\n)"
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

proc handleTextBeforeSolution {beforesol} {
    set inputerrors [regexp -all -inline $inputerror::combined $beforesol]
    if {[llength $inputerrors]>0} {
	foreach inputerror $inputerrors {
	    printSection "i" $inputerror
	}
    } elseif {[regexp $beforesolution::combined $beforesol match remark authoretc boardA board caption conditions gridboard zeroposition]
	      && ([regexp -- {[^[:space:]]} $remark] || [regexp -- {[^[:space:]]} $board])} {
	printSection "r" $remark
	printSection "a" $authoretc
	printSection "ba" $boardA
	printSection "b" $board
	printSection "ca" $caption
	printSection "co" $conditions
	printSection "g" $gridboard
	printSection "z" $zeroposition
	return true
    } else {
	return false
    }
}

proc handleSolutionWithoutTwinning {beforeFooter} {
    set solutionIndices [regexp -all -inline -indices $solution::untwinned::combined $beforeFooter]
    if {[llength $solutionIndices]>0} {
	# we either have 0 or 1 solutions here
	expr {1/(1==[llength $solutionIndices])}
	# TODO decompose further
	foreach {solutionStart solutionEnd} [lindex $solutionIndices 0] break
	handleTextBeforeSolution [string range $beforeFooter 0 [expr {$solutionStart-1}]]
	set solution [string range $beforeFooter $solutionStart $solutionEnd]
	printSection "s" $solution
    } else {
	# input error ...
	handleTextBeforeSolution $beforeFooter
    }
}

proc handleTwinSolution {twinSolution} {
    set solutionIndices [regexp -all -inline -indices $solution::untwinned::combined $twinSolution]
    # we have 1 solution here
    # TODO decompose further
    expr {1/(1==[llength $solutionIndices])}
    foreach {solutionStart solutionEnd} [lindex $solutionIndices 0] break
    set solution [string range $twinSolution $solutionStart $solutionEnd]
    printSection "s" $solution
}

proc findFirstTwinning {beforeFooter twinningIndices} {
    foreach pair $twinningIndices {
	foreach {twinningStart twinningEnd} $pair break
	set beforesol [string range $beforeFooter 0 $twinningStart]
	if {[handleTextBeforeSolution $beforesol]} {
	    return $twinningStart
	}
    }

    return -1
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
	    set firstTwinningStart [findFirstTwinning $beforeFooter $twinningIndices]
	    if {$firstTwinningStart==-1} {
		# "fake twinning", e.g. remark a) blabla
		handleSolutionWithoutTwinning $beforeFooter
	    } else {
		set prevSolutionStart 0
		foreach pair $twinningIndices {
		    foreach {twinningStart twinningEnd} $pair break
		    if {$twinningStart>=$firstTwinningStart} {
			if {$prevSolutionStart>0} {
			    set twinSolution [string range $beforeFooter $prevSolutionStart [expr {$twinningStart-1}]]
			    handleTwinSolution $twinSolution
			}
			set nextTwinning [string range $beforeFooter $twinningStart $twinningEnd]
			printSection "t" $nextTwinning
			set prevSolutionStart [expr {$twinningEnd+1}]
		    }
		}
		set twinSolution [string range $beforeFooter $prevSolutionStart "end"]
		handleTwinSolution $twinSolution
	    }
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
