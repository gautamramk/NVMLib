/**
 * This file is a demo for how `object_maintainence.c` would work.
 * The idea here is:
 *  - There are two arrays: 1. HASHMAP_DRAM and 
 *                          2. HASHMAP_NVRAM 
 *  - There are two loops: 1. Logistics loop (for moving objects) runs every MOVE_LOOP_SLEEP_TIME secs
 *                         2. Deletion loop (for deleting objects) runs every DELETE_LOOP_SLEEP_TIME secs
 *  - There is one more thread, `thread_maintainer`, which is required to run the two loops
 *  - There `main thread` prints out the array
**/

// Compile: `$ gcc uv_loop_demo_example.c -luv -lpthread`

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <uv.h>

#define DELETE_LOOP_SLEEP_TIME 5000    // 5 milli secs
#define MOVE_LOOP_SLEEP_TIME 5000    // 5 milli secs
#define MAIN_LOOP_SLEEP_TIME 10000    // 5 milli secs

#define ARRAY_SIZE 20

int HASHMAP_DRAM[ARRAY_SIZE];
int HASHMAP_NVRAM[ARRAY_SIZE];

uv_mutex_t nvram_mutex;
uv_mutex_t dram_mutex;

void array_initialise() {
    for (int i=0; i<ARRAY_SIZE; i++) {
        HASHMAP_DRAM[i] = rand() % 100;
        HASHMAP_NVRAM[i] = 0;
    }
}

void print_array(int arr[], int size, char* msg) {
    printf("%s\n", msg);
    for(int i=0; i<size-1; i++) {
        printf("%d, ", arr[i]);
    }
    printf("%d\n", arr[size-1]);
}

void move_from_dram(uv_work_t *req) {
    int idx = *((int*)req->data);

    uv_mutex_lock(&nvram_mutex);
    uv_mutex_lock(&dram_mutex);

    printf("Move from DRAM to NVRAM: %d at [%ld ms]\n", idx, (uv_hrtime() / 1000000) % 100000);
    HASHMAP_NVRAM[idx] = HASHMAP_DRAM[idx];
    HASHMAP_DRAM[idx] = 0;
    // HASHMAP_DRAM[idx] = rand() % 100;

    sleep(5);
    uv_mutex_unlock(&dram_mutex);
    uv_mutex_unlock(&nvram_mutex);
}

void move_from_nvram(uv_work_t *req) {
    int idx = *((int*)req->data);

    uv_mutex_lock(&dram_mutex);
    uv_mutex_lock(&nvram_mutex);

    printf("Move from NVRAM to DRAM: %d at [%ld ms]\n", idx, (uv_hrtime() / 1000000) % 100000);
    HASHMAP_DRAM[idx] = HASHMAP_NVRAM[idx];
    HASHMAP_NVRAM[idx] = 0;
    // HASHMAP_NVRAM[idx] = rand() % 100;

    sleep(2);
    uv_mutex_unlock(&nvram_mutex);
    uv_mutex_unlock(&dram_mutex);
}

void delete_from_dram(uv_work_t *req) {
    int idx = *((int*)req->data);

    uv_mutex_lock(&dram_mutex);

    printf("Delete from DRAM: %d at [%ld ms]\n", idx, (uv_hrtime() / 1000000) % 100000);
    HASHMAP_DRAM[idx] = 0;

    uv_mutex_unlock(&dram_mutex);
}

void delete_from_nvram(uv_work_t *req) {
    int idx = *((int*)req->data);

    uv_mutex_lock(&nvram_mutex);

    printf("Delete from NVRAM: %d at [%ld ms]\n", idx, (uv_hrtime() / 1000000) % 100000);
    HASHMAP_NVRAM[idx] = 0;

    uv_mutex_unlock(&nvram_mutex);
}

void on_after_work(uv_work_t* req, int status) {
  free(req);
}


void on_logistics_timer(uv_timer_t *timer, int status) {
    uint64_t timestamp = uv_hrtime();
    printf("on_logistics_timer [%ld ms]\n", (timestamp / 1000000) % 100000);
    uv_work_t* work_req = (uv_work_t*)malloc(sizeof(*work_req));
    if (rand() % 2) {
        work_req->data = malloc(sizeof(int));
        *((int *)(work_req->data)) = rand() % ARRAY_SIZE;
        // *((int *)(work_req->data)) = 11;
        uv_queue_work(timer->loop, work_req, move_from_nvram, on_after_work);
    } else {
        work_req->data = malloc(sizeof(int));
        *((int *)(work_req->data)) = rand() % ARRAY_SIZE;
        // *((int *)(work_req->data)) = 11;
        uv_queue_work(timer->loop, work_req, move_from_dram, on_after_work);
    }
}

void on_deletion_timer(uv_timer_t *timer, int status) {
    uint64_t timestamp = uv_hrtime();
    printf("on_deletion_timer [%ld ms]\n", (timestamp / 1000000) % 100000);
    uv_work_t* work_req = (uv_work_t*)malloc(sizeof(*work_req));
    if (rand() % 2) {
        work_req->data = malloc(sizeof(int));
        *((int *)(work_req->data)) = rand() % ARRAY_SIZE;
        // *((int *)(work_req->data)) = 11;

        uv_queue_work(timer->loop, work_req, delete_from_nvram, on_after_work);
    } else {
        work_req->data = malloc(sizeof(int));
        *((int *)(work_req->data)) = rand() % ARRAY_SIZE;
        // *((int *)(work_req->data)) = 11;

        uv_queue_work(timer->loop, work_req, delete_from_dram, on_after_work);
    }
}

void *logistics_thread_function(void *data){
    uv_loop_t *thread_loop = (uv_loop_t *) data;
    fprintf(stderr, "Logistics thread will start event loop\n");

    uv_timer_t timer_logistics;
    uv_timer_init(thread_loop, &timer_logistics);
    uv_timer_start(&timer_logistics, on_logistics_timer, 0, MOVE_LOOP_SLEEP_TIME);

    //Start this loop
    uv_run(thread_loop, UV_RUN_DEFAULT);
    pthread_exit(NULL);
}

void *deletion_thread_function(void *data){
    uv_loop_t *thread_loop = (uv_loop_t *) data;
    fprintf(stderr, "Delete thread will start event loop\n");

    uv_timer_t timer_deletion;
    uv_timer_init(thread_loop, &timer_deletion);
    uv_timer_start(&timer_deletion, on_deletion_timer, 0, DELETE_LOOP_SLEEP_TIME);

    //Start this loop
    uv_run(thread_loop, UV_RUN_DEFAULT);
    pthread_exit(NULL);
}

// void *thread_maintainer_thread(void *data){
//     uv_loop_t *thread_loop = (uv_loop_t *) data;
//     fprintf(stderr, "Thread-maintainer thread will start event loop\n");

//     // uv_timer_t timer_logistics;
//     // uv_timer_init(thread_loop, &timer_logistics);

//     // uv_timer_t timer_deletion;
//     // uv_timer_init(thread_loop, &timer_deletion);

//     // uv_timer_start(&timer_logistics, on_logistics_timer, 0, MOVE_LOOP_SLEEP_TIME);
//     // uv_timer_start(&timer_deletion, on_deletion_timer, 0, DELETE_LOOP_SLEEP_TIME);

//     //Start this loop
//     uv_run(thread_loop);
//     pthread_exit(NULL);
// }

int main() {
    srand(time(NULL));

    array_initialise();

    uv_mutex_init(&nvram_mutex);
    uv_mutex_init(&dram_mutex);

    pthread_t logistics_thread, deletion_thread;

    uv_loop_t *logistics_loop = uv_loop_new();
    pthread_create(&logistics_thread, NULL, logistics_thread_function, logistics_loop);

    uv_loop_t *deletion_loop = uv_loop_new();
    pthread_create(&deletion_thread, NULL, deletion_thread_function, deletion_loop);

    while(1) {
        uv_mutex_lock(&nvram_mutex);
        uv_mutex_lock(&dram_mutex);

        printf("==========================================\n");
        print_array(HASHMAP_DRAM, ARRAY_SIZE, "DRAM");
        printf("------------------------------------------\n");
        print_array(HASHMAP_NVRAM, ARRAY_SIZE, "NVRAM");
        printf("==========================================\n");

        
        uv_mutex_unlock(&dram_mutex);
        uv_mutex_unlock(&nvram_mutex);

        // sleep(MAIN_LOOP_SLEEP_TIME);
        
        // sleep in milliseconds
        struct timespec ts;
        ts.tv_sec = MAIN_LOOP_SLEEP_TIME / 1000;
        ts.tv_nsec = (MAIN_LOOP_SLEEP_TIME % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }

    return 0;
}
