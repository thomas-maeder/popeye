#! /usr/bin/env tclsh

source output/plaintext/documentation/german
source output/plaintext/documentation/english
source output/plaintext/documentation/cmdline.tcl
source output/plaintext/documentation/grammarParser.tcl
source output/plaintext/documentation/verifier.tcl

set languagesRE [join [regsub -all ::language:: [namespace children ::language] ""] "|"]

set options [subst -nocommands -nobackslashes {
    { language.arg "english" "output language ($languagesRE)" }
}]

set usageIntro [join {
    {: [options] filepath ...}
    {}
    {options:}
} "\n"]

try {
    array set params [::cmdline::getoptions argv $options $usageIntro]

    if {[regexp -- $languagesRE $params(language)]} {
	# Note: argv is modified now. The recognized options are
	# removed from it, leaving the non-option arguments behind.
	set params(inputFilenames) $argv
    } else {
	::cmdline::Error [::cmdline::usage $options $usageIntro] USAGE
    }
} trap {CMDLINE USAGE} {msg o} {
    # Trap the usage signal, print the message, and exit the application.
    # Note: Other errors are not caught and passed through to higher levels!
    puts $msg
    exit 1
}

::grammarParser::parse output/plaintext/documentation/grammar ::grammar

::verifier::init ::grammar

foreach inputFilename $params(inputFilenames) {
    set f [open $inputFilename "r"]
    set input [read $f]
    close $f

    ::verifier::treatInputFile $inputFilename $input
}
