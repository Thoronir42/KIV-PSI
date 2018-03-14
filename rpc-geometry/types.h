#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define SHAPE_COUNT 4
#define OPERATION_TYPE_COUNT 3
#define SHAPE_OP_PARAMS 6

    typedef char shape_t;
#define shape_point 1
#define shape_line 2
#define shape_rectangle 3
#define shape_block 4

    typedef char shape_op_t;
#define shape_op_perimeter 1
#define shape_op_surface 2
#define shape_op_volume 3

typedef char request_state_t;
#define request_state_idle  1
#define request_state_computing  2
#define request_state_reserved 3
#define request_state_finished  4
#define request_state_retrieved 5

typedef char response_state_t;
#define response_state_ok 0
#define response_state_err 1    


#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */

