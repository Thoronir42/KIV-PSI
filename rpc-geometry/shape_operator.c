#include <stddef.h>
#include <string.h>

#include "shape_operator.h"

// shape operations
float (*__shops[SHAPE_COUNT * OPERATION_TYPE_COUNT])(float *);

void __put(shape_t shape, shape_op_t operation, float (*fun)(float*)) {
    __shops[shape * OPERATION_TYPE_COUNT + operation] = fun;
}

// operation implementations

float _op_zero(float *params) {
    return 0;
}

float _op_block_perimeter(float *params) {
    return 4 * (params[0] + params[1] + params[2]);
}

float _op_block_surface(float *params) {
    return 2 * (params[0] * params[1] + params[0] * params[2] + params[1] * params[2]) ;
}

float _op_block_volume(float *params) {
    return params[0] * params[1] * params[2];
}


void init() {
    memset(__shops, 0, SHAPE_COUNT * OPERATION_TYPE_COUNT * sizeof(void *));

    __put(shape_block, shape_op_perimeter, _op_block_perimeter);
    __put(shape_block, shape_op_surface, _op_block_surface);
    __put(shape_block, shape_op_volume, _op_block_volume);
}

float shape_op_execute(shape_t shape, shape_op_t operation, float *params) {
    if(shape < 1 || shape > SHAPE_COUNT) {
        return -1;
    }
    if(operation < 1 || operation > OPERATION_TYPE_COUNT) {
        return -1;
    }
    
    float (*op)(float*) = __shops[shape * OPERATION_TYPE_COUNT + operation];

    if(op == NULL) {
        return -1;
    }
    
    return op(params);
}
