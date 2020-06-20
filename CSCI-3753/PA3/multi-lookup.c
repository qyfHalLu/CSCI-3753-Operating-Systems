#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#include "util.h"
#include "safe_queue.h"
#include "multi-lookup.h"

#define MAX_NAME_LENGTH 1025

/*
 * Service some hostname files, read a list of domain and push
 * the shared queue.
 */
void* requester_thread(void* arg) {
    RequesterArg* req_arg = (RequesterArg*)arg;
    int i = 0;
    char hostname[MAX_NAME_LENGTH];
    for (i = 0; i < req_arg->num; ++i) {
        FILE* f = fopen(req_arg->filenames[i], "r");
        if (!f) {
            fprintf(stderr, "Can't open %s\n", req_arg->filenames[i]);
            continue;
        }
        while (fgets(hostname, MAX_NAME_LENGTH-1, f)) {
            hostname[strlen(hostname)-1] = '\0';
            pushQueue(req_arg->sfq, strdup(hostname));
        }
        fclose(f);
    }
    pthread_mutex_lock(req_arg->service_lock);
    fprintf(req_arg->serviced_file,
            "Thread %ld serviced %d files.\n",
            gettid(), req_arg->num);
    pthread_mutex_unlock(req_arg->service_lock);
    return NULL;
}

/*
 * Get the domain names from shared queue.
 * Querying it's IP address write to the file.
 * Until the shared queue is empty and all requester threads stop.
 */
void* solver_thread(void* arg) {
    SolverArg* sol_arg = arg;
    while (!(*sol_arg->request_ending) || !emptyQueue(sol_arg->sfq)) {
        char* hostname = popQueue(sol_arg->sfq, 100);
        char ip[INET6_ADDRSTRLEN];
        ip[0] = '\0';
        if (hostname) {
            dnslookup(hostname, ip, INET6_ADDRSTRLEN);
        }
        pthread_mutex_lock(sol_arg->result_lock);
        fprintf(sol_arg->result_file, "%s,%s\n", hostname, ip);
        pthread_mutex_unlock(sol_arg->result_lock);
        free(hostname);
    }
    return NULL;
}

long int now_time() {
    struct timeval now;
    gettimeofday(&now, NULL);
    long int ms = now.tv_sec * 1000 + now.tv_usec / 1000;
    return ms;
}

int main(int argc, char *argv[])
{
    if (argc < 5) {
        printf("Usage: multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ...]\n");
        exit(EXIT_FAILURE);
    }
    int num_requester = atoi(argv[1]);
    int num_resolver = atoi(argv[2]);
    FILE* serviced_file = fopen(argv[3], "w");
    if (!serviced_file) {
        fprintf(stderr, "Invalid requester log\n");
        exit(EXIT_FAILURE);
    }
    FILE* result_file = fopen(argv[4], "w");
    if (!result_file) {
        fprintf(stderr, "Invalid resolver log\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_t service_lock, result_lock;
    pthread_mutex_init(&service_lock, NULL);
    pthread_mutex_init(&result_lock, NULL);
    int num_file = argc - 5;

    // init shared queue
    SafeQueue sfq = createQueue(100);
    // init the requester thread arguments.
    RequesterArg* req_args = malloc(num_requester * sizeof (struct RequesterArgImp));
    int i = 0, j = 0;
    for (i = 0; i < num_requester; ++i) {
        req_args[i].serviced_file = serviced_file;
        req_args[i].filenames = malloc(num_file * sizeof(char*));
        req_args[i].num = 0;
        req_args[i].sfq = sfq;
        req_args[i].service_lock = &service_lock;
        // set the process the filename which this thread process.
        for (j = 0; j < num_file; ++j) {
            if (j % num_requester == i) {
                req_args[i].filenames[req_args[i].num] = argv[j+5];
                req_args[i].num += 1;
            }
        }
    }

    int request_ending = 0; // flag for whether all requester threads is ending.
    // init the solver arguments.
    SolverArg* sol_args = malloc(num_resolver * sizeof (struct SolverArgImp));
    for (i = 0; i < num_resolver; ++i) {
        sol_args[i].request_ending = &request_ending;
        sol_args[i].result_file = result_file;
        sol_args[i].result_lock = &result_lock;
        sol_args[i].sfq = sfq;
    }

    long int begin = now_time();
    // lanch all thread.
    for (i = 0; i < num_requester; ++i) {
        pthread_create(&req_args[i].ptid, NULL, requester_thread, &req_args[i]);
    }
    for (i = 0; i < num_resolver; ++i) {
        pthread_create(&sol_args[i].ptid, NULL, solver_thread, &sol_args[i]);
    }

    // waiting all requester thread.
    for (i = 0; i < num_requester; ++i) {
        pthread_join(req_args[i].ptid, NULL);
    }
    request_ending = 1;
    for (i = 0; i < num_resolver; ++i) {
        pthread_join(sol_args[i].ptid, NULL);
    }

    long int cost = now_time() - begin;
    printf("Total run time: %ld\n", cost);

    for (i = 0; i < num_requester; ++i) {
        free(req_args[i].filenames);
    }
    free(req_args);
    free(sol_args);
    freeQueue(sfq);
    fclose(serviced_file);
    fclose(result_file);

    return 0;
}
