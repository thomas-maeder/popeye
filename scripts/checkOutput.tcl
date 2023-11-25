#! /usr/bin/env tclsh

# Usage: $argv0 INPUTFILEPATH | diff -y --width=200 INPUTFILEPATH -

switch -re [lindex $argv 0] {
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

set inputfiles $argv

namespace eval german {
    set popeye "Popeye"
    set time "Zeit"
    set endOfSolution "Loesung beendet."
    set partialSolution "Partielle Loesung"
    set white "Weiss"
    set black "Schwarz"
    set zugzwang "Zugzwang."
    set threat "Drohung:"
    set but "Aber"
    # yes, some shortcuts are ambiguous
    namespace eval pieceAttributeShortcut {
        set white "w"
        set black "s"
        set neutral "n"
        set royal "k"
        set kamikaze "k"
        set paralyzing "p"
        set chameleon "c"
        set jigger "j"
        set volage "v"
        set functionary "b"
        set halfneutral "hn"
        set hurdleColourChanging "sfw"
        set protean "p"
        set magic "m"
        set uncapturable "u"
        set patrol "p"
        set frischauf "f"
    }
    set zeroposition "NullStellung"
    set potentialPositionsIn "moegliche Stellungen in"
    set kingmissing "Es fehlt ein weisser oder schwarzer Koenig"
    set threatNotApplicable "Option \"Drohung\" nicht anwendbar order maximale Laenge der Drohung zu gross"
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
    set duplex "Duplex"
    set halfduplex "HalbDuplex"
    set inputError "Eingabe-Fehler:"
    set offendingItem "Stoerende Eingabe: "
}

namespace eval english {
    set popeye "Popeye"
    set time "Time"
    set endOfSolution "solution finished."
    set partialSolution "Partial solution"
    set white "White"
    set black "Black"
    set zugzwang "zugzwang."
    set threat "threat:"
    set but "but"
    # yes, some shortcuts are ambiguous - not the same as in German
    namespace eval pieceAttributeShortcut {
        set white "w"
        set black "b"
        set neutral "n"
        set royal "r"
        set kamikaze "k"
        set paralyzing "p"
        set chameleon "c"
        set jigger "j"
        set volage "v"
        set functionary "f"
        set halfneutral "hn"
        set hurdleColourChanging "hcc"
        set protean "p"
        set magic "m"
        set uncapturable "u"
        set patrol "p"
        set frischauf "f"
    }
    set zeroposition "zeroposition"
    set potentialPositionsIn "potential positions in"
    set kingmissing "both sides need a king"
    set threatNotApplicable "Option \"Threat\" not applicable or indicated maximum threat length too big"
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
    set duplex "Duplex"
    set halfduplex "HalfDuplex"
    set inputError "input-error:"
    set offendingItem "offending item: "
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

proc decomposeExpr {expr {end ""}} {
    set result {}

    while {$expr!=""} {
        set firstChar [string range $expr 0 0]
        if {$firstChar=="("} {
            lassign [decomposeExpr [string range $expr 1 "end"] ")"] nested expr
            lappend result [list "(" $nested ")"]
        } elseif {$firstChar=="\["} {
            lassign [decomposeExpr [string range $expr 1 "end"] "]"] nested expr
            lappend result [list "\[" $nested "\]"]
        } elseif {$firstChar=="\\"} {
            set escapedChar [string range $expr 0 1]
            set expr [string range $expr 2 "end"]
            lappend result $escapedChar
        } else {
            set expr [string range $expr 1 "end"]
            if {$firstChar==$end} {
                return [list $result $expr]
            } else {
                lappend result $firstChar
            }
        }
    }

    return [list $result ""]
}

proc nonterminal {name production} {
    upvar $name result

    regsub -all {[[:space:]]+} $production " " production
    regsub {^[[:space:]]+} $production "" production
    regsub {[[:space:]]+$} $production "" production

    set result "(?:"
    foreach token [split $production " "] {
        switch -regexp -matchvar matches -- $token {
            ^([[:alnum:]_:]+)([?*+]|{.*})?$ {
                set name [lindex $matches 1]
                set value [uplevel v $name]
                set quantifier [lindex $matches 2]
                set decomposition [lindex [decomposeExpr $value] 0]
                if {[lsearch -exact $decomposition "|"]!=-1} {
                    append result "(?:$value)"
                } elseif {$quantifier==""} {
                    append result $value
                } elseif {[llength $decomposition]<=1} {
                    append result $value
                } else {
                    append result "(?:$value)"
                }
                append result $quantifier
            }
            default {
                append result $token
            }
        }
    }
    append result ")"
}

puts "preparing Popeye output grammar parser"

namespace eval format {
    terminal eol {\n}
    terminal emptyLine {\n}
    terminal lineText {[^\n]}
    terminal space { }
    terminal hyphen {-}
    terminal nonspace {[^ ]}
    terminal period {[.]}

    # we can't distinguish remarks (in the input) and error messages produced during validation
    namespace eval remarkOrValidationError {
        nonterminal remark { lineText+ }
        nonterminal remarkLine { remark eol }

        nonterminal block { remarkLine* }
    }

    namespace eval meta {
        nonterminal leadingBlanks { space* }

        nonterminal authorOriginAwardTitle { lineText+ }
        nonterminal authorOriginAwardTitleLine { leadingBlanks authorOriginAwardTitle eol }

        nonterminal block { emptyLine authorOriginAwardTitleLine* }
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
        nonterminal columns { cornerSign horizontalBorderSign columnSpec+ horizontalBorderSign horizontalBorderSign cornerSign eol }

        nonterminal gridHorizontal { space hyphen hyphen hyphen }
        nonterminal squareEmptyChar1 { space | gridVertical }
        nonterminal squareEmpty { squareEmptyChar1 space period }
        nonterminal hole { squareEmptyChar1 space space }
        nonterminal color { white | black | neutral }
        nonterminal walk1Char { squareEmptyChar1 color walkChar }
        nonterminal squareOccupiedChar1 { color | gridVertical }
        nonterminal walk2Chars { squareOccupiedChar1 walkChar walkChar }
        nonterminal regularSeparator { space }
        nonterminal separator { regularSeparator | hunterPartsSeparator }
        nonterminal pieceOrNoPiece { squareEmpty | hole | walk1Char | walk2Chars }
        nonterminal pieceCell { pieceOrNoPiece separator }
        nonterminal piecesLine { rowNo space pieceCell+ space space rowNo eol }

        nonterminal hunter2ndPart { hole | walk1Char | walk2Chars }
        nonterminal spaceLineCell { hunter2ndPart regularSeparator | gridHorizontal }
        nonterminal spaceLine { verticalBorderSign space spaceLineCell+ space space verticalBorderSign eol }

        nonterminal linePair { spaceLine piecesLine }

        nonterminal block {
            emptyLine
            columns
            linePair+
            spaceLine
            columns
        }
    }

    namespace eval gridboard {
        terminal cell {  [ [:digit:]][[:digit:]]}

        nonterminal cellsline { board::rowNo cell+ space space space board::rowNo eol }
        nonterminal linePair { board::spaceLine cellsline }
        nonterminal block {
            emptyLine
            board::columns
            linePair+
            board::spaceLine
            board::columns
        }
    }

    namespace eval goal {
        terminal mate "#"
	terminal stalemate "="
	terminal dia "dia"
	terminal aToB "a=>b"
	terminal targetSquare {z[a-h][1-8]}
	terminal targetSquareImplicit "z"
	terminal circuit "ct"
	terminal exchange "<>"
	terminal exchangeByRebirth "<>r"
	terminal check {[+]}
	terminal doubleStalemate "=="
	terminal castling "00"
	terminal pieceWin "%"
	terminal anyMove "~"
	terminal bothMate "##"
	terminal counterMate "##!"
	terminal autoMate "!="
	terminal enPassant "ep"
	terminal capture "x"
	terminal circuitByRebirth "ctr"
	terminal chess81 "c81"
	terminal immobile "#="
	terminal noMate "!#"
	terminal kiss {k[a-h][1-8]}
    }

    namespace eval stipulation {
        terminal exactPrefix {exact-}
        terminal introPrefix {[[:digit:]]+->}
        terminal parryPrefix {ph?}
        terminal seriesPrefix {ser-}
        terminal helpPrefix "h"
        terminal paren_open {[(]}
        terminal paren_close {[)]}
        terminal reciPrefix {reci-h}
        terminal selfPrefix "s"
        terminal reflexPrefix {(?:semi-)?r}
        terminal length {[[:digit:]]+(?:[.]5)?}
        terminal sideWhite [l white]
        terminal sideBlack [l black]
        terminal maxthreatSuffix {/[[:digit:]]*}
        terminal maxflightSuffix {/[[:digit:]]+}
        terminal nontrivialSuffix {;[[:digit:]]+,[[:digit:]]+}

        nonterminal side { sideWhite | sideBlack }

	nonterminal goal {
	    goal::mate
	    | goal::stalemate
	    | goal::dia
	    | goal::aToB
	    | goal::targetSquare
	    | goal::circuit
	    | goal::exchange
	    | goal::check
	    | goal::doubleStalemate
	    | goal::castling
	    | goal::pieceWin
	    | goal::anyMove
	    | goal::bothMate
	    | goal::counterMate
	    | goal::autoMate
	    | goal::enPassant
	    | goal::capture
	    | goal::circuitByRebirth
	    | goal::exchangeByRebirth
	    | goal::chess81
	    | goal::immobile
	    | goal::noMate
	    | goal::kiss
	}

        nonterminal helpselfPrefix { helpPrefix selfPrefix }
        nonterminal helpreflexPrefix { helpPrefix reflexPrefix }
        nonterminal genericSeriesPrefix { introPrefix? parryPrefix? seriesPrefix }
        nonterminal recigoal { paren_open goal paren_close }
        nonterminal recihelpPrefix { reciPrefix recigoal? }
        nonterminal alternatePrefix { helpPrefix | recihelpPrefix | selfPrefix | helpselfPrefix | reflexPrefix | helpreflexPrefix }
        nonterminal playPrefix { exactPrefix? genericSeriesPrefix? alternatePrefix? }
        nonterminal traditional { playPrefix goal length }

	# structured stipulations are recursive, which can't be represented by
	# regular expressions
        nonterminal structured { side space nonspace+ }

        nonterminal maxSuffix { maxthreatSuffix maxflightSuffix? }
        nonterminal suffix { maxSuffix? nontrivialSuffix? }

        nonterminal combined { traditional | structured }

        nonterminal block { combined suffix }
    }
        
    namespace eval pieceControl {
        terminal piecesOfColor {[[:digit:]]+}
        terminal separator { [+] }
        terminal totalInvisiblePseudoWalk "TI"
        terminal neutral "n"

        nonterminal piecesNeutral { separator piecesOfColor neutral }
        nonterminal piecesTotalInvisible { separator piecesOfColor space totalInvisiblePseudoWalk }
        nonterminal block { piecesOfColor separator piecesOfColor piecesNeutral? piecesTotalInvisible? }
    }

    namespace eval caption {
        nonterminal block { space* stipulation::block space* pieceControl::block eol }
    }

    namespace eval boardA {
        terminal tomoveIndicator "->"
        terminal arrow "=>"

        # the caption of board A doesn't indicate the stipulation
        nonterminal captionLine { space* pieceControl::block eol }
        nonterminal tomove { stipulation::paren_open stipulation::side space tomoveIndicator stipulation::paren_close }
        nonterminal tomoveLine { space* arrow space tomove eol }
        nonterminal block {
            board::block
            captionLine
            emptyLine
            tomoveLine
            emptyLine
        }
    }

    namespace eval conditions {
        nonterminal line { space* lineText+ eol }
        nonterminal block { line* }
    }

    namespace eval duplex {
        terminal duplex [l duplex]
        terminal halfduplex [l halfduplex]

        nonterminal duplexOrHalf { duplex | halfduplex }
        nonterminal line { space* duplexOrHalf eol }
        nonterminal block { line? }
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
        terminal enPassant { ep[.]}
        terminal vulcanization "->v"
        terminal promotionIndicator "="
        terminal pieceAdditionIndicator {[+]}
        terminal pieceRemovalIndicator "-"
        terminal pieceMovementIndicator "->"
        terminal pieceExchangeIndicator "<->"
        terminal bglNumber {[[:digit:]]+(?:[.][[:digit:]]{1,2})?}
        terminal bglNone "-"
        terminal bglDividedBy "/"
        terminal checkIndicator { [+]}
        terminal moveNumber {[1-9][0-9]*}
        terminal nrPositions {[[:digit:]]+}
        terminal nrMoves {[[:digit:]]+[+][[:digit:]]+}
        terminal legalityUndecidable [l legalityUndecidable]
        terminal refutationUndecidable [l refutationUndecidable]
        terminal imitatorSign "I"
        terminal castlingPartnerSeparator "/"
        terminal roleExchange [l roleExchange]
        terminal potentialPositionsIn [l potentialPositionsIn]
        terminal totalInvisibleMovePrefix "TI~"
        terminal totalInvisibleMoveSuffix "-~"
        terminal forcedReflexMoveIndicator {[?]![?]}
        terminal measurement { *[[:alpha:]_]+: *[[:digit:]]+}

	namespace eval pieceAttributeShortcut {
	    terminal white [l pieceAttributeShortcut::white]
	    terminal black [l pieceAttributeShortcut::black]
	    terminal neutral [l pieceAttributeShortcut::neutral]
	    terminal royal [l pieceAttributeShortcut::royal]
	    terminal kamikaze [l pieceAttributeShortcut::kamikaze]
	    terminal paralyzing [l pieceAttributeShortcut::paralyzing]
	    terminal chameleon [l pieceAttributeShortcut::chameleon]
	    terminal jigger [l pieceAttributeShortcut::jigger]
	    terminal volage [l pieceAttributeShortcut::volage]
	    terminal functionary [l pieceAttributeShortcut::functionary]
	    terminal halfneutral [l pieceAttributeShortcut::halfneutral]
	    terminal hurdleColourChanging [l pieceAttributeShortcut::hurdleColourChanging]
	    terminal protean [l pieceAttributeShortcut::protean]
	    terminal magic [l pieceAttributeShortcut::magic]
	    terminal uncapturable [l pieceAttributeShortcut::uncapturable]
	    terminal patrol [l pieceAttributeShortcut::patrol]
	    terminal frischauf [l pieceAttributeShortcut::frischauf]

	    nonterminal colour { white | black | neutral }

	    nonterminal nonColour {
		royal?
		kamikaze?
		paralyzing?
		chameleon?
		jigger?
		volage?
		functionary?
		halfneutral?
		hurdleColourChanging?
		protean?
		magic?
		uncapturable?
		patrol?
		frischauf?
	    }
	}

        nonterminal undec { legalityUndecidable | refutationUndecidable }

	nonterminal goal {
	    goal::mate
	    | goal::stalemate
	    | goal::dia
	    | goal::aToB
	    | goal::targetSquareImplicit
	    | goal::circuit
	    | goal::exchange
	    | goal::check
	    | goal::doubleStalemate
	    | goal::castling
	    | goal::pieceWin
	    | goal::anyMove
	    | goal::bothMate
	    | goal::counterMate
	    | goal::autoMate
	    | goal::enPassant
	    | goal::capture
	    | goal::circuitByRebirth
	    | goal::exchangeByRebirth
	    | goal::chess81
	    | goal::immobile
	    | goal::noMate
	    | goal::kiss
	}
	nonterminal goalIndicator { space goal }

        nonterminal ordinalNumber { naturalNumber period }

        nonterminal hunterSuffix { board::hunterPartsSeparator board::walkChar{1,2} }
        nonterminal walk { board::walkChar{1,2} hunterSuffix? }
        nonterminal walkPawnImplicit { board::walkChar{0,2} hunterSuffix? }

        nonterminal movementTo { captureOrNot square }
        nonterminal movementFromTo { pieceAttributeShortcut::colour? pieceAttributeShortcut::nonColour walkPawnImplicit square movementTo }
        nonterminal castlingPartnerMovement { castlingPartnerSeparator pieceAttributeShortcut::colour pieceAttributeShortcut::nonColour walkPawnImplicit square movementTo }
        nonterminal movementBasic { movementFromTo castlingPartnerMovement? | castlingQ | castlingK }
        nonterminal movementComposite { movementBasic movementTo* }
        nonterminal messignyExchange { walk square pieceExchangeIndicator walk square }
        nonterminal promotion { promotionIndicator pieceAttributeShortcut::colour? pieceAttributeShortcut::nonColour walk? }
        nonterminal pieceChangement { square promotion }
        nonterminal pieceSpec { pieceAttributeShortcut::colour? pieceAttributeShortcut::nonColour walk square }
        nonterminal pieceMovement {
	    pieceSpec
	    pieceMovementIndicator
	    pieceAttributeShortcut::colour?
	    pieceAttributeShortcut::nonColour
	    walk?
	    square
	    promotion*
	    vulcanization?
	}
        nonterminal pieceAddition { pieceAdditionIndicator pieceSpec promotion* vulcanization? }
        nonterminal pieceRemoval { pieceRemovalIndicator pieceSpec }
        nonterminal pieceExchange { pieceSpec pieceExchangeIndicator pieceSpec }
        nonterminal pieceEffect { pieceMovement | pieceAddition | pieceRemoval | pieceChangement | pieceExchange }
        nonterminal otherPieceEffect { bracket_open pieceEffect bracket_close }
        nonterminal nextImitatorPosition { comma square }
        nonterminal imitatorMovement { bracket_open imitatorSign square nextImitatorPosition* bracket_close }
        nonterminal bglFirstPart { bglNone | bglNumber }
        nonterminal bglSecondPart { bglDividedBy bglNumber }
        nonterminal bglBalance { space paren_open bglFirstPart bglSecondPart? paren_close }
        nonterminal totalInvisibleMove { totalInvisibleMovePrefix totalInvisibleMoveSuffix }
        nonterminal totalInvisibleCapture { totalInvisibleMovePrefix capture square }
        nonterminal movingPieceMovement { movementComposite | totalInvisibleMove | totalInvisibleCapture | messignyExchange }
        nonterminal movementAddons { enPassant? imitatorMovement? promotion* otherPieceEffect* }
        nonterminal movingPieceMovementWithEffects { movingPieceMovement movementAddons }
        nonterminal seriesCaptureStep { movementTo movementAddons }
        nonterminal regularMove { otherPieceEffect* movingPieceMovementWithEffects seriesCaptureStep* bglBalance? checkIndicator? }
        nonterminal move { space roleExchange | space ellipsis | regularMove }

        nonterminal moveNumberLineNonIntelligent { space* moveNumber space space paren_open move space paren_close eol }
        nonterminal moveNumberLineIntelligent { nrPositions space potentialPositionsIn space nrMoves eol }

        nonterminal forcedReflexMove { space+ ordinalNumber move goalIndicator space forcedReflexMoveIndicator eol }

        namespace eval tree {
            terminal zugzwang [l zugzwang]
            terminal threat [l threat]
            terminal refutationIndicator "!"
            terminal refutesIndicator [l refutes]
            terminal keySuccess {[?!]}
            terminal but [l but]

            nonterminal attack { ordinalNumber move }
            nonterminal defense { naturalNumber ellipsis move }

            # in condition "lost pieces", lost pieces of the attacker may be removed
            nonterminal zugzwangOrThreat { space zugzwang | space threat otherPieceEffect? }

            nonterminal keySuccessSuffix { space undec | goalIndicator? space keySuccess zugzwangOrThreat? }
            nonterminal keyLine { space space space attack keySuccessSuffix eol }

            nonterminal attackSuffix { space undec | goalIndicator | zugzwangOrThreat }
            nonterminal attackLine { space+ attack attackSuffix? eol }
            nonterminal defenseSuffix { space undec | goalIndicator }
            nonterminal defenseLine { space+ defense defenseSuffix? eol }

            # TODO should Popeye write an empty line before the check indicator?
            nonterminal checkOrZugzwangOrThreat { checkIndicator | emptyLine zugzwangOrThreat }
            nonterminal checkOrZugzwangOrThreatLine { checkOrZugzwangOrThreat eol }

            nonterminal refutesLine { space+ refutesIndicator eol }
            nonterminal postKeyPlayLine { defenseLine refutesLine? | attackLine }
            nonterminal postKeyPlay { checkOrZugzwangOrThreatLine? postKeyPlayLine* }

            nonterminal refutation {
                space+ defense goalIndicator? space refutationIndicator eol
                forcedReflexMove?
            }

            nonterminal refutationBlock {
                space+ but eol
                refutation+
            }

            nonterminal playAfterKeyLine { defenseLine | attackLine }
            nonterminal playAfterKeyBlock { playAfterKeyLine+ }

            nonterminal fullPhaseBlock {
                keyLine
                playAfterKeyBlock?
                refutationBlock?
                emptyLine
            }

            nonterminal setplayBlock { emptyLine playAfterKeyBlock }

            nonterminal regularPlaySegment { moveNumberLineNonIntelligent | fullPhaseBlock }
            nonterminal regularplayBlock { emptyLine regularPlaySegment+ }

            nonterminal block { postKeyPlay | setplayBlock? regularplayBlock* }
        }

        namespace eval line {
            nonterminal moveSuffix { space undec eol | goalIndicator }
            nonterminal moveNumberLine { moveNumberLineNonIntelligent | moveNumberLineIntelligent }
            nonterminal numberedHalfMove { space+ ordinalNumber move }
            nonterminal unnumberedHalfMove { space+ move }

            namespace eval helpplay {
                terminal one "1"

                nonterminal movePair { numberedHalfMove unnumberedHalfMove }
                # this is too generic: it allows lines without any move
                # requiring >=1 move would be complicated, though
                nonterminal movesSequence { movePair* numberedHalfMove? moveSuffix? eol}

                # set play of h#n.5
                namespace eval twoEllipsis {
                    nonterminal firstMovePairSkipped { one ellipsis space+ ellipsis }
                    nonterminal movesLine { space+ firstMovePairSkipped movesSequence }
                    nonterminal line { movesLine | moveNumberLine }
                    nonterminal block { line+ }
                }

                # set play of h#n or regular play of h#n.5
                namespace eval oneEllipsis {
                    nonterminal firstMoveSkipped { one ellipsis move }
                    nonterminal movesLine { space+ firstMoveSkipped movesSequence }
                    nonterminal line { movesLine | moveNumberLine }
                    nonterminal block { line+ }
                }

                # regular play of h#n
                namespace eval noEllipsis {
                    nonterminal line { movesSequence | moveNumberLine }
                    nonterminal block { line+ }
                }

                nonterminal block {
                    twoEllipsis::block? emptyLine? oneEllipsis::block
                    | oneEllipsis::block? emptyLine? noEllipsis::block?
                }
            }

            namespace eval seriesplay {
                nonterminal fullMove { numberedHalfMove unnumberedHalfMove* }
                nonterminal movesLine { fullMove+ moveSuffix? eol }
                nonterminal line { movesLine | moveNumberLine }
                nonterminal block { line* }
            }

            nonterminal block { helpplay::block | seriesplay::block }
        }

        nonterminal measurementLine { measurement eol }
        nonterminal measurementsBlock { measurementLine+ }

        namespace eval untwinned {
            terminal toofairy [l toofairy]
            terminal nonsensecombination [l nonsensecombination]
            terminal conditionSideUndecidable [l conditionSideUndecidable]
            terminal problemignored [l problemignored]
            terminal illegalSelfCheck [l illegalSelfCheck]
	    terminal kingmissing [l kingmissing]
	    terminal threatNotApplicable [l threatNotApplicable]
            terminal intelligentAndFairy [l intelligentAndFairy]

            nonterminal error {
                toofairy
                | nonsensecombination
                | conditionSideUndecidable
            }
            nonterminal problemignoredMsg {
                error eol
                problemignored eol
            }
            nonterminal simplexPart { illegalSelfCheck eol | emptyLine forcedReflexMove+ | tree::block | emptyLine line::block }
            nonterminal simplex { simplexPart+ measurementsBlock? }
            nonterminal solvingResult { problemignoredMsg | simplex{1,2} }

	    nonterminal warning {
		kingmissing
		| threatNotApplicable
		| intelligentAndFairy
	    }
	    nonterminal warningLine { warning eol }

            nonterminal block { warningLine* solvingResult remarkOrValidationError::block? }
        }

        namespace eval twinning {
            terminal continued {[+]}
            terminal label {[[:lower:]][[:digit:]]*[)]}

            nonterminal additionalLine { space space space+ lineText+ eol }
            nonterminal block {
                emptyLine
                continued? label space lineText* eol
                additionalLine*
            }
        }

        namespace eval twinned {
            nonterminal twinblock { twinning::block untwinned::block }
            nonterminal block { twinblock+ }
        }

        nonterminal block { untwinned::block | twinned::block }
    }

    namespace eval footer {
        terminal endOfSolution [l endOfSolution]
        terminal partialSolution [l partialSolution]
	terminal time [l time]
	terminal equals {=}
	terminal durationSeconds {[[:digit:]]{1,2}[.][[:digit:]]{3} s}
	terminal durationMinutes {[[:digit:]]{1,2}:[[:digit:]]{2}[.][[:digit:]]{3} m:s}
	terminal durationHours {[[:digit:]]+:[[:digit:]]{2}:[[:digit:]]{2}[.][[:digit:]]{3} h:m:s}

	nonterminal duration { durationSeconds | durationMinutes | durationHours }

	nonterminal solvingTime { space time space equals space duration }

        nonterminal solutionEnd { endOfSolution | partialSolution }
        nonterminal block { eol solutionEnd solvingTime? eol emptyLine emptyLine }
    }

    namespace eval zeroposition {
        terminal zeroposition [l zeroposition]

        nonterminal block { emptyLine zeroposition eol emptyLine }
    }

    namespace eval inputerror {
        terminal inputError [l inputError]
        terminal offendingItem [l offendingItem]

        nonterminal block {
            inputError lineText+ eol
            offendingItem lineText+ eol
        }
    }

    namespace eval problem {
	terminal popeye [l popeye]

	nonterminal popeyeLine { popeye lineText+ eol }

        nonterminal noNonboardBlock {
            meta::block
            boardA::block?
            board::block
            caption::block
            conditions::block
            duplex::block
            gridboard::block?
            zeroposition::block?
        }

	nonterminal block {
	    popeyeLine?
	    inputerror::block*
	    remarkOrValidationError::block
	    noNonboardBlock?
	    solution::block?
	    footer::block
	}
    }
}

foreach inputfile $inputfiles {
    puts "parsing input file: $inputfile"
    
    set differ [open "| diff $inputfile -" "r+"]

    set f [open $inputfile "r"]
    set input [read $f]
    close $f

    set problemIndices [regexp -all -indices -inline $format::problem::block $input]

    foreach problemIndexPair $problemIndices {
	lassign $problemIndexPair problemStart problemEnd
	set problem [string range $input $problemStart $problemEnd]
	puts -nonewline $differ $problem
    }

    chan close $differ "write"
    
    set differences {}
    while {[gets $differ line]>=0} {
	lappend differences $line
    }

    if {[catch {
	close $differ
    } cres e]} {
	puts "differences found:"
	puts [join $differences "\n"]
    }
}
