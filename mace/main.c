/*
 * Giovanni Agosta, Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * main.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetch.h"
#include "machine.h"
#include "decode.h"

/*Returns 0 if is ok, other number if an error occured */
static int check_signature(FILE *fp);

int main(int argc, char **argv)
{
   FILE *fp;                  /* pointer to the object file    */
   int len = 0;               /* length of the object file   */
   unsigned int *code = NULL; /* pointer to the code block */
   int lcode = 0;             /* length of the code block         */
   int mmgmt = BASIC;         /* memory management */
   int i;
   int breakat = -1; /* break execution at instruction # */
   int count = 0;    /* iterations counter    */
   pc = 0;           /* PC register is set ot zero in the beginning */
#ifdef DEBUG
   decoded_instr *current_instr;
#endif

   /* Opening the object file */
   if (argc < 2) {
      /* No object file has been specified as an argument:
       * execution terminates */
      fprintf(stdout,
            "Formal Languages & Compilers Machine, 2007/2008.\n"
            "\n\nSyntax:\n\tmace [options] objectfile\n");
      return NOARGS;
   }

   /* open the object file specified as an argument */
   fp = fopen(argv[argc - 1], "rb");
   if (fp == NULL) {
      fprintf(stderr, "Object file %s doesn't exist.\n", argv[argc - 1]);

      /* the object file specified does not exist: quit */
      return NOFILE;
   }

   for (i = 1; i < argc - 1; i++) {
      if (strcmp(argv[i], "segmented") == 0)
         mmgmt = SEGMENTED;
      else if (strcmp(argv[i], "break") == 0) {
         char *error;

         /* read the next argument as a base-10 number */
         breakat = strtol(argv[i + 1], &error, 10);

         if (*error != '\0') {
/* could not parse the argument */
#ifdef DEBUG
            fprintf(stderr,
                  "Error while reading option "
                  "break: %d=%c, string=%s "
                  "=> %d.\n",
                  *error, *error, argv[i + 1], breakat);
#endif
            return WRONG_ARGS;
         }
         i++; /* skip the argument we have just read */
      }
   }

#ifdef DEBUG
   fprintf(stderr, "Running with %s memory \n",
         (mmgmt == BASIC) ? "BASIC" : "SEGMENTED");
#endif

   /* initialize registers and memory */
   for (i = 0; i < NREGS; i++)
      reg[i] = 0;
   for (i = 0; i < MEMSIZE; i++)
      mem[i] = 0;

   /* get file length */
   fseek(fp, 0, SEEK_END);
   /* The binary is made of 4-byte words */
   len = ftell(fp) / 4;
   fseek(fp, 0, SEEK_SET);

#ifdef DEBUG
   fprintf(stderr,
         "Available memory: %d. "
         "Requested memory: %d \n",
         MEMSIZE, len);
#endif

   if (mmgmt == BASIC) {
      /* single memory area for both code and data */
      if (len > MEMSIZE) {
         fprintf(stderr, "Out of memory.\n");
         return MEM_FAULT;
      }

      code = (unsigned int *)mem;
   } else {
      /* allocate a separate memory area just for the code */
      code = (unsigned int *)malloc(sizeof(unsigned int) * len);
   }

   /* load the machine code into memory */
   if (check_signature(fp)) {
      return WRONG_FORMAT;
   }
   /* skip currently unused 16 bits of header */
   fseek(fp, 16, SEEK_CUR);
   /* actually load the code */
   lcode = len - 5; /* compute the number of instructions by subtracting the
                       length of the header (which is = 5 in 4 bytes
                       instructions, = 20 in bytes) */
   fread(code, 4, lcode, fp);
#ifdef DEBUG
   fprintf(stderr, "Starting execution.\n");
   print_regs(stderr);
   print_psw(stderr);
   print_Memory_Dump(stderr, lcode);
   if (pc < lcode) {
      current_instr = decode(code[0]);
      print(stderr, current_instr);
      free(current_instr);
      fflush(stderr);
   }
#endif

   /* decode and execute each instruction */
   for (pc = 0; pc < lcode && pc >= 0;) {
      pc = fetch_execute(code, pc);

#ifdef DEBUG
      print_regs(stderr);
      print_psw(stderr);
      print_Memory_Dump(stderr, lcode);
#endif

      reg[0] = 0; /* reset R0 to 0; R0 is wired to 0, so we ignore all writes */

      count++; /* count the amount of instructions we execute */
      if ((breakat > 0) && (breakat <= count)) {
#ifdef DEBUG
         fprintf(stderr, "Break after %d instructions.\n", count);
#endif
         return BREAK;
      }

      /* Check the HALT condition */
      if (pc == _HALT)
         return OK;

#ifdef DEBUG
      current_instr = decode(code[pc]);
      fprintf(stderr, "\n\n");
      print(stderr, current_instr);
      free(current_instr);
      fflush(stderr);
#endif
   }

#ifdef DEBUG
   fprintf(stderr, "Memory access error.\n");
#endif

   return pc;
}

static int check_signature(FILE *fp)
{
   int c1, c2, c3, c4;
   c1 = fgetc(fp);
   c2 = fgetc(fp);
   c3 = fgetc(fp);
   c4 = fgetc(fp);
   if (c1 != 'L' || c2 != 'F' || c3 != 'C' || c4 != 'M') {
#ifdef DEBUG
      fprintf(stderr, "Wrong object file format.\n");
#endif

      return WRONG_FORMAT;
   }
   return 0;
}
