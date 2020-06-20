#include "safe_queue.h"
#include <pthread.h>

typedef struct RequesterArgImp {
    char** filenames;
    int num;
    SafeQueue sfq;
    FILE* serviced_file;
    pthread_mutex_t* service_lock;
    pthread_t ptid;
} RequesterArg;

typedef struct SolverArgImp {
    SafeQueue sfq;
    FILE* result_file;
    pthread_mutex_t* result_lock;
    int* request_ending;
    pthread_t ptid;
} SolverArg;

void* requester_thread(void* arg);
void* solver_thread(void* arg);


