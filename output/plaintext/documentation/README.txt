The Popeye plaintext output checker verifies that the ouptut files provided in
directory REGRESSIONTESTS correspond to the grammar documented in this directory.

It does so by converting the grammar into very complex regular expressions, which
are then applied by the Tcl interpreter to the requested input files.

The regular expressions are too comples for a regular tclsh executable obtainable
through the usual package repositories. A tclsh executable specially built for the
purpose is necessary.

The tcl-build project provides an easy way of building such a tclsh executable in
a portable and reliable way.

1. Pull the latest tcl-bulid image
$ docker pull oupfiz5/tcl-build:latest

2. Start and prepare a docker container named tcl-engine with the tclsh executable
$ git clone https://github.com/oupfiz5/tcl-build.git
$ cd tcl-build/
$ mkdir workspaces
$ docker run --interactive --tty --detach --volume $(pwd)/workspaces:/workspaces --volume $(pwd)/builds:/builds --volume POPEYEDIR:/popeye --name tcl-engine oupfiz5/tcl-build:latest
  replace POPEYEDIR with the directory where you cloned the Popeye github repository to (contains directory REGRESSIONTESTS)
  you can use basically any name instead of /popeye for the mapped directory - just make sure to use the same name with the --workdir paramter below
$ docker exec --interactive --tty --env CFLAGS=-DREG_MAX_COMPILE_SPACE=10000000000 tcl-engine bash -c "/builds/tcl-build.sh && /builds/tcllib-build.sh"
  this compiles the tclsh executable and installs the Tcl Standard Library
$ rm -rf workspaces/*
  optional: save some 100M of disk space

3. Use the tclsh executable
$ docker exec --interactive --tty --workdir /popeye tcl-engine scripts/checkOutput.tcl REGRESSIONTESTS/*.ref

4. Clean-up
$ docker stop tcl-engine
$ docker rm tcl-engine
$ rm -rf tcl-build
