#! /usr/bin/env tclsh

package require cmdline
package require control
package require debug
package require msgcat

# debug.something has the nasty habit of subst-in its argument
# to protect it from subst, do something like
# debug.something "argument:[debuggable $argument]"
proc debuggable {string} {
    return [regsub -all {\[} $string {[return "\["]}]
}

control::control assert enabled 0

# define the debugging tags in deactivated state
# they can be activated using command line options
debug off cmdline
debug off input
debug off output
debug off language
debug off movenumbers
debug off popeye
debug off problem
debug off sync
debug off tester
debug off twin
debug off grouping
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
    namespace eval fr {
	namespace eval input {
	    variable BeginProblem "DebugProblem"
	    variable NextProblem "Asuivre"
	    variable EndProblem "FinProblem"
	    variable ZeroPosition "zeroposition"
	    variable Twin "Jumeau"
	    variable Title "Titre"
	    variable Author "Auteur"
	    variable Origin "Source"
	    variable Award "prix"
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
	    variable Rotate "rotation"
	    variable Pieces "Pieces"
	}

	namespace eval output {
	    variable Time "Temps"
	    variable SolutionFinished "solution terminee"
	    variable PartialSolution "solution partielle"
	    variable SquareIsEmptyRE "la case est vide - on ne peut pas oter ou deplacer une piece."
	    variable BothSidesNeedAKingRE "il faut un roi de chaque couleur"
	}

	namespace eval grouping {
	    variable auto "Choisir une des méthodes suivantes de grouper les coups en fonction du problème à résoudre."
	    variable byweight "Essayer de grouper les premier coups selon une estimation la durée de solution."
	    variable movebymove "Chaque premier coup forme son propre groupe."
	}

	namespace eval cmdline {
	    variable options "options"
	    variable inputfile "Fichier input"
	    variable stopProcessing "Fin des options"
	    variable printThis "Ecrire ce message"
	    variable value "valeur"
	    variable popeyePath "Chemin d'accès du programme Popeye exécutable"
	    variable numberProcesses "Nombre de processus Popeye à démarrer"
	    variable processMemory "Mémoire maximale par processus"
	    variable popeyeDefault "valeur standard de Popeye"
	}
    }

    namespace eval de {
	namespace eval input {
	    variable BeginProblem "AnfangProblem"
	    variable NextProblem "WeiteresProblem"
	    variable EndProblem "EndeProblem"
	    variable ZeroPosition "NullStellung"
	    variable Twin "Zwilling"
	    variable Title "Titel"
	    variable Author "Autor"
	    variable Origin "Quelle"
	    variable Award "Auszeichnung"
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
	    variable Pieces "Steine"
	}

	namespace eval output {
	    variable Time "Zeit"
	    variable SolutionFinished "Loesung beendet"
	    variable PartialSolution "Partielle Loesung"
	    variable SquareIsEmptyRE "Feld leer - kann keine Figur entfernen/versetzen."
	    variable BothSidesNeedAKingRE "Es fehlt ein weisser oder schwarzer Koenig\n"
	}

	namespace eval grouping {
	    variable auto "Ein der folgenden Methoden in Abhängigkeit des zu lösenden Problems wählen."
	    variable byweight "Versuche die Erstzüge gemäss der geschätzten Lösungszeit zu gruppieren."
	    variable movebymove "Jeder Erstzug bildet eine eigene Gruppe."
	}

	namespace eval cmdline {
	    variable options "Optionen"
	    variable inputfile "Eingabedatei"
	    variable stopProcessing "Ende der Optionen"
	    variable printThis "Diese Meldung ausgeben"
	    variable value "Wert"
	    variable popeyePath "Pfad zur ausführbaren Popeye-Datei"
	    variable numberProcesses "Anzahl Popeye-Prozesse, die parallel gestartet werden sollen"
	    variable processMemory "Maximaler Arbeitsspeicher pro Prozess"
	    variable popeyeDefault "Popeye-Vorgabe"
	}
    }

    namespace eval en {
	namespace eval input {
	    variable BeginProblem "beginproblem"
	    variable NextProblem "nextproblem"
	    variable EndProblem "endproblem"
	    variable ZeroPosition "zeroposition"
	    variable Twin "twin"
	    variable Title "title"
	    variable Author "author"
	    variable Origin "origin"
	    variable Award "award"
	    variable Remark "remark"
	    variable Protocol "protocol"
	    variable LaTeX "LaTeX"
	    variable Stipulation "stipulation"
	    variable Option "option"
	    variable MoveNumber "MoveNumber"
	    variable StartMoveNumber "StartMoveNumber"
	    variable UpToMoveNumber "UpToMoveNumber"
	    variable NoBoard "NoBoard"
	    variable MaxSolutions "MaxSolutions"
	    variable HalfDuplex "HalfDuplex"
	    variable Rotate "rotate"
	    variable Pieces "Pieces"
	}

	namespace eval output {
	    variable Time "Time"
	    variable SolutionFinished "solution finished"
	    variable PartialSolution "Partial solution"
	    variable SquareIsEmptyRE "square is empty - cannot .re.move any piece."
	    variable BothSidesNeedAKingRE "both sides need a king"
	}

	namespace eval grouping {
	    variable auto "Select one of the following methods depending on the problem being solved."
	    variable byweight "Try to group the first moves based on an estimat of the solving time."
	    variable movebymove "Each first move is its own group."
	}

	namespace eval cmdline {
	    variable options "options"
	    variable inputfile "input file"
	    variable stopProcessing "Forcibly stop option processing"
	    variable printThis "Print this message"
	    variable value "value"
	    variable popeyePath "path to Popeye executable"
	    variable numberProcesses "number of Popeye processes to spawn"
	    variable processMemory "maximum memory for each process"
	    variable popeyeDefault "Popeye default"
	}
    }

    set minElementLength 3
}

proc ::language::tokenIsElement {token elementId} {
    variable minElementLength

    debug.language "tokenIsElement token:$token elementId:$elementId"

    set tokenLength [string length $token]

    if {$tokenLength>=$minElementLength} {
	debug.language "language:[::msgcat::mclocale]" 2

	set elementString [msgcat::mc $elementId]
	debug.language "elementString:$elementString" 2

	set elementStringFragment [string range $elementString 0 [expr {$tokenLength-1}]]
	debug.language "elementStringFragment:$elementStringFragment" 2

	set result [expr {[string compare -nocase $elementStringFragment $token]==0}]
    } else {
	set result false
    }

    debug.language "tokenIsElement <- $result"
    return $result
}

proc ::language::init {} {
    debug.language "init"

    foreach language [namespace children ::language] {
	set shortcut [namespace tail $language]
	foreach category [namespace children $language] {
	    foreach var [info vars ${category}::*] {
		set category [lindex [split [namespace qualifiers $var] :] end]
		set symbol [namespace tail $var]
		namespace eval :: [list ::msgcat::mcset $shortcut ${category}::${symbol} [set $var] ]
	    }
	}
    }

    set languages [::getAllNames ::language]

    set prefs [::msgcat::mcpreferences]
    debug.language "prefs:$prefs"

    debug.language "setting default fr"
    ::msgcat::mclocale "fr"

    foreach pref $prefs {
	if {[lsearch -exact $languages $pref]!=-1} {
	    debug.language "overriding default with $pref"
	    ::msgcat::mclocale $pref
	    break
	}
    }

    debug.language "init <-"
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
	Award
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
	if {[::language::tokenIsElement $token input::$elmt]} {
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

    set languages [::getAllNames ::language]
    debug.input "languages:$languages" 2

    set token [nextToken $chan]
    set tokenLength [string length $token]
    debug.input "tokenLength:$tokenLength" 2

    if {$tokenLength<$minTokenLength} {
	reportNoLanguageDetected $token
    } else {
	set detected false
	set saveLocale [::msgcat::mclocale]
	foreach language $languages {
	    debug.input "trying $language" 2
	    ::msgcat::mclocale $language
	    if {[::language::tokenIsElement $token input::BeginProblem]} {
		set detected true
		break
	    }
	}

	if {!$detected} {
	    ::msgcat::mclocale $saveLocale
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
	    if {[::language::tokenIsElement $token input::$id]} {
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

    closeProtocol
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
    _puts $string
}

proc ::output::movenumberLine {line} {
    variable areMovenumbersSuppressed

    if {!$areMovenumbersSuppressed} {
	_puts $line
    }
}

proc ::output::solutionTerminator {terminator} {
    variable isSolutionTerminatorSuppressed

    debug.output "solutionTerminator terminator:$terminator"

    debug.output "isSolutionTerminatorSuppressed:$isSolutionTerminatorSuppressed" 2

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

    debug.popeye "setMaxmem [debuggable $setting]"

    set maxmemOption "-maxmem $setting"
}

proc ::popeye::spawn {firstTwin options} {
    variable executablePath
    variable maxmemOption

    debug.popeye "spawn firstTwin:|$firstTwin| options:|$options|"

    debug.popeye "executablePath:$executablePath" 2
    debug.popeye "maxmemOption:[debuggable $maxmemOption]" 2

    set pipe [open "| $executablePath $maxmemOption" "r+"]
    debug.popeye "pipe:$pipe pid:[pid $pipe]" 2
    debug.popeye "caller:[debuggable [info level -1]]" 2

    fconfigure $pipe -encoding binary -buffering line

    gets $pipe greetingLine

    set result [list $pipe $greetingLine]

    puts $pipe [::msgcat::mc input::BeginProblem]
    puts $pipe $firstTwin
    puts $pipe "[::msgcat::mc input::Option] $options"

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
    debug.popeye "input::ZeroPosition pipe:$pipe twinning:|$twinning|"

    puts $pipe "[::msgcat::mc input::ZeroPosition] $twinning"
}

proc ::popeye::input::Twin {pipe twinning} {
    debug.popeye "input::Twin pipe:$pipe twinning:|$twinning|"

    puts $pipe "[::msgcat::mc input::Twin] $twinning"
}

proc ::popeye::input::Pieces {pipe pieces} {
    debug.popeye "input::Pieces pieces:|$pieces|"

    puts $pipe "[::msgcat::mc input::Pieces] $pieces"
}

proc ::popeye::input::EndProblem {pipe} {
    debug.popeye "input::EndProblem pipe:$pipe"

    puts $pipe "[::msgcat::mc input::EndProblem]"
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

    set callback [linsert $arguments 0 [uplevel namespace which -command $listener] $pipe]
    fileevent $pipe readable $callback
}

# this is a hack
# ::cmdline::usage from Tcllib has "-name value" format hard-wired - I prefer "--name=value"
# Also, I want the usage string to be language-dependant
proc ::cmdline::usage {optlist {usage {options:}}} {
    lappend optlist [list "" "[::msgcat::mc cmdline::stopProcessing]" ]
    lappend optlist [list help "[::msgcat::mc cmdline::printThis]" ]
    lappend optlist [list ? "[::msgcat::mc cmdline::printThis]" ]

    set str "[getArgv0] [subst $usage]\n"
    set longest 20
    set lines {}
    foreach opt $optlist {
        set name "--[lindex $opt 0]"
        if {[regsub -- {\.secret$} $name {} name] == 1} {
            # Hidden option
            continue
        }
        if {[regsub -- {\.arg$} $name {} name] == 1} {
            append name "=[::msgcat::mc cmdline::value]"
            set desc "[lindex $opt 2] <[lindex $opt 1]>"
        } else {
            set desc "[lindex $opt 1]"
        }
        set n [string length $name]
        if {$n > $longest} {
	    set longest $n
	}
        # max not available before 8.5 - set longest [expr {max($longest, )}]
        lappend lines $name $desc
    }
    foreach {name desc} $lines {
        append str "[string trimright [format " %-*s %s" $longest $name $desc]]\n"
    }

    return $str
}

proc getAllNames {ns} {
    set result {}
    foreach c [namespace children $ns] {
	lappend result [namespace tail $c]
    }
    return $result
}

proc parseCommandLine {} {
    set options [subst {
	{ executable.arg "[defaultPopeyeExecutable]"             "[::msgcat::mc cmdline::popeyePath]" }
	{ nrprocs.arg    "[defaultNumberOfProcesses]"            "[::msgcat::mc cmdline::numberProcesses]" }
	{ maxmem.arg     "[::msgcat::mc cmdline::popeyeDefault]" "[::msgcat::mc cmdline::processMemory]" }
	{ grouping.arg   "auto"                                  "([join [getAllNames ::grouping] ,])" }
	{ help-grouping }
    }]
    if {[string match "*.tcl" $::argv0]} {
	lappend options { assert.arg false "enable asserts" }
	foreach name [debug names] {
	    lappend options [subst { debug-$name.arg "0" "debug level for tag $name" }]
	}
    }

    set usage {: [::cmdline::getArgv0] \[[::msgcat::mc cmdline::options]\] \[[::msgcat::mc cmdline::inputfile]\]\n[::msgcat::mc cmdline::options]:}

    try {
	array set ::params [::cmdline::getoptions ::argv $options $usage]
    } trap {CMDLINE USAGE} {msg o} {
	puts stderr $msg
	exit 1
    }

    if {$::params(help-grouping)} {
	puts stderr grouping:
	set maxLengthName 0
	foreach grouping [namespace children ::grouping] {
	    set lengthName [string length [namespace tail $grouping]]
	    if {$lengthName>$maxLengthName} {
		set maxLengthName $lengthName
	    }
	}
	incr maxLengthName
	foreach grouping [namespace children ::grouping] {
	    set name [namespace tail $grouping]
	    puts stderr "- [format %-*s $maxLengthName $name:] [::msgcat::mc grouping::$name]"
	}
	exit 1
    }

    if {$::params(executable)==""} {
	puts stderr [::cmdline::usage $options $usage]
	exit 1
    } else {
	::popeye::setExecutable $::params(executable)
    }
    if {$::params(maxmem)!=[::msgcat::mc cmdline::popeyeDefault]} {
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

    set latestNotification {}
}

proc ::sync::Fini {} {
    variable latestNotification

    unset latestNotification
}

proc ::sync::Wait {callback args} {
    variable latestNotification

    debug.sync "Wait callback:$callback args:[debuggable $args]"

    set callback [uplevel namespace which -command $callback]

    while {true} {
	debug.sync "vwait latestNotification:$latestNotification" 2
	vwait [namespace which -variable latestNotification]
	foreach l $latestNotification {
	    set result [catch {
		set args [$callback {*}$l {*}$args]
	    } error options]
	}

	set latestNotification {}

	switch -exact $result {
	    0 {} # TCL_OK
	    2 {} # TCL_RETURN
	    4 {} # TCL_CONTINUE
	    1 {
		# TCL_ERROR
		# error will be re-raised by return below
		break
	    }
	    3 {
		# TCL_BREAK
		break
	    }
	}
    }

    debug.sync "Wait <-"
    if {$result==1} {
	return -options $options $result
    } else {
	return $args
    }
}

proc ::sync::Notify {sender notification} {
    variable latestNotification

    debug.sync "Notify sender:$sender notification:$notification"
    lappend latestNotification [list $sender $notification]
}

namespace eval tester {
}

namespace eval tester::async {
}

proc ::tester::async::flushSolution {pipe solution} {
    debug.tester "flushSolution pipe:$pipe solution:[debuggable $solution]"

    set parenOpenRE {[\(]}
    set parenCloseRE {[\)]}
    set movenumberRE { *[[:digit:]]+}
    set moveRE {[^\n]+}
    set timeLabelRE [::msgcat::mc output::Time]
    set timeRE {[[:digit:]:.]+}
    set timeUnitRE {(?:(?:h:)?m:)?s}
    set movenumberLineRE "\n$movenumberRE +$parenOpenRE$moveRE +$timeLabelRE = $timeRE $timeUnitRE$parenCloseRE"

    set moveNumberLineRanges [regexp -all -inline -indices -- $movenumberLineRE $solution]

    set prevEnd 0
    foreach moveNumberLineRange $moveNumberLineRanges {
	debug.tester "moveNumberLineRange:$moveNumberLineRange" 2
        ::sync::Notify $pipe "move"
	lassign $moveNumberLineRange start end
	::output::solution [string range $solution $prevEnd [expr {$start-1}]]
	::output::movenumberLine [string range $solution $start $end]
	set prevEnd [expr {$end+1}]
    }

    ::output::solution [string range $solution $prevEnd "end"]

    debug.tester "flushSolution <-"
}

proc ::tester::async::flushBelowBoard {pipe chunk solutionTerminatorRE} {
    debug.tester "flushBelowBoard pipe:$pipe chunk:|[debuggable $chunk]| solutionTerminatorRE:[::debuggable $solutionTerminatorRE]"

    if {[regexp -- "^(.*)($solutionTerminatorRE)(.*)$" $chunk - solution terminator remainder]} {
	debug.tester "solution:|[debuggable $solution]|"
	debug.tester "terminator:|[debuggable $terminator]|"

	flushSolution $pipe $solution
        ::sync::Notify $pipe "solution"
	::output::solutionTerminator $terminator

	set result true
    } else {
	debug.tester "terminator not found" 2

	flushSolution $pipe $chunk
	set result false
    }

    flush stdout

    debug.tester "flushBelowBoard <- $result"
    return $result
}

proc ::tester::async::belowBoard {pipe solutionTerminatorRE} {
    debug.tester "belowBoard pipe:$pipe"

    flushBelowBoard $pipe [read $pipe] $solutionTerminatorRE
}

proc ::tester::async::board {pipe boardTerminatorRE solutionTerminatorRE {chunk ""}} {
    debug.tester "board pipe:$pipe chunk:|[debuggable $chunk]| boardTerminatorRE:[debuggable $boardTerminatorRE]"

    append chunk [read $pipe]
    debug.tester "chunk:|[debuggable $chunk]|"

    if {[regexp -- "^(.*)($boardTerminatorRE)(.*)$" $chunk - board terminator remainder]} {
	debug.tester "terminator:|[debuggable $terminator]|"
	debug.tester "remainder:|[debuggable $remainder]|"
	::output::board $board $terminator
	::sync::Notify $pipe "board"
	if {![flushBelowBoard $pipe $remainder $solutionTerminatorRE]} {
	    # While writing the solution, Popeye writes EOLs before the next line
	    ::popeye::output::doAsync $pipe belowBoard [list "\\n$solutionTerminatorRE"]
	}
    } else {
	debug.tester "terminator not found"
	::popeye::output::doAsync $pipe board [list $boardTerminatorRE $solutionTerminatorRE $chunk]
    }
}

proc ::tester::moveRange {firstTwin endElmt twinnings boardTerminatorRE solutionTerminatorRE moveRange} {
    debug.tester "moveRange firstTwin:|$firstTwin| endElmt:$endElmt twinnings:$twinnings boardTerminatorRE:[debuggable $boardTerminatorRE] solutionTerminatorRE:[debuggable $solutionTerminatorRE] moveRange:$moveRange"

    lassign $moveRange start upto

    set options "[::msgcat::mc input::MoveNumber] [::msgcat::mc input::UpToMoveNumber] $upto"
    if {$start>1} {
	append options " [::msgcat::mc input::StartMoveNumber] $start"
    }

    lassign [::popeye::spawn $firstTwin $options] result greetingLine
    debug.tester "result:$result" 2

    ::output::greetingLine $greetingLine

    debug.tester "inserting accumulated twinnings" 2
    foreach t $twinnings {
	lassign $t key twinning
	::popeye::input::$key $result $twinning
    }

    ::popeye::output::doAsync $result async::board [list $boardTerminatorRE $solutionTerminatorRE]

    debug.tester "moveRange <- $result"
    return $result
}

proc ::tester::moveRangesProgress {pipe notification endElmt nrRunningProcesses nrBoardsRead nrMovesPlayed pipes} {
    debug.tester "moveRangesProgress pipe:$pipe notification:$notification endElmt:$endElmt nrRunningProcesses:$nrRunningProcesses nrBoardsRead:$nrBoardsRead nrMovesPlayed:$nrMovesPlayed pipes:$pipes"

    switch -exact $notification {
	board {
	    incr nrBoardsRead
	}
	move {
	    incr nrMovesPlayed
	}
	solution  {
	    ::popeye::terminate $pipe
	    if {[llength $pipes]==0} {
		incr nrRunningProcesses -1
	    } else {
		debug.tester "moveRangesProgress - [llength $pipes] left - spawning next process" 2
		set pipe [lindex $pipes 0]
		set pipes [lrange $pipes 1 end]
		::popeye::input::EndProblem $pipe
	    }
	}
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
	debug.tester "moveRangesProgress <- break"
	return -code break
    } else {
	set result [list $endElmt $nrRunningProcesses $nrBoardsRead $nrMovesPlayed $pipes]
	debug.tester "moveRangesProgress <- [debuggable $result]"
	return $result
    }
}

proc ::tester::moveRanges {firstTwin twinnings endElmt moveRanges} {
    debug.tester "moveRanges firstTwin:|$firstTwin| twinnings:$twinnings endElmt:$endElmt moveRanges:$moveRanges"

    ::sync::Init

    set ::output::isSolutionTerminatorSuppressed true

    set twinningRE {\n\n..?[\)] [^\n]*\n}

    if {$endElmt=="Twin"} {
	debug.tester "inserting fake twinning as solution terminator" 2
	set fakeTwinning "[::msgcat::mc input::Rotate] 180  [::msgcat::mc input::Rotate] 180"
	lappend twinnings [list Twin $fakeTwinning]
	set solutionTerminatorRE "\\n..?\[\)] $fakeTwinning *\\n"
    } else {
	set solutionFinishedRE "(?:\\n[::msgcat::mc output::SolutionFinished]|[::msgcat::mc output::PartialSolution])\[^\\n]+\\n+"
	set solutionTerminatorRE $solutionFinishedRE
    }

    if {[llength $twinnings]==0 && $endElmt!="Twin"} {
	debug.tester "no twin and no zeroposition - inserting fake zero position as board terminator" 2
	set twinnings [linsert $twinnings 0 [list "ZeroPosition" "[::msgcat::mc input::Rotate] 90 [::msgcat::mc input::Rotate] 270"]]
	set fakeZeroPositionRE "\\n[::msgcat::mc input::ZeroPosition].*?270 *\\n"
	set boardTerminatorRE $fakeZeroPositionRE
	set ::output::isBoardTerminatorSuppressed true
    } else {
	if {[info exists fakeTwinning]} {
	    set boardTerminatorRE "\\n\\n..?\[\)] (?!$fakeTwinning)\[^\\n]*\\n"
	} else {
	    set boardTerminatorRE "\\n\\n..?\[\)] \[^\\n]*\\n"
	}
    }

    set pipes {}
    foreach moveRange $moveRanges {
	lappend pipes [moveRange $firstTwin $endElmt $twinnings $boardTerminatorRE $solutionTerminatorRE $moveRange]
    }
    debug.tester "pipes:$pipes" 2

    set nrRunningProcesses 0
    foreach pipe $pipes {
	::popeye::input::EndProblem $pipe
	incr nrRunningProcesses
	if {$nrRunningProcesses==$::params(nrprocs)} {
	    break
	}
    }

    if {$endElmt!="Twin" && $nrRunningProcesses==1} {
	set ::output::isSolutionTerminatorSuppressed false
    }

    set nrBoardsRead 0
    set nrMovesPlayed 0
    set pipes [lrange $pipes $nrRunningProcesses end]
    lassign [::sync::Wait moveRangesProgress $endElmt $nrRunningProcesses $nrBoardsRead $nrMovesPlayed $pipes] endElmt nrProcesses nrBoardsRead nrMovesPlayed

    ::sync::Fini

    debug.tester "moveRanges <- $nrMovesPlayed"
    return $nrMovesPlayed
}

namespace eval grouping {
}

namespace eval grouping::byweight {
}

proc grouping::byweight::makeGroups {weights skipMoves} {
    debug.grouping "byweight::makeGroups weights:$weights skipMoves:$skipMoves"

    set weightTotal 0
    foreach weight $weights {
	incr weightTotal $weight
    }
    debug.grouping "weightTotal:$weightTotal" 2

    set nrGroups $::params(nrprocs)
    if {[llength $weights]<$nrGroups} {
	set nrGroups [llength $weights]
    }
    debug.grouping "nrGroups:$nrGroups" 2

    set avgWeightPerProcess [expr {($weightTotal+$::params(nrprocs)-1)/$nrGroups}]
    debug.grouping "weightTotal:$weightTotal avgWeightPerProcess:$avgWeightPerProcess" 2

    set start [expr {$skipMoves+1}]
    set curr $skipMoves
    set weightTarget $avgWeightPerProcess
    set weightAccumulated 0

    set result {}

    foreach weight $weights {
	incr curr
	incr weightAccumulated $weight
	debug.grouping "start:$start curr:$curr weight:$weight weightTarget:$weightTarget weightAccumulated:$weightAccumulated" 2
	if {$weightAccumulated>$weightTarget} {
	    set weightExcess [expr {$weightAccumulated-$weightTarget}]
	    debug.tester "weightExcess:$weightExcess"
	    if {$weightExcess>$weight/2 && $start>$curr} {
		lappend result [list $start [expr {$curr-1}]]
		set start $curr
	    } else {
		lappend result [list $start $curr]
		set start [expr {$curr+1}]
	    }
	    incr weightTarget $avgWeightPerProcess
	}
    }
    set end [expr {$skipMoves+[llength $weights]}]
    if {$start<=$end} {
	lappend result [list $start $end]
    }

    debug.grouping "byweight::makeGroups <- $result"
    return $result
}

proc grouping::byweight::findWeights {firstTwin twinnings whomoves skipMoves} {
    debug.grouping "byweight::findWeights firstTwin:|$firstTwin| twinnings:$twinnings whomoves:$whomoves skipMoves:$skipMoves"

    set options "[::msgcat::mc input::NoBoard] [::msgcat::mc input::MoveNumber] [::msgcat::mc input::StartMoveNumber] [expr {$skipMoves+1}]"
    if {$whomoves=="black"} {
	append options " [::msgcat::mc input::HalfDuplex]"
    }
    debug.grouping "options:$options"

    lassign [::popeye::spawn $firstTwin $options] pipe greetingLine

    if {[llength $twinnings]==0} {
	::popeye::input::ZeroPosition $pipe "[::msgcat::mc input::Stipulation] ~1"
    } else {
	foreach t $twinnings {
	    lassign $t key twinning
	    ::popeye::input::$key $pipe "$twinning [::msgcat::mc input::Stipulation] ~1"
	}
    }
    ::popeye::input::EndProblem $pipe

    set result {}
    while {[::popeye::output::getLine $pipe line]>=0} {
	debug.grouping "line:|[debuggable $line]|" 2
	append output "$line\n"
	switch -glob $line {
	    "*[::msgcat::mc output::Time] = *" {
		debug.grouping "next move" 2
		# this matches both
		#  21  (Ke6-e7    Time = 0.016 s)
		# solution finished. Time = 0.016 s
		# so all moves are taken care of
		if {[info exists move]} {
		    lappend result $weight
		    debug.grouping "[llength $result] move:[debuggable $move] weight:$weight" 2
		}
		set weight 0
		regexp -- {[(]([^ ]+).*[)]} $line - move
	    }
	    {*+ !} {
		debug.grouping "checking move" 2
		set weight 2
	    }
	    {*!} {
		debug.grouping "non-checking move" 2
		set weight 20
	    }
	    {*TI~-~*} {
		debug.grouping "random move by TI" 2
		set weight 100
	    }
	    default {
		debug.grouping "other line" 2
	    }
	}
    }

    ::popeye::terminate $pipe

    debug.grouping "byweight::findWeights <- $result"
    return $result
}

proc ::grouping::byweight::makeRanges {firstTwin twinnings whomoves skipMoves} {
    debug.grouping "byweight::makeRanges firstTwin:|$firstTwin| twinnings:$twinnings whomoves:$whomoves skipMoves:$skipMoves"

    set weights [::grouping::byweight::findWeights $firstTwin $twinnings $whomoves $skipMoves]
    debug.grouping "weights:$weights" 2

    if {[llength $weights]==0} {
	set result {}
    } else {
	set result [::grouping::byweight::makeGroups $weights $skipMoves]
    }

    debug.grouping "byweight::makeRanges <- $result"
    return $result
}

namespace eval ::grouping::movebymove {
}

proc ::grouping::movebymove::makeRanges {firstTwin twinnings whomoves skipMoves} {
    debug.grouping "movebymove::makeRanges firstTwin:|$firstTwin| twinnings:$twinnings whomoves:$whomoves skipMoves:$skipMoves"

    set options "[::msgcat::mc input::NoBoard] [::msgcat::mc input::MoveNumber] [::msgcat::mc input::StartMoveNumber] [expr {$skipMoves+1}]"
    if {$whomoves=="black"} {
	append options " [::msgcat::mc input::HalfDuplex]"
    }
    debug.grouping "options:$options"

    lassign [::popeye::spawn $firstTwin $options] pipe greetingLine

    if {[llength $twinnings]==0} {
	::popeye::input::ZeroPosition $pipe "[::msgcat::mc input::Stipulation] h#0.5"
    } else {
	foreach t $twinnings {
	    lassign $t key twinning
	    ::popeye::input::$key $pipe "$twinning [::msgcat::mc input::Stipulation] h#0.5"
	}
    }
    ::popeye::input::EndProblem $pipe

    set parenOpenRE {[\(]}
    set parenCloseRE {[\)]}
    set movenumberRE { *[[:digit:]]+}
    set moveRE {[^\n]+}
    set timeLabelRE [::msgcat::mc output::Time]
    set timeRE {[[:digit:]:.]+}
    set timeUnitRE {(?:(?:h:)?m:)?s}
    set movenumberLineRE "$movenumberRE +$parenOpenRE$moveRE +$timeLabelRE = $timeRE $timeUnitRE$parenCloseRE"

    set result {}
    set nrMoves 0
    while {[::popeye::output::getLine $pipe line]>=0} {
	debug.grouping "line:[debuggable $line]" 2
	if {[regexp -- $movenumberLineRE $line]} {
	    incr nrMoves
	    lappend result [list $nrMoves $nrMoves]
	}
    }

    ::popeye::terminate $pipe

    debug.grouping "nrMoves:$nrMoves" 2

    debug.grouping "movebymove::makeRanges <- $result"
    return $result
}

namespace eval ::grouping::auto {
}

proc ::grouping::auto::makeRanges {firstTwin twinnings whomoves skipMoves} {
    debug.grouping "auto::makeRanges firstTwin:|$firstTwin| twinnings:$twinnings whomoves:$whomoves skipMoves:$skipMoves"

    set byweight [::grouping::byweight::makeRanges $firstTwin $twinnings $whomoves $skipMoves]
    set movebymove [::grouping::movebymove::makeRanges $firstTwin $twinnings $whomoves $skipMoves]

    # we currently prefer byweight in general, but this method may miss some moves...
    if {[llength $byweight]==0} {
	set result $movebymove
    } else {
	lassign [lindex $byweight end] first last
	if {$last>=[llength $movebymove]} {
	    set result $byweight
	} else {
	    set result $movebymove
	}
    }

    debug.grouping "auto::makeRanges <- $result"
    return $result
}

proc whoMoves {twin twinnings} {
    debug.whomoves "whoMoves twin:|$twin| twinnings:$twinnings"

    set movenumbersRE { *[[:digit:]]+ +[\(][^\)]+[::msgcat::mc output::Time] =}

    set options "[::msgcat::mc input::NoBoard] [::msgcat::mc input::MoveNumber] [::msgcat::mc input::StartMoveNumber] 1"

    lassign [::popeye::spawn $twin $options] pipe greetingLine
    ::popeye::input::Pieces $pipe "total 0"
    ::popeye::input::EndProblem $pipe

    fconfigure $pipe -blocking false
    set output ""
    variable whomovesReadable false
    fileevent $pipe readable { set whomovesReadable true }
    while {![eof $pipe]} {
	vwait whomovesReadable
	set readable false
	append output [read $pipe]
	debug.whomoves "output:|[debuggable $output]|" 2
	if {[regexp -- "\n($movenumbersRE)" $output - firstLine]} {
	    debug.whomoves "firstLine:[debuggable $firstLine]"
	    break
	}
    }
    ::popeye::terminate $pipe

    lassign [::popeye::spawn $twin $options] pipe greetingLine
    ::popeye::input::Pieces $pipe "total 0"
    ::popeye::input::ZeroPosition $pipe "[::msgcat::mc input::Stipulation] h#1"
    ::popeye::input::EndProblem $pipe

    fconfigure $pipe -blocking false
    set output ""
    variable whomovesReadable false
    fileevent $pipe readable { set whomovesReadable true }
    while {![eof $pipe]} {
	vwait whomovesReadable
	set readable false
	append output [read $pipe]
	debug.whomoves "output:|[debuggable $output]|" 2
	if {[regexp -- "\n($movenumbersRE)" $output - firstLineBlack]} {
	    debug.whomoves "firstLineBlack:[debuggable $firstLineBlack]"
	    break
	}
    }
    ::popeye::terminate $pipe

    if {![info exists firstLineBlack]} {
	set result "white"
    } elseif {![info exists firstLine]} {
	set result "black"
    } elseif {$firstLine==$firstLineBlack} {
	set result "black"
    } else {
	set result "white"
    }

    debug.whomoves "whoMoves <- $result"
    return $result
}

proc handleTwin {firstTwin endElmt twinnings skipMoves} {
    debug.twin "handleTwin firstTwin:|$firstTwin| endElmt:$endElmt twinnings:$twinnings skipMoves:$skipMoves"

    set ::output::isBoardTerminatorSuppressed false

    set whomoves [whoMoves $firstTwin $twinnings]
    debug.twin "whomoves:$whomoves" 2

    set ranges [::grouping::auto::makeRanges $firstTwin $twinnings $whomoves $skipMoves]
    debug.twin "ranges:$ranges" 2

    if {[llength $ranges]==0} {
	lappend ranges [list [expr {$skipMoves+1}] [expr {$skipMoves+1}]]
    }
    set result [::tester::moveRanges $firstTwin $twinnings $endElmt $ranges]
	
    debug.twin "handleTwin <- $result"
    return $result
}

proc areMoveNumbersActivated {firstTwin zeroTwinning} {
    debug.movenumbers "areMoveNumbersActivated firstTwin:|$firstTwin| zeroTwinning:$zeroTwinning"

    set result false

    set options "[::msgcat::mc input::NoBoard] [::msgcat::mc input::MaxSolutions] 1"
    debug.movenumbers "options:$options" 2

    lassign [::popeye::spawn $firstTwin $options] pipe greetingLine

    debug.movenumbers "zeroTwinning:|$zeroTwinning|" 2
    ::popeye::input::ZeroPosition $pipe "[::msgcat::mc input::Stipulation] h#0.5"
    ::popeye::input::EndProblem $pipe

    while {[::popeye::output::getLine $pipe line]>=0} {
	debug.movenumbers "line:[debuggable $line]" 2
	set movenumbersRE { *[[:digit:]]+ +[\(][^\)]+[::msgcat::mc output::Time] = [^\)]+[\)]}
	if {[regexp -- "^$movenumbersRE\$" $line]} {
	    set result true
	    break
	}
    }

    ::popeye::terminate $pipe

    if {!$result} {
	append options "[::msgcat::mc input::HalfDuplex]"
	lassign [::popeye::spawn $firstTwin $options] pipe greetingLine

	debug.movenumbers "zeroTwinning:|$zeroTwinning|" 2
	::popeye::input::ZeroPosition $pipe "[::msgcat::mc input::Stipulation] h#0.5"
	::popeye::input::EndProblem $pipe

	while {[::popeye::output::getLine $pipe line]>=0} {
	    debug.movenumbers "line:[debuggable $line]" 2
	    set movenumbersRE { *[[:digit:]]+ +[\(][^\)]+[::msgcat::mc output::Time] = [^\)]+[\)]}
	    if {[regexp -- "^$movenumbersRE\$" $line]} {
		set result true
		break
	    }
	}

	::popeye::terminate $pipe
    }

    debug.movenumbers "areMoveNumbersActivated <- $result"
    return $result
}

proc readFirstTwin {chan} {
    debug.problem "readFirstTwin"

    lassign [::input::readUpTo $chan {Twin ZeroPosition NextProblem EndProblem Protocol}] firstTwin endElmt
    debug.problem "firstTwin:|[debuggable $firstTwin]| endElmt:$endElmt" 2
    while {true} {
	if {$endElmt=="Protocol"} {
	    set protocol [string trim [::input::getLine]]
	    debug.problem "protocol:[debuggable $protocol]" 2
	    lassign [::input::readUpTo $chan {Twin ZeroPosition NextProblem EndProblem Protocol}] firstTwinPart2 endElmt
	    debug.problem "firstTwinPart2:|[debuggable $firstTwinPart2]| endElmt:$endElmt" 2
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
    ::language::init
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
