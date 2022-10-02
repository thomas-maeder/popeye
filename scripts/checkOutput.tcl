#! /usr/bin/env tclsh

# Usage: $argv0 INPUTFILEPATH | diff -y --width=200 INPUTFILEPATH -

set inputfile [lindex $argv 0]

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
    set endlines {(?:Loesung beendet[.]|Partielle Loesung)\n+?}
    set white {Weiss}
    set black {Schwarz}
    set zugzwang {Zugzwang[.]}
    set threat "Drohung:"
    set but {Aber}
    set colorShortcut {(?:[wsn])}
    set pieceAttributeShortcut {(?:[k])}
    set zeroposition "NullStellung"
}

namespace eval english {
    set endlines {(?:solution finished[.]|Partial solution)\n+?}
    set white {white}
    set black {black}
    set zugzwang {Zugzwang[.]}
    set threat "Threat:"
    set but {But}
    set colorShortcut {(?:[wbn])}
    set pieceAttributeShortcut {(?:[r])}
    set zeroposition "zeroposition"
}

namespace eval intro {
    set emptyLine {\n}
    set leadingBlanks { *}

    set remark {[^\n]+}
    set remarkLine "(?:$remark\n)"

    set author {[^\n]+}
    set authorLine "(?:$leadingBlanks$author\n)"

    set origin {[^\n]+}
    set originLine "(?:$leadingBlanks$origin\n)"

    set award {[^\n]+}
    set awardLine "(?:$leadingBlanks$award\n)"

    set title {[^\n]+}
    set titleLine "(?:$leadingBlanks$title\n)"

    set combined "$remarkLine*$emptyLine+$authorLine*$originLine*$awardLine?$titleLine?"
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

    set combined "$emptyLine${columns}(?:$spaceLine$piecesLine){$nrRows}$spaceLine$columns"
}

namespace eval stipulation {
    set goal {(?:\#|=|dia|a=>b|z[a-h][1-8]|ct|<>|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k[a-h][1-8])}
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
    set combined " *$stipulation::combined *$pieceControl::combined\n"
}

namespace eval boardA {
    # the caption of board A doesn't indicate the stipulation
    set caption " *$pieceControl::combined\n"
    set tomove "$stipulation::paren_open$stipulation::side ->$stipulation::paren_close"
    set combined "(?:$board::combined$boardA::caption\n *=> $tomove\n\n)"
}

namespace eval conditions {
    set line { *[^\n]+\n}
    set combined "(?:$line)*"
}

namespace eval solution {
    set emptyLine {\n}
    set pieceChar {[[:upper:][:digit:]]}
    set piecePawnImplicit "$pieceChar{0,2}"
    set piece "$pieceChar{1,2}"
    set square {[a-h][1-8]}
    set captureOrNot {[-*]}
    set castlingQ "0-0-0"
    set castlingK "0-0"
    set movement "(?:[set ${language}::pieceAttributeShortcut]?$piecePawnImplicit$square$captureOrNot$square|$castlingQ|$castlingK)"
    set takeAndMakeAndTake "(?:$captureOrNot$square)"
    set promotion "(?:=$piece)"
    set enPassant {(?: ep[.])}
    # TODO replace . by []
    set pieceMovement "(?:.[set ${language}::colorShortcut]$piece$square->$square.)"
    set pieceAddition "(?:.\[+][set ${language}::colorShortcut]$piece$square.)"
    set pieceRemoval "(?:.-[set ${language}::colorShortcut][set ${language}::pieceAttributeShortcut]?$piece$square.)"
    set changeOfColor "(?:=[set ${language}::colorShortcut])"
    set messignyExchange "(?:.$piece$square<->$piece$square.)"
    # yes, this is slightly different from stipulation::goal!
    set goal {(?: (?:\#|=|dia|a=>b|z|ct|<>|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k[a-h][1-8]))}
    set move "$movement$takeAndMakeAndTake?$enPassant?$promotion?$pieceMovement?$pieceAddition?$pieceRemoval?$changeOfColor?$messignyExchange?$goal?"

    set zeroposition "(?:[set ${language}::zeroposition]\n+)"
    set twinning {[a-z][)].*?\n}; # TODO be more explicit

    namespace eval tree {
	set ordinalNumber {[1-9][0-9]*[.]}
	set attackNumber $ordinalNumber
	set defenseNumber "$ordinalNumber\[.]{2}"

	set keySuccess { [?!]}
	set zugzwangOrThreat "(?: (?:[set ${language}::zugzwang]|[set ${language}::threat]))?"

	set keyLine "   $attackNumber$solution::move$keySuccess$zugzwangOrThreat\n"
	set attackLine " +$attackNumber$solution::move$zugzwangOrThreat\n"
	set threatLine $attackLine

	set defense " +$defenseNumber$solution::move"
	set defenseLine "$defense\n"

        set postKeyPlayBlock "(?:(?:$zugzwangOrThreat)(?:$defenseLine|$attackLine)*)"

	set butLine "    [set ${language}::but]\n"
	set refutationLine "$defense !\n"
	set refutationBlock "(?:$butLine$refutationLine)"

	set fullPhaseBlock "(?:$keyLine$postKeyPlayBlock$refutationBlock?$solution::emptyLine)"

	set setPlayBlock $postKeyPlayBlock

	set combined "(?:$solution::emptyLine$setPlayBlock?$fullPhaseBlock*)"
    }

    namespace eval line {
	set ordinalNumber {[1-9][0-9]*[.]}
	set ellipsis {[.][.][.]}

	set moveNumber {[1-9][0-9]*}
	set moveNumberLine "(?: +$moveNumber  \[(]$solution::move \[)]\n)"

	set firstMovePair "(?:1.$solution::move +$solution::move)"
	set firstMoveSkipped "1$ellipsis$solution::move"
	set subsequentMovePair "(?: +$ordinalNumber$solution::move +$solution::move)"

	set regularPlayFirstMovePair "(?:$firstMoveSkipped|$firstMovePair)"
	set regularPlayLine "(?: +$regularPlayFirstMovePair$subsequentMovePair*\n)"
	set regularPlayBlock "(?:${solution::emptyLine}(?:$regularPlayLine|$moveNumberLine)*)"

	set setPlayFirstMovePairSkipped "1$ellipsis +$ellipsis"
	set setPlayFirstMovePair "(?:$setPlayFirstMovePairSkipped|$firstMoveSkipped|$firstMovePair)"
	set setPlayLine "(?: +$setPlayFirstMovePair$subsequentMovePair*\n)"
	set setPlayBlock "(?:$solution::emptyLine$setPlayLine*)"

	set seriesPlayMove "(?: +$ordinalNumber$solution::move)"
	set seriesPlayLine "(?:$seriesPlayMove*(?:$seriesPlayMove|$subsequentMovePair)\n)"
	set seriesPlayBlock "(?:$solution::emptyLine$seriesPlayLine*)"

	set combined "$setPlayBlock?(?:$regularPlayBlock|$seriesPlayBlock)"
    }

    set combined "(?:(?:$emptyLine$zeroposition?$twinning)?(?:$tree::combined*|$line::combined))"
}

namespace eval measurements {
    set line {(?: *[a-z_]+: *[0-9]+\n)}
    set combined "(?:$line{4})"
}

set problem "($intro::combined)($boardA::combined?)($board::combined)($caption::combined)($conditions::combined)($solution::combined)($measurements::combined?\n*)([set ${language}::endlines]\n\n)"

set problems "(?:$problem)+?"

set f [open $inputfile "r"]
set input [read $f]
close $f

set matches [regexp -all -inline -nocase $problems $input]

foreach { whole intro boardA board caption conditions solution measurements footer } $matches {
    puts -nonewline $intro
    puts -nonewline $boardA
    puts -nonewline $board
    puts -nonewline $caption
    puts -nonewline $conditions
    puts -nonewline $solution
    puts -nonewline $measurements
    puts -nonewline $footer
}