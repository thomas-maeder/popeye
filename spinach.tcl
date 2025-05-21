#! /usr/bin/env tclsh

set version "v4.93"

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

control::control assert enabled [string match "*.tcl" $::argv0]

# define the debugging tags in deactivated state
# they can be activated using command line options
debug off cmdline
debug off input
debug off output
debug off language
debug off popeye
debug off problem
debug off sync
debug off tester
debug off twin

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

proc defaultFreePhysicalMemory {} {
    if {$::tcl_platform(platform)=="windows"} {
	set result [::msgcat::mc cmdline::freePhysicalDividedByNrProcs]
    } else {
	set result [::msgcat::mc cmdline::popeyeDefault]
    }

    return $result
}

namespace eval language {
    namespace eval fr {
	namespace eval popeye {
	    variable versionNotDetected "Avertissement: Version de Popeye indétectable"
	    variable versionNotSupported "Version de Popeye non supportée; >=4.91 nécessaire"
	}

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
	    variable MoveNumbers "Trace"
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
	    variable freePhysicalDividedByNrProcs "Mémoire physique disponible / nrprocs"
	    variable maxnrmoves "Nombre maximum de coups de départ par jumeau"
	    variable version "Version de ce programme"
	}

	namespace eval spinach {
	    variable elementNotSupported "Element non supporté"
	    variable unexpectedEOF "Fin d'output inattendue d'un sous-processus Popeye"
	    variable unexpectedPopeyeOutput "Output Popeye inattendu"
	}
    }

    namespace eval de {
	namespace eval popeye {
	    variable versionNotDetected "Warnung: Popeye-Version kann nicht ermittelt werden"
	    variable versionNotSupported "Popeye-Version nicht unterstützt; >=4.91 erforderlich"
	}

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
	    variable MoveNumbers "ZugNummer"
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
	    variable freePhysicalDividedByNrProcs "Freier physischer Speicher / nrprocs"
	    variable maxnrmoves "Maximale Anzahl Startzüge pro Zwilling"
	    variable version "Version dieses Programms"
	}

	namespace eval spinach {
	    variable elementNotSupported "Element nicht unterstützt"
	    variable unexpectedEOF "Unerwartetes Ende der Ausgabe von einem Popeye-Unterprozess"
	    variable unexpectedPopeyeOutput "Unerwartete Ausgabe von Popeye"
	}
    }

    namespace eval en {
	namespace eval popeye {
	    variable versionNotDetected "Warning: Popeye version not detected"
	    variable versionNotSupported "Popeye version not supported; >=4.91 required"
	}

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
	    variable MoveNumbers "MoveNumbers"
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
	    variable freePhysicalDividedByNrProcs "Free physical memory / nrprocs"
	    variable maxnrmoves "maximum number of starting moves per twin"
	    variable version "Version of this program"
	}

	namespace eval spinach {
	    variable elementNotSupported "Element not supported"
	    variable unexpectedEOF "Unexpected end of output from a Popeye sub-process"
	    variable unexpectedPopeyeOutput "Unexpected output from Popeye"
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
		# Windows uses some cpnnn encoding, but this source file is UTF-8
		set value [encoding convertfrom utf-8 [set $var]]
		namespace eval :: [list ::msgcat::mcset $shortcut ${category}::${symbol} $value]
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
    exit 1
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
    variable areMovenumbersSuppressed true

    variable startTime [clock milliseconds]
    variable latestCarry ""
    variable latestFinish

    variable nextMoveNumber 1
    variable nextTwinningMark "a"
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

proc ::output::enableMovenumbers {enable} {
    variable areMovenumbersSuppressed

    debug.output "enableMovenumbers enable:$enable"

    set areMovenumbersSuppressed [expr {!$enable}]
}

proc ::output::advanceTwinningMark {} {
    variable nextTwinningMark

    if {$nextTwinningMark=="z"} {
	set nextTwinningMark "z1"
    } elseif {[regexp -- {^z([1-9][0-9]*)$} $nextTwinningMark - number]} {
	incr number
	set nextTwinningMark "z$number"
    } else {
	set nrConversions [scan $nextTwinningMark "%c" characterCode]
	::control::assert {$nrConversions==1} "::output::advanceTwinningMark: failed to advance twinning mark"
	incr characterCode
	set nextTwinningMark [format "%c" $characterCode]
    }
}

proc ::output::twinning {twinning} {
    variable nextTwinningMark

    _puts "\n$nextTwinningMark) $twinning"
}

proc ::output::solution {string} {
    variable latestCarry

    _puts $latestCarry
    set latestCarry ""

    _puts $string
}

proc ::output::endOfPhase {} {
    solution "\n"
}

proc ::output::_formattedTime {} {
    variable startTime

    set timeNow [clock milliseconds]

   set solvingTimeMS [expr {$timeNow-$startTime}]
    set solvingTimeS [expr {$solvingTimeMS/1000}]
    set solvingTimeM [expr {$solvingTimeS/60}]
    set solvingTimeH [expr {$solvingTimeM/60}]

    if {$solvingTimeH>0} {
	incr solvingTimeS [expr {-$solvingTimeM*60}]
	incr solvingTimeM [expr {-$solvingTimeH*60}]
	set timeFormatted [format "%lu:%02lu:%02lu h:m:s" $solvingTimeH $solvingTimeM $solvingTimeS]
    } elseif {$solvingTimeM>0} {
	incr solvingTimeMS [expr {-$solvingTimeS*1000}]
	incr solvingTimeS [expr {-$solvingTimeM*60}]
	set timeFormatted [format "%lu:%02lu.%02lu m:s" $solvingTimeM $solvingTimeS $solvingTimeMS]
    } else {
	incr solvingTimeMS [expr {-$solvingTimeS*1000}]
	set timeFormatted [format "%lu.%03lu s" $solvingTimeS $solvingTimeMS]
    }

    return $timeFormatted
}

proc ::output::movenumberLine {numberedMove time} {
    variable areMovenumbersSuppressed
    variable latestCarry
    variable nextMoveNumber

    _puts $latestCarry
    set latestCarry ""

    if {!$areMovenumbersSuppressed} {
	_puts "\n[format %3d $nextMoveNumber]$numberedMove[::msgcat::mc output::Time] = [_formattedTime])"
	incr nextMoveNumber
    }
}

proc ::output::rememberFinish {carry finish time suffix} {
    variable latestCarry
    variable latestFinish

    set latestCarry $carry
    set latestFinish [list $finish $time $suffix]
}

proc ::output::writeLatestFinish {} {
    variable latestFinish

    if {[info exists latestFinish]} {
	lassign $latestFinish finish time suffix
	_puts "$finish[_formattedTime]$suffix"
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

proc ::popeye::spawn {} {
    variable executablePath
    variable maxmemOption

    debug.popeye "spawn"
    
    debug.popeye "executablePath:$executablePath" 2
    debug.popeye "maxmemOption:[debuggable $maxmemOption]" 2

    set pipe [open "| \"[join [file split $executablePath] /]\" $maxmemOption" "r+"]
    debug.popeye "pipe:$pipe pid:[pid $pipe]" 2
    debug.popeye "caller:[debuggable [info level -1]]" 2

    # seems not to work on Windows
    #fconfigure $pipe -encoding binary -buffering line

    gets $pipe greetingLine

    set result [list $pipe $greetingLine]

    puts $pipe [::msgcat::mc input::BeginProblem]

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

proc ::popeye::input::Problem {pipe problem} {
    debug.popeye "input::Problem pipe:$pipe problem:|$problem|"

    puts $pipe $problem
}

proc ::popeye::input::Options {pipe options} {
    debug.popeye "input::Options pipe:$pipe options:|$options|"

    puts $pipe "[::msgcat::mc input::Option] [join $options]"
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

proc ::popeye::input::NextProblem {pipe} {
    debug.popeye "input::NextProblem pipe:$pipe"

    puts $pipe "[::msgcat::mc input::NextProblem]"

    # Windows seems not to allow configuring line buffering
    flush $pipe
}

proc ::popeye::input::EndProblem {pipe} {
    debug.popeye "input::EndProblem pipe:$pipe"

    puts $pipe "[::msgcat::mc input::EndProblem]"

    # Windows seems not to allow configuring line buffering
    flush $pipe
}

namespace eval ::popeye::output {
}

proc ::popeye::output::getLine {pipe varname} {
    upvar $varname line

    return [gets $pipe line]
}

proc ::popeye::output::doAsync {pipe listener args} {
    debug.popeye "output::doAsync pipe:$pipe listener:$listener arguments:[debuggable $args]"

    fconfigure $pipe -blocking false

    set callback [linsert $args 0 [uplevel namespace which -command $listener] $pipe]
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
	{ executable.arg "[defaultPopeyeExecutable]"   "[::msgcat::mc cmdline::popeyePath]" }
	{ nrprocs.arg    "[defaultNumberOfProcesses]"  "[::msgcat::mc cmdline::numberProcesses]" }
	{ maxmem.arg     "[defaultFreePhysicalMemory]" "[::msgcat::mc cmdline::processMemory]" }
	{ version        "[::msgcat::mc cmdline::version]" }
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
	if {[info exists ::env(SHELL)] && [file tail $::env(SHELL)]=="bash"} {
	    puts stderr "Use the following command to activate simple bash completion:"
	    puts stderr "complete -F _longopt [file tail $::argv0]"
	}
	exit 1
    }

    if {$::params(version)} {
	puts "[::cmdline::getArgv0] $::version"
	exit
    }

    if {$::params(executable)==""} {
	puts stderr [::cmdline::usage $options $usage]
	exit 1
    } else {
	::popeye::setExecutable $::params(executable)
    }
    if {$::params(maxmem)==[::msgcat::mc cmdline::freePhysicalDividedByNrProcs]} {
	set freeTotal [lindex [split [string trim [exec wmic OS get FreePhysicalMemory]]] end]
	::popeye::setMaxmem "[expr {$freeTotal/$::params(nrprocs)}]k"
    } elseif {$::params(maxmem)==[::msgcat::mc cmdline::popeyeDefault]} {
    } else {
	::popeye::setMaxmem $::params(maxmem)
    }

    if {[llength $::argv]>0} {
	set ::params(inputfile) [lindex $::argv 0]
    }

    if {[info exists ::params(assert)] && $::params(assert)} {
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
		debug.sync "args:$args" 2
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

    if {$result==1} {
	debug.sync "Wait <-! $options $result"
	return -options $options $result
    } else {
	debug.sync "Wait <- $args"
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

proc ::tester::async::_consume {pipe} {
    variable buffers

    debug.tester "eof:[eof $pipe]"
    if {[eof $pipe]} {
	::popeye::terminate $pipe
	::sync::Notify $pipe "eof"
	return false
    } else {
	set chunk [read $pipe]
	debug.tester "chunk:$chunk" 2
	append buffers($pipe) $chunk
	return true
    }
}

proc ::tester::async::_endOfSolutionReached {pipe} {
    variable buffers

    set timeLabelRE [::msgcat::mc output::Time]
    set timeRE {[[:digit:]:.]+}
    set entireSolutionRE "(.*)\n(\n[::msgcat::mc output::SolutionFinished]\[.] $timeLabelRE = )($timeRE \[^\n]+)(\n+)"
    if {[regexp -- $entireSolutionRE $buffers($pipe) - solution finished time suffix]} {
	debug.tester "solution:>$solution<" 2
	if {![regexp -- "^(.*)(\n)$" $solution - solution carry]} {
	    set carry ""
	}
	unset buffers($pipe)
	set result [list $solution $carry $finished $time $suffix]
    } else {
	set result [list "" "" "" "" ""]
    }
}

proc ::tester::async::_moveNumberRead {pipe} {
    variable buffers

    set parenOpenRE {[\(]}
    set parenCloseRE {[\)]}
    set movenumberRE { *[[:digit:]]+}
    set moveRE {[^\n]+}
    set timeLabelRE [::msgcat::mc output::Time]
    set timeRE {[[:digit:]:.]+}
    set timeUnitRE {(?:(?:h:)?m:)?s}
    set movenumberLineRE "\n${movenumberRE}( +$parenOpenRE$moveRE +)$timeLabelRE = ($timeRE) $timeUnitRE$parenCloseRE"

    if {[regexp -- "${movenumberLineRE}(.*)" $buffers($pipe) - numberedMove time remainder]} {
	set buffers($pipe) $remainder
	set result [list $numberedMove $time]
    } else {
	set result [list "" ""]
    }
}

proc ::tester::async::readable {pipe} {
    variable buffers

    debug.tester "readable pipe:$pipe"

    if {[_consume $pipe]} {
	lassign [_endOfSolutionReached $pipe] solution carry finished time suffix
	if {$finished!=""} {
	    ::output::solution $solution
	    ::sync::Notify $pipe "solution"
	    ::output::rememberFinish $carry $finished $time $suffix
	}
    }

    debug.tester "readable <-"
}

proc ::tester::async::moveNumber {pipe} {
    variable buffers

    debug.tester "moveNumber pipe:$pipe"

    if {[_consume $pipe]} {
	lassign [_moveNumberRead $pipe] numberedMove time
	lassign [_endOfSolutionReached $pipe] solution carry finished time suffix
	if {$numberedMove!=""} {
	    ::output::movenumberLine $numberedMove $time
	    if {$finished==""} {
		::popeye::output::doAsync $pipe readable
	    } else {
		::output::solution $solution
		::sync::Notify $pipe "solution"
		::output::rememberFinish $carry $finished $time $suffix
	    }
	} elseif {$finished!=""} {
	    ::sync::Notify $pipe "prematureEndOfSolution"
	}
    }

    debug.tester "moveNumber <-"
}

proc ::tester::setplayRange {pipe firstTwin writeTwinning twinning} {
    debug.tester "setplayRange pipe:$pipe firstTwin:|[debuggable $firstTwin]| writeTwinning:$writeTwinning twinning:|[debuggable $twinning]|"

    lappend options [::msgcat::mc input::NoBoard]
    lappend options [::msgcat::mc input::MoveNumbers]
    lappend options "[::msgcat::mc input::UpToMoveNumber] 0"

    ::popeye::input::Problem $pipe $firstTwin
    ::popeye::input::Options $pipe $options
    if {$twinning!=""} {
	::popeye::input::ZeroPosition $pipe $twinning
    }
    ::popeye::input::EndProblem $pipe

    set lines ""
    while {[::popeye::output::getLine $pipe line]>=0} {
	append lines "$line\n"
    }

    debug.tester "lines:>[debuggable $lines]<" 2

    if {$twinning==""} {
	if {$writeTwinning} {
	    ::output::twinning "\n"
	}
    } else {
	set twinningLetterRE {a\)}
	set twinningRE {[^\n]*\n(?:[^\n]+\n)*}
	if {[regexp -- "^\n[::msgcat::mc input::ZeroPosition]\n\n\n$twinningLetterRE ($twinningRE)(\n.*)" $lines - twinning lines]} {
	    debug.tester "setplayRange - twinning:|[debuggable $twinning]| lines:>[debuggable $lines]<" 2
	    if {$writeTwinning} {
		::output::twinning $twinning
	    }
	} else {
	    puts stderr "[::msgcat::mc spinach::unexpectedPopeyeOutput]:\n$lines"
	    exit 1
	}
    }

    set timeLabelRE [::msgcat::mc output::Time]
    set timeRE {[[:digit:]:.]+}
    set entireSolutionRE "(.*)(\n)(\n\n[::msgcat::mc output::SolutionFinished]\[.] $timeLabelRE = )($timeRE \[^\n]+)(\n+)"
    if {[regexp -- $entireSolutionRE $lines - solution carry finished time suffix]} {
	debug.tester "solution:>$solution<" 2
	::output::solution $solution
	::output::rememberFinish $carry $finished $time $suffix
    } else {
	# no set play etc.
    }

    debug.tester "setplayRange <-"
}

proc ::tester::testMove {pipe firstTwin twinning move} {
    debug.tester "testMove pipe:$pipe firstTwin:|[debuggable $firstTwin]| twinning:|[debuggable $twinning]| move:$move"

    lappend options [::msgcat::mc input::NoBoard]
    lappend options [::msgcat::mc input::MoveNumbers]
    lappend options "[::msgcat::mc input::StartMoveNumber] $move"
    lappend options "[::msgcat::mc input::UpToMoveNumber] $move"

    ::popeye::input::Problem $pipe $firstTwin
    ::popeye::input::Options $pipe $options
    if {$twinning!=""} {
	::popeye::input::ZeroPosition $pipe $twinning
    }
    ::popeye::input::NextProblem $pipe

    ::popeye::output::doAsync $pipe async::moveNumber

    debug.tester "testMove <-"
}

proc ::tester::testProgress {pipe notification firstTwin twinning nrRunningProcesses currMove} {
    debug.tester "testProgress pipe:$pipe notification:$notification twinning:|[debuggable $twinning]| nrRunningProcesses:$nrRunningProcesses currMove:$currMove"

    switch -exact $notification {
	solution  {
	    testMove $pipe $firstTwin $twinning $currMove
	    incr currMove
	}
	prematureEndOfSolution {
	    ::popeye::terminate $pipe
	    incr nrRunningProcesses -1
	}
	eof {
	    puts stderr "[::msgcat::mc spinach::unexpectedEOF]"
	    exit 1
	}
	default {
	    control::assert false "::tester::testProgress: unexpected notification $notification"
	    exit 1
	}
    }

    if {$nrRunningProcesses==0} {
	::output::endOfPhase
	debug.tester "testProgress <- break"
	return -code break
    } else {
	set result [list $firstTwin $twinning $nrRunningProcesses $currMove]
	debug.tester "testProgress <- [debuggable $result]"
	return $result
    }
}

proc ::tester::test {firstTwin writeTwinning twinning} {
    debug.tester "test firstTwin:|[debuggable $firstTwin]| writeTwinning:$writeTwinning twinning:|[debuggable $twinning]|"

    # synchronously deal with everything happening before move 1, e.g. set play
    lassign [::popeye::spawn] setplayPipe greetingLine
    debug.tester "setplayPipe:$setplayPipe" 2
    setplayRange $setplayPipe $firstTwin $writeTwinning $twinning
    ::popeye::terminate $setplayPipe

    ::sync::Init

    set currMove 1
    for {set nrRunningProcesses 0} {$nrRunningProcesses<$::params(nrprocs)} {incr nrRunningProcesses} {
	lassign [::popeye::spawn] pipe greetingLine
	testMove $pipe $firstTwin $twinning $currMove
	incr currMove
    }

    debug.tester "nrRunningProcesses:$nrRunningProcesses" 2

    # asynchronously deal with the processes created for the real moves
    lassign [::sync::Wait testProgress $firstTwin $twinning $nrRunningProcesses $currMove] firstTwin nrProcesses nrMovesPlayed

    debug.tester "test - firstTwin:|[debuggable $firstTwin]| nrProcesses:$nrProcesses $nrMovesPlayed:$nrMovesPlayed"

    ::sync::Fini

    debug.tester "test <-"
}

proc handleTwin {firstTwin writeTwinning {twinning ""}} {
    debug.twin "handleTwin firstTwin:|[debuggable $firstTwin]| writeTwinning:$writeTwinning twinning:|[debuggable $twinning]|"

    ::tester::test $firstTwin $writeTwinning $twinning
	
    debug.twin "handleTwin <-"
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

    if {$endElmt!="EndProblem" && $endElmt!="Twin" && $endElmt!="ZeroPosition"} {
	puts stderr "[::msgcat::mc spinach::elementNotSupported]: [::msgcat::mc input::$endElmt]"
	exit 1
    }

    if {[info exists protocol]} {
	::output::openProtocol $protocol
    }

    set result [list $firstTwin $endElmt]

    debug.problem "readFirstTwin <- [debuggable $result]"
    return $result
}

proc handleFirstTwin {chan} {
    debug.problem "handleFirstTwin"

    lassign [readFirstTwin $chan] firstTwin endElmt

    if {[string match -nocase "*[::msgcat::mc input::MoveNumbers]*" $firstTwin]} {
	::output::enableMovenumbers true
    }

    if {$endElmt!="ZeroPosition"} {
	set writeTwinning [expr {$endElmt=="Twin"}]
	handleTwin $firstTwin $writeTwinning
	::output::advanceTwinningMark
    }

    set result [list $firstTwin $endElmt]

    debug.problem "handleFirstTwin <- $result"
    return $result
}

proc handleNextTwin {chan firstTwin} {
    debug.problem "handleNextTwin - firstTwin:[debuggable $firstTwin]"

    lassign [::input::readUpTo $chan {Twin NextProblem EndProblem}] twinning endElmt
    debug.problem "twinning:|[debuggable $twinning]| endElmt:$endElmt" 2

    set writeTwinning true
    set nrFirstMoves [handleTwin $firstTwin $writeTwinning $twinning]
    ::output::advanceTwinningMark

    debug.problem "handleNextTwin <- $endElmt"
    return $endElmt
}

proc handleProblem {chan} {
    debug.problem "handleProblem"

    lassign [handleFirstTwin $chan] firstTwin endElmt

    while {$endElmt=="Twin" || $endElmt=="ZeroPosition"} {
	set endElmt [handleNextTwin $chan $firstTwin]
    }

    debug.problem "handleProblem <- $endElmt"
    return $endElmt
}

proc handleInput {chan} {
    ::input::detectLanguage $chan

    set endElmt [handleProblem $chan]
    while {$endElmt=="NextProblem"} {
	set endElmt [handleProblem $chan]
    }
}

proc handleGreetingLine {} {
    debug.problem handleGreetingLine

    lassign [::popeye::spawn] pipe greetingLine
    ::popeye::terminate $pipe

    set greetingLineRE {^Popeye [^ ]* v([0-9]+)[.]([0-9]+) [(][^\n]+[)]}
    if {[regexp -- $greetingLineRE $greetingLine - major minor]} {
	if {$major<4 || ($major==4 && $minor<91)} {
	    puts stderr [::msgcat::mc popeye::versionNotSupported]
	    exit 1
	}
    } else {
	puts stderr [::msgcat::mc popeye::versionNotDetected]
    }

    ::output::greetingLine "$greetingLine\n"

    debug.problem "handleGreetingLine <-"
}

proc main {} {
    ::language::init
    parseCommandLine
    handleGreetingLine

    if {[info exists ::params(inputfile)]} {
	set chan [open $::params(inputfile) "r"]
	handleInput $chan
	close $chan
    } else {
	handleInput stdin
    }

    ::output::writeLatestFinish
    ::output::closeProtocol
}

main
