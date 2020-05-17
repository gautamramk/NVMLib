#ifndef __MEM_LOG_H__
#define __MEM_LOG_H__
#include "malloc.h"
#include <sys/queue.h>

typedef struct address_log {
    void* addr;
    size_t size;
    TAILQ_ENTRY(address_log) list;
} address_log;

TAILQ_HEAD(address_list_queue, address_log) write_queue_head;
TAILQ_HEAD(address_list_queue, address_log) read_queue_head;

void log_write(void* addr, size_t size);
void log_read(void* addr, size_t size);

#endif //__MEM_LOG_H__