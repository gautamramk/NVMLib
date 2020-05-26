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
    ad_log->size = size >> 3;

    addr2memoid_key skey;
    skey.comp = cmp_addr;
    skey.addr = addr;
    time(&ad_log->access_time);
    uv_mutex_lock(&write_splay_tree_mutex);
    splay_tree_node ret_node = splay_tree_lookup(addr2MemOID_write, (splay_tree_key)&skey);

    MEMoidKey mkey = ((addr2memoid_key*)ret_node->key)->key;
    ad_log->key = mkey;
    ad_log->offset = addr - KEY_FIRST(mkey);
    uv_mutex_unlock(&write_splay_tree_mutex);
    TAILQ_INSERT_TAIL(&write_queue_head, ad_log, list);
}

void log_read(void* addr, size_t size) {
    address_log* ad_log = (address_log*)malloc(sizeof(address_log));
    ad_log->size = size >> 3;

    addr2memoid_key skey;
    skey.comp = cmp_addr;
    skey.addr = addr;
    time(&ad_log->access_time);
    uv_mutex_lock(&read_splay_tree_mutex);
    splay_tree_node ret_node = splay_tree_lookup(addr2MemOID_read, (splay_tree_key)&skey);

    MEMoidKey mkey = ((addr2memoid_key*)ret_node->key)->key;
    ad_log->key = mkey;
    ad_log->offset = addr - KEY_FIRST(mkey);
    uv_mutex_unlock(&read_splay_tree_mutex);
    TAILQ_INSERT_TAIL(&read_queue_head, ad_log, list);
}