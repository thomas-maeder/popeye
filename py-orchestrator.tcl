#! /usr/bin/env tclsh

package require cmdline
package require control
package require debug

# debug.something has the nasty habit of subst-in its argument
# to protect it from subst, do something like
# debug.something "argument:[debuggable $argument]"
proc debuggable {string} {
    return [regsub -all {\[} $string {[return "\["]}]
}

control::control assert enabled 0

# define the debugging tags in deactivated state
# they can be activated using command line options
debug off board
debug off cmdline
debug off input
debug off output
debug off language
debug off movenumbers
debug off popeye
debug off problem
debug off processes
debug off solution
debug off sync
debug off twin
debug off weight
debug off whomoves

# shamelessly copied from
# https://stackoverflow.com/questions/29482303/how-to-find-the-number-of-cpus-in-tcl
# Thanks, guys!!
proc detectNumberOfCPUs {} {
    # Windows puts it in an environment variable
    global tcl_platform env
    if {$tcl_platform(platform) eq "windows"} {
        return $env(NUMBER_OF_PROCESSORS)
    }

    # Check for sysctl (OSX, BSD)
    set sysctl [auto_execok "sysctl"]
    if {[llength $sysctl]} {
        if {![catch {exec {*}$sysctl -n "hw.ncpu"} cores]} {
            return $cores
        }
    }

    # Assume Linux, which has /proc/cpuinfo, but be careful
    if {![catch {
	open "/proc/cpuinfo"
    } f]} {
        set cores [regexp -all -line {^processor\s} [read $f]]
        close $f
        if {$cores > 0} {
            return $cores
        }
    }

    # No idea what the actual number of cores is; exhausted all our options
    # Fall back to returning 1; there must be at least that because we're running on it!
    return 1
}

proc defaultNumberOfProcesses {} {
    set detected [detectNumberOfCPUs]
    if {$detected==1} {
	# assume detection error - after all, there is a reason why
	# you invoke this wrapper rather than the Popeye executable...
	set result 2
    } elseif {$detected<=4} {
	set result 2
    } else {
	set result [expr {$detected/2}]
    }
}

proc defaultPopeyeExecutable {} {
    set basenames(unix) { py }
    set basenames(windows) { pywin64.exe pywin32.exe }

    set result ""

    if {[info exists basenames($::tcl_platform(platform))]} {
	set pwd [file dirname $::argv0]

	foreach basename $basenames($::tcl_platform(platform)) {
	    set candidate [file join $pwd $basename]

	    set find [glob -nocomplain $candidate]
	    if {[llength $find]>0} {
		set result [lindex $find 0]
		break
	    }
	}
    }

    return $result
}

namespace eval language {
    namespace eval francais {
	variable BeginProblem "DebugProblem"
	variable NextProblem "Asuivre"
	variable EndProblem "FinProblem"
	variable ZeroPosition "zeroposition"
	variable Twin "Jumeau"
	variable Title "Titre"
	variable Author "Auteur"
	variable Origin "Source"
	variable Remark "Remarque"
	variable Protocol "Protocol"
	variable LaTeX "LaTeX"
	variable Stipulation "Enonce"
	variable Option "Option"
	variable MoveNumber "Trace"
	variable StartMoveNumber "Enroute"
	variable UpToMoveNumber "JusquAuCoup"
	variable NoBoard "SansEchiquier"
	variable MaxSolutions "MaxSolutions"
	variable HalfDuplex "DemiDuplex"
	variable Rotate "Rotation"
	variable Add "ajoute"
	variable Remove "ote"
	variable Neutral "Neutre"
	variable Pawn "p"
	variable Time "Temps"
	variable SolutionFinished "solution terminee"
	variable SquareIsEmptyRE "la case est vide - on ne peut pas oter ou deplacer une piece."
	variable BothSidesNeedAKingRE "il faut un roi de chaque couleur"
    }

    namespace eval deutsch {
	variable BeginProblem "AnfangProblem"
	variable NextProblem "WeiteresProblem"
	variable EndProblem "EndeProblem"
	variable ZeroPosition "NullStellung"
	variable Twin "Zwilling"
	variable Title "Titel"
	variable Author "Autor"
	variable Origin "Quelle"
	variable Remark "Bemerkung"
	variable Protocol "Protokoll"
	variable LaTeX "LaTeX"
	variable Stipulation "Forderung"
	variable Option "Option"
	variable MoveNumber "ZugNummer"
	variable StartMoveNumber "StartZugnummer"
	variable UpToMoveNumber "BisZugnummer"
	variable NoBoard "OhneBrett"
	variable MaxSolutions "MaxLoesungen"
	variable HalfDuplex "HalbDuplex"
	variable Rotate "Drehung"
	variable Add "hinzufuegen"
	variable Remove "entferne"
	variable Neutral "Neutral"
	variable Pawn "b"
	variable Time "Zeit"
	variable SolutionFinished "Loesung beendet"
	variable SquareIsEmptyRE "Feld leer - kann keine Figur entfernen/versetzen."
	variable BothSidesNeedAKingRE "Es fehlt ein weisser oder schwarzer Koenig\n"
    }

    namespace eval english {
	variable BeginProblem "BeginProblem"
	variable NextProblem "NextProblem"
	variable EndProblem "EndProblem"
	variable ZeroPosition "zeroposition"
	variable Twin "Twin"
	variable Title "Title"
	variable Author "Author"
	variable Origin "Origin"
	variable Remark "Remark"
	variable Protocol "Protocol"
	variable LaTeX "LaTeX"
	variable Stipulation "Stipulation"
	variable Option "Option"
	variable MoveNumber "MoveNumber"
	variable StartMoveNumber "StartMoveNumber"
	variable UpToMoveNumber "UpToMoveNumber"
	variable NoBoard "NoBoard"
	variable MaxSolutions "MaxSolutions"
	variable HalfDuplex "HalfDuplex"
	variable Rotate "Rotate"
	variable Add "add"
	variable Remove "remove"
	variable Neutral "Neutral"
	variable Pawn "p"
	variable Time "Time"
	variable SolutionFinished "solution finished"
	variable SquareIsEmptyRE "square is empty - cannot .re.move any piece."
	variable BothSidesNeedAKingRE "both sides need a king"
    }

    variable detected
}

proc ::language::getAll {} {
    set result [namespace children]

    debug.language "getAll <- $result"
    return $result
}

proc ::language::setDetected {language} {
    variable detected

    debug.language "setDetected language:$language"

    set detected $language
}

proc ::language::getDetected {} {
    variable detected

    return $detected
}

proc ::language::getSelector {} {
    variable detected

    debug.language "getSelector"

    set result [set ${detected}::BeginProblem]

    debug.language "getLanguageSelector <- $result"
    return $result
}

proc ::language::tokenIsElement {token elementId {language ""}} {
    variable detected

    if {$language==""} {
	set language $detected
    }
    debug.language "tokenIsElement token:$token elementId:$elementId language:$language"

    set tokenLength [string length $token]

    set elementString [set ${language}::$elementId]
    debug.language "elementString:$elementString" 2

    set elementStringFragment [string range $elementString 0 [expr {$tokenLength-1}]]
    debug.language "elementStringFragment:$elementStringFragment" 2

    set result [expr {[string compare -nocase $elementStringFragment $token]==0}]

    debug.language "tokenIsElement <- $result"
    return $result
}

proc ::language::getElement {elementId} {
    variable detected

    debug.input "getElement elementId:$elementId"

    set result [set ${detected}::$elementId]

    debug.input "getElement <- $result"
    return $result
}

namespace eval input {
    variable lineBuffer ""
    variable minTokenLength 3

    variable whiteSpaceRE {[[:blank:]]}
    variable tokenRE {[^[:blank:]]}

    variable lineElements {
	Title
	Author
	Origin
	Remark
	Protocol
	LaTeX
    }
}

proc ::input::nextToken {chan} {
    variable lineBuffer
    variable whiteSpaceRE
    variable tokenRE

    debug.input "nextToken"

    while {true} {
	debug.input "lineBuffer:|[debuggable $lineBuffer]|" 2
	if {[regexp -- "$whiteSpaceRE*($tokenRE+)(.*)" $lineBuffer - result lineBuffer]} {
	    break
	} elseif {[gets $chan lineBuffer]<0} {
	    set result ""
	    break
	}
    }

    debug.input "nextToken <- $result"
    return $result
}

proc ::input::reportNoLanguageDetected {token} {
    debug.input "::input::reportNoLanguageDetected token:$token"

    puts stderr "erreur d'entree:pas debut probleme"
    puts stderr "element d'offenser: $token"
    exit
}

proc ::input::isLineElement {token} {
    variable lineElements

    debug.input "isLineElement token:$token"

    set result false

    foreach elmt $lineElements {
	if {[::language::tokenIsElement $token $elmt]} {
	    set result true
	    break
	}
    }

    debug.input "isLineElement <- $result"
    return $result
}

proc ::input::detectLanguage {chan} {
    variable minTokenLength

    debug.input "detectLanguage"

    set languages [::language::getAll]
    debug.input "languages:$languages" 2

    set token [nextToken $chan]
    set tokenLength [string length $token]
    debug.input "tokenLength:$tokenLength" 2

    if {$tokenLength<$minTokenLength} {
	reportNoLanguageDetected $token
    } else {
	foreach language $languages {
	    if {[::language::tokenIsElement $token BeginProblem $language]} {
		::language::setDetected $language
		break
	    }
	}

	if {[catch {
	    ::language::getDetected
	}]} {
	    reportNoLanguageDetected $token
	}
    }

    debug.input "detectLanguage <-"
}

proc ::input::readUpTo {chan elementIds} {
    variable lineBuffer

    debug.input "readUpTo elementIds:$elementIds"

    set skipped ""
    while {true} {
	set token [nextToken $chan]
	debug.input "token:$token" 2

	foreach id $elementIds {
	    if {[::language::tokenIsElement $token $id]} {
		debug.input "identified element $id" 2
		set result [list $skipped $id]
		break
	    }
	}

	if {[info exists result]} {
	    break
	} elseif {[isLineElement $token]} {
	    append skipped "$token $lineBuffer\n"
	    set lineBuffer ""
	} else {
	    append skipped "$token "
	}
	debug.input "skipped:|$skipped|"
    }

    debug.input "readUpTo <- $result"
    return $result
}

proc ::input::getLine {} {
    variable lineBuffer

    debug.input "getLine"

    set result $lineBuffer
    set lineBuffer ""

    debug.input "getLine <- $result"
    return $result
}

namespace eval output {
    variable protocol

    variable isGreetingLineSuppressed false
    variable isBoardSuppressed
    variable isBoardTerminatorSuppressed
    variable areMovenumbersSuppressed true
    variable isSolutionTerminatorSuppressed true
}

proc ::output::openProtocol {path} {
    variable protocol

    debug.output "openProtocol $path"

    close
    set protocol [::open $path "w+"]

    debug.output "openProtocol <-"
}

proc ::output::closeProtocol {} {
    variable protocol

    debug.output "closeProtocol"

    if {[info exists protocol]} {
	::close $protocol
	unset protocol
    }

    debug.output "closeProtocol <-"
}

proc ::output::_puts {string} {
    variable protocol

    ::puts -nonewline $string

    if {[info exists protocol]} {
	::puts -nonewline $protocol $string
    }
}

proc ::output::greetingLine {greetingLine} {
    variable isGreetingLineSuppressed

    if {!$isGreetingLineSuppressed} {
	_puts $greetingLine
	set isGreetingLineSuppressed true
    }
}

proc ::output::board {board terminator} {
    variable isBoardSuppressed
    variable isBoardTerminatorSuppressed

    if {!$isBoardSuppressed} {
	_puts "$board"
    }

    if {!$isBoardTerminatorSuppressed} {
	_puts "$terminator"
    }
}

proc ::output::enableMovenumbers {enable} {
    variable areMovenumbersSuppressed

    debug.output "enableMovenumbers enable:$enable"

    set areMovenumbersSuppressed [expr {!$enable}]
}

proc ::output::solution {string} {
    variable areMovenumbersSuppressed

    if {$areMovenumbersSuppressed} {
	set parenOpenRE {[\(]}
	set parenCloseRE {[\)]}
	set movenumberRE { *[[:digit:]]+}
	set moveRE {[^\n]+}
	set timeLabelRE [::language::getElement Time]
	set timeRE {[[:digit:]:.]+}
	set timeUnitRE {(?:(?:h:)?m:)?s}
	set movenumberLineRE "$movenumberRE +$parenOpenRE$moveRE +$timeLabelRE = $timeRE $timeUnitRE$parenCloseRE"
	debug.solution "movenumberLineRE:[debuggable $movenumberLineRE]" 2
	if {[regsub -all "\n${movenumberLineRE}" $string "" string]>0} {
	    debug.solution "removed move numbers - string:|$string|"
	}
    }

    _puts $string
}

proc ::output::solutionTerminator {terminator} {
    variable isSolutionTerminatorSuppressed

    if {!$isSolutionTerminatorSuppressed} {
	_puts $terminator
    }
}

namespace eval popeye {
    variable executablePath
}

proc ::popeye::setExecutable {path} {
    variable executablePath
    variable maxmemOption ""

    debug.popeye "setExecutable path:$path"

    set executablePath $path
}

proc ::popeye::setMaxmem {setting} {
    variable maxmemOption

    set maxmemOption "-maxmem $setting"
}

proc ::popeye::spawn {firstTwin options} {
    variable executablePath
    variable maxmemOption

    debug.popeye "spawn firstTwin:|$firstTwin| options:|$options|"

    set pipe [open "| $executablePath $maxmemOption" "r+"]
    fconfigure $pipe -encoding binary -buffering line

    gets $pipe greetingLine

    set result [list $pipe $greetingLine]

    puts $pipe [::language::getSelector]
    puts $pipe $firstTwin
    puts $pipe "[::language::getElement Option] $options"

    debug.popeye "spawn <- $result"
    return $result
}

proc ::popeye::terminate {pipe {expectedErrorMessageREs {}}} {
    debug.popeye "terminate pipe:$pipe expectedErrorMessageREs:$expectedErrorMessageREs"
    if {[catch {
	# empty the buffer first to avoid "child killed: write on pipe with no readers"
	# cf. https://stackoverflow.com/questions/38020500/tcl-script-breaks-with-child-killed-write-on-pipe-with-no-readers
	fconfigure $pipe -blocking false
	set pending [read $pipe]
	close $pipe
	if {$pending!=""} {
	    debug.popeye "pending:|[debuggable $pending]|" 2
	}
    } error]} {
	debug.popeye "caught error |[debuggable $error]|" 2
	foreach expectedErrorMesssageRE $expectedErrorMessageREs {
	    if {[regsub -all "$expectedErrorMesssageRE" $error "" error]>0} {
		debug.popeye "suppressed [debuggable $expectedErrorMesssageRE]" 2
		debug.popeye "error:[debuggable $error]" 2
	    }
	}
	if {$error!=""} {
	    puts stderr $error
	    exit 1
	}
    }
    debug.popeye "terminate <-"
}

namespace eval ::popeye::input {
}

proc ::popeye::input::ZeroPosition {pipe twinning} {
    debug.popeye "input::ZeroPosition twinning:|$twinning|"

    puts $pipe "[::language::getElement ZeroPosition] $twinning"
}

proc ::popeye::input::Twin {pipe twinning} {
    debug.popeye "input::Twin twinning:|$twinning|"

    puts $pipe "[::language::getElement Twin] $twinning"
}

proc ::popeye::input::EndProblem {pipe} {
    debug.popeye "input::EndProblem"

    puts $pipe "[::language::getElement EndProblem]"
}

namespace eval ::popeye::output {
}

proc ::popeye::output::getLine {pipe varname} {
    upvar $varname line

    return [gets $pipe line]
}

proc ::popeye::output::doAsync {pipe listener arguments} {
    debug.popeye "output::doAsync pipe:$pipe listener:$listener arguments:[debuggable $arguments]"

    fconfigure $pipe -blocking false

    set callback [linsert $arguments 0 $listener $pipe]
    fileevent $pipe readable $callback
}

proc parseCommandLine {} {
    set options [subst {
	{ executable.arg "[defaultPopeyeExecutable]"  "path to Popeye executable" }
	{ nrprocs.arg    "[defaultNumberOfProcesses]" "number of Popeye processes to spawn" }
	{ maxmem.arg     "Popeye default"             "maximum memory for each process" }
    }]
    if {[string match "*.tcl" $::argv0]} {
	lappend options { assert false "enable asserts" }
	foreach name [debug names] {
	    lappend options [subst { debug-$name.arg "0" "debug level for tag $name" }]
	}
    }

    set usage ": [::cmdline::getArgv0] \[options] \[inputfile]\noptions:"

    try {
	array set ::params [::cmdline::getoptions ::argv $options $usage]
    } trap {CMDLINE USAGE} {msg o} {
	puts stderr $msg
	exit 1
    }

    if {$::params(executable)==""} {
	puts stderr [::cmdline::usage $options $usage]
	exit 1
    } else {
	::popeye::setExecutable $::params(executable)
    }
    if {$::params(maxmem)!="Popeye default"} {
	::popeye::setMaxmem $::params(maxmem)
    }

    if {[llength $::argv]>0} {
	set ::params(inputfile) [lindex $::argv 0]
    }

    if {$::params(assert)} {
	control::control assert enabled 1
    }

    foreach name [array names ::params -glob "debug-*"] {
	if {$::params($name)>0 && [regexp -- {debug-(.*)} $name - tag]} {
	    debug on $tag
	    debug level $tag $::params($name)
	}
    }

    debug.cmdline "argv0:$::argv0 argv:$::argv"
    debug.cmdline "options:$options"
    debug.cmdline params:[debug parray ::params]
}

namespace eval sync {
    variable latestNotification
}

proc ::sync::Init {} {
    variable latestNotification

    set latestNotification ""
}

proc ::sync::Fini {} {
    variable latestNotification

    unset latestNotification
}

proc ::sync::Wait {callback state} {
    variable latestNotification

    debug.sync "Wait callback:$callback state:[debuggable $state]"

    while {true} {
	debug.sync "vwait latestNotification:$latestNotification" 2
	vwait ::sync::latestNotification
	set state [eval $callback $latestNotification $state]
    }

    debug.sync "Wait <-"
}

proc ::sync::Notify {sender notification} {
    variable latestNotification

    debug.sync "Notify sender:$sender notification:$notification"
    set latestNotification [list $sender $notification]
}

namespace eval tester {
}

proc ::tester::flushSolution {pipe chunk solutionTerminatorRE} {
    debug.solution "flushSolution pipe:$pipe chunk:|[debuggable $chunk]| solutionTerminatorRE:[::debuggable $solutionTerminatorRE]"

    if {[regexp -- "^(.*)($solutionTerminatorRE)(.*)$" $chunk - solution terminator remainder]} {
	debug.solution "solution:|[debuggable $solution]|"
	debug.solution "terminator:|$terminator|"

        ::sync::Notify $pipe "solution"

	::output::solution $solution
	::output::solutionTerminator $terminator

	set result true
    } else {
	debug.solution "terminator not found" 2

	::output::solution $chunk
	set result false
    }

    flush stdout

    debug.solution "flushSolution <- $result"
    return $result
}

proc ::tester::solution {pipe solutionTerminatorRE} {
    debug.solution "solution pipe:$pipe"

    flushSolution $pipe [read $pipe] $solutionTerminatorRE
}

proc ::tester::board {pipe boardTerminatorRE solutionTerminatorRE {chunk ""}} {
    debug.board "board pipe:$pipe chunk:|[debuggable $chunk]|"

    append chunk [read $pipe]
    debug.board "chunk:|$chunk|"

    if {[regexp -- "(.*)($boardTerminatorRE)(.*)" $chunk - board terminator remainder]} {
	debug.board "terminator:|[debuggable $terminator]|"
	debug.board "remainder:|[debuggable $remainder]|"
	::output::board $board $terminator
	::sync::Notify $pipe "board"
	if {![flushSolution $pipe $remainder $solutionTerminatorRE]} {
	    ::popeye::output::doAsync $pipe ::tester::solution [list $solutionTerminatorRE]
	}
    } else {
	debug.board "terminator not found"
	::popeye::output::doAsync $pipe ::tester::board [list $boardTerminatorRE $solutionTerminatorRE $chunk]
    }
}

proc ::tester::moveRange {firstTwin endElmt twinnings boardTerminatorRE solutionTerminatorRE moveRange} {
    debug.processes "testMoveRange firstTwin:|$firstTwin| endElmt:$endElmt twinnings:$twinnings boardTerminatorRE:$boardTerminatorRE solutionTerminatorRE:$solutionTerminatorRE moveRange:$moveRange"

    lassign $moveRange start upto

    set options "[::language::getElement MoveNumber] [::language::getElement UpToMoveNumber] $upto"
    if {$start>1} {
	append options " [::language::getElement StartMoveNumber] $start"
    }

    lassign [::popeye::spawn $firstTwin $options] pipe greetingLine
    debug.processes "pipe:$pipe" 2

    ::output::greetingLine $greetingLine

    debug.processes "inserting accumulated twinnings" 2
    foreach t $twinnings {
	lassign $t key twinning
	::popeye::input::$key $pipe $twinning
    }

    ::popeye::input::EndProblem $pipe

    ::popeye::output::doAsync $pipe ::tester::board [list $boardTerminatorRE $solutionTerminatorRE]

    debug.processes "testMoveRange <-"
}

proc ::tester::moveRangesProgress {endElmt pipe notification nrRunningProcesses nrBoardsRead} {
    debug.processes "testMoveRangesProgress endElmt:$endElmt pipe:$pipe notification:$notification nrRunningProcesses:$nrRunningProcesses nrBoardsRead:$nrBoardsRead"

    if {$notification=="board"} {
	incr nrBoardsRead
    } else {
	::popeye::terminate $pipe
	incr nrRunningProcesses -1
    }

    # we print board and board terminator only once (if at all)
    if {$nrBoardsRead==1} {
	set ::output::isBoardSuppressed true
	set ::output::isBoardTerminatorSuppressed true
    }

    # the next iteration deals with the last bit of solution of the problem - activate output of solution terminator
    if {$endElmt!="Twin" && $nrRunningProcesses==1} {
	set ::output::isSolutionTerminatorSuppressed false
    }

    if {$nrRunningProcesses==0} {
	debug.processes "testMoveRangesProgress <- break"
	return -code break
    } else {
	set result [list $nrRunningProcesses $nrBoardsRead]
	debug.processes "testMoveRangesProgress <- $result"
	return $result
    }
}

proc ::tester::moveRanges {firstTwin twinnings endElmt moveRanges} {
    debug.processes "testMoveRanges firstTwin:|$firstTwin| twinnings:$twinnings endElmt:$endElmt moveRanges:$moveRanges"

    ::sync::Init

    set ::output::isSolutionTerminatorSuppressed true

    set twinningRE {\n\n..?[\)] [^\n]*\n}

    if {[llength $twinnings]==0 && $endElmt!="Twin"} {
	debug.processes "no twin and no zeroposition - inserting fake zero position as board terminator" 2
	set twinnings [linsert $twinnings 0 [list "ZeroPosition" "[::language::getElement Rotate] 90 [::language::getElement Rotate] 270"]]
	set fakeZeroPositionRE "\n[::language::getElement ZeroPosition].*?270 *\n"
	set boardTerminatorRE $fakeZeroPositionRE
	set ::output::isBoardTerminatorSuppressed true
    } else {
	set boardTerminatorRE $twinningRE
    }

    if {$endElmt=="Twin"} {
	debug.processes "inserting fake twinning as solution terminator" 2
	lappend twinnings [list Twin "rotate 180"]
	set solutionTerminatorRE $twinningRE
    } else {
	set solutionFinishedRE "\n\n[::language::getElement SolutionFinished]\[^\n]+\n\n\n"
	set solutionTerminatorRE $solutionFinishedRE
    }

    set testMoveRangeArguments [list $firstTwin $endElmt $twinnings $boardTerminatorRE $solutionTerminatorRE]

    set nrRunningProcesses 0
    foreach moveRange $moveRanges {
	moveRange {*}$testMoveRangeArguments $moveRange
	incr nrRunningProcesses
    }

    set nrBoardsRead 0
    ::sync::Wait [namespace code [list moveRangesProgress $endElmt]] [list $nrRunningProcesses $nrBoardsRead]

    ::sync::Fini

    debug.processes "testMoveRanges <-"
}

proc groupByWeight {weights skipMoves} {
    debug.weight "groupByWeight weights:$weights skipMoves:$skipMoves"

    set weightTotal 0
    foreach weight $weights {
	incr weightTotal $weight
    }
    debug.weight "weightTotal:$weightTotal" 2

    set avgWeightPerProcess [expr {($weightTotal+$::params(nrprocs)-1)/$::params(nrprocs)}]
    debug.weight "weightTotal:$weightTotal avgWeightPerProcess:$avgWeightPerProcess" 2

    set start [expr {$skipMoves+1}]
    set curr $skipMoves
    set weightTarget $avgWeightPerProcess
    set weightAccumulated 0

    set result {}

    foreach weight $weights {
	incr curr
	incr weightAccumulated $weight
	debug.weight "curr:$curr weight:$weight weightTarget:$weightTarget weightAccumulated:$weightAccumulated" 2
	if {$weightAccumulated>$weightTarget} {
	    set weightExcess [expr {$weightAccumulated-$weightTarget}]
	    debug.processes "weightExcess:$weightExcess"
	    if {$weightExcess>$weight/2} {
		lappend result [list $start [expr {$curr-1}]]
		set start $curr
	    } else {
		lappend result [list $start $curr]
		set start [expr {$curr+1}]
	    }
	    incr weightTarget $avgWeightPerProcess
	}
    }
    lappend result [list $start [expr {$skipMoves+[llength $weights]}]]

    debug.weight "groupByWeight <- $result"
    return $result
}

proc findMoveWeights {firstTwin twinnings whomoves skipmoves} {
    debug.weight "findMoveWeights firstTwin:|$firstTwin| twinnings:$twinnings whomoves:$whomoves skipmoves:$skipmoves"

    set options "[::language::getElement NoBoard] [::language::getElement MoveNumber] [::language::getElement StartMoveNumber] [expr {$skipmoves+1}]"
    if {$whomoves=="black"} {
	append options " [::language::getElement HalfDuplex]"
    }
    debug.weight "options:$options"

    lassign [::popeye::spawn $firstTwin $options] pipe greetingLine

    if {[llength $twinnings]==0} {
	::popeye::input::ZeroPosition $pipe "[::language::getElement Stipulation] ~1"
    } else {
	foreach t $twinnings {
	    lassign $t key twinning
	    ::popeye::input::$key $pipe "$twinning [::language::getElement Stipulation] ~1"
	}
    }
    ::popeye::input::EndProblem $pipe

    set result {}
    while {[::popeye::output::getLine $pipe line]>=0} {
	debug.weight "line:[debuggable $line]" 2
	append output "$line\n"
	switch -glob $line {
	    "*[::language::getElement Time] = *" {
		debug.weight "next move" 2
		# this matches both
		#  21  (Ke6-e7    Time = 0.016 s)
		# solution finished. Time = 0.016 s
		# so all moves are taken care of
		if {[info exists move]} {
		    lappend result $weight
		    debug.weight "[llength $result] move:$move weight:$weight" 2
		}
		set weight 0
		regexp -- {[(]([^ ]+).*[)]} $line - move
	    }
	    {*+ !} {
		debug.weight "checking move" 2
		set weight 2
	    }
	    {*!} {
		debug.weight "non-checking move" 2
		set weight 20
	    }
	    {*TI~-~*} {
		debug.weight "random move by TI" 2
		set weight 100
	    }
	    default {
		debug.weight "other line" 2
	    }
	}
    }

    ::popeye::terminate $pipe

    debug.weight "findMoveWeights <- $result"
    return $result
}

proc whoMoves {twin twinnings} {
    debug.whomoves "whoMoves twin:|$twin| twinnings:$twinnings"

    set options "[::language::getElement NoBoard] [::language::getElement MoveNumber] [::language::getElement StartMoveNumber] 1 [::language::getElement UpToMoveNumber] 1"

    set accumulatedZeroposition ""
    foreach t $twinnings {
	lassign $t key twinning
	debug.whomoves "key:$key twinning:$twinning"
	append accumulatedZeroposition "$twinning "
    }
    foreach row {1 2 3 4 5 6 7 8} {
	foreach col {a b c d e f g h} {
	    append accumulatedZeroposition "[::language::getElement Remove] $col$row "
	}
    }
    append accumulatedZeroposition "[::language::getElement Add] [::language::getElement Neutral] [::language::getElement Pawn]b5"
    debug.whomoves accumulatedZeroposition:$accumulatedZeroposition

    lassign [::popeye::spawn $twin $options] pipe greetingLine

    ::popeye::input::ZeroPosition $pipe "$accumulatedZeroposition"
    ::popeye::input::EndProblem $pipe

    set result "white"
    debug.whomoves "assuming white"
    while {[::popeye::output::getLine $pipe line]>=0} {
	debug.whomoves "line:[debuggable $line]" 2
	if {[string first "b5-b4" $line]>=0} {
	    set result "black"
	    debug.whomoves "detected black"
	    break
	} elseif {[string first "b5-b6" $line]>=0} {
	    debug.whomoves "detected white"
	    break
	}
    }

    # don't confuse caller with errors caused by us
    lappend expectedErrorMessageREs "..: [::language::getElement SquareIsEmptyRE]\\n?"
    lappend expectedErrorMessageREs "[::language::getElement BothSidesNeedAKingRE]"

    ::popeye::terminate $pipe $expectedErrorMessageREs

    debug.whomoves "result:$result"
    return $result
}

proc handleTwin {firstTwin endElmt twinnings skipMoves} {
    debug.twin "handleTwin firstTwin:|$firstTwin| endElmt:$endElmt twinnings:$twinnings skipMoves:$skipMoves"

    set ::output::isBoardTerminatorSuppressed false

    set whomoves [whoMoves $firstTwin $twinnings]
    debug.twin "whomoves:$whomoves" 2

    set weights [findMoveWeights $firstTwin $twinnings $whomoves $skipMoves]
    debug.twin "weights:$weights" 2

    set groups [groupByWeight $weights $skipMoves]
    debug.twin "groups:$groups" 2

    ::tester::moveRanges $firstTwin $twinnings $endElmt $groups

    set result [llength $weights]

    debug.twin "handleTwin <- $result"
    return $result
}

proc areMoveNumbersActivated {firstTwin zeroTwinning} {
    debug.movenumbers "areMoveNumbersActivated firstTwin:|$firstTwin| zeroTwinning:$zeroTwinning"

    set options "[::language::getElement MaxSolutions] 1"
    lassign [::popeye::spawn $firstTwin $options] pipe greetingLine

    ::popeye::input::ZeroPosition $pipe "$zeroTwinning [::language::getElement Stipulation] ~1"
    ::popeye::input::EndProblem $pipe

    set result false
    while {[::popeye::output::getLine $pipe line]>=0} {
	debug.movenumbers "line:[debuggable $line]" 2
	set movenumbersRE { *[[:digit:]]+ +[\(][^\)]+[::language::getElement Time] = [^\)]+[\)]}
	if {[regexp -- "^$movenumbersRE\$" $line]} {
	    set result true
	    break
	}
    }

    ::popeye::terminate $pipe

    debug.movenumbers "areMoveNumbersActivated <- $result"
    return $result
}

proc readFirstTwin {chan} {
    debug.problem "readFirstTwin"

    lassign [::input::readUpTo $chan {Twin ZeroPosition NextProblem EndProblem Protocol}] firstTwin endElmt
    while {true} {
	if {$endElmt=="Protocol"} {
	    set protocol [string trim [::input::getLine]]
	    lassign [::input::readUpTo $chan {Twin ZeroPosition NextProblem EndProblem Protocol}] firstTwinPart2 endElmt
	    append firstTwin $firstTwinPart2
	} else {
	    break
	}
    }

    if {[info exists protocol]} {
	::output::openProtocol $protocol
    }

    set result [list $firstTwin $endElmt]

    debug.problem "readFirstTwin <- $result"
    return $result
}

proc handleFirstTwin {chan} {
    debug.problem "handleFirstTwin"

    lassign [readFirstTwin $chan] firstTwin endElmt

    set twinnings {}

    if {$endElmt=="ZeroPosition"} {
	lassign [::input::readUpTo $chan {Twin NextProblem EndProblem}] zeroTwinning endElmt
	lappend twinnings [list "ZeroPosition" $zeroTwinning]
	::output::enableMovenumbers [areMoveNumbersActivated $firstTwin $zeroTwinning]
    } else {
	::output::enableMovenumbers [areMoveNumbersActivated $firstTwin ""]
    }

    set nrFirstMoves [handleTwin $firstTwin $endElmt $twinnings 0]
    set result [list $firstTwin $twinnings $endElmt $nrFirstMoves]

    debug.problem "handleFirstTwin <- $result"
    return $result
}

proc handleProblem {chan} {
    debug.problem "handleProblem"

    set ::output::isBoardSuppressed false

    lassign [handleFirstTwin $chan] firstTwin twinnings endElmt nrFirstMoves

    while {$endElmt=="Twin"} {
	lassign [::input::readUpTo $chan {Twin NextProblem EndProblem}] twinning endElmt

	lappend twinnings [list "Twin" $twinning]
	debug.problem "twinnings:$twinnings" 2

	incr nrFirstMoves [handleTwin $firstTwin $endElmt $twinnings $nrFirstMoves]
    }

    set result $endElmt
    debug.problem "handleProblem <- $result"
    return $result
}

proc handleInput {chan} {
    ::input::detectLanguage $chan

    set endElmt [handleProblem $chan]
    while {$endElmt=="NextProblem"} {
	set endElmt [handleProblem $chan]
    }
}

proc main {} {
    parseCommandLine
    if {[info exists ::params(inputfile)]} {
	set chan [open $::params(inputfile) "r"]
	handleInput $chan
	close $chan
    } else {
	handleInput stdin
    }

    ::output::closeProtocol
}

main
