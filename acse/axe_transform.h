/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_transform.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_TRANSFORM_H
#define _AXE_TRANSFORM_H

#include "axe_engine.h"
#include "axe_cflow_graph.h"
#include "axe_reg_alloc.h"

/* once executed the liveness analysis, by calling this function
 * we will obtain a graph with the correct load and store
 * instructions */
extern t_cflow_Graph * insertLoadAndStoreInstr
         (t_program_infos *program, t_cflow_Graph *graph);

/* Replace the variable identifiers in the instructions of the CFG with the
 * register assignments in the register allocator. Materialize spilled
 * variables to the scratch registers. All new instructions are inserted
 * in the CFG. */
void materializeRegisterAllocation(t_program_infos *program,
  t_cflow_Graph *graph, t_reg_allocator *RA);

/* synchronize the list of instructions inside the given program with
 * the contents of the control flow graph. */
void updateProgramInfos(t_program_infos *program,
  t_cflow_Graph *graph);
         
#endif
