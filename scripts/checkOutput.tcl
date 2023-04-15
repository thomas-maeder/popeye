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
    set pieceAttributeShortcut {(?:[wsn]?k?(?:hn)?c?b?p?f?(?:sfw)?j?v?)}
    set zeroposition "NullStellung"
    set potentialPositionsIn "moegliche Stellungen in"
    set kingmissing "Es fehlt ein weisser oder schwarzer Koenig"
    set legailtyUndecidable "kann nicht entscheiden, ob dieser Zug legal ist."
}

namespace eval english {
    set endlines {(?:\nsolution finished[.]|Partial solution)\n}
    set white {white}
    set black {black}
    set zugzwang {Zugzwang[.]}
    set threat "Threat:"
    set but {But}
    set colorShortcut {(?:[wbn])}
    # TODO is "f" correct for functionary piece?
    # TODO is hcc correct for hurdle colour changing?
    set pieceAttributeShortcut {(?:[wbn]?r?(?:hn)?c?f?p?(?:hcc)?j?v?)}
    set zeroposition "zeroposition"
    set potentialPositionsIn "potential positions in"
    set kingmissing "both sides need a king"
    # TODO correct English sentence?
    set legailtyUndecidable "can't decide whether this move is legal."
}

namespace eval intro {
    set emptyLine {\n}
    set leadingBlanks { *}

    set remark {[^\n]+}
    set remarkLine "(?:$remark\n)"

    set authorOriginAwardTitle {[^\n]+}
    set authorOriginAwardTitleLine "(?:$leadingBlanks$authorOriginAwardTitle\n)"

    set combined "$remarkLine*$emptyLine$authorOriginAwardTitleLine*$emptyLine"
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

    set combined "${columns}(?:$spaceLine$piecesLine){$nrRows}$spaceLine$columns"
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
    set stipulation_structured [subst -nocommands {(?:$side [^ ]+)}]; # TODO

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
    set combined "(?:$board::combined$boardA::caption\n$emptyLine *=> $tomove\n)"
    set combined "(?:$board::combined$boardA::caption\n *=> $tomove\n$emptyLine$emptyLine)?"
}

namespace eval conditions {
    set line { *[^\n]+\n}
    set combined "(?:$line)*"
}

namespace eval solution {
    set emptyLine {\n}
    set pieceChar {[[:upper:][:digit:]]}
    set hunterSuffix "(?:/$pieceChar{1,2})"
    set piecePawnImplicit "$pieceChar{0,2}$hunterSuffix?"
    set piece "(?:$pieceChar{1,2}$hunterSuffix?)"
    set square {[a-h][1-8]}
    set captureOrNot {[-*]}
    set castlingQ "0-0-0"
    set castlingK "0-0"
    set movement "(?:[set ${language}::pieceAttributeShortcut]?$piecePawnImplicit$square$captureOrNot$square|$castlingQ|$castlingK)"
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
    set pieceAddition "(?:.\[+][set ${language}::pieceAttributeShortcut]?$piece${square}(?:=[set ${language}::colorShortcut]?$piece?$chameleonization?)?.)"
    set pieceRemoval "(?:.-[set ${language}::pieceAttributeShortcut]?$piece$square.)"
    set messignyExchange "(?:.$piece$square<->$piece$square.)"
    set imitatorMovement "(?:.I${square}(?:,$square)*.)"
    set paren_open {[(]}
    set paren_close {[)]}
    set bglNumber {[[:digit:]]+(?:[.][[:digit:]]{1,2})?}
    set bglBalance "(?: ${paren_open}(?:-|$bglNumber)(?:/$bglNumber)?$paren_close)"
    set checkIndicator {(?: [+])}
    # yes, this is slightly different from stipulation::goal!
    set goal {(?: (?:\#|=|dia|a=>b|z|ct|<>|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k[a-h][1-8]))}
    set castlingPartnerMovement $movement
    set move "${movement}(?:/$castlingPartnerMovement)?$takeAndMakeAndTake?$enPassant?$imitatorMovement?$promotion?$chameleonization?$changeOfColor?$pieceMovement?$pieceAddition?$pieceRemoval?$messignyExchange?$changeOfColorOtherPiece?$bglBalance?$checkIndicator?$goal?"

    set moveNumber {[1-9][0-9]*}
    set moveNumberLine "(?: +$moveNumber  \[(]$move \[)]\n)"

    set nrPositions {[[:digit:]]+}
    set nrMoves {[[:digit:]]+[+][[:digit:]]+}
    set moveNumberLineIntelligent "$nrPositions [set ${language}::potentialPositionsIn] $nrMoves\n"

    namespace eval twinning {
	set combined "$solution::emptyLine\[+\]?\[a-z]\\) \[^\n\]*\n(?: \[^\n\]+\n)*"; # TODO be more explicit
    }

    namespace eval zeroposition {
	set combined "(?:$solution::emptyLine[set ${language}::zeroposition]\n\n)"
    }

    namespace eval tree {
	set ordinalNumber {[1-9][0-9]*[.]}
	set attackNumber $ordinalNumber
	set defenseNumber "$ordinalNumber\\.{2}"

	set zugzwangOrThreat "(?: (?:[set ${language}::zugzwang]|[set ${language}::threat]))?"

	namespace eval keyline {
	    set success {(?: [?!])}
	    set combined "   $solution::tree::attackNumber${solution::move}(?:(?: [set ${language}::legailtyUndecidable])|$success$solution::tree::zugzwangOrThreat)\n"
	}

        namespace eval attackline {
	    set combined " +$solution::tree::attackNumber${solution::move}(?:(?: [set ${language}::legailtyUndecidable])|$solution::tree::zugzwangOrThreat)\n"
	}

	set threatLine $attackline::combined

	set defense " +$defenseNumber$solution::move"

        namespace eval defenseline {
	    set combined "${solution::tree::defense}(?: [set ${language}::legailtyUndecidable])?\n"
	}

        namespace eval postkeyplay {
            set combined "(?:(?:$solution::tree::zugzwangOrThreat)(?:$solution::tree::defenseline::combined|$solution::tree::attackline::combined)+)"
	}

        namespace eval fullphase {
	    set butLine "    [set ${language}::but]\n"
	    set forcedReflexMoveIndicator {[?]![?]}
	    set forcedReflexMove " +$solution::tree::attackNumber$solution::move $forcedReflexMoveIndicator"
	    set refutationLine "(?:$solution::tree::defense !\n(?:$forcedReflexMove\n)?)"
	    set refutationBlock "(?:$butLine$refutationLine+)"
	    set combined "(?:$solution::tree::keyline::combined$solution::tree::postkeyplay::combined?$refutationBlock?$solution::emptyLine)"
	}

        namespace eval setplay {
	    set combined $solution::tree::postkeyplay::combined
	}

        set combined "(?:${solution::emptyLine}(?:$setplay::combined$solution::emptyLine)?(?:$solution::moveNumberLine|$fullphase::combined)+)"
    }

    namespace eval line {
	set ordinalNumber {[1-9][0-9]*[.]}
	set ellipsis {[.][.][.]}
	set undec " [set ${language}::legailtyUndecidable]"

	set firstMovePair "(?:1.${solution::move}(?:$undec\n| +${solution::move}(?:$undec\n)?))"
	set firstMoveSkipped "1$ellipsis${solution::move}(?:$undec\n)?"
	set subsequentMovePair "(?: +$ordinalNumber${solution::move}(?:$undec\n| +${solution::move}(?:$undec\n)?))"
	set finalMove "(?: +$ordinalNumber${solution::move}(?:$undec\n)?)"

	namespace eval regularplay {
	    set firstMovePair "(?:$solution::line::firstMoveSkipped|$solution::line::firstMovePair)"
	    set line "(?: +$firstMovePair$solution::line::subsequentMovePair*$solution::line::finalMove?\n)"
	    set combined "(?:${solution::emptyLine}(?:$line|$solution::moveNumberLine|$solution::moveNumberLineIntelligent)+)"
	}

	namespace eval setplay {
	    set firstMovePairSkipped "1$solution::line::ellipsis +$solution::line::ellipsis"
	    set firstMovePair "(?:$firstMovePairSkipped|$solution::line::firstMoveSkipped|$solution::line::firstMovePair)"
	    set line "(?: +$firstMovePair$solution::line::subsequentMovePair*\n)"
	    set combined "(?:${solution::emptyLine}(?:$line|$solution::moveNumberLineIntelligent)+)"
	}

	namespace eval seriesplay {
	    set numberedMove "(?: +$solution::line::ordinalNumber$solution::move)"
	    set line "(?:$numberedMove*(?:$numberedMove|$solution::line::subsequentMovePair)\n)"
	    set combined "(?:${solution::emptyLine}(?:$line|$solution::moveNumberLine|$solution::moveNumberLineIntelligent)+)"
	}

	set combined "$setplay::combined?(?:$regularplay::combined|$seriesplay::combined)"
    }

    namespace eval measurements {
	set line {(?: *[a-z_]+: *[0-9]+\n)}
	set combined "(?:$line{4})"
    }

    namespace eval untwinned {
	set combined "(?:$solution::emptyLine|$solution::tree::combined*|$solution::line::combined)(?:$solution::measurements::combined)"
    }

    namespace eval twinned {
	set combined "$solution::zeroposition::combined?(?:$solution::twinning::combined$solution::untwinned::combined)+"
    }

    # allow 2 for duplex
    set combined "(?:$untwinned::combined|$twinned::combined){1,2}"
}

namespace eval kingmissing {
    set emptyLine "\n"
    set combined "(?:[set ${language}::kingmissing]\n$emptyLine)?"
}

namespace eval footer {
    set emptyLine "\n"
    set combined "[set ${language}::endlines]\n$emptyLine"
}

# applying this gives an "expression is too complex" error :-(
# dividing the input at recognized problem footers is also much, much faster...
namespace eval problem {
    set combined "($intro::combined)($boardA::combined?)($board::combined)($caption::combined)($conditions::combined)($kingmissing::combined)?($solution::combined)($footer::combined)"
}

namespace eval beforesolution {
    set combined "($intro::combined)($boardA::combined$board::combined)($caption::combined)($conditions::combined)($kingmissing::combined)"
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

if {[llength $sections]==0 || [lindex $sections 0]=="debug"} {
    set footerIndices [regexp -all -indices -inline $footer::combined $input]
    set problemStart 0
    foreach footerIndexPair $footerIndices {
	foreach {footerStart footerEnd} $footerIndexPair break
	set currentproblem [string range $input $problemStart $footerEnd]
	set matches [regexp -all -inline $beforesolution::combined $currentproblem]
	foreach { whole intro board caption conditions kingmissing } $matches {
	    printSection "i" $intro
	    printSection "b" $board
	    printSection "ca" $caption
	    printSection "co" $conditions
	    printSection "k" $kingmissing
	    set solutionIndices [regexp -all -inline -indices $solution::twinned::combined $currentproblem]
	    if {[llength $solutionIndices]==0} {
		set solutionIndices [regexp -all -inline -indices $solution::untwinned::combined $currentproblem]
	    }
	    foreach pair $solutionIndices {
		foreach {solutionStart solutionEnd} $pair break
		printSection "s" [string range $currentproblem $solutionStart $solutionEnd]
	    }
	    printSection "f" [string range $input $footerStart $footerEnd]
	}

	set problemStart [expr {$footerEnd+1}]
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
