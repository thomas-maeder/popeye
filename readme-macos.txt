To install on MacOS (tested on an M2 Macbook Pro in 2025 under MacOS Sequoia):

1. Popeye's build system requires an up-to-date `clang`, `ld` etc. which are not provided by Apple by default. Update by using [homebrew](https://brew.sh):

```
$ brew install llvm
```

2. Either copy the following lines into your .zshrc to point at the newly installed tools, or execute the following commands in a shell where you will be building Popeye:

```
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
export LDFLAGS="-L/opt/homebrew/opt/llvm/lib"
export CPPFLAGS="-I/opt/homebrew/opt/llvm/include" 
```

3. Now simply run `make -f makefile.unx TOOLCHAIN=clang`, which, after compiling and linking 500-odd source files, should result in a working Popeye binary called `py` in the current directory.

4. (Optional: basic testing) Launch `py` with no arguments. It should display the following header (or some variation, depending on your MacOS and popeye versions):

`Popeye Darwin-24.6.0-arm-64Bit v4.99 (1024 MB)`

Now paste the following into the terminal:

```
BeginProblem
Stipulation #2
Option Variation NoBoard
Pieces
    White Rb8 Kb3
    Black Kb1
EndProblem
```

Popeye should respond by solving the problem, then exiting with no errors:

```
   1.Rb8-c8 ! zugzwang.
      1...Kb1-a1
          2.Rc8-c1 #


solution finished. Time = 0.018 s
```

5. (Optional: comprehensive testing) Run any of the self-test scripts (`solveExamples.sh` is quite comprehensive, for instance) to confirm all functionality works.
