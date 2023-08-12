To install on MacOS (tested on an M2 Macbook Pro in 2023 under MacOS Monerey):

1. Popeye's build system requires an up-to-date `clang`, `ld` etc. which are not provided by Apple by default.

```
$ brew install llvm
```

2. Either copy the following lines into your .zshrc to point at the newly installed tools, or execute the following commands in a shell where you will be building Popeye:

```
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
export LDFLAGS="-L/opt/homebrew/opt/llvm/lib"
export CPPFLAGS="-I/opt/homebrew/opt/llvm/include" 
```

3. At the top level of the Popeye source code distribution, edit `makefile.defaults`. Find the line that says `TOOLCHAIN` and ensure it is pointing at `clang`:

```
TOOLCHAIN=clang
```

4. Now simply run `make -f makefile.unx`, which, after compiling and linking 500-odd source files, should result in a working Popeye binary called `py` in the current directory.

5. (Optional: basic testing) Launch `py` with no arguments. It should display the following header (or some variation, depending on your MacOS version):

`Popeye Darwin-21.6.0-arm-64Bit v4.87 (1024 MB)`

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

6. (Optional: comprehensive testing) Run any of the self-test scripts (`solveExamples.sh` is quite comprehensive, for instance) to confirm all functionality works.
