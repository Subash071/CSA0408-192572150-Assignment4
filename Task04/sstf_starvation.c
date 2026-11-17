#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIM_TIME_MS 1000
#define STARVING_CYL 183
#define LOCAL_CLUSTER_MIN 55
#define LOCAL_CLUSTER_MAX 65
#define INIT_HEAD 53
#define MAX_QUEUE 2048

typedef struct {
    int cylinder;
    int arrival_time;
    int id;
} DiskRequest;

/* Pure SSTF simulation */
static void simulate_pure_sstf(void)
{
    printf("========================================================================================\n");
    printf("                  EXPERIMENT 1: PURE SSTF DISK SCHEDULING (STARVATION)\n");
    printf("========================================================================================\n");
    printf("Configuration:\n");
    printf(" - Initial Head Position : %d\n", INIT_HEAD);
    printf(" - Starving Target       : Cylinder %d (Arrived at t = 0 ms)\n", STARVING_CYL);
    printf(" - Workload Stream       : New requests near cylinder 60 [%d-%d] arriving every 1 ms\n",
           LOCAL_CLUSTER_MIN, LOCAL_CLUSTER_MAX);
    printf(" - Total Simulation Time : %d ms\n\n", SIM_TIME_MS);

    DiskRequest queue[MAX_QUEUE];
    int q_size = 0;

    /* Enqueue initial request at cylinder 183 */
    queue[q_size].cylinder = STARVING_CYL;
    queue[q_size].arrival_time = 0;
    queue[q_size].id = 0;
    q_size++;

    int head = INIT_HEAD;
    int local_serviced = 0;
    bool starving_req_serviced = false;
    int serviced_at_time = -1;

    printf("+----------+------------------------+------------+--------------------------------------+\n");
    printf("| Time(ms) | Local Reqs Serviced    | Head Pos   | Cylinder 183 Status                  |\n");
    printf("+----------+------------------------+------------+--------------------------------------+\n");

    srand(42);

    for (int t = 1; t <= SIM_TIME_MS; t++) {
        /* A new request arrives near cylinder 60 */
        int new_cyl = LOCAL_CLUSTER_MIN + (rand() % (LOCAL_CLUSTER_MAX - LOCAL_CLUSTER_MIN + 1));
        if (q_size < MAX_QUEUE) {
            queue[q_size].cylinder = new_cyl;
            queue[q_size].arrival_time = t;
            queue[q_size].id = t;
            q_size++;
        }

        /* SSTF selection: find request with minimum seek distance from head */
        int best_idx = -1;
        int best_dist = 1000000;
        for (int i = 0; i < q_size; i++) {
            int d = abs(queue[i].cylinder - head);
            if (d < best_dist) {
                best_dist = d;
                best_idx = i;
            }
        }

        /* Service selected request */
        DiskRequest chosen = queue[best_idx];
        head = chosen.cylinder;

        if (chosen.cylinder == STARVING_CYL) {
            starving_req_serviced = true;
            serviced_at_time = t;
            /* Remove chosen */
            for (int i = best_idx; i < q_size - 1; i++) queue[i] = queue[i + 1];
            q_size--;
            break;
        } else {
            local_serviced++;
            /* Remove chosen */
            for (int i = best_idx; i < q_size - 1; i++) queue[i] = queue[i + 1];
            q_size--;
        }

        /* Print periodic status reports */
        if (t == 10 || t == 50 || t == 100 || t == 250 || t == 500 || t == 1000) {
            printf("| %8d | %22d | %10d | STARVING (Waited %4d ms, dist=%3d) |\n",
                   t, local_serviced, head, t, abs(STARVING_CYL - head));
        }
    }
    printf("+----------+------------------------+------------+--------------------------------------+\n\n");

    if (!starving_req_serviced) {
        printf("RESULT: Cylinder %d suffered COMPLETE STARVATION!\n", STARVING_CYL);
        printf(" - Serviced %d local requests near cylinder 60 while Request %d remained unserviced.\n",
               local_serviced, STARVING_CYL);
        printf(" - Reason: SSTF is a greedy local optimizer. Because incoming local requests are\n");
        printf("   within %d-%d cylinders of the head, distance to local requests (~0-12 cyl) is always\n",
               LOCAL_CLUSTER_MIN, LOCAL_CLUSTER_MAX);
        printf("   substantially smaller than distance to cylinder 183 (~%d cyl).\n\n", abs(STARVING_CYL - head));
    } else {
        printf("RESULT: Request %d was serviced at t = %d ms.\n\n", STARVING_CYL, serviced_at_time);
    }
}

/* SSTF with Aging simulation */
static void simulate_sstf_with_aging(void)
{
    printf("========================================================================================\n");
    printf("                  EXPERIMENT 2: SSTF WITH AGING (STARVATION MITIGATION)\n");
    printf("========================================================================================\n");
    printf("Aging Heuristic: Priority Score = abs(cylinder - head) - (alpha * waiting_time)\n");
    printf("Parameters     : alpha = 0.50 (older requests receive progressively higher priority)\n\n");

    DiskRequest queue[MAX_QUEUE];
    int q_size = 0;

    queue[q_size].cylinder = STARVING_CYL;
    queue[q_size].arrival_time = 0;
    queue[q_size].id = 0;
    q_size++;

    int head = INIT_HEAD;
    int local_serviced = 0;
    int serviced_at_time = -1;
    double alpha = 0.50;

    srand(42);

    for (int t = 1; t <= SIM_TIME_MS; t++) {
        int new_cyl = LOCAL_CLUSTER_MIN + (rand() % (LOCAL_CLUSTER_MAX - LOCAL_CLUSTER_MIN + 1));
        if (q_size < MAX_QUEUE) {
            queue[q_size].cylinder = new_cyl;
            queue[q_size].arrival_time = t;
            queue[q_size].id = t;
            q_size++;
        }

        /* Select request minimizing priority score */
        int best_idx = -1;
        double best_score = 1e9;
        for (int i = 0; i < q_size; i++) {
            int seek_dist = abs(queue[i].cylinder - head);
            int wait_time = t - queue[i].arrival_time;
            double score = (double)seek_dist - (alpha * (double)wait_time);
            if (score < best_score) {
                best_score = score;
                best_idx = i;
            }
        }

        DiskRequest chosen = queue[best_idx];
        head = chosen.cylinder;

        /* Remove chosen from queue */
        for (int i = best_idx; i < q_size - 1; i++) queue[i] = queue[i + 1];
        q_size--;

        if (chosen.cylinder == STARVING_CYL) {
            serviced_at_time = t;
            break;
        } else {
            local_serviced++;
        }
    }

    if (serviced_at_time != -1) {
        printf("RESULT: Starvation Resolved! Request %d was successfully serviced at t = %d ms.\n",
               STARVING_CYL, serviced_at_time);
        printf(" - Local requests serviced before Request %d: %d\n", local_serviced);
        printf(" - Waiting time for Request %d: %d ms\n", serviced_at_time);
        printf(" - Aging successfully bounded maximum latency by elevating the starving request's\n");
        printf("   priority over newly arriving local requests.\n\n");
    }
}

int main(void)
{
    printf("########################################################################################\n");
    printf("#                    DEMONSTRATION OF DISK HEAD STARVATION IN SSTF                     #\n");
    printf("########################################################################################\n\n");

    simulate_pure_sstf();
    simulate_sstf_with_aging();

    printf("========================================================================================\n");
    printf("                                   SUMMARY & CONCLUSION                                 \n");
    printf("========================================================================================\n");
    printf("1. Vulnerability: Pure Shortest Seek Time First (SSTF) guarantees minimum average seek\n");
    printf("   time locally, but is prone to starvation under heavy or localized request patterns.\n");
    printf("2. Root Cause: Requests at the peripheral tracks (like cylinder 183) can be delayed\n");
    printf("   indefinitely if traffic continuously feeds near the current cylinder head (~60).\n");
    printf("3. Practical Solutions:\n");
    printf("   - Elevator Algorithms (SCAN, C-SCAN, LOOK): Sweep across tracks sequentially, ensuring\n");
    printf("     every cylinder is serviced within a bounded cycle time.\n");
    printf("   - Aging / Deadline Schedulers: As implemented in production operating systems\n");
    printf("     (e.g., Linux BFQ and Deadline schedulers), expiry timers guarantee service bounds.\n");

    return 0;
}
