/*
 * video_sim.c - Video Streaming Storage Simulation
 * Task 6: Design Challenge - Video-Streaming Start-up
 * 
 * Simulates two storage architectures for a video streaming service
 * serving 200,000 students.
 *
 * Design A: SSD tier (hot) + HDD (cold), ext4, indexed alloc, C-LOOK
 * Design B: All HDD storage, XFS, contiguous alloc, SCAN
 *
 * Compile: gcc -std=c11 -Wall -Wextra -o video_sim video_sim.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ---- Configuration ---- */
#define NUM_VIDEOS       1000
#define MIN_SIZE_MB      300
#define MAX_SIZE_MB      2000
#define NUM_SIMULATIONS  10000

/* Design A: SSD + HDD tiered */
#define SSD_BLOCK_KB     64
#define SSD_SEQ_MBPS     2800.0   /* sequential read MB/s   */
#define SSD_RAND_US      100.0    /* random access latency us */
#define HDD_BLOCK_KB_A   64
#define HDD_SEQ_MBPS_A   180.0
#define HDD_RAND_MS      10.0    /* seek + rotational ms    */

/* Design B: All HDD */
#define HDD_BLOCK_KB_B   4
#define HDD_SEQ_MBPS_B   150.0
#define HDD_RAND_MS_B    12.0

/* ---- Structures ---- */
typedef struct {
    int    id;
    double size_mb;
    int    age_days;      /* 0 = just uploaded */
    int    view_count;
} Video;

typedef struct {
    double avg_seek_ms;
    double seq_throughput_mbps;
    double rand_throughput_mbps;
    double reads_to_minute30;
    int    concurrent_per_server;
} SimResult;

/* ---- Random helpers ---- */
static double rand_range(double lo, double hi)
{
    return lo + (double)rand() / RAND_MAX * (hi - lo);
}

static int rand_int(int lo, int hi)
{
    return lo + rand() % (hi - lo + 1);
}

/* ---- Zipf-like popularity (most views in first week) ---- */
static int generate_view_count(int age_days)
{
    if (age_days <= 7)
        return rand_int(500, 5000);
    else if (age_days <= 30)
        return rand_int(50, 500);
    else
        return rand_int(1, 50);
}

/* ---- Simulate Design A: SSD + HDD tiered ---- */
static SimResult simulate_design_a(Video videos[], int n)
{
    SimResult res = {0};
    double total_seek = 0;
    int    ssd_reads = 0, hdd_reads = 0;

    for (int i = 0; i < NUM_SIMULATIONS; i++) {
        int vi = rand_int(0, n - 1);
        Video *v = &videos[vi];

        /* Pick a random minute to seek to */
        double duration_min = v->size_mb / (5.0 * 60 / 8.0); /* 5 Mbps */
        double target_min   = rand_range(0, duration_min);
        double target_mb    = target_min / duration_min * v->size_mb;

        int blocks = (int)(v->size_mb * 1024 / SSD_BLOCK_KB);
        int target_block = (int)(target_mb * 1024 / SSD_BLOCK_KB);
        (void)blocks; (void)target_block;

        if (v->age_days <= 7) {
            /* SSD tier: 1 inode + 1 extent + 1 data = 3 reads */
            total_seek += SSD_RAND_US / 1000.0; /* ms */
            ssd_reads++;
            res.reads_to_minute30 += 3;
        } else {
            /* HDD tier: 1 inode + 2 extent + 1 seek + 1 data = 5 reads */
            total_seek += HDD_RAND_MS;
            hdd_reads++;
            res.reads_to_minute30 += 5;
        }
    }

    res.avg_seek_ms          = total_seek / NUM_SIMULATIONS;
    res.seq_throughput_mbps   = (ssd_reads > 0) ? SSD_SEQ_MBPS : HDD_SEQ_MBPS_A;
    res.rand_throughput_mbps  = SSD_SEQ_MBPS * 0.5; /* estimate */
    res.reads_to_minute30    /= NUM_SIMULATIONS;
    /* Concurrent streams: 5 Mbps each */
    res.concurrent_per_server = (int)(SSD_SEQ_MBPS * 8 / 5); /* Mbps conversion */

    return res;
}

/* ---- Simulate Design B: All HDD ---- */
static SimResult simulate_design_b(Video videos[], int n)
{
    SimResult res = {0};
    double total_seek = 0;

    for (int i = 0; i < NUM_SIMULATIONS; i++) {
        int vi = rand_int(0, n - 1);
        Video *v = &videos[vi];

        double duration_min = v->size_mb / (5.0 * 60 / 8.0);
        double target_min   = rand_range(0, duration_min);
        (void)target_min; (void)duration_min;

        /* HDD: contiguous alloc, SCAN scheduler */
        /* 1 directory + 1 seek + 1 data read; SCAN adds avg 1/3 disk sweep */
        total_seek += HDD_RAND_MS_B;
        res.reads_to_minute30 += (v->size_mb > 500) ? 6 : 4;
    }

    res.avg_seek_ms          = total_seek / NUM_SIMULATIONS;
    res.seq_throughput_mbps   = HDD_SEQ_MBPS_B;
    res.rand_throughput_mbps  = 1.5;
    res.reads_to_minute30    /= NUM_SIMULATIONS;
    res.concurrent_per_server = (int)(HDD_SEQ_MBPS_B * 8 / 5);

    return res;
}

/* ---- Main ---- */
int main(void)
{
    srand((unsigned)time(NULL));

    /* Generate video catalogue */
    Video videos[NUM_VIDEOS];
    for (int i = 0; i < NUM_VIDEOS; i++) {
        videos[i].id         = i;
        videos[i].size_mb    = rand_range(MIN_SIZE_MB, MAX_SIZE_MB);
        videos[i].age_days   = rand_int(0, 60);
        videos[i].view_count = generate_view_count(videos[i].age_days);
    }

    printf("=== Video Streaming Storage Simulation ===\n");
    printf("Videos: %d | Simulated accesses: %d\n\n", NUM_VIDEOS, NUM_SIMULATIONS);

    SimResult a = simulate_design_a(videos, NUM_VIDEOS);
    SimResult b = simulate_design_b(videos, NUM_VIDEOS);

    printf("+-------------------------------+-----------------------+-----------------------+\n");
    printf("| %-29s | %-21s | %-21s |\n", "Metric", "Design A (SSD+HDD)", "Design B (All HDD)");
    printf("+-------------------------------+-----------------------+-----------------------+\n");
    printf("| %-29s | %17.2f ms | %17.2f ms |\n", "Avg seek time",
           a.avg_seek_ms, b.avg_seek_ms);
    printf("| %-29s | %15.0f MB/s | %15.0f MB/s |\n", "Sequential throughput",
           a.seq_throughput_mbps, b.seq_throughput_mbps);
    printf("| %-29s | %15.1f MB/s | %15.1f MB/s |\n", "Random read throughput",
           a.rand_throughput_mbps, b.rand_throughput_mbps);
    printf("| %-29s | %18.1f   | %18.1f   |\n", "Avg reads to play min 30",
           a.reads_to_minute30, b.reads_to_minute30);
    printf("| %-29s | %18d   | %18d   |\n", "Concurrent streams/server",
           a.concurrent_per_server, b.concurrent_per_server);
    printf("+-------------------------------+-----------------------+-----------------------+\n");

    printf("\nConclusion: Design A (SSD + HDD tiered) is %.1fx faster seek,\n",
           b.avg_seek_ms / a.avg_seek_ms);
    printf("%.1fx higher throughput, and supports %dx more concurrent streams.\n",
           a.seq_throughput_mbps / b.seq_throughput_mbps,
           a.concurrent_per_server / (b.concurrent_per_server > 0 ? b.concurrent_per_server : 1));

    return 0;
}
