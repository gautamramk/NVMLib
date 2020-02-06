#ifndef  __NVM_TYPES__
#define __NVM_TYPES__

// this file contains the information required for maintaining the types
// of objects.

#include "globals.h"
#include "malloc.h"


#define TOID_NULL(t)	((TOID(t))OID_NULL)


#define TOID_ASSIGN(o, value)(\
{\
	(o).oid = value;\
	(o); /* to avoid "error: statement with no effect" */\
})


#define TOID_EQUALS(lhs, rhs)\
((lhs).oid.off == (rhs).oid.off &&\
	(lhs).oid.pool_uuid_lo == (rhs).oid.pool_uuid_lo)


// dummys to make sure the <space> (ex: struct blah) has no effect
#define _toid_struct
#define _toid_union
#define _toid_enum


// Typed Object 
#define TOID(t)\
union _toid_##t##_toid

// declaration of type
#define TOID_DECLARE(t)\
typedef uint8_t _toid_##t##_toid_type_num[get_next_type_num() + 1];\
TOID(t)\
{\
	MEMoid oid;\
	t *_type;\
	_toid_##t##_toid_type_num *_type_num;\
}

// Type number
#define TOID_TYPE_NUM(t) (sizeof(_toid_##t##_toid_type_num) - 1)


//Type number of object read from typed OID
#define TOID_TYPE_NUM_OF(o) (sizeof(*(o)._type_num) - 1)


// NULL check
#define TOID_IS_NULL(o)	((o).oid.off == 0)

// Actual type of stored object
#define TOID_TYPEOF(o) __typeof__(*(o)._type)

// Direct Write
#define DIRECT_RW(o) (\
{__typeof__(o) _o; _o._type = NULL; (void)_o;\
(__typeof__(*(o)._type) *)get_memobj_direct((o).oid); })

// Direct Read
#define DIRECT_RO(o) ((const __typeof__(*(o)._type) *)get_memobj_direct((o).oid))

#define D_RW	DIRECT_RW
#define D_RO	DIRECT_RO

// Fuctions

uint64_t get_current_type_num();

uint64_t get_next_type_num();


#endif // ! __NVM_TYPES__
