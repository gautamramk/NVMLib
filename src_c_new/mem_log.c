#include "mem_log.h"
#include "malloc.h"
#include <time.h>
#include <libiberty/splay-tree.h>
// #include <splay-tree.h>

void initialize_log_queues() {
    TAILQ_INIT(&write_queue_head);
    TAILQ_INIT(&read_queue_head);
}

void log_write(void* addr, size_t size) {
    address_log* ad_log = (address_log*)malloc(sizeof(address_log));
    ad_log->addr = addr;
    ad_log->size = size;
    time(&ad_log->access_time);
    TAILQ_INSERT_TAIL(&write_queue_head, ad_log, list);
}

void log_read(void* addr, size_t size) {
    address_log* ad_log = (address_log*)malloc(sizeof(address_log));
    ad_log->addr = addr;
    ad_log->size = size;
    time(&ad_log->access_time);
    TAILQ_INSERT_TAIL(&read_queue_head, ad_log, list);
}