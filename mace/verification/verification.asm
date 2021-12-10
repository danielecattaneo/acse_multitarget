/* 
 * MACE instruction verification program
 * 2020 Daniele Cattaneo, Politecnico di Milano
 *
 *   The MACE instruction verification program self-verifies the behavior of
 * (almost) all instructions supported by the simulator (with some exceptions).
 *   In case an instruction does not behave as expected, an error code is
 * written to the output depending on the instruction tested and on which
 * specific test failed. Additional numbers are printed in order to log the
 * failure mode more exactly.
 *
 * The test program operates in 3 phases:
 *   Phase 1: The only instructions assumed to be working are addi, nop, halt,
 *            write, xpsw. 
 *            This phase verifies the correct behavior of bne, bt load, store, 
 *            mova, jsr and ret.
 *            There are two sub-phases to Phase 1:
 *              1A: Only tests basic behavior of instructions
 *              1B: Test of corner cases
 *            The tests in phase 1A are considered critical, and as soon as an
 *            error is detected, the program displays a single error code and 
 *            halts.
 *   Phase 2: In addition to the assumptions of Phase 1, assumes instructions 
 *            add, shr(i), eorb andbi, sub are working (at least in their basic 
 *            behavior).
 *            All sXX and bXX instructions are tested with all possible
 *            values of the flags (PSW) register.
 *   Phase 3: In addition to the assumptions of Phase 2, assumes instructions
 *            orb, subi, shli are working (at least in their basic behavior).
 *            All ternary and binary instructions are tested with predefined
 *            sets of "interesting" inputs.
 *
 * Possible output codes:
 *   -1001  Phase 1: BNE/BT test failed (BNE or BT jumped to the wrong location)
 *   -1002  Phase 1: LOAD test failed (LOAD did not load the expected value)
 *   -1003  Phase 1: STORE test failed (STORE did not store the expected value 
 *          in the expected location)
 *   -1004  Phase 1: MOVA test failed (MOVA did not load the expected value)
 *   -1005  Phase 1: JSR test failed (JSR did not jump to the expected location,
 *          or did not push the expected return value in the expected location)
 *   -1006  Phase 1: RET test failed (RET did not jump to the expected location,
 *          or did not pop the expected return value from the expected location)
 *   -2000  All phases: instruction test failed. Additional output:
 *            1. ID of the instruction that failed. The mapping between IDs and
 *               instructions is found in the instruction table (search the
 *               label Instructions).
 *            2. Initial value of RS1
 *            3. Initial value of RS2
 *            4. Value of RD after the execution of the instruction.
 *            5. Value of the PSW after the execution of the instruction.
 *            6. Expected value of RD
 *            7. Expected value of the PSW
 *    1000  Tests terminated. Additional output:
 *            1. number of succeeded tests in phases 2 and 3
 *            2. number of failed tests in phases 2 and 3
 *
 * Caveats: For practical reasons, not all instructions are tested throughly.
 *   Some instructions cannot be tested due to insufficient introspection
 * functionality (i.e. write, read)
 *   Phases 2 and 3 could be modified to require less instructions to be
 * functional, and phase 1 could be modified to test more instructions at an
 * early phase.
 */

.data


Const55AA55AA:
      .WORD 0x55AA55AA
Const7FFFFFFF:
      .WORD 0x7FFFFFFF
ConstNeg12345:
      .WORD -12345
ConstNeg0x7FFFF:
      .WORD -0x7FFFF
ConstTestFailedCode:
      .WORD -2000
ConstTestsEndedCode:
      .WORD 1000
NumSucceeded:
      .WORD 0
NumFailed:
      .WORD 0
AddressOne:
      .WORD 0
AddressTwo:
      .WORD 0

TestTernOrBinData:
      .WORD     0x20612
      .WORD         0xa        0x14 /* input 0 */
      .WORD         0xa       -0x14 /* input 1 */
      .WORD        -0xa        0x14 /* input 2 */
      .WORD        -0xa       -0x14 /* input 3 */
      .WORD  0x40000000  0x40000000 /* input 4 */
      .WORD -0x40000000  0x40000000 /* input 5 */
      .WORD  0x40000000 -0x40000000 /* input 6 */
      .WORD -0x40000000 -0x40000000 /* input 7 */
      .WORD  0x7fffffff         0x0 /* input 8 */
      .WORD  0x7fffffff         0x1 /* input 9 */
      .WORD  0x7fffffff        -0x1 /* input 10 */
      .WORD  0x7fffffff  0x7fffffff /* input 11 */
      .WORD  0x7fffffff -0x80000000 /* input 12 */
      .WORD -0x80000000         0x0 /* input 13 */
      .WORD -0x80000000         0x1 /* input 14 */
      .WORD -0x80000000        -0x1 /* input 15 */
      .WORD -0x80000000 -0x80000000 /* input 16 */
      .WORD -0x80000000  0x7fffffff /* input 17 */
      /* outputs of add */
      .WORD        0x1e        -0xa         0xa       -0x1e
      .WORD -0x80000000         0x0         0x0 -0x80000000
      .WORD  0x7fffffff -0x80000000  0x7ffffffe        -0x2
      .WORD        -0x1 -0x80000000 -0x7fffffff  0x7fffffff
      .WORD         0x0        -0x1
      /* outputs of sub */
      .WORD        -0xa        0x1e       -0x1e         0xa
      .WORD         0x0 -0x80000000 -0x80000000         0x0
      .WORD  0x7fffffff  0x7ffffffe -0x80000000         0x0
      .WORD        -0x1 -0x80000000  0x7fffffff -0x7fffffff
      .WORD         0x0         0x1
      /* expected flags */
      .WORD -0x6aa56e80  0x3888a1a0 -0x7bf7e679 -0x74b4ffb5
      .WORD      0x2492

      .WORD     0x3060a
      .WORD         0x0         0x0 /* input 0 */
      .WORD         0x0         0x1 /* input 1 */
      .WORD         0x1         0x0 /* input 2 */
      .WORD         0x1         0x1 /* input 3 */
      .WORD         0x0    0xbc614e /* input 4 */
      .WORD    0xbc614e         0x0 /* input 5 */
      .WORD    0xbc614e    0xbc614e /* input 6 */
      .WORD         0x0   -0xbc614e /* input 7 */
      .WORD   -0xbc614e         0x0 /* input 8 */
      .WORD   -0xbc614e   -0xbc614e /* input 9 */
      /* outputs of andl */
      .WORD         0x0         0x0         0x0         0x1
      .WORD         0x0         0x0         0x1         0x0
      .WORD         0x0         0x1
      /* outputs of orl */
      .WORD         0x0         0x1         0x1         0x1
      .WORD         0x1         0x1         0x1         0x1
      .WORD         0x1         0x1
      /* outputs of eorl */
      .WORD         0x0         0x1         0x1         0x0
      .WORD         0x1         0x1         0x0         0x1
      .WORD         0x1         0x0
      .WORD  0x40440444       0x404  0x40040000    0x400400 /* expected flags */

      .WORD     0x30604
      .WORD  0x55555555  0x55555555 /* input 0 */
      .WORD  0x55555555 -0x55555556 /* input 1 */
      .WORD -0x55555556  0x55555555 /* input 2 */
      .WORD -0x55555556 -0x55555556 /* input 3 */
      /* outputs of andb */
      .WORD  0x55555555         0x0         0x0 -0x55555556
      .WORD  0x55555555        -0x1        -0x1 -0x55555556 /* outputs of orb */
      /* outputs of eorb */
      .WORD         0x0        -0x1        -0x1         0x0
      .WORD -0x777f7bc0      0x4884 /* expected flags */

      .WORD     0x1060c
      .WORD         0x0         0x0 /* input 0 */
      .WORD         0x0        -0x1 /* input 1 */
      .WORD        -0x1         0x0 /* input 2 */
      .WORD   0x69f6bc7         0x8 /* input 3 */
      .WORD  -0x69f6bc7         0x8 /* input 4 */
      .WORD  0x7fffffff         0x1 /* input 5 */
      .WORD -0x80000000         0x1 /* input 6 */
      .WORD -0x80000000        -0x1 /* input 7 */
      .WORD         0x1  0x7fffffff /* input 8 */
      .WORD         0x1 -0x80000000 /* input 9 */
      .WORD      0x8001      0x7fff /* input 10 */
      .WORD     0x20001      0x7fff /* input 11 */
      /* outputs of mul */
      .WORD         0x0         0x0         0x0  0x34fb5e38
      .WORD -0x34fb5e38  0x7fffffff -0x80000000 -0x80000000
      .WORD  0x7fffffff -0x80000000  0x3fffffff    -0x18001
      .WORD -0x57f7fbbc      0xa080 /* expected flags */

      .WORD     0x1060b
      .WORD         0x0        -0x1 /* input 0 */
      .WORD  0x34fb5e38         0x8 /* input 1 */
      .WORD -0x34fb5e38         0x8 /* input 2 */
      .WORD -0x80000000         0x2 /* input 3 */
      .WORD  0x40000000         0x2 /* input 4 */
      .WORD -0x80000000        -0x1 /* input 5 */
      .WORD  0x499602d2      0x1616 /* input 6 */
      .WORD -0x499602d2      0x1616 /* input 7 */
      .WORD  0x499602d2     -0x1616 /* input 8 */
      .WORD -0x499602d2     -0x1616 /* input 9 */
      .WORD -0x80000000  0x7fffffff /* input 10 */
      /* outputs of div */
      .WORD         0x0   0x69f6bc7  -0x69f6bc7 -0x40000000
      .WORD  0x20000000 -0x80000000     0x354f1    -0x354f1
      .WORD    -0x354f1     0x354f1        -0x1
      .WORD -0x7f5f77fc       0x808 /* expected flags */

      .WORD     0x40637
      .WORD         0x0        -0x1 /* input 0 */
      .WORD         0x0         0x0 /* input 1 */
      .WORD         0x0         0x1 /* input 2 */
      .WORD         0x0         0x2 /* input 3 */
      .WORD         0x0        0x10 /* input 4 */
      .WORD         0x0        0x1e /* input 5 */
      .WORD         0x0        0x1f /* input 6 */
      .WORD         0x0        0x20 /* input 7 */
      .WORD         0x0        0x21 /* input 8 */
      .WORD         0x0  0x7fffffff /* input 9 */
      .WORD         0x0   0x8000000 /* input 10 */
      .WORD  0x55555555        -0x1 /* input 11 */
      .WORD  0x55555555         0x0 /* input 12 */
      .WORD  0x55555555         0x1 /* input 13 */
      .WORD  0x55555555         0x2 /* input 14 */
      .WORD  0x55555555        0x10 /* input 15 */
      .WORD  0x55555555        0x1e /* input 16 */
      .WORD  0x55555555        0x1f /* input 17 */
      .WORD  0x55555555        0x20 /* input 18 */
      .WORD  0x55555555        0x21 /* input 19 */
      .WORD  0x55555555  0x7fffffff /* input 20 */
      .WORD  0x55555555   0x8000000 /* input 21 */
      .WORD  0x79fedcba        -0x1 /* input 22 */
      .WORD  0x79fedcba         0x0 /* input 23 */
      .WORD  0x79fedcba         0x1 /* input 24 */
      .WORD  0x79fedcba         0x2 /* input 25 */
      .WORD  0x79fedcba        0x10 /* input 26 */
      .WORD  0x79fedcba        0x1e /* input 27 */
      .WORD  0x79fedcba        0x1f /* input 28 */
      .WORD  0x79fedcba        0x20 /* input 29 */
      .WORD  0x79fedcba        0x21 /* input 30 */
      .WORD  0x79fedcba  0x7fffffff /* input 31 */
      .WORD  0x79fedcba   0x8000000 /* input 32 */
      .WORD -0x54321069        -0x1 /* input 33 */
      .WORD -0x54321069         0x0 /* input 34 */
      .WORD -0x54321069         0x1 /* input 35 */
      .WORD -0x54321069         0x2 /* input 36 */
      .WORD -0x54321069        0x10 /* input 37 */
      .WORD -0x54321069        0x1e /* input 38 */
      .WORD -0x54321069        0x1f /* input 39 */
      .WORD -0x54321069        0x20 /* input 40 */
      .WORD -0x54321069        0x21 /* input 41 */
      .WORD -0x54321069  0x7fffffff /* input 42 */
      .WORD -0x54321069   0x8000000 /* input 43 */
      .WORD        -0x1        -0x1 /* input 44 */
      .WORD        -0x1         0x0 /* input 45 */
      .WORD        -0x1         0x1 /* input 46 */
      .WORD        -0x1         0x2 /* input 47 */
      .WORD        -0x1        0x10 /* input 48 */
      .WORD        -0x1        0x1e /* input 49 */
      .WORD        -0x1        0x1f /* input 50 */
      .WORD        -0x1        0x20 /* input 51 */
      .WORD        -0x1        0x21 /* input 52 */
      .WORD        -0x1  0x7fffffff /* input 53 */
      .WORD        -0x1   0x8000000 /* input 54 */
      /* outputs of shr */
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0  0x55555555
      .WORD  0x55555555  0x2aaaaaaa  0x15555555      0x5555
      .WORD         0x1         0x0         0x0         0x0
      .WORD         0x0         0x0  0x79fedcba  0x79fedcba
      .WORD  0x3cff6e5d  0x1e7fb72e      0x79fe         0x1
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0 -0x54321069 -0x54321069 -0x2a190835
      .WORD -0x150c841b     -0x5433        -0x2        -0x1
      .WORD        -0x1        -0x1        -0x1        -0x1
      .WORD        -0x1        -0x1        -0x1        -0x1
      .WORD        -0x1        -0x1        -0x1        -0x1
      .WORD        -0x1        -0x1        -0x1
      /* outputs of shl */
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0  0x55555555
      .WORD  0x55555555 -0x55555556  0x55555554  0x55550000
      .WORD  0x40000000 -0x80000000         0x0         0x0
      .WORD         0x0         0x0  0x79fedcba  0x79fedcba
      .WORD  -0xc02468c -0x18048d18 -0x23460000 -0x80000000
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0 -0x54321069 -0x54321069  0x579bdf2e
      .WORD -0x50c841a4 -0x10690000 -0x40000000 -0x80000000
      .WORD         0x0         0x0         0x0         0x0
      .WORD        -0x1        -0x1        -0x2        -0x4
      .WORD    -0x10000 -0x40000000 -0x80000000         0x0
      .WORD         0x0         0x0         0x0
      /* outputs of rotl */
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0 -0x55555556
      .WORD  0x55555555 -0x55555556  0x55555555  0x55555555
      .WORD  0x55555555 -0x55555556  0x55555555 -0x55555556
      .WORD -0x55555556  0x55555555  0x3cff6e5d  0x79fedcba
      .WORD  -0xc02468c -0x18048d17 -0x23458602 -0x618048d2
      .WORD  0x3cff6e5d  0x79fedcba  -0xc02468c  0x3cff6e5d
      .WORD  0x79fedcba -0x2a190835 -0x54321069  0x579bdf2f
      .WORD -0x50c841a2 -0x10685433 -0x150c841b -0x2a190835
      .WORD -0x54321069  0x579bdf2f -0x2a190835 -0x54321069
      .WORD        -0x1        -0x1        -0x1        -0x1
      .WORD        -0x1        -0x1        -0x1        -0x1
      .WORD        -0x1        -0x1        -0x1
      /* outputs of rotr */
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0         0x0
      .WORD         0x0         0x0         0x0 -0x55555556
      .WORD  0x55555555 -0x55555556  0x55555555  0x55555555
      .WORD  0x55555555 -0x55555556  0x55555555 -0x55555556
      .WORD -0x55555556  0x55555555  -0xc02468c  0x79fedcba
      .WORD  0x3cff6e5d -0x618048d2 -0x23458602 -0x18048d17
      .WORD  -0xc02468c  0x79fedcba  0x3cff6e5d  -0xc02468c
      .WORD  0x79fedcba  0x579bdf2f -0x54321069 -0x2a190835
      .WORD -0x150c841b -0x10685433 -0x50c841a2  0x579bdf2f
      .WORD -0x54321069 -0x2a190835  0x579bdf2f -0x54321069
      .WORD        -0x1        -0x1        -0x1        -0x1
      .WORD        -0x1        -0x1        -0x1        -0x1
      .WORD        -0x1        -0x1        -0x1
      /* expected flags */
      .WORD  0x44444444    0x100444    0x555550  0x55551110
      .WORD -0x7666677b -0x66777778  0x49999999  0x44444444
      .WORD  0x11180044 -0x7ffbbba8  0x44445889  0x59998188
      .WORD -0x66677bbc  0x44444599  0x44444444 -0x7eee7f7c
      .WORD -0x67fef780 -0x6fe7fe78  0x18999818 -0x66667677
      .WORD  0x44489989  0x44444444   0x9000909 -0x66ff6f67
      .WORD -0x7ff6ff67   0x9809999 -0x66666768      0x8998

      .WORD     0x10506
      .WORD         0x0         0x0 /* input 0 */
      .WORD         0x0         0x1 /* input 1 */
      .WORD         0x0  0x7fffffff /* input 2 */
      .WORD         0x0 -0x80000000 /* input 3 */
      .WORD         0x0 -0x7fffffff /* input 4 */
      .WORD         0x0        -0x1 /* input 5 */
      /* outputs of neg */
      .WORD         0x0        -0x1 -0x7fffffff -0x80000000
      .WORD  0x7fffffff         0x1
      .WORD    0x11b994 /* expected flags */

      .WORD     0x10206
      .WORD         0x0         0x0 /* input 0 */
      .WORD         0x1         0x0 /* input 1 */
      .WORD  0x7fffffff         0x0 /* input 2 */
      .WORD -0x80000000         0x0 /* input 3 */
      .WORD -0x7fffffff         0x0 /* input 4 */
      .WORD        -0x1         0x0 /* input 5 */
      /* outputs of notl */
      .WORD         0x1         0x0         0x0         0x0
      .WORD         0x0         0x0
      .WORD    0x444440 /* expected flags */

      .WORD     0x10208
      .WORD         0x0         0x0 /* input 0 */
      .WORD         0x1         0x0 /* input 1 */
      .WORD  0x55555555         0x0 /* input 2 */
      .WORD  0x7fffffff         0x0 /* input 3 */
      .WORD -0x80000000         0x0 /* input 4 */
      .WORD -0x7fffffff         0x0 /* input 5 */
      .WORD -0x55555556         0x0 /* input 6 */
      .WORD        -0x1         0x0 /* input 7 */
      /* outputs of notb */
      .WORD        -0x1        -0x2 -0x55555556 -0x80000000
      .WORD  0x7fffffff  0x7ffffffe  0x55555555         0x0
      .WORD  0x40008888 /* expected flags */

      .WORD         0x0

      /*   Expected Behavior:
       *   Instr.     Inputs:                          Outputs:         
       *                  RS1         RS2  N Z V C           RD  N Z V C
       *      add         0xa        0x14  - - - -         0x1e  0 0 0 0
       *      add         0xa       -0x14  - - - -         -0xa  1 0 0 0
       *      add        -0xa        0x14  - - - -          0xa  0 0 0 1
       *      add        -0xa       -0x14  - - - -        -0x1e  1 0 0 1
       *      add  0x40000000  0x40000000  - - - -  -0x80000000  1 0 1 0
       *      add -0x40000000  0x40000000  - - - -          0x0  0 1 0 1
       *      add  0x40000000 -0x40000000  - - - -          0x0  0 1 0 1
       *      add -0x40000000 -0x40000000  - - - -  -0x80000000  1 0 0 1
       *      add  0x7fffffff         0x0  - - - -   0x7fffffff  0 0 0 0
       *      add  0x7fffffff         0x1  - - - -  -0x80000000  1 0 1 0
       *      add  0x7fffffff        -0x1  - - - -   0x7ffffffe  0 0 0 1
       *      add  0x7fffffff  0x7fffffff  - - - -         -0x2  1 0 1 0
       *      add  0x7fffffff -0x80000000  - - - -         -0x1  1 0 0 0
       *      add -0x80000000         0x0  - - - -  -0x80000000  1 0 0 0
       *      add -0x80000000         0x1  - - - -  -0x7fffffff  1 0 0 0
       *      add -0x80000000        -0x1  - - - -   0x7fffffff  0 0 1 1
       *      add -0x80000000 -0x80000000  - - - -          0x0  0 1 1 1
       *      add -0x80000000  0x7fffffff  - - - -         -0x1  1 0 0 0
       *      sub         0xa        0x14  - - - -         -0xa  1 0 0 1
       *      sub         0xa       -0x14  - - - -         0x1e  0 0 0 1
       *      sub        -0xa        0x14  - - - -        -0x1e  1 0 0 0
       *      sub        -0xa       -0x14  - - - -          0xa  0 0 0 0
       *      sub  0x40000000  0x40000000  - - - -          0x0  0 1 0 0
       *      sub -0x40000000  0x40000000  - - - -  -0x80000000  1 0 0 0
       *      sub  0x40000000 -0x40000000  - - - -  -0x80000000  1 0 1 1
       *      sub -0x40000000 -0x40000000  - - - -          0x0  0 1 0 0
       *      sub  0x7fffffff         0x0  - - - -   0x7fffffff  0 0 0 0
       *      sub  0x7fffffff         0x1  - - - -   0x7ffffffe  0 0 0 0
       *      sub  0x7fffffff        -0x1  - - - -  -0x80000000  1 0 1 1
       *      sub  0x7fffffff  0x7fffffff  - - - -          0x0  0 1 0 0
       *      sub  0x7fffffff -0x80000000  - - - -         -0x1  1 0 1 1
       *      sub -0x80000000         0x0  - - - -  -0x80000000  1 0 0 0
       *      sub -0x80000000         0x1  - - - -   0x7fffffff  0 0 1 0
       *      sub -0x80000000        -0x1  - - - -  -0x7fffffff  1 0 0 1
       *      sub -0x80000000 -0x80000000  - - - -          0x0  0 1 0 0
       *      sub -0x80000000  0x7fffffff  - - - -          0x1  0 0 1 0
       *     andl         0x0         0x0  - - - -          0x0  0 1 0 0
       *     andl         0x0         0x1  - - - -          0x0  0 1 0 0
       *     andl         0x1         0x0  - - - -          0x0  0 1 0 0
       *     andl         0x1         0x1  - - - -          0x1  0 0 0 0
       *     andl         0x0    0xbc614e  - - - -          0x0  0 1 0 0
       *     andl    0xbc614e         0x0  - - - -          0x0  0 1 0 0
       *     andl    0xbc614e    0xbc614e  - - - -          0x1  0 0 0 0
       *     andl         0x0   -0xbc614e  - - - -          0x0  0 1 0 0
       *     andl   -0xbc614e         0x0  - - - -          0x0  0 1 0 0
       *     andl   -0xbc614e   -0xbc614e  - - - -          0x1  0 0 0 0
       *      orl         0x0         0x0  - - - -          0x0  0 1 0 0
       *      orl         0x0         0x1  - - - -          0x1  0 0 0 0
       *      orl         0x1         0x0  - - - -          0x1  0 0 0 0
       *      orl         0x1         0x1  - - - -          0x1  0 0 0 0
       *      orl         0x0    0xbc614e  - - - -          0x1  0 0 0 0
       *      orl    0xbc614e         0x0  - - - -          0x1  0 0 0 0
       *      orl    0xbc614e    0xbc614e  - - - -          0x1  0 0 0 0
       *      orl         0x0   -0xbc614e  - - - -          0x1  0 0 0 0
       *      orl   -0xbc614e         0x0  - - - -          0x1  0 0 0 0
       *      orl   -0xbc614e   -0xbc614e  - - - -          0x1  0 0 0 0
       *     eorl         0x0         0x0  - - - -          0x0  0 1 0 0
       *     eorl         0x0         0x1  - - - -          0x1  0 0 0 0
       *     eorl         0x1         0x0  - - - -          0x1  0 0 0 0
       *     eorl         0x1         0x1  - - - -          0x0  0 1 0 0
       *     eorl         0x0    0xbc614e  - - - -          0x1  0 0 0 0
       *     eorl    0xbc614e         0x0  - - - -          0x1  0 0 0 0
       *     eorl    0xbc614e    0xbc614e  - - - -          0x0  0 1 0 0
       *     eorl         0x0   -0xbc614e  - - - -          0x1  0 0 0 0
       *     eorl   -0xbc614e         0x0  - - - -          0x1  0 0 0 0
       *     eorl   -0xbc614e   -0xbc614e  - - - -          0x0  0 1 0 0
       *     andb  0x55555555  0x55555555  - - - -   0x55555555  0 0 0 0
       *     andb  0x55555555  0xaaaaaaaa  - - - -          0x0  0 1 0 0
       *     andb  0xaaaaaaaa  0x55555555  - - - -          0x0  0 1 0 0
       *     andb  0xaaaaaaaa  0xaaaaaaaa  - - - -  -0x55555556  1 0 0 0
       *      orb  0x55555555  0x55555555  - - - -   0x55555555  0 0 0 0
       *      orb  0x55555555  0xaaaaaaaa  - - - -         -0x1  1 0 0 0
       *      orb  0xaaaaaaaa  0x55555555  - - - -         -0x1  1 0 0 0
       *      orb  0xaaaaaaaa  0xaaaaaaaa  - - - -  -0x55555556  1 0 0 0
       *     eorb  0x55555555  0x55555555  - - - -          0x0  0 1 0 0
       *     eorb  0x55555555  0xaaaaaaaa  - - - -         -0x1  1 0 0 0
       *     eorb  0xaaaaaaaa  0x55555555  - - - -         -0x1  1 0 0 0
       *     eorb  0xaaaaaaaa  0xaaaaaaaa  - - - -          0x0  0 1 0 0
       *      mul         0x0         0x0  - - - -          0x0  0 1 0 0
       *      mul         0x0        -0x1  - - - -          0x0  0 1 0 0
       *      mul        -0x1         0x0  - - - -          0x0  0 1 0 0
       *      mul   0x69f6bc7         0x8  - - - -   0x34fb5e38  0 0 0 0
       *      mul  -0x69f6bc7         0x8  - - - -  -0x34fb5e38  1 0 0 0
       *      mul  0x7fffffff         0x1  - - - -   0x7fffffff  0 0 0 0
       *      mul -0x80000000         0x1  - - - -  -0x80000000  1 0 0 0
       *      mul -0x80000000        -0x1  - - - -  -0x80000000  1 0 1 0
       *      mul         0x1  0x7fffffff  - - - -   0x7fffffff  0 0 0 0
       *      mul         0x1 -0x80000000  - - - -  -0x80000000  1 0 0 0
       *      mul      0x8001      0x7fff  - - - -   0x3fffffff  0 0 0 0
       *      mul     0x20001      0x7fff  - - - -     -0x18001  1 0 1 0
       *      div         0x0        -0x1  - - - -          0x0  0 1 0 0
       *      div  0x34fb5e38         0x8  - - - -    0x69f6bc7  0 0 0 0
       *      div -0x34fb5e38         0x8  - - - -   -0x69f6bc7  1 0 0 0
       *      div -0x80000000         0x2  - - - -  -0x40000000  1 0 0 0
       *      div  0x40000000         0x2  - - - -   0x20000000  0 0 0 0
       *      div -0x80000000        -0x1  - - - -  -0x80000000  1 0 1 0
       *      div  0x499602d2      0x1616  - - - -      0x354f1  0 0 0 0
       *      div -0x499602d2      0x1616  - - - -     -0x354f1  1 0 0 0
       *      div  0x499602d2     -0x1616  - - - -     -0x354f1  1 0 0 0
       *      div -0x499602d2     -0x1616  - - - -      0x354f1  0 0 0 0
       *      div -0x80000000  0x7fffffff  - - - -         -0x1  1 0 0 0
       *      shr         0x0        -0x1  - - - -          0x0  0 1 0 0
       *      shr         0x0         0x0  - - - -          0x0  0 1 0 0
       *      shr         0x0         0x1  - - - -          0x0  0 1 0 0
       *      shr         0x0         0x2  - - - -          0x0  0 1 0 0
       *      shr         0x0        0x10  - - - -          0x0  0 1 0 0
       *      shr         0x0        0x1e  - - - -          0x0  0 1 0 0
       *      shr         0x0        0x1f  - - - -          0x0  0 1 0 0
       *      shr         0x0        0x20  - - - -          0x0  0 1 0 0
       *      shr         0x0        0x21  - - - -          0x0  0 1 0 0
       *      shr         0x0  0x7fffffff  - - - -          0x0  0 1 0 0
       *      shr         0x0   0x8000000  - - - -          0x0  0 1 0 0
       *      shr  0x55555555        -0x1  - - - -   0x55555555  0 0 0 0
       *      shr  0x55555555         0x0  - - - -   0x55555555  0 0 0 0
       *      shr  0x55555555         0x1  - - - -   0x2aaaaaaa  0 0 0 1
       *      shr  0x55555555         0x2  - - - -   0x15555555  0 0 0 0
       *      shr  0x55555555        0x10  - - - -       0x5555  0 0 0 0
       *      shr  0x55555555        0x1e  - - - -          0x1  0 0 0 0
       *      shr  0x55555555        0x1f  - - - -          0x0  0 1 0 1
       *      shr  0x55555555        0x20  - - - -          0x0  0 1 0 1
       *      shr  0x55555555        0x21  - - - -          0x0  0 1 0 1
       *      shr  0x55555555  0x7fffffff  - - - -          0x0  0 1 0 1
       *      shr  0x55555555   0x8000000  - - - -          0x0  0 1 0 1
       *      shr  0x79fedcba        -0x1  - - - -   0x79fedcba  0 0 0 0
       *      shr  0x79fedcba         0x0  - - - -   0x79fedcba  0 0 0 0
       *      shr  0x79fedcba         0x1  - - - -   0x3cff6e5d  0 0 0 0
       *      shr  0x79fedcba         0x2  - - - -   0x1e7fb72e  0 0 0 1
       *      shr  0x79fedcba        0x10  - - - -       0x79fe  0 0 0 1
       *      shr  0x79fedcba        0x1e  - - - -          0x1  0 0 0 1
       *      shr  0x79fedcba        0x1f  - - - -          0x0  0 1 0 1
       *      shr  0x79fedcba        0x20  - - - -          0x0  0 1 0 1
       *      shr  0x79fedcba        0x21  - - - -          0x0  0 1 0 1
       *      shr  0x79fedcba  0x7fffffff  - - - -          0x0  0 1 0 1
       *      shr  0x79fedcba   0x8000000  - - - -          0x0  0 1 0 1
       *      shr  0xabcdef97        -0x1  - - - -  -0x54321069  1 0 0 0
       *      shr  0xabcdef97         0x0  - - - -  -0x54321069  1 0 0 0
       *      shr  0xabcdef97         0x1  - - - -  -0x2a190835  1 0 0 1
       *      shr  0xabcdef97         0x2  - - - -  -0x150c841b  1 0 0 1
       *      shr  0xabcdef97        0x10  - - - -      -0x5433  1 0 0 1
       *      shr  0xabcdef97        0x1e  - - - -         -0x2  1 0 0 1
       *      shr  0xabcdef97        0x1f  - - - -         -0x1  1 0 0 0
       *      shr  0xabcdef97        0x20  - - - -         -0x1  1 0 0 0
       *      shr  0xabcdef97        0x21  - - - -         -0x1  1 0 0 0
       *      shr  0xabcdef97  0x7fffffff  - - - -         -0x1  1 0 0 0
       *      shr  0xabcdef97   0x8000000  - - - -         -0x1  1 0 0 0
       *      shr        -0x1        -0x1  - - - -         -0x1  1 0 0 0
       *      shr        -0x1         0x0  - - - -         -0x1  1 0 0 0
       *      shr        -0x1         0x1  - - - -         -0x1  1 0 0 1
       *      shr        -0x1         0x2  - - - -         -0x1  1 0 0 1
       *      shr        -0x1        0x10  - - - -         -0x1  1 0 0 1
       *      shr        -0x1        0x1e  - - - -         -0x1  1 0 0 1
       *      shr        -0x1        0x1f  - - - -         -0x1  1 0 0 1
       *      shr        -0x1        0x20  - - - -         -0x1  1 0 0 1
       *      shr        -0x1        0x21  - - - -         -0x1  1 0 0 1
       *      shr        -0x1  0x7fffffff  - - - -         -0x1  1 0 0 1
       *      shr        -0x1   0x8000000  - - - -         -0x1  1 0 0 1
       *      shl         0x0        -0x1  - - - -          0x0  0 1 0 0
       *      shl         0x0         0x0  - - - -          0x0  0 1 0 0
       *      shl         0x0         0x1  - - - -          0x0  0 1 0 0
       *      shl         0x0         0x2  - - - -          0x0  0 1 0 0
       *      shl         0x0        0x10  - - - -          0x0  0 1 0 0
       *      shl         0x0        0x1e  - - - -          0x0  0 1 0 0
       *      shl         0x0        0x1f  - - - -          0x0  0 1 0 0
       *      shl         0x0        0x20  - - - -          0x0  0 1 0 0
       *      shl         0x0        0x21  - - - -          0x0  0 1 0 0
       *      shl         0x0  0x7fffffff  - - - -          0x0  0 1 0 0
       *      shl         0x0   0x8000000  - - - -          0x0  0 1 0 0
       *      shl  0x55555555        -0x1  - - - -   0x55555555  0 0 0 0
       *      shl  0x55555555         0x0  - - - -   0x55555555  0 0 0 0
       *      shl  0x55555555         0x1  - - - -  -0x55555556  1 0 0 0
       *      shl  0x55555555         0x2  - - - -   0x55555554  0 0 0 1
       *      shl  0x55555555        0x10  - - - -   0x55550000  0 0 0 1
       *      shl  0x55555555        0x1e  - - - -   0x40000000  0 0 0 1
       *      shl  0x55555555        0x1f  - - - -  -0x80000000  1 0 0 0
       *      shl  0x55555555        0x20  - - - -          0x0  0 1 0 1
       *      shl  0x55555555        0x21  - - - -          0x0  0 1 0 0
       *      shl  0x55555555  0x7fffffff  - - - -          0x0  0 1 0 0
       *      shl  0x55555555   0x8000000  - - - -          0x0  0 1 0 0
       *      shl  0x79fedcba        -0x1  - - - -   0x79fedcba  0 0 0 0
       *      shl  0x79fedcba         0x0  - - - -   0x79fedcba  0 0 0 0
       *      shl  0x79fedcba         0x1  - - - -   -0xc02468c  1 0 0 0
       *      shl  0x79fedcba         0x2  - - - -  -0x18048d18  1 0 0 1
       *      shl  0x79fedcba        0x10  - - - -  -0x23460000  1 0 0 0
       *      shl  0x79fedcba        0x1e  - - - -  -0x80000000  1 0 0 0
       *      shl  0x79fedcba        0x1f  - - - -          0x0  0 1 0 1
       *      shl  0x79fedcba        0x20  - - - -          0x0  0 1 0 0
       *      shl  0x79fedcba        0x21  - - - -          0x0  0 1 0 0
       *      shl  0x79fedcba  0x7fffffff  - - - -          0x0  0 1 0 0
       *      shl  0x79fedcba   0x8000000  - - - -          0x0  0 1 0 0
       *      shl  0xabcdef97        -0x1  - - - -  -0x54321069  1 0 0 0
       *      shl  0xabcdef97         0x0  - - - -  -0x54321069  1 0 0 0
       *      shl  0xabcdef97         0x1  - - - -   0x579bdf2e  0 0 0 1
       *      shl  0xabcdef97         0x2  - - - -  -0x50c841a4  1 0 0 0
       *      shl  0xabcdef97        0x10  - - - -  -0x10690000  1 0 0 1
       *      shl  0xabcdef97        0x1e  - - - -  -0x40000000  1 0 0 1
       *      shl  0xabcdef97        0x1f  - - - -  -0x80000000  1 0 0 1
       *      shl  0xabcdef97        0x20  - - - -          0x0  0 1 0 1
       *      shl  0xabcdef97        0x21  - - - -          0x0  0 1 0 0
       *      shl  0xabcdef97  0x7fffffff  - - - -          0x0  0 1 0 0
       *      shl  0xabcdef97   0x8000000  - - - -          0x0  0 1 0 0
       *      shl        -0x1        -0x1  - - - -         -0x1  1 0 0 0
       *      shl        -0x1         0x0  - - - -         -0x1  1 0 0 0
       *      shl        -0x1         0x1  - - - -         -0x2  1 0 0 1
       *      shl        -0x1         0x2  - - - -         -0x4  1 0 0 1
       *      shl        -0x1        0x10  - - - -     -0x10000  1 0 0 1
       *      shl        -0x1        0x1e  - - - -  -0x40000000  1 0 0 1
       *      shl        -0x1        0x1f  - - - -  -0x80000000  1 0 0 1
       *      shl        -0x1        0x20  - - - -          0x0  0 1 0 1
       *      shl        -0x1        0x21  - - - -          0x0  0 1 0 0
       *      shl        -0x1  0x7fffffff  - - - -          0x0  0 1 0 0
       *      shl        -0x1   0x8000000  - - - -          0x0  0 1 0 0
       *     rotl         0x0        -0x1  - - - -          0x0  0 1 0 0
       *     rotl         0x0         0x0  - - - -          0x0  0 1 0 0
       *     rotl         0x0         0x1  - - - -          0x0  0 1 0 0
       *     rotl         0x0         0x2  - - - -          0x0  0 1 0 0
       *     rotl         0x0        0x10  - - - -          0x0  0 1 0 0
       *     rotl         0x0        0x1e  - - - -          0x0  0 1 0 0
       *     rotl         0x0        0x1f  - - - -          0x0  0 1 0 0
       *     rotl         0x0        0x20  - - - -          0x0  0 1 0 0
       *     rotl         0x0        0x21  - - - -          0x0  0 1 0 0
       *     rotl         0x0  0x7fffffff  - - - -          0x0  0 1 0 0
       *     rotl         0x0   0x8000000  - - - -          0x0  0 1 0 0
       *     rotl  0x55555555        -0x1  - - - -  -0x55555556  1 0 0 0
       *     rotl  0x55555555         0x0  - - - -   0x55555555  0 0 0 0
       *     rotl  0x55555555         0x1  - - - -  -0x55555556  1 0 0 0
       *     rotl  0x55555555         0x2  - - - -   0x55555555  0 0 0 1
       *     rotl  0x55555555        0x10  - - - -   0x55555555  0 0 0 1
       *     rotl  0x55555555        0x1e  - - - -   0x55555555  0 0 0 1
       *     rotl  0x55555555        0x1f  - - - -  -0x55555556  1 0 0 0
       *     rotl  0x55555555        0x20  - - - -   0x55555555  0 0 0 0
       *     rotl  0x55555555        0x21  - - - -  -0x55555556  1 0 0 0
       *     rotl  0x55555555  0x7fffffff  - - - -  -0x55555556  1 0 0 0
       *     rotl  0x55555555   0x8000000  - - - -   0x55555555  0 0 0 0
       *     rotl  0x79fedcba        -0x1  - - - -   0x3cff6e5d  0 0 0 1
       *     rotl  0x79fedcba         0x0  - - - -   0x79fedcba  0 0 0 0
       *     rotl  0x79fedcba         0x1  - - - -   -0xc02468c  1 0 0 0
       *     rotl  0x79fedcba         0x2  - - - -  -0x18048d17  1 0 0 1
       *     rotl  0x79fedcba        0x10  - - - -  -0x23458602  1 0 0 0
       *     rotl  0x79fedcba        0x1e  - - - -  -0x618048d2  1 0 0 0
       *     rotl  0x79fedcba        0x1f  - - - -   0x3cff6e5d  0 0 0 1
       *     rotl  0x79fedcba        0x20  - - - -   0x79fedcba  0 0 0 0
       *     rotl  0x79fedcba        0x21  - - - -   -0xc02468c  1 0 0 0
       *     rotl  0x79fedcba  0x7fffffff  - - - -   0x3cff6e5d  0 0 0 1
       *     rotl  0x79fedcba   0x8000000  - - - -   0x79fedcba  0 0 0 0
       *     rotl  0xabcdef97        -0x1  - - - -  -0x2a190835  1 0 0 1
       *     rotl  0xabcdef97         0x0  - - - -  -0x54321069  1 0 0 0
       *     rotl  0xabcdef97         0x1  - - - -   0x579bdf2f  0 0 0 1
       *     rotl  0xabcdef97         0x2  - - - -  -0x50c841a2  1 0 0 0
       *     rotl  0xabcdef97        0x10  - - - -  -0x10685433  1 0 0 1
       *     rotl  0xabcdef97        0x1e  - - - -  -0x150c841b  1 0 0 1
       *     rotl  0xabcdef97        0x1f  - - - -  -0x2a190835  1 0 0 1
       *     rotl  0xabcdef97        0x20  - - - -  -0x54321069  1 0 0 0
       *     rotl  0xabcdef97        0x21  - - - -   0x579bdf2f  0 0 0 1
       *     rotl  0xabcdef97  0x7fffffff  - - - -  -0x2a190835  1 0 0 1
       *     rotl  0xabcdef97   0x8000000  - - - -  -0x54321069  1 0 0 0
       *     rotl        -0x1        -0x1  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1         0x0  - - - -         -0x1  1 0 0 0
       *     rotl        -0x1         0x1  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1         0x2  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1        0x10  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1        0x1e  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1        0x1f  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1        0x20  - - - -         -0x1  1 0 0 0
       *     rotl        -0x1        0x21  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1  0x7fffffff  - - - -         -0x1  1 0 0 1
       *     rotl        -0x1   0x8000000  - - - -         -0x1  1 0 0 0
       *     rotr         0x0        -0x1  - - - -          0x0  0 1 0 0
       *     rotr         0x0         0x0  - - - -          0x0  0 1 0 0
       *     rotr         0x0         0x1  - - - -          0x0  0 1 0 0
       *     rotr         0x0         0x2  - - - -          0x0  0 1 0 0
       *     rotr         0x0        0x10  - - - -          0x0  0 1 0 0
       *     rotr         0x0        0x1e  - - - -          0x0  0 1 0 0
       *     rotr         0x0        0x1f  - - - -          0x0  0 1 0 0
       *     rotr         0x0        0x20  - - - -          0x0  0 1 0 0
       *     rotr         0x0        0x21  - - - -          0x0  0 1 0 0
       *     rotr         0x0  0x7fffffff  - - - -          0x0  0 1 0 0
       *     rotr         0x0   0x8000000  - - - -          0x0  0 1 0 0
       *     rotr  0x55555555        -0x1  - - - -  -0x55555556  1 0 0 1
       *     rotr  0x55555555         0x0  - - - -   0x55555555  0 0 0 0
       *     rotr  0x55555555         0x1  - - - -  -0x55555556  1 0 0 1
       *     rotr  0x55555555         0x2  - - - -   0x55555555  0 0 0 0
       *     rotr  0x55555555        0x10  - - - -   0x55555555  0 0 0 0
       *     rotr  0x55555555        0x1e  - - - -   0x55555555  0 0 0 0
       *     rotr  0x55555555        0x1f  - - - -  -0x55555556  1 0 0 1
       *     rotr  0x55555555        0x20  - - - -   0x55555555  0 0 0 0
       *     rotr  0x55555555        0x21  - - - -  -0x55555556  1 0 0 1
       *     rotr  0x55555555  0x7fffffff  - - - -  -0x55555556  1 0 0 1
       *     rotr  0x55555555   0x8000000  - - - -   0x55555555  0 0 0 0
       *     rotr  0x79fedcba        -0x1  - - - -   -0xc02468c  1 0 0 1
       *     rotr  0x79fedcba         0x0  - - - -   0x79fedcba  0 0 0 0
       *     rotr  0x79fedcba         0x1  - - - -   0x3cff6e5d  0 0 0 0
       *     rotr  0x79fedcba         0x2  - - - -  -0x618048d2  1 0 0 1
       *     rotr  0x79fedcba        0x10  - - - -  -0x23458602  1 0 0 1
       *     rotr  0x79fedcba        0x1e  - - - -  -0x18048d17  1 0 0 1
       *     rotr  0x79fedcba        0x1f  - - - -   -0xc02468c  1 0 0 1
       *     rotr  0x79fedcba        0x20  - - - -   0x79fedcba  0 0 0 0
       *     rotr  0x79fedcba        0x21  - - - -   0x3cff6e5d  0 0 0 0
       *     rotr  0x79fedcba  0x7fffffff  - - - -   -0xc02468c  1 0 0 1
       *     rotr  0x79fedcba   0x8000000  - - - -   0x79fedcba  0 0 0 0
       *     rotr  0xabcdef97        -0x1  - - - -   0x579bdf2f  0 0 0 0
       *     rotr  0xabcdef97         0x0  - - - -  -0x54321069  1 0 0 0
       *     rotr  0xabcdef97         0x1  - - - -  -0x2a190835  1 0 0 1
       *     rotr  0xabcdef97         0x2  - - - -  -0x150c841b  1 0 0 1
       *     rotr  0xabcdef97        0x10  - - - -  -0x10685433  1 0 0 1
       *     rotr  0xabcdef97        0x1e  - - - -  -0x50c841a2  1 0 0 1
       *     rotr  0xabcdef97        0x1f  - - - -   0x579bdf2f  0 0 0 0
       *     rotr  0xabcdef97        0x20  - - - -  -0x54321069  1 0 0 0
       *     rotr  0xabcdef97        0x21  - - - -  -0x2a190835  1 0 0 1
       *     rotr  0xabcdef97  0x7fffffff  - - - -   0x579bdf2f  0 0 0 0
       *     rotr  0xabcdef97   0x8000000  - - - -  -0x54321069  1 0 0 0
       *     rotr        -0x1        -0x1  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1         0x0  - - - -         -0x1  1 0 0 0
       *     rotr        -0x1         0x1  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1         0x2  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1        0x10  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1        0x1e  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1        0x1f  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1        0x20  - - - -         -0x1  1 0 0 0
       *     rotr        -0x1        0x21  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1  0x7fffffff  - - - -         -0x1  1 0 0 1
       *     rotr        -0x1   0x8000000  - - - -         -0x1  1 0 0 0
       *      neg         0x0         0x0  - - - -          0x0  0 1 0 0
       *      neg         0x0         0x1  - - - -         -0x1  1 0 0 1
       *      neg         0x0  0x7fffffff  - - - -  -0x7fffffff  1 0 0 1
       *      neg         0x0 -0x80000000  - - - -  -0x80000000  1 0 1 1
       *      neg         0x0 -0x7fffffff  - - - -   0x7fffffff  0 0 0 1
       *      neg         0x0        -0x1  - - - -          0x1  0 0 0 1
       *     notl         0x0         0x0  - - - -          0x1  0 0 0 0
       *     notl         0x1         0x0  - - - -          0x0  0 1 0 0
       *     notl  0x7fffffff         0x0  - - - -          0x0  0 1 0 0
       *     notl -0x80000000         0x0  - - - -          0x0  0 1 0 0
       *     notl -0x7fffffff         0x0  - - - -          0x0  0 1 0 0
       *     notl        -0x1         0x0  - - - -          0x0  0 1 0 0
       *     notb         0x0         0x0  - - - -   0xffffffff  1 0 0 0
       *     notb         0x1         0x0  - - - -   0xfffffffe  1 0 0 0
       *     notb  0x55555555         0x0  - - - -   0xaaaaaaaa  1 0 0 0
       *     notb  0x7fffffff         0x0  - - - -   0x80000000  1 0 0 0
       *     notb -0x80000000         0x0  - - - -   0x7fffffff  0 0 0 0
       *     notb -0x7fffffff         0x0  - - - -   0x7ffffffe  0 0 0 0
       *     notb -0x55555556         0x0  - - - -   0x55555555  0 0 0 0
       *     notb        -0x1         0x0  - - - -          0x0  0 1 0 0
       */

TestBranchAndCondData:
      .WORD       0x160 /* number of tests */
      /* flags */
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD  0x76543210  -0x1234568  0x76543210  -0x1234568
      .WORD      0x4444      0x4444  0x44004400    0x440044
      .WORD  0x44444400  0x44440044        0x44      0x4400
      .WORD    0x440044  0x44004400  0x44440000  0x44440000
      /* rd or branch taken */
      .WORD -0x33cc0f10      0xffff  -0x505fafb  -0xc03f3fd
      .WORD   0xf0f33cc -0x5555aaab -0x3333cccd   -0xffff01
      .WORD -0x33cc0f10  -0xc03f3fd   0xf0f33cc

      /*   Expected Behavior:
       *   Instr.     Inputs:                          Outputs:         
       *                  RS1         RS2  N Z V C           RD  N Z V C
       *      beq           -           -  0 0 0 0          0x0  0 0 0 0
       *      beq           -           -  0 0 0 1          0x0  0 0 0 1
       *      beq           -           -  0 0 1 0          0x0  0 0 1 0
       *      beq           -           -  0 0 1 1          0x0  0 0 1 1
       *      beq           -           -  0 1 0 0          0x1  0 1 0 0
       *      beq           -           -  0 1 0 1          0x1  0 1 0 1
       *      beq           -           -  0 1 1 0          0x1  0 1 1 0
       *      beq           -           -  0 1 1 1          0x1  0 1 1 1
       *      beq           -           -  1 0 0 0          0x0  1 0 0 0
       *      beq           -           -  1 0 0 1          0x0  1 0 0 1
       *      beq           -           -  1 0 1 0          0x0  1 0 1 0
       *      beq           -           -  1 0 1 1          0x0  1 0 1 1
       *      beq           -           -  1 1 0 0          0x1  1 1 0 0
       *      beq           -           -  1 1 0 1          0x1  1 1 0 1
       *      beq           -           -  1 1 1 0          0x1  1 1 1 0
       *      beq           -           -  1 1 1 1          0x1  1 1 1 1
       *      bge           -           -  0 0 0 0          0x1  0 0 0 0
       *      bge           -           -  0 0 0 1          0x1  0 0 0 1
       *      bge           -           -  0 0 1 0          0x0  0 0 1 0
       *      bge           -           -  0 0 1 1          0x0  0 0 1 1
       *      bge           -           -  0 1 0 0          0x1  0 1 0 0
       *      bge           -           -  0 1 0 1          0x1  0 1 0 1
       *      bge           -           -  0 1 1 0          0x0  0 1 1 0
       *      bge           -           -  0 1 1 1          0x0  0 1 1 1
       *      bge           -           -  1 0 0 0          0x0  1 0 0 0
       *      bge           -           -  1 0 0 1          0x0  1 0 0 1
       *      bge           -           -  1 0 1 0          0x1  1 0 1 0
       *      bge           -           -  1 0 1 1          0x1  1 0 1 1
       *      bge           -           -  1 1 0 0          0x0  1 1 0 0
       *      bge           -           -  1 1 0 1          0x0  1 1 0 1
       *      bge           -           -  1 1 1 0          0x1  1 1 1 0
       *      bge           -           -  1 1 1 1          0x1  1 1 1 1
       *       bt           -           -  0 0 0 0          0x1  0 0 0 0
       *       bt           -           -  0 0 0 1          0x1  0 0 0 1
       *       bt           -           -  0 0 1 0          0x1  0 0 1 0
       *       bt           -           -  0 0 1 1          0x1  0 0 1 1
       *       bt           -           -  0 1 0 0          0x1  0 1 0 0
       *       bt           -           -  0 1 0 1          0x1  0 1 0 1
       *       bt           -           -  0 1 1 0          0x1  0 1 1 0
       *       bt           -           -  0 1 1 1          0x1  0 1 1 1
       *       bt           -           -  1 0 0 0          0x1  1 0 0 0
       *       bt           -           -  1 0 0 1          0x1  1 0 0 1
       *       bt           -           -  1 0 1 0          0x1  1 0 1 0
       *       bt           -           -  1 0 1 1          0x1  1 0 1 1
       *       bt           -           -  1 1 0 0          0x1  1 1 0 0
       *       bt           -           -  1 1 0 1          0x1  1 1 0 1
       *       bt           -           -  1 1 1 0          0x1  1 1 1 0
       *       bt           -           -  1 1 1 1          0x1  1 1 1 1
       *       bf           -           -  0 0 0 0          0x0  0 0 0 0
       *       bf           -           -  0 0 0 1          0x0  0 0 0 1
       *       bf           -           -  0 0 1 0          0x0  0 0 1 0
       *       bf           -           -  0 0 1 1          0x0  0 0 1 1
       *       bf           -           -  0 1 0 0          0x0  0 1 0 0
       *       bf           -           -  0 1 0 1          0x0  0 1 0 1
       *       bf           -           -  0 1 1 0          0x0  0 1 1 0
       *       bf           -           -  0 1 1 1          0x0  0 1 1 1
       *       bf           -           -  1 0 0 0          0x0  1 0 0 0
       *       bf           -           -  1 0 0 1          0x0  1 0 0 1
       *       bf           -           -  1 0 1 0          0x0  1 0 1 0
       *       bf           -           -  1 0 1 1          0x0  1 0 1 1
       *       bf           -           -  1 1 0 0          0x0  1 1 0 0
       *       bf           -           -  1 1 0 1          0x0  1 1 0 1
       *       bf           -           -  1 1 1 0          0x0  1 1 1 0
       *       bf           -           -  1 1 1 1          0x0  1 1 1 1
       *      bhi           -           -  0 0 0 0          0x1  0 0 0 0
       *      bhi           -           -  0 0 0 1          0x0  0 0 0 1
       *      bhi           -           -  0 0 1 0          0x1  0 0 1 0
       *      bhi           -           -  0 0 1 1          0x0  0 0 1 1
       *      bhi           -           -  0 1 0 0          0x0  0 1 0 0
       *      bhi           -           -  0 1 0 1          0x0  0 1 0 1
       *      bhi           -           -  0 1 1 0          0x0  0 1 1 0
       *      bhi           -           -  0 1 1 1          0x0  0 1 1 1
       *      bhi           -           -  1 0 0 0          0x1  1 0 0 0
       *      bhi           -           -  1 0 0 1          0x0  1 0 0 1
       *      bhi           -           -  1 0 1 0          0x1  1 0 1 0
       *      bhi           -           -  1 0 1 1          0x0  1 0 1 1
       *      bhi           -           -  1 1 0 0          0x0  1 1 0 0
       *      bhi           -           -  1 1 0 1          0x0  1 1 0 1
       *      bhi           -           -  1 1 1 0          0x0  1 1 1 0
       *      bhi           -           -  1 1 1 1          0x0  1 1 1 1
       *      bls           -           -  0 0 0 0          0x0  0 0 0 0
       *      bls           -           -  0 0 0 1          0x1  0 0 0 1
       *      bls           -           -  0 0 1 0          0x0  0 0 1 0
       *      bls           -           -  0 0 1 1          0x1  0 0 1 1
       *      bls           -           -  0 1 0 0          0x1  0 1 0 0
       *      bls           -           -  0 1 0 1          0x1  0 1 0 1
       *      bls           -           -  0 1 1 0          0x1  0 1 1 0
       *      bls           -           -  0 1 1 1          0x1  0 1 1 1
       *      bls           -           -  1 0 0 0          0x0  1 0 0 0
       *      bls           -           -  1 0 0 1          0x1  1 0 0 1
       *      bls           -           -  1 0 1 0          0x0  1 0 1 0
       *      bls           -           -  1 0 1 1          0x1  1 0 1 1
       *      bls           -           -  1 1 0 0          0x1  1 1 0 0
       *      bls           -           -  1 1 0 1          0x1  1 1 0 1
       *      bls           -           -  1 1 1 0          0x1  1 1 1 0
       *      bls           -           -  1 1 1 1          0x1  1 1 1 1
       *      bgt           -           -  0 0 0 0          0x1  0 0 0 0
       *      bgt           -           -  0 0 0 1          0x1  0 0 0 1
       *      bgt           -           -  0 0 1 0          0x0  0 0 1 0
       *      bgt           -           -  0 0 1 1          0x0  0 0 1 1
       *      bgt           -           -  0 1 0 0          0x0  0 1 0 0
       *      bgt           -           -  0 1 0 1          0x0  0 1 0 1
       *      bgt           -           -  0 1 1 0          0x0  0 1 1 0
       *      bgt           -           -  0 1 1 1          0x0  0 1 1 1
       *      bgt           -           -  1 0 0 0          0x0  1 0 0 0
       *      bgt           -           -  1 0 0 1          0x0  1 0 0 1
       *      bgt           -           -  1 0 1 0          0x1  1 0 1 0
       *      bgt           -           -  1 0 1 1          0x1  1 0 1 1
       *      bgt           -           -  1 1 0 0          0x0  1 1 0 0
       *      bgt           -           -  1 1 0 1          0x0  1 1 0 1
       *      bgt           -           -  1 1 1 0          0x0  1 1 1 0
       *      bgt           -           -  1 1 1 1          0x0  1 1 1 1
       *      ble           -           -  0 0 0 0          0x0  0 0 0 0
       *      ble           -           -  0 0 0 1          0x0  0 0 0 1
       *      ble           -           -  0 0 1 0          0x1  0 0 1 0
       *      ble           -           -  0 0 1 1          0x1  0 0 1 1
       *      ble           -           -  0 1 0 0          0x1  0 1 0 0
       *      ble           -           -  0 1 0 1          0x1  0 1 0 1
       *      ble           -           -  0 1 1 0          0x1  0 1 1 0
       *      ble           -           -  0 1 1 1          0x1  0 1 1 1
       *      ble           -           -  1 0 0 0          0x1  1 0 0 0
       *      ble           -           -  1 0 0 1          0x1  1 0 0 1
       *      ble           -           -  1 0 1 0          0x0  1 0 1 0
       *      ble           -           -  1 0 1 1          0x0  1 0 1 1
       *      ble           -           -  1 1 0 0          0x1  1 1 0 0
       *      ble           -           -  1 1 0 1          0x1  1 1 0 1
       *      ble           -           -  1 1 1 0          0x1  1 1 1 0
       *      ble           -           -  1 1 1 1          0x1  1 1 1 1
       *      blt           -           -  0 0 0 0          0x0  0 0 0 0
       *      blt           -           -  0 0 0 1          0x0  0 0 0 1
       *      blt           -           -  0 0 1 0          0x1  0 0 1 0
       *      blt           -           -  0 0 1 1          0x1  0 0 1 1
       *      blt           -           -  0 1 0 0          0x0  0 1 0 0
       *      blt           -           -  0 1 0 1          0x0  0 1 0 1
       *      blt           -           -  0 1 1 0          0x1  0 1 1 0
       *      blt           -           -  0 1 1 1          0x1  0 1 1 1
       *      blt           -           -  1 0 0 0          0x1  1 0 0 0
       *      blt           -           -  1 0 0 1          0x1  1 0 0 1
       *      blt           -           -  1 0 1 0          0x0  1 0 1 0
       *      blt           -           -  1 0 1 1          0x0  1 0 1 1
       *      blt           -           -  1 1 0 0          0x1  1 1 0 0
       *      blt           -           -  1 1 0 1          0x1  1 1 0 1
       *      blt           -           -  1 1 1 0          0x0  1 1 1 0
       *      blt           -           -  1 1 1 1          0x0  1 1 1 1
       *      bne           -           -  0 0 0 0          0x1  0 0 0 0
       *      bne           -           -  0 0 0 1          0x1  0 0 0 1
       *      bne           -           -  0 0 1 0          0x1  0 0 1 0
       *      bne           -           -  0 0 1 1          0x1  0 0 1 1
       *      bne           -           -  0 1 0 0          0x0  0 1 0 0
       *      bne           -           -  0 1 0 1          0x0  0 1 0 1
       *      bne           -           -  0 1 1 0          0x0  0 1 1 0
       *      bne           -           -  0 1 1 1          0x0  0 1 1 1
       *      bne           -           -  1 0 0 0          0x1  1 0 0 0
       *      bne           -           -  1 0 0 1          0x1  1 0 0 1
       *      bne           -           -  1 0 1 0          0x1  1 0 1 0
       *      bne           -           -  1 0 1 1          0x1  1 0 1 1
       *      bne           -           -  1 1 0 0          0x0  1 1 0 0
       *      bne           -           -  1 1 0 1          0x0  1 1 0 1
       *      bne           -           -  1 1 1 0          0x0  1 1 1 0
       *      bne           -           -  1 1 1 1          0x0  1 1 1 1
       *      bcc           -           -  0 0 0 0          0x1  0 0 0 0
       *      bcc           -           -  0 0 0 1          0x0  0 0 0 1
       *      bcc           -           -  0 0 1 0          0x1  0 0 1 0
       *      bcc           -           -  0 0 1 1          0x0  0 0 1 1
       *      bcc           -           -  0 1 0 0          0x1  0 1 0 0
       *      bcc           -           -  0 1 0 1          0x0  0 1 0 1
       *      bcc           -           -  0 1 1 0          0x1  0 1 1 0
       *      bcc           -           -  0 1 1 1          0x0  0 1 1 1
       *      bcc           -           -  1 0 0 0          0x1  1 0 0 0
       *      bcc           -           -  1 0 0 1          0x0  1 0 0 1
       *      bcc           -           -  1 0 1 0          0x1  1 0 1 0
       *      bcc           -           -  1 0 1 1          0x0  1 0 1 1
       *      bcc           -           -  1 1 0 0          0x1  1 1 0 0
       *      bcc           -           -  1 1 0 1          0x0  1 1 0 1
       *      bcc           -           -  1 1 1 0          0x1  1 1 1 0
       *      bcc           -           -  1 1 1 1          0x0  1 1 1 1
       *      bcs           -           -  0 0 0 0          0x0  0 0 0 0
       *      bcs           -           -  0 0 0 1          0x1  0 0 0 1
       *      bcs           -           -  0 0 1 0          0x0  0 0 1 0
       *      bcs           -           -  0 0 1 1          0x1  0 0 1 1
       *      bcs           -           -  0 1 0 0          0x0  0 1 0 0
       *      bcs           -           -  0 1 0 1          0x1  0 1 0 1
       *      bcs           -           -  0 1 1 0          0x0  0 1 1 0
       *      bcs           -           -  0 1 1 1          0x1  0 1 1 1
       *      bcs           -           -  1 0 0 0          0x0  1 0 0 0
       *      bcs           -           -  1 0 0 1          0x1  1 0 0 1
       *      bcs           -           -  1 0 1 0          0x0  1 0 1 0
       *      bcs           -           -  1 0 1 1          0x1  1 0 1 1
       *      bcs           -           -  1 1 0 0          0x0  1 1 0 0
       *      bcs           -           -  1 1 0 1          0x1  1 1 0 1
       *      bcs           -           -  1 1 1 0          0x0  1 1 1 0
       *      bcs           -           -  1 1 1 1          0x1  1 1 1 1
       *      bvc           -           -  0 0 0 0          0x1  0 0 0 0
       *      bvc           -           -  0 0 0 1          0x1  0 0 0 1
       *      bvc           -           -  0 0 1 0          0x0  0 0 1 0
       *      bvc           -           -  0 0 1 1          0x0  0 0 1 1
       *      bvc           -           -  0 1 0 0          0x1  0 1 0 0
       *      bvc           -           -  0 1 0 1          0x1  0 1 0 1
       *      bvc           -           -  0 1 1 0          0x0  0 1 1 0
       *      bvc           -           -  0 1 1 1          0x0  0 1 1 1
       *      bvc           -           -  1 0 0 0          0x1  1 0 0 0
       *      bvc           -           -  1 0 0 1          0x1  1 0 0 1
       *      bvc           -           -  1 0 1 0          0x0  1 0 1 0
       *      bvc           -           -  1 0 1 1          0x0  1 0 1 1
       *      bvc           -           -  1 1 0 0          0x1  1 1 0 0
       *      bvc           -           -  1 1 0 1          0x1  1 1 0 1
       *      bvc           -           -  1 1 1 0          0x0  1 1 1 0
       *      bvc           -           -  1 1 1 1          0x0  1 1 1 1
       *      bvs           -           -  0 0 0 0          0x0  0 0 0 0
       *      bvs           -           -  0 0 0 1          0x0  0 0 0 1
       *      bvs           -           -  0 0 1 0          0x1  0 0 1 0
       *      bvs           -           -  0 0 1 1          0x1  0 0 1 1
       *      bvs           -           -  0 1 0 0          0x0  0 1 0 0
       *      bvs           -           -  0 1 0 1          0x0  0 1 0 1
       *      bvs           -           -  0 1 1 0          0x1  0 1 1 0
       *      bvs           -           -  0 1 1 1          0x1  0 1 1 1
       *      bvs           -           -  1 0 0 0          0x0  1 0 0 0
       *      bvs           -           -  1 0 0 1          0x0  1 0 0 1
       *      bvs           -           -  1 0 1 0          0x1  1 0 1 0
       *      bvs           -           -  1 0 1 1          0x1  1 0 1 1
       *      bvs           -           -  1 1 0 0          0x0  1 1 0 0
       *      bvs           -           -  1 1 0 1          0x0  1 1 0 1
       *      bvs           -           -  1 1 1 0          0x1  1 1 1 0
       *      bvs           -           -  1 1 1 1          0x1  1 1 1 1
       *      bpl           -           -  0 0 0 0          0x1  0 0 0 0
       *      bpl           -           -  0 0 0 1          0x1  0 0 0 1
       *      bpl           -           -  0 0 1 0          0x1  0 0 1 0
       *      bpl           -           -  0 0 1 1          0x1  0 0 1 1
       *      bpl           -           -  0 1 0 0          0x1  0 1 0 0
       *      bpl           -           -  0 1 0 1          0x1  0 1 0 1
       *      bpl           -           -  0 1 1 0          0x1  0 1 1 0
       *      bpl           -           -  0 1 1 1          0x1  0 1 1 1
       *      bpl           -           -  1 0 0 0          0x0  1 0 0 0
       *      bpl           -           -  1 0 0 1          0x0  1 0 0 1
       *      bpl           -           -  1 0 1 0          0x0  1 0 1 0
       *      bpl           -           -  1 0 1 1          0x0  1 0 1 1
       *      bpl           -           -  1 1 0 0          0x0  1 1 0 0
       *      bpl           -           -  1 1 0 1          0x0  1 1 0 1
       *      bpl           -           -  1 1 1 0          0x0  1 1 1 0
       *      bpl           -           -  1 1 1 1          0x0  1 1 1 1
       *      bmi           -           -  0 0 0 0          0x0  0 0 0 0
       *      bmi           -           -  0 0 0 1          0x0  0 0 0 1
       *      bmi           -           -  0 0 1 0          0x0  0 0 1 0
       *      bmi           -           -  0 0 1 1          0x0  0 0 1 1
       *      bmi           -           -  0 1 0 0          0x0  0 1 0 0
       *      bmi           -           -  0 1 0 1          0x0  0 1 0 1
       *      bmi           -           -  0 1 1 0          0x0  0 1 1 0
       *      bmi           -           -  0 1 1 1          0x0  0 1 1 1
       *      bmi           -           -  1 0 0 0          0x1  1 0 0 0
       *      bmi           -           -  1 0 0 1          0x1  1 0 0 1
       *      bmi           -           -  1 0 1 0          0x1  1 0 1 0
       *      bmi           -           -  1 0 1 1          0x1  1 0 1 1
       *      bmi           -           -  1 1 0 0          0x1  1 1 0 0
       *      bmi           -           -  1 1 0 1          0x1  1 1 0 1
       *      bmi           -           -  1 1 1 0          0x1  1 1 1 0
       *      bmi           -           -  1 1 1 1          0x1  1 1 1 1
       *      seq           -           -  0 0 0 0          0x0  0 1 0 0
       *      seq           -           -  0 0 0 1          0x0  0 1 0 0
       *      seq           -           -  0 0 1 0          0x0  0 1 0 0
       *      seq           -           -  0 0 1 1          0x0  0 1 0 0
       *      seq           -           -  0 1 0 0          0x1  0 0 0 0
       *      seq           -           -  0 1 0 1          0x1  0 0 0 0
       *      seq           -           -  0 1 1 0          0x1  0 0 0 0
       *      seq           -           -  0 1 1 1          0x1  0 0 0 0
       *      seq           -           -  1 0 0 0          0x0  0 1 0 0
       *      seq           -           -  1 0 0 1          0x0  0 1 0 0
       *      seq           -           -  1 0 1 0          0x0  0 1 0 0
       *      seq           -           -  1 0 1 1          0x0  0 1 0 0
       *      seq           -           -  1 1 0 0          0x1  0 0 0 0
       *      seq           -           -  1 1 0 1          0x1  0 0 0 0
       *      seq           -           -  1 1 1 0          0x1  0 0 0 0
       *      seq           -           -  1 1 1 1          0x1  0 0 0 0
       *      sge           -           -  0 0 0 0          0x1  0 0 0 0
       *      sge           -           -  0 0 0 1          0x1  0 0 0 0
       *      sge           -           -  0 0 1 0          0x0  0 1 0 0
       *      sge           -           -  0 0 1 1          0x0  0 1 0 0
       *      sge           -           -  0 1 0 0          0x1  0 0 0 0
       *      sge           -           -  0 1 0 1          0x1  0 0 0 0
       *      sge           -           -  0 1 1 0          0x0  0 1 0 0
       *      sge           -           -  0 1 1 1          0x0  0 1 0 0
       *      sge           -           -  1 0 0 0          0x0  0 1 0 0
       *      sge           -           -  1 0 0 1          0x0  0 1 0 0
       *      sge           -           -  1 0 1 0          0x1  0 0 0 0
       *      sge           -           -  1 0 1 1          0x1  0 0 0 0
       *      sge           -           -  1 1 0 0          0x0  0 1 0 0
       *      sge           -           -  1 1 0 1          0x0  0 1 0 0
       *      sge           -           -  1 1 1 0          0x1  0 0 0 0
       *      sge           -           -  1 1 1 1          0x1  0 0 0 0
       *      sgt           -           -  0 0 0 0          0x1  0 0 0 0
       *      sgt           -           -  0 0 0 1          0x1  0 0 0 0
       *      sgt           -           -  0 0 1 0          0x0  0 1 0 0
       *      sgt           -           -  0 0 1 1          0x0  0 1 0 0
       *      sgt           -           -  0 1 0 0          0x0  0 1 0 0
       *      sgt           -           -  0 1 0 1          0x0  0 1 0 0
       *      sgt           -           -  0 1 1 0          0x0  0 1 0 0
       *      sgt           -           -  0 1 1 1          0x0  0 1 0 0
       *      sgt           -           -  1 0 0 0          0x0  0 1 0 0
       *      sgt           -           -  1 0 0 1          0x0  0 1 0 0
       *      sgt           -           -  1 0 1 0          0x1  0 0 0 0
       *      sgt           -           -  1 0 1 1          0x1  0 0 0 0
       *      sgt           -           -  1 1 0 0          0x0  0 1 0 0
       *      sgt           -           -  1 1 0 1          0x0  0 1 0 0
       *      sgt           -           -  1 1 1 0          0x0  0 1 0 0
       *      sgt           -           -  1 1 1 1          0x0  0 1 0 0
       *      sle           -           -  0 0 0 0          0x0  0 1 0 0
       *      sle           -           -  0 0 0 1          0x0  0 1 0 0
       *      sle           -           -  0 0 1 0          0x1  0 0 0 0
       *      sle           -           -  0 0 1 1          0x1  0 0 0 0
       *      sle           -           -  0 1 0 0          0x1  0 0 0 0
       *      sle           -           -  0 1 0 1          0x1  0 0 0 0
       *      sle           -           -  0 1 1 0          0x1  0 0 0 0
       *      sle           -           -  0 1 1 1          0x1  0 0 0 0
       *      sle           -           -  1 0 0 0          0x1  0 0 0 0
       *      sle           -           -  1 0 0 1          0x1  0 0 0 0
       *      sle           -           -  1 0 1 0          0x0  0 1 0 0
       *      sle           -           -  1 0 1 1          0x0  0 1 0 0
       *      sle           -           -  1 1 0 0          0x1  0 0 0 0
       *      sle           -           -  1 1 0 1          0x1  0 0 0 0
       *      sle           -           -  1 1 1 0          0x1  0 0 0 0
       *      sle           -           -  1 1 1 1          0x1  0 0 0 0
       *      slt           -           -  0 0 0 0          0x0  0 1 0 0
       *      slt           -           -  0 0 0 1          0x0  0 1 0 0
       *      slt           -           -  0 0 1 0          0x1  0 0 0 0
       *      slt           -           -  0 0 1 1          0x1  0 0 0 0
       *      slt           -           -  0 1 0 0          0x0  0 1 0 0
       *      slt           -           -  0 1 0 1          0x0  0 1 0 0
       *      slt           -           -  0 1 1 0          0x1  0 0 0 0
       *      slt           -           -  0 1 1 1          0x1  0 0 0 0
       *      slt           -           -  1 0 0 0          0x1  0 0 0 0
       *      slt           -           -  1 0 0 1          0x1  0 0 0 0
       *      slt           -           -  1 0 1 0          0x0  0 1 0 0
       *      slt           -           -  1 0 1 1          0x0  0 1 0 0
       *      slt           -           -  1 1 0 0          0x1  0 0 0 0
       *      slt           -           -  1 1 0 1          0x1  0 0 0 0
       *      slt           -           -  1 1 1 0          0x0  0 1 0 0
       *      slt           -           -  1 1 1 1          0x0  0 1 0 0
       *      sne           -           -  0 0 0 0          0x1  0 0 0 0
       *      sne           -           -  0 0 0 1          0x1  0 0 0 0
       *      sne           -           -  0 0 1 0          0x1  0 0 0 0
       *      sne           -           -  0 0 1 1          0x1  0 0 0 0
       *      sne           -           -  0 1 0 0          0x0  0 1 0 0
       *      sne           -           -  0 1 0 1          0x0  0 1 0 0
       *      sne           -           -  0 1 1 0          0x0  0 1 0 0
       *      sne           -           -  0 1 1 1          0x0  0 1 0 0
       *      sne           -           -  1 0 0 0          0x1  0 0 0 0
       *      sne           -           -  1 0 0 1          0x1  0 0 0 0
       *      sne           -           -  1 0 1 0          0x1  0 0 0 0
       *      sne           -           -  1 0 1 1          0x1  0 0 0 0
       *      sne           -           -  1 1 0 0          0x0  0 1 0 0
       *      sne           -           -  1 1 0 1          0x0  0 1 0 0
       *      sne           -           -  1 1 1 0          0x0  0 1 0 0
       *      sne           -           -  1 1 1 1          0x0  0 1 0 0
       */
       
EndOfLoadedZone:
      .WORD 0x80000000


.text


      /* Main procedure of the verification program
       *   Inputs:     --
       *   Outputs:    --
       *   Clobbered:  -- */
Entry:
      addi r31 r0 #4096             /* initialize stack pointer */
      
      /* preliminary BT/BNE test */
      addi r1 r0 #-1001             /* load in R1 the error code */ 
      bne E_BNETestOK               /* this should branch */
      
      nop
      nop
      nop                           /* no it did not branch right... */
E_BNETestFailed:
      write r1 0
      halt                          /* error out and halt */
E_BNETestOK:
      bt E_BTTestOK                 /* the target was hit! try BT now */
      nop
      nop                           /* either BT did not branch right, */
      nop                           /* or BNE overshot the target      */
      write r1 0
      halt                          /* error out and halt */
E_BTTestOK:
      bt E_BNETestNotTaken          /* target hit again! go to the last test */
      nop
      nop
      nop                           /* BT overshot... */
      write r1 0
      halt                          /* error out and halt */
E_BNETestNotTaken:
      addi r2 r0 #0                 /* zero out a random register */
      bne E_BNETestFailed           /* this should NOT branch. If it does, we
                                     * error out */   
      /* preliminary LOAD test */
      load r1 ConstNeg12345         /* load -12345 in R1 with LOAD */
      addi r0 r1 #12345
      bne E_LoadTestFail            /* did it load -12345? no: fail */
      
      /* preliminary STORE test */
      addi r1 r1 #-5555             /* r1 = -12345 + (-5555) = -17900 */
      store r1 EndOfLoadedZone      /* STORE R1 to a test address */
      load r2 EndOfLoadedZone       /* read it back */
      addi r0 r2 #17900
      bne E_StoreTestFail           /* fail if loaded value incorrect */
      
      /* preliminary MOVA test */
      mova r1 0x7FFFF               /* load a number in R1 via MOVA */
      load r2 ConstNeg0x7FFFF       /* use LOAD+ADD to test it */
      add r0 r1 r2
      bne E_MOVATestFail            /* fail if loaded value incorrect */    
      
      /* preliminary JSR test */
E_JSRTest:
      jsr r31 E_JSRTestNextInstr    /* JSR to a test location */
E_JSRTestReturnLocation:            /* (we will never go back here again) */
      nop
      nop
      nop                           /* jump function of JSR undershot... */
      bt E_JSRTestFail              /* fail */
E_JSRTestNextInstr:
      bt E_JSRTestCheckStackUpdate  /* target hit! go check stack state */
      nop
      nop
      nop                           /* jump function of JSR overshot... */
      bt E_JSRTestFail              /* fail */
E_JSRTestCheckStackUpdate:
      addi r0 r31 #-4095
      bne E_JSRTestFail             /* stack pointer incorrect? fail */
      mova r1 E_JSRTestReturnLocation
      sub r0 r1 (r31)
      bne E_JSRTestFail             /* fail if pushed ret address incorrect */
      
      /* preliminary RET test */
E_RETTest:
      mova r1 E_RETTestNextInstr    /* change the return address to a value */
      store r1 4095                 /*  we chose */
      ret r31 0                     /* jump there by popping the stack */
      nop
      nop
      nop
      bt E_RETTestFail              /* RET undershot, fail */
E_RETTestNextInstr:
      bt E_RETTestCheckStackUpdate  /* target hit! go check stack state */
      nop
      nop
      nop
      bt E_RETTestFail              /* RET overshot, fail */
E_RETTestCheckStackUpdate:
      addi r0 r31 #-4096
      bne E_RETTestFail             /* stack pointer incorrect? fail */
      
      /* End of preliminary tests! */
      /* At this point we know we can use JSR/RET without worrying if they
       * work or not */
      addi r31 r0 #4096             /* reset the stack pointer */
      
      jsr r31 TestUnaryPhase2       /* finish testing unary instructions */
      jsr r31 TestTernOrBin         /* test binary and ternary instructions */
      jsr r31 TestBranchAndCond     /* test branches and Sxx instructions */
      
      load r1 ConstTestsEndedCode         
      write r1 0                    /* write the 'end of test' code */
      load r1 NumSucceeded          
      write r1 0                    /* write the number of succeeded tests */
      load r1 NumFailed          
      write r1 0                    /* write the number of failed tests */
      halt                          /* terminate */
      
      /* failure handler for LOAD */
E_LoadTestFail:
      addi r1 r0 #-1002
      write r1 0                    /* write the error code */
      halt                          /* hang up */
      
      /* failure handler for STORE */
E_StoreTestFail:
      addi r1 r0 #-1003
      write r1 0                    /* yell at the user */
      halt                          /* go away */
      
      /* failure handler for MOVA */
E_MOVATestFail:
      addi r1 r0 #-1004
      write r1 0                    /* spit it out */
      halt                          /* go to rest */
      
      /* failure handler for JSR */
E_JSRTestFail:
      addi r1 r0 #-1005
      write r1 0                    /* say "panic" but in numbers */
      halt                          /* pass out */
      
      /* failure handler for RET */
E_RETTestFail:
      addi r1 r0 #-1006
      write r1 0                    /* point at the murderer */
      halt                          /* die */
      
      
      
      /* Tests flag behavior of unary instructions except branches and Scc
       * instructions (nop, jsr, ret, mova, load, store)
       *   Inputs:     --
       *   Outputs:    NumSucceeded, NumFailed
       *   Clobbered:  r2, r11, r13, r15 - r17, r20, r23 */
TestUnaryPhase2:
      mova r15 0                 /* RS1, RS2, RD expected and RD real do not */                
      mova r16 0                 /* apply here, so we initialize the input to */
      mova r17 0                 /* LogError ahead of time to zero in case */
      mova r20 0                 /* there will be an error. These regs will */
                                 /* stay unused through the entire procedure */
      mova r2 Instructions
      addi r2 r2 #1000           /* set instruction ID to start at 1000 */
      addi r11 r0 #0xF           /* always expect PSW 0xF in output */
      addi r13 r11 #0            /* give PSW 0xF in input */
      xpsw r13 0                 /* set PSW */
      jsr r31 TUP2_TestOneInstr  /* test JSR. Also increment instr. ID and */
                                 /* test RET because this subprogram returns 
                                 /* with RET */
                                 
      xpsw r13 0                 /* (code duplicated from TUP2_TestOneInstr) */
      addi r0 r13 #-0xF          /* check if after RET the PSW is unchanged */
      beq TUP2_TestRETFlagsOK    /* unchanged? RET is OK */
      jsr r31 LogError           /* otherwise, log the error */
      bt TUP2_TestOtherInstr     /* continue with the rest of the instructions*/
TUP2_TestRETFlagsOK:
      jsr r31 LogOK              /* log that RET is OK */
TUP2_TestOtherInstr:
      addi r2 r2 #1              /* increment instr. ID by 1 */
      addi r13 r11 #0
      xpsw r13 0                 /* restore PSW to 0xF */
      nop
      jsr r31 TUP2_TestOneInstr  /* test NOP */
      mova r1 12345
      jsr r31 TUP2_TestOneInstr  /* test MOVA */
      load r1 AddressOne
      jsr r31 TUP2_TestOneInstr  /* test LOAD */
      store r1 AddressOne
      jsr r31 TUP2_TestOneInstr  /* test STORE */
      ret r31 0                  /* done! */

      /* checks if PSW is as expected and logs the error if so; otherwise
       * increments the success counter, increments the instr. ID and
       * restores PSW to the input value. */
TUP2_TestOneInstr:
      xpsw r13 0                 /* load current PSW */
      addi r0 r13 #-0xF
      beq TUP2_TestFlagsOK       /* if PSW equal to 0xF, ok */
      jsr r31 LogError
      bt TUP2_PrepareNextTest
TUP2_TestFlagsOK:
      jsr r31 LogOK              /* log correct test */
TUP2_PrepareNextTest:
      addi r2 r2 #1              /* increment instr. ID by 1 */
      addi r13 r11 #0             
      xpsw r13 0                 /* restore PSW to 0xF */
      ret r31 0                  /* return */

      
      
      /* Test binary and ternary instructions
       *   Inputs:     --
       *   Outputs:    NumSucceeded, NumFailed
       *   Clobbered:  r1 - r14, r20 - r23 */
TestTernOrBin:
      mova r1 TestTernOrBinData    /* load pointer to bytecode */
      mova r2 Instructions             /* load pointer to the instructions */
TTOB_Loop:
      orb r0 r0 (r1)
      beq TTOB_Stop                    /* reached terminator? yes: exit */
      jsr r31 TestTernOrBinInsnGroup   /* execute this test group */
      bt TTOB_Loop                     /* next test group */
TTOB_Stop:
      ret r31 0                        /* return */
      
      
      
      /* Test a single group of binary and ternary instructions
       *   Inputs:     r1  Address of the bytecode of the test group
       *               r2  Address of the instructions to be tested
       *   Outputs:    NumSucceeded, NumFailed
       *               r1  Address of the bytecode of the next test group
       *               r2  Address of the next instructions to be tested
       *   Clobbered:  r3 - r14, r20 - r23
       *   Note: a group of tests consists of a set of inputs to be applied to
       * a list of instructions. Each subgroup of tests consists of
       * instructions with the same behavior but different encoding (i.e. all
       * ADD(I) instructions). */
TestTernOrBinInsnGroup:
      orb r3 r0 (r1)       /* load the group header to unpack it */
      shri r5 r3 #16       /* r5 = number of instructions */
      shri r4 r3 #8
      andbi r4 r4 #0xFF    /* r4 = number of variations per instruction */
      andbi r3 r3 #0xFF    /* r3 = number of tests per variation */
      addi r1 r1 #1
      add r7 r3 r3         /* r7 = pointer to the array of expected */
      add r7 r7 r1         /*      RDs for the instructions */
      mul r8 r5 r3
      add r8 r8 r7         /* r8 = pointer to the array of expected flags */
      eorb r9 r9 r9        /* r9 = offset of the flag in the packed struct */
   TTOBIG_InsnLoop:

         eorb r6 r6 r6                    /* r6 = count of inputs tested */   
      TTOBIG_InsnTestLoop:

            eorb r10 r10 r10              /* r10 = count of subgroups tested */
            jsr r31 LoadNextFlags         /* r11 = output flags for all insns.
                                           * in this subgroup */
         TTOBIG_InsnTestVariantsLoop:
               jsr r31 TestTernOrBinInsn  /* do a test */
               addi r2 r2 #1              /* move instr. pointer to next one */
               addi r10 r10 #1
               sub r0 r10 r4
               bne TTOBIG_InsnTestVariantsLoop  /* loop r4 times */
      
            addi r7 r7 #1                 
            addi r1 r1 #2
            addi r6 r6 #1                 /* advance to next inputs */
            sub r2 r2 r4                  /* move back instr. pointer to start 
                                           * of subgroup */
            sub r0 r6 r3
            bne TTOBIG_InsnTestLoop       /* loop r3 times */
      
         add r2 r2 r4         /* advance instr. pointer to next subgroup */
         sub r1 r1 r3
         sub r1 r1 r3         /* move inputs pointer back to start */         
         subi r5 r5 #1
         bne TTOBIG_InsnLoop  /* loop r5 times */
         
      add r1 r0 r8            
      andb r9 r9 r9
      beq TTOBIG_Exit         
      addi r1 r1 #1           /* advance to next group header */
TTOBIG_Exit:                  
      ret r31 0               /* return */
      
      
   
      /* Unpack a set of flags from a packed array and increment pointers.
       *   Inputs:     r8  Address of the current word in the array
       *               r9  Offset (in bits) of the 4-bit flag to extract
       *   Outputs:    r11 The extracted 4-bits of flags
       *               r8  Address of the next word in the array
       *               r9  Offset (in bits) of the next 4-bit flag to extract
       *   Clobbered:  -- */
LoadNextFlags:
      add r11 r0 (r8)      /* load the word of flags */
      shr r11 r11 r9
      andbi r11 r11 #0xF   /* extract the flag */
      addi r9 r9 #4        /* increment the bit counter */
      subi r0 r9 #32
      bne LNF_Exit         /* if next flag is within same word, done */
      addi r8 r8 #1
      eorb r9 r9 r9        /* otherwise increment address, reset bit counter */
LNF_Exit:
      ret r31 0            /* return */
      
      
      
      /* Test a single instruction with specific inputs.
       *   Inputs:     r2  Address of the instruction to test
       *               r1  Address of the array of input RSes
       *               r7  Address of the expected RD
       *               r11 Expected flags
       *   Outputs:    NumSucceeded, NumFailed
       *   Clobbered:  r12 - r14, r20 - r23 */
TestTernOrBinInsn:
      load r20 Const55AA55AA        /* load a dummy value in the RD register 
                                     * to detect issues with setting it */
      add r21 r0 (r1)               /* load the value of RS1 */
      addi r22 r1 #1
      add r22 r0 (r22)              /* load the value of RS2 */
      
      add r12 r0 (r2)               /* r12 = machine instruction to test */
      shri r13 r12 #30
      subi r0 r13 #0                /* is it a ternary coded instruction? */
      beq TTOBI_AdjustIndirection   /* yes: check for indirectly indexed regs */
                                    /* no: assume it is a binary coded 
                                     * instruction, patch the immediate */
TTOBI_AdjustImmediate:
      /* Check if the value of RS2 fits in 16 bits (with sign extension).
       * This is true if RS2 is within [-32768, 32767].
       *   We test this condition by flipping the [-32768, -1] range to
       * [0, 32767], and then testing that the result is <= than 32767. */
      shri r13 r22 #31              /* r13 = r22 < 0 ? -1 : 0 */
      eorb r14 r22 r13              /* r14 = r22 < 0 ? ~r22 : r22 */
      shri r14 r14 #15              /* r14 = shift out the lower 15 bits */
      bne TTOBI_Ignore              /* result is not zero (ie RS2 cannot fit 
                                     * in 16 bit)? yes: skip the test */
      /* RS2 fits in 16 bits. Patch the immediate in the instruction to its
       * value. */
      shli r13 r13 #16              /* r13 = r22 < 0 ? 0xFFFF0000 : 0 */
      eorb r14 r22 r13              /* mask out the higher 16 bits of RS2 */ 
      orb r12 r12 r14               /* patch it in */
      bt TTOBI_WriteAndTest         /* go to the actual test! */
      
TTOBI_AdjustIndirection:
      andbi r0 r12 #8               /* test the RS2 indirect bit */
      beq TTOBI_AdjustIndirectionCheckRD  /* if zero, not indirect */
      store r22 AddressOne          /* move the RS2 argument to memory */
      mova r22 AddressOne           /* set RS2 to the address of the argument */
TTOBI_AdjustIndirectionCheckRD:
      andbi r0 r12 #4               /* test the RD indirect bit */
      beq TTOBI_WriteAndTest        /* if zero, not indirect */
      store r20 AddressTwo          /* move the RD dummy value to memory */
      mova r20 AddressTwo           /* set RD to the destination address */
      
TTOBI_WriteAndTest:
      store r12 TTOBI_Test          /* replace the nop at TTOBI_Test with the 
                                     * instruction to test */
      addi r13 r0 #0xF
      xpsw r13 0                    /* set all flags to 0xF (which is an
                                     * otherwise impossible value) to detect
                                     * issues with setting flags */
TTOBI_Test:
      nop                           /* execute the instruction to test */
      
TTOBI_StoreFlags:
      xpsw r13 0                    /* r13 = flag output from the instr. */
      
      /* if RD is zero, check that R0 was not modified by the instruction */
TTOBI_CheckRDZero:
      shri r17 r12 #21
      andbi r17 r17 #0x1F            /* r17 = extracted RD from instruction */
      bne TTOBI_CheckAdjustRDInverse /* RD is R0? no: assume RD is R20 */
      add r14 r17 r0                 /* r14 = current value of R0 */
      bne TTOBI_ErrorRDNotZero       /* if R0 not zero, error! */
      bt TTOBI_ComparePSW            /* otherwise continue and check the PSW */
      
      /* load RD from memory if it was indirectly addressed */
TTOBI_CheckAdjustRDInverse:
      shri r0 r12 #30
      bne TTOBI_Compare             /* not a ternary instruction? yes: skip */
      andbi r0 r12 #4
      beq TTOBI_Compare             /* no indirect addressing? yes: skip */
      load r20 AddressTwo           /* no: load memory RD */
      
TTOBI_Compare:
      sub r0 r20 (r7)
      bne TTOBI_Error               /* expected RD != real RD? error */
TTOBI_ComparePSW:
      sub r0 r13 r11
      bne TTOBI_Error               /* expected PSW != real PSW? error */
      
      bt LogOK                      /* otherwise log a successful test */
TTOBI_Ignore:
      ret r31 0                     /* return */
      
TTOBI_Error:
      add r17 r0 (r7)               /* load expected rd */
TTOBI_ErrorRDNotZero:
      add r15 r0 (r1)               /* load rs1 */
      addi r16 r1 #1
      add r16 r0 (r16)              /* load rs2 */
      bt LogError                   /* log the error and return */
      
      
      
      /* Tests conditional instructions (Scc instructions) and branch
       * instructions.
       *   Inputs:     --
       *   Outputs:    NumSucceeded, NumFailed
       *   Clobbered:  r1 - r6, r11, r13, r15 - r17, r20, r23 */
TestBranchAndCond:
      mova r1 TestBranchAndCondData /* r1 = data pointer */
      mova r2 UnaryInstructions     /* r2 = instruction pointer */
      
      add r4 r0 (r1)                /* r4 = number of tests */
      addi r1 r1 #1                 /*  (instr. * flag combos) */
      shri r3 r4 #3
      add r3 r3 r1                  /* r3 = pointer to array of RDs
                                     *  (as packed bits) */                
      eorb r15 r15 r15              /* r15 = currently tested PSW */
      eorb r5 r5 r5                 /* r5 = sub-index in the current word of 
                                     *  packed expected PSWs */ 
      eorb r6 r6 r6                 /* r6 = sub-index in the current word of 
                                     *  packed expected RDs */
TBC_Loop:
      jsr r31 TBC_Trampoline        /* execute next tested instruction */
      xpsw r13 0                    /* r13 = output PSW */
      
      add r11 r0 (r1)
      shr r11 r11 r5
      andbi r11 r11 #0xF            /* r11 = expected output PSW */
      
      add r17 r0 (r3)
      shr r17 r17 r6
      andbi r17 r17 #1              /* r17 = expected output RD */
      
      sub r0 r13 r11
      bne TBC_Fail                  /* PSW not as expected? fail */
      sub r0 r20 r17
      beq TBC_OK                    /* RD as expected? ok */
TBC_Fail:
      mova r16 0                    /* these instr. have no RS2, set to zero */
      jsr r31 LogError              /* show error codes */
      bt TBC_Next                   /* test next instruction */
TBC_OK:  
      jsr r31 LogOK                 /* increment success counter */
TBC_Next:
      addi r15 r15 #1
      andbi r15 r15 #0xF            /* advance tested PSW data pointer */
      bne TBC_AdvanceFlagCtr        /* increment test instr. every 16 values */
      addi r2 r2 #2                 /* of PSW (as there can be only 16) */
TBC_AdvanceFlagCtr:
      addi r5 r5 #4
      andbi r5 r5 #0x1F
      bne TBC_AdvanceRDCtr
      addi r1 r1 #1                 /* advance expected PSW data pointer */
TBC_AdvanceRDCtr:
      addi r6 r6 #1
      andbi r6 r6 #0x1F
      bne TBC_DecTestCtr
      addi r3 r3 #1                 /* advance expected RD data pointer */
TBC_DecTestCtr:
      subi r4 r4 #1
      bne TBC_Loop                  /* loop until # of tests left is zero */
      
TBC_Exit:
      ret r31 0                     /* return when finished */
      
      /* Prepare the CPU state and jump to the instruction to test */
TBC_Trampoline:
      mova r20 0x5A5A               /* initialize RD to a sentinel value */
      subi r31 r31 #1
      add (r31) r0 r2               /* push the test instruction pointer to
                                     * the call stack */
      add r13 r0 r15
      xpsw r13 0                    /* load the input into the PSW */
      
      ret r31 0                     /* jump to the test instruction */
      
      

      /* Log a failure of a test to the console
       *   Inputs:     NumFailed
       *               r2  Address of the instruction whose test has failed
       *               r11 Expected value of the PSW 
       *               r13 Real value of the PSW
       *               r15 RS1 input to the failed test
       *               r16 RS2 input to the failed test
       *               r17 Expected value of RD
       *               r20 Real value of RD
       *   Outputs:    NumFailed
       *   Clobbered:  r23 */
LogError:
      mova r23 -2000
      write r23 0             /* print the "test failed" status code */
      mova r23 Instructions
      sub r23 r2 r23
      write r23 0             /* log instruction */
      write r15 0             /* log rs1 */
      write r16 0             /* log rs2 */
      write r20 0             /* log real dest */
      write r13 0             /* log real flags */        
      write r17 0             /* log expected rd */
      write r11 0             /* log expected flags */
      load r23 NumFailed
      addi r23 r23 #1
      store r23 NumFailed     /* increment number of failed tests */
      ret r31 0               /* return */

      
      
      /* Increment the count of successful tests
       *   Inputs:     NumSucceeded
       *   Outputs:    NumSucceeded
       *   Clobbered:  r23 */
LogOK:
      load r23 NumSucceeded
      addi r23 r23 #1
      store r23 NumSucceeded  /* increment number of succeeded tests */
      ret r31 0               /* return */

      
      
      /* Instructions to test
       *   Standard registers used for the test are R20, R21, R22 as RD, RS1 and
       * RS2 respectively. */
Instructions:
    /*    0 */ add r20 r21 r22
    /*    1 */ add r20 r21 (r22)
    /*    2 */ add (r20) r21 r22
    /*    3 */ add (r20) r21 (r22)
    /*    4 */ addi r20 r21 #0
    /*    5 */ add r0 r21 r22
      
    /*    6 */ sub r20 r21 r22
    /*    7 */ sub r20 r21 (r22)
    /*    8 */ sub (r20) r21 r22
    /*    9 */ sub (r20) r21 (r22)
    /*   10 */ subi r20 r21 #0
    /*   11 */ sub r0 r21 r22
     
    /*   12 */ andl r20 r21 r22
    /*   13 */ andl r20 r21 (r22)
    /*   14 */ andl (r20) r21 r22
    /*   15 */ andl (r20) r21 (r22)
    /*   16 */ andli r20 r21 #0
    /*   17 */ andl r0 r21 r22
     
    /*   18 */ orl r20 r21 r22
    /*   19 */ orl r20 r21 (r22)
    /*   20 */ orl (r20) r21 r22
    /*   21 */ orl (r20) r21 (r22)
    /*   22 */ orli r20 r21 #0
    /*   23 */ orl r0 r21 r22
     
    /*   24 */ eorl r20 r21 r22
    /*   25 */ eorl r20 r21 (r22)
    /*   26 */ eorl (r20) r21 r22
    /*   27 */ eorl (r20) r21 (r22)
    /*   28 */ eorli r20 r21 #0
    /*   29 */ eorl r0 r21 r22
     
    /*   30 */ andb r20 r21 r22
    /*   31 */ andb r20 r21 (r22)
    /*   32 */ andb (r20) r21 r22
    /*   33 */ andb (r20) r21 (r22)
    /*   34 */ andbi r20 r21 #0
    /*   35 */ andb r0 r21 r22
     
    /*   36 */ orb r20 r21 r22
    /*   37 */ orb r20 r21 (r22)
    /*   38 */ orb (r20) r21 r22
    /*   39 */ orb (r20) r21 (r22)
    /*   40 */ orbi r20 r21 #0
    /*   41 */ orb r0 r21 r22
     
    /*   42 */ eorb r20 r21 r22
    /*   43 */ eorb r20 r21 (r22)
    /*   44 */ eorb (r20) r21 r22
    /*   45 */ eorb (r20) r21 (r22)
    /*   46 */ eorbi r20 r21 #0
    /*   47 */ eorb r0 r21 r22
     
    /*   48 */ mul r20 r21 r22
    /*   49 */ mul r20 r21 (r22)
    /*   50 */ mul (r20) r21 r22
    /*   51 */ mul (r20) r21 (r22)
    /*   52 */ muli r20 r21 #0
    /*   53 */ mul r0 r21 r22
     
    /*   54 */ div r20 r21 r22
    /*   55 */ div r20 r21 (r22)
    /*   56 */ div (r20) r21 r22
    /*   57 */ div (r20) r21 (r22)
    /*   58 */ divi r20 r21 #0
    /*   59 */ div r0 r21 r22
     
    /*   60 */ shr r20 r21 r22
    /*   61 */ shr r20 r21 (r22)
    /*   62 */ shr (r20) r21 r22
    /*   63 */ shr (r20) r21 (r22)
    /*   64 */ shri r20 r21 #0
    /*   65 */ shr r0 r21 r22
     
    /*   66 */ shl r20 r21 r22
    /*   67 */ shl r20 r21 (r22)
    /*   68 */ shl (r20) r21 r22
    /*   69 */ shl (r20) r21 (r22)
    /*   70 */ shli r20 r21 #0
    /*   71 */ shl r0 r21 r22
     
    /*   72 */ rotl r20 r21 r22
    /*   73 */ rotl r20 r21 (r22)
    /*   74 */ rotl (r20) r21 r22
    /*   75 */ rotl (r20) r21 (r22)
    /*   76 */ rotli r20 r21 #0
    /*   77 */ rotl r0 r21 r22
     
    /*   78 */ rotr r20 r21 r22
    /*   79 */ rotr r20 r21 (r22)
    /*   80 */ rotr (r20) r21 r22
    /*   81 */ rotr (r20) r21 (r22)
    /*   82 */ rotri r20 r21 #0
    /*   83 */ rotr r0 r21 r22
     
    /*   84 */ neg r20 r21 r22
    /*   85 */ neg r20 r21 (r22)
    /*   86 */ neg (r20) r21 r22
    /*   87 */ neg (r20) r21 (r22)
    /*   88 */ neg r0 r21 r22
     
    /*   89 */ notl r20 r21 #0
    /*   90 */ notl r0 r21 #0
     
    /*   91 */ notb r20 r21 #0
    /*   92 */ notb r0 r21 #0
      
UnaryInstructions:
    /*   93 */ beq UI_ReturnTrue
               bt UI_ReturnFalse
    /*   95 */ bge UI_ReturnTrue
               bt UI_ReturnFalse
    /*   97 */ bt UI_ReturnTrue
               bt UI_ReturnFalse
    /*   99 */ bf UI_ReturnTrue
               bt UI_ReturnFalse
    /*  101 */ bhi UI_ReturnTrue
               bt UI_ReturnFalse
    /*  103 */ bls UI_ReturnTrue
               bt UI_ReturnFalse
    /*  105 */ bgt UI_ReturnTrue
               bt UI_ReturnFalse
    /*  107 */ ble UI_ReturnTrue
               bt UI_ReturnFalse
    /*  109 */ blt UI_ReturnTrue
               bt UI_ReturnFalse
    /*  111 */ bne UI_ReturnTrue
               bt UI_ReturnFalse
    /*  113 */ bcc UI_ReturnTrue
               bt UI_ReturnFalse
    /*  115 */ bcs UI_ReturnTrue
               bt UI_ReturnFalse
    /*  117 */ bvc UI_ReturnTrue
               bt UI_ReturnFalse
    /*  119 */ bvs UI_ReturnTrue
               bt UI_ReturnFalse
    /*  121 */ bpl UI_ReturnTrue
               bt UI_ReturnFalse
    /*  123 */ bmi UI_ReturnTrue
               bt UI_ReturnFalse
  
    /*  125 */ seq r20 0
               ret r31 0
    /*  127 */ sge r20 0
               ret r31 0
    /*  129 */ sgt r20 0
               ret r31 0
    /*  131 */ sle r20 0
               ret r31 0
    /*  133 */ slt r20 0
               ret r31 0
    /*  135 */ sne r20 0
               ret r31 0
               
    /* (the following instructions are special-cased in TestUnaryPhase2) */
    
    /* 1000    jsr r31 <pointer> */
    /* 1001    ret r31 0 */
    /* 1002    mova r1 12345 */
    /* 1003    load r1 AddressOne */
    /* 1004    store r1 AddressOne */
      
UI_ReturnTrue:
      mova r20 1
      ret r31 0
UI_ReturnFalse:
      mova r20 0
      ret r31 0
