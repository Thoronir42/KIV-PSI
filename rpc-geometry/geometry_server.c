/* RPC - procedury na strane serveru
 * Zpracoval: Jen Jezek, FAV 5. rocnik, 2001
 */

#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include <sched.h> /* Funguje jenom na linuixu! */
#include <unistd.h>
#include <pthread.h>

#include "types.h"
#include "_geometry.h"

#include "shape_operator.h"
#include "logger.h"



#define MAX_REQUESTS 128
#define THREAD_STACK_SIZE 16384

typedef struct {
    request_t request;
    geometry_op_query_t input;
    geometry_op_result_t output;
} REQUEST_ITEM;

static REQUEST_ITEM error_respose;
static REQUEST_ITEM request_table[MAX_REQUESTS];

static int first_run = 1;
static pthread_mutex_t handle_mutex = PTHREAD_MUTEX_INITIALIZER;

static int processed_requests = 0;

void *thread_func(void *v_request_item) {
    REQUEST_ITEM *request_item = (REQUEST_ITEM *) v_request_item;

    geometry_op_query_t *query = &request_item->input;
    geometry_op_result_t *result = &request_item->output;

    char param_buffer[256];
    char param[32];
    int p;


    memset(param_buffer, 0, 256);

    for (p = 0; p < SHAPE_OP_PARAMS; p++) {
        snprintf(param, 32, "%d:%6.2f, ", p, query->params[p]);
        strncat(param_buffer, param, 256);
    }

    glog(LOG_INFO, "Req %3i: Request processing: Shape: %i, Operation:%i,\n\t Params:[%s]\n",
            request_item->request.handle, query->shape, query->operation, param_buffer);

    useconds_t comp_time = 1000 + (rand() % 4000);

    glog(LOG_INFO, "Req %3d: Computing... time remaining: %4.2f s", request_item->request.handle, comp_time / 1000.0f);
    request_item->request.state = request_state_computing;
    usleep(comp_time * 1000);


    result->value = shape_op_execute(query->shape, query->operation, query->params);
    request_item->request.state = request_state_finished;

    glog(LOG_INFO, "Req %3d: Computations finished, wating for retrival", request_item->request.handle);

    return NULL;
}

void first_run_init() {
    logger_init_file(stdout);
    shape_operator_init();
    
    int i;
    first_run = 0;
    for (i = 0; i < MAX_REQUESTS; i++) {
        request_table[i].request.state = request_state_idle;
    }

    error_respose.request.handle = -1;
    error_respose.output.value = -1;
    error_respose.output.value = response_state_err;

    printf("Inicializace serveru dokoncena.\n");
}

request_t *geometry_op_calc_1_svc(geometry_op_query_t *query, struct svc_req *b) {
    int i;
    int index;

    /* Pri prvnim behu vymazat tabulku pozadavku. */
    if (first_run) {
        first_run = 0;
        first_run_init();
    }
    request_t *request = &error_respose.request;

    /* Najit volnou pozici pro pozadavek. */
    pthread_mutex_lock(&handle_mutex);
    index = -1;
    for (i = 0; i < MAX_REQUESTS; i++) {
        if (request_table[i].request.state == request_state_idle ) {
            request = &request_table[i].request;
            request->state = request_state_reserved;
            request->handle = i;
            request->_seq_n = ++processed_requests;
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&handle_mutex);
    glog(LOG_INFO, "Assigning %d-th handle %d", processed_requests, index);

    // -1 should be ok bc before request_table there is error_request
    
    
    

    /* Spustit vlakno pro obsluhu pozadavku. */

    if (index >= 0) {
        request_table[index].input = *query;

        pthread_t thread;
        pthread_create(&thread, NULL, thread_func, (void *) (request_table + index));

        glog(LOG_INFO, "Req %3i: Thread created", index);
    } else {
        glog(LOG_WARNING, "Req %3i: Request queue is full, unable to process request.", index);
    }

    return &request_table[index].request;
}

geometry_op_result_t *geometry_op_retrieve_1_svc(request_t *request, struct svc_req *b) {
    if (request->handle < 0 || request->handle >= MAX_REQUESTS) {
        glog(LOG_WARNING, "Req %3i: Invallid handle result requested.\n", request->handle);
        return &error_respose.output;
    }

    REQUEST_ITEM *ri = request_table + request->handle;


    
    /* Pockame, kdyz neni dopocitano. */
    if (ri->request.state == request_state_computing || ri->request.state == request_state_reserved) {
        glog(LOG_INFO, "Req %3i: Pozadovan vysledek, ceka se na dopocitani.\n", request->handle);
//        int w = 0;
        while (ri->request.state == request_state_computing || ri->request.state == request_state_reserved) {
            /*if (++w >= 1000) {
                w = 0;
                glog(LOG_INFO, "Req %3d: still waiting", request->handle);
            }*/
            usleep(100);
            // noop
        }
    }

    /* Vyzvedneme vysledek. */
    if (ri->request.state != request_state_finished && ri->request.state != request_state_retrieved) {
        glog(LOG_WARNING, "Req %3i: Computation resulted in error state(%d)", request->handle, ri->request.state);
        ri->output.state = response_state_err;
    }

    glog(LOG_INFO, "Req %3i: Result %f retrieved.\n", request->handle, ri->output.value);
    ri->request.state = request_state_retrieved;

    return &ri->output;
}
