/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_target_transform.h
 * Formal Languages & Compilers Machine, 2007-2020
 */

#ifndef _AXE_TARGET_TRANSFORM_H
#define _AXE_TARGET_TRANSFORM_H

#include "axe_engine.h"

/* Perform lowering of the program to a subset of the IR which can be
 * represented as instructions of the target architecture. */
void doTargetSpecificTransformations(t_program_infos *program);

#endif
