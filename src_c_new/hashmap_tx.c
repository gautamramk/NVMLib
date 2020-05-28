/**
 * @file
 * The implemantation of a Persistent Hashmap using only transaction APIs
 * of `libpmemobj`
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include "hashmap_tx.h"

/* large prime number used as a hashing function coefficient */
#define HASH_FUNC_COEFF_P 32212254719ULL

/* initial number of buckets */
#define INIT_BUCKETS_NUM 10

/* number of values in a bucket which trigger hashtable rebuild check */
#define MIN_HASHSET_THRESHOLD 5

/* number of values in a bucket which force hashtable rebuild */
#define MAX_HASHSET_THRESHOLD 10

/**
 * Hashmap initializer
 * 
 * This is the function called by hm_tx_create() for creation of the hashmap.
 * @param PMEMobjpool: the `pool` where the `hashmap` would be stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param seed: the seed for initialising hashing varaibles.
 * @return nothing
 * 
 * @note This is an *internal function*. Not exposed to other files.
 * @see hm_tx_create()
 */
static void
create_hashmap(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, uint32_t seed)
{
	size_t len = INIT_BUCKETS_NUM;
	size_t sz = sizeof(struct buckets) +
			len * sizeof(TOID(struct entry));

	TX_BEGIN(pop) {
		TX_ADD(hashmap);

		D_RW(hashmap)->seed = seed;
		do {
			D_RW(hashmap)->hash_fun_a = (uint32_t)rand();
		} while (D_RW(hashmap)->hash_fun_a == 0);
		D_RW(hashmap)->hash_fun_b = (uint32_t)rand();
		D_RW(hashmap)->hash_fun_p = HASH_FUNC_COEFF_P;

		D_RW(hashmap)->buckets = TX_ZALLOC(struct buckets, sz);
		D_RW(D_RW(hashmap)->buckets)->nbuckets = len;
	} TX_ONABORT {
		fprintf(stderr, "%s: create transaction aborted: %s\n", __func__,
			pmemobj_errormsg());
		abort();
	} TX_END
}

/**
 * The hash function. Its the simplest hashing function.
 * 
 * This function is used to get the hash for a `key` in the hashmap.
 * @param hashmap: the hashmap which is being accessed
 * @param buckets: the pointer to the buckets in the `hashmap`
 * @param value: the `value` that needs to be hashed. * 
 * 
 * @note This is an *internal function*. Not exposed to other files.
 * @see <a href="https://en.wikipedia.org/wiki/Universal_hashing#Hashing_integers"> For more info on the hash function</a>
 * @see hm_tx_insert() hm_tx_remove() hm_tx_get() hm_tx_lookup()
 */
static uint64_t
hash(const TOID(struct hashmap_tx) *hashmap,
	const TOID(struct buckets) *buckets, uint64_t value)
{
	uint32_t a = D_RO(*hashmap)->hash_fun_a;
	uint32_t b = D_RO(*hashmap)->hash_fun_b;
	uint64_t p = D_RO(*hashmap)->hash_fun_p;
	size_t len = D_RO(*buckets)->nbuckets;

	return ((a * value + b) % p) % len;
}

/**
 * Rebuilds the hashmap when the *hashmap size threshold* is reached.
 * 
 * The `hashmap` is rebuilt, i.e its size is increased/decreased, when its `size`
 * exceeds the `MAX_HASHSET_THRESHOLD` or when the number of elements in the `hashmap`
 * are less than the number of `buckets` in the `hashmap`.
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param new_len: the length of the resulting `hashmap` after resizing.
 * @return nothing
 * 
 * @note This is an *internal function*. Not exposed to other files.
 * @see hm_tx_insert() hm_tx_remove()
 */
static void
hm_tx_rebuild(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, size_t new_len)
{
	TOID(struct buckets) buckets_old = D_RO(hashmap)->buckets;

	if (new_len == 0)
		new_len = D_RO(buckets_old)->nbuckets;

	size_t sz_old = sizeof(struct buckets) +
			D_RO(buckets_old)->nbuckets *
			sizeof(TOID(struct entry));
	size_t sz_new = sizeof(struct buckets) +
			new_len * sizeof(TOID(struct entry));

	TX_BEGIN(pop) {
		TX_ADD_FIELD(hashmap, buckets);
		TOID(struct buckets) buckets_new =
				TX_ZALLOC(struct buckets, sz_new);
		D_RW(buckets_new)->nbuckets = new_len;
		pmemobj_tx_add_range(buckets_old.oid, 0, sz_old);

		for (size_t i = 0; i < D_RO(buckets_old)->nbuckets; ++i) {
			while (!TOID_IS_NULL(D_RO(buckets_old)->bucket[i])) {
				TOID(struct entry) en =
					D_RO(buckets_old)->bucket[i];
				uint64_t h = hash(&hashmap, &buckets_new,
						D_RO(en)->key);

				D_RW(buckets_old)->bucket[i] = D_RO(en)->next;

				TX_ADD_FIELD(en, next);
				D_RW(en)->next = D_RO(buckets_new)->bucket[h];
				D_RW(buckets_new)->bucket[h] = en;
			}
		}

		D_RW(hashmap)->buckets = buckets_new;
		TX_FREE(buckets_old);
	} TX_ONABORT {
		fprintf(stderr, "%s: rebuild transaction aborted: %s\n", __func__,
			pmemobj_errormsg());
		/*
		 * We don't need to do anything here, because everything is
		 * consistent. The only thing affected is performance.
		 */
	} TX_END

}

/**
 * Inserts the specified `value` into the `hashmap`.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param key: the `key` for the `value` being inserted. It will be used to calculate the `hash`.
 * @param value: the `value` that is supposed to be inserted.
 * @return 0: if successful
 * @return 1: if `value` already existed
 * @return -1: if the insertion failed
 * 
 * @warning If the `key` already exists in the `hashmap`, then the `value` provided is **not** inserted.
 */
int
hm_tx_insert(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
	uint64_t key, MEMoid value)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	uint64_t h = hash(&hashmap, &buckets, key);
	int num = 0;

	for (var = D_RO(buckets)->bucket[h];
			!TOID_IS_NULL(var);
			var = D_RO(var)->next) {
		if (D_RO(var)->key == key)
			return 1;
		num++;
	}

	int ret = 0;
	TX_BEGIN(pop) {
		TX_ADD_FIELD(D_RO(hashmap)->buckets, bucket[h]);
		TX_ADD_FIELD(hashmap, count);

		TOID(struct entry) e = TX_NEW(struct entry);
		D_RW(e)->key = key;
		D_RW(e)->value = value;
		D_RW(e)->next = D_RO(buckets)->bucket[h];
		D_RW(buckets)->bucket[h] = e;

		D_RW(hashmap)->count++;
		num++;
	} TX_ONABORT {
		fprintf(stderr, "insert transaction aborted: %s\n",
			pmemobj_errormsg());
		ret = -1;
	} TX_END

	if (ret)
		return ret;

	if (num > MAX_HASHSET_THRESHOLD ||
			(num > MIN_HASHSET_THRESHOLD &&
			D_RO(hashmap)->count > 2 * D_RO(buckets)->nbuckets))
		hm_tx_rebuild(pop, hashmap, D_RO(buckets)->nbuckets * 2);

	return 0;
}

/**
 * Removes specified `value` from the `hashmap`.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param key: the `key` for the `value` being inserted. It will be used to calculate the `hash`.
 * @return the `value` associated with the `key` deleted upon success 
 * @return MEMOID_NULL upon failure or if not such `key` existed
 * 
 * @note This function does call hm_tx_rebuild() if the the number of elements in the 
 * `hashmap` are less that number of `buckets`.
 */
MEMoid
hm_tx_remove(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, uint64_t key)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var, prev = TOID_NULL(struct entry);

	uint64_t h = hash(&hashmap, &buckets, key);
	for (var = D_RO(buckets)->bucket[h];
			!TOID_IS_NULL(var);
			prev = var, var = D_RO(var)->next) {
		if (D_RO(var)->key == key)
			break;
	}

	if (TOID_IS_NULL(var))
		return MEMOID_NULL;
	int ret = 0;

	MEMoid retoid = D_RO(var)->value;
	TX_BEGIN(pop) {
		if (TOID_IS_NULL(prev))
			TX_ADD_FIELD(D_RO(hashmap)->buckets, bucket[h]);
		else
			TX_ADD_FIELD(prev, next);
		TX_ADD_FIELD(hashmap, count);

		if (TOID_IS_NULL(prev))
			D_RW(buckets)->bucket[h] = D_RO(var)->next;
		else
			D_RW(prev)->next = D_RO(var)->next;
		D_RW(hashmap)->count--;
		TX_FREE(var);
	} TX_ONABORT {
		fprintf(stderr, "remove transaction aborted: %s\n",
			pmemobj_errormsg());
		ret = -1;
	} TX_END

	if (ret)
		return MEMOID_NULL;

	if (D_RO(hashmap)->count < D_RO(buckets)->nbuckets)
		hm_tx_rebuild(pop, hashmap, D_RO(buckets)->nbuckets / 2);

	return retoid;
}

/**
 * Calls a given `callback` for every element in the `hashmap`.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param cb: the `calback` function.
 * @param arg: additional arguments for the `callback` function
 * @return The return value of the `callback` function.
 * 
 */
int
hm_tx_foreach(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
	int (*cb)(uint64_t key, MEMoid value, void *arg), void *arg)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	int ret = 0;
	for (size_t i = 0; i < D_RO(buckets)->nbuckets; ++i) {
		if (TOID_IS_NULL(D_RO(buckets)->bucket[i]))
			continue;

		for (var = D_RO(buckets)->bucket[i]; !TOID_IS_NULL(var);
				var = D_RO(var)->next) {
			ret = cb(D_RO(var)->key, D_RO(var)->value, arg);
			if (ret)
				break;
		}
	}

	return ret;
}

/**
 * Prints complete hashmap state. This function is used for debugging purposes.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` would be stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param out: the `file desciptor` for where the output needs to be recorded.
 * @return Nothing
 * 
 * @note The `out` paramater can be `stdout` and `stderr` too.
 */
void
hm_tx_debug(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, FILE *out)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	fprintf(out, "a: %u b: %u p: %" PRIu64 "\n", D_RO(hashmap)->hash_fun_a,
		D_RO(hashmap)->hash_fun_b, D_RO(hashmap)->hash_fun_p);
	fprintf(out, "count: %" PRIu64 ", buckets: %zu\n",
		D_RO(hashmap)->count, D_RO(buckets)->nbuckets);

	for (size_t i = 0; i < D_RO(buckets)->nbuckets; ++i) {
		if (TOID_IS_NULL(D_RO(buckets)->bucket[i]))
			continue;

		int num = 0;
		fprintf(out, "%zu: ", i);
		for (var = D_RO(buckets)->bucket[i]; !TOID_IS_NULL(var);
				var = D_RO(var)->next) {
			fprintf(out, "%" PRIu64 " ", D_RO(var)->key);
			num++;
		}
		fprintf(out, "(%d)\n", num);
	}
}

/**
 * Checks whether specified `key` is in the `hashmap` and retries the `value` associated with it.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param key: the `key` for the `value` being inserted. It will be used to calculate the `hash`.
 * @return the `value` associated with the `key`.
 * @return MEMOID_NULL upon failure or if not such `key` existed
 */
MEMoid
hm_tx_get(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, uint64_t key)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	uint64_t h = hash(&hashmap, &buckets, key);

	for (var = D_RO(buckets)->bucket[h];
			!TOID_IS_NULL(var);
			var = D_RO(var)->next)
		if (D_RO(var)->key == key)
			return D_RO(var)->value;

	return MEMOID_NULL;
}

/**
 * Checks whether specified `key` exists in the `hashmap`.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param key: the `key` for the `value` being inserted. It will be used to calculate the `hash`.
 * @return 1: if the `key` exists
 * @return 0: otherwise
 * 
 * @see hm_tx_get()
 */
int
hm_tx_lookup(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, uint64_t key)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	uint64_t h = hash(&hashmap, &buckets, key);

	for (var = D_RO(buckets)->bucket[h];
			!TOID_IS_NULL(var);
			var = D_RO(var)->next)
		if (D_RO(var)->key == key)
			return 1;

	return 0;
}

/**
 * Returns number of elements in the `hashmap`.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @return Number of elements in the `hashmap`.
 */
size_t
hm_tx_count(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap)
{
	return D_RO(hashmap)->count;
}

/**
 * Recovers `hashmap` state. Called after pmemobj_open().
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * 
 * @see init_types_table()
 */
int
hm_tx_init(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap)
{
	srand(D_RO(hashmap)->seed);
	return 0;
}

/**
 * Allocates a new `hashmap`.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` would be stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param arg: additional arguments if necessary
 * @return 0: on successful creation
 * @return -1: on failure.
 * 
 * @see create_hashmap()
 */
int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {
	#ifdef DEBUG
		printf("map address %p\n", (*map).oid.off);
	#endif	
		//TX_ADD_DIRECT(map);
		//*map = TX_ZNEW(struct hashmap_tx);
		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		printf("hm create errno = %d\n", errno);
		ret = -1;
	} TX_END

	return ret;
}

/**
 * Checks if specified `persistent` object is an instance of `hashmap`.
 * 
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @return 0: if it is a `hashmap`
 * @return 1: otherwise
 * 
 */
int
hm_tx_check(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap)
{
	return TOID_IS_NULL(hashmap) || !TOID_VALID(hashmap);
}

/**
 * Executes a command for `hashmap`.
 * 
 * Available commands are: `HASHMAP_CMD_REBUILD` and `HASHMAP_CMD_DEBUG`
 * @param PMEMobjpool: the `pool` where the `hashmap` is stored.
 * @param hashmap: the *typed* hashmap which needs to be initialised.
 * @param cmd: the command to be executed.
 * @param arg: the arguments for the command.
 * @return 0: on successful execution
 * @return -`EINVAL`: otherwise
 */
int
hm_tx_cmd(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
		unsigned cmd, uint64_t arg)
{
	switch (cmd) {
		case HASHMAP_CMD_REBUILD:
			hm_tx_rebuild(pop, hashmap, arg);
			return 0;
		case HASHMAP_CMD_DEBUG:
			if (!arg)
				return -EINVAL;
			hm_tx_debug(pop, hashmap, (FILE *)arg);
			return 0;
		default:
			return -EINVAL;
	}
}
