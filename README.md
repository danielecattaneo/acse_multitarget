# ACSE (Advanced Compiler System for Education) Multitarget

ACSE is a complete toolchain consisting of a compiler (named ACSE), an
assembler, and a simulator (named MACE). It provides a simple but reasonably
accurate sandbox for learning how compilers work.

ACSE has been used in Politecnico di Milano as the main framework for practical
illustration of an application using Flex and Bison since 2008. It is the
subject of countless exam papers in the Formal Languages and Compilers course.
It is able to compile a simplified subset of the C language called LANCE.

Ordinarily, the official version of ACSE generates code for a fictional
architecture named MACE.

This fork makes ACSE multitarget. In other words, the code has been modified
such that it can support target architectures different than MACE. The target
architecture is selected at compile time and can now be one of the
following:

 - `mace` The old MACE fictional architecture
 - `amd64` Unix-like x86_64 operating systems

## Architecture of multitarget ACSE

The MACE instruction set is retained as an intermediate representation internal
to the compiler, common to all architectures. This allows the frontend to stay
completely unmodified with respect to the MACE-only version of ACSE, retaining
compatibility with almost 20 years of exam paper solutions developed at
Politecnico di Milano.

At the end of the initial frontend code generation step (implemented in
Acse.y), depending on the target architecture, the instructions in the IR are
immediately translated to machine-specific instructions.

Each machine-specific instruction is represented using a subset of ordinary
MACE-style instructions. For example, the x86_64 instruction `add r1, r2` is
represented by a MACE instruction in the form `add r1 r1 r2`. Any MACE `add`
instruction with a destination reg. different than the first source register is
now illegal after this pass.

This pass is implemented by the `doTargetSpecificTransformations()` function.
In order to handle architecture (such as amd64) where some instructions have
fixed source or destinations registers, this step also introduces constraints
for the register allocator to force the allocation of some temporaries to one
or more machine registers.

At this point, the compilation flow is largely unchanged until reaching
the final assembly code emission pass. The assembly printer will do the final
translation of IR language code into the syntax required by the assembler for
the target architecture.

## How to use ACSE

ACSE was tested on the following operating systems:

- **Linux** (any recent 32 bit or 64 bit distribution should work)
- **macOS** (any recent version should work)
- **Windows** (both 32 bit or 64 bit) when built with
  [MinGW](http://www.mingw.org) under [MSYS2](https://www.msys2.org), or inside
  **Windows Services for Linux** (WSL).

If you are using **Linux** or **macOS**, ACSE requires the following programs
to be installed:

- a **C compiler** (for example *GCC* or *clang*)
- **GNU bison**
- **GNU flex**

If you use **Windows**, first you must install either the
[MSYS2](https://www.msys2.org) environment or **Windows Services for Linux**
(WSL). Both MSYS2 and Windows Services for Linux (WSL) provide a Linux-like
environment inside of Windows.

Once you have installed either MSYS2 or WSL, you can use the following
instructions just as if you were using Linux or macOS.

However notice that the output of ACSE multitarget amd64 is not tested to work
on Windows (although it might).

### Building ACSE

To build the ACSE compiler toolchain, open a terminal and type:

      make target=xxx

where `xxx` is either `amd64` or `mace`. If unspecified the target is `mace`,
so make sure to set `target` to `amd64` to check out the new hotness.

The built executables will be located in the `bin` directory.

### Testing ACSE

To compile some examples (located in the directory `tests`) type:

      make tests

### Using ACSE

You can compile new Lance programs in this way (suppose you
have saved a Lance program in `myprog.src`):

      ./bin/acse myprog.src myprog.asm

The following steps will depend on the architecture.

- For `mace` use the builtin `asm` and `mace` tools to build a binary from the
  assembly file and execute it:
  ```
  ./bin/asm myprog.asm myprog.o
  ./bin/mace myprog.o
  ```
- For `amd64`, the output assembly files must be compiled to a valid object
  file using [NASM](https://www.nasm.us/). The GNU assembler (`as`) **will NOT**
  work, as ACSE is only able to generate Intel-syntax assembly. Intel syntax
  is the best syntax for x86_64 assembly code anyway.
  
  At this point the resulting object file must be linked with the LANCE runtime
  using any decent C compiler you like (GCC or clang).
  
  In summary, the commands to execute are:
  ```
  nasm -o myprog.o myprog.asm
  cc -o myprog myprog.o ./acse/amd64/runtime/lance_rt.c
  ```
  Now you have an executable binary that can be run.

Alternatively, you can add a test to the `tests` directory by following these
steps:

1. Create a new directory inside `tests`. You can choose whatever directory
   name you wish, as long as it is not `test`.
2. Move the source code files to be compiled inside the directory you have
   created. The extension of these files **must** be `.src`.
3. Run the command `make tests target=xxx` to compile all tests, included the
   one you have just added. **Remember to set the `target` variable to the
   same string used to compile ACSE!**

The `make tests` command only compiles and assembles the programs, they must be
run separately (duh).
