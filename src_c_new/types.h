#ifndef  __NVM_TYPES__
#define __NVM_TYPES__

// this file contains the information required for maintaining the types
// of objects.

#include "globals.h"
#include "malloc.h"
#include "hashmap_tx.h"


#define LIB_TOID_NULL(t)	((LIB_TOID(t))MEMOID_NULL)


#define LIB_TOID_ASSIGN(o, value)(\
{\
	(o).oidkey = value;\
	(o); /* to avoid "error: statement with no effect" */\
})


#define LIB_TOID_EQUALS(lhs, rhs)\
(get_MEMoid((lhs).oid).off == get_MEMoid((rhs).oid).off &&\
	get_MEMoid((lhs).oid).pool_uuid_lo == get_MEMoid((rhs).oid).pool_uuid_lo)


// dummys to make sure the <space> (ex: struct blah) has no effect
#define _lib_toid_struct
#define _lib_toid_union
#define _lib_toid_enum


// Typed Object 
#define LIB_TOID(t)\
union _lib_toid_##t##_toid

// Declaration of type
// Note:- only one instance of __COUNTER__ exists in preporcess!!!!
#define LIB_TOID_DECLARE(t)\
typedef uint8_t _lib_toid_##t##_toid_type_num[__COUNTER__ + 1];\
LIB_TOID(t)\
{\
	MEMoidKey oidkey;\
	t *_type;\
	_lib_toid_##t##_toid_type_num *_type_num;\
}

// Type number
#define LIB_TOID_TYPE_NUM(t) (sizeof(_lib_toid_##t##_toid_type_num) - 1)


//Type number of object read from typed OID
#define LIB_TOID_TYPE_NUM_OF(o) (sizeof(*(o)._type_num) - 1)


// NULL check
#define LIB_TOID_IS_NULL(o)	(get_MEMoid((o).oidkey).off == 0)

// Actual type of stored object
#define LIB_TOID_TYPEOF(o) __typeof__(*(o)._type)

// Direct Write
#define LIB_DIRECT_RW(o) (\
{__typeof__(o) _o; _o._type = NULL; (void)_o;\
(__typeof__(*(o)._type) *)get_memobj_direct(get_MEMoid((o).oidkey)); })

// Direct Read
#define LIB_DIRECT_RO(o) ((const __typeof__(*(o)._type) *)get_memobj_direct(get_MEMoid((o).oidkey)))

//#define LIB_TX_BEGIN uv_mutex_lock(&object_maintainence_hashmap_mutex)
//#define LIB_TX_END uv_mutex_unlock(&object_maintainence_hashmap_mutex)
#define LIB_TX_BEGIN
#define LIB_TX_END

#define LIB_D_RW	LIB_DIRECT_RW
#define LIB_D_RO	LIB_DIRECT_RO

// Fuctions

void init_types_table();
// It returns the MEMoid struct after querrying the HashTable.
MEMoid get_MEMoid(MEMoidKey key);

void insert_object_to_hashmap(MEMoidKey key, MEMoid oid);

void remove_object_from_hashmap(MEMoidKey key);

// Will be used by the logistics thread
TOID(struct hashmap_tx) *get_types_map();

void debug_hashmap(MEMoidKey key);


// void* _key_get_first(MEMoidKey key);
// void* _key_get_last(MEMoidKey key)
// #define KEY_FIRST(key) (_key_get_first(key))
// #define KEY_LAST(key) (_key_get_last(key))

#endif // ! __NVM_TYPES__
