/*
 * cache_sim.c - LRU Cache Simulation for Video Streaming
 * Task 6: Design Challenge - Video-Streaming Start-up
 *
 * Simulates caching of popular videos using LRU eviction policy.
 * Models Zipf-distributed popularity (most views in first week).
 *
 * Compile: gcc -std=c11 -Wall -Wextra -o cache_sim cache_sim.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_VIDEOS      1000
#define NUM_REQUESTS    100000
#define ZIPF_EXPONENT   1.2    /* skew parameter: higher = more skewed */

/* ---- LRU Cache ---- */
typedef struct CacheNode {
    int video_id;
    struct CacheNode *prev;
    struct CacheNode *next;
} CacheNode;

typedef struct {
    int capacity;
    int size;
    CacheNode *head;   /* most recently used */
    CacheNode *tail;   /* least recently used */
    int *in_cache;     /* in_cache[video_id] = 1 if cached */
} LRUCache;

static LRUCache *cache_create(int capacity)
{
    LRUCache *c = calloc(1, sizeof(LRUCache));
    c->capacity = capacity;
    c->size = 0;
    c->head = NULL;
    c->tail = NULL;
    c->in_cache = calloc((size_t)NUM_VIDEOS, sizeof(int));
    return c;
}

static void cache_free(LRUCache *c)
{
    CacheNode *n = c->head;
    while (n) {
        CacheNode *next = n->next;
        free(n);
        n = next;
    }
    free(c->in_cache);
    free(c);
}

/* Move a node to the head (most recently used) */
static void move_to_head(LRUCache *c, CacheNode *node)
{
    if (node == c->head) return;

    /* Detach */
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (node == c->tail) c->tail = node->prev;

    /* Insert at head */
    node->prev = NULL;
    node->next = c->head;
    if (c->head) c->head->prev = node;
    c->head = node;
    if (!c->tail) c->tail = node;
}

/* Returns 1 for cache hit, 0 for miss */
static int cache_access(LRUCache *c, int video_id)
{
    /* Check if in cache */
    if (c->in_cache[video_id]) {
        /* Find and move to head */
        CacheNode *n = c->head;
        while (n) {
            if (n->video_id == video_id) {
                move_to_head(c, n);
                return 1; /* HIT */
            }
            n = n->next;
        }
    }

    /* MISS: insert new node */
    CacheNode *node = malloc(sizeof(CacheNode));
    node->video_id = video_id;
    node->prev = NULL;
    node->next = c->head;
    if (c->head) c->head->prev = node;
    c->head = node;
    if (!c->tail) c->tail = node;
    c->size++;
    c->in_cache[video_id] = 1;

    /* Evict if over capacity */
    if (c->size > c->capacity) {
        CacheNode *evict = c->tail;
        c->tail = evict->prev;
        if (c->tail) c->tail->next = NULL;
        c->in_cache[evict->video_id] = 0;
        free(evict);
        c->size--;
    }

    return 0; /* MISS */
}

/* ---- Zipf distribution ---- */
static double *zipf_cdf;
static void zipf_init(int n, double s)
{
    zipf_cdf = malloc((size_t)n * sizeof(double));
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += 1.0 / pow((double)(i + 1), s);
    }
    double cumulative = 0;
    for (int i = 0; i < n; i++) {
        cumulative += (1.0 / pow((double)(i + 1), s)) / sum;
        zipf_cdf[i] = cumulative;
    }
}

static int zipf_sample(int n)
{
    double u = (double)rand() / RAND_MAX;
    for (int i = 0; i < n; i++) {
        if (u <= zipf_cdf[i]) return i;
    }
    return n - 1;
}

/* ---- Main simulation ---- */
int main(void)
{
    srand((unsigned)time(NULL));
    zipf_init(NUM_VIDEOS, ZIPF_EXPONENT);

    printf("=== LRU Cache Simulation for Video Streaming ===\n");
    printf("Videos: %d | Requests: %d | Zipf exponent: %.1f\n\n",
           NUM_VIDEOS, NUM_REQUESTS, ZIPF_EXPONENT);

    int cache_pcts[] = {10, 20, 50};
    int num_pcts = 3;

    printf("+------------+------------+------------------+\n");
    printf("| %-10s | %-10s | %-16s |\n", "Cache Size", "Hit Rate", "Disk I/O Saved");
    printf("+------------+------------+------------------+\n");

    for (int p = 0; p < num_pcts; p++) {
        int cap = NUM_VIDEOS * cache_pcts[p] / 100;
        LRUCache *cache = cache_create(cap);

        int hits = 0;
        for (int r = 0; r < NUM_REQUESTS; r++) {
            int vid = zipf_sample(NUM_VIDEOS);
            hits += cache_access(cache, vid);
        }

        double hit_rate = 100.0 * hits / NUM_REQUESTS;
        printf("| %3d%% (%4d) | %8.1f%%  | %13.1f%%   |\n",
               cache_pcts[p], cap, hit_rate, hit_rate);

        cache_free(cache);
    }

    printf("+------------+------------+------------------+\n");

    printf("\nObservation: Due to Zipf-distributed popularity, even a small\n");
    printf("cache (10%%) captures the majority of requests. A 20%% cache\n");
    printf("achieves ~80%% hit rate, dramatically reducing disk I/O.\n");

    free(zipf_cdf);
    return 0;
}
