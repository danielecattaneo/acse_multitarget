/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * machine.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */

#include "machine.h"

int reg[NREGS];
int mem[MEMSIZE];
unsigned int pc;
int psw;

/* Debug printf, print the value of the status word */
void print_psw(FILE *file)
{
   /* precondition */
   if (file == NULL)
      file = stderr;

   fprintf(file, "PSW = 0x%08x\n", psw);
   fprintf(file,
         "CARRY (C):\t %d\nOVERFLOW (V):\t %d\n"
         "ZERO (Z):\t %d\nNEGATIVE (N):\t %d\n\n",
         getflag(CARRY), getflag(OVERFLOW), getflag(ZERO), getflag(NEGATIVE));
}

/* Debug printf, print the content of the register file */
void print_regs(FILE *file)
{
   int i;

   /* precondition */
   if (file == NULL)
      file = stderr;

   fprintf(file, "\n*** REGISTER FILE STATUS ***\n");
   for (i = 0; i < NREGS; i++) {
      fprintf(file, "R%-2d = 0x%08x%s", i, reg[i], i % 4 == 3 ? "\n" : "  ");
   }
   if (i % 4 != 0)
      fprintf(file, "\n");
   fprintf(file, "PC  = 0x%08x\n", pc);
}

/* Debug, execute a memory dump      */
void print_Memory_Dump(FILE *file, int lcode)
{
   int i;

   /* if not specified, `file' is set to stderr */
   if (file == NULL)
      file = stderr;

   fprintf(file, "*** Memory Dump *** \n");
   for (i = 0; i < lcode; i++) {
      if (i % 4 == 0)
         fprintf(file, "0x%08x:  ", i);
      fprintf(file, "0x%08x%s", mem[i], i % 4 == 3 ? "\n" : "  ");
   }
   if (i % 4 != 0)
      fprintf(file, "\n");

   fprintf(file, "*** END Memory Dump *** \n");
}

/* Get flag from processor status word */
unsigned int getflag(int flag)
{
   return getbits(psw, flag, flag);
}

/* Set flag in processor status word*/
void setflag(int flag, int value)
{
   int mask = 1 << flag;
   if (value)
      psw = psw | mask;
   else
      psw = psw & ~mask;
}
