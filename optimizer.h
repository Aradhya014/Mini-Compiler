#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "codegen.h"

typedef struct {
    int foldedConstants;
    int eliminatedDeadCode;
} OptStats;

OptStats optimizeTAC(CodeGen *cg);

#endif /* OPTIMIZER_H */
