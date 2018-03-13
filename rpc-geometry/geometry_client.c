#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <rpc/rpc.h>

#include "logger.h"

#include "types.h"
#include "_geometry.h"

#define SERVER "localhost"
#define REQUEST_COUNT 10

void halt_error(char *s1, char *s2) {
    fprintf(stderr, "%s: %s\n", s1, s2);
    exit(1);
}

int main(int argc, char *argv[]) {
    logger_init(time(NULL));
    
    CLIENT *client;
    
    geometry_op_query_t query;
    request_t request_table[REQUEST_COUNT];
    request_t *request;
    geometry_op_result_t *geometry_result;
    int i, p;

    
    glog(LOG_INFO, "Creating RPC client");
    if ((client = clnt_create(SERVER, GEOMETRY, GEOMETRY_VERSION, "udp")) == NULL) {
        clnt_pcreateerror(argv[0]);
        exit(0);
    }

    srand(time(NULL));

    glog(LOG_INFO, "Starting requests");
    for (i = 0; i <= REQUEST_COUNT; i++) {
        query.shape = rand() % SHAPE_COUNT;
        query.operation = rand() % OPERATION_TYPE_COUNT;
        for(p = 0; p < SHAPE_OP_PARAMS; p++) {
            query.params[p] = 286 * (1.0f * rand() / RAND_MAX);
        }

        printf("%3i: Posilam pozadavek: Shape: %1i, Operation: %1i\n",
                i, query.shape, query.operation);

        request = geometry_op_calc_1(&query, client);
        if (request == NULL)
            halt_error(argv[0], "Vzdalene volani geometry_op_calc() selhalo.");
        if (request->handle < 0)
            halt_error(argv[0], "Server nema zadne volne handly.");

        memcpy(request_table + i, request, sizeof (request_t));
    }

    glog(LOG_INFO, "Waiting for results");
    for (i = 0; i <= REQUEST_COUNT; i++) {
        geometry_result = geometry_op_retrieve_1(&request_table[i], client);
        if (geometry_result == NULL){
            halt_error(argv[0], "Vzdalene volani geometry_op_retrieve() selhalo.");
        }   
        if (geometry_result->state == response_state_err) {
            halt_error(argv[0], "Server odmitnul vratit vysledek pozadavku.");
        }
            
        
        printf("%3i: Vysledek pozadavku: %6.2f\n",
                i, geometry_result->value);
    }

    glog(LOG_INFO, "Shutting down");
    clnt_destroy(client);
    return 0;
}
