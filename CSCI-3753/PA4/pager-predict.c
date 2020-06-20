/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "simulator.h"


/*
Genaral Ideas:
Generally this pager is an lru pager with aggressive page preloading based on statistic info

For each pageit(mem access) request
1. Record transition for each process
2. Preload MRU page and most frequently hit page for each process after next 101 instrcutions(pagein latency)
3. Preload as much history transition target as possible for each process after next 101 instrcutions(pagein latency)

Extra Considerations:
Aggressive preloading costs physical frames heavily
Pageout pages once it is been used
Pageout all pages for a process on its unloading
Sort history transition targets to preload as much high value pages as possible(NOT WORING AS EXPECTED, DEPRECATED)
*/

#define NIL ((int)0xFFFFFFFF)
#define GET_PAGE(pc) ((pc) / PAGESIZE)
#define PAGEING_LATENCY 101

typedef struct page_stat {
    int hits;
    int last_ts;
} page_stat;

int page_stat_cmp(page_stat* l, page_stat* r) {
    int lhits = l->hits == NIL ? 0 : l->hits;
    int rhits = r->hits == NIL ? 0 : r->hits;
    return rhits - lhits;
    // int lts = l->last_ts == NIL ? 0 : l->last_ts;
    // int rts = r->last_ts == NIL ? 0 : r->last_ts;
    // return lts - rts;
}

page_stat transit_graph[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];
void init_trasition_graph() {
    memset(transit_graph, 0xFF, sizeof(page_stat) * MAXPROCPAGES * MAXPROCPAGES * MAXPROCPAGES);
}
void clear_transitions_for_inactive_proc(int pid) {
    memset(transit_graph[pid], 0xFF, sizeof(page_stat) * MAXPROCPAGES * MAXPROCPAGES);
}

void update_transition_graph(int pid, Pentry p, int tick) {
    static int initialized = 0;
    static int last_pages[MAXPROCESSES];

    if(!initialized) {
        memset(last_pages, 0, sizeof(int) * MAXPROCESSES);
        initialized = 1;
    }
    if(!p.active) {
        return;
    }
    int last_page = last_pages[pid];
    int current_page = GET_PAGE(p.pc);
    last_pages[pid] = current_page;
    // printf("INSERT FOR PROC %d, LASTPAGE %d, CURRENT PAGE %d\n", pid, last_page, current_page);
    if(last_page == current_page) {
        // Same page, skip
        return;
    }
    pageout(pid, last_page);
    if(transit_graph[pid][last_page][current_page].hits == NIL) {
        // printf("INSERT FOR PROC %d, LASTPAGE %d, CURRENT PAGE %d, TS %d,\n", pid, last_page, current_page, tick);
        transit_graph[pid][last_page][current_page].hits = 1;
        transit_graph[pid][last_page][current_page].last_ts = tick;
    } else {
        // printf("UPDATE FOR PROC %d, LASTPAGE %d, CURRENT PAGE %d, TS %d,\n", pid, last_page, current_page, tick);
        transit_graph[pid][last_page][current_page].hits++;
        transit_graph[pid][last_page][current_page].last_ts = tick;
    }
}

void print_transition_for_pid(int tick, int pid) {
    int start = 0;
    // printf("PROC:%d\t", pid);
    for(int j = 0; j < MAXPROCPAGES; j++) {
        for(int k = 0; k < MAXPROCPAGES; k++) {
            if(transit_graph[pid][j][k].hits != NIL) {
                if(!start) {
                    printf("PROC:%d TICK:%d\t", pid, tick);
                    start = 1;
                }
            printf("%d -> %d: (%d, %d); ",j, k,
                transit_graph[pid][j][k].hits,
                transit_graph[pid][j][k].last_ts);
            }
        }
    }
    if(!start) {
        //printf("no trasition available for proc %d @ %d\n", pid, tick);
        return;
    }
    fflush(stdout);
    // assert(0);
    printf("\n");
}
void print_transit_graph(int tick) {
    for(int i = 0; i < MAXPROCESSES; i++) {
        print_transition_for_pid(tick, i);
    }
}

void print_sorted_transitions(page_stat trans[MAXPROCPAGES]) {
    for(int i = 0; i < MAXPROCPAGES; i++) {
        if(trans[i].hits == NIL) {
            break;
        }
        printf("%d(%d,%d), ", i, trans[i].hits, trans[i].last_ts);
    }
    printf("\n");
}

// Get Most Recently Used Page & Most Probable Transit Target
void predict_incoming_page(int pid, int page, int* most_recent, int* most_probable) {
    page_stat* entry = transit_graph[pid][page];

    int most_recent_tick = -1;
    int most_recent_page = NIL;
    for(int i = 0; i < MAXPROCPAGES; i++) {
        if(entry[i].last_ts > most_recent_tick) {
            most_recent_page = i;
            most_recent_tick = entry[i].last_ts;
        }
    }

    int most_probable_hits = -1;
    int most_probable_page = NIL;
    for(int i = 0; i < MAXPROCPAGES; i++) {
        if(entry[i].hits > most_probable_hits) {
            most_probable_hits = entry[i].hits;
            most_probable_page = i;
        }
    }

    *most_recent = most_recent_page;
    *most_probable = most_probable_page;
}

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for a predictive pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    

    /* initialize static vars on first run */
    if(!initialized){
        /* Init complex static vars here */
        init_trasition_graph();
        for(int proc = 0; proc < MAXPROCESSES; proc++){
            for(int page =0; page < MAXPROCPAGES; page++){
                timestamps[proc][page] = 0; 
            }
        }
        initialized = 1;
    }
    
    /* TODO: Implement Predictive Paging */
    // Update Transition Record
    for(int p = 0; p < MAXPROCESSES; p++) {
        if(q[p].active != 1) {
            // Clear records for inactive proc to reset the statistic info
            clear_transitions_for_inactive_proc(p);
            continue;
        }
        update_transition_graph(p, q[p], tick);
    }
    // print_transit_graph(tick);

    // LRU Paging
    for(int p = 0; p < MAXPROCESSES; p++) {
        if(q[p].active != 1){
            // Not active, skip
            continue;
        }
        // update_transition_graph(p, q[p], tick);
        int page = q[p].pc / PAGESIZE;
        timestamps[p][page] = tick;
        if(q[p].pages[page] == 1) {
            // Already Swapped in, skip
            continue;
        }

        if(pagein(p, page) == 1) {
            // Pagein started, skip
            continue;
        }

        // pagein failed, need evict
        int idx = -1;
        int lru = INT32_MAX;
        for(int i = 0; i < q[p].npages; i++) {
            if(lru > timestamps[p][i] && q[p].pages[i] != 0) {
                lru = timestamps[p][i];
                idx = i;
            }
        }
        if(idx != -1) {
            pageout(p, idx);
        }
    }

    // Predict upcoming page
    // print_transition_for_pid(tick, p);
    for(int p = 0; p < MAXPROCESSES; p++) {
        if(q[p].active != 1){
            // Not active, skip
            continue;
        }
        page_stat transitions[MAXPROCPAGES];
        // Assume no branching in next 101(page in latency) instructions
        int next_page = GET_PAGE(q[p].pc + PAGEING_LATENCY);
        // Pagein most probable transition and most recent used transition for next_page
        int most_recent;
        int most_probable;
        predict_incoming_page(p, next_page, &most_recent, &most_probable);
        if(most_probable != NIL) {
            pagein(p, most_probable);
        }
        if(most_recent != NIL) {
            pagein(p, most_recent);
        }

        // Pagein as much probable pages as possible, Discard pagein failures(due to no frames, etc)
        memcpy(transitions, transit_graph[p][next_page], sizeof(page_stat) * MAXPROCPAGES);
        // qsort(transitions, MAXPROCPAGES, sizeof(page_stat), page_stat_cmp);
        // print_sorted_transitions(transitions);
        int cnt = 0;
        for(int i = 0; i < MAXPROCPAGES; i++) {
            if(transitions[i].hits != NIL) {
                pagein(p, i);
                cnt++;
            }
        }
        // printf("PAGED IN %d probable pages @ %d\n", cnt, tick);
    }
    /* advance time for next pageit iteration */
    tick++;
} 
