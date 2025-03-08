#! /usr/bin/env tclsh

package require cmdline
package require debug

# debug.something has the nasty habit of subst-in its argument
# to protect it from subst, do something like
# debug.something "argument:[debuggable $argument]"
proc debuggable {string} {
    return [regsub -all {\[} $string {[return "\["]}]
}

# define the debugging tags in deactivated state
# they can be activated using command line options
debug off board
debug off cmdline
debug off input
debug off movenumbers
debug off problem
debug off processes
debug off solution
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
    namespace eval english {
	variable BeginProblem "BeginProblem"
	variable NextProblem "NextProblem"
	variable EndProblem "EndProblem"
	variable Zero "ZeroPosition"
	variable Twin "Twin"
	variable MoveNumber "MoveNumber"
    }
}

namespace eval input {
    variable detectedLanguage
    variable lineBuffer ""
    variable minTokenLength 3

    variable whiteSpaceRE {[[:blank:]]}
    variable tokenRE {[^[:blank:]]}
}

proc ::input::getLanguageSelector {} {
    variable detectedLanguage

    debug.input "getLanguageSelector"

    set result [set ${detectedLanguage}::BeginProblem]

    debug.input "getLanguageSelector <- $result"
    return $result
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

proc ::input::tokenIsElement {token language elementId} {
    debug.input "tokenIsElement token:$token language:$language elementid:$elementId"

    set tokenLength [string length $token]

    set elementString [set ${language}::$elementId]
    debug.input "elementString:$elementString" 2

    set elementStringFragment [string range $elementString 0 [expr {$tokenLength-1}]]
    debug.input "elementStringFragment:$elementStringFragment" 2

    set result [expr {[string compare -nocase $elementStringFragment $token]==0}]

    debug.input "tokenIsElement <- $result"
    return $result
}

proc ::input::detectLanguage {chan} {
    variable detectedLanguage
    variable minTokenLength

    debug.input "detectLanguage"

    set languages [namespace children ::language]
    debug.input "languages:$languages" 2

    set token [nextToken $chan]
    set tokenLength [string length $token]
    debug.input "tokenLength:$tokenLength" 2

    if {$tokenLength<$minTokenLength} {
	reportNoLanguageDetected $token
    } else {
	foreach language $languages {
	    if {[tokenIsElement $token $language BeginProblem]} {
		set detectedLanguage $language
		debug.input "detectedLanguage:$detectedLanguage"
		break
	    }
	}

	if {![info exists detectedLanguage]} {
	    reportNoLanguageDetected $token
	}
    }

    debug.input "detectLanguage <-"
}

namespace eval frontend {
    namespace eval olive {
	variable MoveNumber "MoveNumber"
	variable Zero "Zero"
	variable Twin "Twin"
	variable NextProblem "NextProblem"
	variable EndProblem "EndProblem"
    }
}

variable frontend::selected "olive"

proc frontend::get {id} {
    variable selected

    return [set ${selected}::$id]
}

proc parseCommandLine {} {

    set frontends {}
    foreach frontend [namespace children ::frontend] {
	lappend frontends [namespace tail $frontend]
    }
    set options [subst {
	{ executable.arg "[defaultPopeyeExecutable]"  "path to Popeye executable" }
	{ nrprocs.arg    "[defaultNumberOfProcesses]" "number of Popeye processes to spawn" }
	{ maxmem.arg     "Popeye default"             "maximum memory for each process" }
	{ frontend.arg   "$::frontend::selected"      "frontend invoking [::cmdline::getArgv0]; one of ([join $frontends ,])" }
    }]
    if {[string match "*.tcl" $::argv0]} {
	foreach name [debug names] {
	    lappend options [subst { debug-$name.arg "0" "debug level for tag $name" }]
	}
    }

    set usage ": [::cmdline::getArgv0] \[options] \[inputfile]\noptions:"

    try {
	array set ::params [::cmdline::getoptions ::argv $options $usage]
    } trap {CMDLINE USAGE} {msg o} {
	puts $msg
	exit 1
    }

    if {$::params(executable)==""} {
	puts stderr [::cmdline::usage $options $usage]
	exit 1
    }

    if {[llength $::argv]>0} {
	set ::params(inputfile) [lindex $::argv 0]
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

proc syncWait {value} {
    global processSync processValue

    debug.processes "syncWait value:$value"

    set processSync 0
    set processValue $value
    while {$processSync<$processValue} {
	debug.processes "vwait processSync:$processSync"
	vwait processSync
	debug.processes "vwait <- processSync:$processSync"
    }
    unset processSync

    debug.processes "syncWait <-"
}

proc syncEnded {} {
    global processSync processValue

    set result [expr {$processSync==$processValue}]
    debug.processes "syncEnded <- $result"
    return $result
}

proc syncNotify {} {
    global processSync

    incr processSync
    debug.processes processSync:$processSync
}

set movenumbersRE { *[[:digit:]]+ +[\(][^\)]+Time = [^\)]+[\)]}

proc flushSolution {pipe chunk solutionTerminatorRE movenumbers} {
    debug.solution "flushSolution pipe:$pipe chunk:|[debuggable $chunk]| movenumbers:$movenumbers"

    if {!$movenumbers} {
	regsub -all "\n$::movenumbersRE" $chunk "" chunk
    }
    
    if {[regexp -- "^(.*)($solutionTerminatorRE)(.*)$" $chunk - solution terminator remainder]} {
	debug.solution "solution:|[debuggable $solution]|"
	debug.solution "terminator:|$terminator|"

	puts -nonewline $solution

        syncNotify
	if {[syncEnded]} {
	    puts -nonewline $terminator
	}

	close $pipe
	set result true
    } else {
	debug.solution "terminator not found"

	puts -nonewline $chunk
	set result false
    }

    flush stdout

    debug.solution "flushSolution <- $result"
    return $result
}

proc solution {pipe solutionTerminatorRE movenumbers} {
    debug.solution "solution pipe:$pipe movenumbers:$movenumbers"

    flushSolution $pipe [read $pipe] $solutionTerminatorRE $movenumbers
}

proc firstTwin {pipe chunk boardTerminatorRE boardTerminatorSilentRE solutionTerminatorRE movenumbers} {
    debug.board "firstTwin pipe:$pipe chunk:|[debuggable $chunk]| movenumbers:$movenumbers"

    append chunk [read $pipe]
    debug.board "chunk:|$chunk|"

    if {[regexp -- "(.*)(${boardTerminatorRE})${boardTerminatorSilentRE}(.*)" $chunk - board terminator remainder]} {
	debug.board "terminator:|[debuggable $terminator]|"
	puts -nonewline "$board$terminator"
	syncNotify
	if {![flushSolution $pipe $remainder $solutionTerminatorRE $movenumbers]} {
	    fileevent $pipe readable [list solution $pipe $solutionTerminatorRE $movenumbers]
	}
    } else {
	debug.board "terminator not found"
	fileevent $pipe readable [list firstTwin $pipe $chunk $boardTerminatorRE $boardTerminatorSilentRE $solutionTerminatorRE $movenumbers]
    }
}

proc otherTwin {pipe chunk boardTerminatorSilentRE solutionTerminatorRE movenumbers} {
    debug.board "otherTwin pipe:$pipe chunk:|[debuggable $chunk]| boardTerminatorSilentRE:[debuggable $boardTerminatorSilentRE] solutionTerminatorRE:[debuggable $solutionTerminatorRE] movenumbers:$movenumbers"

    append chunk [read $pipe]
    debug.board "chunk:|$chunk|"

    if {[regexp -- "(.*)($boardTerminatorSilentRE)(.*)" $chunk - board terminator remainder]} {
	debug.board "terminator:|[debuggable $terminator]|"
	if {![flushSolution $pipe $remainder $solutionTerminatorRE $movenumbers]} {
	    fileevent $pipe readable [list solution $pipe $solutionTerminatorRE $movenumbers]
	}
    } else {
	debug.board "terminator not found"
	fileevent $pipe readable [list otherTwin $pipe $chunk $boardTerminatorSilentRE $solutionTerminatorRE $movenumbers]
    }
}

proc tryPartialTwin {problemnr firstTwin movenumbers endToken accumulatedTwinnings start upto processnr} {
    debug.processes "tryPartialTwin problemnr:$problemnr firstTwin:|$firstTwin| movenumbers:$movenumbers endToken:$endToken accumulatedTwinnings:$accumulatedTwinnings start:$start upto:$upto $processnr"

    set commandline $::params(executable)
    if {$::params(maxmem)!="Popeye default"} {
	append commandline " -maxmem $::params(maxmem)"
    }
    debug.processes "commandline:$commandline"

    set options "option MoveNumber Start $start Upto $upto"

    set pipe [open "| $commandline" "r+"]
    debug.processes "pipe:$pipe"

    if {[string match "[::frontend::get Twin]*" $endToken]} {
	set solutionTerminatorRE {\n\n..?[\)][^\n]+\n}
    } else {
	set solutionTerminatorRE {\n\nsolution finished[^\n]+\n\n\n}
    }

    gets $pipe greetingLine
    if {$problemnr==1 && $start==1} {
	# first twin of first problem: print greeting line
	puts $greetingLine
    }

    fconfigure $pipe -blocking false -encoding binary

    puts $pipe $firstTwin
    puts $pipe $options
    if {($endToken==[frontend::get "EndProblem"] || $endToken==[frontend::get "NextProblem"])
	&& [llength $accumulatedTwinnings]==0} {
	debug.processes "inserting fake zero position"
	puts $pipe "Zero rotate 90 rotate 270"
	puts $pipe "EndProblem"
	flush $pipe

	set boardTerminatorRE {\nzeroposition.*?270 *\n}
	if {$start==1} {
	    fileevent $pipe readable [list firstTwin $pipe "" "" $boardTerminatorRE $solutionTerminatorRE $movenumbers]
	    syncWait 1
	} else {
	    fileevent $pipe readable [list otherTwin $pipe "" $boardTerminatorRE $solutionTerminatorRE $movenumbers]
	}
    } else {
	debug.processes "inserting accumulated twinnings $accumulatedTwinnings"
	puts $pipe $accumulatedTwinnings
	puts $pipe $endToken
	if {!($endToken==[frontend::get "EndProblem"] || $endToken==[frontend::get "NextProblem"])} {
	    puts $pipe "EndProblem"
	}
	flush $pipe

	set boardTerminatorRE {\n..?[\)] [^\n]*\n}
	if {$start==1} {
	    fileevent $pipe readable [list firstTwin $pipe "" $boardTerminatorRE "" $solutionTerminatorRE $movenumbers]
	    syncWait 1
	} else {
	    fileevent $pipe readable [list otherTwin $pipe "" $boardTerminatorRE $solutionTerminatorRE $movenumbers]
	}
    }

    debug.processes "tryPartialTwin <-"
}

proc tryEntireTwin {problemnr firstTwin movenumbers endToken twinnings skipMoves weights weightTotal} {
    global isTwinningWritten

    debug.processes "tryEntireTwin problemnr:$problemnr firstTwin:|$firstTwin| movenumbers:$movenumbers endToken:$endToken twinnings:$twinnings skipMoves:$skipMoves weights:$weights weightTotal:$weightTotal"

    set isTwinningWritten false

    set accumulatedTwinnings ""
    foreach t $twinnings {
	lassign $t key twinning
	debug.processes "key:$key twinning:$twinning"
	append accumulatedTwinnings " $key $twinning"
    }
    debug.processes accumulatedTwinnings:$accumulatedTwinnings

    set avgWeightPerProcess [expr {($weightTotal+$::params(nrprocs)-1)/$::params(nrprocs)}]
    debug.processes "weightTotal:$weightTotal avgWeightPerProcess:$avgWeightPerProcess"

    set start [expr {$skipMoves+1}]
    set curr $skipMoves
    set weightTarget $avgWeightPerProcess
    set weightAccumulated 0
    set processnr 1

    foreach weight $weights {
	incr curr
	incr weightAccumulated $weight
	debug.processes "curr:$curr weight:$weight weightTarget:$weightTarget weightAccumulated:$weightAccumulated"
	if {$weightAccumulated>$weightTarget} {
	    set weightExcess [expr {$weightAccumulated-$weightTarget}]
	    debug.processes "weightExcess:$weightExcess"
	    if {$weightExcess>$weight/2} {
		tryPartialTwin $problemnr $firstTwin $movenumbers $endToken $accumulatedTwinnings $start [expr {$curr-1}] $processnr
		set start $curr
	    } else {
		tryPartialTwin $problemnr $firstTwin $movenumbers $endToken $accumulatedTwinnings $start $curr $processnr
		set start [expr {$curr+1}]
	    }
	    incr processnr
	    incr weightTarget $avgWeightPerProcess
	}
    }
    tryPartialTwin $problemnr $firstTwin $movenumbers $endToken $accumulatedTwinnings $start [expr {$skipMoves+[llength $weights]}] $processnr

    syncWait $processnr

    debug.processes "tryEntireTwin <-"
}

proc findMoveWeights {firstTwin twinnings whomoves skipmoves} {
    debug.weight "findMoveWeights firstTwin:|$firstTwin| twinnings:$twinnings whomoves:$whomoves skipmoves:$skipmoves"

    set options "option Noboard MoveNumber start [expr {$skipmoves+1}]"
    if {$whomoves=="black"} {
	append options " HalfDuplex"
    }
    debug.weight "options:$options"

    set isZero false
    set accumulatedTwinnings ""
    foreach t $twinnings {
	lassign $t key twinning
	debug.weight "key:$key twinning:$twinning"
	append accumulatedTwinnings "$key $twinning "
	if {$key==[frontend::get Zero]} {
	    set isZero true
	}
    }
    debug.weight accumulatedTwinnings:$accumulatedTwinnings

    set pipe [open "| $::params(executable)" "r+"]
    fconfigure $pipe -encoding binary -buffering line

    puts $pipe $firstTwin
    puts $pipe $options
    if {!$isZero} {
	puts $pipe "zeroposition stipulation ~1"
    }
    puts $pipe "$accumulatedTwinnings stipulation ~1"
    puts $pipe "EndProblem"

    set weightTotal 0
    while {[gets $pipe line]>=0} {
	debug.weight "line:[debuggable $line]" 2
	switch -regexp $line {
	    {Time = } {
		# this matches both
		#  21  (Ke6-e7    Time = 0.016 s)
		# solution finished. Time = 0.016 s
		# so all moves are taken care of
		if {[info exists move]} {
		    lappend weights $weight
		    debug.weight "[llength $weights] move:$move weight:$weight" 2
		    incr weightTotal $weight
		}
		set weight 0
		regexp -- {[(]([^ ]+).*[)]} $line - move
	    }
	    {[+] !} {
		set weight 2
	    }
	    {!} {
		set weight 20
	    }
	    {TI~-~} {
		set weight 100
	    }
	    default {
	    }
	}
    }

    if {[catch {
	close $pipe
    } error]} {
	debug.weight error:[debuggable $error] 2
	
	# don't confuse caller with "zeroposition" inserted by us
	regsub -- {zeroposition.*} $output {} output
	regsub -- {\noffending item: zeroposition} $error {} error

	puts [string trim $output]
	puts ""

	puts stderr $error
	exit 1
    }

    set result [list $weights $weightTotal]
    debug.weight "findMoveWeights <- $result"
    return $result
}

proc whoMoves {twin twinnings} {
    debug.whomoves "whoMoves twin:|$twin| twinnings:$twinnings"

    set options "option noboard movenumber start 1 upto 1"

    set accumulatedZeroposition ""
    foreach t $twinnings {
	lassign $t key twinning
	debug.whomoves "key:$key twinning:$twinning"
	append accumulatedZeroposition "$twinning "
    }
    foreach row {1 2 3 4 5 6 7 8} {
	foreach col {a b c d e f g h} {
	    append accumulatedZeroposition "remove $col$row "
	}
    }
    append accumulatedZeroposition "add neutral pb5"
    debug.whomoves accumulatedZeroposition:$accumulatedZeroposition

    set pipe [open "| $::params(executable)" "r+"]
    fconfigure $pipe -encoding binary -buffering line

    puts $pipe $twin
    puts $pipe $options
    puts $pipe "zeroposition $accumulatedZeroposition"
    puts $pipe "EndProblem"

    set result "white"
    debug.whomoves "assuming white"
    while {[gets $pipe line]>=0} {
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

    if {[catch {
	close $pipe
    } error]} {
	debug.whomoves error:[debuggable $error]

	# don't confuse caller with errors caused by us
	regsub -all {..: square is empty - cannot .re.move any piece.\n?} $error {} error
	regsub {both sides need a king} $error {} error
	
	if {$error!=""} {
	    puts stderr $error
	    exit 1
	}
    }

    debug.whomoves "result:$result"
    return $result
}

proc solveTwin {problemnr firstTwin movenumbers endToken twinnings skipMoves} {
    debug.twin "solveTwin problemnr:$problemnr firstTwin:|$firstTwin| movenumbers:$movenumbers endToken:$endToken twinnings:$twinnings skipMoves:$skipMoves"

    set whomoves [whoMoves $firstTwin $twinnings]
    debug.twin "whomoves:$whomoves" 2
    lassign [findMoveWeights $firstTwin $twinnings $whomoves $skipMoves] weights weightTotal
    debug.twin "weights:$weights weightTotal:$weightTotal" 2
    tryEntireTwin $problemnr $firstTwin $movenumbers $endToken $twinnings $skipMoves $weights $weightTotal

    set result [llength $weights]

    debug.twin "solveTwin <- $result"
    return $result
}

proc areMoveNumbersActivated {firstTwin zeroTwinning} {
    debug.movenumbers "areMoveNumbersActivated firstTwin:|$firstTwin| zeroTwinning:$zeroTwinning"

    set pipe [open "| $::params(executable)" "r+"]
    fconfigure $pipe -encoding binary -buffering line

    puts $pipe $firstTwin
    puts $pipe "zeroposition $zeroTwinning stipulation ~1"
    puts $pipe "EndProblem"
    flush $pipe

    set result false
    while {[gets $pipe line]>=0} {
	debug.movenumbers "line:[debuggable $line]" 2
	if {[regexp -- "^$::movenumbersRE\$" $line]} {
	    set result true
	    break
	}
    }

    if {[catch {
	close $pipe
    } error]} {
	debug.movenumbers "error:[debuggable $error]" 2
    }

    debug.movenumbers "areMoveNumbersActivated <- $result"
    return $result
}

proc readFirstTwin {chan} {
    debug.problem "readFirstTwin"

    set firstTwin "[::input::getLanguageSelector] "
    while {[gets $chan line]>=0} {
	debug.problem "line:[debuggable $line]" 2
	if {$line==[frontend::get EndProblem]
	    || $line==[frontend::get NextProblem]
	    || [string match "[frontend::get Twin] *" $line]
	    || [string match "[frontend::get Zero] *" $line]} {
	    set result [list $firstTwin $line]
	    break
	} else {
	    append firstTwin "$line\n"
	}
    }

    debug.problem "readFirstTwin <- $result"
    return $result
}

proc handleFirstTwin {chan problemnr} {
    debug.problem "handleFirstTwin problemnr:$problemnr"

    lassign [readFirstTwin $chan] firstTwin endTokenLine

    if {[regexp -- "[frontend::get Zero] (.*)" $endTokenLine - zeroTwinning]} {
	lappend twinnings [list "Zero" $zeroTwinning]
	lassign [readFirstTwin $chan ""] dummy endTokenLine
	set movenumbers [areMoveNumbersActivated $firstTwin $zeroTwinning]
	set solveResult [solveTwin $problemnr $firstTwin $movenumbers $endTokenLine $twinnings 0]
	set result [list $firstTwin $movenumbers $endTokenLine $solveResult]
    } else {
	set twinnings {}
	set movenumbers [areMoveNumbersActivated $firstTwin ""]
	set solveResult [solveTwin $problemnr $firstTwin $movenumbers $endTokenLine $twinnings 0]
	set result [list $firstTwin $movenumbers $endTokenLine $solveResult]
    }

    debug.problem "handleFirstTwin <- $result"
    return $result
}

proc handleOtherTwins {chan problemnr firstTwin movenumbers endTokenLine nrFirstMoves} {
    debug.problem "handleOtherTwins problemnr:$problemnr firstTwin:$firstTwin movenumbers:$movenumbers endTokenLine:$endTokenLine nrFirstMoves:$nrFirstMoves"

    set twinnings {}
    while {[regexp -- "[frontend::get Twin] (.*)" $endTokenLine - twinning]} {
	lappend twinnings [list "Twin" $twinning]
	debug.problem "twinnings:$twinnings"
	gets $chan endTokenLine
	debug.problem endTokenLine:$endTokenLine
	incr nrFirstMoves [solveTwin $problemnr $firstTwin $movenumbers $endTokenLine $twinnings $nrFirstMoves]
    }

    return $endTokenLine
}

proc handleProblem {chan problemnr} {
    debug.problem "handleProblem problemnr:$problemnr"

    lassign [handleFirstTwin $chan $problemnr] firstTwin movenumbers endTokenLine nrFirstMoves
    set endTokenLine [handleOtherTwins $chan $problemnr $firstTwin $movenumbers $endTokenLine $nrFirstMoves]
    debug.problem endTokenLine:$endTokenLine

    set result $endTokenLine
    debug.problem "handleProblem <- $result"
    return $result
}

proc handleInput {chan} {
    set problemnr 1

    ::input::detectLanguage $chan

    set endToken [handleProblem $chan $problemnr]

    while {$endToken==[frontend::get NextProblem]} {
	incr problemnr
	set endToken [handleProblem $chan $problemnr]
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
}

main
