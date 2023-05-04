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

proc terminal {name expression} {
    upvar $name result

    set result $expression
}

proc nonterminal {name production} {
    upvar $name result

    regsub -all {[[:space:]]+} $production " " production
    regsub {^[[:space:]]+} $production "" production
    regsub {[[:space:]]+$} $production "" production

    set result "(?:"
    foreach token [split $production " "] {
        switch -regexp $token {
            ^[[:alnum:]_:]+$ {
                append result [uplevel v $token]
            }
            default {
                append result $token
            }
        }
    }
    append result ")"
}

namespace eval format {
    terminal eol {\n}
    terminal emptyLine {\n}
    terminal lineText {[^\n]}
    terminal space { }
    terminal hyphen {-}
    terminal nonspace {[^ ]}
    terminal period {[.]}

    namespace eval remark {
        nonterminal remark { lineText + }
        nonterminal remarkLine { remark eol }

        nonterminal block { remarkLine +  emptyLine * }
    }

    namespace eval authoretc {
        nonterminal leadingBlanks { space * }

        nonterminal authorOriginAwardTitle { lineText + }
        nonterminal authorOriginAwardTitleLine { leadingBlanks authorOriginAwardTitle eol }

        nonterminal block { emptyLine ?  authorOriginAwardTitleLine *  emptyLine }
    }

    namespace eval board {
        terminal cornerSign {[+]}
        terminal verticalBorderSign {[|]}
        terminal horizontalBorderSign "-"
        terminal columnName {[a-h]}
        terminal rowNo {[1-8]}
        terminal gridVertical {[|]}
        terminal white " "
        terminal black "-"
        terminal neutral "="
        terminal walkChar {[[:upper:][:digit:]]}
        terminal hunterPartsSeparator "/"

        nonterminal columnSpec { horizontalBorderSign horizontalBorderSign columnName horizontalBorderSign }
        nonterminal columns { cornerSign horizontalBorderSign columnSpec + horizontalBorderSign horizontalBorderSign cornerSign eol }

        nonterminal gridHorizontal { space hyphen hyphen hyphen }
        nonterminal squareEmpty { (?: space | gridVertical ) space period }
        nonterminal hole { (?: space | gridVertical ) space space }
        nonterminal color { white | black | neutral }
        nonterminal walk1Char { (?: space | gridVertical ) color walkChar }
        nonterminal walk2Chars { (?: color | gridVertical ) walkChar walkChar }
        nonterminal regularSeparator { space }
        nonterminal separator { regularSeparator | hunterPartsSeparator }
        nonterminal pieceOrNoPiece { squareEmpty | hole | walk1Char | walk2Chars }
        nonterminal piecesLine { rowNo space (?: pieceOrNoPiece separator ) + space space rowNo eol }

        nonterminal hunter2ndPart { hole | walk1Char | walk2Chars }
        nonterminal spaceLine { verticalBorderSign space (?: hunter2ndPart regularSeparator | gridHorizontal ) + space space verticalBorderSign eol }

        nonterminal block {
	    columns
	    (?:
	     spaceLine
	     piecesLine
	     ) +
	    spaceLine
	    columns
        }
    }

    namespace eval gridboard {
	terminal cell {(?:  [ [:digit:]][[:digit:]])}

	nonterminal cellsline { board::rowNo cell + space space space board::rowNo eol }
        nonterminal block {
	    emptyLine
	    board::columns
	    (?: board::spaceLine cellsline ) +
	    board::spaceLine
	    board::columns
	}
    }

    namespace eval stipulation {
        terminal goal {(?:\#|=|dia|a=>b|z[a-h][1-8]|ct|<>|<>r|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k[a-h][1-8])}
        terminal exactPrefix {(?:exact-)}
        terminal introPrefix {(?:[[:digit:]]+->)}
        terminal parryPrefix {(?:ph?)}
        terminal seriesPrefix {ser-}
        terminal helpPrefix "h"
        terminal paren_open {[(]}
            terminal paren_close {[)]}
        terminal reciPrefix {reci-h}
        terminal selfPrefix "s"
        terminal reflexPrefix {(?:(?:semi-)?r)}
        terminal length {(?:[[:digit:]]+(?:[.]5)?)}
        terminal side "(?:[l white]|[l black])"
        terminal maxthreatSuffix {(?:/[[:digit:]]*)}
        terminal maxflightSuffix {(?:/[[:digit:]]+)}
        terminal nontrivialSuffix {(?:;[[:digit:]]+,[[:digit:]]+)}

        nonterminal helpselfPrefix { helpPrefix selfPrefix }
        nonterminal helpreflexPrefix { helpPrefix reflexPrefix }
        nonterminal genericSeriesPrefix { introPrefix ?  parryPrefix ?  seriesPrefix }
        nonterminal recigoal { paren_open goal paren_close }
        nonterminal recihelpPrefix { reciPrefix recigoal ? }
        nonterminal playPrefix { genericSeriesPrefix ?  (?: helpPrefix | recihelpPrefix | selfPrefix | helpselfPrefix | reflexPrefix | helpreflexPrefix ) ? }
        nonterminal stipulation_traditional { exactPrefix ? playPrefix goal length }

        nonterminal stipulation_structured { side space  nonspace + }; # TODO

        nonterminal suffix { (?: maxthreatSuffix maxflightSuffix ? ) ?  nontrivialSuffix ? }

        nonterminal block { (?: stipulation_traditional | stipulation_structured ) suffix }; # TODO order of suffixes?
    }
        
    namespace eval pieceControl {
        terminal piecesOfColor {[[:digit:]]+}
        terminal plus {[+]}
        terminal totalInvisiblePseudoWalk "TI"
        terminal neutral "n"

        nonterminal separator { space plus space }
        nonterminal block {
            piecesOfColor separator piecesOfColor
            (?: separator piecesOfColor neutral ) ?
            (?: separator piecesOfColor space totalInvisiblePseudoWalk ) ?
        }
    }

    namespace eval caption {
        nonterminal block { space *  stipulation::block  space *  pieceControl::block  eol }
    }

    namespace eval boardA {
        terminal tomoveIndicator "->"
        terminal arrow "=>"

        # the caption of board A doesn't indicate the stipulation
        nonterminal captionLine { space *  pieceControl::block  eol }
        nonterminal tomove { stipulation::paren_open stipulation::side space tomoveIndicator stipulation::paren_close }
        nonterminal tomoveLine { space * arrow space tomove eol }
        nonterminal block {
	    board::block
	    captionLine
	    emptyLine
	    tomoveLine
	    emptyLine
	    emptyLine
        }
    }

    namespace eval conditions {
        nonterminal block { (?: space *  lineText +  eol ) * }
    }

    namespace eval duplex {
        terminal duplexOrHalf "(?:[l duplex]|[l halfduplex])"

        nonterminal block { (?: space *  duplexOrHalf  eol ) ? }
    }

    namespace eval solution {
        terminal naturalNumber {[1-9][0-9]*}
        terminal paren_open {[(]}
        terminal paren_close {[)]}
        terminal bracket_open {\[}
        terminal bracket_close {\]}
        terminal ellipsis {[.][.][.]}
        terminal comma ","
        terminal period {[.]}
        terminal square {[a-h][1-8]}
        terminal capture {[*]}
        terminal captureOrNot {[-*]}
        terminal castlingQ "0-0-0"
        terminal castlingK "0-0"
        terminal pieceAttributeShortcut "(?:"
        foreach p [l pieceAttributeShortcuts] {
            if {[string length $p]==1} {
                append pieceAttributeShortcut "$p?"
            } else {
                append pieceAttributeShortcut "(?:$p)?"
            }
        }
        append pieceAttributeShortcut ")"
        terminal enPassant {(?: ep[.])}
        terminal vulcanization "(?:->v)"
        terminal promotionIndicator "="
        terminal pieceAdditionIndicator {[+]}
        terminal pieceRemovalIndicator "-"
        terminal pieceMovementIndicator "->"
        terminal pieceExchangeIndicator "<->"
        terminal bglNumber {[[:digit:]]+(?:[.][[:digit:]]{1,2})?}
        terminal bglNone "-"
        terminal bglDividedBy "/"
        terminal checkIndicator {(?: [+])}
        # yes, this is slightly different from stipulation::goal!
        terminal goal "(?: (?:\#|=|dia|a=>b|z|ct|<>|\[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=|!\#|k\[a-h]\[1-8]))"
        terminal moveNumber {[1-9][0-9]*}
        terminal nrPositions {[[:digit:]]+}
        terminal nrMoves {[[:digit:]]+[+][[:digit:]]+}
        terminal undec "(?: (?:[l legalityUndecidable]|[l refutationUndecidable]))"
        terminal imitatorSign "I"
        terminal castlingPartnerSeparator "/"
        terminal roleExchange " [l roleExchange]"
        terminal potentialPositionsIn "[l potentialPositionsIn]"
        terminal totalInvisibleMovePrefix "TI~"
        terminal totalInvisibleMoveSuffix "-~"
        terminal forcedReflexMoveIndicator {[?]![?]}
        terminal kingmissing "[l kingmissing]"
        terminal measurementsLine {(?: *[[:alpha:]_]+: *[[:digit:]]+)}

        nonterminal ordinalNumber { naturalNumber period }

        nonterminal hunterSuffix { board::hunterPartsSeparator  board::walkChar {1,2} }
        nonterminal walk { board::walkChar {1,2}  hunterSuffix ? }
        nonterminal walkPawnImplicit { board::walkChar {0,2}  hunterSuffix ? }

        nonterminal movementFromTo { pieceAttributeShortcut walkPawnImplicit square captureOrNot square }
        nonterminal castlingPartnerMovement { castlingPartnerSeparator movementFromTo }
        nonterminal movement { (?: movementFromTo castlingPartnerMovement ? | castlingQ | castlingK ) (?: captureOrNot square ) * }
        nonterminal messignyExchange { walk square pieceExchangeIndicator walk square }
        nonterminal promotion { promotionIndicator pieceAttributeShortcut walk ? }
        nonterminal pieceChangement { square promotion }
        nonterminal pieceSpec { pieceAttributeShortcut walk square }
        nonterminal pieceMovement { pieceSpec  pieceMovementIndicator  pieceAttributeShortcut  walk ?  square  promotion *  vulcanization ? }
        nonterminal pieceAddition { pieceAdditionIndicator pieceSpec promotion *  vulcanization ? }
        nonterminal pieceRemoval { pieceRemovalIndicator pieceSpec }
        nonterminal pieceExchange { pieceSpec pieceExchangeIndicator pieceSpec }
        nonterminal pieceEffect { bracket_open (?: pieceMovement | pieceAddition | pieceRemoval | pieceChangement | pieceExchange ) bracket_close }
        nonterminal imitatorMovement { bracket_open imitatorSign square (?: comma square ) * bracket_close }
        nonterminal bglBalance { space paren_open (?: bglNone | bglNumber ) (?: bglDividedBy bglNumber ) ? paren_close }
        nonterminal totalInvisibleMove { totalInvisibleMovePrefix totalInvisibleMoveSuffix }
        nonterminal totalInvisibleCapture { totalInvisibleMovePrefix capture square }
        nonterminal move { (?: roleExchange | space ellipsis | pieceEffect * (?: movement | totalInvisibleMove | totalInvisibleCapture | messignyExchange ) enPassant ? imitatorMovement ? promotion * pieceEffect * bglBalance ? checkIndicator ? ) goal ? } ; # TODO move goal away from here

        nonterminal moveNumberLineNonIntelligent { space * moveNumber space space paren_open move space paren_close eol }
        nonterminal moveNumberLineIntelligent { nrPositions space potentialPositionsIn space nrMoves eol }

        namespace eval twinning {
            terminal continued {[+]}
            terminal label {[[:lower:]][)]}

            nonterminal block {
		emptyLine
		continued ? label space lineText * eol
		(?: space lineText + eol ) *
            }; # TODO be more explicit
        }

        nonterminal forcedReflexMove { space + ordinalNumber move space forcedReflexMoveIndicator eol }

        namespace eval tree {
            terminal zugzwang "[l zugzwang]"
            terminal threat "[l threat]"
            terminal refutationIndicator "!"
	    terminal refutesIndicator "[l refutes]"
	    terminal keySuccess {[?!]}
	    terminal but "[l but]"

            nonterminal attack { ordinalNumber move }
            nonterminal defense { naturalNumber ellipsis move }

            # in condition "lost pieces", lost pieces of the attacker may be removed
            nonterminal zugzwangOrThreat { zugzwang | threat pieceEffect ? }

	    nonterminal keyLine { space space space attack (?: undec | space keySuccess (?: space zugzwangOrThreat ) ? ) eol }

	    nonterminal attackLine { space + attack (?: undec | (?: space zugzwangOrThreat ) ? ) eol }
	    nonterminal defenseLine { space + defense (?: undec ) ? eol }

	    # TODO should Popeye write an empty line before the check indicator?
	    nonterminal checkOrZugzwangOrThreatLine { (?: checkIndicator | emptyLine space zugzwangOrThreat ) eol }

	    nonterminal postKeyPlay { checkOrZugzwangOrThreatLine ? (?: defenseLine (?: space + refutesIndicator eol ) ? | attackLine ) * }

	    nonterminal refutation { space + defense space refutationIndicator eol forcedReflexMove ? }

	    nonterminal refutationBlock {
		space + but eol
		refutation +
            }

	    nonterminal playAfterKey { (?: defenseLine | attackLine ) + }

	    nonterminal fullPhase {
		keyLine
		playAfterKey ?
		refutationBlock ?
		emptyLine
	    }

	    nonterminal setplay { playAfterKey }

            nonterminal regularplay { (?: moveNumberLineNonIntelligent | fullPhase ) + }
            nonterminal block { postKeyPlay | (?: (?: emptyLine setplay ) ? (?: emptyLine regularplay ) * ) }
        }

        namespace eval line {
            nonterminal subsequentMovePair { space + ordinalNumber move (?: undec eol | (?: space + move ) + (?: undec eol ) ? ) }
            nonterminal moveNumberLine { moveNumberLineNonIntelligent | moveNumberLineIntelligent }

            namespace eval helpplay {
                terminal one "1"

                nonterminal finalMove { space + ordinalNumber move (?: undec eol ) ? }

                # set play of h#n.5
                namespace eval twoEllipsis {
                    nonterminal firstMovePairSkipped { one ellipsis space + ellipsis }
                    nonterminal line { space + firstMovePairSkipped subsequentMovePair * finalMove ? eol }
                    nonterminal block { (?: line | moveNumberLine ) + }
                }

                # set play of h#n or regular play of h#n.5
                namespace eval oneEllipsis {
                    nonterminal firstMoveSkipped { one  ellipsis move (?: undec eol ) ? }
                    nonterminal line { space + firstMoveSkipped subsequentMovePair * finalMove ? eol }
                    nonterminal block { (?: line | moveNumberLine ) + }
                }

                # regular play of h#n
                namespace eval noEllipsis {
                    nonterminal firstMovePair { one period move (?: undec eol | (?: space + move ) + (?: undec eol ) ? ) }
                    nonterminal line { space + firstMovePair subsequentMovePair * finalMove ? eol }
                    nonterminal block { (?: line | moveNumberLine ) + }
                }

                nonterminal block {
		    twoEllipsis::block ? emptyLine ? oneEllipsis::block
		    | oneEllipsis::block ? emptyLine ? noEllipsis::block ?
		}
            }

            namespace eval seriesplay {
                # TODO while not in help play?
                terminal setplayNotApplicable "[l setplayNotApplicable]"
                terminal tryplayNotApplicable "[l tryplayNotApplicable]"

                nonterminal numberedMove { space + ordinalNumber move }
                nonterminal line { (?: numberedMove | subsequentMovePair ) + eol }
                nonterminal block { (?: (?: setplayNotApplicable | tryplayNotApplicable ) eol * ) ? emptyLine (?: line | moveNumberLine ) * }
            }

            # TODO why emptyLine only before help play?
            nonterminal block { emptyLine helpplay::block | seriesplay::block }
        }

        nonterminal measurementsBlock { (?: measurementsLine eol ) + }

        nonterminal kingMissingLine { kingmissing eol }

        namespace eval untwinned {
            terminal toofairy "[l toofairy]"
            terminal nonsensecombination "[l nonsensecombination]"
            terminal conditionSideUndecidable "[l conditionSideUndecidable]"
            terminal problemignored "[l problemignored]"
            terminal illegalSelfCheck "[l illegalSelfCheck]"
            terminal intelligentAndFairy "(?:[l intelligentAndFairy])"

            nonterminal problemignoredMsgs {
		(?:
		 toofairy eol
		 | nonsensecombination eol
		 | conditionSideUndecidable eol emptyLine
		 )
		problemignored eol
            }
            nonterminal simplex { (?: emptyLine illegalSelfCheck | emptyLine forcedReflexMove + | tree::block | line::block ) + }
            # the + should be {1,2}, but that would make the expression too complex
            nonterminal block { kingMissingLine ? intelligentAndFairy ? (?: problemignoredMsgs | (?: simplex measurementsBlock ) + ) remark::block ? }
        }

        namespace eval twinned {
            # too complex for regexp
            nonterminal block { (?: twinning::block untwinned::block ) + }
            nonterminal separator { twinning::block }
        }

        # too complex for regexp
        nonterminal block { untwinned::block | twinned::block }
    }

    namespace eval footer {
        terminal endOfSolution "[l endOfSolution]"
        terminal partialSolution "[l partialSolution]"

        nonterminal block { (?: eol endOfSolution | partialSolution ) eol emptyLine emptyLine }
    }

    # applying this gives an "expression is too complex" error :-(
    # dividing the input at recognized problem footers is also much, much faster...
    namespace eval problem {
        # too complex for regexp
        nonterminal block { ( remark::block (?: ( authoretc::block ) ( boardA::block ? ) ( board::block ) ( caption::block ) ( conditions::block ) ( duplex::block ) ( gridboard::block ? ) ) ? ( solution::block ) ( footer::block ) ) }
    }

    namespace eval inputerror {
        terminal inputError "[l inputError]:"
        terminal offendingItem "[l offendingItem]:"

        nonterminal block {
	    inputError lineText + eol
	    offendingItem space lineText + eol
        }
    }

    namespace eval zeroposition {
        terminal zeroposition "[l zeroposition]"

        nonterminal block { emptyLine zeroposition eol emptyLine }
    }

    namespace eval beforesolution {
        nonterminal block {
            ^
            ( inputerror::block * )
            ( remark::block ? )
            (?: ( authoretc::block ) ( boardA::block ? ) ( board::block ) ( caption::block ) ( conditions::block ) ( duplex::block ) ( gridboard::block ? ) ) ?
            ( zeroposition::block ? )
        }
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
    if {[regexp $format::beforesolution::block $beforesol - inputerrors remark authoretc boardA board caption conditions duplex gridboard zeroposition]
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
    set noboardExpr "^()($format::solution::untwinned::block)\$"
    # make sure that the board caption, conditions or whatever comes last ends with a newline character
    set withBoardExpr "^(.*?\n)($format::solution::untwinned::block)\$"
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
        if {[regexp $format::solution::untwinned::block $solution]} {
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
    set footerIndices [regexp -all -indices -inline $format::footer::block $input]
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
        append expr [set format::${section}::block]
    }
    foreach match [regexp -all -inline $expr $input] {
        puts -nonewline $match
        puts "===="
    }
}
