#include "types.h"

struct request_t {
    int handle;
    request_state_t state;

};

struct geometry_op_query_t {
    shape_t shape;
    shape_op_t operation;
    float params[SHAPE_OP_PARAMS];
};

struct geometry_op_result_t {
    response_state_t state;
    float value;
};

typedef struct request_t request_t;
typedef struct geometry_op_query_t geometry_op_query_t;
typedef struct geometry_op_result_t geometry_op_result_t;

program GEOMETRY{
    version GEOMETRY_VERSION{
        struct request_t geometry_op_calc(struct geometry_op_query_t) = 1;
        struct geometry_op_result_t geometry_op_retrieve(struct request_t) = 2;
    } = 1;
} = 0xF00D00;

