#! /usr/bin/env tclsh

package require cmdline
package require debug

# define the debugging tags in deactivated state
# they can be activated using command line options
debug off board
debug off cmdline
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

proc solution {pipe endToken chunk terminator} {
    debug.solution "solution pipe:$pipe endToken:$endToken chunk:|$chunk| terminator:$terminator"

    append chunk [read $pipe]
    debug.solution "chunk:|$chunk|"

    set terminatorPos [string first $terminator $chunk]
    debug.solution "terminatorPos:$terminatorPos"
	
    set finishPos [string first "solution finished." $chunk]
    debug.solution "finishPos:$finishPos"

    if {$terminatorPos==-1} {
	if {$finishPos!=-1
	    && ($endToken==[::frontend::get "NextProblem"] || $endToken==[::frontend::get "EndProblem"])} {
	    close $pipe
	    
	    global processSync
	    incr processSync
	    debug.solution processSync:$processSync

	    if {$processSync==$::params(nrprocs)} {
		puts -nonewline $chunk
	    } else {
		puts -nonewline [string range $chunk 0 [expr {$finishPos-2}]]
	    }
	} else {
	    set lines [split $chunk "\n"]
	    foreach line [lrange $lines 0 end-1] {
		puts $line
	    }
	
	    fileevent $pipe readable [list solution $pipe $endToken [lindex $lines end] $terminator]
	}
    } else {
	close $pipe
	
	global processSync
	incr processSync
	debug.solution processSync:$processSync

	if {$processSync==$::params(nrprocs)} {
	    puts -nonewline [string range $chunk 0 [expr {$finishPos-2}]]
	} else {
	    puts -nonewline [string range $chunk 0 [expr {$terminatorPos-1}]]
	}	    
    }
}

proc onlyTwinOfProblemFirstProcess {pipe endToken chunk} {
    debug.board "onlyTwinOfProblemFirstProcess pipe:$pipe endToken:$endToken chunk:|$chunk|"

    append  chunk [read $pipe]
    debug.board "chunk:$chunk"

    set fakeTerminator {zeroposition.*?270 *\n\n}

    if {[regexp -- "(.*)${fakeTerminator}(.*)" $chunk - board remainder]} {
	puts -nonewline $board

	fileevent $pipe readable [list solution $pipe $endToken $remainder "\nb)"]

	global processSync
	set processSync 1
	debug.board processSync:$processSync
    } else {
	fileevent $pipe readable [list onlyTwinOfProblemFirstProcess $pipe $endToken $chunk]
    }
}

proc onlyTwinOfProblemOtherProcess {pipe endToken chunk} {
    debug.board "onlyTwinOfProblemOtherProcess pipe:$pipe endToken:$endToken chunk:|$chunk|"

    append chunk [read $pipe]
    debug.board "chunk:$chunk"

    set fakeTerminator {zeroposition.*?270 *\n\n}
    if {[regexp -- ".*${fakeTerminator}(.*)" $chunk - remainder]} {
	fileevent $pipe readable [list solution $pipe $endToken $remainder "\nb)"]
    } else {
	fileevent $pipe readable [list onlyTwinOfProblemOtherProcess $pipe $endToken $chunk]
    }
}

proc firstTwinOfRegularTwinningFirstProcess {pipe endToken chunk} {
    debug.board "firstTwinOfRegularTwinning pipe:$pipe endToken:$endToken chunk:|$chunk|"

    append chunk [read $pipe]
    debug.board "chunk:|$chunk|"

    set aPos [string first "a) \n\n" $chunk]
    debug.board "aPos:$aPos"

    if {$aPos==-1} {
	fileevent $pipe readable [list firstTwinOfRegularTwinningFirstProcess $pipe $endToken $chunk]
    } else {
	set board [string range $chunk 0 [expr {$aPos+4}]]
	set remainder [string range $chunk [expr {$aPos+5}] end]
	debug.board "board:|$board| remainder:|$remainder|"

	puts -nonewline $board

	fileevent $pipe readable [list solution $pipe $endToken $remainder "\nb)"]

	global processSync
	set processSync 1
	debug.board processSync:$processSync
    }
}

proc firstTwinOfRegularTwinningOtherProcess {pipe endToken chunk} {
    debug.board "firstTwinOfRegularTwinningOtherProcess pipe:$pipe endToken:$endToken chunk:|$chunk|"

    append chunk [read $pipe]
    debug.board "chunk:|$chunk|"

    set aPos [string first "a) \n\n" $chunk]
    debug.board "aPos:$aPos"

    if {$aPos==-1} {
	fileevent $pipe readable [list firstTwinOfRegularTwinningOtherProcess $pipe $endToken $chunk]
    } else {
	set remainder [string range $chunk [expr {$aPos+5}] end]
	debug.board "remainder:|$remainder|"

	fileevent $pipe readable [list solution $pipe $endToken $remainder "\nb)"]
    }
}

proc otherTwinOfRegularTwinning {pipe endToken chunk} {
    debug.board "otherTwinOfRegularTwinning pipe:$pipe endToken:$endToken chunk:|$chunk|"

    append chunk [read $pipe]
    debug.board "chunk:|$chunk|"

    if {[regexp -- {b[)] [^\n]*\n\n(.*)} $chunk - remainder]} {
	fileevent $pipe readable [list solution $pipe $endToken $remainder "\nc)"]
    } else {
	fileevent $pipe readable [list otherTwinOfRegularTwinning $pipe $endToken $chunk]
    }
}

proc tryPartialTwin {problemnr firstTwin endToken accumulatedTwinnings start upto processnr} {
    debug.processes "tryPartialTwin problemnr:$problemnr firstTwin:$firstTwin endToken:$endToken accumulatedTwinnings:$accumulatedTwinnings start:$start upto:$upto $processnr"

    set commandline $::params(executable)
    if {$::params(maxmem)!="Popeye default"} {
	append commandline " -maxmem $::params(maxmem)"
    }
    debug.processes "commandline:$commandline"

    set options "option MoveNumber Start $start Upto $upto"

    set pipe [open "| $commandline" "r+"]

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
    } else {
	debug.processes "inserting accumulated twinnings $accumulatedTwinnings"
	puts $pipe $accumulatedTwinnings
    }
    puts $pipe $endToken
    puts $pipe "EndProblem"
    flush $pipe

    if {$start==1} {
	# first process of first twin of problem
	if {[llength $accumulatedTwinnings]==0} {
	    if {[string match "[frontend::get Twin] *" $endToken]} {
		fileevent $pipe readable [list firstTwinOfRegularTwinningFirstProcess $pipe $endToken ""]
	    } else {
		fileevent $pipe readable [list onlyTwinOfProblemFirstProcess $pipe $endToken ""]
	    }
	    global processSync
	    set processSync 0
	    debug.processes "vwait processSync:$processSync"
	    vwait processSync
	    debug.processes "vwait <- processSync:$processSync"
	    unset processSync
	} else {
	    # first twin of zeroposition
	}
    } else {
	if {[llength $accumulatedTwinnings]==0} {
	    if {[string match "[frontend::get Twin] *" $endToken]} {
		fileevent $pipe readable [list firstTwinOfRegularTwinningOtherProcess $pipe $endToken ""]
	    } else {
		fileevent $pipe readable [list onlyTwinOfProblemOtherProcess $pipe $endToken ""]
	    }
	} else {
	    fileevent $pipe readable [list otherTwinOfRegularTwinning $pipe $endToken ""]
	}
    }

    debug.processes "tryPartialTwin <-"
}

proc tryEntireTwin {problemnr firstTwin endToken twinnings skipMoves weights weightTotal} {
    global processSync isTwinningWritten

    debug.processes "tryEntireTwin problemnr:$problemnr firstTwin:$firstTwin endToken:$endToken twinnings:$twinnings skipMoves:$skipMoves weights:$weights weightTotal:$weightTotal"

    set isTwinningWritten false

    if {[llength $twinnings]==0} {
	set accumulatedTwinnings ""
    } else {
	set accumulatedTwinnings "Twin"
	foreach t $twinnings {
	    lassign $t key twinning
	    debug.processes "key:$key twinning:$twinning"
	    append accumulatedTwinnings " $twinning"
	}
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
		tryPartialTwin $problemnr $firstTwin $endToken $accumulatedTwinnings $start [expr {$curr-1}] $processnr
		set start $curr
	    } else {
		tryPartialTwin $problemnr $firstTwin $endToken $accumulatedTwinnings $start $curr $processnr
		set start [expr {$curr+1}]
	    }
	    incr processnr
	    incr weightTarget $avgWeightPerProcess
	}
    }
    tryPartialTwin $problemnr $firstTwin $endToken $accumulatedTwinnings $start [expr {$skipMoves+[llength $weights]}] $processnr

    set processSync 0
    while {$processSync<$::params(nrprocs)} {
	debug.processes "vwait processSync:$processSync"
	vwait processSync
	debug.processes "vwait <- processSync:$processSync"
    }
    unset processSync

    debug.processes "tryEntireTwin <-"
}

proc findMoveWeights {firstTwin twinnings whomoves} {
    debug.weight "findMoveWeights firstTwin:$firstTwin twinnings:$twinnings whomoves:$whomoves"

    set options "option Noboard MoveNumber"
    if {$whomoves=="black"} {
	append options " HalfDuplex"
    }
    debug.weight "options:$options"

    set accumulatedZeroposition ""
    foreach t $twinnings {
	lassign $t key twinning
	debug.weight "key:$key twinning:$twinning"
	append accumulatedZeroposition "$twinning "
    }
    debug.weight accumulatedZeroposition:$accumulatedZeroposition

    set pipe [open "| $::params(executable)" "r+"]
    fconfigure $pipe -encoding binary -buffering line

    puts $pipe $firstTwin
    puts $pipe $options
    puts $pipe "zeroposition $accumulatedZeroposition stipulation ~1"
    puts $pipe "EndProblem"

    set output ""

    set weightTotal 0
    while {[gets $pipe line]>=0} {
	append output "$line\n"
	debug.weight "line:$line" 2
	# TODO lower weight for capturing moves?
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
	    default {
	    }
	}
    }

    if {[catch {
	close $pipe
    } error]} {
	debug.weight output:$output 2
	debug.weight error:$error 2
	
	# don't confuse caller with "zeroposition" inserted by us
	regsub -- {zeroposition.*} $output {} output
	regsub -- {\noffending item: zeroposition} $error {} error

	puts [string trim $output]
	puts ""

	puts stderr $error
	exit 1
    }

    set result [list $weights $weightTotal]
    debug.weight weights:$result
    return $result
}

proc whoMoves {twin twinnings} {
    debug.whomoves "whoMoves twin:$twin twinnings:$twinnings"

    set options "option noboard movenumber maxtime 1"

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
    while {[gets $pipe line]>=0} {
	debug.whomoves "line:$line" 2
	if {[string first "b5-b4" $line]>=0} {
	    set result "black"
	}
    }

    if {[catch {
	close $pipe
    } error]} {
	debug.whomoves error:$error

	# don't confuse caller with errors caused by us
	regsub -all {..: square is empty - cannot .re.move any piece.\n} $error {} error
	regsub {both sides need a king} $error {} error
	
	if {$error!=""} {
	    puts stderr $error
	    exit 1
	}
    }

    debug.whomoves "result:$result"
    return $result
}

proc solveTwin {problemnr firstTwin endToken twinnings skipMoves} {
    debug.twin "solveTwin problemnr:$problemnr firstTwin:$firstTwin endToken:$endToken twinnings:$twinnings skipMoves:$skipMoves"

    set whomoves [whoMoves $firstTwin $twinnings]
    debug.twin "whomoves:$whomoves"
    lassign [findMoveWeights $firstTwin $twinnings $whomoves] weights weightTotal
    debug.twin "weights:$weights weightTotal:$weightTotal"
    tryEntireTwin $problemnr $firstTwin $endToken $twinnings $skipMoves $weights $weightTotal

    set result [llength $weights]

    debug.twin "solveTwin <- $result"
    return $result
}

proc handleFirstTwin {chan problemnr {languageSelectorWord ""}} {
    debug.problem "handleFirstTwin problemnr:$problemnr languageSelectorWord:$languageSelectorWord"

    set firstTwin "$languageSelectorWord "
    while {[gets $chan line]>=0} {
	if {$line==[frontend::get EndProblem]
	    || $line==[frontend::get NextProblem]
	    || [string match "[frontend::get Twin] *" $line]} {
	    set twinnings {}
	    return [list $firstTwin $line [solveTwin $problemnr $firstTwin $line $twinnings 0]]
	} elseif {[string match "[frontend::get Zero] *" $line]} {
	    return [list $firstTwin $line]
	} else {
	    append firstTwin "$line\n"
	}
    }
}

proc handleOtherTwins {chan problemnr firstTwin endTokenLine nrFirstMoves} {
    debug.problem "handleOtherTwins problemnr:$problemnr firstTwin:$firstTwin endTokenLine:$endTokenLine nrFirstMoves:$nrFirstMoves"

    set twinnings {}
    while {[regexp "[frontend::get Twin] (.*)" $endTokenLine - twinning]} {
	lappend twinnings [list "Twin" $twinning]
	debug.problem "twinnings:$twinnings"
	gets $chan endTokenLine
	debug.problem endTokenLine:$endTokenLine
	solveTwin $problemnr $firstTwin $endTokenLine $twinnings $nrFirstMoves
    }

    return $endTokenLine
}

proc handleFirstProblem {chan problemnr} {
    debug.problem "handleFirstProblem problemnr:$problemnr"

    lassign [handleFirstTwin $chan $problemnr] firstTwin endTokenLine nrFirstMoves
    set endTokenLine [handleOtherTwins $chan $problemnr $firstTwin $endTokenLine $nrFirstMoves]
    debug.problem endTokenLine:$endTokenLine

    set languageSelectorWord [lindex [split [string trim $firstTwin]] 0]

    set result [list $languageSelectorWord $endTokenLine]
    debug.problem "handleFirstProblem <- $result"
    return $result
}

proc handleNextProblem {chan problemnr languageSelectorWord} {
    debug.problem "handleNextProblem problemnr:$problemnr languageSelectorWord:$languageSelectorWord"

    lassign [handleFirstTwin $chan $problemnr $languageSelectorWord] firstTwin endTokenLine nrFirstMoves
    set endTokenLine [handleOtherTwins $chan $problemnr $firstTwin $endTokenLine $nrFirstMoves]
    debug.problem endTokenLine:$endTokenLine

    set result $endTokenLine
    debug.problem "handleNextProblem <- $result"
    return $result
}

proc handleInput {chan} {
    set problemnr 1

    lassign [handleFirstProblem $chan $problemnr] languageSelectorWord endToken

    while {$endToken==[frontend::get NextProblem]} {
	incr problemnr
	set endToken [handleNextProblem $chan $problemnr $languageSelectorWord]
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
