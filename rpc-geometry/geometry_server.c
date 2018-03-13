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

void *thread_func(void *v_request_item) {
    REQUEST_ITEM *request_item = (REQUEST_ITEM *) v_request_item;

    geometry_op_query_t *query = &request_item->input;
    geometry_op_result_t *result = &request_item->output;

    result->value = shape_op_execute(query->shape, query->operation, query->params);

    request_item->request.state = request_state_finished;
    printf("%3i: Pozadavek ceka na vyzvednuti vysledku.\n",
            request_item->request.handle);

    return 0;
}

void first_run_init() {
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
    int i, p;
    int index;
    request_t *result;

    /* Pri prvnim behu vymazat tabulku pozadavku. */
    if (first_run) {
        first_run = 0;
        first_run_init();
    }

    /* Najit volnou pozici pro pozadavek. */
    index = -1;
    for (i = 0; i < MAX_REQUESTS; i++) {
        if (request_table[i].request.state == request_state_idle) {
            index = i;
            break;
        }
    }

    // -1 should be ok bc before request_table there is error_request
    result = &request_table[index].request;
    result->handle = index;

    /* Spustit vlakno pro obsluhu pozadavku. */

    if (index >= 0) {
        printf("%3i: Pozadavek byl zahajen: Shape: %i, Operation:%i,\n\t Params",
                index, query->shape, query->operation);
        for (p = 0; p < SHAPE_OP_PARAMS; p++) {
            printf("%6.2f, ", query->params[p]);
        }
        printf("\n");

        memcpy(&request_table[index].input, query, sizeof (geometry_op_query_t));
        request_table[index].request.state = request_state_computing;

        pthread_t thread;
        pthread_create(&thread, NULL, thread_func, (void *) request_table + index);

        glog(LOG_INFO, "Thread %d created", thread);
    } else {
        printf("%3i: !! Pozadavek nelze zpracovat, je plna fronta.\n", index);
    }

    return result;
}

geometry_op_result_t *geometry_op_retrieve_1_svc(request_t *request, struct svc_req *b) {
    if (request->handle < 0 || request->handle >= MAX_REQUESTS) {
        return &error_respose.output;
    }

    REQUEST_ITEM *ri = request_table + request->handle;

    geometry_op_result_t *result;


    /* Pockame, kdyz neni dopocitano. */
    if (ri->request.state == request_state_computing) {
        printf("%3i: Pozadovan vysledek, ceka se na dopocitani.\n", request->handle);
        while (ri->request.state == request_state_computing) {
            usleep(100);
            // noop
        }
    }

    /* Vyzvedneme vysledek. */
    if (ri->request.state == request_state_finished) {
        result = &ri->output;
        ri->request.state = request_state_retrieved;
        result->state = 1;
        printf("%3i: Vyzvednut vysledek.\n", request->handle);
    } else {
        result->state = 0;
        printf("%3i: !! Pozadovan vysledek neexistujiciho pozadavku.\n", request->handle);
    }

    return result;
}
