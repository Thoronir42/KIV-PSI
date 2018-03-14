#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <pthread.h>
#include <unistd.h>

#include "logger.h"

#include "types.h"
#include "_geometry.h"

#define REQUEST_COUNT 4

typedef struct {
    int req_id;
    request_t request;
    geometry_op_query_t query;
    geometry_op_result_t result;
} req_struct;

req_struct _rs[REQUEST_COUNT];

// fixme: global is ugly
static CLIENT *client;

void *request_function(void* arg) {
    req_struct *rs = (req_struct *) arg;

    request_t *req;
    geometry_op_result_t *response;
    
    glog(LOG_INFO, "Thr %3i: Sending request: Shape: %1i, Operation: %1i\n",
            rs->req_id, rs->query.shape, rs->query.operation);

    req = geometry_op_calc_1(&rs->query, client);
    if (req == NULL) {
        glog(LOG_ERROR, "Thr %3i: RPC geometry_op_calc() failed", rs->req_id);
        return NULL;
    }
    if (req->handle < 0) {
        glog(LOG_WARNING, "Thr %3i: Server handle resources currently unavailable.", rs->req_id);
        return NULL;
    }
    
    rs->request = *req;
    glog(LOG_INFO, "Thr %3i-%3i: handle assigned (seq-n = %d)", rs->req_id, rs->request.handle, rs->request._seq_n);

    response = geometry_op_retrieve_1(&rs->request, client);
    if (response == NULL) {
        glog(LOG_ERROR, "Thr %3i-%3i: RPC geometry_op_retrieve() failed", rs->req_id, rs->request.handle);
        return NULL;
    }
    if (response->state != response_state_ok) {
//        glog(LOG_WARNING, "Thr %3i: Server returned error(%d) on geometry_op_retrieve()", rs->req_id, req->state);
        glog(LOG_WARNING, "Thr %3i-%3i: Server returned error(%d) on geometry_op_retrieve() result = %f", rs->req_id, rs->request.handle, response->state, response->value);
        return NULL;
    }
    rs->result = *response;

    glog(LOG_INFO, "Thr %3i-%3i: Vysledek pozadavku: %6.2f\n", rs->req_id, rs->request.handle, rs->result.value);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    logger_init_file(stdout);

    pthread_t request_threads[REQUEST_COUNT];
    int i, p;


    glog(LOG_INFO, "Creating RPC client");
    client = clnt_create("localhost", GEOMETRY, GEOMETRY_VERSION, "udp");
    if (client == NULL) {
        clnt_pcreateerror(argv[0]);

        return 1;
    }

    srand(time(NULL));

    glog(LOG_INFO, "Starting requests");

    for (i = 0; i < REQUEST_COUNT; i++) {
        req_struct *rs = _rs + i;

        rs->req_id = i;
        rs->query.shape = rand() % SHAPE_COUNT;
        rs->query.operation = rand() % OPERATION_TYPE_COUNT;
        for (p = 0; p < SHAPE_OP_PARAMS; p++) {
            rs->query.params[p] = 286 * (1.0f * rand() / RAND_MAX);
        }
        pthread_create(request_threads + i, NULL, request_function, (void *) rs);
        usleep(50 * 10000);
    }

    for (i = 0; i < REQUEST_COUNT; i++) {
        pthread_join(request_threads[i], NULL);
    }

    glog(LOG_INFO, "All threads finished, shutting down");
    clnt_destroy(client);
    return 0;
}
