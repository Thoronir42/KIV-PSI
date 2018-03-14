#ifndef SHAPE_OPERATOR_H
#define SHAPE_OPERATOR_H

#include "types.h"


void shape_operator_init();

float shape_op_execute(shape_t shape, shape_op_t operation, float *params);

#endif /* SHAPE_OPERATOR_H */

