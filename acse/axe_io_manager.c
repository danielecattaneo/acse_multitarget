/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_io_manager.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdlib.h>
#include <string.h>
#include "axe_io_manager.h"

static t_io_infos * allocOutputInfos();


t_io_infos * initializeOutputInfos(int argc, char **argv)
{
   t_io_infos *result;
#ifndef NDEBUG
   char *basename;
   int lastDot, i, max_fn;
   char *frontend_out_fn;
   char *symbol_table_out_fn;
   char *cfg_out_fn;
   char *data_flow_out_fn;
   char *regalloc_out_fn;
#endif

   /* create a new instance of `t_io_infos' */
   result = allocOutputInfos();
   if (result == NULL)
      return NULL;
   
   argc--;
   argv++;

   if (argc > 0)
   {
      result->input_file = fopen(argv[0], "r");
      if (result->input_file == NULL)
      {
         fprintf( stderr, "File not found : %s.\n", argv[0]);
         exit(-1);
      }
#ifndef NDEBUG
      fprintf(stdout, "Input file: %s. \n", argv[0]);
#endif
   }
#ifndef NDEBUG
   else
      fprintf(stdout, "Input file: %s. \n", "standard input");
#endif

   /* update the value of yyin */
   yyin = result->input_file;

   if (argc == 2)
      result->output_file_name = argv[1];
   else
      result->output_file_name = "output.asm";

#ifndef NDEBUG
   basename = strdup(result->output_file_name);
   lastDot = -1;
   for (i = 0; basename[i] != '\0'; i++) {
      if (basename[i] == '.')
         lastDot = i;
   }
   if (lastDot >= 0)
      basename[lastDot] = '\0';

   max_fn = strlen(basename) + 24;
   frontend_out_fn = calloc(max_fn, sizeof(char));
   snprintf(frontend_out_fn, max_fn, "%s_frontend.log", basename);
   symbol_table_out_fn = calloc(max_fn, sizeof(char));
   snprintf(symbol_table_out_fn, max_fn, "%s_symbol_table.log", basename);
   cfg_out_fn = calloc(max_fn, sizeof(char));
   snprintf(cfg_out_fn, max_fn, "%s_control_flow.log", basename);
   data_flow_out_fn = calloc(max_fn, sizeof(char));
   snprintf(data_flow_out_fn, max_fn, "%s_data_flow.log", basename);
   regalloc_out_fn = calloc(max_fn, sizeof(char));
   snprintf(regalloc_out_fn, max_fn, "%s_reg_alloc.log", basename);

   fprintf(stdout, "Output will be written on file: "
         "\"%s\". \n", result->output_file_name);
   fprintf(stdout, "The output of the frontend will be written on file: "
         "\"%s\". \n", frontend_out_fn);
   fprintf(stdout, "The Symbol Table will be written on file: "
         "\"%s\". \n", symbol_table_out_fn);
   fprintf(stdout, "Intermediate code will be written on file: "
         "\"%s\". \n", cfg_out_fn);
   fprintf(stdout, "control/dataflow informations will "
                   "be written on file: \"%s\". \n", data_flow_out_fn);
   fprintf(stdout, "Output of the register allocator "
                   "will be written on file: \"%s\". \n\n", regalloc_out_fn);

   result->frontend_output = fopen(frontend_out_fn, "w");
   result->cfg_1 = fopen(cfg_out_fn, "w");
   result->cfg_2 = fopen(data_flow_out_fn, "w");
   result->reg_alloc_output = fopen(regalloc_out_fn, "w");
   result->syTable_output = fopen(symbol_table_out_fn, "w");
   if (result->frontend_output == NULL)
      fprintf(stderr, "WARNING : Unable to create file: %s.\n", frontend_out_fn);
   if (result->cfg_1 == NULL)
      fprintf(stderr, "WARNING : Unable to create file: %s.\n", cfg_out_fn);
   if (result->cfg_2 == NULL)
      fprintf(stderr, "WARNING : Unable to create file: %s.\n", data_flow_out_fn);
   if (result->syTable_output == NULL)
      fprintf(stderr, "WARNING : Unable to create file: %s.\n", symbol_table_out_fn);
   if (result->reg_alloc_output == NULL)
      fprintf(stderr, "WARNING : Unable to create file: %s.\n", regalloc_out_fn);

   free(basename);
   free(frontend_out_fn);
   free(symbol_table_out_fn);
   free(cfg_out_fn);
   free(data_flow_out_fn);
   free(regalloc_out_fn);
#endif

   return result;
}

t_io_infos * allocOutputInfos()
{
   t_io_infos *result;

   /* Allocate memory for an instance of `t_output_infos' */
   result = (t_io_infos *)
         malloc(sizeof(t_io_infos));

   /* test if malloc returned a null pointer */
   if (result == NULL)
      return NULL;
      
   /* initialize the instance internal data */
   result->output_file_name = NULL;
   result->input_file = stdin;
#ifndef NDEBUG
   result->frontend_output = stdout;
   result->cfg_1 = stdout;
   result->cfg_2 = stdout;
   result->reg_alloc_output = stdout;
   result->syTable_output = stdout;
#endif

   /* return the result */
   return result;
}

void finalizeOutputInfos(t_io_infos *infos)
{
   if (infos == NULL)
      return;
   if (infos->input_file != NULL)
      fclose(infos->input_file);
#ifndef NDEBUG
   if (infos->frontend_output != NULL)
      fclose(infos->frontend_output);
   if (infos->cfg_1 != NULL)
      fclose(infos->cfg_1);
   if (infos->cfg_2 != NULL)
      fclose(infos->cfg_2);
   if (infos->reg_alloc_output != NULL)
      fclose(infos->reg_alloc_output);
   if (infos->syTable_output != NULL)
      fclose(infos->syTable_output);
#endif

   free(infos);
}
